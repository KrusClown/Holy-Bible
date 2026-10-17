// =============================================================================
//   H O L Y   B I B L E  -  King James Version, for the console
//   Read all 66 books, search every verse, find words for what you need.
// =============================================================================
#include "terminal.hpp"
#include "bible.hpp"
#include "search.hpp"
#include "content.hpp"

#include <chrono>
#include <ctime>
#include <fstream>
#include <map>
#include <random>
#include <sstream>
#include <thread>

using namespace bible;
using term::fg;
using term::bg;
using term::bold;
using term::dim;
using term::italic;
using term::spaces;
using term::visibleLen;

namespace col {
constexpr int GOLD = 220, AMBER = 214, ORANGE = 208, CREAM = 230, TEXT = 253, GREY = 245,
              DARK = 239, SKY = 117, CYAN = 81, ROSE = 211, GREEN = 114, VIOLET = 141,
              VERSENUM = 178;
}
static std::string R() { return term::reset(); }

struct QuitSignal {};   // thrown when input ends (Ctrl+D / Ctrl+Z) to exit gracefully

static bool readLine(const std::string& prompt, std::string& out) {
    std::cout << prompt << std::flush;
    if (!std::getline(std::cin, out)) throw QuitSignal{};
    if (!out.empty() && out.back() == '\r') out.pop_back();
    out = trim(out);
    return true;
}
static std::string promptStr(const std::string& label = "") {
    return "  " + fg(col::GOLD) + bold() + "\xE2\x80\xA0 " + R() + fg(col::GREY) + label + R() +
           fg(col::AMBER) + bold() + "\xC2\xBB " + R();          // "† label » "
}

// ---------------------------------------------------------------- helpers ---
static int W() { return std::max(44, std::min(term::cols(), 100)); }
static int H() { return std::max(16, term::rows()); }

static std::vector<std::string> splitCodepoints(const std::string& s) {
    std::vector<std::string> out;
    for (size_t i = 0; i < s.size();) {
        unsigned char c = (unsigned char)s[i];
        size_t len = c < 0x80 ? 1 : (c >> 5) == 6 ? 2 : (c >> 4) == 14 ? 3 : 4;
        out.push_back(s.substr(i, len));
        i += len;
    }
    return out;
}

static std::vector<std::string> wrap(const std::string& t, int width) {
    std::vector<std::string> out;
    std::string line;
    size_t i = 0, n = t.size();
    while (i < n) {
        while (i < n && t[i] == ' ') ++i;
        size_t s = i;
        while (i < n && t[i] != ' ') ++i;
        if (i == s) break;
        std::string word = t.substr(s, i - s);
        if (line.empty()) line = word;
        else if ((int)(line.size() + 1 + word.size()) <= width) line += " " + word;
        else { out.push_back(line); line = word; }
    }
    if (!line.empty() || out.empty()) out.push_back(line);
    return out;
}

using Highlight = std::unordered_set<std::string>;

// Colors one line of Scripture: search hits get a golden marker and words the
// KJV prints in capitals (LORD, GOD, I AM THAT I AM ...) shine in gold.
static std::string colorLine(const std::string& line, const Highlight* hl, const std::string& base) {
    std::string out = base;
    size_t last = 0;
    forEachWord(line, [&](size_t s, size_t l) {
        std::string raw = line.substr(s, l);
        bool hit = hl && !hl->empty() && hl->count(normWord(raw));
        bool caps = false;
        if (l >= 2) {
            int letters = 0;
            caps = true;
            for (char c : raw) {
                if (std::isalpha((unsigned char)c)) { ++letters; if (!std::isupper((unsigned char)c)) caps = false; }
            }
            caps = caps && letters >= 2;
        }
        if (!hit && !caps) return;
        out += line.substr(last, s - last);
        if (hit) out += R() + bold() + fg(16) + bg(col::GOLD) + raw + R() + base;
        else     out += R() + bold() + fg(col::GOLD) + raw + R() + base;
        last = s + l;
    });
    out += line.substr(last) + R();
    return out;
}

static std::string catColor(int book) { return fg(kCategories[kBooks[book].category].color); }

static std::string spacedUpper(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (i) out += ' ';
        out += s[i] == ' ' ? std::string("  ") : std::string(1, (char)std::toupper((unsigned char)s[i]));
    }
    return out;
}

static std::string chip(const std::string& key, int color) {
    return bg(color) + fg(16) + bold() + " " + key + " " + R();
}

// A horizontal rule that glows in the middle.
static std::string glowRule(int w, int color) {
    if (!term::g_color) return "  " + term::rep("\xE2\x94\x80", w - 4);
    int n = w - 4, side = n / 4;
    return "  " + fg(237) + term::rep("\xE2\x94\x80", side) + fg(color) + term::rep("\xE2\x94\x80", n - 2 * side) +
           fg(237) + term::rep("\xE2\x94\x80", side) + R();
}

static std::string title(const std::string& text, int color, int w) {
    return term::center(fg(col::GOLD) + "\xE2\x80\xA0  " + R() + fg(color) + bold() + spacedUpper(text) + R() +
                        fg(col::GOLD) + "  \xE2\x80\xA0" + R(), w);
}

// ============================================================ ASCII ART =====
static const std::map<char, std::array<const char*, 6>> kGlyphs = {
    {'H', {"██╗  ██╗", "██║  ██║", "███████║", "██╔══██║", "██║  ██║", "╚═╝  ╚═╝"}},
    {'O', {" ██████╗ ", "██╔═══██╗", "██║   ██║", "██║   ██║", "╚██████╔╝", " ╚═════╝ "}},
    {'L', {"██╗     ", "██║     ", "██║     ", "██║     ", "███████╗", "╚══════╝"}},
    {'Y', {"██╗   ██╗", "╚██╗ ██╔╝", " ╚████╔╝ ", "  ╚██╔╝  ", "   ██║   ", "   ╚═╝   "}},
    {'B', {"██████╗ ", "██╔══██╗", "██████╔╝", "██╔══██╗", "██████╔╝", "╚═════╝ "}},
    {'I', {"██╗", "██║", "██║", "██║", "██║", "╚═╝"}},
    {'E', {"███████╗", "██╔════╝", "█████╗  ", "██╔══╝  ", "███████╗", "╚══════╝"}},
    {' ', {"   ", "   ", "   ", "   ", "   ", "   "}},
};

static void printBanner(int w) {
    const std::string text = "HOLY BIBLE";
    const int rowColor[6] = {230, 229, 228, 221, 220, 214};
    for (int r = 0; r < 6; ++r) {
        std::string raw;
        for (char ch : text) raw += kGlyphs.at(ch)[r];
        std::string out;
        for (auto& cp : splitCodepoints(raw)) {
            if (cp == "█") out += fg(rowColor[r]) + cp;
            else if (cp == " ") out += cp;
            else out += fg(130) + cp;
        }
        std::cout << term::center(out + R(), w) << "\n";
    }
    std::cout << term::center(fg(col::GREY) + "T H E   K I N G   J A M E S   V E R S I O N" + R(), w) << "\n";
}

static void printCross(int w) {
    static const char* art[] = {
        "         \\     |     /",
        "           \\   |   /",
        "      \xE2\x94\x80 \xE2\x94\x80 \xE2\x94\x80   ████   \xE2\x94\x80 \xE2\x94\x80 \xE2\x94\x80",
        "              ████",
        "      ████████████████████",
        "              ████",
        "              ████",
        "              ████",
        "              ████",
        "        ░▒▓▓████████▓▓▒░",
    };
    int bw = 26;
    int left = std::max(0, (w - bw) / 2);
    for (const char* line : art) {
        std::string out;
        for (auto& cp : splitCodepoints(line)) {
            if (cp == "█") out += fg(col::GOLD) + bold() + cp + R();
            else if (cp == "░" || cp == "▒" || cp == "▓") out += fg(94) + cp;
            else if (cp == " ") out += cp;
            else out += fg(229) + dim() + cp + R();
        }
        std::cout << spaces(left) << out << R() << "\n";
    }
}

// ================================================================= STORE =====
// Remembers where you stopped reading and your bookmarks.
struct Store {
    std::string path;
    int lastBook = 0, lastChapter = 1;
    std::vector<int> marks;

    void load(const Bible& b) {
        const char* home = std::getenv("HOME");
        if (!home) home = std::getenv("USERPROFILE");
        path = std::string(home ? home : ".") + "/.holybible";
        std::ifstream in(path);
        std::string kind;
        int bk, ch, vs;
        while (in >> kind) {
            if (kind == "last" && in >> bk >> ch) {
                if (bk >= 0 && bk < 66 && ch >= 1 && ch <= b.chapters(bk)) { lastBook = bk; lastChapter = ch; }
            } else if (kind == "mark" && in >> bk >> ch >> vs) {
                if (bk >= 0 && bk < 66 && ch >= 1 && ch <= b.chapters(bk) && vs >= 1 && vs <= b.verseCount(bk, ch))
                    marks.push_back(b.index(bk, ch, vs));
            } else {
                std::string skip; std::getline(in, skip);
            }
        }
    }
    void save(const Bible& b) const {
        std::ofstream out(path);
        if (!out) return;
        out << "last " << lastBook << " " << lastChapter << "\n";
        for (int i : marks) {
            const Verse& v = b.verses[i];
            out << "mark " << (int)v.book << " " << v.chapter << " " << v.verse << "\n";
        }
    }
};

// =================================================================== APP =====
class App {
public:
    Bible bible;
    SearchEngine se;
    Store store;
    std::mt19937 rng{(unsigned)std::chrono::steady_clock::now().time_since_epoch().count()};
    std::string flash;     // one-line message shown on the next screen

    // ---------------------------------------------------------------- pager
    using Hints = std::vector<std::pair<std::string, std::string>>;

    // Shows `body` page by page under a fixed header.
    // Returns the user's command; `finished` is true if they pressed Enter on the last page.
    std::string pager(const std::vector<std::string>& head, const std::vector<std::string>& body,
                      const Hints& hints, int startLine, bool& finished) {
        finished = false;
        const int w = W();
        // hint lines (split in two if too wide)
        std::vector<std::string> hintLines(1, "  ");
        for (auto& [k, d] : hints) {
            std::string item = fg(col::AMBER) + bold() + k + R() + " " + fg(col::GREY) + d + R() + "   ";
            if (visibleLen(hintLines.back()) + visibleLen(item) > w - 2) hintLines.push_back("  ");
            hintLines.back() += item;
        }
        int page = H() - (int)head.size() - (int)hintLines.size() - 3;
        page = std::max(page, 6);
        int total = (int)body.size();
        int top = std::max(0, std::min(startLine - 1, total - page));
        if (startLine <= 1) top = 0;
        while (true) {
            term::clear();
            for (auto& h : head) std::cout << h << "\n";
            int end = std::min(total, top + page);
            for (int i = top; i < end; ++i) std::cout << body[i] << "\n";
            for (int i = end - top; i < page && total > page; ++i) std::cout << "\n";
            int pages = std::max(1, (total + page - 1) / page);
            int cur = std::min(pages, top / page + 1 + (top % page ? 1 : 0));
            if (end >= total) cur = pages;
            std::string pg = " " + std::to_string(cur) + "/" + std::to_string(pages) + " ";
            std::cout << "  " << fg(237) << term::rep("\xE2\x94\x80", w - 6 - (int)pg.size()) << R()
                      << fg(col::GREY) << pg << R() << fg(237) << "\xE2\x94\x80\xE2\x94\x80" << R() << "\n";
            for (auto& hl : hintLines) std::cout << hl << "\n";
            std::string in;
            readLine(promptStr(), in);
            if (in.empty()) {
                if (end >= total) { finished = true; return ""; }
                top = end;
                continue;
            }
            if (in == "-" || in == "<") { top = std::max(0, top - page); continue; }
            if (in == ">" || in == "+") { if (end < total) top = end; continue; }
            return in;
        }
    }

    // ---------------------------------------------------------- verse lines
    // Renders verses [a,b] as numbered, wrapped, colored lines.
    void addVerses(std::vector<std::string>& out, int a, int b, const Highlight* hl, int focus,
                   int indent, bool chapterHeads) {
        const int w = W();
        const int tw = std::max(20, w - indent - 6);
        int prevCh = -1;
        for (int i = a; i <= b; ++i) {
            const Verse& v = bible.verses[i];
            if (chapterHeads && prevCh != -1 && v.chapter != prevCh) {
                out.push_back("");
                out.push_back(spaces(indent) + catColor(v.book) + bold() + "\xE2\x94\x80\xE2\x94\x80 " +
                              bible.chapterRef(v.book, v.chapter) + " \xE2\x94\x80\xE2\x94\x80" + R());
            }
            prevCh = v.chapter;
            bool isF = (i == focus);
            std::string num = std::to_string(v.verse);
            num = spaces(3 - (int)num.size()) + num;
            std::string numCol = isF ? bg(col::GOLD) + fg(16) + bold() : fg(col::VERSENUM);
            std::string base = isF ? fg(col::CREAM) + bold() : fg(col::TEXT);
            auto lines = wrap(v.text, tw);
            for (size_t j = 0; j < lines.size(); ++j) {
                std::string pre = j == 0 ? numCol + num + R() + " " : spaces(4);
                out.push_back(spaces(indent) + pre + colorLine(lines[j], hl, base));
            }
        }
    }

    // ---------------------------------------------------------------- reader
    std::vector<std::string> chapterHeader(int b, int c) {
        const int w = W();
        std::vector<std::string> h;
        h.push_back("");
        std::string name = b == kPsalms ? "Psalms" : Bible::bookName(b);
        h.push_back(title(name, kCategories[kBooks[b].category].color, w));
        std::string sub = (b == kPsalms ? "Psalm " : "Chapter ") + std::to_string(c) + " of " +
                          std::to_string(bible.chapters(b)) + "  \xC2\xB7  " +
                          std::to_string(bible.verseCount(b, c)) + " verses  \xC2\xB7  " +
                          kCategories[kBooks[b].category].name;
        h.push_back(term::center(fg(col::GREY) + sub + R(), w));
        h.push_back(glowRule(w, kCategories[kBooks[b].category].color));
        if (!flash.empty()) { h.push_back("  " + fg(col::GREEN) + "\xE2\x99\xA6 " + flash + R()); flash.clear(); }
        return h;
    }

    void reader(int b, int c, int focusIdx = -1, const Highlight* hl = nullptr) {
        while (true) {
            store.lastBook = b; store.lastChapter = c;
            store.save(bible);
            const int first = bible.first(b, c), last = first + bible.verseCount(b, c) - 1;
            std::vector<std::string> body;
            addVerses(body, first, last, hl, focusIdx, 1, false);
            int focusLine = 0;
            if (focusIdx >= first && focusIdx <= last) {
                for (int i = first, line = 0; i <= last; ++i) {
                    if (i == focusIdx) { focusLine = line; break; }
                    line += (int)wrap(bible.verses[i].text, std::max(20, W() - 7)).size();
                }
            }
            body.push_back("");
            bool endOfBible = (b == 65 && c == bible.chapters(b));
            body.push_back(term::center(fg(col::DARK) + (endOfBible ? "\xE2\x80\xA0  Amen  \xE2\x80\xA0" : "\xC2\xB7  \xC2\xB7  \xC2\xB7") + R(), W()));

            bool finished;
            std::string cmd = pager(chapterHeader(b, c), body,
                {{"Enter", "more"}, {"n/p", "next/prev chapter"}, {"12", "go to chapter"},
                 {"b 16", "bookmark verse"}, {"s word", "search"}, {"q", "back"}},
                focusLine, finished);
            std::string low = toLower(cmd);
            hl = nullptr;
            focusIdx = -1;
            if (finished || low == "n") {
                if (c < bible.chapters(b)) ++c;
                else if (b < 65) { ++b; c = 1; }
                else flash = "You have reached the end of the Holy Bible. Amen.";
                continue;
            }
            if (low == "p") {
                if (c > 1) --c;
                else if (b > 0) { --b; c = bible.chapters(b); }
                continue;
            }
            if (low == "q" || low == "0" || low == "m") return;
            if (allDigits(low)) {
                int n = std::atoi(low.c_str());
                if (n >= 1 && n <= bible.chapters(b)) c = n;
                else flash = Bible::bookName(b) + " has " + std::to_string(bible.chapters(b)) + " chapters.";
                continue;
            }
            if ((low[0] == 'b' || low[0] == 'v') && low.size() > 1 && low[1] == ' ' && allDigits(trim(low.substr(2)))) {
                int v = std::atoi(trim(low.substr(2)).c_str());
                if (v < 1 || v > bible.verseCount(b, c)) {
                    flash = bible.chapterRef(b, c) + " has " + std::to_string(bible.verseCount(b, c)) + " verses.";
                    continue;
                }
                focusIdx = bible.index(b, c, v);
                if (low[0] == 'b') addBookmark(focusIdx);
                continue;
            }
            if (low == "s" || startsWith(low, "s ")) { searchScreen(trim(cmd.substr(1))); continue; }
            Ref r = bible.parse(cmd);
            if (r.kind != RefKind::None) {
                b = r.book; c = r.chapter;
                focusIdx = r.kind == RefKind::Passage ? r.start : -1;
                continue;
            }
            flash = "Type n, p, a chapter number, a reference or q.";
        }
    }

    void addBookmark(int idx) {
        if (std::find(store.marks.begin(), store.marks.end(), idx) == store.marks.end()) {
            store.marks.push_back(idx);
            store.save(bible);
            flash = "Bookmarked " + bible.ref(idx) + ".";
        } else {
            flash = bible.ref(idx) + " is already in your bookmarks.";
        }
    }

    // ------------------------------------------------------------- passages
    // Shows one or more passages (topics, famous passages, references).
    void passageScreen(const std::string& heading, const std::string& sub,
                       const std::vector<std::pair<int, int>>& ranges, const Highlight* hl = nullptr) {
        const int w = W();
        std::vector<std::string> head = {"", title(heading, col::GOLD, w)};
        if (!sub.empty()) head.push_back(term::center(fg(col::GREY) + italic() + sub + R(), w));
        head.push_back(glowRule(w, col::AMBER));
        std::vector<std::string> body;
        for (size_t k = 0; k < ranges.size(); ++k) {
            auto [a, z] = ranges[k];
            int bk = bible.verses[a].book;
            std::string label = ranges.size() > 1 ? chip(std::to_string(k + 1), kCategories[kBooks[bk].category].color) + " " : "";
            body.push_back("  " + label + catColor(bk) + bold() + bible.rangeRef(a, z) + R());
            addVerses(body, a, z, hl, -1, 2, true);
            body.push_back("");
        }
        bool single = ranges.size() == 1;
        Hints hints = single ? Hints{{"Enter", "more/back"}, {"r", "read in context"}, {"b", "bookmark"}, {"q", "back"}}
                             : Hints{{"Enter", "more/back"}, {"1-" + std::to_string(ranges.size()), "read in context"},
                                     {"b 2", "bookmark"}, {"q", "back"}};
        while (true) {
            bool finished;
            if (!flash.empty()) { head.push_back("  " + fg(col::GREEN) + "\xE2\x99\xA6 " + flash + R()); flash.clear(); }
            std::string cmd = toLower(pager(head, body, hints, 0, finished));
            if (head.back().find("\xE2\x99\xA6") != std::string::npos) head.pop_back();
            if (finished || cmd == "q" || cmd == "0") return;
            int n = 0;
            if (cmd == "r" && single) n = 1;
            else if (allDigits(cmd)) n = std::atoi(cmd.c_str());
            if (n >= 1 && n <= (int)ranges.size()) {
                const Verse& v = bible.verses[ranges[n - 1].first];
                reader(v.book, v.chapter, ranges[n - 1].first, hl);
                continue;
            }
            if (cmd == "b" && single) { addBookmark(ranges[0].first); continue; }
            if (startsWith(cmd, "b ") && allDigits(trim(cmd.substr(2)))) {
                int k = std::atoi(trim(cmd.substr(2)).c_str());
                if (k >= 1 && k <= (int)ranges.size()) addBookmark(ranges[k - 1].first);
                continue;
            }
            return;
        }
    }

    std::vector<std::pair<int, int>> rangesOf(const std::vector<const char*>& refs) {
        std::vector<std::pair<int, int>> out;
        for (auto* s : refs) {
            Ref r = bible.parse(s);
            if (r.kind != RefKind::None) out.push_back({r.start, r.end});
        }
        return out;
    }

    void showRef(const Ref& r) {
        if (r.kind == RefKind::Book || r.kind == RefKind::Chapter) { reader(r.book, r.chapter); return; }
        if (r.kind == RefKind::Passage) passageScreen(bible.rangeRef(r.start, r.end), "", {{r.start, r.end}});
    }

    // ---------------------------------------------------------------- cards
    void verseCard(const std::string& kicker, int a, int z) {
        const int w = W();
        const int bw = std::min(w - 4, 74);
        const int inner = bw - 2;
        const int tw = inner - 8;
        std::string text;
        for (int i = a; i <= z; ++i) text += (text.empty() ? "" : " ") + bible.verses[i].text;
        auto lines = wrap(text, tw);
        int left = std::max(2, (w - bw) / 2);
        std::string L = spaces(left);
        const int bc = col::AMBER;
        std::string k = " " + kicker + " ";
        int kl = visibleLen(k);
        int side = (inner - kl) / 2;
        std::cout << L << fg(bc) << "╭" << term::rep("─", side) << R() << fg(col::GOLD) << bold() << k << R()
                  << fg(bc) << term::rep("─", inner - side - kl) << "╮" << R() << "\n";
        auto row = [&](const std::string& content) {
            std::cout << L << fg(bc) << "│" << R() << content << spaces(inner - visibleLen(content)) << fg(bc) << "│" << R() << "\n";
        };
        row("");
        for (size_t i = 0; i < lines.size(); ++i) {
            std::string q1 = i == 0 ? fg(col::GOLD) + bold() + "\xE2\x80\x9C" + R() : " ";
            row("   " + q1 + " " + colorLine(lines[i], nullptr, fg(col::CREAM) + italic()) +
                (i + 1 == lines.size() ? fg(col::GOLD) + bold() + "\xE2\x80\x9D" + R() : ""));
        }
        row("");
        std::string ref = "\xE2\x80\x94 " + bible.rangeRef(a, z);
        row(spaces(inner - visibleLen(ref) - 4) + catColor(bible.verses[a].book) + bold() + ref + R());
        row("");
        std::cout << L << fg(bc) << "╰" << term::rep("─", inner) << "╯" << R() << "\n";
    }

    void verseOfTheDay() {
        std::time_t t = std::time(nullptr);
        std::tm lt = *std::localtime(&t);
        char date[64];
        std::strftime(date, sizeof date, "%A, %B %d, %Y", &lt);
        long days = (long)(t / 86400);
        Ref r = bible.parse(kVerseOfTheDay[(size_t)(days % (long)kVerseOfTheDay.size())]);
        cardLoop("V E R S E   O F   T H E   D A Y", date, [&] { return std::make_pair(r.start, r.end); }, false);
    }

    void randomVerse() {
        static std::vector<std::pair<int, int>> pool;
        if (pool.empty()) {
            for (auto& c : kTopics) for (auto& p : rangesOf(c.refs)) pool.push_back(p);
            for (auto& p : rangesOf(kVerseOfTheDay)) pool.push_back(p);
        }
        cardLoop("A   W O R D   F O R   Y O U", "Enter for another verse",
                 [&] { return pool[std::uniform_int_distribution<size_t>(0, pool.size() - 1)(rng)]; }, true);
    }

    template <class Pick>
    void cardLoop(const std::string& heading, const std::string& sub, Pick pick, bool again) {
        auto cur = pick();
        while (true) {
            term::clear();
            const int w = W();
            std::cout << "\n";
            if (H() >= 34) { printCross(w); std::cout << "\n"; }
            std::cout << term::center(fg(col::GOLD) + bold() + heading + R(), w) << "\n";
            std::cout << term::center(fg(col::GREY) + italic() + sub + R(), w) << "\n\n";
            verseCard(bible.rangeRef(cur.first, cur.second), cur.first, cur.second);
            std::cout << "\n";
            if (!flash.empty()) { std::cout << "  " << fg(col::GREEN) << "\xE2\x99\xA6 " << flash << R() << "\n"; flash.clear(); }
            std::cout << "  ";
            if (again) std::cout << fg(col::AMBER) << bold() << "Enter" << R() << fg(col::GREY) << " another   " << R();
            std::cout << fg(col::AMBER) << bold() << "r" << R()
                      << fg(col::GREY) << " read in context   " << R() << fg(col::AMBER) << bold() << "b" << R()
                      << fg(col::GREY) << " bookmark   " << R() << fg(col::AMBER) << bold() << "q" << R()
                      << fg(col::GREY) << " back" << R() << "\n";
            std::string in;
            readLine(promptStr(), in);
            in = toLower(in);
            if (in.empty()) { if (again) { cur = pick(); continue; } return; }
            if (in == "r") { const Verse& v = bible.verses[cur.first]; reader(v.book, v.chapter, cur.first); continue; }
            if (in == "b") { addBookmark(cur.first); continue; }
            return;
        }
    }

    // ---------------------------------------------------------- book picker
    void bookPicker() {
        while (true) {
            term::clear();
            const int w = W();
            std::cout << "\n" << title("The Books of the Bible", col::GOLD, w) << "\n" << glowRule(w, col::AMBER) << "\n";
            int cell = 19, cols = std::max(1, (w - 2) / cell);
            auto grid = [&](int from, int to) {
                int n = to - from + 1, rows = (n + cols - 1) / cols;
                for (int r = 0; r < rows; ++r) {
                    std::string line = "  ";
                    for (int c = 0; c < cols; ++c) {
                        int i = from + c * rows + r;
                        if (i > to) break;
                        std::string num = std::to_string(i + 1);
                        line += term::padRight(fg(col::DARK) + spaces(2 - (int)num.size()) + num + R() + " " +
                                               catColor(i) + kBooks[i].name + R(), cell);
                    }
                    std::cout << line << "\n";
                }
            };
            std::cout << "  " << fg(col::AMBER) << bold() << "OLD TESTAMENT" << R() << fg(col::DARK) << "  39 books" << R() << "\n";
            grid(0, 38);
            std::cout << "\n  " << fg(col::SKY) << bold() << "NEW TESTAMENT" << R() << fg(col::DARK) << "  27 books" << R() << "\n";
            grid(39, 65);
            std::cout << "\n  ";
            int used = 2;
            for (auto& c : kCategories) {
                std::string item = fg(c.color) + "\xE2\x96\xA0 " + c.name + R() + "  ";
                if (used + visibleLen(item) > w) { std::cout << "\n  "; used = 2; }
                std::cout << item;
                used += visibleLen(item);
            }
            std::cout << "\n\n";
            if (!flash.empty()) { std::cout << "  " << fg(col::ROSE) << flash << R() << "\n"; flash.clear(); }
            std::string in;
            readLine(promptStr("Book number or name (q back) "), in);
            std::string low = toLower(in);
            if (low.empty() || low == "q" || low == "0") return;
            if (allDigits(low)) {
                int n = std::atoi(low.c_str());
                if (n >= 1 && n <= 66) chapterPicker(n - 1);
                else flash = "Choose a number from 1 to 66.";
                continue;
            }
            Ref r = bible.parse(in);
            if (r.kind == RefKind::Book) chapterPicker(r.book);
            else if (r.kind != RefKind::None) showRef(r);
            else flash = r.error + suggestionText(r);
        }
    }

    void chapterPicker(int b) {
        if (bible.chapters(b) == 1) { reader(b, 1); return; }
        while (true) {
            term::clear();
            const int w = W();
            int color = kCategories[kBooks[b].category].color;
            int n = bible.chapters(b);
            std::cout << "\n" << title(b == kPsalms ? "Psalms" : Bible::bookName(b), color, w) << "\n";
            std::cout << term::center(fg(col::GREY) + std::to_string(n) + " " + (b == kPsalms ? "psalms" : "chapters") + "  \xC2\xB7  " +
                                      kCategories[kBooks[b].category].name + R(), w) << "\n";
            std::cout << glowRule(w, color) << "\n\n";
            int per = std::max(5, std::min(15, (w - 4) / 5));
            for (int c = 1; c <= n; ++c) {
                if ((c - 1) % per == 0) std::cout << "  ";
                std::string s = std::to_string(c);
                bool ten = c % 10 == 0;
                std::cout << spaces(4 - (int)s.size()) << (ten ? fg(col::GOLD) + bold() : fg(color)) << s << R() << " ";
                if (c % per == 0 || c == n) std::cout << "\n";
            }
            std::cout << "\n";
            if (!flash.empty()) { std::cout << "  " << fg(col::ROSE) << flash << R() << "\n"; flash.clear(); }
            std::string in;
            readLine(promptStr("Chapter (q back) "), in);
            std::string low = toLower(in);
            if (low.empty() || low == "q" || low == "0") return;
            if (allDigits(low)) {
                int c = std::atoi(low.c_str());
                if (c >= 1 && c <= n) { reader(b, c); return; }
                flash = "Choose a chapter from 1 to " + std::to_string(n) + ".";
                continue;
            }
            Ref r = bible.parse(Bible::bookName(b) + " " + in);
            if (r.kind != RefKind::None) { showRef(r); return; }
            flash = r.error;
        }
    }

    std::string suggestionText(const Ref& r) {
        if (r.suggestions.empty()) return "";
        std::string s = " Did you mean ";
        for (size_t i = 0; i < r.suggestions.size() && i < 6; ++i)
            s += (i ? ", " : "") + Bible::bookName(r.suggestions[i]);
        return s + "?";
    }

    // --------------------------------------------------------------- search
    void searchScreen(std::string q) {
        bool bibleOrder = false;
        while (true) {
            const int w = W();
            if (q.empty()) {
                term::clear();
                std::cout << "\n" << title("Search the Scriptures", col::CYAN, w) << "\n" << glowRule(w, col::CYAN) << "\n\n";
                auto ex = [&](const std::string& e, const std::string& d) {
                    std::cout << "    " << fg(col::GOLD) << term::padRight(e, 30) << R() << fg(col::GREY) << d << R() << "\n";
                };
                ex("love one another", "verses with all these words");
                ex("\"the lord is my shepherd\"", "an exact phrase");
                ex("bless*", "bless, blessed, blessing, blesseth...");
                ex("faith -works", "faith, but not works");
                ex("peace in:psalms", "only in Psalms  (in:nt in:ot in:gospels in:john)");
                ex("fiath", "typos are corrected for you");
                std::cout << "\n";
                if (!flash.empty()) { std::cout << "  " << fg(col::ROSE) << flash << R() << "\n\n"; flash.clear(); }
                readLine(promptStr("Search (q back) "), q);
                if (q.empty() || toLower(q) == "q") return;
            }
            SearchResult res = se.search(q);
            if (!res.error.empty()) { flash = res.error; q.clear(); continue; }

            std::vector<int> order = res.verses;
            if (bibleOrder) std::sort(order.begin(), order.end());
            std::vector<std::string> head = {"", title("Search", col::CYAN, w)};
            head.push_back("  " + fg(col::GREY) + "for " + R() + fg(col::CREAM) + bold() + q + R());
            char ms[32];
            std::snprintf(ms, sizeof ms, "%.2f ms", res.millis);
            std::string stats = "  " + chip(std::to_string(res.verses.size()) + (res.verses.size() == 1 ? " verse" : " verses"), col::CYAN) +
                                fg(col::GREY) + "  found in " + ms + "  \xC2\xB7  " + (bibleOrder ? "Bible order" : "most relevant first") + R();
            if (!res.wordStats.empty()) {
                stats += fg(col::DARK) + "  \xC2\xB7  ";
                for (size_t i = 0; i < res.wordStats.size() && i < 3; ++i) stats += (i ? ", " : "") + res.wordStats[i];
                stats += R();
            }
            head.push_back(stats);
            for (auto& n : res.notes) head.push_back("  " + fg(col::AMBER) + "\xE2\x99\xA6 " + n + R());
            head.push_back(glowRule(w, col::CYAN));

            std::vector<std::string> body;
            const int limit = 500;
            const int tw = std::max(20, w - 10);
            for (size_t k = 0; k < order.size() && (int)k < limit; ++k) {
                int vi = order[k];
                std::string num = std::to_string(k + 1);
                body.push_back("  " + fg(col::DARK) + spaces(4 - (int)num.size()) + num + R() + "  " +
                               catColor(bible.verses[vi].book) + bold() + bible.ref(vi) + R());
                for (auto& line : wrap(bible.verses[vi].text, tw))
                    body.push_back(spaces(8) + colorLine(line, &res.highlight, fg(col::TEXT)));
            }
            if (order.empty()) body.push_back("  " + fg(col::GREY) + "No verse matches all of that. Try fewer words, or a wildcard like love*." + R());
            if ((int)order.size() > limit)
                body.push_back("  " + fg(col::GREY) + "... showing the first " + std::to_string(limit) + ". Add words to narrow it down." + R());

            bool finished;
            std::string cmd = pager(head, body, {{"Enter", "more"}, {"7", "open result 7"}, {"o", "order"},
                                                 {"words", "new search"}, {"q", "back"}}, 0, finished);
            std::string low = toLower(cmd);
            if (finished || low == "q" || low == "0") return;
            if (low == "o") { bibleOrder = !bibleOrder; continue; }
            if (allDigits(low)) {
                size_t n = (size_t)std::atoi(low.c_str());
                if (n >= 1 && n <= order.size() && (int)n <= limit) {
                    const Verse& v = bible.verses[order[n - 1]];
                    reader(v.book, v.chapter, order[n - 1], &res.highlight);
                }
                continue;
            }
            q = cmd;
        }
    }

    // ---------------------------------------------------------- collections
    void collectionMenu(const std::string& heading, const std::string& sub, const std::vector<Collection>& items, int color) {
        while (true) {
            term::clear();
            const int w = W();
            std::cout << "\n" << title(heading, color, w) << "\n";
            std::cout << term::center(fg(col::GREY) + italic() + sub + R(), w) << "\n" << glowRule(w, color) << "\n\n";
            const int palette[] = {220, 214, 208, 203, 211, 141, 117, 81, 114, 79};
            int cols = w >= 84 ? 2 : 1;
            int rows = ((int)items.size() + cols - 1) / cols;
            int cw = (w - 4) / cols;
            for (int r = 0; r < rows; ++r) {
                std::string line = "  ";
                for (int c = 0; c < cols; ++c) {
                    int i = c * rows + r;
                    if (i >= (int)items.size()) break;
                    std::string num = std::to_string(i + 1);
                    std::string cell = fg(palette[i % 10]) + bold() + spaces(2 - (int)num.size()) + num + R() + "  " +
                                       fg(col::TEXT) + items[i].title + R() + fg(col::DARK) + "  " + items[i].subtitle + R();
                    if (visibleLen(cell) > cw - 3) cell = fg(palette[i % 10]) + bold() + spaces(2 - (int)num.size()) + num + R() + "  " + fg(col::TEXT) + items[i].title + R();
                    line += term::padRight(cell, cw);
                }
                std::cout << line << "\n";
            }
            std::cout << "\n";
            std::string in;
            readLine(promptStr("Number (q back) "), in);
            std::string low = toLower(in);
            if (low.empty() || low == "q" || low == "0") return;
            int n = allDigits(low) ? std::atoi(low.c_str()) : 0;
            if (n >= 1 && n <= (int)items.size())
                passageScreen(items[n - 1].title, items[n - 1].subtitle, rangesOf(items[n - 1].refs));
        }
    }

    // ------------------------------------------------------- daily reading
    void dailyReading() {
        std::time_t t = std::time(nullptr);
        std::tm lt = *std::localtime(&t);
        int year = lt.tm_year + 1900;
        bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
        int days = leap ? 366 : 365, day = lt.tm_yday;
        int total = bible.totalChapters();
        int s = day * total / days, e = (day + 1) * total / days - 1;
        std::vector<std::pair<int, int>> chs;
        for (int b = 0, k = 0; b < 66; ++b)
            for (int c = 1; c <= bible.chapters(b); ++c, ++k)
                if (k >= s && k <= e) chs.push_back({b, c});
        while (true) {
            term::clear();
            const int w = W();
            std::cout << "\n" << title("Daily Reading", col::GREEN, w) << "\n";
            std::cout << term::center(fg(col::GREY) + italic() + "The whole Bible in one year" + R(), w) << "\n" << glowRule(w, col::GREEN) << "\n\n";
            int barW = std::min(50, w - 20);
            int filled = (day + 1) * barW / days;
            const int grad[] = {22, 28, 34, 40, 76, 112, 148, 184, 220, 226};
            std::string bar;
            for (int i = 0; i < barW; ++i)
                bar += i < filled ? fg(grad[i * 10 / barW]) + "█" : fg(236) + "░";
            std::cout << "  " << fg(col::GREY) << "Day " << R() << fg(col::CREAM) << bold() << day + 1 << R() << fg(col::GREY)
                      << " of " << days << R() << "\n  " << bar << R() << " " << fg(col::GREEN) << bold()
                      << (day + 1) * 100 / days << "%" << R() << "\n\n";
            std::cout << "  " << fg(col::GOLD) << bold() << "Today's chapters" << R() << "\n";
            for (size_t i = 0; i < chs.size(); ++i)
                std::cout << "    " << chip(std::to_string(i + 1), col::GREEN) << " " << catColor(chs[i].first)
                          << bible.chapterRef(chs[i].first, chs[i].second) << R() << fg(col::DARK) << "  "
                          << bible.verseCount(chs[i].first, chs[i].second) << " verses" << R() << "\n";
            std::cout << "\n  " << fg(col::GREY) << "Press " << R() << fg(col::AMBER) << bold() << "Enter" << R() << fg(col::GREY)
                      << " to begin (the reader continues chapter after chapter), a number to pick one, or q." << R() << "\n";
            std::string in;
            readLine(promptStr(), in);
            std::string low = toLower(in);
            if (low == "q" || low == "0") return;
            int n = low.empty() ? 1 : (allDigits(low) ? std::atoi(low.c_str()) : 0);
            if (n >= 1 && n <= (int)chs.size()) { reader(chs[n - 1].first, chs[n - 1].second); return; }
        }
    }

    // ------------------------------------------------------------ bookmarks
    void bookmarks() {
        while (true) {
            term::clear();
            const int w = W();
            std::cout << "\n" << title("Bookmarks", col::ROSE, w) << "\n" << glowRule(w, col::ROSE) << "\n\n";
            if (store.marks.empty()) {
                std::cout << "  " << fg(col::GREY) << "No bookmarks yet. While reading, type " << R() << fg(col::AMBER) << bold()
                          << "b 16" << R() << fg(col::GREY) << " to keep verse 16." << R() << "\n\n";
            }
            for (size_t i = 0; i < store.marks.size(); ++i) {
                int vi = store.marks[i];
                std::string ref = bible.ref(vi);
                std::string text = bible.verses[vi].text;
                int room = w - 16 - (int)ref.size();
                if ((int)text.size() > room) text = text.substr(0, (size_t)std::max(0, room - 3)) + "...";
                std::string num = std::to_string(i + 1);
                std::cout << "  " << fg(col::ROSE) << bold() << spaces(3 - (int)num.size()) << num << R() << "  "
                          << catColor(bible.verses[vi].book) << bold() << ref << R() << "  " << fg(col::GREY) << text << R() << "\n";
            }
            std::cout << "\n  " << fg(col::AMBER) << bold() << "3" << R() << fg(col::GREY) << " open   " << R()
                      << fg(col::AMBER) << bold() << "d 3" << R() << fg(col::GREY) << " delete   " << R()
                      << fg(col::AMBER) << bold() << "q" << R() << fg(col::GREY) << " back" << R() << "\n";
            std::string in;
            readLine(promptStr(), in);
            std::string low = toLower(in);
            if (low.empty() || low == "q" || low == "0") return;
            if (allDigits(low)) {
                size_t n = (size_t)std::atoi(low.c_str());
                if (n >= 1 && n <= store.marks.size()) {
                    const Verse& v = bible.verses[store.marks[n - 1]];
                    reader(v.book, v.chapter, store.marks[n - 1]);
                }
            } else if (startsWith(low, "d ") && allDigits(trim(low.substr(2)))) {
                size_t n = (size_t)std::atoi(trim(low.substr(2)).c_str());
                if (n >= 1 && n <= store.marks.size()) { store.marks.erase(store.marks.begin() + (long)(n - 1)); store.save(bible); }
            }
        }
    }

    // ----------------------------------------------------------------- help
    void help() {
        const int w = W();
        std::vector<std::string> head = {"", title("How to use", col::VIOLET, w), glowRule(w, col::VIOLET)};
        std::vector<std::string> b;
        auto sec = [&](const std::string& s) { b.push_back(""); b.push_back("  " + fg(col::GOLD) + bold() + s + R()); };
        auto row = [&](const std::string& k, const std::string& d) {
            b.push_back("    " + fg(col::AMBER) + term::padRight(k, 28) + R() + fg(col::TEXT) + d + R());
        };
        sec("Type anything on the main menu");
        row("John 3:16", "a verse");
        row("Ps 23    psalm 91", "a whole psalm or chapter");
        row("1 Cor 13:4-7", "a range of verses");
        row("Gen 1:1-2:3", "across chapters");
        row("Romans", "a book from chapter 1");
        row("love your enemies", "anything else is searched");
        sec("Search");
        row("\"in the beginning\"", "exact phrase");
        row("bless*", "every word starting with bless");
        row("mercy -judgment", "exclude a word");
        row("in:psalms  in:nt  in:ot", "filter by book or testament");
        row("in:gospels in:law in:prophets", "filter by group");
        row("o", "switch relevance / Bible order");
        sec("Reading");
        row("Enter   -", "next page / previous page");
        row("n   p", "next / previous chapter");
        row("12", "jump to chapter 12");
        row("v 16   b 16", "highlight / bookmark verse 16");
        row("s grace", "search from the reader");
        sec("About this Bible");
        char buf[160];
        std::snprintf(buf, sizeof buf, "%d books, %d chapters, %d verses, %lld words.", 66, bible.totalChapters(), bible.size(), se.totalWords());
        row("Text", "King James Version (1769), public domain.");
        row("Size", buf);
        std::snprintf(buf, sizeof buf, "%zu distinct words indexed in %.0f ms (BM25 ranking).", se.vocabularySize(), se.buildMillis);
        row("Search index", buf);
        row("Saved in", store.path);
        bool finished;
        pager(head, b, {{"Enter", "more/back"}, {"q", "back"}}, 0, finished);
    }

    // ----------------------------------------------------------- main menu
    void splash() {
        term::clear();
        const int w = W();
        std::cout << "\n";
        if (H() >= 26) { printCross(w); std::cout << "\n"; }
        if (w >= 78) printBanner(w);
        else std::cout << title("Holy Bible", col::GOLD, w) << "\n";
        std::cout << "\n";
        Ref r = bible.parse("Psalm 119:105");
        std::string verse = "\xE2\x80\x9C" + bible.verses[r.start].text + "\xE2\x80\x9D";
        std::string line = fg(col::CREAM) + italic();
        std::cout << spaces(std::max(0, (w - visibleLen(verse)) / 2)) << line;
        for (auto& cp : splitCodepoints(verse)) {
            std::cout << cp << std::flush;
            if (term::g_tty) std::this_thread::sleep_for(std::chrono::milliseconds(14));
        }
        std::cout << R() << "\n" << term::center(fg(col::GOLD) + "\xE2\x80\x94 Psalm 119:105" + R(), w) << "\n\n";
        char buf[128];
        std::snprintf(buf, sizeof buf, "66 books  \xC2\xB7  %d chapters  \xC2\xB7  %d verses  \xC2\xB7  indexed in %.0f ms",
                      bible.totalChapters(), bible.size(), se.buildMillis);
        std::cout << term::center(fg(col::DARK) + buf + R(), w) << "\n\n";
        std::string in;
        readLine(term::center(fg(col::GREY) + "Press " + R() + fg(col::AMBER) + bold() + "Enter" + R() + fg(col::GREY) + " to open the Word " + R(), w), in);
    }

    void mainMenu() {
        while (true) {
            term::clear();
            const int w = W();
            std::cout << "\n";
            if (w >= 78 && H() >= 28) printBanner(w);
            else std::cout << title("Holy Bible", col::GOLD, w) << "\n" << term::center(fg(col::GREY) + "King James Version" + R(), w) << "\n";
            std::cout << "\n";

            struct Item { std::string key, label, extra; int color; };
            std::string cont = bible.chapterRef(store.lastBook, store.lastChapter);
            std::vector<Item> left = {
                {"1", "Read the Bible", "", 220}, {"2", "Continue reading", cont, 214}, {"3", "Go to a reference", "", 208},
                {"4", "Search the Scriptures", "", 81}, {"5", "Verses for what I need", "", 211}, {"6", "Famous passages", "", 141}};
            std::vector<Item> right = {
                {"7", "Verse of the day", "", 229}, {"8", "Random verse", "", 117}, {"9", "Random Psalm", "", 183},
                {"D", "Daily reading plan", "", 114}, {"B", "Bookmarks", store.marks.empty() ? "" : std::to_string(store.marks.size()), 211},
                {"H", "Help", "", 250}};
            int bw = std::min(w - 4, 78);
            int inner = bw - 2;
            int left0 = std::max(2, (w - bw) / 2);
            std::string L = spaces(left0);
            auto fmt = [&](const Item& it) {
                std::string s = chip(it.key, it.color) + " " + fg(col::TEXT) + it.label + R();
                if (!it.extra.empty()) s += fg(col::GREY) + " \xC2\xB7 " + it.extra + R();
                return s;
            };
            std::string ttl = " MENU ";
            int side = (inner - 6) / 2;
            std::cout << L << fg(col::AMBER) << "╭" << term::rep("─", side) << fg(col::GOLD) << bold() << ttl << R()
                      << fg(col::AMBER) << term::rep("─", inner - side - 6) << "╮" << R() << "\n";
            auto boxRow = [&](const std::string& content) {
                std::cout << L << fg(col::AMBER) << "│" << R() << content << spaces(inner - visibleLen(content)) << fg(col::AMBER) << "│" << R() << "\n";
            };
            bool two = inner >= 66;
            if (two) {
                int cw = (inner - 2) / 2;
                for (size_t i = 0; i < left.size(); ++i) boxRow("  " + term::padRight(fmt(left[i]), cw) + fmt(right[i]));
            } else {
                for (auto& it : left) boxRow("  " + fmt(it));
                for (auto& it : right) boxRow("  " + fmt(it));
            }
            boxRow("  " + chip("0", 203) + " " + fg(col::TEXT) + "Exit" + R());
            std::cout << L << fg(col::AMBER) << "╰" << term::rep("─", inner) << "╯" << R() << "\n\n";
            if (!flash.empty()) { std::cout << "  " << fg(col::ROSE) << "\xE2\x99\xA6 " << flash << R() << "\n"; flash.clear(); }
            std::cout << "  " << fg(col::GREY) << "Choose a number, or just type a reference " << R() << fg(col::GOLD) << "(John 3:16)" << R()
                      << fg(col::GREY) << " or words to search " << R() << fg(col::GOLD) << "(love one another)" << R() << "\n";
            std::string in;
            readLine(promptStr(), in);
            std::string low = toLower(in);
            if (low.empty()) continue;
            if (low == "0" || low == "q" || low == "exit" || low == "quit") return;
            if (low == "1") bookPicker();
            else if (low == "2") reader(store.lastBook, store.lastChapter);
            else if (low == "3") gotoReference();
            else if (low == "4") searchScreen("");
            else if (low == "5") collectionMenu("Verses for what I need", "Choose what is on your heart", kTopics, col::ROSE);
            else if (low == "6") collectionMenu("Famous Passages", "The best-loved chapters and verses", kFamous, col::VIOLET);
            else if (low == "7") verseOfTheDay();
            else if (low == "8") randomVerse();
            else if (low == "9") reader(kPsalms, std::uniform_int_distribution<int>(1, 150)(rng));
            else if (low == "d") dailyReading();
            else if (low == "b") bookmarks();
            else if (low == "h" || low == "?" || low == "help") help();
            else omnibox(in);
        }
    }

    // Anything typed: a reference opens it, otherwise it is searched.
    void omnibox(const std::string& in) {
        Ref r = bible.parse(in, true);
        if (r.kind != RefKind::None) { showRef(r); return; }
        if (r.book >= 0) { flash = r.error; return; }
        if (!r.suggestions.empty() && std::any_of(in.begin(), in.end(), ::isdigit)) {
            flash = "Which book?" + suggestionText(r);
            return;
        }
        searchScreen(in);
    }

    void gotoReference() {
        while (true) {
            term::clear();
            const int w = W();
            std::cout << "\n" << title("Go to a reference", col::ORANGE, w) << "\n" << glowRule(w, col::ORANGE) << "\n\n";
            std::cout << "  " << fg(col::GREY) << "Examples: " << R();
            for (const char* e : {"John 3:16", "Psalm 23", "1 Cor 13:4-7", "Rom 8:28-39", "Gen 1", "Jude 24-25"})
                std::cout << fg(col::GOLD) << e << R() << fg(col::DARK) << "  \xC2\xB7  " << R();
            std::cout << "\n\n";
            if (!flash.empty()) { std::cout << "  " << fg(col::ROSE) << flash << R() << "\n\n"; flash.clear(); }
            std::string in;
            readLine(promptStr("Reference (q back) "), in);
            if (in.empty() || toLower(in) == "q") return;
            Ref r = bible.parse(in);
            if (r.kind == RefKind::None) { flash = r.error + suggestionText(r); continue; }
            showRef(r);
            return;
        }
    }

    void farewell() {
        term::clear();
        std::cout << "\n";
        Ref r = bible.parse("Numbers 6:24-26");
        verseCard("Go in peace", r.start, r.end);
        std::cout << "\n" << term::center(fg(col::GREY) + italic() + "Grace and peace be with you." + R(), W()) << "\n\n";
    }

    // ---------------------------------------------------- command-line use
    void printPassage(int a, int z) {
        std::cout << "\n  " << catColor(bible.verses[a].book) << bold() << bible.rangeRef(a, z) << R() << "\n";
        std::vector<std::string> lines;
        addVerses(lines, a, z, nullptr, -1, 2, true);
        for (auto& l : lines) std::cout << l << "\n";
        std::cout << "\n";
    }
    void printSearch(const std::string& q, size_t limit) {
        SearchResult res = se.search(q);
        if (!res.error.empty()) { std::cout << res.error << "\n"; return; }
        for (auto& n : res.notes) std::cout << fg(col::AMBER) << n << R() << "\n";
        char ms[32];
        std::snprintf(ms, sizeof ms, "%.2f", res.millis);
        std::cout << fg(col::GREY) << res.verses.size() << (res.verses.size() == 1 ? " verse" : " verses")
                  << " found in " << ms << " ms" << R() << "\n\n";
        for (size_t k = 0; k < res.verses.size() && k < limit; ++k) {
            int vi = res.verses[k];
            std::cout << "  " << catColor(bible.verses[vi].book) << bold() << bible.ref(vi) << R() << "\n";
            for (auto& line : wrap(bible.verses[vi].text, W() - 8))
                std::cout << "      " << colorLine(line, &res.highlight, fg(col::TEXT)) << "\n";
        }
    }
};

// ================================================================= main ======
static std::string findData(const std::string& argv0, const std::string& override) {
    std::vector<std::string> tries;
    if (!override.empty()) tries.push_back(override);
    if (const char* e = std::getenv("HOLYBIBLE_DATA")) tries.push_back(e);
    std::string dir = ".";
    size_t p = argv0.find_last_of("/\\");
    if (p != std::string::npos) dir = argv0.substr(0, p);
    for (const std::string& d : {dir, dir + "/..", std::string("."), std::string("..")}) {
        tries.push_back(d + "/data/kjv.tsv");
        tries.push_back(d + "/kjv.tsv");
    }
    for (auto& t : tries) if (std::ifstream(t)) return t;
    return tries.empty() ? "data/kjv.tsv" : tries.front();
}

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    bool noColor = false, searchMode = false, votd = false;
    std::string dataPath, query;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--no-color") noColor = true;
        else if (a == "--data" && i + 1 < argc) dataPath = argv[++i];
        else if (a == "-s" || a == "--search") searchMode = true;
        else if (a == "--votd") votd = true;
        else if (a == "-h" || a == "--help") {
            std::cout << "Holy Bible (KJV) for the console\n\n"
                         "  holybible                     open the interactive Bible\n"
                         "  holybible John 3:16           print a verse or passage\n"
                         "  holybible -s love one another search and print the best 20 verses\n"
                         "  holybible --votd              verse of the day\n"
                         "  --data FILE   path to kjv.tsv     --no-color   plain text\n";
            return 0;
        } else query += (query.empty() ? "" : " ") + a;
    }
    term::init(noColor);

    App app;
    std::string err, path = findData(argv[0], dataPath);
    if (!app.bible.load(path, err)) {
        std::cerr << "Could not load the Bible text: " << err << "\n"
                  << "Keep the 'data' folder (with kjv.tsv) next to the program, or use --data <file>.\n";
        return 1;
    }
    app.se.build(app.bible);
    app.store.load(app.bible);

    if (votd) {
        std::time_t t = std::time(nullptr);
        Ref r = app.bible.parse(kVerseOfTheDay[(size_t)((t / 86400) % (long)kVerseOfTheDay.size())]);
        std::cout << "\n";
        app.verseCard("Verse of the day", r.start, r.end);
        std::cout << "\n";
        return 0;
    }
    if (!query.empty()) {
        Ref r = searchMode ? Ref{} : app.bible.parse(query, true);
        if (r.kind != RefKind::None) app.printPassage(r.start, r.end);
        else if (!searchMode && r.book >= 0) { std::cout << r.error << "\n"; return 1; }
        else app.printSearch(query, 20);
        return 0;
    }

    try {
        app.splash();
        app.mainMenu();
    } catch (const QuitSignal&) {
        std::cout << "\n";
    }
    app.farewell();
    return 0;
}
