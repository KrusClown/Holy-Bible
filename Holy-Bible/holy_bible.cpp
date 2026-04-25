/*
 ============================================================
   THE HOLY BIBLE - Complete Interactive Console Experience
   ASCII Art · Colors · Animations · All Psalms · Versicles
   Compile: g++ -std=c++17 -o holy_bible holy_bible.cpp
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
#include <ctime>

// ─── ANSI Color & Style Codes ─────────────────────────────
#define RESET           "\033[0m"
#define BOLD            "\033[1m"
#define DIM             "\033[2m"
#define ITALIC          "\033[3m"
#define UNDERLINE       "\033[4m"

#define BLACK           "\033[30m"
#define RED             "\033[31m"
#define GREEN           "\033[32m"
#define YELLOW          "\033[33m"
#define BLUE            "\033[34m"
#define MAGENTA         "\033[35m"
#define CYAN            "\033[36m"
#define WHITE           "\033[37m"

#define BRIGHT_RED      "\033[91m"
#define BRIGHT_GREEN    "\033[92m"
#define BRIGHT_YELLOW   "\033[93m"
#define BRIGHT_BLUE     "\033[94m"
#define BRIGHT_MAGENTA  "\033[95m"
#define BRIGHT_CYAN     "\033[96m"
#define BRIGHT_WHITE    "\033[97m"

#define CLEAR_SCREEN    "\033[2J\033[H"
#define HIDE_CURSOR     "\033[?25l"
#define SHOW_CURSOR     "\033[?25h"

// ─── Utilities ────────────────────────────────────────────
void sleep_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
void clearScreen() { std::cout << CLEAR_SCREEN; std::cout.flush(); }

void typewrite(const std::string& text, int delay_ms = 25, const std::string& color = "") {
    for (char c : text) {
        if (!color.empty()) std::cout << color;
        std::cout << c << RESET;
        std::cout.flush();
        sleep_ms(delay_ms);
    }
}

void printLine(const std::string& text, const std::string& color = WHITE, bool bold = false) {
    if (bold) std::cout << BOLD;
    std::cout << color << text << RESET << "\n";
}

void printRule(int width = 72, const std::string& color = YELLOW) {
    std::cout << BOLD << color;
    for (int i = 0; i < width; ++i) std::cout << "=";
    std::cout << RESET << "\n";
}

void animatedRule(int width = 72, const std::string& color = YELLOW, int delay = 8) {
    std::cout << BOLD << color;
    for (int i = 0; i < width; ++i) { std::cout << "="; std::cout.flush(); sleep_ms(delay); }
    std::cout << RESET << "\n";
}

void spinLoader(const std::string& msg, int duration_ms = 1500) {
    const std::string frames = "|/-\\";
    int iters = duration_ms / 80;
    std::cout << HIDE_CURSOR;
    for (int i = 0; i < iters; ++i) {
        std::cout << "\r" << BRIGHT_YELLOW << BOLD << frames[i%4] << " " << msg << "  " << RESET;
        std::cout.flush(); sleep_ms(80);
    }
    std::cout << "\r" << BRIGHT_GREEN << BOLD << "* " << msg << " -- Amen!\n" << RESET;
    std::cout << SHOW_CURSOR;
}

std::vector<std::string> wordWrap(const std::string& text, int max_width) {
    std::vector<std::string> lines;
    std::istringstream iss(text);
    std::string word, line;
    while (iss >> word) {
        if (!line.empty() && (int)(line.size() + 1 + word.size()) > max_width) {
            lines.push_back(line); line = word;
        } else {
            if (!line.empty()) line += " ";
            line += word;
        }
    }
    if (!line.empty()) lines.push_back(line);
    return lines;
}

void verseBox(const std::string& ref, const std::string& text,
              const std::string& frameColor = BRIGHT_YELLOW,
              const std::string& textColor  = BRIGHT_WHITE,
              int boxWidth = 62) {
    auto lines = wordWrap(text, boxWidth - 4);
    std::cout << BOLD << frameColor;
    std::cout << "  +" << std::string(boxWidth, '-') << "+\n";
    std::cout << "  |" << std::string(boxWidth, ' ') << "|\n" << RESET;
    int refPad = (boxWidth - (int)ref.size()) / 2;
    if (refPad < 0) refPad = 0;
    std::cout << BOLD << frameColor << "  |"
              << std::string(refPad, ' ') << BRIGHT_WHITE << ref
              << std::string(std::max(0, boxWidth - refPad - (int)ref.size()), ' ')
              << frameColor << "|\n"
              << "  |" << std::string(boxWidth, ' ') << "|\n"
              << "  +" << std::string(boxWidth, '-') << "+\n"
              << "  |" << std::string(boxWidth, ' ') << "|\n" << RESET;
    for (auto& ln : lines) {
        int pad = boxWidth - 2 - (int)ln.size();
        if (pad < 0) pad = 0;
        std::cout << BOLD << frameColor << "  | " << RESET
                  << ITALIC << textColor << ln
                  << std::string(pad, ' ')
                  << BOLD << frameColor << " |\n" << RESET;
        sleep_ms(35);
    }
    std::cout << BOLD << frameColor
              << "  |" << std::string(boxWidth, ' ') << "|\n"
              << "  +" << std::string(boxWidth, '-') << "+\n" << RESET;
}

void sectionHeader(const std::string& title, const std::string& icon = "+") {
    std::cout << "\n"; printRule(72, YELLOW);
    std::cout << BOLD << BRIGHT_YELLOW << "  " << icon << "  ";
    typewrite(title, 16, BRIGHT_WHITE);
    std::cout << "  " << BRIGHT_YELLOW << icon << "\n" << RESET;
    printRule(72, YELLOW); std::cout << "\n";
}

void pressAnyKey() {
    std::cout << "\n" << DIM << CYAN << "  [ Press ENTER to continue... ]" << RESET;
    std::cin.ignore(10000, '\n');
    std::cin.get();
}

// ─── Star Field ───────────────────────────────────────────
void starField(int rows = 4, int cols = 72, int frames = 6) {
    std::cout << HIDE_CURSOR;
    const std::string starChars = "*+.~";
    const std::vector<std::string> colors = {BRIGHT_WHITE, BRIGHT_YELLOW, BRIGHT_CYAN, DIM WHITE};
    srand(42);
    for (int f = 0; f < frames; ++f) {
        if (f > 0) std::cout << "\033[" << rows << "A";
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                if (rand() % 9 == 0) std::cout << colors[rand()%4] << starChars[rand()%4] << RESET;
                else std::cout << " ";
            }
            std::cout << "\n";
        }
        std::cout.flush(); sleep_ms(180);
    }
    std::cout << SHOW_CURSOR;
}

// ─── Animated Flame ───────────────────────────────────────
void animateFlame(int seconds = 3) {
    const std::vector<std::string> ff = {
        "    (  )  \n   ( :: ) \n  ( :::  )\n  | ::: |\n  |/\\/\\/ |\n  `-----'",
        "    ( )   \n   (  :)  \n  (  ::  )\n  | ::: |\n  |/\\/\\/ |\n  `-----'",
        "   (   )  \n  ( :::)  \n (  :::  )\n  | ::: |\n  |/\\/\\/ |\n  `-----'",
    };
    std::cout << HIDE_CURSOR;
    for (int t = 0; t < seconds*6; ++t) {
        if (t > 0) std::cout << "\033[6A";
        const std::string c = (t%3==0)?BRIGHT_YELLOW:(t%3==1)?BRIGHT_RED:YELLOW;
        std::cout << c << BOLD << ff[t%3] << RESET << "\n";
        std::cout.flush(); sleep_ms(155);
    }
    std::cout << SHOW_CURSOR;
}

// ─── Cross Art ────────────────────────────────────────────
void drawCross() {
    const std::vector<std::string> c = {
        "               +===========+               ",
        "               |  +  +  +  |               ",
        "               |           |               ",
        "+==============+===========+==============+",
        "|   * * * * *  |           |  * * * * *   |",
        "+==============+===========+==============+",
        "               |           |               ",
        "               |  I N R I  |               ",
        "               |           |               ",
        "               +===========+               ",
    };
    const std::vector<std::string> cols = {
        BRIGHT_YELLOW,YELLOW,BRIGHT_YELLOW,YELLOW,
        BRIGHT_WHITE,YELLOW,BRIGHT_YELLOW,BRIGHT_CYAN,
        BRIGHT_YELLOW,YELLOW
    };
    for (size_t i = 0; i < c.size(); ++i) {
        std::cout << BOLD << cols[i] << "          " << c[i] << RESET << "\n";
        sleep_ms(40);
    }
}

// ─── Bible Title ──────────────────────────────────────────
void drawBibleTitle() {
    const std::vector<std::string> art = {
        "  ######  ######  ######  #      ######",
        "  #    #  #    #  #    #  #      #     ",
        "  ######  #    #  ######  #      ####  ",
        "  #    #  #    #  #    #  #      #     ",
        "  ######  ######  ######  ######  ######",
    };
    const std::vector<std::string> gr = {BRIGHT_YELLOW,YELLOW,BRIGHT_WHITE,YELLOW,BRIGHT_YELLOW};
    for (size_t i = 0; i < art.size(); ++i)
        typewrite(art[i]+"\n", 5, gr[i%gr.size()]);
}

// ─── Opening Ceremony ─────────────────────────────────────
void openingCeremony() {
    clearScreen();
    std::cout << HIDE_CURSOR << "\n";
    starField(4, 72, 5);
    std::cout << BRIGHT_WHITE << BOLD;
    std::cout << "               ___     _,--'.`-.    \n";
    std::cout << "           .-'   `--=/     `    \\   \n";
    std::cout << "          /        /            |   \n";
    std::cout << "         |   , .__/ `._,  \\     |   \n";
    std::cout << "          \\  `'         `--'`--'/   \n";
    std::cout << "           `.     ~ Holy Spirit ~  .'\n";
    std::cout << "             `-._          _.-'     \n";
    std::cout << "                  `------''          \n" << RESET;
    sleep_ms(400);
    animatedRule(72, YELLOW, 6);
    drawBibleTitle();
    animatedRule(72, YELLOW, 6);
    std::cout << "\n";
    printLine("    ** Complete Holy Bible Explorer -- King James Version **\n", BRIGHT_CYAN, true);
    spinLoader("Opening the Sacred Scriptures", 1600);
    sleep_ms(400);
    std::cout << SHOW_CURSOR;
}

// ==========================================================
//                        DATA
// ==========================================================

// ─── Psalm struct ─────────────────────────────────────────
struct Psalm {
    int number;
    std::string title, keyVerse, theme;
    std::vector<std::string> fullText;
};

// ─── ALL 150 Psalms (key verses + selected full text) ─────
std::vector<Psalm> allPsalms = {
{1,"The Two Ways","Blessed is the man that walketh not in the counsel of the ungodly.","Righteousness",
{"v1  Blessed is the man that walketh not in the counsel of the ungodly, nor standeth in the way of sinners.",
 "v2  But his delight is in the law of the LORD; and in his law doth he meditate day and night.",
 "v3  And he shall be like a tree planted by the rivers of water, that bringeth forth his fruit in his season.",
 "v6  For the LORD knoweth the way of the righteous: but the way of the ungodly shall perish."}},
{2,"The LORD's Anointed","Blessed are all they that put their trust in him.","Messianic",
{"v1  Why do the heathen rage, and the people imagine a vain thing?",
 "v6  Yet have I set my king upon my holy hill of Zion.",
 "v7  The LORD hath said unto me, Thou art my Son; this day have I begotten thee.",
 "v12 Kiss the Son, lest he be angry. Blessed are all they that put their trust in him."}},
{3,"Morning Prayer","But thou, O LORD, art a shield for me; my glory, and the lifter up of mine head.","Trust",
{"v1  LORD, how are they increased that trouble me! Many are they that rise up against me.",
 "v3  But thou, O LORD, art a shield for me; my glory, and the lifter up of mine head.",
 "v5  I laid me down and slept; I awaked; for the LORD sustained me.",
 "v8  Salvation belongeth unto the LORD: thy blessing is upon thy people."}},
{4,"Evening Prayer","The LORD will hear when I call unto him.","Peace",
{"v1  Hear me when I call, O God of my righteousness.",
 "v4  Stand in awe, and sin not: commune with your own heart upon your bed, and be still.",
 "v7  Thou hast put gladness in my heart, more than in the time that their corn and wine increased.",
 "v8  I will both lay me down in peace, and sleep: for thou, LORD, only makest me dwell in safety."}},
{5,"Morning Cry","Lead me, O LORD, in thy righteousness.","Guidance",
{"v1  Give ear to my words, O LORD, consider my meditation.",
 "v3  My voice shalt thou hear in the morning, O LORD; in the morning will I direct my prayer.",
 "v8  Lead me, O LORD, in thy righteousness because of mine enemies; make thy way straight.",
 "v12 For thou, LORD, wilt bless the righteous; with favour wilt thou compass him as with a shield."}},
{6,"Prayer in Distress","O LORD, heal me; for my bones are vexed.","Healing",
{"v1  O LORD, rebuke me not in thine anger, neither chasten me in thy hot displeasure.",
 "v2  Have mercy upon me, O LORD; for I am weak: O LORD, heal me; for my bones are vexed.",
 "v4  Return, O LORD, deliver my soul: oh save me for thy mercies' sake.",
 "v9  The LORD hath heard my supplication; the LORD will receive my prayer."}},
{7,"Trust in God's Justice","In thee do I put my trust.","Justice",
{"v1  O LORD my God, in thee do I put my trust: save me from all them that persecute me.",
 "v9  Oh let the wickedness of the wicked come to an end; but establish the just.",
 "v10 My defence is of God, which saveth the upright in heart.",
 "v17 I will praise the LORD according to his righteousness."}},
{8,"God's Glory and Man's Dignity","O LORD our Lord, how excellent is thy name in all the earth!","Creation",
{"v1  O LORD our Lord, how excellent is thy name in all the earth!",
 "v3  When I consider thy heavens, the work of thy fingers, the moon and the stars.",
 "v4  What is man, that thou art mindful of him? and the son of man, that thou visitest him?",
 "v5  For thou hast made him a little lower than the angels, and crowned him with glory and honour."}},
{9,"Praise for Judgment","I will praise thee, O LORD, with my whole heart.","Praise",
{"v1  I will praise thee, O LORD, with my whole heart; I will shew forth all thy marvellous works.",
 "v9  The LORD also will be a refuge for the oppressed, a refuge in times of trouble.",
 "v10 And they that know thy name will put their trust in thee.",
 "v18 For the needy shall not alway be forgotten: the expectation of the poor shall not perish."}},
{10,"Prayer Against the Wicked","LORD, thou hast heard the desire of the humble.","Justice",
{"v1  Why standest thou afar off, O LORD? why hidest thou thyself in times of trouble?",
 "v12 Arise, O LORD; O God, lift up thine hand: forget not the humble.",
 "v17 LORD, thou hast heard the desire of the humble: thou wilt prepare their heart.",
 "v18 To judge the fatherless and the oppressed, that the man of the earth may no more oppress."}},
{11,"Trust in the LORD","The LORD is in his holy temple, the LORD's throne is in heaven.","Refuge",
{"v1  In the LORD put I my trust: how say ye to my soul, Flee as a bird to your mountain?",
 "v4  The LORD is in his holy temple, the LORD's throne is in heaven.",
 "v5  The LORD trieth the righteous: but the wicked and him that loveth violence his soul hateth.",
 "v7  For the righteous LORD loveth righteousness; his countenance doth behold the upright."}},
{12,"Help, LORD","The words of the LORD are pure words.","Purity",
{"v1  Help, LORD; for the godly man ceaseth; for the faithful fail from among the children of men.",
 "v5  For the oppression of the poor, for the sighing of the needy, now will I arise, saith the LORD.",
 "v6  The words of the LORD are pure words: as silver tried in a furnace of earth, purified seven times.",
 "v7  Thou shalt keep them, O LORD, thou shalt preserve them from this generation for ever."}},
{13,"How Long, O LORD?","I will sing unto the LORD, because he hath dealt bountifully with me.","Faith",
{"v1  How long wilt thou forget me, O LORD? for ever? how long wilt thou hide thy face from me?",
 "v3  Consider and hear me, O LORD my God: lighten mine eyes, lest I sleep the sleep of death.",
 "v5  But I have trusted in thy mercy; my heart shall rejoice in thy salvation.",
 "v6  I will sing unto the LORD, because he hath dealt bountifully with me."}},
{14,"The Fool","The LORD looked down from heaven upon the children of men.","Wisdom",
{"v1  The fool hath said in his heart, There is no God. They are corrupt.",
 "v2  The LORD looked down from heaven upon the children of men, to see if there were any that did understand.",
 "v3  They are all gone aside, they are all together become filthy: there is none that doeth good, no, not one.",
 "v7  Oh that the salvation of Israel were come out of Zion!"}},
{15,"Who May Dwell with God?","He that doeth these things shall never be moved.","Holiness",
{"v1  LORD, who shall abide in thy tabernacle? who shall dwell in thy holy hill?",
 "v2  He that walketh uprightly, and worketh righteousness, and speaketh the truth in his heart.",
 "v3  He that backbiteth not with his tongue, nor doeth evil to his neighbour.",
 "v5  He that doeth these things shall never be moved."}},
{16,"Fullness of Joy","In thy presence is fulness of joy.","Joy",
{"v1  Preserve me, O God: for in thee do I put my trust.",
 "v8  I have set the LORD always before me: because he is at my right hand, I shall not be moved.",
 "v10 For thou wilt not leave my soul in hell; neither wilt thou suffer thine Holy One to see corruption.",
 "v11 Thou wilt shew me the path of life: in thy presence is fulness of joy."}},
{17,"Prayer for Deliverance","Keep me as the apple of the eye, hide me under the shadow of thy wings.","Protection",
{"v1  Hear the right, O LORD, attend unto my cry; give ear unto my prayer.",
 "v7  Shew thy marvellous lovingkindness, O thou that savest by thy right hand.",
 "v8  Keep me as the apple of the eye, hide me under the shadow of thy wings.",
 "v15 As for me, I will behold thy face in righteousness: I shall be satisfied."}},
{18,"Praise for Deliverance","The LORD is my rock, and my fortress, and my deliverer.","Victory",
{"v1  I will love thee, O LORD, my strength.",
 "v2  The LORD is my rock, and my fortress, and my deliverer; my God, my strength, in whom I will trust.",
 "v6  In my distress I called upon the LORD, and cried unto my God: he heard my voice out of his temple.",
 "v46 The LORD liveth; and blessed be my rock; and let the God of my salvation be exalted."}},
{19,"The Heavens Declare","The heavens declare the glory of God.","Creation",
{"v1  The heavens declare the glory of God; and the firmament sheweth his handywork.",
 "v2  Day unto day uttereth speech, and night unto night sheweth knowledge.",
 "v7  The law of the LORD is perfect, converting the soul: the testimony of the LORD is sure.",
 "v14 Let the words of my mouth, and the meditation of my heart, be acceptable in thy sight, O LORD."}},
{20,"Prayer for the King","The LORD hear thee in the day of trouble.","Prayer",
{"v1  The LORD hear thee in the day of trouble; the name of the God of Jacob defend thee.",
 "v5  We will rejoice in thy salvation, and in the name of our God we will set up our banners.",
 "v7  Some trust in chariots, and some in horses: but we will remember the name of the LORD our God.",
 "v9  Save, LORD: let the king hear us when we call."}},
{21,"Joy in God's Strength","The king shall joy in thy strength, O LORD.","Strength",
{"v1  The king shall joy in thy strength, O LORD; and in thy salvation how greatly shall he rejoice!",
 "v4  He asked life of thee, and thou gavest it him, even length of days for ever and ever.",
 "v7  For the king trusteth in the LORD, and through the mercy of the most High he shall not be moved.",
 "v13 Be thou exalted, LORD, in thine own strength: so will we sing and praise thy power."}},
{22,"Cry of Desolation","My God, my God, why hast thou forsaken me?","Suffering",
{"v1  My God, my God, why hast thou forsaken me? why art thou so far from helping me?",
 "v3  But thou art holy, O thou that inhabitest the praises of Israel.",
 "v18 They part my garments among them, and cast lots upon my vesture.",
 "v31 They shall come, and shall declare his righteousness unto a people that shall be born."}},
{23,"The Good Shepherd","The LORD is my shepherd; I shall not want.","Comfort",
{"v1  The LORD is my shepherd; I shall not want.",
 "v2  He maketh me to lie down in green pastures: he leadeth me beside the still waters.",
 "v3  He restoreth my soul: he leadeth me in the paths of righteousness for his name's sake.",
 "v4  Yea, though I walk through the valley of the shadow of death, I will fear no evil: for thou art with me.",
 "v5  Thou preparest a table before me in the presence of mine enemies.",
 "v6  Surely goodness and mercy shall follow me all the days of my life."}},
{24,"The King of Glory","Who is this King of glory? The LORD strong and mighty.","Majesty",
{"v1  The earth is the LORD's, and the fulness thereof; the world, and they that dwell therein.",
 "v3  Who shall ascend into the hill of the LORD? or who shall stand in his holy place?",
 "v4  He that hath clean hands, and a pure heart.",
 "v7  Lift up your heads, O ye gates; and the King of glory shall come in."}},
{25,"Trust and Guidance","Lead me in thy truth, and teach me.","Guidance",
{"v1  Unto thee, O LORD, do I lift up my soul.",
 "v4  Shew me thy ways, O LORD; teach me thy paths.",
 "v5  Lead me in thy truth, and teach me: for thou art the God of my salvation.",
 "v14 The secret of the LORD is with them that fear him; and he will shew them his covenant."}},
{26,"Walking in Integrity","I have walked in mine integrity.","Integrity",
{"v1  Judge me, O LORD; for I have walked in mine integrity: I have trusted also in the LORD.",
 "v3  For thy lovingkindness is before mine eyes: and I have walked in thy truth.",
 "v8  LORD, I have loved the habitation of thy house, and the place where thine honour dwelleth.",
 "v12 My foot standeth in an even place: in the congregations will I bless the LORD."}},
{27,"The LORD Is My Light","The LORD is my light and my salvation; whom shall I fear?","Courage",
{"v1  The LORD is my light and my salvation; whom shall I fear? the LORD is the strength of my life.",
 "v4  One thing have I desired of the LORD, that I may dwell in the house of the LORD all the days of my life.",
 "v13 I had fainted, unless I had believed to see the goodness of the LORD in the land of the living.",
 "v14 Wait on the LORD: be of good courage, and he shall strengthen thine heart."}},
{28,"My Strength and Shield","The LORD is my strength and my shield; my heart trusted in him.","Strength",
{"v1  Unto thee will I cry, O LORD my rock; be not silent to me.",
 "v6  Blessed be the LORD, because he hath heard the voice of my supplications.",
 "v7  The LORD is my strength and my shield; my heart trusted in him, and I am helped.",
 "v9  Save thy people, and bless thine inheritance: feed them also, and lift them up for ever."}},
{29,"The Voice of the LORD","The LORD will give strength unto his people.","Power",
{"v1  Give unto the LORD, O ye mighty, give unto the LORD glory and strength.",
 "v3  The voice of the LORD is upon the waters: the God of glory thundereth.",
 "v4  The voice of the LORD is powerful; the voice of the LORD is full of majesty.",
 "v11 The LORD will give strength unto his people; the LORD will bless his people with peace."}},
{30,"Weeping and Joy","Weeping may endure for a night, but joy cometh in the morning.","Joy",
{"v2  O LORD my God, I cried unto thee, and thou hast healed me.",
 "v4  Sing unto the LORD, O ye saints of his, and give thanks at the remembrance of his holiness.",
 "v5  Weeping may endure for a night, but joy cometh in the morning.",
 "v11 Thou hast turned for me my mourning into dancing."}},
{31,"In Thee Do I Trust","Into thine hand I commit my spirit.","Trust",
{"v1  In thee, O LORD, do I put my trust; let me never be ashamed.",
 "v3  For thou art my rock and my fortress; therefore for thy name's sake lead me, and guide me.",
 "v5  Into thine hand I commit my spirit: thou hast redeemed me, O LORD God of truth.",
 "v24 Be of good courage, and he shall strengthen your heart, all ye that hope in the LORD."}},
{32,"Blessed Forgiveness","Blessed is he whose transgression is forgiven.","Forgiveness",
{"v1  Blessed is he whose transgression is forgiven, whose sin is covered.",
 "v2  Blessed is the man unto whom the LORD imputeth not iniquity, and in whose spirit there is no guile.",
 "v7  Thou art my hiding place; thou shalt preserve me from trouble.",
 "v8  I will instruct thee and teach thee in the way which thou shalt go."}},
{33,"Praise to the Creator","The earth is full of the goodness of the LORD.","Praise",
{"v4  For the word of the LORD is right; and all his works are done in truth.",
 "v5  He loveth righteousness and judgment: the earth is full of the goodness of the LORD.",
 "v6  By the word of the LORD were the heavens made; and all the host of them by the breath of his mouth.",
 "v18 Behold, the eye of the LORD is upon them that fear him, upon them that hope in his mercy."}},
{34,"Taste and See","O taste and see that the LORD is good.","Thanksgiving",
{"v1  I will bless the LORD at all times: his praise shall continually be in my mouth.",
 "v3  O magnify the LORD with me, and let us exalt his name together.",
 "v8  O taste and see that the LORD is good: blessed is the man that trusteth in him.",
 "v18 The LORD is nigh unto them that are of a broken heart; and saveth such as be of a contrite spirit."}},
{35,"Prayer for Help","Plead my cause, O LORD, with them that strive with me.","Deliverance",
{"v1  Plead my cause, O LORD, with them that strive with me: fight against them that fight against me.",
 "v9  And my soul shall be joyful in the LORD: it shall rejoice in his salvation.",
 "v18 I will give thee thanks in the great congregation: I will praise thee among much people.",
 "v27 Let them shout for joy, and be glad, that favour my righteous cause."}},
{36,"God's Lovingkindness","How excellent is thy lovingkindness, O God!","Love",
{"v5  Thy mercy, O LORD, is in the heavens; and thy faithfulness reacheth unto the clouds.",
 "v6  Thy righteousness is like the great mountains; thy judgments are a great deep.",
 "v7  How excellent is thy lovingkindness, O God! therefore the children of men put their trust under the shadow of thy wings.",
 "v9  For with thee is the fountain of life: in thy light shall we see light."}},
{37,"Trust in the LORD","Delight thyself also in the LORD.","Trust",
{"v1  Fret not thyself because of evildoers, neither be thou envious against the workers of iniquity.",
 "v3  Trust in the LORD, and do good; so shalt thou dwell in the land.",
 "v4  Delight thyself also in the LORD; and he shall give thee the desires of thine heart.",
 "v5  Commit thy way unto the LORD; trust also in him; and he shall bring it to pass."}},
{38,"Prayer of Confession","LORD, all my desire is before thee.","Penitence",
{"v1  O LORD, rebuke me not in thy wrath: neither chasten me in thy hot displeasure.",
 "v9  Lord, all my desire is before thee; and my groaning is not hid from thee.",
 "v15 For in thee, O LORD, do I hope: thou wilt hear, O Lord my God.",
 "v22 Make haste to help me, O Lord my salvation."}},
{39,"Life Is Brief","LORD, make me to know mine end, and the measure of my days.","Mortality",
{"v1  I said, I will take heed to my ways, that I sin not with my tongue.",
 "v4  LORD, make me to know mine end, and the measure of my days, what it is.",
 "v7  And now, Lord, what wait I for? my hope is in thee.",
 "v12 Hear my prayer, O LORD, and give ear unto my cry; hold not thy peace at my tears."}},
{40,"A New Song","He hath put a new song in my mouth, even praise unto our God.","Praise",
{"v1  I waited patiently for the LORD; and he inclined unto me, and heard my cry.",
 "v2  He brought me up also out of an horrible pit, out of the miry clay.",
 "v3  And he hath put a new song in my mouth, even praise unto our God.",
 "v5  Many, O LORD my God, are thy wonderful works which thou hast done."}},
{41,"Blessed Is He Who Cares","Blessed is he that considereth the poor.","Mercy",
{"v1  Blessed is he that considereth the poor: the LORD will deliver him in time of trouble.",
 "v3  The LORD will strengthen him upon the bed of languishing.",
 "v10 But thou, O LORD, be merciful unto me, and raise me up.",
 "v13 Blessed be the LORD God of Israel from everlasting, and to everlasting. Amen, and Amen."}},
{42,"As the Deer Pants","As the hart panteth after the water brooks, so panteth my soul after thee.","Longing",
{"v1  As the hart panteth after the water brooks, so panteth my soul after thee, O God.",
 "v2  My soul thirsteth for God, for the living God: when shall I come and appear before God?",
 "v5  Why art thou cast down, O my soul? hope thou in God: for I shall yet praise him.",
 "v8  Yet the LORD will command his lovingkindness in the daytime."}},
{43,"Send Thy Light","O send out thy light and thy truth: let them lead me.","Guidance",
{"v1  Judge me, O God, and plead my cause against an ungodly nation.",
 "v3  O send out thy light and thy truth: let them lead me; let them bring me unto thy holy hill.",
 "v4  Then will I go unto the altar of God, unto God my exceeding joy.",
 "v5  Why art thou cast down, O my soul? hope in God: for I shall yet praise him."}},
{44,"We Have Heard with Our Ears","Help us, O God of our salvation.","Faith",
{"v1  We have heard with our ears, O God, our fathers have told us, what work thou didst in their days.",
 "v7  But thou hast saved us from our enemies, and hast put them to shame that hated us.",
 "v8  In God we boast all the day long, and praise thy name for ever.",
 "v26 Arise for our help, and redeem us for thy mercies' sake."}},
{45,"The King's Wedding","Thy throne, O God, is for ever and ever.","Messianic",
{"v2  Thou art fairer than the children of men: grace is poured into thy lips.",
 "v6  Thy throne, O God, is for ever and ever: the sceptre of thy kingdom is a right sceptre.",
 "v7  Thou lovest righteousness, and hatest wickedness: therefore God hath anointed thee.",
 "v17 I will make thy name to be remembered in all generations."}},
{46,"God Our Refuge","God is our refuge and strength, a very present help in trouble.","Refuge",
{"v1  God is our refuge and strength, a very present help in trouble.",
 "v2  Therefore will not we fear, though the earth be removed.",
 "v10 Be still, and know that I am God: I will be exalted among the heathen.",
 "v11 The LORD of hosts is with us; the God of Jacob is our refuge."}},
{47,"Clap Your Hands","God is the King of all the earth: sing ye praises.","Joy",
{"v1  O clap your hands, all ye people; shout unto God with the voice of triumph.",
 "v2  For the LORD most high is terrible; he is a great King over all the earth.",
 "v7  For God is the King of all the earth: sing ye praises with understanding.",
 "v8  God reigneth over the heathen: God sitteth upon the throne of his holiness."}},
{48,"The City of God","Great is the LORD, and greatly to be praised.","Worship",
{"v1  Great is the LORD, and greatly to be praised in the city of our God.",
 "v9  We have thought of thy lovingkindness, O God, in the midst of thy temple.",
 "v10 According to thy name, O God, so is thy praise unto the ends of the earth.",
 "v14 For this God is our God for ever and ever: he will be our guide even unto death."}},
{49,"Do Not Fear the Rich","God will redeem my soul from the power of the grave.","Wisdom",
{"v3  My mouth shall speak of wisdom; and the meditation of my heart shall be of understanding.",
 "v12 Nevertheless man being in honour abideth not: he is like the beasts that perish.",
 "v15 But God will redeem my soul from the power of the grave: for he shall receive me.",
 "v20 Man that is in honour, and understandeth not, is like the beasts that perish."}},
{50,"God the Judge Speaks","Offer unto God thanksgiving.","Worship",
{"v1  The mighty God, even the LORD, hath spoken, and called the earth from the rising of the sun unto the going down thereof.",
 "v14 Offer unto God thanksgiving; and pay thy vows unto the most High.",
 "v15 And call upon me in the day of trouble: I will deliver thee, and thou shalt glorify me.",
 "v23 Whoso offereth praise glorifieth me: and to him that ordereth his conversation aright will I shew the salvation of God."}},
{51,"Create in Me a Clean Heart","Create in me a clean heart, O God.","Repentance",
{"v1  Have mercy upon me, O God, according to thy lovingkindness.",
 "v2  Wash me throughly from mine iniquity, and cleanse me from my sin.",
 "v7  Purge me with hyssop, and I shall be clean: wash me, and I shall be whiter than snow.",
 "v10 Create in me a clean heart, O God; and renew a right spirit within me.",
 "v17 The sacrifices of God are a broken spirit: a broken and a contrite heart, O God, thou wilt not despise."}},
{52,"Trust in God's Mercy","I trust in the mercy of God for ever and ever.","Trust",
{"v1  Why boastest thou thyself in mischief, O mighty man? the goodness of God endureth continually.",
 "v7  Lo, this is the man that made not God his strength; but trusted in the abundance of his riches.",
 "v8  But I am like a green olive tree in the house of God: I trust in the mercy of God for ever.",
 "v9  I will praise thee for ever, because thou hast done it."}},
{53,"There Is None That Doeth Good","God looked down from heaven upon the children of men.","Sin",
{"v1  The fool hath said in his heart, There is no God.",
 "v2  God looked down from heaven upon the children of men, to see if there were any that did understand.",
 "v3  Every one of them is gone back: they are altogether become filthy.",
 "v6  Oh that the salvation of Israel were come out of Zion!"}},
{54,"Save Me, O God","Behold, God is mine helper.","Deliverance",
{"v1  Save me, O God, by thy name, and judge me by thy strength.",
 "v2  Hear my prayer, O God; give ear to the words of my mouth.",
 "v4  Behold, God is mine helper: the Lord is with them that uphold my soul.",
 "v6  I will freely sacrifice unto thee: I will praise thy name, O LORD; for it is good."}},
{55,"Cast Thy Burden","Cast thy burden upon the LORD, and he shall sustain thee.","Comfort",
{"v1  Give ear to my prayer, O God; and hide not thyself from my supplication.",
 "v17 Evening, and morning, and at noon, will I pray, and cry aloud: and he shall hear my voice.",
 "v22 Cast thy burden upon the LORD, and he shall sustain thee: he shall never suffer the righteous to be moved.",
 "v23 But thou, O God, shalt bring them down into the pit of destruction."}},
{56,"When I Am Afraid","What time I am afraid, I will trust in thee.","Fear",
{"v3  What time I am afraid, I will trust in thee.",
 "v4  In God I will praise his word, in God I have put my trust; I will not fear what flesh can do unto me.",
 "v8  Thou tellest my wanderings: put thou my tears into thy bottle: are they not in thy book?",
 "v13 For thou hast delivered my soul from death: wilt not thou deliver my feet from falling?"}},
{57,"In the Shadow of Thy Wings","Thy mercy is great unto the heavens.","Refuge",
{"v1  Be merciful unto me, O God, be merciful unto me: for my soul trusteth in thee.",
 "v2  I will cry unto God most high; unto God that performeth all things for me.",
 "v5  Be thou exalted, O God, above the heavens; let thy glory be above all the earth.",
 "v10 For thy mercy is great unto the heavens, and thy truth unto the clouds."}},
{58,"God Judges the Earth","Verily there is a God that judgeth in the earth.","Justice",
{"v1  Do ye indeed speak righteousness, O congregation? do ye judge uprightly?",
 "v6  Break their teeth, O God, in their mouth: break out the great teeth of the young lions.",
 "v11 So that a man shall say, Verily there is a reward for the righteous.",
 "v11 Verily he is a God that judgeth in the earth."}},
{59,"Deliver Me from Mine Enemies","But I will sing of thy power.","Victory",
{"v1  Deliver me from mine enemies, O my God: defend me from them that rise up against me.",
 "v9  Because of his strength will I wait upon thee: for God is my defence.",
 "v16 But I will sing of thy power; yea, I will sing aloud of thy mercy in the morning.",
 "v17 Unto thee, O my strength, will I sing: for God is my defence, and the God of my mercy."}},
{60,"O God, Thou Hast Cast Us Off","Give us help from trouble: for vain is the help of man.","Hope",
{"v1  O God, thou hast cast us off, thou hast scattered us, thou hast been displeased.",
 "v4  Thou hast given a banner to them that fear thee, that it may be displayed because of the truth.",
 "v11 Give us help from trouble: for vain is the help of man.",
 "v12 Through God we shall do valiantly: for he it is that shall tread down our enemies."}},
{61,"From the End of the Earth","Lead me to the rock that is higher than I.","Refuge",
{"v1  Hear my cry, O God; attend unto my prayer.",
 "v2  From the end of the earth will I cry unto thee, when my heart is overwhelmed: lead me to the rock that is higher than I.",
 "v3  For thou hast been a shelter for me, and a strong tower from the enemy.",
 "v8  So will I sing praise unto thy name for ever."}},
{62,"My Soul Waits for God","My soul, wait thou only upon God; for my expectation is from him.","Rest",
{"v1  Truly my soul waiteth upon God: from him cometh my salvation.",
 "v5  My soul, wait thou only upon God; for my expectation is from him.",
 "v7  In God is my salvation and my glory: the rock of my strength, and my refuge, is in God.",
 "v8  Trust in him at all times; ye people, pour out your heart before him: God is a refuge for us."}},
{63,"Thirsting for God","My soul thirsteth for thee, my flesh longeth for thee.","Longing",
{"v1  O God, thou art my God; early will I seek thee: my soul thirsteth for thee, my flesh longeth for thee.",
 "v3  Because thy lovingkindness is better than life, my lips shall praise thee.",
 "v4  Thus will I bless thee while I live: I will lift up my hands in thy name.",
 "v7  Because thou hast been my help, therefore in the shadow of thy wings will I rejoice."}},
{64,"Hide Me from the Secret Counsel","The righteous shall be glad in the LORD.","Protection",
{"v1  Hear my voice, O God, in my prayer: preserve my life from fear of the enemy.",
 "v9  And all men shall fear, and shall declare the work of God; for they shall wisely consider of his doing.",
 "v10 The righteous shall be glad in the LORD, and shall trust in him.",
 "v10 And all the upright in heart shall glory."}},
{65,"God's Provision","Thou crownest the year with thy goodness.","Harvest",
{"v1  Praise waiteth for thee, O God, in Sion: and unto thee shall the vow be performed.",
 "v4  Blessed is the man whom thou choosest, and causest to approach unto thee.",
 "v9  Thou visitest the earth, and waterest it: thou greatly enrichest it.",
 "v11 Thou crownest the year with thy goodness; and thy paths drop fatness."}},
{66,"Shout for Joy","Come and hear, all ye that fear God, and I will declare what he hath done for my soul.","Praise",
{"v1  Make a joyful noise unto God, all ye lands.",
 "v5  Come and see the works of God: he is terrible in his doing toward the children of men.",
 "v16 Come and hear, all ye that fear God, and I will declare what he hath done for my soul.",
 "v20 Blessed be God, which hath not turned away my prayer, nor his mercy from me."}},
{67,"God Be Merciful","Let all the people praise thee.","Mission",
{"v1  God be merciful unto us, and bless us; and cause his face to shine upon us.",
 "v2  That thy way may be known upon earth, thy saving health among all nations.",
 "v5  Let the people praise thee, O God; let all the people praise thee.",
 "v7  God shall bless us; and all the ends of the earth shall fear him."}},
{68,"Let God Arise","Blessed be the Lord, who daily loadeth us with benefits.","Victory",
{"v1  Let God arise, let his enemies be scattered.",
 "v4  Sing unto God, sing praises to his name: extol him that rideth upon the heavens.",
 "v19 Blessed be the Lord, who daily loadeth us with benefits, even the God of our salvation.",
 "v35 O God, thou art terrible out of thy holy places: the God of Israel is he that giveth strength."}},
{69,"Save Me, O God","They that seek thee shall praise the LORD.","Suffering",
{"v1  Save me, O God; for the waters are come in unto my soul.",
 "v13 But as for me, my prayer is unto thee, O LORD, in an acceptable time.",
 "v32 The humble shall see this, and be glad: and your heart shall live that seek God.",
 "v34 Let the heaven and earth praise him, the seas, and every thing that moveth therein."}},
{70,"Make Haste, O God","Make haste to help me, O LORD.","Urgency",
{"v1  Make haste, O God, to deliver me; make haste to help me, O LORD.",
 "v4  Let all those that seek thee rejoice and be glad in thee.",
 "v5  But I am poor and needy: make haste unto me, O God.",
 "v5  Thou art my help and my deliverer; O LORD, make no tarrying."}},
{71,"In Thee Do I Take Refuge","I will hope continually, and will yet praise thee more and more.","Hope",
{"v1  In thee, O LORD, do I put my trust: let me never be put to confusion.",
 "v5  For thou art my hope, O Lord GOD: thou art my trust from my youth.",
 "v14 But I will hope continually, and will yet praise thee more and more.",
 "v18 Now also when I am old and greyheaded, O God, forsake me not."}},
{72,"The Righteous King","His name shall endure for ever.","Messianic",
{"v1  Give the king thy judgments, O God, and thy righteousness unto the king's son.",
 "v11 Yea, all kings shall fall down before him: all nations shall serve him.",
 "v17 His name shall endure for ever: his name shall be continued as long as the sun.",
 "v19 And blessed be his glorious name for ever: and let the whole earth be filled with his glory."}},
{73,"God Is Good to Israel","God is the strength of my heart, and my portion for ever.","Faith",
{"v1  Truly God is good to Israel, even to such as are of a clean heart.",
 "v16 When I thought to know this, it was too painful for me.",
 "v17 Until I went into the sanctuary of God; then understood I their end.",
 "v25 Whom have I in heaven but thee? and there is none upon earth that I desire beside thee.",
 "v26 My flesh and my heart faileth: but God is the strength of my heart."}},
{74,"O God, Why Hast Thou Cast Us Off?","Arise, O God, plead thine own cause.","Lament",
{"v1  O God, why hast thou cast us off for ever?",
 "v12 For God is my King of old, working salvation in the midst of the earth.",
 "v20 Have respect unto the covenant: for the dark places of the earth are full of habitations of cruelty.",
 "v22 Arise, O God, plead thine own cause: remember how the foolish man reproacheth thee daily."}},
{75,"Praise for God's Justice","At the set time I will judge uprightly.","Justice",
{"v1  Unto thee, O God, do we give thanks: for that thy name is near thy wondrous works declare.",
 "v6  For promotion cometh neither from the east, nor from the west, nor from the south.",
 "v7  But God is the judge: he putteth down one, and setteth up another.",
 "v9  But I will declare for ever; I will sing praises to the God of Jacob."}},
{76,"God Is Known in Judah","God of Jacob, the horse and chariot are cast into a dead sleep.","Majesty",
{"v1  In Judah is God known: his name is great in Israel.",
 "v4  Thou art more glorious and excellent than the mountains of prey.",
 "v9  When God arose to judgment, to save all the meek of the earth.",
 "v11 Vow, and pay unto the LORD your God: let all that be round about him bring presents."}},
{77,"I Cried Unto God","Thy way, O God, is in the sanctuary.","Faith",
{"v1  I cried unto God with my voice, even unto God with my voice; and he gave ear unto me.",
 "v11 I will remember the works of the LORD: surely I will remember thy wonders of old.",
 "v13 Thy way, O God, is in the sanctuary: who is so great a God as our God?",
 "v14 Thou art the God that doest wonders: thou hast declared thy strength among the people."}},
{78,"Teaching Future Generations","We will not hide them from their children.","History",
{"v2  I will open my mouth in a parable: I will utter dark sayings of old.",
 "v4  We will not hide them from their children, shewing to the generation to come the praises of the LORD.",
 "v7  That they might set their hope in God, and not forget the works of God.",
 "v72 So he fed them according to the integrity of his heart; and guided them by the skilfulness of his hands."}},
{79,"O God, the Heathen Are Come","Help us, O God of our salvation, for the glory of thy name.","Lament",
{"v1  O God, the heathen are come into thine inheritance; thy holy temple have they defiled.",
 "v5  How long, LORD? wilt thou be angry for ever?",
 "v9  Help us, O God of our salvation, for the glory of thy name: and deliver us.",
 "v13 So we thy people and sheep of thy pasture will give thee thanks for ever."}},
{80,"Restore Us, O God","Turn us again, O God, and cause thy face to shine; and we shall be saved.","Restoration",
{"v1  Give ear, O Shepherd of Israel, thou that leadest Joseph like a flock.",
 "v3  Turn us again, O God, and cause thy face to shine; and we shall be saved.",
 "v7  Turn us again, O God of hosts, and cause thy face to shine; and we shall be saved.",
 "v19 Turn us again, O LORD God of hosts, cause thy face to shine; and we shall be saved."}},
{81,"Hear, O My People","Open thy mouth wide, and I will fill it.","Obedience",
{"v1  Sing aloud unto God our strength: make a joyful noise unto the God of Jacob.",
 "v7  Thou calledst in trouble, and I delivered thee.",
 "v10 I am the LORD thy God, which brought thee out of the land of Egypt: open thy mouth wide, and I will fill it.",
 "v13 Oh that my people had hearkened unto me, and Israel had walked in my ways!"}},
{82,"God Stands in the Assembly","Arise, O God, judge the earth.","Justice",
{"v1  God standeth in the congregation of the mighty; he judgeth among the gods.",
 "v3  Defend the poor and fatherless: do justice to the afflicted and needy.",
 "v4  Deliver the poor and needy: rid them out of the hand of the wicked.",
 "v8  Arise, O God, judge the earth: for thou shalt inherit all nations."}},
{83,"Do Not Keep Silent, O God","That men may know that thou, whose name alone is JEHOVAH.","Prayer",
{"v1  Keep not thou silence, O God: hold not thy peace, and be not still, O God.",
 "v13 O my God, make them like a wheel; as the stubble before the wind.",
 "v17 Let them be confounded and troubled for ever; yea, let them be put to shame.",
 "v18 That men may know that thou, whose name alone is JEHOVAH, art the most high over all the earth."}},
{84,"Longing for God's House","Blessed are they that dwell in thy house.","Worship",
{"v1  How amiable are thy tabernacles, O LORD of hosts!",
 "v2  My soul longeth, yea, even fainteth for the courts of the LORD.",
 "v10 For a day in thy courts is better than a thousand. I had rather be a doorkeeper in the house of my God.",
 "v11 For the LORD God is a sun and shield: the LORD will give grace and glory."}},
{85,"Restore Us Again","Wilt thou not revive us again: that thy people may rejoice in thee?","Revival",
{"v1  LORD, thou hast been favourable unto thy land: thou hast brought back the captivity of Jacob.",
 "v6  Wilt thou not revive us again: that thy people may rejoice in thee?",
 "v10 Mercy and truth are met together; righteousness and peace have kissed each other.",
 "v13 Righteousness shall go before him; and shall set us in the way of his steps."}},
{86,"Hear My Prayer, O LORD","There is none like unto thee, O Lord.","Prayer",
{"v1  Bow down thine ear, O LORD, hear me: for I am poor and needy.",
 "v5  For thou, Lord, art good, and ready to forgive; and plenteous in mercy.",
 "v10 For thou art great, and doest wondrous things: thou art God alone.",
 "v15 But thou, O Lord, art a God full of compassion, and gracious, longsuffering, and plenteous in mercy."}},
{87,"The City of God","This and that man was born in her.","Zion",
{"v1  His foundation is in the holy mountains.",
 "v2  The LORD loveth the gates of Zion more than all the dwellings of Jacob.",
 "v3  Glorious things are spoken of thee, O city of God.",
 "v7  As well the singers as the players on instruments shall be there."}},
{88,"A Cry in the Darkness","Let my prayer come before thee: incline thine ear unto my cry.","Lament",
{"v1  O LORD God of my salvation, I have cried day and night before thee.",
 "v2  Let my prayer come before thee: incline thine ear unto my cry.",
 "v9  LORD, I have called daily upon thee, I have stretched out my hands unto thee.",
 "v13 But unto thee have I cried, O LORD; and in the morning shall my prayer prevent thee."}},
{89,"God's Faithfulness to David","I will sing of the mercies of the LORD for ever.","Covenant",
{"v1  I will sing of the mercies of the LORD for ever: with my mouth will I make known thy faithfulness.",
 "v2  For I have said, Mercy shall be built up for ever: thy faithfulness shalt thou establish in the very heavens.",
 "v8  O LORD God of hosts, who is a strong LORD like unto thee?",
 "v14 Justice and judgment are the habitation of thy throne: mercy and truth shall go before thy face."}},
{90,"God's Eternity","Lord, thou hast been our dwelling place in all generations.","Eternity",
{"v1  Lord, thou hast been our dwelling place in all generations.",
 "v2  Before the mountains were brought forth, or ever thou hadst formed the earth — from everlasting to everlasting, thou art God.",
 "v10 The days of our years are threescore years and ten.",
 "v12 So teach us to number our days, that we may apply our hearts unto wisdom.",
 "v14 O satisfy us early with thy mercy; that we may rejoice and be glad all our days."}},
{91,"Shelter of the Most High","He that dwelleth in the secret place of the most High.","Protection",
{"v1  He that dwelleth in the secret place of the most High shall abide under the shadow of the Almighty.",
 "v2  I will say of the LORD, He is my refuge and my fortress: my God; in him will I trust.",
 "v4  He shall cover thee with his feathers, and under his wings shalt thou trust.",
 "v11 For he shall give his angels charge over thee, to keep thee in all thy ways.",
 "v15 He shall call upon me, and I will answer him: I will be with him in trouble; I will deliver him."}},
{92,"A Song for the Sabbath","It is a good thing to give thanks unto the LORD.","Worship",
{"v1  It is a good thing to give thanks unto the LORD, and to sing praises unto thy name, O most High.",
 "v4  For thou, LORD, hast made me glad through thy work: I will triumph in the works of thy hands.",
 "v12 The righteous shall flourish like the palm tree: he shall grow like a cedar in Lebanon.",
 "v15 To shew that the LORD is upright: he is my rock, and there is no unrighteousness in him."}},
{93,"The LORD Reigns","The LORD reigneth, he is clothed with majesty.","Sovereignty",
{"v1  The LORD reigneth, he is clothed with majesty; the LORD is clothed with strength.",
 "v2  Thy throne is established of old: thou art from everlasting.",
 "v4  The LORD on high is mightier than the noise of many waters.",
 "v5  Thy testimonies are very sure: holiness becometh thine house, O LORD, for ever."}},
{94,"The LORD Knows the Thoughts","Blessed is the man whom thou chastenest, O LORD.","Justice",
{"v1  O LORD God, to whom vengeance belongeth; O God, to whom vengeance belongeth, shew thyself.",
 "v9  He that planted the ear, shall he not hear? he that formed the eye, shall he not see?",
 "v12 Blessed is the man whom thou chastenest, O LORD, and teachest him out of thy law.",
 "v18 When I said, My foot slippeth; thy mercy, O LORD, held me up."}},
{95,"Come Let Us Worship","O come, let us worship and bow down.","Worship",
{"v1  O come, let us sing unto the LORD: let us make a joyful noise to the rock of our salvation.",
 "v3  For the LORD is a great God, and a great King above all gods.",
 "v6  O come, let us worship and bow down: let us kneel before the LORD our maker.",
 "v7  For he is our God; and we are the people of his pasture."}},
{96,"Sing a New Song","Sing unto the LORD a new song.","Praise",
{"v1  Sing unto the LORD a new song: sing unto the LORD, all the earth.",
 "v3  Declare his glory among the heathen, his wonders among all people.",
 "v9  O worship the LORD in the beauty of holiness: fear before him, all the earth.",
 "v13 For he cometh to judge the earth: he shall judge the world with righteousness."}},
{97,"The LORD Reigns in Righteousness","Rejoice in the LORD, ye righteous.","Sovereignty",
{"v1  The LORD reigneth; let the earth rejoice; let the multitude of isles be glad thereof.",
 "v2  Clouds and darkness are round about him: righteousness and judgment are the habitation of his throne.",
 "v9  For thou, LORD, art high above all the earth: thou art exalted far above all gods.",
 "v12 Rejoice in the LORD, ye righteous; and give thanks at the remembrance of his holiness."}},
{98,"Sing to the LORD","Make a joyful noise unto the LORD, all the earth.","Praise",
{"v1  O sing unto the LORD a new song; for he hath done marvellous things.",
 "v3  He hath remembered his mercy and his truth toward the house of Israel.",
 "v4  Make a joyful noise unto the LORD, all the earth: make a loud noise, and rejoice.",
 "v9  For he cometh to judge the earth: with righteousness shall he judge the world."}},
{99,"The LORD Is Holy","Exalt the LORD our God, and worship at his holy hill.","Holiness",
{"v1  The LORD reigneth; let the people tremble: he sitteth between the cherubims; let the earth be moved.",
 "v3  Let them praise thy great and terrible name; for it is holy.",
 "v5  Exalt ye the LORD our God, and worship at his footstool; for he is holy.",
 "v9  Exalt the LORD our God, and worship at his holy hill; for the LORD our God is holy."}},
{100,"Make a Joyful Noise","The LORD is good; his mercy is everlasting.","Joy",
{"v1  Make a joyful noise unto the LORD, all ye lands.",
 "v2  Serve the LORD with gladness: come before his presence with singing.",
 "v3  Know ye that the LORD he is God: it is he that hath made us, and not we ourselves.",
 "v4  Enter into his gates with thanksgiving, and into his courts with praise.",
 "v5  For the LORD is good; his mercy is everlasting; and his truth endureth to all generations."}},
{101,"A Psalm of Integrity","I will walk within my house with a perfect heart.","Holiness",
{"v1  I will sing of mercy and judgment: unto thee, O LORD, will I sing.",
 "v2  I will behave myself wisely in a perfect way.",
 "v3  I will set no wicked thing before mine eyes.",
 "v4  A froward heart shall depart from me: I will not know a wicked person."}},
{102,"A Prayer When Overwhelmed","Thou, O LORD, shalt endure for ever.","Lament",
{"v1  Hear my prayer, O LORD, and let my cry come unto thee.",
 "v12 But thou, O LORD, shalt endure for ever; and thy remembrance unto all generations.",
 "v17 He will regard the prayer of the destitute, and not despise their prayer.",
 "v27 But thou art the same, and thy years shall have no end."}},
{103,"Bless the LORD, O My Soul","The LORD is merciful and gracious, slow to anger.","Gratitude",
{"v1  Bless the LORD, O my soul: and all that is within me, bless his holy name.",
 "v3  Who forgiveth all thine iniquities; who healeth all thy diseases.",
 "v8  The LORD is merciful and gracious, slow to anger, and plenteous in mercy.",
 "v12 As far as the east is from the west, so far hath he removed our transgressions from us.",
 "v13 Like as a father pitieth his children, so the LORD pitieth them that fear him."}},
{104,"The LORD Who Created All","O LORD my God, thou art very great.","Creation",
{"v1  Bless the LORD, O my soul. O LORD my God, thou art very great; thou art clothed with honour.",
 "v14 He causeth the grass to grow for the cattle, and herb for the service of man.",
 "v24 O LORD, how manifold are thy works! in wisdom hast thou made them all.",
 "v33 I will sing unto the LORD as long as I live: I will sing praise to my God while I have my being."}},
{105,"Remember His Wonderful Works","He remembered his holy promise.","History",
{"v1  O give thanks unto the LORD; call upon his name: make known his deeds among the people.",
 "v2  Sing unto him, sing psalms unto him: talk ye of all his wondrous works.",
 "v8  He hath remembered his covenant for ever, the word which he commanded to a thousand generations.",
 "v42 For he remembered his holy promise, and Abraham his servant."}},
{106,"We Have Sinned Like Our Fathers","Save us, O LORD our God.","Confession",
{"v1  Praise ye the LORD. O give thanks unto the LORD; for he is good.",
 "v6  We have sinned with our fathers, we have committed iniquity, we have done wickedly.",
 "v8  Nevertheless he saved them for his name's sake, that he might make his mighty power to be known.",
 "v48 Blessed be the LORD God of Israel from everlasting to everlasting: and let all the people say, Amen."}},
{107,"Give Thanks to the LORD","O give thanks unto the LORD, for he is good.","Thanksgiving",
{"v1  O give thanks unto the LORD, for he is good: for his mercy endureth for ever.",
 "v8  Oh that men would praise the LORD for his goodness, and for his wonderful works!",
 "v14 He brought them out of darkness and the shadow of death, and brake their bands in sunder.",
 "v43 Whoso is wise, and will observe these things, even they shall understand the lovingkindness of the LORD."}},
{108,"My Heart Is Steadfast","I will sing and give praise, even with my glory.","Praise",
{"v1  O God, my heart is fixed; I will sing and give praise, even with my glory.",
 "v4  For thy mercy is great above the heavens: and thy truth reacheth unto the clouds.",
 "v5  Be thou exalted, O God, above the heavens: and thy glory above all the earth.",
 "v13 Through God we shall do valiantly: for he it is that shall tread down our enemies."}},
{109,"Prayer Against the Wicked","Help me, O LORD my God: O save me according to thy mercy.","Lament",
{"v1  Hold not thy peace, O God of my praise.",
 "v21 But do thou for me, O GOD the Lord, for thy name's sake: because thy mercy is good, deliver thou me.",
 "v26 Help me, O LORD my God: O save me according to thy mercy.",
 "v30 I will greatly praise the LORD with my mouth; yea, I will praise him among the multitude."}},
{110,"The LORD Said to My Lord","Sit thou at my right hand.","Messianic",
{"v1  The LORD said unto my Lord, Sit thou at my right hand, until I make thine enemies thy footstool.",
 "v2  The LORD shall send the rod of thy strength out of Zion.",
 "v4  Thou art a priest for ever after the order of Melchizedek.",
 "v5  The Lord at thy right hand shall strike through kings in the day of his wrath."}},
{111,"Praise for God's Works","The fear of the LORD is the beginning of wisdom.","Wisdom",
{"v1  Praise ye the LORD. I will praise the LORD with my whole heart.",
 "v4  He hath made his wonderful works to be remembered: the LORD is gracious and full of compassion.",
 "v9  He sent redemption unto his people: he hath commanded his covenant for ever.",
 "v10 The fear of the LORD is the beginning of wisdom: a good understanding have all they that do his commandments."}},
{112,"Blessed Is the Man Who Fears","His righteousness endureth for ever.","Righteousness",
{"v1  Praise ye the LORD. Blessed is the man that feareth the LORD, that delighteth greatly in his commandments.",
 "v4  Unto the upright there ariseth light in the darkness: he is gracious, and full of compassion.",
 "v6  Surely he shall not be moved for ever: the righteous shall be in everlasting remembrance.",
 "v9  He hath dispersed, he hath given to the poor; his righteousness endureth for ever."}},
{113,"Praise from the Rising of the Sun","Who is like unto the LORD our God?","Praise",
{"v1  Praise ye the LORD. Praise, O ye servants of the LORD, praise the name of the LORD.",
 "v2  Blessed be the name of the LORD from this time forth and for evermore.",
 "v3  From the rising of the sun unto the going down of the same the LORD's name is to be praised.",
 "v5  Who is like unto the LORD our God, who dwelleth on high?"}},
{114,"When Israel Came Out of Egypt","The sea saw it, and fled.","History",
{"v1  When Israel went out of Egypt, the house of Jacob from a people of strange language.",
 "v3  The sea saw it, and fled: Jordan was driven back.",
 "v7  Tremble, thou earth, at the presence of the Lord, at the presence of the God of Jacob.",
 "v8  Which turned the rock into a standing water, the flint into a fountain of waters."}},
{115,"Not to Us, O LORD","Our God is in the heavens: he hath done whatsoever he hath pleased.","Sovereignty",
{"v1  Not unto us, O LORD, not unto us, but unto thy name give glory.",
 "v3  But our God is in the heavens: he hath done whatsoever he hath pleased.",
 "v11 Ye that fear the LORD, trust in the LORD: he is their help and their shield.",
 "v18 But we will bless the LORD from this time forth and for evermore. Praise the LORD."}},
{116,"I Love the LORD","I will walk before the LORD in the land of the living.","Gratitude",
{"v1  I love the LORD, because he hath heard my voice and my supplications.",
 "v6  The LORD preserveth the simple: I was brought low, and he helped me.",
 "v12 What shall I render unto the LORD for all his benefits toward me?",
 "v17 I will offer to thee the sacrifice of thanksgiving, and will call upon the name of the LORD."}},
{117,"Praise the LORD, All Nations","His merciful kindness is great toward us.","Praise",
{"v1  O praise the LORD, all ye nations: praise him, all ye people.",
 "v2  For his merciful kindness is great toward us: and the truth of the LORD endureth for ever.",
 "v2  Praise ye the LORD."}},
{118,"This Is the Day","This is the day which the LORD hath made; we will rejoice.","Victory",
{"v14 The LORD is my strength and song, and is become my salvation.",
 "v22 The stone which the builders refused is become the head stone of the corner.",
 "v24 This is the day which the LORD hath made; we will rejoice and be glad in it.",
 "v26 Blessed is he that cometh in the name of the LORD."}},
{119,"The Word of God","Thy word is a lamp unto my feet, and a light unto my path.","Scripture",
{"v9  Wherewithal shall a young man cleanse his way? by taking heed according to thy word.",
 "v11 Thy word have I hid in mine heart, that I might not sin against thee.",
 "v18 Open thou mine eyes, that I may behold wondrous things out of thy law.",
 "v105 Thy word is a lamp unto my feet, and a light unto my path.",
 "v130 The entrance of thy words giveth light; it giveth understanding unto the simple.",
 "v162 I rejoice at thy word, as one that findeth great spoil."}},
{120,"Deliver Me, O LORD","My soul hath long dwelt with him that hateth peace.","Lament",
{"v1  In my distress I cried unto the LORD, and he heard me.",
 "v2  Deliver my soul, O LORD, from lying lips, and from a deceitful tongue.",
 "v6  My soul hath long dwelt with him that hateth peace.",
 "v7  I am for peace: but when I speak, they are for war."}},
{121,"My Help Comes from the LORD","My help cometh from the LORD, which made heaven and earth.","Protection",
{"v1  I will lift up mine eyes unto the hills, from whence cometh my help.",
 "v2  My help cometh from the LORD, which made heaven and earth.",
 "v3  He will not suffer thy foot to be moved: he that keepeth thee will not slumber.",
 "v7  The LORD shall preserve thee from all evil: he shall preserve thy soul.",
 "v8  The LORD shall preserve thy going out and thy coming in from this time forth."}},
{122,"I Was Glad","I was glad when they said unto me, Let us go into the house of the LORD.","Worship",
{"v1  I was glad when they said unto me, Let us go into the house of the LORD.",
 "v2  Our feet shall stand within thy gates, O Jerusalem.",
 "v6  Pray for the peace of Jerusalem: they shall prosper that love thee.",
 "v7  Peace be within thy walls, and prosperity within thy palaces."}},
{123,"Eyes on the LORD","Our eyes wait upon the LORD our God.","Dependence",
{"v1  Unto thee lift I up mine eyes, O thou that dwellest in the heavens.",
 "v2  Behold, as the eyes of servants look unto the hand of their masters — so our eyes wait upon the LORD.",
 "v3  Have mercy upon us, O LORD, have mercy upon us.",
 "v4  Our soul is exceedingly filled with the scorning of those that are at ease."}},
{124,"Our Help Is in the Name of the LORD","The LORD who is on our side.","Deliverance",
{"v1  If it had not been the LORD who was on our side, now may Israel say.",
 "v3  Then they had swallowed us up quick, when their wrath was kindled against us.",
 "v7  Our soul is escaped as a bird out of the snare of the fowlers: the snare is broken.",
 "v8  Our help is in the name of the LORD, who made heaven and earth."}},
{125,"Those Who Trust in the LORD","They that trust in the LORD shall be as mount Zion.","Trust",
{"v1  They that trust in the LORD shall be as mount Zion, which cannot be removed.",
 "v2  As the mountains are round about Jerusalem, so the LORD is round about his people.",
 "v4  Do good, O LORD, unto those that be good, and to them that are upright in their hearts.",
 "v5  Peace shall be upon Israel."}},
{126,"Restore Our Fortunes","They that sow in tears shall reap in joy.","Joy",
{"v1  When the LORD turned again the captivity of Zion, we were like them that dream.",
 "v3  The LORD hath done great things for us; whereof we are glad.",
 "v5  They that sow in tears shall reap in joy.",
 "v6  He that goeth forth and weepeth, bearing precious seed, shall doubtless come again with rejoicing."}},
{127,"Unless the LORD","Except the LORD build the house, they labour in vain.","Dependence",
{"v1  Except the LORD build the house, they labour in vain that build it.",
 "v2  It is vain for you to rise up early, to sit up late, to eat the bread of sorrows.",
 "v3  Lo, children are an heritage of the LORD: and the fruit of the womb is his reward.",
 "v4  As arrows are in the hand of a mighty man; so are children of the youth."}},
{128,"Blessed Is Everyone Who Fears","Thou shalt see thy children's children.","Family",
{"v1  Blessed is every one that feareth the LORD; that walketh in his ways.",
 "v2  For thou shalt eat the labour of thine hands: happy shalt thou be, and it shall be well with thee.",
 "v3  Thy wife shall be as a fruitful vine by the sides of thine house.",
 "v6  Yea, thou shalt see thy children's children, and peace upon Israel."}},
{129,"Persecuted but Not Destroyed","The LORD is righteous.","Perseverance",
{"v1  Many a time have they afflicted me from my youth, may Israel now say.",
 "v2  Many a time have they afflicted me from my youth: yet they have not prevailed against me.",
 "v4  The LORD is righteous: he hath cut asunder the cords of the wicked.",
 "v8  Neither do they which go by say, The blessing of the LORD be upon you."}},
{130,"Out of the Depths","With the LORD there is mercy, and with him is plenteous redemption.","Forgiveness",
{"v1  Out of the depths have I cried unto thee, O LORD.",
 "v3  If thou, LORD, shouldest mark iniquities, O Lord, who shall stand?",
 "v4  But there is forgiveness with thee, that thou mayest be feared.",
 "v5  I wait for the LORD, my soul doth wait, and in his word do I hope.",
 "v7  Let Israel hope in the LORD: for with the LORD there is mercy, and plenteous redemption."}},
{131,"Childlike Trust","I have calmed and quieted my soul, like a weaned child.","Humility",
{"v1  LORD, my heart is not haughty, nor mine eyes lofty.",
 "v2  Surely I have behaved and quieted myself, as a child that is weaned of his mother.",
 "v3  Let Israel hope in the LORD from henceforth and for ever."}},
{132,"Remember David","The LORD hath chosen Zion.","Covenant",
{"v1  LORD, remember David, and all his afflictions.",
 "v11 The LORD hath sworn in truth unto David; he will not turn from it.",
 "v13 For the LORD hath chosen Zion; he hath desired it for his habitation.",
 "v17 There will I make the horn of David to bud: I have ordained a lamp for mine anointed."}},
{133,"Brotherly Unity","Behold, how good and how pleasant it is for brethren to dwell together in unity!","Unity",
{"v1  Behold, how good and how pleasant it is for brethren to dwell together in unity!",
 "v2  It is like the precious ointment upon the head, that ran down upon the beard.",
 "v3  As the dew of Hermon, and as the dew that descended upon the mountains of Zion.",
 "v3  For there the LORD commanded the blessing, even life for evermore."}},
{134,"Bless the LORD at Night","Lift up your hands in the sanctuary, and bless the LORD.","Night Praise",
{"v1  Behold, bless ye the LORD, all ye servants of the LORD.",
 "v2  Lift up your hands in the sanctuary, and bless the LORD.",
 "v3  The LORD that made heaven and earth bless thee out of Zion."}},
{135,"Praise the Name of the LORD","Praise ye the LORD; for the LORD is good.","Praise",
{"v1  Praise ye the LORD. Praise ye the name of the LORD; praise him, O ye servants of the LORD.",
 "v3  Praise the LORD; for the LORD is good: sing praises unto his name; for it is pleasant.",
 "v5  For I know that the LORD is great, and that our Lord is above all gods.",
 "v21 Blessed be the LORD out of Zion, which dwelleth at Jerusalem. Praise ye the LORD."}},
{136,"His Mercy Endures Forever","O give thanks unto the LORD; for he is good.","Mercy",
{"v1  O give thanks unto the LORD; for he is good: for his mercy endureth for ever.",
 "v4  To him who alone doeth great wonders: for his mercy endureth for ever.",
 "v23 Who remembered us in our low estate: for his mercy endureth for ever.",
 "v26 O give thanks unto the God of heaven: for his mercy endureth for ever."}},
{137,"By the Waters of Babylon","How shall we sing the LORD's song in a strange land?","Exile",
{"v1  By the rivers of Babylon, there we sat down, yea, we wept, when we remembered Zion.",
 "v2  We hanged our harps upon the willows in the midst thereof.",
 "v4  How shall we sing the LORD's song in a strange land?",
 "v6  If I do not remember thee, let my tongue cleave to the roof of my mouth."}},
{138,"I Will Praise Thee with My Whole Heart","The LORD is high, yet hath he respect unto the lowly.","Praise",
{"v1  I will praise thee with my whole heart: before the gods will I sing praise unto thee.",
 "v3  In the day when I cried thou answeredst me, and strengthenedst me with strength in my soul.",
 "v6  Though the LORD be high, yet hath he respect unto the lowly.",
 "v8  The LORD will perfect that which concerneth me."}},
{139,"Thou Hast Searched Me","I am fearfully and wonderfully made.","God's Knowledge",
{"v1  O LORD, thou hast searched me, and known me.",
 "v2  Thou knowest my downsitting and mine uprising, thou understandest my thought afar off.",
 "v7  Whither shall I go from thy spirit? or whither shall I flee from thy presence?",
 "v13 For thou hast possessed my reins: thou hast covered me in my mother's womb.",
 "v14 I will praise thee; for I am fearfully and wonderfully made.",
 "v23 Search me, O God, and know my heart: try me, and know my thoughts."}},
{140,"Deliver Me from Evil Men","I know that the LORD will maintain the cause of the afflicted.","Protection",
{"v1  Deliver me, O LORD, from the evil man: preserve me from the violent man.",
 "v4  Keep me, O LORD, from the hands of the wicked.",
 "v12 I know that the LORD will maintain the cause of the afflicted.",
 "v13 Surely the righteous shall give thanks unto thy name."}},
{141,"Let My Prayer Be Set Forth","Set a watch, O LORD, before my mouth.","Prayer",
{"v1  LORD, I cry unto thee: make haste unto me; give ear unto my voice.",
 "v2  Let my prayer be set forth before thee as incense; and the lifting up of my hands as the evening sacrifice.",
 "v3  Set a watch, O LORD, before my mouth; keep the door of my lips.",
 "v5  Let the righteous smite me; it shall be a kindness: and let him reprove me."}},
{142,"Refuge in God Alone","Thou art my refuge and my portion in the land of the living.","Refuge",
{"v1  I cried unto the LORD with my voice; with my voice unto the LORD did I make my supplication.",
 "v4  I looked on my right hand, and beheld, but there was no man that would know me.",
 "v5  I cried unto thee, O LORD: I said, Thou art my refuge and my portion in the land of the living.",
 "v7  Bring my soul out of prison, that I may praise thy name."}},
{143,"Teach Me to Do Thy Will","Teach me to do thy will; for thou art my God.","Guidance",
{"v1  Hear my prayer, O LORD, give ear to my supplications.",
 "v5  I remember the days of old; I meditate on all thy works.",
 "v8  Cause me to hear thy lovingkindness in the morning; for in thee do I trust.",
 "v10 Teach me to do thy will; for thou art my God: thy spirit is good."}},
{144,"Blessed Be the LORD My Rock","What is man, that thou takest knowledge of him?","Praise",
{"v1  Blessed be the LORD my strength, which teacheth my hands to war.",
 "v3  LORD, what is man, that thou takest knowledge of him! or the son of man, that thou makest account of him!",
 "v9  I will sing a new song unto thee, O God.",
 "v15 Happy is that people, that is in such a case: yea, happy is that people, whose God is the LORD."}},
{145,"I Will Extol Thee","Great is the LORD, and greatly to be praised.","Praise",
{"v1  I will extol thee, my God, O king; and I will bless thy name for ever and ever.",
 "v3  Great is the LORD, and greatly to be praised; and his greatness is unsearchable.",
 "v8  The LORD is gracious, and full of compassion; slow to anger, and of great mercy.",
 "v18 The LORD is nigh unto all them that call upon him, to all that call upon him in truth.",
 "v21 My mouth shall speak the praise of the LORD: and let all flesh bless his holy name for ever."}},
{146,"Praise the LORD, O My Soul","Happy is he that hath the God of Jacob for his help.","Trust",
{"v1  Praise ye the LORD. Praise the LORD, O my soul.",
 "v3  Put not your trust in princes, nor in the son of man, in whom there is no help.",
 "v5  Happy is he that hath the God of Jacob for his help, whose hope is in the LORD his God.",
 "v10 The LORD shall reign for ever, even thy God, O Zion, unto all generations."}},
{147,"He Heals the Brokenhearted","He healeth the broken in heart, and bindeth up their wounds.","Healing",
{"v1  Praise ye the LORD: for it is good to sing praises unto our God.",
 "v3  He healeth the broken in heart, and bindeth up their wounds.",
 "v5  Great is our Lord, and of great power: his understanding is infinite.",
 "v11 The LORD taketh pleasure in them that fear him, in those that hope in his mercy."}},
{148,"All Creation Praise","Praise ye the LORD from the heavens.","Creation Praise",
{"v1  Praise ye the LORD. Praise ye the LORD from the heavens: praise him in the heights.",
 "v3  Praise ye him, sun and moon: praise him, all ye stars of light.",
 "v5  Let them praise the name of the LORD: for he commanded, and they were created.",
 "v13 Let them praise the name of the LORD: for his name alone is excellent; his glory is above the earth."}},
{149,"Sing to the LORD a New Song","Let them praise his name in the dance.","Praise",
{"v1  Praise ye the LORD. Sing unto the LORD a new song, and his praise in the congregation of saints.",
 "v3  Let them praise his name in the dance: let them sing praises unto him with the timbrel and harp.",
 "v4  For the LORD taketh pleasure in his people: he will beautify the meek with salvation.",
 "v5  Let the saints be joyful in glory: let them sing aloud upon their beds."}},
{150,"Praise God in His Sanctuary","Let every thing that hath breath praise the LORD.","Final Praise",
{"v1  Praise ye the LORD. Praise God in his sanctuary: praise him in the firmament of his power.",
 "v2  Praise him for his mighty acts: praise him according to his excellent greatness.",
 "v3  Praise him with the sound of the trumpet: praise him with the psaltery and harp.",
 "v4  Praise him with the timbrel and dance: praise him with stringed instruments and organs.",
 "v5  Praise him upon the loud cymbals: praise him upon the high sounding cymbals.",
 "v6  Let every thing that hath breath praise the LORD. Praise ye the LORD! HALLELUJAH!"}},
};

// ─── Versicles ────────────────────────────────────────────
struct Versicle {
    std::string title, reference;
    std::vector<std::pair<std::string,std::string>> lines;
};
std::vector<Versicle> versicles = {
{"The Grace","2 Corinthians 13:14",
{{"L:","The grace of the Lord Jesus Christ,"},
 {"R:","And the love of God,"},
 {"L:","And the fellowship of the Holy Spirit,"},
 {"R:","Be with you all."},
 {"ALL:","Amen."}}},
{"Opening Versicle","Psalm 51:15 / Psalm 70:1",
{{"L:","O Lord, open thou our lips."},
 {"R:","And our mouth shall show forth thy praise."},
 {"L:","O God, make speed to save us."},
 {"R:","O Lord, make haste to help us."},
 {"L:","Glory be to the Father, and to the Son,"},
 {"R:","And to the Holy Ghost."},
 {"L:","As it was in the beginning, is now, and ever shall be,"},
 {"R:","World without end. Amen."}}},
{"The Apostles' Creed","Ancient Confession of Faith",
{{"ALL:","I believe in God, the Father Almighty, Creator of heaven and earth."},
 {"ALL:","I believe in Jesus Christ, His only Son, our Lord,"},
 {"ALL:","Who was conceived by the Holy Spirit, born of the Virgin Mary,"},
 {"ALL:","Suffered under Pontius Pilate, was crucified, died, and was buried."},
 {"ALL:","He descended to the dead. On the third day He rose again."},
 {"ALL:","He ascended into heaven, and is seated at the right hand of the Father."},
 {"ALL:","He will come again to judge the living and the dead."},
 {"ALL:","I believe in the Holy Spirit, the holy universal Church,"},
 {"ALL:","The communion of saints, the forgiveness of sins,"},
 {"ALL:","The resurrection of the body, and the life everlasting. Amen."}}},
{"The Nicene Creed","Council of Nicaea, AD 325",
{{"ALL:","We believe in one God, the Father Almighty,"},
 {"ALL:","Maker of heaven and earth, and of all things visible and invisible."},
 {"ALL:","And in one Lord Jesus Christ, the only-begotten Son of God,"},
 {"ALL:","Begotten of the Father before all worlds; God of God, Light of Light,"},
 {"ALL:","Very God of very God; begotten, not made, being of one substance with the Father."},
 {"ALL:","By whom all things were made; who for us men and for our salvation came down."},
 {"ALL:","And was incarnate by the Holy Spirit of the Virgin Mary, and was made man."},
 {"ALL:","And was crucified also for us under Pontius Pilate; He suffered and was buried."},
 {"ALL:","And the third day He rose again according to the Scriptures."},
 {"ALL:","And ascended into heaven, and sits on the right hand of the Father."},
 {"ALL:","And He shall come again with glory to judge both the living and the dead."},
 {"ALL:","I believe in the Holy Ghost, the Lord and Giver of Life."},
 {"ALL:","Who proceeds from the Father and the Son."},
 {"ALL:","Who with the Father and the Son together is worshipped and glorified."},
 {"ALL:","I believe in one holy universal and apostolic Church."},
 {"ALL:","I acknowledge one baptism for the remission of sins."},
 {"ALL:","And I look for the resurrection of the dead, and the life of the world to come. Amen."}}},
{"The Doxology","Praise God from Whom All Blessings Flow",
{{"ALL:","Praise God, from Whom all blessings flow;"},
 {"ALL:","Praise Him, all creatures here below;"},
 {"ALL:","Praise Him above, ye heavenly host;"},
 {"ALL:","Praise Father, Son, and Holy Ghost. Amen."}}},
{"Kyrie Eleison","Ancient Litany — Lord Have Mercy",
{{"L:","Lord, have mercy upon us."},
 {"R:","Lord, have mercy upon us."},
 {"L:","Christ, have mercy upon us."},
 {"R:","Christ, have mercy upon us."},
 {"L:","Lord, have mercy upon us."},
 {"R:","Lord, have mercy upon us."}}},
{"Gloria in Excelsis","Luke 2:14",
{{"ALL:","Glory to God in the highest,"},
 {"ALL:","And on earth peace, good will toward men."},
 {"L:","We praise thee, we bless thee, we worship thee, we glorify thee."},
 {"R:","We give thanks to thee for thy great glory."},
 {"L:","O Lord God, heavenly King, God the Father Almighty."},
 {"ALL:","O Lord, the only-begotten Son, Jesus Christ. Amen."}}},
{"The Sursum Corda","Ancient Eucharistic Dialogue",
{{"L:","The Lord be with you."},
 {"R:","And also with you."},
 {"L:","Lift up your hearts."},
 {"R:","We lift them up to the Lord."},
 {"L:","Let us give thanks to the Lord our God."},
 {"R:","It is right to give him thanks and praise."}}},
{"Benediction Response","Numbers 6:24-26",
{{"L:","The Lord bless you and keep you."},
 {"R:","Thanks be to God."},
 {"L:","The Lord make his face shine upon you and be gracious to you."},
 {"R:","Thanks be to God."},
 {"L:","The Lord turn his face toward you and give you peace."},
 {"ALL:","Amen. Thanks be to God!"}}},
{"The Great Litany","Traditional Prayer",
{{"L:","From all evil and mischief; from sin, from the crafts of the devil,"},
 {"R:","Good Lord, deliver us."},
 {"L:","From all blindness of heart; from pride, vainglory, and hypocrisy,"},
 {"R:","Good Lord, deliver us."},
 {"L:","From lightning and tempest; from plague, pestilence, and famine,"},
 {"R:","Good Lord, deliver us."},
 {"L:","By the mystery of thy holy Incarnation; by thy holy Nativity,"},
 {"R:","Good Lord, deliver us."},
 {"L:","By thine agony and bloody sweat; by thy cross and passion,"},
 {"R:","Good Lord, deliver us."},
 {"L:","By thy precious death and burial; by thy glorious Resurrection,"},
 {"R:","Good Lord, deliver us."},
 {"L:","In all time of our tribulation; in the hour of death,"},
 {"R:","Good Lord, deliver us."},
 {"ALL:","O Lamb of God, that takest away the sins of the world, grant us thy peace."}}},
{"Agnus Dei","Lamb of God — John 1:29",
{{"ALL:","O Lamb of God, that takest away the sins of the world,"},
 {"ALL:","Have mercy upon us."},
 {"ALL:","O Lamb of God, that takest away the sins of the world,"},
 {"ALL:","Have mercy upon us."},
 {"ALL:","O Lamb of God, that takest away the sins of the world,"},
 {"ALL:","Grant us thy peace. Amen."}}},
{"Sanctus","Holy, Holy, Holy — Isaiah 6:3",
{{"ALL:","Holy, holy, holy, Lord God of hosts,"},
 {"ALL:","Heaven and earth are full of thy glory."},
 {"ALL:","Glory be to thee, O Lord Most High."},
 {"ALL:","Hosanna in the highest."},
 {"ALL:","Blessed is he that cometh in the name of the Lord."},
 {"ALL:","Hosanna in the highest. Amen."}}},
};

// ─── Famous Verses ────────────────────────────────────────
struct Verse { std::string ref, text, theme; };
std::vector<Verse> famousVerses = {
{"Genesis 1:1","In the beginning God created the heaven and the earth.","Creation"},
{"Genesis 1:27","So God created man in his own image, in the image of God created he him.","Humanity"},
{"Exodus 14:14","The LORD shall fight for you, and ye shall hold your peace.","Courage"},
{"Deuteronomy 6:5","Thou shalt love the LORD thy God with all thine heart, and with all thy soul, and with all thy might.","Love"},
{"Joshua 1:9","Be strong and of a good courage; be not afraid, neither be thou dismayed: for the LORD thy God is with thee.","Courage"},
{"1 Samuel 16:7","The LORD seeth not as man seeth; for man looketh on the outward appearance, but the LORD looketh on the heart.","Heart"},
{"Psalm 23:1","The LORD is my shepherd; I shall not want.","Comfort"},
{"Psalm 27:1","The LORD is my light and my salvation; whom shall I fear?","Faith"},
{"Psalm 46:1","God is our refuge and strength, a very present help in trouble.","Refuge"},
{"Psalm 46:10","Be still, and know that I am God.","Peace"},
{"Psalm 91:1","He that dwelleth in the secret place of the most High shall abide under the shadow of the Almighty.","Protection"},
{"Psalm 103:12","As far as the east is from the west, so far hath he removed our transgressions from us.","Forgiveness"},
{"Psalm 119:105","Thy word is a lamp unto my feet, and a light unto my path.","Scripture"},
{"Proverbs 3:5-6","Trust in the LORD with all thine heart; and lean not unto thine own understanding. In all thy ways acknowledge him, and he shall direct thy paths.","Wisdom"},
{"Isaiah 40:31","But they that wait upon the LORD shall renew their strength; they shall mount up with wings as eagles.","Strength"},
{"Isaiah 41:10","Fear thou not; for I am with thee: be not dismayed; for I am thy God: I will strengthen thee.","Comfort"},
{"Isaiah 53:5","But he was wounded for our transgressions, he was bruised for our iniquities: and with his stripes we are healed.","Atonement"},
{"Jeremiah 29:11","For I know the thoughts that I think toward you, saith the LORD, thoughts of peace, and not of evil, to give you an expected end.","Hope"},
{"Lamentations 3:22-23","It is of the LORD's mercies that we are not consumed, because his compassions fail not. They are new every morning: great is thy faithfulness.","Faithfulness"},
{"Micah 6:8","He hath shewed thee, O man, what is good; and what doth the LORD require of thee, but to do justly, and to love mercy, and to walk humbly with thy God?","Justice"},
{"Matthew 5:9","Blessed are the peacemakers: for they shall be called the children of God.","Peace"},
{"Matthew 6:33","But seek ye first the kingdom of God, and his righteousness; and all these things shall be added unto you.","Priority"},
{"Matthew 11:28","Come unto me, all ye that labour and are heavy laden, and I will give you rest.","Rest"},
{"Matthew 28:19","Go ye therefore, and teach all nations, baptizing them in the name of the Father, and of the Son, and of the Holy Ghost.","Mission"},
{"John 1:1","In the beginning was the Word, and the Word was with God, and the Word was God.","Christ"},
{"John 3:16","For God so loved the world, that he gave his only begotten Son, that whosoever believeth in him should not perish, but have everlasting life.","Love"},
{"John 3:17","God sent not his Son into the world to condemn the world; but that the world through him might be saved.","Salvation"},
{"John 11:25","I am the resurrection, and the life: he that believeth in me, though he were dead, yet shall he live.","Resurrection"},
{"John 14:6","Jesus saith unto him, I am the way, the truth, and the life: no man cometh unto the Father, but by me.","Salvation"},
{"John 15:13","Greater love hath no man than this, that a man lay down his life for his friends.","Love"},
{"Romans 3:23","For all have sinned, and come short of the glory of God.","Sin"},
{"Romans 6:23","The wages of sin is death; but the gift of God is eternal life through Jesus Christ our Lord.","Grace"},
{"Romans 8:28","All things work together for good to them that love God, to them who are the called according to his purpose.","Providence"},
{"Romans 8:38-39","Neither death, nor life, nor angels, nor principalities, nor powers, nor things present, nor things to come, shall be able to separate us from the love of God.","Love"},
{"1 Corinthians 13:4","Charity suffereth long, and is kind; charity envieth not; vaunteth not itself, is not puffed up.","Love"},
{"1 Corinthians 13:13","And now abideth faith, hope, charity, these three; but the greatest of these is charity.","Love"},
{"Galatians 5:22-23","The fruit of the Spirit is love, joy, peace, longsuffering, gentleness, goodness, faith, meekness, temperance.","Spirit"},
{"Ephesians 2:8","For by grace are ye saved through faith; and that not of yourselves: it is the gift of God.","Grace"},
{"Philippians 4:6-7","In every thing by prayer and supplication with thanksgiving let your requests be made known unto God. And the peace of God, which passeth all understanding, shall keep your hearts.","Prayer"},
{"Philippians 4:13","I can do all things through Christ which strengtheneth me.","Strength"},
{"1 Thessalonians 5:17","Pray without ceasing.","Prayer"},
{"2 Timothy 3:16","All scripture is given by inspiration of God, and is profitable for doctrine, for reproof, for correction, for instruction in righteousness.","Scripture"},
{"Hebrews 11:1","Now faith is the substance of things hoped for, the evidence of things not seen.","Faith"},
{"Hebrews 13:8","Jesus Christ the same yesterday, and to day, and for ever.","Christ"},
{"James 1:17","Every good gift and every perfect gift is from above, and cometh down from the Father of lights.","Gifts"},
{"1 Peter 5:7","Casting all your care upon him; for he careth for you.","Trust"},
{"1 John 4:8","He that loveth not knoweth not God; for God is love.","Love"},
{"Revelation 3:20","Behold, I stand at the door, and knock: if any man hear my voice, and open the door, I will come in to him.","Invitation"},
{"Revelation 21:4","God shall wipe away all tears from their eyes; and there shall be no more death, neither sorrow, nor crying, neither shall there be any more pain.","Heaven"},
};

// ─── Bible Stories ────────────────────────────────────────
struct Story { std::string title, book_ref; std::vector<std::string> lines; };
std::vector<Story> stories = {
{"The Creation","Genesis 1:1-2:3",
{"In the beginning, God created the heavens and the earth.",
 "The earth was formless and void, and darkness covered the deep.",
 "Day 1: God said 'Let there be LIGHT' — and light blazed forth.",
 "Day 2: God separated the waters, creating the sky above.",
 "Day 3: Dry land appeared; the earth burst forth with plants.",
 "Day 4: The sun, moon, and stars were set in the heavens.",
 "Day 5: Fish filled every sea and birds soared through every sky.",
 "Day 6: Every creature was made — then God formed man in His image.",
 "Day 7: God rested, and He blessed the seventh day as holy.",
 "And God saw everything that He had made — it was VERY GOOD."}},
{"Noah's Ark","Genesis 6-9",
{"The earth was filled with wickedness — every thought was only evil.",
 "But Noah found grace in the eyes of the LORD — he walked with God.",
 "God said: 'Build an ark of gopher wood, 300 cubits long.'",
 "Noah obeyed every word God gave him — not one command undone.",
 "God brought two of every creature, male and female, into the ark.",
 "Then God shut the door. Rain fell for 40 days and 40 nights.",
 "The waters rose until even the highest mountains were submerged.",
 "Every living thing outside the ark perished from the earth.",
 "After 150 days, the ark rested on the mountains of Ararat.",
 "A dove returned with an olive leaf — the waters were subsiding!",
 "God placed a rainbow in the sky: His everlasting covenant of peace."}},
{"Abraham and Isaac","Genesis 22",
{"After these things God tested Abraham: 'Take your son, your only son Isaac.'",
 "'Go to the land of Moriah and offer him as a burnt offering.'",
 "Abraham rose early and set out with Isaac and two servants.",
 "On the third day Abraham saw the mountain from afar.",
 "Isaac asked: 'Where is the lamb for the burnt offering?'",
 "Abraham answered: 'God Himself will provide the lamb, my son.'",
 "Abraham built the altar, bound Isaac, and raised the knife.",
 "The Angel of the LORD cried: 'Do not lay your hand on the boy!'",
 "'Now I know you fear God — you have not withheld your only son.'",
 "Abraham looked up and saw a ram caught by its horns in a thicket.",
 "He called that place Jehovah-Jireh — The LORD Will Provide."}},
{"Moses Parts the Red Sea","Exodus 14",
{"Pharaoh changed his mind and pursued Israel with 600 chariots.",
 "Israel was trapped — the sea before them, the army behind them.",
 "The people cried out to Moses: 'Why did you bring us here to die?'",
 "Moses said: 'Fear not! Stand firm and see the salvation of the LORD.'",
 "'The LORD will fight for you — you need only to be still.'",
 "God told Moses: 'Lift your staff and stretch out your hand over the sea.'",
 "A pillar of cloud moved between Israel and the Egyptian army.",
 "Moses stretched his hand — and the sea divided all through the night!",
 "The waters stood like walls; Israel walked through on dry ground.",
 "When the Egyptians followed, Moses stretched his hand again.",
 "The waters returned — and all Pharaoh's army was covered.",
 "Israel saw the great power of the LORD, and they feared the LORD."}},
{"David and Goliath","1 Samuel 17",
{"The Philistines gathered; their champion Goliath stepped forward.",
 "Goliath stood over nine feet tall, clad in heavy bronze armor.",
 "For 40 days he taunted Israel: 'Give me a man to fight me!'",
 "Young David arrived at the camp bringing food for his brothers.",
 "David said: 'Who is this Philistine to defy the armies of the living God?'",
 "'The LORD saved me from the lion — He will deliver me from this giant!'",
 "He chose five smooth stones from the brook and took his sling.",
 "As Goliath charged, David slung a stone.",
 "The stone struck Goliath's forehead — he fell face down to the earth!",
 "David took Goliath's sword and cut off his head.",
 "And all Israel shouted — for the battle belongs to the LORD!"}},
{"Daniel in the Lions' Den","Daniel 6",
{"King Darius appointed Daniel over all the kingdom — he excelled all.",
 "The jealous officials sought to destroy Daniel — but found no fault.",
 "They tricked the king into signing a law: pray only to the king for 30 days.",
 "Daniel opened his window toward Jerusalem and prayed — three times daily.",
 "They caught him and reported it to the grieved king.",
 "The king was bound by his own law and cast Daniel into the lions' den.",
 "The king fasted all night, without sleep, crying: 'May your God save you!'",
 "At dawn he ran to the den and cried: 'Daniel, has your God saved you?'",
 "Daniel answered: 'O king, my God sent His angel and shut the lions' mouths!'",
 "Daniel was lifted out — not a mark on him — for he trusted in his God.",
 "The king decreed: 'In every dominion men must tremble before the God of Daniel.'"}},
{"The Birth of Jesus","Luke 2:1-20",
{"Caesar Augustus decreed a census — all must register.",
 "Joseph went from Nazareth to Bethlehem, city of David.",
 "He went with Mary, his betrothed, who was great with child.",
 "While there, the time came for the baby to be born.",
 "She gave birth to her firstborn Son and wrapped Him in swaddling clothes.",
 "She laid Him in a manger — for there was no room in the inn.",
 "In nearby fields, shepherds watched their flocks by night.",
 "Suddenly an angel appeared and the glory of God shone around them!",
 "'Do not be afraid! I bring good news of great joy for all people!'",
 "'Today in Bethlehem a Savior has been born — He is Christ the Lord!'",
 "Suddenly a multitude of angels praised God: 'Glory in the highest!'",
 "The shepherds found Mary, Joseph, and the baby lying in the manger."}},
{"The Feeding of Five Thousand","John 6:1-15",
{"Jesus crossed to the far shore of the Sea of Galilee.",
 "A great crowd of people followed because they saw His miracles.",
 "Jesus looked up and saw the crowd: 'Where shall we buy bread?'",
 "Philip: 'Eight months' wages would not buy enough for each one a bite!'",
 "Andrew said: 'Here is a boy with five small barley loaves and two small fish.'",
 "'But how far will they go among so many?'",
 "Jesus said: 'Have the people sit down.' There were about five thousand.",
 "Jesus gave thanks, and distributed the bread and fish to all who sat there.",
 "When they had all eaten enough, He said: 'Gather the pieces left over.'",
 "They filled twelve baskets with fragments from five barley loaves!",
 "The people said: 'Surely this is the Prophet who is to come into the world.'"}},
{"The Resurrection","Matthew 28 / John 20",
{"Early on the first day of the week, Mary Magdalene came to the tomb.",
 "She saw the stone had been rolled away from the entrance.",
 "She ran to Simon Peter: 'They have taken the Lord out of the tomb!'",
 "Peter and the beloved disciple ran to the tomb; they saw the burial cloths.",
 "Mary stood outside weeping. She looked in and saw two angels in white.",
 "'Woman, why do you weep?' She turned and saw a man — thought He was the gardener.",
 "The man said: 'Mary.' She cried out: 'Rabboni!' — which means Teacher!",
 "That evening Jesus appeared to the disciples, showed His hands and side.",
 "The disciples were overjoyed when they saw the Lord!",
 "'Peace be with you! As the Father has sent me, I am sending you.'",
 "He breathed on them: 'Receive the Holy Spirit.'",
 "Jesus is risen — death is conquered — the tomb is empty!"}},
};

// ─── Proverbs ─────────────────────────────────────────────
struct Proverb { std::string ref, text, theme; };
std::vector<Proverb> proverbs = {
{"1:7",  "The fear of the LORD is the beginning of knowledge: but fools despise wisdom and instruction.","Wisdom"},
{"1:33", "But whoso hearkeneth unto me shall dwell safely, and shall be quiet from fear of evil.","Safety"},
{"3:5",  "Trust in the LORD with all thine heart; and lean not unto thine own understanding.","Trust"},
{"3:6",  "In all thy ways acknowledge him, and he shall direct thy paths.","Guidance"},
{"3:11-12","My son, despise not the chastening of the LORD; neither be weary of his correction: for whom the LORD loveth he correcteth.","Discipline"},
{"4:7",  "Wisdom is the principal thing; therefore get wisdom: and with all thy getting get understanding.","Wisdom"},
{"4:23", "Keep thy heart with all diligence; for out of it are the issues of life.","Heart"},
{"6:16-17","These six things doth the LORD hate: a proud look, a lying tongue, and hands that shed innocent blood.","Warning"},
{"10:9", "He that walketh uprightly walketh surely: but he that perverteth his ways shall be known.","Integrity"},
{"10:12","Hatred stirreth up strifes: but love covereth all sins.","Love"},
{"11:2", "When pride cometh, then cometh shame: but with the lowly is wisdom.","Humility"},
{"11:14","Where no counsel is, the people fall: but in the multitude of counsellors there is safety.","Counsel"},
{"12:1", "Whoso loveth instruction loveth knowledge: but he that hateth reproof is brutish.","Learning"},
{"13:12","Hope deferred maketh the heart sick: but when the desire cometh, it is a tree of life.","Hope"},
{"14:12","There is a way which seemeth right unto a man, but the end thereof are the ways of death.","Caution"},
{"15:1", "A soft answer turneth away wrath: but grievous words stir up anger.","Speech"},
{"15:22","Without counsel purposes are disappointed: but in the multitude of counsellors they are established.","Counsel"},
{"16:3", "Commit thy works unto the LORD, and thy thoughts shall be established.","Commitment"},
{"16:18","Pride goeth before destruction, and an haughty spirit before a fall.","Pride"},
{"17:17","A friend loveth at all times, and a brother is born for adversity.","Friendship"},
{"17:22","A merry heart doeth good like a medicine: but a broken spirit drieth the bones.","Joy"},
{"18:10","The name of the LORD is a strong tower: the righteous runneth into it, and is safe.","Protection"},
{"18:21","Death and life are in the power of the tongue: and they that love it shall eat the fruit thereof.","Speech"},
{"19:17","He that hath pity upon the poor lendeth unto the LORD.","Generosity"},
{"20:7", "The just man walketh in his integrity: his children are blessed after him.","Integrity"},
{"22:6", "Train up a child in the way he should go: and when he is old, he will not depart from it.","Children"},
{"22:29","Seest thou a man diligent in his business? he shall stand before kings.","Diligence"},
{"23:7", "For as he thinketh in his heart, so is he.","Mind"},
{"25:11","A word fitly spoken is like apples of gold in pictures of silver.","Words"},
{"27:1", "Boast not thyself of to morrow; for thou knowest not what a day may bring forth.","Humility"},
{"28:13","He that covereth his sins shall not prosper: but whoso confesseth and forsaketh them shall have mercy.","Repentance"},
{"29:18","Where there is no vision, the people perish: but he that keepeth the law, happy is he.","Vision"},
{"31:10","Who can find a virtuous woman? for her price is far above rubies.","Virtue"},
{"31:30","Favour is deceitful, and beauty is vain: but a woman that feareth the LORD, she shall be praised.","Character"},
};

// ─── Epistles ─────────────────────────────────────────────
struct Epistle { std::string book, chapter, topic; std::vector<std::string> verses; };
std::vector<Epistle> epistles = {
{"Romans","Romans 8","Life in the Spirit",
{"v1  There is therefore now no condemnation to them which are in Christ Jesus.",
 "v14 For as many as are led by the Spirit of God, they are the sons of God.",
 "v28 All things work together for good to them that love God.",
 "v31 If God be for us, who can be against us?",
 "v37 In all these things we are more than conquerors through him that loved us.",
 "v38 Neither death, nor life...shall be able to separate us from the love of God."}},
{"1 Corinthians","1 Corinthians 13","The Love Chapter",
{"v1  Though I speak with the tongues of men and of angels, and have not charity, I am as sounding brass.",
 "v4  Charity suffereth long, and is kind; charity envieth not; vaunteth not itself.",
 "v7  Beareth all things, believeth all things, hopeth all things, endureth all things.",
 "v8  Charity never faileth: but whether there be prophecies, they shall fail.",
 "v12 For now we see through a glass, darkly; but then face to face.",
 "v13 And now abideth faith, hope, charity, these three; but the greatest of these is charity."}},
{"Ephesians","Ephesians 6","The Armor of God",
{"v10 Finally, my brethren, be strong in the Lord, and in the power of his might.",
 "v11 Put on the whole armour of God, that ye may be able to stand against the wiles of the devil.",
 "v14 Stand therefore, having your loins girt about with truth, and the breastplate of righteousness.",
 "v15 And your feet shod with the preparation of the gospel of peace.",
 "v16 Taking the shield of faith, wherewith ye shall be able to quench the fiery darts of the wicked.",
 "v17 And take the helmet of salvation, and the sword of the Spirit, which is the word of God."}},
{"Philippians","Philippians 4","Rejoice Always",
{"v4  Rejoice in the Lord alway: and again I say, Rejoice.",
 "v6  Be careful for nothing; but in every thing by prayer and supplication with thanksgiving.",
 "v7  And the peace of God, which passeth all understanding, shall keep your hearts and minds.",
 "v8  Whatsoever things are true, honest, just, pure, lovely — think on these things.",
 "v11 I have learned, in whatsoever state I am, therewith to be content.",
 "v13 I can do all things through Christ which strengtheneth me."}},
{"Hebrews","Hebrews 11","The Hall of Faith",
{"v1  Now faith is the substance of things hoped for, the evidence of things not seen.",
 "v4  By faith Abel offered unto God a more excellent sacrifice than Cain.",
 "v7  By faith Noah, being warned of God of things not seen as yet, prepared an ark.",
 "v8  By faith Abraham, when he was called to go out into a place he should receive for an inheritance, obeyed.",
 "v11 Through faith also Sara herself received strength to conceive seed.",
 "v39 And these all, having obtained a good report through faith, received not the promise."}},
{"James","James 1-2","Faith and Works",
{"v1:2  Count it all joy when ye fall into divers temptations.",
 "v1:3  Knowing this, that the trying of your faith worketh patience.",
 "v1:17 Every good gift and every perfect gift is from above.",
 "v2:17 Even so faith, if it hath not works, is dead, being alone.",
 "v2:19 Thou believest that there is one God; thou doest well: the devils also believe, and tremble.",
 "v2:26 For as the body without the spirit is dead, so faith without works is dead also."}},
{"Revelation","Revelation 21-22","The New Jerusalem",
{"v21:1 I saw a new heaven and a new earth: for the first heaven and the first earth were passed away.",
 "v21:3 Behold, the tabernacle of God is with men, and he will dwell with them.",
 "v21:4 God shall wipe away all tears from their eyes; there shall be no more death, neither sorrow.",
 "v21:5 He that sat upon the throne said, Behold, I make all things new.",
 "v22:12 Behold, I come quickly; and my reward is with me.",
 "v22:20 He which testifieth these things saith, Surely I come quickly. Amen. Come, Lord Jesus."}},
};

// ─── Gospel Sections ──────────────────────────────────────
struct GospelSection { std::string title, ref; std::vector<std::string> lines; };
std::vector<GospelSection> gospelSections = {
{"The Sermon on the Mount","Matthew 5-7",
{"Jesus went up on a mountainside; His disciples came to Him.",
 "'Blessed are the poor in spirit — theirs is the kingdom of heaven.'",
 "'Blessed are the meek — they will inherit the earth.'",
 "'You are the salt of the earth. You are the light of the world.'",
 "'Do not store up treasures on earth where moth and rust destroy.'",
 "'Store up for yourselves treasures in heaven.'",
 "'No one can serve two masters — you cannot serve both God and money.'",
 "'Ask and it will be given; seek and you will find; knock and the door will be opened.'",
 "'Do to others what you would have them do to you.'",
 "This is the Law and the Prophets — the Golden Rule.",
 "The crowds were amazed — He taught as one who had authority."}},
{"The Parable of the Prodigal Son","Luke 15:11-32",
{"A man had two sons. The younger asked for his inheritance.",
 "The younger son went away and squandered everything in wild living.",
 "When he had spent everything, a severe famine struck that country.",
 "He was starving — longing to eat the pods the pigs were eating.",
 "'How many of my father's servants have food to spare!'",
 "'I will go back to my father and say: I have sinned against heaven.'",
 "While he was still a long way off, his father saw him and ran to him!",
 "The father threw his arms around him and kissed him.",
 "'This son of mine was dead and is alive again; he was lost and is found!'",
 "They began to celebrate — the father's love knew no bounds.",
 "So it is with our Heavenly Father — He runs to meet the returning sinner."}},
{"The Last Supper","Matthew 26 / Luke 22",
{"On the first day of the Feast, the disciples asked: 'Where shall we prepare?'",
 "When evening came, Jesus reclined at the table with the Twelve.",
 "He took bread, gave thanks and broke it, and gave it to them, saying:",
 "'Take and eat; this is my body — do this in remembrance of me.'",
 "Then He took a cup, gave thanks and offered it to them, saying:",
 "'Drink from it, all of you. This is my blood of the new covenant.'",
 "'It is poured out for many for the forgiveness of sins.'",
 "'I tell you, I will not drink of this fruit of the vine again until that day'",
 "'when I drink it new with you in my Father's kingdom.'",
 "When they had sung a hymn, they went out to the Mount of Olives.",
 "This holy meal is the remembrance of His sacrifice — until He comes again."}},
{"The Great Commission","Matthew 28:18-20",
{"Then Jesus came and said:",
 "'All authority in heaven and on earth has been given to me.'",
 "'Therefore go and make disciples of all nations,'",
 "'Baptizing them in the name of the Father and of the Son and of the Holy Spirit,'",
 "'Teaching them to obey everything I have commanded you.'",
 "'And surely I am with you always, to the very end of the age.'",
 "This is the Church's marching orders — given by the risen Lord.",
 "Go into all the world and preach the gospel to every creature.",
 "Whoever believes and is baptized will be saved.",
 "And these signs will follow those who believe.",
 "They went out and preached everywhere — the Lord confirming His word."}},
{"The 'I AM' Sayings of Jesus","John 6-15",
{"'I AM the bread of life — whoever comes to me will never be hungry.' (John 6:35)",
 "'I AM the light of the world — whoever follows me will never walk in darkness.' (John 8:12)",
 "'I AM the gate — whoever enters through me will be saved.' (John 10:9)",
 "'I AM the good shepherd — the good shepherd lays down his life for the sheep.' (John 10:11)",
 "'I AM the resurrection and the life — he who believes in me will live.' (John 11:25)",
 "'I AM the way and the truth and the life.' (John 14:6)",
 "'I AM the true vine — my Father is the gardener.' (John 15:1)",
 "These seven 'I AM' statements reveal the eternal nature of Christ.",
 "They echo God's great name revealed to Moses: 'I AM THAT I AM.'",
 "Jesus is not just a teacher — He is the eternal Son of God.",
 "'Before Abraham was born, I AM!' (John 8:58)"}},
};

// ─── Books of the Bible ───────────────────────────────────
struct Book { std::string name, testament; int chapters; std::string desc; };
std::vector<Book> bibleBooks = {
{"Genesis","Old",50,"Creation, Adam, Noah, Abraham, Joseph"},
{"Exodus","Old",40,"Moses, Plagues, Passover, Ten Commandments"},
{"Leviticus","Old",27,"Priestly laws, sacrifices, holiness codes"},
{"Numbers","Old",36,"Israel in the wilderness, wandering"},
{"Deuteronomy","Old",34,"Moses' farewell, law reiterated"},
{"Joshua","Old",24,"Conquest of Canaan, the Promised Land"},
{"Judges","Old",21,"Cycles: sin, oppression, deliverance"},
{"Ruth","Old",4,"Loyalty, redemption — ancestor of David"},
{"1 Samuel","Old",31,"Samuel, Saul, and the rise of David"},
{"2 Samuel","Old",24,"David's reign, triumph and tragedy"},
{"1 Kings","Old",22,"Solomon, temple, Elijah, kingdom divided"},
{"2 Kings","Old",25,"Elisha, fall of Israel and Judah"},
{"1 Chronicles","Old",29,"David's genealogy and temple plans"},
{"2 Chronicles","Old",36,"Solomon's temple through Judah's exile"},
{"Ezra","Old",10,"Return from exile, rebuilding the temple"},
{"Nehemiah","Old",13,"Rebuilding Jerusalem's walls"},
{"Esther","Old",10,"Esther saves the Jews from Haman's plot"},
{"Job","Old",42,"Suffering, faith, and God's sovereignty"},
{"Psalms","Old",150,"Songs of praise, lament, wisdom, worship"},
{"Proverbs","Old",31,"Wisdom literature attributed to Solomon"},
{"Ecclesiastes","Old",12,"'Vanity of vanities' — life under the sun"},
{"Song of Solomon","Old",8,"Poem of love — the bride and bridegroom"},
{"Isaiah","Old",66,"Messianic prophecies, comfort for Israel"},
{"Jeremiah","Old",52,"Weeping prophet, fall of Jerusalem"},
{"Lamentations","Old",5,"Mourning the destruction of Jerusalem"},
{"Ezekiel","Old",48,"Visions, Valley of Dry Bones, new temple"},
{"Daniel","Old",12,"Daniel in Babylon, lion's den, visions"},
{"Hosea","Old",14,"God's faithful love for unfaithful Israel"},
{"Joel","Old",3,"Day of the LORD, promise of the Spirit"},
{"Amos","Old",9,"Justice for the poor, judgment on Israel"},
{"Obadiah","Old",1,"Judgment against Edom"},
{"Jonah","Old",4,"Jonah, the whale, Nineveh's repentance"},
{"Micah","Old",7,"Justice, mercy, walking humbly with God"},
{"Nahum","Old",3,"Judgment on Nineveh"},
{"Habakkuk","Old",3,"'The just shall live by his faith'"},
{"Zephaniah","Old",3,"Day of the LORD, restoration of Israel"},
{"Haggai","Old",2,"Rebuilding the temple after exile"},
{"Zechariah","Old",14,"Messianic visions and future glory"},
{"Malachi","Old",4,"Tithing, the coming Messenger, Elijah"},
{"Matthew","New",28,"Jesus: Sermon, parables, resurrection"},
{"Mark","New",16,"Action gospel — shortest, most urgent"},
{"Luke","New",24,"Nativity, Good Samaritan, Prodigal Son"},
{"John","New",21,"'I AM' sayings, love, the Word made flesh"},
{"Acts","New",28,"Early church, Pentecost, Paul's journeys"},
{"Romans","New",16,"Justification by faith, life in the Spirit"},
{"1 Corinthians","New",16,"Love chapter, gifts, resurrection"},
{"2 Corinthians","New",13,"Strength in weakness, Paul's suffering"},
{"Galatians","New",6,"Freedom from law, fruit of the Spirit"},
{"Ephesians","New",6,"Armor of God, church as Christ's body"},
{"Philippians","New",4,"Joy, contentment, 'I can do all things'"},
{"Colossians","New",4,"Christ's supremacy, new life"},
{"1 Thessalonians","New",5,"Second coming, encouragement"},
{"2 Thessalonians","New",3,"Steadfastness, man of lawlessness"},
{"1 Timothy","New",6,"Church leadership, godliness"},
{"2 Timothy","New",4,"All Scripture is God-breathed, endure"},
{"Titus","New",3,"Sound doctrine, good works"},
{"Philemon","New",1,"Appeal for the slave Onesimus"},
{"Hebrews","New",13,"Jesus as High Priest, hall of faith"},
{"James","New",5,"Faith without works is dead"},
{"1 Peter","New",5,"Suffering, hope, holy living"},
{"2 Peter","New",3,"False teachers, the Day of the Lord"},
{"1 John","New",5,"God is love, walking in the light"},
{"2 John","New",1,"Walk in truth and love"},
{"3 John","New",1,"Hospitality and faithful living"},
{"Jude","New",1,"Contend for the faith, false teachers"},
{"Revelation","New",22,"Apocalypse, seven seals, new Jerusalem"},
};

// ==========================================================
//                   DISPLAY FUNCTIONS
// ==========================================================

void showBooks() {
    clearScreen();
    sectionHeader("ALL 66 BOOKS OF THE HOLY BIBLE", "B");
    std::cout << BOLD << BRIGHT_YELLOW << "  --- OLD TESTAMENT (39 books) ---\n" << RESET;
    printRule(72, YELLOW);
    int count = 0;
    for (const auto& b : bibleBooks) {
        if (b.testament != "Old") continue;
        ++count;
        std::cout << BOLD << BRIGHT_CYAN << "  " << std::setw(2) << count << ". "
                  << BRIGHT_WHITE << std::left << std::setw(18) << b.name
                  << BRIGHT_YELLOW << " [" << std::setw(3) << b.chapters << "ch]  "
                  << DIM << WHITE << b.desc.substr(0, 44) << RESET << "\n";
        sleep_ms(15);
    }
    std::cout << "\n" << BOLD << BRIGHT_MAGENTA << "  --- NEW TESTAMENT (27 books) ---\n" << RESET;
    printRule(72, MAGENTA);
    count = 0;
    for (const auto& b : bibleBooks) {
        if (b.testament != "New") continue;
        ++count;
        std::cout << BOLD << BRIGHT_CYAN << "  " << std::setw(2) << count << ". "
                  << BRIGHT_WHITE << std::left << std::setw(18) << b.name
                  << BRIGHT_MAGENTA << " [" << std::setw(3) << b.chapters << "ch]  "
                  << DIM << WHITE << b.desc.substr(0, 44) << RESET << "\n";
        sleep_ms(15);
    }
    std::cout << "\n";
    printLine("  66 Books | 1,189 Chapters | ~31,102 Verses | One Story of Redemption", BRIGHT_CYAN, true);
}

void showPsalmsMenu() {
    clearScreen();
    sectionHeader("THE BOOK OF PSALMS -- ALL 150 SONGS", "~");
    printLine("  Select a Psalm to read its key verses and full text.\n", BRIGHT_CYAN, true);
    for (size_t i = 0; i < allPsalms.size(); ++i) {
        const auto& p = allPsalms[i];
        std::cout << BOLD << BRIGHT_CYAN << "  " << std::setw(3) << (i+1) << ". " << RESET
                  << BRIGHT_YELLOW << "Psalm " << std::setw(3) << p.number << "  " << RESET
                  << BRIGHT_WHITE << std::left << std::setw(30) << p.title
                  << DIM << CYAN << "  " << p.theme << RESET << "\n";
        sleep_ms(8);
    }
    std::cout << BOLD << RED << "\n  [0] Back to Main Menu\n" << RESET;
    std::cout << BRIGHT_YELLOW << BOLD << "\n  Choose Psalm (1-150): " << RESET;
}

void displayPsalm(const Psalm& p) {
    clearScreen();
    std::cout << "\n";
    printLine("  +-----------------------------------------------------------------+", BRIGHT_YELLOW, true);
    std::string hdr = "  Psalm " + std::to_string(p.number) + " -- " + p.title;
    std::cout << BOLD << BRIGHT_YELLOW << "  |  " << BRIGHT_WHITE;
    typewrite(hdr, 12, BRIGHT_WHITE);
    int pad1 = 61 - (int)hdr.size();
    std::cout << std::string(std::max(0,pad1), ' ') << BRIGHT_YELLOW << "  |\n" << RESET;
    std::cout << BOLD << BRIGHT_YELLOW << "  |  " << DIM << CYAN << "Theme: " << p.theme
              << std::string(std::max(0,54-(int)p.theme.size()), ' ')
              << BRIGHT_YELLOW << "  |\n" << RESET;
    printLine("  +-----------------------------------------------------------------+", BRIGHT_YELLOW, true);
    std::cout << BOLD << BRIGHT_YELLOW << "  |  " << BRIGHT_CYAN << "KEY VERSE:\n" << RESET;
    auto kwl = wordWrap(p.keyVerse, 62);
    for (auto& kl : kwl) {
        std::cout << BOLD << BRIGHT_YELLOW << "  |  " << RESET << ITALIC << BRIGHT_WHITE << kl
                  << std::string(std::max(0,62-(int)kl.size()), ' ')
                  << BOLD << BRIGHT_YELLOW << "  |\n" << RESET;
    }
    printLine("  +-----------------------------------------------------------------+", BRIGHT_YELLOW, true);
    for (const auto& line : p.fullText) {
        auto wl = wordWrap(line, 62);
        for (size_t j = 0; j < wl.size(); ++j) {
            std::cout << BOLD << BRIGHT_YELLOW << "  |  " << RESET;
            std::string col = (j==0 && line[0]>='0' && line[0]<='9') ? BRIGHT_YELLOW : BRIGHT_WHITE;
            typewrite(wl[j], 12, col);
            std::cout << std::string(std::max(0,62-(int)wl[j].size()), ' ')
                      << BOLD << BRIGHT_YELLOW << "  |\n" << RESET;
        }
        sleep_ms(25);
    }
    printLine("  +-----------------------------------------------------------------+", BRIGHT_YELLOW, true);
}

void showVersiclesMenu() {
    clearScreen();
    sectionHeader("VERSICLES & RESPONSIVE READINGS", "R");
    printLine("  Liturgical call-and-response prayers of the Church.\n", BRIGHT_CYAN, true);
    for (size_t i = 0; i < versicles.size(); ++i) {
        std::cout << BOLD << BRIGHT_CYAN << "  [" << (i+1) << "] " << RESET
                  << BRIGHT_WHITE << versicles[i].title
                  << DIM << CYAN << "  (" << versicles[i].reference << ")\n" << RESET;
    }
    std::cout << BOLD << RED << "\n  [0] Back\n" << RESET;
    std::cout << BRIGHT_YELLOW << BOLD << "\n  Choose versicle (1-" << versicles.size() << "): " << RESET;
}

void displayVersicle(const Versicle& v) {
    clearScreen();
    sectionHeader(v.title + "  --  " + v.reference, "R");
    std::cout << "  " << BOLD << BRIGHT_CYAN << "L:" << RESET << " = Leader    "
              << BOLD << BRIGHT_GREEN << "R:" << RESET << " = Response    "
              << BOLD << BRIGHT_YELLOW << "ALL:" << RESET << " = Congregation\n\n";
    for (const auto& [role, text] : v.lines) {
        sleep_ms(110);
        std::string col = (role=="L:") ? BRIGHT_CYAN : (role=="R:") ? BRIGHT_GREEN : BRIGHT_YELLOW;
        std::cout << "\n  " << BOLD << col << std::setw(5) << role << "  " << RESET;
        typewrite(text, 20, ITALIC + std::string(BRIGHT_WHITE));
        std::cout << "\n";
    }
    std::cout << "\n";
}

void showProverbs() {
    clearScreen();
    sectionHeader("THE BOOK OF PROVERBS -- Wisdom of Solomon", "P");
    printLine("  'The fear of the LORD is the beginning of wisdom.' -- Prov. 9:10\n", BRIGHT_CYAN, true);
    for (const auto& p : proverbs) {
        std::cout << BOLD << BRIGHT_YELLOW << "  Proverbs " << std::left << std::setw(8) << p.ref
                  << DIM << CYAN << "  [" << p.theme << "]\n" << RESET;
        for (auto& l : wordWrap(p.text, 65))
            std::cout << "  " << ITALIC << BRIGHT_WHITE << l << "\n" << RESET;
        std::cout << "\n";
        sleep_ms(40);
    }
}

void showVerses() {
    clearScreen();
    sectionHeader("FAMOUS VERSES OF HOLY SCRIPTURE", "*");
    printLine("  " + std::to_string(famousVerses.size()) + " key verses spanning the entire Bible.\n", BRIGHT_CYAN, true);
    for (size_t i = 0; i < famousVerses.size(); ++i) {
        const auto& v = famousVerses[i];
        std::cout << BOLD << BRIGHT_YELLOW << "  [" << std::setw(2) << (i+1) << "] "
                  << BRIGHT_WHITE << v.ref << RESET << DIM << CYAN << "  -- " << v.theme << "\n" << RESET;
        for (auto& l : wordWrap("\"" + v.text + "\"", 65))
            std::cout << "  " << ITALIC << BRIGHT_WHITE << l << "\n" << RESET;
        std::cout << "\n";
        sleep_ms(30);
    }
}

void showStoryMenu() {
    clearScreen();
    sectionHeader("GREAT STORIES OF THE BIBLE", "S");
    for (size_t i = 0; i < stories.size(); ++i)
        std::cout << BOLD << BRIGHT_CYAN << "  [" << (i+1) << "] " << RESET
                  << BRIGHT_WHITE << stories[i].title
                  << DIM << WHITE << "  (" << stories[i].book_ref << ")\n" << RESET;
    std::cout << BOLD << RED << "\n  [0] Back\n" << RESET;
    std::cout << BRIGHT_YELLOW << BOLD << "\n  Choose story: " << RESET;
}

void showStory(const Story& s) {
    clearScreen();
    sectionHeader(s.title + "  --  " + s.book_ref, "S");
    printLine("  +------------------------------------------------------------------+", YELLOW, true);
    for (const auto& line : s.lines) {
        std::cout << YELLOW << BOLD << "  |  " << RESET;
        typewrite(line, 15, BRIGHT_WHITE);
        std::cout << "\n"; sleep_ms(50);
    }
    printLine("  +------------------------------------------------------------------+", YELLOW, true);
}

void showCommandments() {
    clearScreen();
    sectionHeader("THE TEN COMMANDMENTS", "=");
    printLine("         Exodus 20:1-17  |  Deuteronomy 5:6-21\n", BRIGHT_CYAN, true);
    const std::vector<std::string> cmds = {
        "I.    You shall have no other gods before Me.",
        "II.   You shall not make or worship idols.",
        "III.  You shall not misuse the name of the LORD your God.",
        "IV.   Remember the Sabbath day by keeping it holy.",
        "V.    Honor your father and your mother.",
        "VI.   You shall not murder.",
        "VII.  You shall not commit adultery.",
        "VIII. You shall not steal.",
        "IX.   You shall not give false testimony against your neighbor.",
        "X.    You shall not covet anything that belongs to your neighbor.",
    };
    printLine("  +--------------------------------------------------------------+", BRIGHT_WHITE, true);
    printLine("  |         * * *   THE LAW OF GOD   * * *                      |", YELLOW, true);
    printLine("  +--------------------------------------------------------------+", BRIGHT_WHITE, true);
    for (const auto& c : cmds) {
        std::cout << BRIGHT_WHITE << BOLD << "  |  " << RESET;
        typewrite(c, 16, BRIGHT_YELLOW);
        int pad = 56 - (int)c.size();
        std::cout << std::string(std::max(0,pad), ' ') << BRIGHT_WHITE << BOLD << "  |\n" << RESET;
        sleep_ms(70);
    }
    printLine("  +--------------------------------------------------------------+", BRIGHT_WHITE, true);
}

void showLordsPrayer() {
    clearScreen();
    sectionHeader("THE LORD'S PRAYER", "~");
    printLine("              Matthew 6:9-13  |  Luke 11:2-4\n", BRIGHT_CYAN, true);
    const std::vector<std::pair<std::string,std::string>> prayer = {
        {"Address",    "Our Father, who art in heaven,"},
        {"",           "hallowed be Thy name."},
        {"Kingdom",    "Thy Kingdom come, Thy will be done,"},
        {"",           "on earth as it is in heaven."},
        {"Provision",  "Give us this day our daily bread,"},
        {"Forgiveness","and forgive us our trespasses,"},
        {"",           "as we forgive those who trespass against us."},
        {"Protection", "And lead us not into temptation,"},
        {"",           "but deliver us from evil."},
        {"Doxology",   "For Thine is the Kingdom,"},
        {"",           "the Power, and the Glory,"},
        {"",           "forever and ever."},
        {"",           "Amen."},
    };
    printLine("  +----------------------------------------------------------+", MAGENTA, true);
    for (const auto& [label, line] : prayer) {
        std::cout << MAGENTA << BOLD << "  |  " << RESET;
        typewrite(line, 18, BRIGHT_WHITE);
        if (!label.empty()) std::cout << DIM << CYAN << "  <- " << label << RESET;
        std::cout << "\n"; sleep_ms(90);
    }
    printLine("  +----------------------------------------------------------+", MAGENTA, true);
}

void showBeatitudes() {
    clearScreen();
    sectionHeader("THE BEATITUDES -- Sermon on the Mount", "^");
    printLine("                    Matthew 5:3-12\n", BRIGHT_CYAN, true);
    const std::vector<std::string> bs = {
        "Blessed are the poor in spirit -- theirs is the kingdom of heaven.",
        "Blessed are those who mourn -- for they will be comforted.",
        "Blessed are the meek -- for they will inherit the earth.",
        "Blessed are those who hunger and thirst for righteousness -- they will be filled.",
        "Blessed are the merciful -- for they will be shown mercy.",
        "Blessed are the pure in heart -- for they will see God.",
        "Blessed are the peacemakers -- they will be called children of God.",
        "Blessed are those persecuted for righteousness -- theirs is the kingdom of heaven.",
        "Blessed are you when people persecute you for my sake -- great is your reward in heaven!",
    };
    for (const auto& b : bs) {
        std::cout << "\n  " << BRIGHT_YELLOW << BOLD << "*  " << RESET;
        typewrite(b, 16, BRIGHT_WHITE); std::cout << "\n"; sleep_ms(80);
    }
    std::cout << "\n"; printRule(72, YELLOW);
}

void showEpistlesMenu() {
    clearScreen();
    sectionHeader("EPISTLES OF THE NEW TESTAMENT", "E");
    for (size_t i = 0; i < epistles.size(); ++i)
        std::cout << BOLD << BRIGHT_CYAN << "  [" << (i+1) << "] " << RESET
                  << BRIGHT_WHITE << epistles[i].book << "  --  "
                  << BRIGHT_YELLOW << epistles[i].chapter
                  << DIM << CYAN << "  \"" << epistles[i].topic << "\"\n" << RESET;
    std::cout << BOLD << RED << "\n  [0] Back\n" << RESET;
    std::cout << BRIGHT_YELLOW << BOLD << "\n  Choose epistle: " << RESET;
}

void displayEpistle(const Epistle& e) {
    clearScreen();
    sectionHeader(e.book + " -- " + e.chapter, "E");
    printLine("  Topic: " + e.topic + "\n", BRIGHT_CYAN, true);
    printLine("  +--------------------------------------------------------------------+", MAGENTA, true);
    for (const auto& v : e.verses) {
        std::cout << MAGENTA << BOLD << "  |  " << RESET;
        typewrite(v, 14, BRIGHT_WHITE); std::cout << "\n"; sleep_ms(50);
    }
    printLine("  +--------------------------------------------------------------------+", MAGENTA, true);
}

void showGospelsMenu() {
    clearScreen();
    sectionHeader("THE FOUR GOSPELS -- Life of Jesus Christ", "+");
    printLine("  Matthew | Mark | Luke | John\n", BRIGHT_CYAN, true);
    for (size_t i = 0; i < gospelSections.size(); ++i)
        std::cout << BOLD << BRIGHT_CYAN << "  [" << (i+1) << "] " << RESET
                  << BRIGHT_WHITE << gospelSections[i].title
                  << DIM << CYAN << "  (" << gospelSections[i].ref << ")\n" << RESET;
    std::cout << BOLD << RED << "\n  [0] Back\n" << RESET;
    std::cout << BRIGHT_YELLOW << BOLD << "\n  Choose passage: " << RESET;
}

void displayGospelSection(const GospelSection& g) {
    clearScreen();
    sectionHeader(g.title + "  --  " + g.ref, "+");
    printLine("  +------------------------------------------------------------------+", BRIGHT_CYAN, true);
    for (const auto& line : g.lines) {
        std::cout << BRIGHT_CYAN << BOLD << "  |  " << RESET;
        typewrite(line, 14, BRIGHT_WHITE); std::cout << "\n"; sleep_ms(50);
    }
    printLine("  +------------------------------------------------------------------+", BRIGHT_CYAN, true);
}

void verseOfTheDay() {
    clearScreen();
    sectionHeader("VERSE OF THE DAY", "*");
    srand((unsigned)time(nullptr));
    const auto& v = famousVerses[rand() % famousVerses.size()];
    std::cout << "\n";
    verseBox(v.ref, "\"" + v.text + "\"", BRIGHT_YELLOW, BRIGHT_WHITE, 64);
    std::cout << "\n  " << BRIGHT_CYAN << "Theme: " << BOLD << v.theme << RESET << "\n";
}

void showPrayerCorner() {
    clearScreen();
    sectionHeader("THE PRAYER CORNER", "~");
    printLine("  'Pray without ceasing.' -- 1 Thessalonians 5:17\n", BRIGHT_CYAN, true);
    struct Prayer { std::string name, text; };
    const std::vector<Prayer> prayers = {
        {"Morning Prayer",
         "Lord, as I rise this day, I thank You for the gift of life. Guide my steps, guard my heart, "
         "and let my words and deeds bring glory to You. Fill me with Your Spirit, grant me wisdom, "
         "and keep me in Your perfect peace. Amen."},
        {"Evening Prayer",
         "Heavenly Father, as the day ends I come to You in gratitude. Forgive me for where I fell short. "
         "Thank You for Your mercies which are new every morning. Watch over me and all whom I love "
         "through this night. In Jesus' name, Amen."},
        {"Prayer for Strength",
         "O Lord, I am weak but You are strong. When my burdens feel too heavy, remind me that Your yoke "
         "is easy and Your burden is light. Be my strength and my shield. I can do all things through "
         "Christ who strengthens me. Amen."},
        {"Prayer for Others",
         "Father in heaven, I lift up all who are suffering, sick, lonely, or afraid. Let Your healing "
         "hand touch them. Give comfort to the grieving, hope to the hopeless, and provision to the poor. "
         "Let Your kingdom come in their lives. Amen."},
        {"Prayer of Thanksgiving",
         "Thank You, Lord, for every good and perfect gift. For breath in my lungs, for the beauty "
         "of Your creation, for the sacrifice of Your Son, for Your Word, and for the hope of heaven. "
         "May gratitude be the song of my life. Amen."},
        {"Prayer for Wisdom",
         "Lord God, You have said that if any lacks wisdom, he should ask of You who gives generously "
         "to all without finding fault. I ask now for Your wisdom in every decision I face. "
         "Let me hear Your voice and walk in Your ways. Amen."},
        {"Prayer of Dedication",
         "Heavenly Father, I present myself to You as a living sacrifice, holy and acceptable. "
         "Let me be transformed by the renewing of my mind, that I may prove what is Your good "
         "and acceptable and perfect will. Use me for Your glory. Amen."},
    };
    for (const auto& p : prayers) {
        std::cout << BOLD << BRIGHT_MAGENTA << "\n  +  " << p.name << "\n" << RESET;
        printLine("  +------------------------------------------------------------------+", MAGENTA, true);
        for (auto& l : wordWrap(p.text, 64))
            std::cout << MAGENTA << BOLD << "  |  " << RESET << ITALIC << BRIGHT_WHITE << l << "\n" << RESET;
        printLine("  +------------------------------------------------------------------+", MAGENTA, true);
        sleep_ms(30);
    }
}

void showTimeline() {
    clearScreen();
    sectionHeader("BIBLE TIMELINE -- Creation to Revelation", "T");
    struct Event { std::string date, event, ref; };
    const std::vector<Event> timeline = {
        {"~4000 BC","Creation of the world","Genesis 1"},
        {"~3000 BC","Adam and Eve in the Garden","Genesis 2-3"},
        {"~2350 BC","Noah's Flood","Genesis 6-9"},
        {"~2091 BC","God calls Abraham from Ur","Genesis 12"},
        {"~2066 BC","Birth of Isaac — the promised son","Genesis 21"},
        {"~2006 BC","Birth of Jacob and Esau","Genesis 25"},
        {"~1915 BC","Joseph sold into slavery in Egypt","Genesis 37"},
        {"~1526 BC","Birth of Moses in Egypt","Exodus 2"},
        {"~1446 BC","The Exodus from Egypt","Exodus 12-14"},
        {"~1446 BC","Moses receives the Law at Sinai","Exodus 19-20"},
        {"~1406 BC","Israel enters Canaan","Joshua 3"},
        {"~1050 BC","Saul becomes Israel's first king","1 Samuel 10"},
        {"~1010 BC","David becomes king of all Israel","2 Samuel 5"},
        {"~970 BC", "Solomon builds the First Temple","1 Kings 6"},
        {"~930 BC", "Kingdom divides: Israel and Judah","1 Kings 12"},
        {"~722 BC", "Fall of Northern Kingdom (Israel)","2 Kings 17"},
        {"~586 BC", "Babylon destroys Jerusalem","2 Kings 25"},
        {"~538 BC", "Jews return from Babylonian exile","Ezra 1"},
        {"~516 BC", "Second Temple completed in Jerusalem","Ezra 6"},
        {"~445 BC", "Nehemiah rebuilds Jerusalem's walls","Nehemiah 2"},
        {"~5 BC",   "Birth of Jesus Christ in Bethlehem","Luke 2"},
        {"~27 AD",  "Jesus begins His public ministry","Luke 3"},
        {"~30 AD",  "Death and Resurrection of Jesus","John 19-20"},
        {"~33 AD",  "Pentecost -- birth of the Church","Acts 2"},
        {"~35 AD",  "Conversion of the Apostle Paul","Acts 9"},
        {"~47 AD",  "Paul's first missionary journey","Acts 13"},
        {"~57 AD",  "Paul writes Romans from Corinth","Romans"},
        {"~62 AD",  "Paul imprisoned in Rome","Acts 28"},
        {"~70 AD",  "Romans destroy Jerusalem and Temple","Matthew 24"},
        {"~95 AD",  "John writes Revelation on Patmos","Revelation 1"},
    };
    std::cout << "\n";
    for (size_t i = 0; i < timeline.size(); ++i) {
        const auto& e = timeline[i];
        std::cout << BOLD << BRIGHT_YELLOW << "  " << std::setw(9) << e.date << "  " << RESET;
        std::cout << BRIGHT_CYAN << "-->  " << RESET;
        std::cout << BRIGHT_WHITE << BOLD << e.event << RESET;
        std::cout << DIM << CYAN << "  [" << e.ref << "]\n" << RESET;
        if (i < timeline.size()-1) std::cout << "               |\n";
        sleep_ms(35);
    }
    std::cout << "\n";
    printLine("  'All scripture is given by inspiration of God.' -- 2 Timothy 3:16", BRIGHT_CYAN, true);
}

void showAttributesOfGod() {
    clearScreen();
    sectionHeader("THE ATTRIBUTES OF GOD", "+");
    struct Attr { std::string name, verse, desc; };
    const std::vector<Attr> attrs = {
        {"OMNISCIENT","Psalm 139:1-4","God knows all things — past, present, and future."},
        {"OMNIPOTENT","Jeremiah 32:17","Nothing is too hard for God; He is all-powerful."},
        {"OMNIPRESENT","Psalm 139:7-10","God is everywhere present at all times."},
        {"ETERNAL","Deuteronomy 33:27","God has no beginning and no end — He always was and is."},
        {"HOLY","Isaiah 6:3","God is perfectly pure, set apart from all sin and impurity."},
        {"LOVE","1 John 4:8","God is love — not just loving, but the very source of love."},
        {"JUST","Deuteronomy 32:4","God is perfectly fair — He rewards good and punishes evil."},
        {"MERCIFUL","Psalm 103:8","God is slow to anger, rich in lovingkindness and compassion."},
        {"FAITHFUL","Lamentations 3:23","His mercies are new every morning — great is His faithfulness!"},
        {"SOVEREIGN","Isaiah 46:10","God accomplishes all His purposes — none can thwart Him."},
        {"IMMUTABLE","Malachi 3:6","God does not change — same yesterday, today, and forever."},
        {"TRANSCENDENT","Isaiah 55:8-9","His ways and thoughts are higher than ours as heaven above earth."},
        {"IMMANENT","Acts 17:27-28","God is also near to each one of us — in Him we live and move."},
        {"GRACIOUS","Ephesians 2:8","By grace we are saved — unmerited favor freely given."},
        {"TRIUNE","Matthew 28:19","Father, Son, and Holy Spirit — three in one, one in three."},
    };
    std::cout << "\n";
    for (const auto& a : attrs) {
        std::cout << "\n  " << BOLD << BRIGHT_YELLOW << std::left << std::setw(16) << a.name
                  << BRIGHT_CYAN << a.verse << "\n" << RESET
                  << "  " << BRIGHT_WHITE << a.desc << "\n" << RESET;
        sleep_ms(45);
    }
}

void showFruitOfSpirit() {
    clearScreen();
    sectionHeader("THE FRUIT OF THE HOLY SPIRIT", "F");
    printLine("  Galatians 5:22-23\n", BRIGHT_CYAN, true);
    verseBox("Galatians 5:22-23",
        "The fruit of the Spirit is love, joy, peace, longsuffering, gentleness, "
        "goodness, faith, meekness, temperance: against such there is no law.",
        BRIGHT_GREEN, BRIGHT_WHITE, 60);
    std::cout << "\n";
    struct Fruit { std::string name, greek, desc; };
    const std::vector<Fruit> fruits = {
        {"LOVE",         "Agape",       "Unconditional love — the very nature of God (1 John 4:8)."},
        {"JOY",          "Chara",       "Deep gladness that transcends all circumstances (Phil 4:4)."},
        {"PEACE",        "Eirene",      "Wholeness and harmony with God and others (John 14:27)."},
        {"LONGSUFFERING","Makrothumia", "Patient endurance — bearing with others graciously (James 1)."},
        {"GENTLENESS",   "Chrestotes",  "Kindness in action — treating others with tender care."},
        {"GOODNESS",     "Agathosune",  "Moral excellence — doing right simply because it is right."},
        {"FAITH",        "Pistis",      "Faithfulness — reliability and trustworthiness in God."},
        {"MEEKNESS",     "Prautes",     "Strength under control — not weakness, but holy humility."},
        {"TEMPERANCE",   "Enkrateia",   "Self-control — mastery of desires, passions, and appetites."},
    };
    for (const auto& f : fruits) {
        std::cout << "  " << BOLD << BRIGHT_GREEN << std::left << std::setw(16) << f.name
                  << DIM << CYAN << std::setw(16) << f.greek << "\n" << RESET
                  << "  " << BRIGHT_WHITE << f.desc << "\n\n" << RESET;
        sleep_ms(40);
    }
}

void showNamesOfJesus() {
    clearScreen();
    sectionHeader("THE NAMES & TITLES OF JESUS CHRIST", "J");
    printLine("  Scripture reveals Jesus through many glorious names.\n", BRIGHT_CYAN, true);
    struct Name { std::string title, verse, meaning; };
    const std::vector<Name> names = {
        {"Jesus",            "Matthew 1:21",  "YHWH saves — His personal name given by the angel"},
        {"Christ",           "Matthew 16:16", "The Anointed One — the Messiah of Israel"},
        {"Son of God",       "John 1:34",     "The eternal Son, one with the Father"},
        {"Son of Man",       "Matthew 8:20",  "His humanity — fully man, identifying with us"},
        {"Emmanuel",         "Matthew 1:23",  "God with us — the Incarnate God"},
        {"The Word",         "John 1:1",      "The eternal Logos — God's self-expression"},
        {"Lamb of God",      "John 1:29",     "The sacrifice who takes away the sin of the world"},
        {"Good Shepherd",    "John 10:11",    "He knows His sheep and lays down His life for them"},
        {"Bread of Life",    "John 6:35",     "He alone satisfies the soul's deepest hunger"},
        {"Light of the World","John 8:12",    "He scatters spiritual darkness with His truth"},
        {"Resurrection & Life","John 11:25",  "Death has no power over Him or those in Him"},
        {"The Way, Truth, Life","John 14:6",  "The only path to the Father"},
        {"King of Kings",    "Revelation 19:16","Lord over all rulers and powers"},
        {"Alpha and Omega",  "Revelation 1:8","First and Last — He encompasses all eternity"},
        {"Prince of Peace",  "Isaiah 9:6",   "His kingdom brings ultimate and lasting peace"},
        {"Wonderful Counselor","Isaiah 9:6", "His wisdom surpasses all human understanding"},
        {"Mighty God",       "Isaiah 9:6",   "He is fully divine — God Almighty in human flesh"},
        {"Everlasting Father","Isaiah 9:6",  "He is the source and sustainer of eternal life"},
        {"Savior",           "Luke 2:11",    "He saves His people from their sins"},
        {"High Priest",      "Hebrews 4:14", "He intercedes for us before the Father"},
        {"Mediator",         "1 Timothy 2:5","The one go-between of God and man"},
        {"Lion of Judah",    "Revelation 5:5","He is sovereign, powerful, and victorious"},
    };
    for (const auto& n : names) {
        std::cout << "  " << BOLD << BRIGHT_YELLOW << std::left << std::setw(22) << n.title
                  << BRIGHT_CYAN << std::setw(18) << n.verse << "\n" << RESET
                  << "  " << DIM << WHITE << n.meaning << "\n\n" << RESET;
        sleep_ms(35);
    }
}

void showHolySpirit() {
    clearScreen();
    sectionHeader("THE HOLY SPIRIT -- The Third Person", "S");
    printLine("  What the Bible teaches about the Spirit of God.\n", BRIGHT_CYAN, true);
    struct HSFact { std::string aspect, verses, desc; };
    const std::vector<HSFact> facts = {
        {"He Is Personal",   "John 16:13-14","The Spirit speaks, hears, guides, and glorifies Christ."},
        {"He Is Divine",     "Acts 5:3-4",   "Lying to the Spirit is lying to God — He is fully God."},
        {"He Convicts",      "John 16:8",    "He convicts the world of sin, righteousness, and judgment."},
        {"He Regenerates",   "John 3:5-6",   "We must be born of the Spirit — He brings new birth."},
        {"He Indwells",      "1 Corinthians 6:19","Your body is a temple of the Holy Spirit."},
        {"He Seals",         "Ephesians 1:13","He seals believers as a guarantee of their inheritance."},
        {"He Sanctifies",    "2 Thessalonians 2:13","He sets believers apart for holiness."},
        {"He Prays",         "Romans 8:26",  "He intercedes for us with groans beyond words."},
        {"He Gives Gifts",   "1 Corinthians 12:4-11","He distributes spiritual gifts as He wills."},
        {"He Produces Fruit","Galatians 5:22-23","Love, joy, peace, and more flow from His presence."},
        {"He Empowers",      "Acts 1:8",     "'You will receive power when the Holy Spirit comes on you.'"},
        {"He Guides",        "John 16:13",   "'He will guide you into all truth.'"},
        {"He Came at Pentecost","Acts 2:1-4","Tongues of fire — He filled all the believers."},
        {"He Is Eternal",    "Hebrews 9:14", "The eternal Spirit offered Christ without blemish."},
    };
    for (const auto& f : facts) {
        std::cout << "\n  " << BOLD << BRIGHT_CYAN << std::left << std::setw(22) << f.aspect
                  << BRIGHT_YELLOW << f.verses << "\n" << RESET
                  << "  " << BRIGHT_WHITE << f.desc << "\n" << RESET;
        sleep_ms(40);
    }
}

void showSalvationPlan() {
    clearScreen();
    sectionHeader("THE PLAN OF SALVATION -- The Roman Road", "S");
    printLine("  God's perfect plan to save sinners through Jesus Christ.\n", BRIGHT_CYAN, true);
    struct Step { std::string ref, text, point; };
    const std::vector<Step> steps = {
        {"Romans 3:23","For all have sinned, and come short of the glory of God.","1. ALL HAVE SINNED"},
        {"Romans 6:23","For the wages of sin is death; but the gift of God is eternal life through Jesus Christ our Lord.","2. SIN'S PENALTY IS DEATH"},
        {"Romans 5:8", "But God commendeth his love toward us, in that, while we were yet sinners, Christ died for us.","3. CHRIST DIED FOR US"},
        {"Romans 10:9","That if thou shalt confess with thy mouth the Lord Jesus, and shalt believe in thine heart that God hath raised him from the dead, thou shalt be saved.","4. CONFESS AND BELIEVE"},
        {"Romans 10:13","For whosoever shall call upon the name of the Lord shall be saved.","5. CALL ON HIS NAME"},
        {"Romans 8:1","There is therefore now no condemnation to them which are in Christ Jesus.","6. FREE FROM CONDEMNATION"},
        {"Romans 8:38-39","Neither death, nor life...shall be able to separate us from the love of God.","7. NOTHING CAN SEPARATE US"},
    };
    for (const auto& s : steps) {
        std::cout << "\n  " << BOLD << BRIGHT_GREEN << s.point << "\n" << RESET;
        std::cout << "  " << BOLD << BRIGHT_YELLOW << s.ref << "\n" << RESET;
        for (auto& l : wordWrap("\"" + s.text + "\"", 65))
            std::cout << "  " << ITALIC << BRIGHT_WHITE << l << "\n" << RESET;
        sleep_ms(60);
    }
    std::cout << "\n\n";
    printRule(72, BRIGHT_GREEN);
    printLine("  If you would like to receive Jesus Christ as your Lord and Savior,", BRIGHT_WHITE, true);
    printLine("  simply pray: 'Lord Jesus, I know I am a sinner. I believe You died", BRIGHT_WHITE, true);
    printLine("  for my sins and rose from the dead. I turn from my sins and invite", BRIGHT_WHITE, true);
    printLine("  You into my heart as Lord and Savior. Thank You for saving me. Amen.'", BRIGHT_WHITE, true);
    printRule(72, BRIGHT_GREEN);
}

void drawBookCover() {
    printLine("  +----------------------------------------------------------+", YELLOW, true);
    printLine("  |                                                          |", YELLOW, true);
    printLine("  |       +  T H E   H O L Y   B I B L E  +                |", BRIGHT_WHITE, true);
    printLine("  |                                                          |", YELLOW, true);
    printLine("  |    Old Testament  *  39 Books  *  New Testament          |", BRIGHT_CYAN, true);
    printLine("  |                                                          |", YELLOW, true);
    printLine("  |    +--------------------------------------------------+  |", YELLOW, true);
    printLine("  |    |  'For God so loved the world, that He gave       |  |", BRIGHT_WHITE, true);
    printLine("  |    |   His only begotten Son, that whosoever          |  |", BRIGHT_WHITE, true);
    printLine("  |    |   believeth in Him should not perish,            |  |", BRIGHT_WHITE, true);
    printLine("  |    |   but have everlasting life.'                    |  |", BRIGHT_WHITE, true);
    printLine("  |    |                        -- John 3:16              |  |", BRIGHT_YELLOW, true);
    printLine("  |    +--------------------------------------------------+  |", YELLOW, true);
    printLine("  |                                                          |", YELLOW, true);
    printLine("  |   King James Version  |  66 Books  |  ~783,000 words    |", BRIGHT_MAGENTA, true);
    printLine("  |                                                          |", YELLOW, true);
    printLine("  +----------------------------------------------------------+", YELLOW, true);
}

void goodbyeScreen() {
    clearScreen();
    std::cout << "\n"; printRule(72, YELLOW);
    typewrite("\n       May the Lord bless you and keep you;\n", 20, BRIGHT_WHITE);
    typewrite("       May the Lord make His face shine on you\n", 20, BRIGHT_WHITE);
    typewrite("       and be gracious to you;\n", 20, BRIGHT_WHITE);
    typewrite("       May the Lord turn His face toward you\n", 20, BRIGHT_WHITE);
    typewrite("       and give you peace.\n", 20, BRIGHT_WHITE);
    printLine("\n                        -- Numbers 6:24-26\n", BRIGHT_YELLOW, true);
    printRule(72, YELLOW);
    std::cout << "\n";
    animateFlame(3);
    std::cout << "\n";
    typewrite("  +  God bless you. Come back and read His Word again!  +\n\n", 25, BRIGHT_YELLOW);
}

// ─── Main Menu ────────────────────────────────────────────
void showMenu() {
    clearScreen();
    std::cout << "\n"; drawCross(); std::cout << "\n";
    printLine("  +=========================================================================+", YELLOW, true);
    printLine("  |           +  COMPLETE HOLY BIBLE EXPLORER -- KJV  +                   |", BRIGHT_WHITE, true);
    printLine("  +=================================+=======================================+", YELLOW, true);
    printLine("  |  B  1. All 66 Books of the Bible |  ~  8. All 150 Psalms              |", BRIGHT_CYAN, true);
    printLine("  |  *  2. Famous Bible Verses (50+) |  R  9. Versicles & Responses (12)  |", BRIGHT_CYAN, true);
    printLine("  |  S  3. Bible Stories (9 stories) |  P 10. Proverbs of Solomon (34+)   |", BRIGHT_CYAN, true);
    printLine("  |  =  4. Ten Commandments          |  E 11. Epistles of Paul & others   |", BRIGHT_CYAN, true);
    printLine("  |  ~  5. The Lord's Prayer          |  +  12. Gospels & Jesus' Life      |", BRIGHT_CYAN, true);
    printLine("  |  ^  6. The Beatitudes             |  F 13. Fruit of the Holy Spirit    |", BRIGHT_CYAN, true);
    printLine("  |  *  7. Verse of the Day (random)  |  +  14. Attributes of God          |", BRIGHT_CYAN, true);
    printLine("  +=================================+=======================================+", YELLOW, true);
    printLine("  |  ~  15. Candlelight Meditation   |  T 16. Bible Timeline               |", BRIGHT_CYAN, true);
    printLine("  |  ~  17. Prayer Corner (7 prayers)|  J 18. Names & Titles of Jesus      |", BRIGHT_CYAN, true);
    printLine("  |  S  19. The Holy Spirit           |  S 20. The Plan of Salvation        |", BRIGHT_CYAN, true);
    printLine("  |  +  21. The Holy Bible Cover      |                                     |", BRIGHT_CYAN, true);
    printLine("  +=========================================================================+", YELLOW, true);
    printLine("  |                       X  0. Exit & God Bless                           |", RED, true);
    printLine("  +=========================================================================+", YELLOW, true);
    std::cout << "\n  " << BRIGHT_YELLOW << BOLD << "+  Your choice (0-21): " << RESET;
}

// ─── Sub-menu helper ──────────────────────────────────────
int pickFromList(const std::string& prompt, int max_items) {
    std::string c;
    std::getline(std::cin, c);
    try {
        int idx = std::stoi(c);
        if (idx == 0) return -1;
        if (idx >= 1 && idx <= max_items) return idx - 1;
    } catch(...) {}
    return -1;
}

// ==========================================================
//                         MAIN
// ==========================================================
int main() {
    openingCeremony();
    sleep_ms(500);

    bool running = true;
    while (running) {
        showMenu();
        std::string input;
        std::getline(std::cin, input);
        if (input.empty()) std::getline(std::cin, input);
        int choice = -1;
        try { choice = std::stoi(input); } catch(...) { choice = -1; }

        switch(choice) {
        case 1:  showBooks(); pressAnyKey(); break;
        case 2:  showVerses(); pressAnyKey(); break;
        case 3: {
            showStoryMenu();
            int idx = pickFromList("", (int)stories.size());
            if (idx >= 0) { showStory(stories[idx]); pressAnyKey(); }
            break;
        }
        case 4:  showCommandments(); pressAnyKey(); break;
        case 5:  showLordsPrayer(); pressAnyKey(); break;
        case 6:  showBeatitudes(); pressAnyKey(); break;
        case 7:  verseOfTheDay(); pressAnyKey(); break;
        case 8: {
            showPsalmsMenu();
            int idx = pickFromList("", (int)allPsalms.size());
            if (idx >= 0) { displayPsalm(allPsalms[idx]); pressAnyKey(); }
            break;
        }
        case 9: {
            showVersiclesMenu();
            int idx = pickFromList("", (int)versicles.size());
            if (idx >= 0) { displayVersicle(versicles[idx]); pressAnyKey(); }
            break;
        }
        case 10: showProverbs(); pressAnyKey(); break;
        case 11: {
            showEpistlesMenu();
            int idx = pickFromList("", (int)epistles.size());
            if (idx >= 0) { displayEpistle(epistles[idx]); pressAnyKey(); }
            break;
        }
        case 12: {
            showGospelsMenu();
            int idx = pickFromList("", (int)gospelSections.size());
            if (idx >= 0) { displayGospelSection(gospelSections[idx]); pressAnyKey(); }
            break;
        }
        case 13: showFruitOfSpirit(); pressAnyKey(); break;
        case 14: showAttributesOfGod(); pressAnyKey(); break;
        case 15: {
            clearScreen();
            sectionHeader("CANDLELIGHT MEDITATION", "~");
            printLine("  Take a quiet moment. Still your heart before the Lord.\n", BRIGHT_WHITE, true);
            animateFlame(4);
            std::cout << "\n";
            verseBox("Psalm 46:10","Be still, and know that I am God.",BRIGHT_YELLOW,BRIGHT_WHITE,50);
            pressAnyKey(); break;
        }
        case 16: showTimeline(); pressAnyKey(); break;
        case 17: showPrayerCorner(); pressAnyKey(); break;
        case 18: showNamesOfJesus(); pressAnyKey(); break;
        case 19: showHolySpirit(); pressAnyKey(); break;
        case 20: showSalvationPlan(); pressAnyKey(); break;
        case 21: {
            clearScreen();
            sectionHeader("THE HOLY BIBLE", "+");
            drawBookCover(); pressAnyKey(); break;
        }
        case 0: goodbyeScreen(); running = false; break;
        default:
            std::cout << "\n  " << BRIGHT_RED << "X Please choose 0-21.\n" << RESET;
            sleep_ms(700);
        }
    }
    std::cout << SHOW_CURSOR << RESET;
    return 0;
}
