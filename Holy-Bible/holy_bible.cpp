/*
 ============================================================
   THE HOLY BIBLE - Interactive Console Experience
   With ASCII Art, Colors, and Animations
   Compile: g++ -o holy_bible holy_bible.cpp
   Run:     ./holy_bible
 ============================================================
*/

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

// ─── ANSI Color & Style Codes ────────────────────────────
#define RESET       "\033[0m"
#define BOLD        "\033[1m"
#define DIM         "\033[2m"
#define ITALIC      "\033[3m"
#define UNDERLINE   "\033[4m"
#define BLINK       "\033[5m"
#define REVERSE     "\033[7m"

// Foreground Colors
#define BLACK       "\033[30m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"

// Bright Foreground Colors
#define BRIGHT_RED      "\033[91m"
#define BRIGHT_GREEN    "\033[92m"
#define BRIGHT_YELLOW   "\033[93m"
#define BRIGHT_BLUE     "\033[94m"
#define BRIGHT_MAGENTA  "\033[95m"
#define BRIGHT_CYAN     "\033[96m"
#define BRIGHT_WHITE    "\033[97m"

// Background Colors
#define BG_BLACK    "\033[40m"
#define BG_BLUE     "\033[44m"
#define BG_YELLOW   "\033[43m"

// ─── ANSI Cursor Control ─────────────────────────────────
#define CLEAR_SCREEN    "\033[2J\033[H"
#define HIDE_CURSOR     "\033[?25l"
#define SHOW_CURSOR     "\033[?25h"
#define SAVE_CURSOR     "\033[s"
#define RESTORE_CURSOR  "\033[u"

void moveCursor(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H";
}

void sleep_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void clearScreen() {
    std::cout << CLEAR_SCREEN;
    std::cout.flush();
}

// ─── Typewriter Effect ────────────────────────────────────
void typewrite(const std::string& text, int delay_ms = 30, const std::string& color = WHITE) {
    for (char c : text) {
        std::cout << color << c << RESET;
        std::cout.flush();
        sleep_ms(delay_ms);
    }
}

void printLine(const std::string& text, const std::string& color = WHITE, bool bold = false) {
    if (bold) std::cout << BOLD;
    std::cout << color << text << RESET << "\n";
}

// ─── Animated Horizontal Rule ─────────────────────────────
void animatedRule(int width = 70, const std::string& color = YELLOW, int delay = 10) {
    std::cout << color << BOLD;
    for (int i = 0; i < width; ++i) {
        std::cout << "═";
        std::cout.flush();
        sleep_ms(delay);
    }
    std::cout << RESET << "\n";
}

void printRule(int width = 70, const std::string& color = YELLOW) {
    std::cout << color << BOLD;
    for (int i = 0; i < width; ++i) std::cout << "═";
    std::cout << RESET << "\n";
}

// ─── Spinning Loader ──────────────────────────────────────
void spinLoader(const std::string& msg, int duration_ms = 1500) {
    const std::string frames = "|/-\\";
    int iterations = duration_ms / 80;
    std::cout << HIDE_CURSOR;
    for (int i = 0; i < iterations; ++i) {
        std::cout << "\r" << BRIGHT_YELLOW << BOLD
                  << frames[i % 4] << " " << msg << "  " << RESET;
        std::cout.flush();
        sleep_ms(80);
    }
    std::cout << "\r" << BRIGHT_GREEN << BOLD << "✓ " << msg << " Done!" << RESET << "\n";
    std::cout << SHOW_CURSOR;
}

// ─── Glowing Stars Animation ─────────────────────────────
void starField(int rows = 5, int cols = 70, int frames = 8) {
    std::cout << HIDE_CURSOR;
    std::string starChars = "✦✧⋆*·";
    std::vector<std::string> colors = {BRIGHT_WHITE, BRIGHT_YELLOW, BRIGHT_CYAN, DIM WHITE};
    srand(42);
    for (int f = 0; f < frames; ++f) {
        // Move cursor back up
        if (f > 0) std::cout << "\033[" << rows << "A";
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                if (rand() % 8 == 0) {
                    int ci = rand() % colors.size();
                    std::cout << colors[ci] << starChars[rand() % starChars.size()] << RESET;
                } else {
                    std::cout << " ";
                }
            }
            std::cout << "\n";
        }
        std::cout.flush();
        sleep_ms(200);
    }
    std::cout << SHOW_CURSOR;
}

// ─── Cross ASCII Art ──────────────────────────────────────
void drawCross(bool animated = false) {
    const std::vector<std::string> cross = {
        "          ╔═══════╗          ",
        "          ║ + + + ║          ",
        "          ║       ║          ",
        "╔═════════╬═══════╬═════════╗",
        "║  * * *  ║       ║  * * *  ║",
        "╚═════════╬═══════╬═════════╝",
        "          ║       ║          ",
        "          ║  ✝    ║          ",
        "          ║       ║          ",
        "          ╚═══════╝          ",
    };

    // Gradient colors for the cross
    std::vector<std::string> crossColors = {
        BRIGHT_YELLOW, YELLOW, BRIGHT_YELLOW, YELLOW,
        BRIGHT_WHITE, YELLOW, BRIGHT_YELLOW, YELLOW,
        BRIGHT_YELLOW, YELLOW
    };

    for (size_t i = 0; i < cross.size(); ++i) {
        std::cout << BOLD << crossColors[i % crossColors.size()]
                  << "          " << cross[i] << RESET << "\n";
        if (animated) sleep_ms(60);
    }
}

// ─── Bible ASCII Art Title ────────────────────────────────
void drawBibleTitle() {
    // Large stylized ASCII Bible
    const std::vector<std::string> bible_art = {
        "  ██████╗ ██╗██████╗ ██╗     ███████╗",
        "  ██╔══██╗██║██╔══██╗██║     ██╔════╝",
        "  ██████╔╝██║██████╔╝██║     █████╗  ",
        "  ██╔══██╗██║██╔══██╗██║     ██╔══╝  ",
        "  ██████╔╝██║██████╔╝███████╗███████╗",
        "  ╚═════╝ ╚═╝╚═════╝ ╚══════╝╚══════╝",
    };

    // Cycle through gold/white gradient
    std::vector<std::string> gradient = {
        BRIGHT_YELLOW, YELLOW, BRIGHT_WHITE, YELLOW, BRIGHT_YELLOW, YELLOW
    };

    for (size_t i = 0; i < bible_art.size(); ++i) {
        typewrite(bible_art[i] + "\n", 5, gradient[i % gradient.size()]);
    }
}

// ─── Holy Bible Book Cover ────────────────────────────────
void drawBookCover() {
    printLine("  ╔══════════════════════════════════════════════════════════╗", YELLOW, true);
    printLine("  ║                                                          ║", YELLOW, true);
    printLine("  ║           ✝  T H E  H O L Y  B I B L E  ✝              ║", BRIGHT_WHITE, true);
    printLine("  ║                                                          ║", YELLOW, true);
    printLine("  ║         Old Testament  ✦  New Testament                  ║", BRIGHT_CYAN, true);
    printLine("  ║                                                          ║", YELLOW, true);
    printLine("  ║    ┌────────────────────────────────────────────────┐    ║", YELLOW, true);
    printLine("  ║    │                                                │    ║", YELLOW, true);
    printLine("  ║    │   \"For God so loved the world that He gave     │    ║", BRIGHT_WHITE, true);
    printLine("  ║    │    His only begotten Son, that whosoever       │    ║", BRIGHT_WHITE, true);
    printLine("  ║    │    believeth in Him should not perish but      │    ║", BRIGHT_WHITE, true);
    printLine("  ║    │    have everlasting life.\"                     │    ║", BRIGHT_WHITE, true);
    printLine("  ║    │                        — John 3:16             │    ║", BRIGHT_YELLOW, true);
    printLine("  ║    └────────────────────────────────────────────────┘    ║", YELLOW, true);
    printLine("  ║                                                          ║", YELLOW, true);
    printLine("  ║            King James Version  •  66 Books               ║", BRIGHT_MAGENTA, true);
    printLine("  ║                                                          ║", YELLOW, true);
    printLine("  ╚══════════════════════════════════════════════════════════╝", YELLOW, true);
}

// ─── Animated Opening Ceremony ───────────────────────────
void openingCeremony() {
    clearScreen();
    std::cout << HIDE_CURSOR;

    // Night sky of stars
    std::cout << "\n";
    starField(4, 70, 6);

    // Dove ASCII Art
    std::cout << BRIGHT_WHITE << BOLD;
    std::cout << "                    ___     _,--'.`-.  \n";
    std::cout << "                 .-'   `--=/     `    \\\n";
    std::cout << "                /        /            |\n";
    std::cout << "               |   , .__/ `._,  \\     |\n";
    std::cout << "                \\  `'         `--'`--'/\n";
    std::cout << "                 `.  🕊️  HOLY SPIRIT  .'\n";
    std::cout << "                   `-._          _.-'  \n";
    std::cout << "                       `'------''      \n" << RESET;

    sleep_ms(800);
    animatedRule(70, YELLOW, 8);
    sleep_ms(200);
    drawBibleTitle();
    sleep_ms(300);
    animatedRule(70, YELLOW, 8);
    sleep_ms(500);

    printLine("\n       ✦ ✦ ✦  Welcome to the Sacred Scripture Explorer  ✦ ✦ ✦\n", BRIGHT_CYAN, true);
    spinLoader("Opening the Holy Bible", 1400);
    sleep_ms(400);
    std::cout << SHOW_CURSOR;
}

// ─── Data: Books of the Bible ─────────────────────────────
struct Book {
    std::string name;
    std::string testament;
    int chapters;
    std::string description;
};

std::vector<Book> bibleBooks = {
    // Old Testament
    {"Genesis",      "Old", 50, "Creation, Adam & Eve, Noah's Ark, Abraham"},
    {"Exodus",       "Old", 40, "Moses leads Israel from Egypt, Ten Commandments"},
    {"Psalms",       "Old", 150,"Songs and poems of praise, lament, and worship"},
    {"Proverbs",     "Old", 31, "Wisdom literature attributed to Solomon"},
    {"Isaiah",       "Old", 66, "Prophecies of the Messiah and coming salvation"},
    {"Daniel",       "Old", 12, "Daniel in Babylon, visions, lion's den"},
    {"Jonah",        "Old", 4,  "Jonah, the whale, Nineveh's repentance"},
    // New Testament
    {"Matthew",      "New", 28, "Gospel of Jesus: Sermon on the Mount, parables"},
    {"John",         "New", 21, "Gospel of love: 'In the beginning was the Word'"},
    {"Luke",         "New", 24, "Gospel: nativity story, Good Samaritan"},
    {"Acts",         "New", 28, "Early Church and Paul's missionary journeys"},
    {"Romans",       "New", 16, "Paul's letter on faith, grace, and righteousness"},
    {"Revelation",   "New", 22, "Apocalyptic vision: end times, new Jerusalem"},
};

// ─── Famous Bible Verses ──────────────────────────────────
struct Verse {
    std::string ref;
    std::string text;
    std::string theme;
};

std::vector<Verse> famousVerses = {
    {"John 3:16",         "For God so loved the world that He gave His only begotten Son, that whosoever believeth in Him should not perish but have everlasting life.",  "Love"},
    {"Psalm 23:1",        "The Lord is my shepherd; I shall not want.",                                                     "Trust"},
    {"Philippians 4:13",  "I can do all things through Christ which strengtheneth me.",                                     "Strength"},
    {"Jeremiah 29:11",    "For I know the plans I have for you, plans to prosper you and not to harm you, plans to give you hope and a future.",  "Hope"},
    {"Romans 8:28",       "And we know that in all things God works for the good of those who love Him, who have been called according to His purpose.", "Faith"},
    {"John 11:35",        "Jesus wept.",                                                                                    "Compassion"},
    {"Psalm 46:1",        "God is our refuge and strength, an ever-present help in trouble.",                               "Protection"},
    {"Proverbs 3:5-6",    "Trust in the Lord with all your heart and lean not on your own understanding; in all your ways submit to Him, and He will make your paths straight.", "Wisdom"},
    {"Matthew 5:9",       "Blessed are the peacemakers, for they will be called children of God.",                          "Peace"},
    {"1 Corinthians 13:4","Love is patient, love is kind. It does not envy, it does not boast, it is not proud.",           "Love"},
};

// ─── Bible Stories ────────────────────────────────────────
struct Story {
    std::string title;
    std::string book_ref;
    std::vector<std::string> lines;
};

std::vector<Story> stories = {
    {
        "The Creation",
        "Genesis 1:1 - 2:3",
        {
            "In the beginning, God created the heavens and the earth.",
            "The earth was formless and empty, darkness was over the surface of the deep.",
            "And the Spirit of God was hovering over the waters.",
            "Day 1: God said 'Let there be LIGHT' — and there was light.",
            "Day 2: God separated the waters and created the sky.",
            "Day 3: Dry land appeared, and plants grew upon the earth.",
            "Day 4: The sun, moon, and stars were placed in the sky.",
            "Day 5: Fish filled the seas and birds filled the skies.",
            "Day 6: Animals and mankind were created — in God's own image.",
            "Day 7: God rested, and blessed the seventh day as holy.",
        }
    },
    {
        "Noah's Ark",
        "Genesis 6-9",
        {
            "The earth was filled with wickedness, and God grieved in His heart.",
            "But Noah found grace in the eyes of the Lord.",
            "God spoke to Noah: 'Build an ark of cypress wood.'",
            "The ark was 300 cubits long, 50 wide, and 30 cubits high.",
            "Noah brought two of every living creature aboard the ark.",
            "For 40 days and 40 nights, rain fell upon the earth.",
            "The waters covered all the mountains; every creature outside perished.",
            "After 150 days, the ark rested on the mountains of Ararat.",
            "Noah sent out a dove — and it returned with an olive leaf!",
            "God placed a RAINBOW in the sky as His covenant of promise.",
        }
    },
    {
        "David and Goliath",
        "1 Samuel 17",
        {
            "The Philistines gathered their armies for battle against Israel.",
            "Their champion: Goliath — nine feet tall, clad in bronze armor.",
            "For 40 days he taunted Israel: 'Give me a man to fight!'",
            "No soldier dared face the giant — but young David stepped forward.",
            "David said: 'The Lord who delivered me from lions will deliver me!'",
            "Saul offered armor, but David refused: 'I am not used to these.'",
            "He picked five smooth stones from the stream.",
            "With a single sling, the stone struck Goliath's forehead — he fell!",
            "David took Goliath's sword and cut off his head.",
            "'The battle belongs to the Lord!' — and all Israel shouted for joy.",
        }
    },
};

// ─── Display Section Header ───────────────────────────────
void sectionHeader(const std::string& title, const std::string& icon = "✝") {
    std::cout << "\n";
    printRule(70, YELLOW);
    std::cout << BOLD << BRIGHT_YELLOW << "  " << icon << "  " << BRIGHT_WHITE;
    typewrite(title, 25, BRIGHT_WHITE);
    std::cout << "  " << BRIGHT_YELLOW << icon << "\n" << RESET;
    printRule(70, YELLOW);
    std::cout << "\n";
}

// ─── Display Books of the Bible ───────────────────────────
void showBooks() {
    clearScreen();
    sectionHeader("BOOKS OF THE HOLY BIBLE", "📖");

    // Old Testament
    std::cout << BOLD << BRIGHT_YELLOW << "  ⬛ OLD TESTAMENT\n" << RESET;
    printRule(68, YELLOW);
    int count = 0;
    for (const auto& book : bibleBooks) {
        if (book.testament == "Old") {
            ++count;
            std::cout << BOLD << BRIGHT_CYAN << "  " << std::setw(2) << count << ". "
                      << BRIGHT_WHITE << std::left << std::setw(14) << book.name
                      << BRIGHT_YELLOW << "  [" << std::setw(3) << book.chapters << " ch]  "
                      << DIM << WHITE << book.description << RESET << "\n";
            sleep_ms(40);
        }
    }

    std::cout << "\n";
    // New Testament
    std::cout << BOLD << BRIGHT_MAGENTA << "  ⬛ NEW TESTAMENT\n" << RESET;
    printRule(68, MAGENTA);
    count = 0;
    for (const auto& book : bibleBooks) {
        if (book.testament == "New") {
            ++count;
            std::cout << BOLD << BRIGHT_CYAN << "  " << std::setw(2) << count << ". "
                      << BRIGHT_WHITE << std::left << std::setw(14) << book.name
                      << BRIGHT_MAGENTA << "  [" << std::setw(3) << book.chapters << " ch]  "
                      << DIM << WHITE << book.description << RESET << "\n";
            sleep_ms(40);
        }
    }

    std::cout << "\n";
    printLine("  📜  The Bible contains 66 books, 1,189 chapters, and ~31,000 verses.", BRIGHT_CYAN, true);
}

// ─── Display Famous Verses ────────────────────────────────
void showVerses() {
    clearScreen();
    sectionHeader("FAMOUS VERSES OF SCRIPTURE", "✦");

    for (size_t i = 0; i < famousVerses.size(); ++i) {
        const auto& v = famousVerses[i];
        std::cout << BOLD << BRIGHT_YELLOW << "  [" << (i+1) << "] " << v.ref << RESET << "\n";
        std::cout << "  " << DIM << CYAN << "Theme: " << v.theme << RESET << "\n";

        // Verse text with word-wrap
        std::string text = "  \"" + v.text + "\"";
        // Simple wrap at ~65 chars
        int lineLen = 0;
        std::cout << "  " << ITALIC << BRIGHT_WHITE << "\"";
        for (size_t j = 0; j < v.text.size(); ++j) {
            std::cout << v.text[j];
            ++lineLen;
            if (lineLen > 62 && v.text[j] == ' ') {
                std::cout << "\n   ";
                lineLen = 0;
            }
        }
        std::cout << "\"\n" << RESET;
        std::cout << "\n";
        sleep_ms(100);
    }
}

// ─── Display a Bible Story ────────────────────────────────
void showStory(const Story& story) {
    clearScreen();
    sectionHeader(story.title + " — " + story.book_ref, "📜");

    // Story frame
    printLine("  ┌─────────────────────────────────────────────────────────────┐", YELLOW, true);
    for (const auto& line : story.lines) {
        std::cout << YELLOW << BOLD << "  │  " << RESET;
        typewrite(line, 18, BRIGHT_WHITE);
        std::cout << "\n";
        sleep_ms(50);
    }
    printLine("  └─────────────────────────────────────────────────────────────┘", YELLOW, true);
}

// ─── Ten Commandments Display ─────────────────────────────
void showCommandments() {
    clearScreen();
    sectionHeader("THE TEN COMMANDMENTS", "⚖️");
    printLine("         Exodus 20:1-17  •  Deuteronomy 5:6-21\n", BRIGHT_CYAN, true);

    const std::vector<std::string> commandments = {
        "You shall have no other gods before Me.",
        "You shall not make or worship idols.",
        "You shall not misuse the name of the Lord your God.",
        "Remember the Sabbath day by keeping it holy.",
        "Honor your father and your mother.",
        "You shall not murder.",
        "You shall not commit adultery.",
        "You shall not steal.",
        "You shall not give false testimony.",
        "You shall not covet anything that belongs to your neighbor.",
    };

    // Stone tablet ASCII frame
    printLine("  ╔══════════════════════════════════════════════════════════╗", BRIGHT_WHITE, true);
    printLine("  ║       ✦ ✦ ✦   THE LAW OF GOD   ✦ ✦ ✦                  ║", YELLOW, true);
    printLine("  ╠══════════════════════════════════════════════════════════╣", BRIGHT_WHITE, true);

    for (int i = 0; i < (int)commandments.size(); ++i) {
        std::cout << BRIGHT_WHITE << BOLD << "  ║  " << RESET;
        std::cout << BRIGHT_YELLOW << BOLD << (i+1) << ". " << RESET;
        typewrite(commandments[i], 20, BRIGHT_WHITE);
        // Padding to align the right border
        int padLen = 54 - (int)commandments[i].size() - 3;
        if (padLen < 1) padLen = 1;
        std::cout << std::string(padLen, ' ') << BRIGHT_WHITE << BOLD << "║\n" << RESET;
        sleep_ms(80);
    }
    printLine("  ╚══════════════════════════════════════════════════════════╝", BRIGHT_WHITE, true);
}

// ─── Lord's Prayer Display ────────────────────────────────
void showLordsPrayer() {
    clearScreen();
    sectionHeader("THE LORD'S PRAYER", "🙏");
    printLine("              Matthew 6:9-13\n", BRIGHT_CYAN, true);

    const std::vector<std::pair<std::string,std::string>> prayer = {
        {"Our Father, who art in heaven,",          "Address"},
        {"hallowed be Thy name.",                    ""},
        {"Thy Kingdom come,",                        "Kingdom"},
        {"Thy will be done,",                        ""},
        {"on earth as it is in heaven.",             ""},
        {"Give us this day our daily bread,",        "Provision"},
        {"and forgive us our trespasses,",           "Forgiveness"},
        {"as we forgive those who trespass against us.", ""},
        {"And lead us not into temptation,",         "Protection"},
        {"but deliver us from evil.",                ""},
        {"For Thine is the Kingdom,",                "Doxology"},
        {"the Power, and the Glory,",                ""},
        {"forever and ever.  Amen. 🙏",             ""},
    };

    printLine("  ╭──────────────────────────────────────────────────╮", MAGENTA, true);
    for (const auto& [line, label] : prayer) {
        std::cout << MAGENTA << BOLD << "  │  " << RESET;
        typewrite(line, 22, BRIGHT_WHITE);
        if (!label.empty()) {
            std::cout << DIM << CYAN << "  ← " << label << RESET;
        }
        std::cout << "\n";
        sleep_ms(120);
    }
    printLine("  ╰──────────────────────────────────────────────────╯", MAGENTA, true);
}

// ─── Beatitudes ───────────────────────────────────────────
void showBeatitudes() {
    clearScreen();
    sectionHeader("THE BEATITUDES", "⛰️");
    printLine("         The Sermon on the Mount — Matthew 5:3-12\n", BRIGHT_CYAN, true);

    const std::vector<std::string> beatitudes = {
        "Blessed are the poor in spirit, for theirs is the kingdom of heaven.",
        "Blessed are those who mourn, for they will be comforted.",
        "Blessed are the meek, for they will inherit the earth.",
        "Blessed are those who hunger and thirst for righteousness,",
        "   for they will be filled.",
        "Blessed are the merciful, for they will be shown mercy.",
        "Blessed are the pure in heart, for they will see God.",
        "Blessed are the peacemakers, for they will be called children of God.",
        "Blessed are those persecuted because of righteousness,",
        "   for theirs is the kingdom of heaven.",
    };

    for (const auto& b : beatitudes) {
        if (b.substr(0, 7) == "Blessed") {
            std::cout << "\n  " << BRIGHT_YELLOW << BOLD << "✦ " << RESET;
            typewrite(b, 20, BRIGHT_WHITE);
        } else {
            std::cout << "    ";
            typewrite(b, 20, DIM + std::string(WHITE));
        }
        std::cout << "\n";
        sleep_ms(80);
    }
}

// ─── Random Verse of Day ──────────────────────────────────
void verseOfTheDay() {
    clearScreen();
    sectionHeader("VERSE OF THE DAY", "☀️");

    // Pick a random verse
    srand((unsigned)time(nullptr));
    const auto& v = famousVerses[rand() % famousVerses.size()];

    // Glowing box
    std::cout << "\n\n";
    printLine("        ╔══════════════════════════════════════════════════╗", BRIGHT_YELLOW, true);
    printLine("        ║                                                  ║", BRIGHT_YELLOW, true);
    std::cout << BRIGHT_YELLOW << BOLD << "        ║  " << RESET;
    typewrite(std::string(22, ' ') + v.ref, 30, BRIGHT_WHITE);
    std::cout << "  " << BRIGHT_YELLOW << BOLD << "║\n" << RESET;
    printLine("        ║                                                  ║", BRIGHT_YELLOW, true);
    printLine("        ╠══════════════════════════════════════════════════╣", BRIGHT_YELLOW, true);
    printLine("        ║                                                  ║", BRIGHT_YELLOW, true);

    // Word-wrap the verse text in the box
    std::string remaining = v.text;
    while (!remaining.empty()) {
        std::string chunk;
        if ((int)remaining.size() <= 46) {
            chunk = remaining;
            remaining = "";
        } else {
            size_t pos = remaining.rfind(' ', 46);
            if (pos == std::string::npos) pos = 46;
            chunk = remaining.substr(0, pos);
            remaining = remaining.substr(pos + 1);
        }
        int pad = 46 - (int)chunk.size();
        std::cout << BRIGHT_YELLOW << BOLD << "        ║  " << RESET;
        typewrite(chunk, 22, ITALIC + std::string(BRIGHT_WHITE));
        std::cout << std::string(pad, ' ') << BRIGHT_YELLOW << BOLD << "  ║\n" << RESET;
    }

    printLine("        ║                                                  ║", BRIGHT_YELLOW, true);
    printLine("        ╚══════════════════════════════════════════════════╝", BRIGHT_YELLOW, true);

    std::cout << "\n\n" << BRIGHT_CYAN << "        Theme: " << BOLD << v.theme << RESET << "\n\n";
}

// ─── Animated Flame / Candle ──────────────────────────────
void animateFlame(int seconds = 3) {
    const std::vector<std::string> flame_frames = {
        "    (   )\n   (    )\n  (  ::  )\n  |  ::  |\n  | /\\/ |\n  `----'",
        "    ( )  \n   (   ) \n  (  :   )\n  |  ::  |\n  | /\\/ |\n  `----'",
        "   (   ) \n  (  :  )\n (   ::  )\n  |  ::  |\n  | /\\/ |\n  `----'",
    };
    std::cout << HIDE_CURSOR;
    for (int t = 0; t < seconds * 6; ++t) {
        if (t > 0) std::cout << "\033[6A";
        std::string color = (t % 3 == 0) ? BRIGHT_YELLOW : (t % 3 == 1) ? BRIGHT_RED : YELLOW;
        std::cout << color << BOLD << flame_frames[t % 3] << RESET << "\n";
        std::cout.flush();
        sleep_ms(160);
    }
    std::cout << SHOW_CURSOR;
}

// ─── Main Menu ────────────────────────────────────────────
void showMenu() {
    clearScreen();
    std::cout << "\n";
    drawCross(false);
    std::cout << "\n";

    printLine("  ╔══════════════════════════════════════════════════════════╗", YELLOW, true);
    printLine("  ║             ✝  HOLY BIBLE EXPLORER  ✝                   ║", BRIGHT_WHITE, true);
    printLine("  ╠══════════════════════════════════════════════════════════╣", YELLOW, true);
    printLine("  ║                                                          ║", YELLOW, true);
    printLine("  ║   📖  1. Books of the Bible                              ║", BRIGHT_CYAN, true);
    printLine("  ║   ✦   2. Famous Verses of Scripture                      ║", BRIGHT_CYAN, true);
    printLine("  ║   📜  3. Bible Stories                                    ║", BRIGHT_CYAN, true);
    printLine("  ║   ⚖️   4. The Ten Commandments                            ║", BRIGHT_CYAN, true);
    printLine("  ║   🙏  5. The Lord's Prayer                               ║", BRIGHT_CYAN, true);
    printLine("  ║   ⛰️   6. The Beatitudes                                  ║", BRIGHT_CYAN, true);
    printLine("  ║   ☀️   7. Verse of the Day                               ║", BRIGHT_CYAN, true);
    printLine("  ║   🕯️   8. Candlelight Moment                              ║", BRIGHT_CYAN, true);
    printLine("  ║   ✝   9. The Bible Cover                                 ║", BRIGHT_CYAN, true);
    printLine("  ║                                                          ║", YELLOW, true);
    printLine("  ║   ✗  0. Exit                                             ║", RED, true);
    printLine("  ║                                                          ║", YELLOW, true);
    printLine("  ╚══════════════════════════════════════════════════════════╝", YELLOW, true);
    std::cout << "\n";
    std::cout << BRIGHT_YELLOW << BOLD << "  ✝  Your choice: " << RESET;
}

// ─── Story Selection Menu ─────────────────────────────────
void showStoryMenu() {
    clearScreen();
    sectionHeader("BIBLE STORIES", "📜");

    for (size_t i = 0; i < stories.size(); ++i) {
        std::cout << BOLD << BRIGHT_CYAN << "  [" << (i+1) << "] " << RESET
                  << BRIGHT_WHITE << stories[i].title << "  "
                  << DIM << WHITE << "(" << stories[i].book_ref << ")\n" << RESET;
    }

    std::cout << BOLD << RED << "\n  [0] Back to Main Menu\n" << RESET;
    std::cout << BRIGHT_YELLOW << BOLD << "\n  ✝  Choose a story: " << RESET;
}

// ─── Goodbye Screen ───────────────────────────────────────
void goodbyeScreen() {
    clearScreen();
    std::cout << "\n";
    printRule(70, YELLOW);
    printLine("\n       May the Lord bless you and keep you;", BRIGHT_WHITE, true);
    printLine("       May the Lord make His face shine on you", BRIGHT_WHITE, true);
    printLine("       and be gracious to you;", BRIGHT_WHITE, true);
    printLine("       May the Lord turn His face toward you", BRIGHT_WHITE, true);
    printLine("       and give you peace.  🕊️", BRIGHT_WHITE, true);
    printLine("\n                        — Numbers 6:24-26\n", BRIGHT_YELLOW, true);
    printRule(70, YELLOW);

    std::cout << "\n";
    animateFlame(2);
    std::cout << "\n";

    typewrite("  ✝  God bless you. Come back and read His Word again!  ✝\n\n", 30, BRIGHT_YELLOW);
    sleep_ms(500);
}

// ─── Press any key ────────────────────────────────────────
void pressAnyKey() {
    std::cout << "\n\n" << DIM << CYAN << "  [ Press ENTER to return to menu... ]" << RESET;
    std::cin.ignore(10000, '\n');
    std::cin.get();
}

// ─── MAIN ─────────────────────────────────────────────────
int main() {
    // Enable UTF-8 output on some systems
    system("stty -echo 2>/dev/null; stty echo 2>/dev/null");

    openingCeremony();
    sleep_ms(600);

    bool running = true;
    while (running) {
        showMenu();

        std::string input;
        std::getline(std::cin, input);
        if (input.empty()) std::getline(std::cin, input);

        if (input == "1") {
            showBooks();
            pressAnyKey();
        }
        else if (input == "2") {
            showVerses();
            pressAnyKey();
        }
        else if (input == "3") {
            showStoryMenu();
            std::string choice;
            std::getline(std::cin, choice);
            if (!choice.empty() && choice != "0") {
                int idx = std::stoi(choice) - 1;
                if (idx >= 0 && idx < (int)stories.size()) {
                    showStory(stories[idx]);
                    pressAnyKey();
                }
            }
        }
        else if (input == "4") {
            showCommandments();
            pressAnyKey();
        }
        else if (input == "5") {
            showLordsPrayer();
            pressAnyKey();
        }
        else if (input == "6") {
            showBeatitudes();
            pressAnyKey();
        }
        else if (input == "7") {
            verseOfTheDay();
            pressAnyKey();
        }
        else if (input == "8") {
            clearScreen();
            sectionHeader("CANDLELIGHT MOMENT", "🕯️");
            printLine("  Take a quiet moment. Let your heart be still.\n", BRIGHT_WHITE, true);
            animateFlame(4);
            printLine("\n  \"Be still, and know that I am God.\" — Psalm 46:10\n", BRIGHT_YELLOW, true);
            pressAnyKey();
        }
        else if (input == "9") {
            clearScreen();
            sectionHeader("THE HOLY BIBLE", "✝");
            drawBookCover();
            pressAnyKey();
        }
        else if (input == "0" || input == "q" || input == "Q") {
            goodbyeScreen();
            running = false;
        }
        else {
            std::cout << "\n  " << BRIGHT_RED << "✗ Invalid option. Please choose 0-9.\n" << RESET;
            sleep_ms(800);
        }
    }

    std::cout << SHOW_CURSOR << RESET;
    return 0;
}
