// books.hpp - the 66 books of the Holy Bible, their groups and abbreviations.
#pragma once
#include <array>

namespace bible {

struct Category {
    const char* name;
    int color;      // 256-color palette index
};

// Literary groups, each with its own color in the console.
inline const std::array<Category, 10> kCategories = {{
    {"Law",              214},
    {"History",          179},
    {"Poetry & Wisdom",  141},
    {"Major Prophets",   203},
    {"Minor Prophets",   210},
    {"Gospels",          220},
    {"Church History",   114},
    {"Pauline Epistles",  81},
    {"General Epistles", 117},
    {"Prophecy",         211},
}};

struct BookInfo {
    const char* name;
    int category;
    const char* aliases;   // space separated, lowercase, no spaces/dots inside
};

inline const std::array<BookInfo, 66> kBooks = {{
    // ---------------- Old Testament ----------------
    {"Genesis",          0, "gen ge gn"},
    {"Exodus",           0, "exo ex exod"},
    {"Leviticus",        0, "lev le lv"},
    {"Numbers",          0, "num nu nm nb"},
    {"Deuteronomy",      0, "deut deu de dt"},
    {"Joshua",           1, "josh jos jsh"},
    {"Judges",           1, "judg jdg jg jdgs"},
    {"Ruth",             1, "rth ru"},
    {"1 Samuel",         1, "1sam 1sa 1sm 1s"},
    {"2 Samuel",         1, "2sam 2sa 2sm 2s"},
    {"1 Kings",          1, "1kgs 1ki 1kg 1k 1kin"},
    {"2 Kings",          1, "2kgs 2ki 2kg 2k 2kin"},
    {"1 Chronicles",     1, "1chr 1ch 1chron"},
    {"2 Chronicles",     1, "2chr 2ch 2chron"},
    {"Ezra",             1, "ezr"},
    {"Nehemiah",         1, "neh ne"},
    {"Esther",           1, "esth est es"},
    {"Job",              2, "jb"},
    {"Psalms",           2, "ps psa psalm pss psm"},
    {"Proverbs",         2, "prov pro prv pr"},
    {"Ecclesiastes",     2, "eccl ecc ec eccles qoh"},
    {"Song of Solomon",  2, "song sos so sg canticles songofsongs"},
    {"Isaiah",           3, "isa is"},
    {"Jeremiah",         3, "jer je jr"},
    {"Lamentations",     3, "lam la"},
    {"Ezekiel",          3, "ezek eze ezk"},
    {"Daniel",           3, "dan da dn"},
    {"Hosea",            4, "hos ho"},
    {"Joel",             4, "jl"},
    {"Amos",             4, "am"},
    {"Obadiah",          4, "obad ob"},
    {"Jonah",            4, "jnh jon"},
    {"Micah",            4, "mic mc"},
    {"Nahum",            4, "nah na"},
    {"Habakkuk",         4, "hab hb"},
    {"Zephaniah",        4, "zeph zep zp"},
    {"Haggai",           4, "hag hg"},
    {"Zechariah",        4, "zech zec zc"},
    {"Malachi",          4, "mal ml"},
    // ---------------- New Testament ----------------
    {"Matthew",          5, "matt mt mat"},
    {"Mark",             5, "mrk mk mr mar"},
    {"Luke",             5, "luk lk lu"},
    {"John",             5, "jhn jn joh"},
    {"Acts",             6, "act ac"},
    {"Romans",           7, "rom ro rm"},
    {"1 Corinthians",    7, "1cor 1co"},
    {"2 Corinthians",    7, "2cor 2co"},
    {"Galatians",        7, "gal ga"},
    {"Ephesians",        7, "eph ephes"},
    {"Philippians",      7, "phil php pp"},
    {"Colossians",       7, "col"},
    {"1 Thessalonians",  7, "1thess 1th 1thes"},
    {"2 Thessalonians",  7, "2thess 2th 2thes"},
    {"1 Timothy",        7, "1tim 1ti 1tm"},
    {"2 Timothy",        7, "2tim 2ti 2tm"},
    {"Titus",            7, "tit ti"},
    {"Philemon",         7, "philem phm phlm pm"},
    {"Hebrews",          8, "heb"},
    {"James",            8, "jas jm jam"},
    {"1 Peter",          8, "1pet 1pe 1pt 1p"},
    {"2 Peter",          8, "2pet 2pe 2pt 2p"},
    {"1 John",           8, "1jn 1jo 1jhn 1j"},
    {"2 John",           8, "2jn 2jo 2jhn 2j"},
    {"3 John",           8, "3jn 3jo 3jhn 3j"},
    {"Jude",             8, "jud jd"},
    {"Revelation",       9, "rev re rv revelations apocalypse"},
}};

// Spanish book names also work: "Juan 3,16", "Salmos 23", "1 Corintios 13".
inline const std::array<const char*, 66> kSpanishNames = {{
    "genesis", "exodo", "levitico", "numeros", "deuteronomio", "josue", "jueces", "rut",
    "1samuel", "2samuel", "1reyes", "2reyes", "1cronicas", "2cronicas", "esdras", "nehemias",
    "ester", "job", "salmos salmo sal", "proverbios", "eclesiastes", "cantares cantar", "isaias",
    "jeremias", "lamentaciones", "ezequiel", "daniel", "oseas", "joel", "amos", "abdias",
    "jonas", "miqueas", "nahum", "habacuc", "sofonias", "hageo", "zacarias", "malaquias",
    "mateo", "marcos", "lucas", "juan", "hechos", "romanos", "1corintios", "2corintios",
    "galatas", "efesios", "filipenses", "colosenses", "1tesalonicenses", "2tesalonicenses",
    "1timoteo", "2timoteo", "tito", "filemon", "hebreos", "santiago", "1pedro", "2pedro",
    "1juan", "2juan", "3juan", "judas", "apocalipsis",
}};

constexpr int kPsalms = 18;    // index of the book of Psalms
constexpr int kFirstNT = 39;    // Matthew

} // namespace bible
