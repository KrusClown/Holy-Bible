// Verifies every curated reference resolves, plus parser edge cases.
#include "../src/search.hpp"
#include "../src/content.hpp"
#include <iostream>
using namespace bible;
int main() {
    Bible b; std::string err;
    if (!b.load("data/kjv.tsv", err)) { std::cerr << err << "\n"; return 1; }
    int bad = 0, total = 0;
    auto check = [&](const char* s) {
        ++total; Ref r = b.parse(s);
        if (r.kind == RefKind::None) { std::cout << "FAIL " << s << ": " << r.error << "\n"; ++bad; return; }
        std::cout << "ok  " << s << " -> " << b.rangeRef(r.start, r.end) << " (" << (r.end - r.start + 1) << ")  " << b.verses[r.start].text.substr(0, 60) << "\n";
    };
    for (auto& c : kTopics) for (auto* s : c.refs) check(s);
    for (auto& c : kFamous) for (auto* s : c.refs) check(s);
    for (auto* s : kVerseOfTheDay) check(s);
    for (auto* s : {"Juan 3,16", "Salmos 23", "1 Corintios 13:4-7", "Génesis 1:1", "Apocalipsis 22:21", "Ps 119:176",
                    "II Kings 2:11", "Philemon 4", "obad 21", "Song 2:4", "Rev 22", "genisis 1", "Mat 5:3-12", "3 jn 4"}) check(s);
    std::cout << "\n" << total << " references, " << bad << " failures\n";
    SearchEngine se; se.build(b);
    for (auto* q : {"fiath", "faith hope charity", "\"god is love\"", "love* in:1john", "peace -war in:ot", "jesus wept", "xyzzy"}) {
        auto r = se.search(q);
        std::cout << "search [" << q << "] -> " << r.verses.size() << " " << (r.error.empty() ? "" : r.error)
                  << (r.notes.empty() ? "" : r.notes[0]) << (r.verses.empty() ? "" : "  top: " + b.ref(r.verses[0])) << "\n";
    }
    std::cout << "index built in " << se.buildMillis << " ms, vocabulary " << se.vocabularySize() << ", words " << se.totalWords() << "\n";
    return bad ? 1 : 0;
}
