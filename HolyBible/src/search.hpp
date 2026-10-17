// search.hpp - a small full-text search engine for the Scriptures.
//
//   * Inverted index (word -> verses) built in memory at startup
//   * BM25 relevance ranking (the same family of scoring used by search engines)
//   * "exact phrases", prefix wildcards (bless*), exclusions (-word)
//   * filters:  in:psalms  in:nt  in:ot  in:gospels  in:john ...
//   * typo correction ("fiath" -> "faith") using edit distance over the vocabulary
#pragma once
#include "bible.hpp"

#include <bitset>
#include <chrono>
#include <cmath>
#include <numeric>
#include <unordered_set>

namespace bible {

// ----------------------------------------------------------- tokenizing ----
inline bool isWordChar(unsigned char c) { return std::isalnum(c) || c == '\''; }

template <class F>
void forEachWord(std::string_view t, F&& f) {
    size_t i = 0, n = t.size();
    while (i < n) {
        while (i < n && !isWordChar((unsigned char)t[i])) ++i;
        size_t s = i;
        while (i < n && isWordChar((unsigned char)t[i])) ++i;
        if (i > s) f(s, i - s);
    }
}

// "LORD'S" -> "lord", "Israel's" -> "israel", "Selah." -> "selah"
inline std::string normWord(std::string_view raw) {
    std::string w;
    w.reserve(raw.size());
    for (char ch : raw) {
        unsigned char c = (unsigned char)ch;
        if (std::isalnum(c)) w += (char)std::tolower(c);
        else if (c == '\'') w += '\'';
    }
    if (w.size() > 2 && w.compare(w.size() - 2, 2, "'s") == 0) w.resize(w.size() - 2);
    w.erase(std::remove(w.begin(), w.end(), '\''), w.end());
    return w;
}

// ------------------------------------------------------------- results -----
struct SearchResult {
    std::vector<int> verses;                 // ranked by relevance
    std::vector<double> scores;
    std::vector<std::string> notes;          // e.g. "Showing results for faith"
    std::vector<std::string> wordStats;      // e.g. "faith x247"
    std::string error;
    std::unordered_set<std::string> highlight;
    double millis = 0;
    bool truncatedPrefix = false;
};

class SearchEngine {
public:
    struct Posting { int verse; uint16_t tf; };

    void build(const Bible& b) {
        auto t0 = std::chrono::steady_clock::now();
        bible_ = &b;
        const int N = b.size();
        std::unordered_map<std::string, int> tmpId;
        std::vector<std::string> words;
        std::vector<std::vector<Posting>> post;
        docLen_.assign(N, 0);
        double total = 0;
        std::vector<int> ids;
        for (int i = 0; i < N; ++i) {
            ids.clear();
            const std::string& text = b.verses[i].text;
            forEachWord(text, [&](size_t s, size_t l) {
                std::string w = normWord(std::string_view(text).substr(s, l));
                if (w.empty()) return;
                auto [it, inserted] = tmpId.try_emplace(w, (int)words.size());
                if (inserted) { words.push_back(w); post.emplace_back(); }
                ids.push_back(it->second);
            });
            docLen_[i] = (uint16_t)ids.size();
            total += ids.size();
            std::sort(ids.begin(), ids.end());
            for (size_t k = 0; k < ids.size();) {
                size_t j = k;
                while (j < ids.size() && ids[j] == ids[k]) ++j;
                post[ids[k]].push_back({i, (uint16_t)(j - k)});
                k = j;
            }
        }
        // Sort the vocabulary alphabetically so prefix searches are a binary search.
        std::vector<int> order(words.size());
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(), [&](int a, int c) { return words[a] < words[c]; });
        vocab_.clear(); postings_.clear(); freq_.clear(); wordId_.clear();
        for (int id : order) {
            wordId_[words[id]] = (int)vocab_.size();
            vocab_.push_back(words[id]);
            uint32_t f = 0;
            for (auto& p : post[id]) f += p.tf;
            freq_.push_back(f);
            postings_.push_back(std::move(post[id]));
        }
        totalWords_ = (long long)total;
        avgLen_ = N ? total / N : 1.0;
        buildMillis = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    }

    double buildMillis = 0;
    size_t vocabularySize() const { return vocab_.size(); }
    long long totalWords() const { return totalWords_; }

    // Closest known word (for "did you mean").
    std::string suggest(const std::string& w) const {
        if (w.size() < 3) return "";
        int limit = w.size() <= 4 ? 1 : 2;
        int bestD = limit + 1;
        uint32_t bestF = 0;
        std::string best;
        for (size_t i = 0; i < vocab_.size(); ++i) {
            const std::string& v = vocab_[i];
            if (std::abs((int)v.size() - (int)w.size()) > limit) continue;
            int d = levenshtein(w, v, limit);
            if (d < bestD || (d == bestD && freq_[i] > bestF)) { bestD = d; bestF = freq_[i]; best = v; }
        }
        return bestD <= limit ? best : "";
    }

    SearchResult search(const std::string& query) const {
        auto t0 = std::chrono::steady_clock::now();
        SearchResult res;

        struct Term { std::string text; bool prefix = false; std::vector<int> ids; };
        std::vector<Term> required, excluded;
        std::vector<std::vector<std::string>> phrases;
        std::bitset<66> mask;
        bool hasMask = false;

        // ---------- 1. parse the query
        size_t i = 0;
        const std::string& q = query;
        while (i < q.size()) {
            while (i < q.size() && std::isspace((unsigned char)q[i])) ++i;
            if (i >= q.size()) break;
            if (q[i] == '"') {                                  // "exact phrase"
                size_t e = q.find('"', i + 1);
                if (e == std::string::npos) e = q.size();
                std::string ph = q.substr(i + 1, e - i - 1);
                std::vector<std::string> words;
                forEachWord(ph, [&](size_t s, size_t l) {
                    auto w = normWord(std::string_view(ph).substr(s, l));
                    if (!w.empty()) words.push_back(w);
                });
                for (auto& w : words) required.push_back({w, false, {}});
                if (words.size() > 1) phrases.push_back(words);
                i = e + 1;
                continue;
            }
            size_t s = i;
            while (i < q.size() && !std::isspace((unsigned char)q[i])) ++i;
            std::string tok = q.substr(s, i - s);
            std::string low = toLower(tok);
            if (startsWith(low, "in:") && low.size() > 3) {      // filters
                hasMask = true;
                if (!applyFilter(low.substr(3), mask))
                    res.notes.push_back("Unknown filter \"" + tok + "\" ignored.");
                continue;
            }
            bool neg = tok[0] == '-';
            if (neg) tok.erase(0, 1);
            bool pre = !tok.empty() && tok.back() == '*';
            std::vector<std::string> words;
            forEachWord(tok, [&](size_t s2, size_t l) {
                auto w = normWord(std::string_view(tok).substr(s2, l));
                if (!w.empty()) words.push_back(w);
            });
            for (size_t k = 0; k < words.size(); ++k) {
                Term t{words[k], pre && k + 1 == words.size(), {}};
                (neg ? excluded : required).push_back(t);
            }
        }
        if (required.empty()) { res.error = "Type at least one word to search for."; return finish(res, t0); }

        // ---------- 2. resolve words to vocabulary ids (with typo correction)
        auto resolve = [&](Term& t, bool correct) -> bool {
            if (t.prefix) {
                auto it = std::lower_bound(vocab_.begin(), vocab_.end(), t.text);
                for (; it != vocab_.end() && startsWith(*it, t.text); ++it) {
                    if (t.ids.size() >= 400) { res.truncatedPrefix = true; break; }
                    t.ids.push_back((int)(it - vocab_.begin()));
                }
                return !t.ids.empty();
            }
            auto it = wordId_.find(t.text);
            if (it != wordId_.end()) { t.ids.push_back(it->second); return true; }
            if (!correct) return false;
            bool alpha = std::all_of(t.text.begin(), t.text.end(), [](char c) { return std::isalpha((unsigned char)c); });
            std::string sug = alpha ? suggest(t.text) : "";
            if (sug.empty()) return false;
            res.notes.push_back("\"" + t.text + "\" is not in the Bible - showing results for \"" + sug + "\".");
            for (auto& ph : phrases) for (auto& w : ph) if (w == t.text) w = sug;
            t.text = sug;
            t.ids.push_back(wordId_.at(sug));
            return true;
        };
        for (auto& t : required) {
            if (!resolve(t, true)) {
                res.error = "No verse contains \"" + t.text + (t.prefix ? "*" : "") + "\".";
                return finish(res, t0);
            }
        }
        for (auto& t : excluded) resolve(t, false);

        for (auto& t : required) {
            for (int id : t.ids) res.highlight.insert(vocab_[id]);
            if (!t.prefix) {
                std::string st = t.text + " x" + std::to_string(freq_[t.ids[0]]);
                if (std::find(res.wordStats.begin(), res.wordStats.end(), st) == res.wordStats.end())
                    res.wordStats.push_back(st);
            }
        }

        // ---------- 3. intersect posting lists (rarest word first)
        auto verseSet = [&](const Term& t) {
            std::vector<int> v;
            for (int id : t.ids) for (auto& p : postings_[id]) v.push_back(p.verse);
            if (t.ids.size() > 1) { std::sort(v.begin(), v.end()); v.erase(std::unique(v.begin(), v.end()), v.end()); }
            return v;
        };
        std::vector<size_t> order(required.size());
        std::iota(order.begin(), order.end(), 0);
        auto weight = [&](const Term& t) { size_t s = 0; for (int id : t.ids) s += postings_[id].size(); return s; };
        std::sort(order.begin(), order.end(), [&](size_t a, size_t b) { return weight(required[a]) < weight(required[b]); });

        std::vector<int> cand = verseSet(required[order[0]]);
        for (size_t k = 1; k < order.size() && !cand.empty(); ++k) {
            std::vector<int> other = verseSet(required[order[k]]), out;
            std::set_intersection(cand.begin(), cand.end(), other.begin(), other.end(), std::back_inserter(out));
            cand.swap(out);
        }
        // exclusions & filters
        if (!excluded.empty()) {
            std::vector<int> ex, out;
            for (auto& t : excluded) { auto v = verseSet(t); ex.insert(ex.end(), v.begin(), v.end()); }
            std::sort(ex.begin(), ex.end());
            std::set_difference(cand.begin(), cand.end(), ex.begin(), ex.end(), std::back_inserter(out));
            cand.swap(out);
        }
        if (hasMask && mask.any()) {
            cand.erase(std::remove_if(cand.begin(), cand.end(),
                       [&](int v) { return !mask.test(bible_->verses[v].book); }), cand.end());
        }
        // exact phrases
        if (!phrases.empty()) {
            std::vector<int> out;
            std::vector<std::string> toks;
            for (int v : cand) {
                toks.clear();
                const std::string& text = bible_->verses[v].text;
                forEachWord(text, [&](size_t s, size_t l) {
                    auto w = normWord(std::string_view(text).substr(s, l));
                    if (!w.empty()) toks.push_back(w);
                });
                bool ok = true;
                for (auto& ph : phrases) {
                    bool found = false;
                    for (size_t a = 0; a + ph.size() <= toks.size() && !found; ++a)
                        found = std::equal(ph.begin(), ph.end(), toks.begin() + (long)a);
                    if (!found) { ok = false; break; }
                }
                if (ok) out.push_back(v);
            }
            cand.swap(out);
        }

        // ---------- 4. BM25 ranking
        const double N = bible_->size(), k1 = 1.2, b = 0.75;
        std::vector<std::pair<double, int>> scored;
        scored.reserve(cand.size());
        for (int v : cand) {
            double score = 0;
            double norm = k1 * (1 - b + b * docLen_[v] / avgLen_);
            for (auto& t : required) {
                for (int id : t.ids) {
                    const auto& pl = postings_[id];
                    auto it = std::lower_bound(pl.begin(), pl.end(), v,
                                               [](const Posting& p, int x) { return p.verse < x; });
                    if (it == pl.end() || it->verse != v) continue;
                    double df = (double)pl.size();
                    double idf = std::log(1.0 + (N - df + 0.5) / (df + 0.5));
                    score += idf * (it->tf * (k1 + 1)) / (it->tf + norm);
                }
            }
            scored.push_back({score, v});
        }
        std::stable_sort(scored.begin(), scored.end(), [](auto& x, auto& y) { return x.first > y.first; });
        for (auto& [s, v] : scored) { res.verses.push_back(v); res.scores.push_back(s); }
        if (res.truncatedPrefix) res.notes.push_back("Wildcard matched many words; only the first 400 were used.");
        return finish(res, t0);
    }

    // Filters: in:ot in:nt in:law in:history in:poetry in:prophets in:gospels in:epistles in:<book>
    bool applyFilter(const std::string& f, std::bitset<66>& mask) const {
        auto range = [&](int a, int z) { for (int i = a; i <= z; ++i) mask.set(i); return true; };
        if (f == "ot" || f == "old" || f == "oldtestament")        return range(0, 38);
        if (f == "nt" || f == "new" || f == "newtestament")        return range(39, 65);
        if (f == "law" || f == "torah" || f == "pentateuch")       return range(0, 4);
        if (f == "history")                                         return range(5, 16);
        if (f == "poetry" || f == "wisdom")                         return range(17, 21);
        if (f == "prophets")                                        return range(22, 38);
        if (f == "major")                                           return range(22, 26);
        if (f == "minor")                                           return range(27, 38);
        if (f == "gospels" || f == "gospel")                        return range(39, 42);
        if (f == "epistles" || f == "letters")                      return range(44, 64);
        if (f == "paul" || f == "pauline")                          return range(44, 56);
        int bk = bible_->resolveBook(f);
        if (bk >= 0) { mask.set(bk); return true; }
        return false;
    }

private:
    const Bible* bible_ = nullptr;
    std::vector<std::string> vocab_;
    std::vector<std::vector<Posting>> postings_;
    std::vector<uint32_t> freq_;
    std::unordered_map<std::string, int> wordId_;
    std::vector<uint16_t> docLen_;
    double avgLen_ = 1;
    long long totalWords_ = 0;

    static SearchResult& finish(SearchResult& r, std::chrono::steady_clock::time_point t0) {
        r.millis = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
        return r;
    }
};

} // namespace bible
