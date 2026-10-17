// bible.hpp - loads the text, navigates chapters and understands references
//              like "John 3:16", "Ps 23", "1 Cor 13:4-7" or "Gen 1:1-2:3".
#pragma once
#include "books.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace bible {

// ============================================================ text utils ====
inline std::string toLower(std::string s) {
    for (auto& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}
inline std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace((unsigned char)s[a])) ++a;
    while (b > a && std::isspace((unsigned char)s[b - 1])) --b;
    return s.substr(a, b - a);
}
inline bool startsWith(const std::string& s, const std::string& p) {
    return s.size() >= p.size() && s.compare(0, p.size(), p) == 0;
}
inline bool allDigits(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), [](char c) { return std::isdigit((unsigned char)c); });
}
inline void replaceAll(std::string& s, const std::string& from, const std::string& to) {
    for (size_t p = s.find(from); p != std::string::npos; p = s.find(from, p + to.size()))
        s.replace(p, from.size(), to);
}

// Damerau-Levenshtein (optimal string alignment) edit distance: a swap of two
// neighbouring letters ("fiath" -> "faith") counts as one typo.
// Exits early once the distance exceeds `limit`.
inline int levenshtein(const std::string& a, const std::string& b, int limit = 99) {
    const int n = (int)a.size(), m = (int)b.size();
    if (std::abs(n - m) > limit) return limit + 1;
    std::vector<int> prev2(m + 1), prev(m + 1), cur(m + 1);
    for (int j = 0; j <= m; ++j) prev[j] = j;
    for (int i = 1; i <= n; ++i) {
        cur[0] = i;
        int rowMin = cur[0];
        for (int j = 1; j <= m; ++j) {
            int cost = a[i - 1] == b[j - 1] ? 0 : 1;
            cur[j] = std::min({prev[j] + 1, cur[j - 1] + 1, prev[j - 1] + cost});
            if (i > 1 && j > 1 && a[i - 1] == b[j - 2] && a[i - 2] == b[j - 1])
                cur[j] = std::min(cur[j], prev2[j - 2] + 1);
            rowMin = std::min(rowMin, cur[j]);
        }
        if (rowMin > limit) return limit + 1;
        prev2 = prev;
        std::swap(prev, cur);
    }
    return prev[m];
}

// Turns accented letters (UTF-8) into plain ones: "Génesis" -> "Genesis".
inline std::string foldAccents(const std::string& s) {
    static const char* map[][2] = {
        {"\xC3\xA1", "a"}, {"\xC3\xA9", "e"}, {"\xC3\xAD", "i"}, {"\xC3\xB3", "o"}, {"\xC3\xBA", "u"},
        {"\xC3\x81", "A"}, {"\xC3\x89", "E"}, {"\xC3\x8D", "I"}, {"\xC3\x93", "O"}, {"\xC3\x9A", "U"},
        {"\xC3\xB1", "n"}, {"\xC3\x91", "N"}, {"\xC3\xBC", "u"}, {"\xC3\x9C", "U"}};
    std::string out = s;
    for (auto& m : map) {
        for (size_t p = out.find(m[0]); p != std::string::npos; p = out.find(m[0], p)) out.replace(p, 2, m[1]);
    }
    return out;
}

// ============================================================ the text ======
struct Verse {
    uint8_t  book;      // 0..65
    uint16_t chapter;   // 1-based
    uint16_t verse;     // 1-based
    std::string text;
};

enum class RefKind { None, Book, Chapter, Passage };

struct Ref {
    RefKind kind = RefKind::None;
    int book = -1, chapter = 1;
    int start = -1, end = -1;           // global verse indices (inclusive)
    std::string error;
    std::vector<int> suggestions;       // possible books when ambiguous
};

class Bible {
public:
    std::vector<Verse> verses;

    bool load(const std::string& path, std::string& err) {
        std::ifstream in(path, std::ios::binary);
        if (!in) { err = "cannot open " + path; return false; }
        verses.clear();
        verses.reserve(31102);
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty() || line[0] == '#') continue;
            size_t a = line.find('\t');
            size_t b = a == std::string::npos ? a : line.find('\t', a + 1);
            size_t c = b == std::string::npos ? b : line.find('\t', b + 1);
            if (c == std::string::npos) continue;
            int bk = std::atoi(line.c_str());
            int ch = std::atoi(line.c_str() + a + 1);
            int vs = std::atoi(line.c_str() + b + 1);
            if (bk < 1 || bk > 66 || ch < 1 || vs < 1) continue;
            verses.push_back({(uint8_t)(bk - 1), (uint16_t)ch, (uint16_t)vs, line.substr(c + 1)});
        }
        if (verses.empty()) { err = "no verses found in " + path; return false; }
        buildIndex();
        for (int b = 0; b < 66; ++b)
            if (chapters(b) == 0) { err = std::string("missing book ") + kBooks[b].name; return false; }
        buildAliases();
        return true;
    }

    // ---- navigation -------------------------------------------------------
    int size() const { return (int)verses.size(); }
    int chapters(int b) const { return (int)chStart_[b].size() - 1; }
    int first(int b, int c) const { return chStart_[b][c - 1]; }
    int verseCount(int b, int c) const { return chStart_[b][c] - chStart_[b][c - 1]; }
    int index(int b, int c, int v) const { return first(b, c) + v - 1; }
    int totalChapters() const { int n = 0; for (int b = 0; b < 66; ++b) n += chapters(b); return n; }

    static std::string bookName(int b) { return kBooks[b].name; }

    std::string chapterRef(int b, int c) const {
        if (b == kPsalms) return "Psalm " + std::to_string(c);
        return bookName(b) + " " + std::to_string(c);
    }
    std::string ref(int i) const {
        const Verse& v = verses[i];
        return chapterRef(v.book, v.chapter) + ":" + std::to_string(v.verse);
    }
    std::string rangeRef(int a, int b) const {
        if (a == b) return ref(a);
        const Verse& x = verses[a];
        const Verse& y = verses[b];
        if (x.book != y.book) return ref(a) + " - " + ref(b);
        if (x.verse == 1 && y.verse == verseCount(y.book, y.chapter)) {      // whole chapters
            if (x.chapter == y.chapter) return chapterRef(x.book, x.chapter);
            std::string nm = x.book == kPsalms ? "Psalms" : bookName(x.book);
            return nm + " " + std::to_string(x.chapter) + "-" + std::to_string(y.chapter);
        }
        if (x.chapter == y.chapter) return ref(a) + "-" + std::to_string(y.verse);
        return ref(a) + "-" + std::to_string(y.chapter) + ":" + std::to_string(y.verse);
    }

    // ---- book names -------------------------------------------------------
    static std::string normKey(const std::string& s) {
        std::string k;
        for (char ch : foldAccents(s)) if (std::isalnum((unsigned char)ch)) k += (char)std::tolower((unsigned char)ch);
        return k;
    }
    // "First John", "II Kings", "1st Peter"  ->  "1 John", "2 Kings", "1 Peter"
    static std::string fixOrdinal(const std::string& raw) {
        std::string s = raw;
        for (auto& ch : s) if (ch == '.') ch = ' ';
        s = trim(s);
        size_t sp = s.find(' ');
        if (sp == std::string::npos) return s;
        std::string w = toLower(s.substr(0, sp));
        std::string rest = s.substr(sp);
        if (w == "first" || w == "1st" || w == "i" || w == "primera" || w == "primer")     return "1" + rest;
        if (w == "second" || w == "2nd" || w == "ii" || w == "segunda" || w == "segundo") return "2" + rest;
        if (w == "third" || w == "3rd" || w == "iii" || w == "tercera" || w == "tercer")  return "3" + rest;
        return s;
    }

    // strict = only full names (or long aliases) are accepted, no guessing.
    int resolveBook(const std::string& raw, std::vector<int>* sugg = nullptr, bool strict = false) const {
        std::string key = normKey(fixOrdinal(raw));
        if (key.empty()) return -1;
        for (int b = 0; b < 66; ++b) if (normName_[b] == key) return b;
        auto it = alias_.find(key);
        if (it != alias_.end() && (!strict || key.size() >= 4)) return it->second;
        if (strict) return -1;

        std::vector<int> cand;                                       // unique prefix?
        for (int b = 0; b < 66; ++b) {
            bool hit = startsWith(normName_[b], key);
            for (auto& a : aliasList_[b]) if (!hit && key.size() >= 2 && startsWith(a, key)) hit = true;
            if (hit) cand.push_back(b);
        }
        if (cand.size() == 1) return cand[0];
        if (cand.size() > 1) { if (sugg) *sugg = cand; return -1; }

        if (key.size() >= 4) {                                      // small typo? "genisis"
            int best = -1, bestD = 99, ties = 0;
            int limit = key.size() >= 7 ? 2 : 1;
            for (int b = 0; b < 66; ++b) {
                int d = levenshtein(key, normName_[b], limit);
                if (d < bestD) { bestD = d; best = b; ties = 1; }
                else if (d == bestD) ++ties;
            }
            if (bestD <= limit && ties == 1) return best;
            if (bestD <= limit + 1 && sugg) {
                for (int b = 0; b < 66; ++b)
                    if (levenshtein(key, normName_[b], limit + 1) <= limit + 1) sugg->push_back(b);
            }
        }
        return -1;
    }

    // ---- reference parsing ------------------------------------------------
    // strict: used by the "type anything" box. Words without numbers are only
    // treated as a reference when they are exactly a book name.
    Ref parse(std::string s, bool strict = false) const {
        Ref r;
        replaceAll(s, "\xE2\x80\x93", "-");   // en dash
        replaceAll(s, "\xE2\x80\x94", "-");   // em dash
        s = trim(s);
        if (s.empty()) { r.error = "Empty reference."; return r; }

        auto isNum = [](char c) {
            return std::isdigit((unsigned char)c) || c == ':' || c == '.' || c == '-' || c == ',' || c == ' ';
        };
        size_t i = s.size();
        while (i > 0 && isNum(s[i - 1])) --i;
        std::string bookPart = trim(s.substr(0, i));
        std::string num;
        for (char c : s.substr(i)) if (c != ' ') num += (c == '.' || c == ',') ? ':' : c;
        while (!num.empty() && (num[0] == ':' || num[0] == '-')) num.erase(0, 1);

        if (bookPart.empty()) { r.error = "Please start with a book name, e.g. John 3:16."; return r; }
        bool hasDigits = !num.empty();
        int b = resolveBook(bookPart, &r.suggestions, strict && !hasDigits);
        if (b < 0) {
            r.error = "I don't know the book \"" + bookPart + "\".";
            return r;
        }
        r.book = b;
        const int nCh = chapters(b);

        if (num.empty()) {                       // just a book: open chapter 1
            r.kind = RefKind::Book;
            r.chapter = 1;
            r.start = first(b, 1);
            r.end = first(b, 1) + verseCount(b, 1) - 1;
            return r;
        }

        size_t p = 0;
        auto readInt = [&](int& out) {
            if (p >= num.size() || !std::isdigit((unsigned char)num[p])) return false;
            out = 0;
            while (p < num.size() && std::isdigit((unsigned char)num[p])) {
                out = std::min(out * 10 + (num[p] - '0'), 99999);
                ++p;
            }
            return true;
        };
        int c1 = 0, v1 = 0, c2 = 0, v2 = 0;
        bool colon1 = false, dash = false, colon2 = false;
        if (!readInt(c1)) { r.error = "I couldn't read the chapter number."; return r; }
        if (p < num.size() && num[p] == ':') { ++p; colon1 = true; if (!readInt(v1)) { r.error = "Missing verse after ':'."; return r; } }
        if (p < num.size() && num[p] == '-') {
            ++p; dash = true;
            if (!readInt(c2)) { r.error = "Missing number after '-'."; return r; }
            if (p < num.size() && num[p] == ':') { ++p; colon2 = true; if (!readInt(v2)) { r.error = "Missing verse after ':'."; return r; } }
        }
        if (p != num.size()) { r.error = "I couldn't understand \"" + num + "\"."; return r; }

        // One-chapter books (Obadiah, Philemon, 2-3 John, Jude): "Jude 3" means verse 3.
        if (nCh == 1 && !colon1 && (c1 > 1 || dash)) {
            colon1 = true; v1 = c1; c1 = 1;
            if (dash && !colon2) { colon2 = true; v2 = c2; c2 = 1; }
        }

        int sc, sv, ec, ev;
        if (!colon1 && !dash)       { sc = c1; sv = 1;  ec = c1; ev = -1; }
        else if (!colon1 && !colon2){ sc = c1; sv = 1;  ec = c2; ev = -1; }
        else if (!colon1)           { sc = c1; sv = 1;  ec = c2; ev = v2; }
        else if (!dash)             { sc = c1; sv = v1; ec = c1; ev = v1; }
        else if (!colon2)           { sc = c1; sv = v1; ec = c1; ev = c2; }
        else                        { sc = c1; sv = v1; ec = c2; ev = v2; }

        auto chName = [&](int c) { return chapterRef(b, c); };
        if (sc < 1 || sc > nCh) {
            r.error = bookName(b) + " has " + std::to_string(nCh) + (nCh == 1 ? " chapter." : " chapters.");
            return r;
        }
        if (sv < 1 || sv > verseCount(b, sc)) {
            r.error = chName(sc) + " has " + std::to_string(verseCount(b, sc)) + " verses.";
            return r;
        }
        ec = std::min(std::max(ec, sc), nCh);
        if (ev < 0 || ev > verseCount(b, ec)) ev = verseCount(b, ec);

        r.chapter = sc;
        r.start = index(b, sc, sv);
        r.end = std::max(r.start, index(b, ec, std::max(ev, 1)));
        bool wholeOne = !colon1 && (!dash || c2 == c1);
        r.kind = wholeOne ? RefKind::Chapter : RefKind::Passage;
        return r;
    }

private:
    std::vector<std::vector<int>> chStart_;     // per book: first verse of each chapter + end sentinel
    std::vector<std::string> normName_;
    std::vector<std::vector<std::string>> aliasList_;
    std::unordered_map<std::string, int> alias_;

    void buildIndex() {
        chStart_.assign(66, {});
        for (int i = 0; i < (int)verses.size(); ++i) {
            const Verse& v = verses[i];
            if (i == 0 || verses[i - 1].book != v.book || verses[i - 1].chapter != v.chapter)
                chStart_[v.book].push_back(i);
        }
        int next = (int)verses.size();
        for (int b = 65; b >= 0; --b) {
            if (chStart_[b].empty()) continue;
            int start = chStart_[b][0];
            chStart_[b].push_back(next);
            next = start;
        }
        for (auto& cs : chStart_) if (cs.size() == 1) cs.clear();
    }

    void buildAliases() {
        normName_.clear(); aliasList_.assign(66, {}); alias_.clear();
        for (int b = 0; b < 66; ++b) {
            normName_.push_back(normKey(kBooks[b].name));
            std::string a, all = kBooks[b].aliases;
            all += ' ';
            all += kSpanishNames[b];
            all += ' ';
            for (char c : all) {
                if (c == ' ') { if (!a.empty()) { alias_[a] = b; aliasList_[b].push_back(a); } a.clear(); }
                else a += c;
            }
        }
    }
};

} // namespace bible
