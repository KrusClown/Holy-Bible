/*
 ==============================================================
   THE HOLY BIBLE - Complete Interactive Console Experience
   ASCII Art | Colors | Animations | All Books | Tree Index
   Data: BibleTree (BST map) + MapReduce helpers
   Compile: g++ -std=c++17 -o holy_bible holy_bible.cpp
   Run:     ./holy_bible
 ==============================================================
*/

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <functional>
#include <numeric>
#include <algorithm>
#include <memory>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

// ANSI Codes
#define RESET         "\033[0m"
#define BOLD          "\033[1m"
#define DIM           "\033[2m"
#define ITALIC        "\033[3m"
#define RED           "\033[31m"
#define GREEN         "\033[32m"
#define YELLOW        "\033[33m"
#define MAGENTA       "\033[35m"
#define CYAN          "\033[36m"
#define WHITE         "\033[37m"
#define BRIGHT_RED    "\033[91m"
#define BRIGHT_GREEN  "\033[92m"
#define BRIGHT_YELLOW "\033[93m"
#define BRIGHT_MAGENTA "\033[95m"
#define BRIGHT_CYAN   "\033[96m"
#define BRIGHT_WHITE  "\033[97m"
#define CLEAR_SCREEN  "\033[2J\033[H"
#define HIDE_CURSOR   "\033[?25l"
#define SHOW_CURSOR   "\033[?25h"

// ============================================================
//  SECTION 1: DATA STRUCTURES  (BibleTree + MapReduce)
// ============================================================

struct BibleVerse {
    int chapter, verse;
    std::string text;
};

// BibleBook: holds a balanced BST (std::map) of chapters -> verses
struct BibleBook {
    std::string name, testament, shortCode, description;
    // Chapter BST: O(log n) lookup by chapter number
    std::map<int, std::vector<BibleVerse>> chapters;

    // MAP: transform every verse into T
    template<typename T>
    std::vector<T> mapVerses(std::function<T(const BibleVerse&)> f) const {
        std::vector<T> out;
        for (auto& [ch,vs] : chapters)
            for (auto& v : vs) out.push_back(f(v));
        return out;
    }

    // REDUCE: fold all verses into one value
    template<typename T>
    T reduceVerses(T init, std::function<T(T, const BibleVerse&)> f) const {
        T acc = init;
        for (auto& [ch,vs] : chapters)
            for (auto& v : vs) acc = f(acc, v);
        return acc;
    }

    // FILTER: predicate-based extraction
    std::vector<BibleVerse> filterVerses(std::function<bool(const BibleVerse&)> p) const {
        std::vector<BibleVerse> out;
        for (auto& [ch,vs] : chapters)
            for (auto& v : vs) if (p(v)) out.push_back(v);
        return out;
    }

    int totalVerses()   const { return reduceVerses<int>(0,[](int a,const BibleVerse&){return a+1;}); }
    int totalChapters() const { return (int)chapters.size(); }
};

// BibleTree: root holding all 66 books with multi-index maps
class BibleTree {
public:
    std::vector<BibleBook>                    books;
    std::map<std::string,int>                 nameIndex;   // O(log n)
    std::map<std::string,int>                 codeIndex;
    std::map<std::string,std::vector<int>>    testamentIndex; // "OT"/"NT"

    void addBook(BibleBook b) {
        int idx = (int)books.size();
        nameIndex[b.name]      = idx;
        codeIndex[b.shortCode] = idx;
        testamentIndex[b.testament].push_back(idx);
        books.push_back(std::move(b));
    }

    BibleBook* find(const std::string& key) {
        auto it = nameIndex.find(key);
        if (it != nameIndex.end()) return &books[it->second];
        auto it2 = codeIndex.find(key);
        if (it2 != codeIndex.end()) return &books[it2->second];
        return nullptr;
    }

    // Global search via MapReduce grep
    std::vector<std::tuple<std::string,int,int,std::string>>
    search(const std::string& keyword) const {
        std::string kw = keyword;
        std::transform(kw.begin(),kw.end(),kw.begin(),::tolower);
        std::vector<std::tuple<std::string,int,int,std::string>> hits;
        for (auto& bk : books)
            for (auto& [ch,vs] : bk.chapters)
                for (auto& v : vs) {
                    std::string t = v.text;
                    std::transform(t.begin(),t.end(),t.begin(),::tolower);
                    if (t.find(kw) != std::string::npos)
                        hits.emplace_back(bk.name,ch,v.verse,v.text);
                    if ((int)hits.size() >= 25) return hits;
                }
        return hits;
    }

    int totalVerses() const {
        return std::accumulate(books.begin(),books.end(),0,
            [](int a,const BibleBook& b){return a+b.totalVerses();});
    }
    int totalChapters() const {
        return std::accumulate(books.begin(),books.end(),0,
            [](int a,const BibleBook& b){return a+b.totalChapters();});
    }
};

BibleTree g_bible;

// ============================================================
//  SECTION 2: UTILITY / DISPLAY
// ============================================================
void sleep_ms(int ms){ std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
void clearScreen(){ std::cout << CLEAR_SCREEN; std::cout.flush(); }

void typewrite(const std::string& s, int d=22, const std::string& col=""){
    for(char c:s){ if(!col.empty())std::cout<<col; std::cout<<c<<RESET; std::cout.flush(); sleep_ms(d); }
}
void printLine(const std::string& s, const std::string& col=WHITE, bool bold=false){
    if(bold) std::cout<<BOLD; std::cout<<col<<s<<RESET<<"\n";
}
void printRule(int w=74, const std::string& col=YELLOW){
    std::cout<<BOLD<<col<<std::string(w,'=')<<RESET<<"\n";
}
void animRule(int w=74,const std::string& col=YELLOW,int d=7){
    std::cout<<BOLD<<col;
    for(int i=0;i<w;++i){std::cout<<'=';std::cout.flush();sleep_ms(d);}
    std::cout<<RESET<<"\n";
}

std::vector<std::string> wordWrap(const std::string& text, int maxW){
    std::vector<std::string> lines;
    std::istringstream iss(text);
    std::string word,line;
    while(iss>>word){
        if(!line.empty()&&(int)(line.size()+1+word.size())>maxW){lines.push_back(line);line=word;}
        else{if(!line.empty())line+=" ";line+=word;}
    }
    if(!line.empty())lines.push_back(line);
    return lines;
}

void verseBox(const std::string& ref,const std::string& text,
              const std::string& fc=BRIGHT_YELLOW,const std::string& tc=BRIGHT_WHITE,int bw=62){
    auto lines=wordWrap(text,bw-4);
    std::cout<<BOLD<<fc<<"  +"<<std::string(bw,'-')<<"+\n"
             <<"  |"<<std::string(bw,' ')<<"|\n"<<RESET;
    int rp=std::max(0,(bw-(int)ref.size())/2);
    std::cout<<BOLD<<fc<<"  |"<<std::string(rp,' ')<<BRIGHT_WHITE<<ref
             <<std::string(std::max(0,bw-rp-(int)ref.size()),' ')
             <<fc<<"|\n"<<"  |"<<std::string(bw,' ')<<"|\n"
             <<"  +"<<std::string(bw,'-')<<"+\n"<<"  |"<<std::string(bw,' ')<<"|\n"<<RESET;
    for(auto& ln:lines){
        int pad=bw-2-(int)ln.size();
        std::cout<<BOLD<<fc<<"  | "<<RESET<<ITALIC<<tc<<ln<<std::string(std::max(0,pad),' ')
                 <<BOLD<<fc<<" |\n"<<RESET;
    }
    std::cout<<BOLD<<fc<<"  |"<<std::string(bw,' ')<<"|\n"<<"  +"<<std::string(bw,'-')<<"+\n"<<RESET;
}

void sectionHeader(const std::string& title,const std::string& icon="+"){
    std::cout<<"\n"; printRule(74,YELLOW);
    std::cout<<BOLD<<BRIGHT_YELLOW<<"  "<<icon<<"  ";
    typewrite(title,15,BRIGHT_WHITE);
    std::cout<<"  "<<BRIGHT_YELLOW<<icon<<"\n"<<RESET;
    printRule(74,YELLOW); std::cout<<"\n";
}

void pressAnyKey(){
    std::cout<<"\n"<<DIM<<CYAN<<"  [ Press ENTER to continue... ]"<<RESET;
    std::cin.ignore(100000,'\n'); std::cin.get();
}

void spinLoader(const std::string& msg,int ms=1400){
    const std::string fr="|/-\\"; std::cout<<HIDE_CURSOR;
    for(int i=0;i<ms/80;++i){
        std::cout<<"\r"<<BRIGHT_YELLOW<<BOLD<<fr[i%4]<<" "<<msg<<"  "<<RESET;
        std::cout.flush(); sleep_ms(80);
    }
    std::cout<<"\r"<<BRIGHT_GREEN<<BOLD<<"* "<<msg<<" -- Amen!\n"<<RESET;
    std::cout<<SHOW_CURSOR;
}

void starField(int rows=4,int cols=74,int frames=5){
    std::cout<<HIDE_CURSOR; srand(42);
    const char sc[]="*+.~";
    const std::string cc[]={BRIGHT_WHITE,BRIGHT_YELLOW,BRIGHT_CYAN,DIM WHITE};
    for(int f=0;f<frames;++f){
        if(f>0)std::cout<<"\033["<<rows<<"A";
        for(int r=0;r<rows;++r){
            for(int c=0;c<cols;++c)
                if(rand()%9==0)std::cout<<cc[rand()%4]<<sc[rand()%4]<<RESET;
                else std::cout<<" ";
            std::cout<<"\n";
        }
        std::cout.flush(); sleep_ms(180);
    }
    std::cout<<SHOW_CURSOR;
}

void animFlame(int sec=3){
    const std::vector<std::string> ff={
        "    (  )  \n   ( :: ) \n  ( :::  )\n  | ::: |\n  |/\\/\\/ |\n  `-----'",
        "    ( )   \n   (  :)  \n  (  ::  )\n  | ::: |\n  |/\\/\\/ |\n  `-----'",
        "   (   )  \n  ( :::)  \n (  :::  )\n  | ::: |\n  |/\\/\\/ |\n  `-----'",
    };
    std::cout<<HIDE_CURSOR;
    for(int t=0;t<sec*6;++t){
        if(t>0)std::cout<<"\033[6A";
        const std::string c=(t%3==0)?BRIGHT_YELLOW:(t%3==1)?BRIGHT_RED:YELLOW;
        std::cout<<c<<BOLD<<ff[t%3]<<RESET<<"\n"; std::cout.flush(); sleep_ms(155);
    }
    std::cout<<SHOW_CURSOR;
}

// ============================================================
//  SECTION 3: ORIGINAL BLOCK-LETTER ASCII BIBLE TITLE
// ============================================================
void drawBibleTitle(){
    const std::vector<std::string> art={
        "  \u2588\u2588\u2588\u2588\u2588\u2588\u2557 \u2588\u2588\u2557\u2588\u2588\u2588\u2588\u2588\u2588\u2557 \u2588\u2588\u2557     \u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557",
        "  \u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2557\u2588\u2588\u2551\u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2557\u2588\u2588\u2551     \u2588\u2588\u2554\u2550\u2550\u2550\u2550\u255d",
        "  \u2588\u2588\u2588\u2588\u2588\u2588\u2554\u255d\u2588\u2588\u2551\u2588\u2588\u2588\u2588\u2588\u2588\u2554\u255d\u2588\u2588\u2551     \u2588\u2588\u2588\u2588\u2588\u2557  ",
        "  \u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2557\u2588\u2588\u2551\u2588\u2588\u2554\u2550\u2550\u2588\u2588\u2557\u2588\u2588\u2551     \u2588\u2588\u2554\u2550\u2550\u255d  ",
        "  \u2588\u2588\u2588\u2588\u2588\u2588\u2554\u255d\u2588\u2588\u2551\u2588\u2588\u2588\u2588\u2588\u2588\u2554\u255d\u2588\u2588\u2588\u2588\u2588\u2588\u2557\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2557",
        "  \u255a\u2550\u2550\u2550\u2550\u2550\u255d \u255a\u2550\u255d\u255a\u2550\u2550\u2550\u2550\u2550\u255d \u255a\u2550\u2550\u2550\u2550\u2550\u255d\u255a\u2550\u2550\u2550\u2550\u2550\u2550\u255d",
    };
    const std::string grad[]={BRIGHT_YELLOW,YELLOW,BRIGHT_WHITE,YELLOW,BRIGHT_YELLOW,YELLOW};
    for(int i=0;i<(int)art.size();++i)
        typewrite(art[i]+"\n",4,grad[i]);
}

void drawCross(){
    const std::vector<std::string> c={
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
    const std::string cols[]={BRIGHT_YELLOW,YELLOW,BRIGHT_YELLOW,YELLOW,
        BRIGHT_WHITE,YELLOW,BRIGHT_YELLOW,BRIGHT_CYAN,BRIGHT_YELLOW,YELLOW};
    for(int i=0;i<(int)c.size();++i){
        std::cout<<BOLD<<cols[i]<<"          "<<c[i]<<RESET<<"\n"; sleep_ms(40);
    }
}

// ============================================================
//  SECTION 4: POPULATE THE BIBLE TREE  (all 66 books)
// ============================================================
void addV(BibleBook& b,int ch,int vn,const std::string& t){
    b.chapters[ch].push_back({ch,vn,t});
}

void buildBibleTree(){

// ── OLD TESTAMENT ─────────────────────────────────────────

{BibleBook b; b.name="Genesis"; b.shortCode="Gen"; b.testament="OT";
 b.description="Creation, Adam & Eve, Noah, Abraham, Joseph";
 addV(b,1,1,"In the beginning God created the heaven and the earth.");
 addV(b,1,2,"And the earth was without form, and void; and darkness was upon the face of the deep.");
 addV(b,1,3,"And God said, Let there be light: and there was light.");
 addV(b,1,26,"And God said, Let us make man in our image, after our likeness.");
 addV(b,1,27,"So God created man in his own image, in the image of God created he him; male and female created he them.");
 addV(b,1,31,"And God saw every thing that he had made, and, behold, it was very good.");
 addV(b,2,7,"And the LORD God formed man of the dust of the ground, and breathed into his nostrils the breath of life.");
 addV(b,2,18,"And the LORD God said, It is not good that the man should be alone; I will make him an help meet for him.");
 addV(b,2,24,"Therefore shall a man leave his father and his mother, and shall cleave unto his wife: and they shall be one flesh.");
 addV(b,3,15,"And I will put enmity between thee and the woman, and between thy seed and her seed; it shall bruise thy head.");
 addV(b,3,19,"In the sweat of thy face shalt thou eat bread, till thou return unto the ground; for out of it wast thou taken.");
 addV(b,6,8,"But Noah found grace in the eyes of the LORD.");
 addV(b,6,14,"Make thee an ark of gopher wood; rooms shalt thou make in the ark.");
 addV(b,6,22,"Thus did Noah; according to all that God commanded him, so did he.");
 addV(b,7,12,"And the rain was upon the earth forty days and forty nights.");
 addV(b,9,13,"I do set my bow in the cloud, and it shall be for a token of a covenant between me and the earth.");
 addV(b,12,1,"Now the LORD had said unto Abram, Get thee out of thy country, and from thy kindred, unto a land that I will shew thee.");
 addV(b,12,3,"And in thee shall all families of the earth be blessed.");
 addV(b,22,8,"And Abraham said, My son, God will provide himself a lamb for a burnt offering.");
 addV(b,22,14,"And Abraham called the name of that place Jehovahjireh: as it is said, In the mount of the LORD it shall be seen.");
 addV(b,37,28,"Then there passed by Midianites merchantmen; and his brethren sold Joseph for twenty pieces of silver.");
 addV(b,50,20,"But as for you, ye thought evil against me; but God meant it unto good, to bring to pass, as it is this day, to save much people alive.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Exodus"; b.shortCode="Exo"; b.testament="OT";
 b.description="Moses, Plagues, Passover, Ten Commandments";
 addV(b,3,2,"And the angel of the LORD appeared unto him in a flame of fire out of the midst of a bush.");
 addV(b,3,14,"And God said unto Moses, I AM THAT I AM: and he said, Thus shalt thou say unto the children of Israel, I AM hath sent me unto you.");
 addV(b,12,13,"And the blood shall be to you for a token upon the houses where ye are: and when I see the blood, I will pass over you.");
 addV(b,14,13,"And Moses said unto the people, Fear ye not, stand still, and see the salvation of the LORD.");
 addV(b,14,21,"And Moses stretched out his hand over the sea; and the LORD caused the sea to go back by a strong east wind all that night, and made the sea dry land, and the waters were divided.");
 addV(b,20,2,"I am the LORD thy God, which have brought thee out of the land of Egypt, out of the house of bondage.");
 addV(b,20,3,"Thou shalt have no other gods before me.");
 addV(b,20,4,"Thou shalt not make unto thee any graven image.");
 addV(b,20,7,"Thou shalt not take the name of the LORD thy God in vain.");
 addV(b,20,8,"Remember the sabbath day, to keep it holy.");
 addV(b,20,12,"Honour thy father and thy mother.");
 addV(b,20,13,"Thou shalt not kill.");
 addV(b,20,14,"Thou shalt not commit adultery.");
 addV(b,20,15,"Thou shalt not steal.");
 addV(b,20,16,"Thou shalt not bear false witness against thy neighbour.");
 addV(b,20,17,"Thou shalt not covet thy neighbour's house.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Leviticus"; b.shortCode="Lev"; b.testament="OT";
 b.description="Priestly laws, sacrifices, holiness codes";
 addV(b,11,44,"For I am the LORD your God: ye shall therefore sanctify yourselves, and ye shall be holy; for I am holy.");
 addV(b,19,18,"Thou shalt love thy neighbour as thyself: I am the LORD.");
 addV(b,19,32,"Thou shalt rise up before the hoary head, and honour the face of the old man, and fear thy God: I am the LORD.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Numbers"; b.shortCode="Num"; b.testament="OT";
 b.description="Israel in the wilderness, census, wandering";
 addV(b,6,24,"The LORD bless thee, and keep thee:");
 addV(b,6,25,"The LORD make his face shine upon thee, and be gracious unto thee:");
 addV(b,6,26,"The LORD lift up his countenance upon thee, and give thee peace.");
 addV(b,14,8,"If the LORD delight in us, then he will bring us into this land, and give it us; a land which floweth with milk and honey.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Deuteronomy"; b.shortCode="Deu"; b.testament="OT";
 b.description="Moses' farewell, law reiterated";
 addV(b,6,4,"Hear, O Israel: The LORD our God is one LORD:");
 addV(b,6,5,"And thou shalt love the LORD thy God with all thine heart, and with all thy soul, and with all thy might.");
 addV(b,31,6,"Be strong and of a good courage, fear not, nor be afraid of them: for the LORD thy God, he it is that doth go with thee; he will not fail thee, nor forsake thee.");
 addV(b,33,27,"The eternal God is thy refuge, and underneath are the everlasting arms.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Joshua"; b.shortCode="Jos"; b.testament="OT";
 b.description="Conquest of Canaan, the Promised Land";
 addV(b,1,8,"This book of the law shall not depart out of thy mouth; but thou shalt meditate therein day and night.");
 addV(b,1,9,"Have not I commanded thee? Be strong and of a good courage; be not afraid, neither be thou dismayed: for the LORD thy God is with thee whithersoever thou goest.");
 addV(b,24,15,"Choose you this day whom ye will serve; but as for me and my house, we will serve the LORD.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Judges"; b.shortCode="Jdg"; b.testament="OT";
 b.description="Cycles: sin, oppression, deliverance";
 addV(b,6,12,"And the angel of the LORD appeared unto him, and said unto him, The LORD is with thee, thou mighty man of valour.");
 addV(b,16,30,"And Samson said, Let me die with the Philistines. And he bowed himself with all his might; and the house fell upon the lords, and upon all the people that were therein.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Ruth"; b.shortCode="Rut"; b.testament="OT";
 b.description="Loyalty, redemption, ancestor of David";
 addV(b,1,16,"And Ruth said, Intreat me not to leave thee, or to return from following after thee: for whither thou goest, I will go; and where thou lodgest, I will lodge.");
 addV(b,1,17,"Where thou diest, will I die, and there will I be buried: the LORD do so to me, and more also, if ought but death part thee and me.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="1 Samuel"; b.shortCode="1Sa"; b.testament="OT";
 b.description="Samuel, Saul, and the rise of David";
 addV(b,16,7,"The LORD seeth not as man seeth; for man looketh on the outward appearance, but the LORD looketh on the heart.");
 addV(b,17,45,"Then said David to the Philistine, Thou comest to me with a sword, and with a spear: but I come to thee in the name of the LORD of hosts.");
 addV(b,17,47,"And all this assembly shall know that the LORD saveth not with sword and spear: for the battle is the LORD's.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="2 Samuel"; b.shortCode="2Sa"; b.testament="OT";
 b.description="David's reign, triumph and tragedy";
 addV(b,7,16,"And thine house and thy kingdom shall be established for ever before thee: thy throne shall be established for ever.");
 addV(b,22,2,"And he said, The LORD is my rock, and my fortress, and my deliverer;");
 addV(b,22,47,"The LORD liveth; and blessed be my rock; and exalted be the God of the rock of my salvation.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="1 Kings"; b.shortCode="1Ki"; b.testament="OT";
 b.description="Solomon, temple, Elijah, kingdom divided";
 addV(b,3,9,"Give therefore thy servant an understanding heart to judge thy people, that I may discern between good and bad.");
 addV(b,18,21,"And Elijah came unto all the people, and said, How long halt ye between two opinions? if the LORD be God, follow him: but if Baal, then follow him.");
 addV(b,18,39,"And when all the people saw it, they fell on their faces: and they said, The LORD, he is the God; the LORD, he is the God.");
 addV(b,19,12,"And after the earthquake a fire; but the LORD was not in the fire: and after the fire a still small voice.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="2 Kings"; b.shortCode="2Ki"; b.testament="OT";
 b.description="Elisha, fall of Israel and Judah";
 addV(b,2,11,"And it came to pass, as they still went on, and talked, that, behold, there appeared a chariot of fire, and horses of fire, and Elijah went up by a whirlwind into heaven.");
 addV(b,6,16,"And he answered, Fear not: for they that be with us are more than they that be with them.");
 addV(b,6,17,"And the LORD opened the eyes of the young man; and he saw: and, behold, the mountain was full of horses and chariots of fire round about Elisha.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="1 Chronicles"; b.shortCode="1Ch"; b.testament="OT";
 b.description="David's genealogy and temple preparations";
 addV(b,16,34,"O give thanks unto the LORD; for he is good; for his mercy endureth for ever.");
 addV(b,29,11,"Thine, O LORD, is the greatness, and the power, and the glory, and the victory, and the majesty: for all that is in the heaven and in the earth is thine.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="2 Chronicles"; b.shortCode="2Ch"; b.testament="OT";
 b.description="Solomon's temple through Judah's exile";
 addV(b,7,14,"If my people, which are called by my name, shall humble themselves, and pray, and seek my face, and turn from their wicked ways; then will I hear from heaven, and will forgive their sin, and will heal their land.");
 addV(b,20,15,"Thus saith the LORD unto you, Be not afraid nor dismayed by reason of this great multitude; for the battle is not yours, but God's.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Ezra"; b.shortCode="Ezr"; b.testament="OT";
 b.description="Return from exile, rebuilding the temple";
 addV(b,1,3,"Who is there among you of all his people? his God be with him, and let him go up to Jerusalem, which is in Judah, and build the house of the LORD God of Israel.");
 addV(b,7,10,"For Ezra had prepared his heart to seek the law of the LORD, and to do it, and to teach in Israel statutes and judgments.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Nehemiah"; b.shortCode="Neh"; b.testament="OT";
 b.description="Rebuilding Jerusalem's walls";
 addV(b,4,14,"Be not ye afraid of them: remember the LORD, which is great and terrible, and fight for your brethren, your sons, and your daughters, your wives, and your houses.");
 addV(b,8,10,"The joy of the LORD is your strength.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Esther"; b.shortCode="Est"; b.testament="OT";
 b.description="Esther saves the Jews from Haman's plot";
 addV(b,4,14,"And who knoweth whether thou art come to the kingdom for such a time as this?");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Job"; b.shortCode="Job"; b.testament="OT";
 b.description="Suffering, faith, and God's sovereignty";
 addV(b,1,21,"Naked came I out of my mother's womb, and naked shall I return thither: the LORD gave, and the LORD hath taken away; blessed be the name of the LORD.");
 addV(b,19,25,"For I know that my redeemer liveth, and that he shall stand at the latter day upon the earth.");
 addV(b,38,4,"Where wast thou when I laid the foundations of the earth? declare, if thou hast understanding.");
 addV(b,42,5,"I have heard of thee by the hearing of the ear: but now mine eye seeth thee.");
 addV(b,42,10,"And the LORD turned the captivity of Job, when he prayed for his friends: also the LORD gave Job twice as much as he had before.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Psalms"; b.shortCode="Psa"; b.testament="OT";
 b.description="150 songs of praise, lament, wisdom, worship";
 addV(b,1,1,"Blessed is the man that walketh not in the counsel of the ungodly, nor standeth in the way of sinners, nor sitteth in the seat of the scornful.");
 addV(b,1,2,"But his delight is in the law of the LORD; and in his law doth he meditate day and night.");
 addV(b,1,3,"And he shall be like a tree planted by the rivers of water, that bringeth forth his fruit in his season.");
 addV(b,8,1,"O LORD our Lord, how excellent is thy name in all the earth! who hast set thy glory above the heavens.");
 addV(b,8,4,"What is man, that thou art mindful of him? and the son of man, that thou visitest him?");
 addV(b,19,1,"The heavens declare the glory of God; and the firmament sheweth his handywork.");
 addV(b,19,14,"Let the words of my mouth, and the meditation of my heart, be acceptable in thy sight, O LORD, my strength, and my redeemer.");
 addV(b,22,1,"My God, my God, why hast thou forsaken me?");
 addV(b,23,1,"The LORD is my shepherd; I shall not want.");
 addV(b,23,2,"He maketh me to lie down in green pastures: he leadeth me beside the still waters.");
 addV(b,23,3,"He restoreth my soul: he leadeth me in the paths of righteousness for his name's sake.");
 addV(b,23,4,"Yea, though I walk through the valley of the shadow of death, I will fear no evil: for thou art with me; thy rod and thy staff they comfort me.");
 addV(b,23,5,"Thou preparest a table before me in the presence of mine enemies: thou anointest my head with oil; my cup runneth over.");
 addV(b,23,6,"Surely goodness and mercy shall follow me all the days of my life: and I will dwell in the house of the LORD for ever.");
 addV(b,24,1,"The earth is the LORD's, and the fulness thereof; the world, and they that dwell therein.");
 addV(b,27,1,"The LORD is my light and my salvation; whom shall I fear? the LORD is the strength of my life; of whom shall I be afraid?");
 addV(b,27,14,"Wait on the LORD: be of good courage, and he shall strengthen thine heart: wait, I say, on the LORD.");
 addV(b,30,5,"Weeping may endure for a night, but joy cometh in the morning.");
 addV(b,32,1,"Blessed is he whose transgression is forgiven, whose sin is covered.");
 addV(b,34,8,"O taste and see that the LORD is good: blessed is the man that trusteth in him.");
 addV(b,37,4,"Delight thyself also in the LORD; and he shall give thee the desires of thine heart.");
 addV(b,37,5,"Commit thy way unto the LORD; trust also in him; and he shall bring it to pass.");
 addV(b,40,1,"I waited patiently for the LORD; and he inclined unto me, and heard my cry.");
 addV(b,42,1,"As the hart panteth after the water brooks, so panteth my soul after thee, O God.");
 addV(b,46,1,"God is our refuge and strength, a very present help in trouble.");
 addV(b,46,10,"Be still, and know that I am God: I will be exalted among the heathen, I will be exalted in the earth.");
 addV(b,51,1,"Have mercy upon me, O God, according to thy lovingkindness: according unto the multitude of thy tender mercies blot out my transgressions.");
 addV(b,51,10,"Create in me a clean heart, O God; and renew a right spirit within me.");
 addV(b,51,17,"The sacrifices of God are a broken spirit: a broken and a contrite heart, O God, thou wilt not despise.");
 addV(b,63,1,"O God, thou art my God; early will I seek thee: my soul thirsteth for thee, my flesh longeth for thee in a dry and thirsty land.");
 addV(b,84,10,"For a day in thy courts is better than a thousand. I had rather be a doorkeeper in the house of my God, than to dwell in the tents of wickedness.");
 addV(b,90,1,"Lord, thou hast been our dwelling place in all generations.");
 addV(b,90,2,"Before the mountains were brought forth, or ever thou hadst formed the earth and the world, even from everlasting to everlasting, thou art God.");
 addV(b,90,12,"So teach us to number our days, that we may apply our hearts unto wisdom.");
 addV(b,91,1,"He that dwelleth in the secret place of the most High shall abide under the shadow of the Almighty.");
 addV(b,91,2,"I will say of the LORD, He is my refuge and my fortress: my God; in him will I trust.");
 addV(b,91,4,"He shall cover thee with his feathers, and under his wings shalt thou trust: his truth shall be thy shield and buckler.");
 addV(b,91,11,"For he shall give his angels charge over thee, to keep thee in all thy ways.");
 addV(b,100,1,"Make a joyful noise unto the LORD, all ye lands.");
 addV(b,100,3,"Know ye that the LORD he is God: it is he that hath made us, and not we ourselves; we are his people, and the sheep of his pasture.");
 addV(b,100,4,"Enter into his gates with thanksgiving, and into his courts with praise: be thankful unto him, and bless his name.");
 addV(b,100,5,"For the LORD is good; his mercy is everlasting; and his truth endureth to all generations.");
 addV(b,103,1,"Bless the LORD, O my soul: and all that is within me, bless his holy name.");
 addV(b,103,8,"The LORD is merciful and gracious, slow to anger, and plenteous in mercy.");
 addV(b,103,12,"As far as the east is from the west, so far hath he removed our transgressions from us.");
 addV(b,103,13,"Like as a father pitieth his children, so the LORD pitieth them that fear him.");
 addV(b,118,24,"This is the day which the LORD hath made; we will rejoice and be glad in it.");
 addV(b,119,9,"Wherewithal shall a young man cleanse his way? by taking heed thereto according to thy word.");
 addV(b,119,11,"Thy word have I hid in mine heart, that I might not sin against thee.");
 addV(b,119,105,"Thy word is a lamp unto my feet, and a light unto my path.");
 addV(b,121,1,"I will lift up mine eyes unto the hills, from whence cometh my help.");
 addV(b,121,2,"My help cometh from the LORD, which made heaven and earth.");
 addV(b,121,3,"He will not suffer thy foot to be moved: he that keepeth thee will not slumber.");
 addV(b,121,7,"The LORD shall preserve thee from all evil: he shall preserve thy soul.");
 addV(b,122,1,"I was glad when they said unto me, Let us go into the house of the LORD.");
 addV(b,127,1,"Except the LORD build the house, they labour in vain that build it.");
 addV(b,130,1,"Out of the depths have I cried unto thee, O LORD.");
 addV(b,130,4,"But there is forgiveness with thee, that thou mayest be feared.");
 addV(b,133,1,"Behold, how good and how pleasant it is for brethren to dwell together in unity!");
 addV(b,136,1,"O give thanks unto the LORD; for he is good: for his mercy endureth for ever.");
 addV(b,139,1,"O LORD, thou hast searched me, and known me.");
 addV(b,139,14,"I will praise thee; for I am fearfully and wonderfully made: marvellous are thy works; and that my soul knoweth right well.");
 addV(b,139,23,"Search me, O God, and know my heart: try me, and know my thoughts:");
 addV(b,145,3,"Great is the LORD, and greatly to be praised; and his greatness is unsearchable.");
 addV(b,145,18,"The LORD is nigh unto all them that call upon him, to all that call upon him in truth.");
 addV(b,147,3,"He healeth the broken in heart, and bindeth up their wounds.");
 addV(b,150,1,"Praise ye the LORD. Praise God in his sanctuary: praise him in the firmament of his power.");
 addV(b,150,6,"Let every thing that hath breath praise the LORD. Praise ye the LORD.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Proverbs"; b.shortCode="Pro"; b.testament="OT";
 b.description="Wisdom literature attributed to Solomon";
 addV(b,1,7,"The fear of the LORD is the beginning of knowledge: but fools despise wisdom and instruction.");
 addV(b,3,5,"Trust in the LORD with all thine heart; and lean not unto thine own understanding.");
 addV(b,3,6,"In all thy ways acknowledge him, and he shall direct thy paths.");
 addV(b,4,7,"Wisdom is the principal thing; therefore get wisdom: and with all thy getting get understanding.");
 addV(b,4,23,"Keep thy heart with all diligence; for out of it are the issues of life.");
 addV(b,9,10,"The fear of the LORD is the beginning of wisdom: and the knowledge of the holy is understanding.");
 addV(b,10,12,"Hatred stirreth up strifes: but love covereth all sins.");
 addV(b,11,2,"When pride cometh, then cometh shame: but with the lowly is wisdom.");
 addV(b,15,1,"A soft answer turneth away wrath: but grievous words stir up anger.");
 addV(b,16,18,"Pride goeth before destruction, and an haughty spirit before a fall.");
 addV(b,17,17,"A friend loveth at all times, and a brother is born for adversity.");
 addV(b,17,22,"A merry heart doeth good like a medicine: but a broken spirit drieth the bones.");
 addV(b,18,10,"The name of the LORD is a strong tower: the righteous runneth into it, and is safe.");
 addV(b,18,21,"Death and life are in the power of the tongue: and they that love it shall eat the fruit thereof.");
 addV(b,22,6,"Train up a child in the way he should go: and when he is old, he will not depart from it.");
 addV(b,25,11,"A word fitly spoken is like apples of gold in pictures of silver.");
 addV(b,29,18,"Where there is no vision, the people perish: but he that keepeth the law, happy is he.");
 addV(b,31,10,"Who can find a virtuous woman? for her price is far above rubies.");
 addV(b,31,30,"Favour is deceitful, and beauty is vain: but a woman that feareth the LORD, she shall be praised.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Ecclesiastes"; b.shortCode="Ecc"; b.testament="OT";
 b.description="Vanity of vanities — life under the sun";
 addV(b,1,2,"Vanity of vanities, saith the Preacher, vanity of vanities; all is vanity.");
 addV(b,3,1,"To every thing there is a season, and a time to every purpose under the heaven.");
 addV(b,3,11,"He hath made every thing beautiful in his time: also he hath set the world in their heart.");
 addV(b,12,1,"Remember now thy Creator in the days of thy youth, while the evil days come not.");
 addV(b,12,13,"Fear God, and keep his commandments: for this is the whole duty of man.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Song of Solomon"; b.shortCode="SoS"; b.testament="OT";
 b.description="Allegorical poem of love";
 addV(b,2,4,"He brought me to the banqueting house, and his banner over me was love.");
 addV(b,2,16,"My beloved is mine, and I am his: he feedeth among the lilies.");
 addV(b,8,6,"Set me as a seal upon thine heart: for love is strong as death.");
 addV(b,8,7,"Many waters cannot quench love, neither can the floods drown it.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Isaiah"; b.shortCode="Isa"; b.testament="OT";
 b.description="Messianic prophecies, comfort for Israel";
 addV(b,6,3,"And one cried unto another, and said, Holy, holy, holy, is the LORD of hosts: the whole earth is full of his glory.");
 addV(b,6,8,"Also I heard the voice of the Lord, saying, Whom shall I send, and who will go for us? Then said I, Here am I; send me.");
 addV(b,9,6,"For unto us a child is born, unto us a son is given: and the government shall be upon his shoulder: and his name shall be called Wonderful, Counsellor, The mighty God, The everlasting Father, The Prince of Peace.");
 addV(b,40,8,"The grass withereth, the flower fadeth: but the word of our God shall stand for ever.");
 addV(b,40,31,"But they that wait upon the LORD shall renew their strength; they shall mount up with wings as eagles; they shall run, and not be weary; and they shall walk, and not faint.");
 addV(b,41,10,"Fear thou not; for I am with thee: be not dismayed; for I am thy God: I will strengthen thee; yea, I will help thee; yea, I will uphold thee with the right hand of my righteousness.");
 addV(b,43,2,"When thou passest through the waters, I will be with thee; and through the rivers, they shall not overflow thee.");
 addV(b,53,3,"He is despised and rejected of men; a man of sorrows, and acquainted with grief.");
 addV(b,53,5,"But he was wounded for our transgressions, he was bruised for our iniquities: the chastisement of our peace was upon him; and with his stripes we are healed.");
 addV(b,53,6,"All we like sheep have gone astray; we have turned every one to his own way; and the LORD hath laid on him the iniquity of us all.");
 addV(b,55,8,"For my thoughts are not your thoughts, neither are your ways my ways, saith the LORD.");
 addV(b,55,9,"For as the heavens are higher than the earth, so are my ways higher than your ways, and my thoughts than your thoughts.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Jeremiah"; b.shortCode="Jer"; b.testament="OT";
 b.description="Weeping prophet, fall of Jerusalem";
 addV(b,1,5,"Before I formed thee in the belly I knew thee; and before thou camest forth out of the womb I sanctified thee, and I ordained thee a prophet unto the nations.");
 addV(b,29,11,"For I know the thoughts that I think toward you, saith the LORD, thoughts of peace, and not of evil, to give you an expected end.");
 addV(b,29,13,"And ye shall seek me, and find me, when ye shall search for me with all your heart.");
 addV(b,31,33,"I will put my law in their inward parts, and write it in their hearts; and will be their God, and they shall be my people.");
 addV(b,33,3,"Call unto me, and I will answer thee, and shew thee great and mighty things, which thou knowest not.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Lamentations"; b.shortCode="Lam"; b.testament="OT";
 b.description="Mourning the destruction of Jerusalem";
 addV(b,3,22,"It is of the LORD's mercies that we are not consumed, because his compassions fail not.");
 addV(b,3,23,"They are new every morning: great is thy faithfulness.");
 addV(b,3,25,"The LORD is good unto them that wait for him, to the soul that seeketh him.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Ezekiel"; b.shortCode="Eze"; b.testament="OT";
 b.description="Visions, Valley of Dry Bones, new temple";
 addV(b,36,26,"A new heart also will I give you, and a new spirit will I put within you: and I will take away the stony heart out of your flesh, and I will give you an heart of flesh.");
 addV(b,37,4,"Again he said unto me, Prophesy upon these bones, and say unto them, O ye dry bones, hear the word of the LORD.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Daniel"; b.shortCode="Dan"; b.testament="OT";
 b.description="Daniel in Babylon, lion's den, visions";
 addV(b,3,17,"If it be so, our God whom we serve is able to deliver us from the burning fiery furnace, and he will deliver us out of thine hand, O king.");
 addV(b,3,18,"But if not, be it known unto thee, O king, that we will not serve thy gods, nor worship the golden image which thou hast set up.");
 addV(b,6,10,"Now when Daniel knew that the writing was signed, he went into his house; and his windows being open in his chamber toward Jerusalem, he kneeled upon his knees three times a day, and prayed.");
 addV(b,6,22,"My God hath sent his angel, and hath shut the lions' mouths, that they have not hurt me.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Hosea"; b.shortCode="Hos"; b.testament="OT"; b.description="God's love for unfaithful Israel";
 addV(b,6,3,"Then shall we know, if we follow on to know the LORD: his going forth is prepared as the morning.");
 addV(b,14,9,"Who is wise, and he shall understand these things? for the ways of the LORD are right, and the just shall walk in them."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Joel"; b.shortCode="Joe"; b.testament="OT"; b.description="Day of the LORD, promise of the Spirit";
 addV(b,2,28,"And it shall come to pass afterward, that I will pour out my spirit upon all flesh; and your sons and your daughters shall prophesy.");
 addV(b,2,32,"And it shall come to pass, that whosoever shall call on the name of the LORD shall be delivered."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Amos"; b.shortCode="Amo"; b.testament="OT"; b.description="Justice for the poor, judgment on Israel";
 addV(b,5,24,"But let judgment run down as waters, and righteousness as a mighty stream."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Obadiah"; b.shortCode="Oba"; b.testament="OT"; b.description="Judgment against Edom";
 addV(b,1,3,"The pride of thine heart hath deceived thee.");
 addV(b,1,21,"And saviours shall come up on mount Zion to judge the mount of Esau; and the kingdom shall be the LORD's."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Jonah"; b.shortCode="Jon"; b.testament="OT"; b.description="Jonah, the whale, Nineveh's repentance";
 addV(b,1,17,"Now the LORD had prepared a great fish to swallow up Jonah. And Jonah was in the belly of the fish three days and three nights.");
 addV(b,4,2,"For I knew that thou art a gracious God, and merciful, slow to anger, and of great kindness, and repentest thee of the evil."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Micah"; b.shortCode="Mic"; b.testament="OT"; b.description="Justice, mercy, walking humbly with God";
 addV(b,5,2,"But thou, Bethlehem Ephratah, though thou be little among the thousands of Judah, yet out of thee shall he come forth unto me that is to be ruler in Israel.");
 addV(b,6,8,"He hath shewed thee, O man, what is good; and what doth the LORD require of thee, but to do justly, and to love mercy, and to walk humbly with thy God?"); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Nahum"; b.shortCode="Nah"; b.testament="OT"; b.description="Judgment on Nineveh";
 addV(b,1,7,"The LORD is good, a strong hold in the day of trouble; and he knoweth them that trust in him."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Habakkuk"; b.shortCode="Hab"; b.testament="OT"; b.description="The just shall live by his faith";
 addV(b,2,4,"The just shall live by his faith.");
 addV(b,3,17,"Although the fig tree shall not blossom, neither shall fruit be in the vines... yet I will rejoice in the LORD, I will joy in the God of my salvation.");
 addV(b,3,19,"The LORD God is my strength, and he will make my feet like hinds' feet."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Zephaniah"; b.shortCode="Zep"; b.testament="OT"; b.description="Day of the LORD, restoration";
 addV(b,3,17,"The LORD thy God in the midst of thee is mighty; he will save, he will rejoice over thee with joy; he will rest in his love, he will joy over thee with singing."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Haggai"; b.shortCode="Hag"; b.testament="OT"; b.description="Rebuilding the temple after exile";
 addV(b,2,4,"Be strong, O Zerubbabel, saith the LORD; and be strong, O Joshua, son of Josedech; and be strong, all ye people of the land, saith the LORD, and work.");
 addV(b,2,9,"The glory of this latter house shall be greater than of the former, saith the LORD of hosts."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Zechariah"; b.shortCode="Zec"; b.testament="OT"; b.description="Messianic visions and future glory";
 addV(b,4,6,"Not by might, nor by power, but by my spirit, saith the LORD of hosts.");
 addV(b,9,9,"Rejoice greatly, O daughter of Zion; shout, O daughter of Jerusalem: behold, thy King cometh unto thee: he is just, and having salvation; lowly, and riding upon an ass."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Malachi"; b.shortCode="Mal"; b.testament="OT"; b.description="Tithing, the coming Messenger";
 addV(b,3,6,"For I am the LORD, I change not.");
 addV(b,3,10,"Bring ye all the tithes into the storehouse, that there may be meat in mine house, and prove me now herewith, saith the LORD of hosts.");
 addV(b,4,2,"But unto you that fear my name shall the Sun of righteousness arise with healing in his wings."); g_bible.addBook(std::move(b));}

// ── NEW TESTAMENT ─────────────────────────────────────────

{BibleBook b; b.name="Matthew"; b.shortCode="Mat"; b.testament="NT";
 b.description="Gospel of Jesus: Sermon on the Mount, resurrection";
 addV(b,1,21,"And she shall bring forth a son, and thou shalt call his name JESUS: for he shall save his people from their sins.");
 addV(b,1,23,"Behold, a virgin shall be with child, and shall bring forth a son, and they shall call his name Emmanuel, which being interpreted is, God with us.");
 addV(b,5,3,"Blessed are the poor in spirit: for theirs is the kingdom of heaven.");
 addV(b,5,4,"Blessed are they that mourn: for they shall be comforted.");
 addV(b,5,5,"Blessed are the meek: for they shall inherit the earth.");
 addV(b,5,6,"Blessed are they which do hunger and thirst after righteousness: for they shall be filled.");
 addV(b,5,7,"Blessed are the merciful: for they shall obtain mercy.");
 addV(b,5,8,"Blessed are the pure in heart: for they shall see God.");
 addV(b,5,9,"Blessed are the peacemakers: for they shall be called the children of God.");
 addV(b,5,13,"Ye are the salt of the earth: but if the salt have lost his savour, wherewith shall it be salted?");
 addV(b,5,14,"Ye are the light of the world. A city that is set on an hill cannot be hid.");
 addV(b,5,44,"But I say unto you, Love your enemies, bless them that curse you, do good to them that hate you.");
 addV(b,6,9,"After this manner therefore pray ye: Our Father which art in heaven, Hallowed be thy name.");
 addV(b,6,10,"Thy kingdom come. Thy will be done in earth, as it is in heaven.");
 addV(b,6,11,"Give us this day our daily bread.");
 addV(b,6,12,"And forgive us our debts, as we forgive our debtors.");
 addV(b,6,13,"And lead us not into temptation, but deliver us from evil: For thine is the kingdom, and the power, and the glory, for ever. Amen.");
 addV(b,6,33,"But seek ye first the kingdom of God, and his righteousness; and all these things shall be added unto you.");
 addV(b,11,28,"Come unto me, all ye that labour and are heavy laden, and I will give you rest.");
 addV(b,11,29,"Take my yoke upon you, and learn of me; for I am meek and lowly in heart: and ye shall find rest unto your souls.");
 addV(b,28,18,"All power is given unto me in heaven and in earth.");
 addV(b,28,19,"Go ye therefore, and teach all nations, baptizing them in the name of the Father, and of the Son, and of the Holy Ghost:");
 addV(b,28,20,"Teaching them to observe all things whatsoever I have commanded you: and, lo, I am with you alway, even unto the end of the world. Amen.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Mark"; b.shortCode="Mar"; b.testament="NT"; b.description="Action gospel — shortest, most urgent";
 addV(b,1,15,"The time is fulfilled, and the kingdom of God is at hand: repent ye, and believe the gospel.");
 addV(b,10,45,"For even the Son of man came not to be ministered unto, but to minister, and to give his life a ransom for many.");
 addV(b,16,15,"And he said unto them, Go ye into all the world, and preach the gospel to every creature.");
 addV(b,16,16,"He that believeth and is baptized shall be saved; but he that believeth not shall be damned."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Luke"; b.shortCode="Luk"; b.testament="NT"; b.description="Nativity, Good Samaritan, Prodigal Son";
 addV(b,1,37,"For with God nothing shall be impossible.");
 addV(b,2,10,"And the angel said unto them, Fear not: for, behold, I bring you good tidings of great joy, which shall be to all people.");
 addV(b,2,11,"For unto you is born this day in the city of David a Saviour, which is Christ the Lord.");
 addV(b,2,14,"Glory to God in the highest, and on earth peace, good will toward men.");
 addV(b,15,24,"For this my son was dead, and is alive again; he was lost, and is found. And they began to be merry.");
 addV(b,19,10,"For the Son of man is come to seek and to save that which was lost.");
 addV(b,23,34,"Then said Jesus, Father, forgive them; for they know not what they do."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="John"; b.shortCode="Joh"; b.testament="NT"; b.description="I AM sayings, love, the Word made flesh";
 addV(b,1,1,"In the beginning was the Word, and the Word was with God, and the Word was God.");
 addV(b,1,14,"And the Word was made flesh, and dwelt among us, (and we beheld his glory, the glory as of the only begotten of the Father,) full of grace and truth.");
 addV(b,3,16,"For God so loved the world, that he gave his only begotten Son, that whosoever believeth in him should not perish, but have everlasting life.");
 addV(b,3,17,"For God sent not his Son into the world to condemn the world; but that the world through him might be saved.");
 addV(b,6,35,"And Jesus said unto them, I am the bread of life: he that cometh to me shall never hunger; and he that believeth on me shall never thirst.");
 addV(b,8,12,"Then spake Jesus again unto them, saying, I am the light of the world: he that followeth me shall not walk in darkness, but shall have the light of life.");
 addV(b,8,32,"And ye shall know the truth, and the truth shall make you free.");
 addV(b,10,10,"I am come that they might have life, and that they might have it more abundantly.");
 addV(b,10,11,"I am the good shepherd: the good shepherd giveth his life for the sheep.");
 addV(b,11,25,"Jesus said unto her, I am the resurrection, and the life: he that believeth in me, though he were dead, yet shall he live.");
 addV(b,11,35,"Jesus wept.");
 addV(b,13,34,"A new commandment I give unto you, That ye love one another; as I have loved you, that ye also love one another.");
 addV(b,14,1,"Let not your heart be troubled: ye believe in God, believe also in me.");
 addV(b,14,6,"Jesus saith unto him, I am the way, the truth, and the life: no man cometh unto the Father, but by me.");
 addV(b,14,27,"Peace I leave with you, my peace I give unto you: not as the world giveth, give I unto you. Let not your heart be troubled, neither let it be afraid.");
 addV(b,15,5,"I am the vine, ye are the branches. He that abideth in me, and I in him, the same bringeth forth much fruit: for without me ye can do nothing.");
 addV(b,15,13,"Greater love hath no man than this, that a man lay down his life for his friends.");
 addV(b,20,29,"Jesus saith unto him, Thomas, because thou hast seen me, thou hast believed: blessed are they that have not seen, and yet have believed.");
 addV(b,21,25,"And there are also many other things which Jesus did, the which, if they should be written every one, I suppose that even the world itself could not contain the books that should be written. Amen.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Acts"; b.shortCode="Act"; b.testament="NT"; b.description="Early church, Pentecost, Paul's journeys";
 addV(b,1,8,"But ye shall receive power, after that the Holy Ghost is come upon you: and ye shall be witnesses unto me both in Jerusalem, and in all Judaea, and in Samaria, and unto the uttermost part of the earth.");
 addV(b,2,4,"And they were all filled with the Holy Ghost, and began to speak with other tongues, as the Spirit gave them utterance.");
 addV(b,2,38,"Then Peter said unto them, Repent, and be baptized every one of you in the name of Jesus Christ for the remission of sins, and ye shall receive the gift of the Holy Ghost.");
 addV(b,4,12,"Neither is there salvation in any other: for there is none other name under heaven given among men, whereby we must be saved.");
 addV(b,9,5,"And he said, Who art thou, Lord? And the Lord said, I am Jesus whom thou persecutest.");
 addV(b,17,28,"For in him we live, and move, and have our being."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Romans"; b.shortCode="Rom"; b.testament="NT"; b.description="Justification by faith, life in the Spirit";
 addV(b,1,16,"For I am not ashamed of the gospel of Christ: for it is the power of God unto salvation to every one that believeth.");
 addV(b,3,23,"For all have sinned, and come short of the glory of God;");
 addV(b,5,8,"But God commendeth his love toward us, in that, while we were yet sinners, Christ died for us.");
 addV(b,6,23,"For the wages of sin is death; but the gift of God is eternal life through Jesus Christ our Lord.");
 addV(b,8,1,"There is therefore now no condemnation to them which are in Christ Jesus.");
 addV(b,8,28,"And we know that all things work together for good to them that love God, to them who are the called according to his purpose.");
 addV(b,8,31,"What shall we then say to these things? If God be for us, who can be against us?");
 addV(b,8,38,"For I am persuaded, that neither death, nor life, nor angels, nor principalities, nor powers, nor things present, nor things to come, shall be able to separate us from the love of God.");
 addV(b,10,9,"That if thou shalt confess with thy mouth the Lord Jesus, and shalt believe in thine heart that God hath raised him from the dead, thou shalt be saved.");
 addV(b,10,13,"For whosoever shall call upon the name of the Lord shall be saved.");
 addV(b,12,1,"I beseech you therefore, brethren, by the mercies of God, that ye present your bodies a living sacrifice, holy, acceptable unto God, which is your reasonable service.");
 addV(b,12,2,"And be not conformed to this world: but be ye transformed by the renewing of your mind, that ye may prove what is that good, and acceptable, and perfect, will of God.");
 g_bible.addBook(std::move(b));}

{BibleBook b; b.name="1 Corinthians"; b.shortCode="1Co"; b.testament="NT"; b.description="Love chapter, spiritual gifts, resurrection";
 addV(b,1,18,"For the preaching of the cross is to them that perish foolishness; but unto us which are saved it is the power of God.");
 addV(b,10,13,"There hath no temptation taken you but such as is common to man: but God is faithful, who will not suffer you to be tempted above that ye are able; but will with the temptation also make a way to escape.");
 addV(b,13,1,"Though I speak with the tongues of men and of angels, and have not charity, I am become as sounding brass, or a tinkling cymbal.");
 addV(b,13,4,"Charity suffereth long, and is kind; charity envieth not; charity vaunteth not itself, is not puffed up;");
 addV(b,13,7,"Beareth all things, believeth all things, hopeth all things, endureth all things.");
 addV(b,13,8,"Charity never faileth: but whether there be prophecies, they shall fail; whether there be tongues, they shall cease.");
 addV(b,13,13,"And now abideth faith, hope, charity, these three; but the greatest of these is charity.");
 addV(b,15,55,"O death, where is thy sting? O grave, where is thy victory?");
 addV(b,15,57,"But thanks be to God, which giveth us the victory through our Lord Jesus Christ."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="2 Corinthians"; b.shortCode="2Co"; b.testament="NT"; b.description="Strength in weakness, Paul's suffering";
 addV(b,5,17,"Therefore if any man be in Christ, he is a new creature: old things are passed away; behold, all things are become new.");
 addV(b,12,9,"And he said unto me, My grace is sufficient for thee: for my strength is made perfect in weakness."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Galatians"; b.shortCode="Gal"; b.testament="NT"; b.description="Freedom from law, fruit of the Spirit";
 addV(b,2,20,"I am crucified with Christ: nevertheless I live; yet not I, but Christ liveth in me.");
 addV(b,5,22,"But the fruit of the Spirit is love, joy, peace, longsuffering, gentleness, goodness, faith,");
 addV(b,5,23,"Meekness, temperance: against such there is no law.");
 addV(b,6,7,"Be not deceived; God is not mocked: for whatsoever a man soweth, that shall he also reap."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Ephesians"; b.shortCode="Eph"; b.testament="NT"; b.description="Armor of God, church as Christ's body";
 addV(b,2,8,"For by grace are ye saved through faith; and that not of yourselves: it is the gift of God:");
 addV(b,2,9,"Not of works, lest any man should boast.");
 addV(b,3,20,"Now unto him that is able to do exceeding abundantly above all that we ask or think, according to the power that worketh in us,");
 addV(b,6,10,"Finally, my brethren, be strong in the Lord, and in the power of his might.");
 addV(b,6,11,"Put on the whole armour of God, that ye may be able to stand against the wiles of the devil.");
 addV(b,6,17,"And take the helmet of salvation, and the sword of the Spirit, which is the word of God:"); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Philippians"; b.shortCode="Phi"; b.testament="NT"; b.description="Joy, contentment, I can do all things";
 addV(b,1,21,"For to me to live is Christ, and to die is gain.");
 addV(b,4,4,"Rejoice in the Lord alway: and again I say, Rejoice.");
 addV(b,4,6,"Be careful for nothing; but in every thing by prayer and supplication with thanksgiving let your requests be made known unto God.");
 addV(b,4,7,"And the peace of God, which passeth all understanding, shall keep your hearts and minds through Christ Jesus.");
 addV(b,4,8,"Finally, brethren, whatsoever things are true, honest, just, pure, lovely, of good report; if there be any virtue, and if there be any praise, think on these things.");
 addV(b,4,11,"I have learned, in whatsoever state I am, therewith to be content.");
 addV(b,4,13,"I can do all things through Christ which strengtheneth me.");
 addV(b,4,19,"But my God shall supply all your need according to his riches in glory by Christ Jesus."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Colossians"; b.shortCode="Col"; b.testament="NT"; b.description="Christ's supremacy, new life";
 addV(b,1,16,"For by him were all things created, that are in heaven, and that are in earth, visible and invisible.");
 addV(b,3,23,"And whatsoever ye do, do it heartily, as to the Lord, and not unto men;"); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="1 Thessalonians"; b.shortCode="1Th"; b.testament="NT"; b.description="Second coming, encouragement";
 addV(b,5,16,"Rejoice evermore.");
 addV(b,5,17,"Pray without ceasing.");
 addV(b,5,18,"In every thing give thanks: for this is the will of God in Christ Jesus concerning you."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="2 Thessalonians"; b.shortCode="2Th"; b.testament="NT"; b.description="Steadfastness, man of lawlessness";
 addV(b,3,3,"But the Lord is faithful, who shall stablish you, and keep you from evil."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="1 Timothy"; b.shortCode="1Ti"; b.testament="NT"; b.description="Church leadership, godliness";
 addV(b,2,5,"For there is one God, and one mediator between God and men, the man Christ Jesus;");
 addV(b,6,6,"But godliness with contentment is great gain.");
 addV(b,6,10,"For the love of money is the root of all evil."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="2 Timothy"; b.shortCode="2Ti"; b.testament="NT"; b.description="All Scripture is God-breathed";
 addV(b,1,7,"For God hath not given us the spirit of fear; but of power, and of love, and of a sound mind.");
 addV(b,3,16,"All scripture is given by inspiration of God, and is profitable for doctrine, for reproof, for correction, for instruction in righteousness:");
 addV(b,4,7,"I have fought a good fight, I have finished my course, I have kept the faith:"); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Titus"; b.shortCode="Tit"; b.testament="NT"; b.description="Sound doctrine, good works";
 addV(b,2,11,"For the grace of God that bringeth salvation hath appeared to all men,");
 addV(b,3,5,"Not by works of righteousness which we have done, but according to his mercy he saved us, by the washing of regeneration, and renewing of the Holy Ghost;"); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Philemon"; b.shortCode="Phm"; b.testament="NT"; b.description="Appeal for the slave Onesimus";
 addV(b,1,16,"Not now as a servant, but above a servant, a brother beloved."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Hebrews"; b.shortCode="Heb"; b.testament="NT"; b.description="Jesus as High Priest, hall of faith";
 addV(b,1,1,"God, who at sundry times and in divers manners spake in time past unto the fathers by the prophets,");
 addV(b,1,2,"Hath in these last days spoken unto us by his Son, whom he hath appointed heir of all things.");
 addV(b,4,12,"For the word of God is quick, and powerful, and sharper than any twoedged sword.");
 addV(b,4,16,"Let us therefore come boldly unto the throne of grace, that we may obtain mercy, and find grace to help in time of need.");
 addV(b,11,1,"Now faith is the substance of things hoped for, the evidence of things not seen.");
 addV(b,11,6,"But without faith it is impossible to please him: for he that cometh to God must believe that he is, and that he is a rewarder of them that diligently seek him.");
 addV(b,12,1,"Wherefore seeing we also are compassed about with so great a cloud of witnesses, let us lay aside every weight, and the sin which doth so easily beset us.");
 addV(b,12,2,"Looking unto Jesus the author and finisher of our faith; who for the joy that was set before him endured the cross.");
 addV(b,13,8,"Jesus Christ the same yesterday, and to day, and for ever."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="James"; b.shortCode="Jam"; b.testament="NT"; b.description="Faith without works is dead";
 addV(b,1,2,"My brethren, count it all joy when ye fall into divers temptations;");
 addV(b,1,5,"If any of you lack wisdom, let him ask of God, that giveth to all men liberally, and upbraideth not; and it shall be given him.");
 addV(b,1,17,"Every good gift and every perfect gift is from above, and cometh down from the Father of lights.");
 addV(b,2,17,"Even so faith, if it hath not works, is dead, being alone.");
 addV(b,4,7,"Submit yourselves therefore to God. Resist the devil, and he will flee from you.");
 addV(b,4,8,"Draw nigh to God, and he will draw nigh to you.");
 addV(b,5,16,"The effectual fervent prayer of a righteous man availeth much."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="1 Peter"; b.shortCode="1Pe"; b.testament="NT"; b.description="Suffering, hope, holy living";
 addV(b,2,9,"But ye are a chosen generation, a royal priesthood, an holy nation, a peculiar people; that ye should shew forth the praises of him who hath called you out of darkness into his marvellous light:");
 addV(b,5,7,"Casting all your care upon him; for he careth for you.");
 addV(b,5,8,"Be sober, be vigilant; because your adversary the devil, as a roaring lion, walketh about, seeking whom he may devour:"); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="2 Peter"; b.shortCode="2Pe"; b.testament="NT"; b.description="False teachers, Day of the Lord";
 addV(b,1,21,"For the prophecy came not in old time by the will of man: but holy men of God spake as they were moved by the Holy Ghost.");
 addV(b,3,9,"The Lord is not slack concerning his promise, as some men count slackness; but is longsuffering to us-ward, not willing that any should perish, but that all should come to repentance."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="1 John"; b.shortCode="1Jn"; b.testament="NT"; b.description="God is love, walking in the light";
 addV(b,1,9,"If we confess our sins, he is faithful and just to forgive us our sins, and to cleanse us from all unrighteousness.");
 addV(b,3,1,"Behold, what manner of love the Father hath bestowed upon us, that we should be called the sons of God.");
 addV(b,4,7,"Beloved, let us love one another: for love is of God; and every one that loveth is born of God, and knoweth God.");
 addV(b,4,8,"He that loveth not knoweth not God; for God is love.");
 addV(b,4,19,"We love him, because he first loved us."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="2 John"; b.shortCode="2Jn"; b.testament="NT"; b.description="Walk in truth and love";
 addV(b,1,6,"And this is love, that we walk after his commandments."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="3 John"; b.shortCode="3Jn"; b.testament="NT"; b.description="Hospitality and faithful living";
 addV(b,1,2,"Beloved, I wish above all things that thou mayest prosper and be in health, even as thy soul prospereth.");
 addV(b,1,4,"I have no greater joy than to hear that my children walk in truth."); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Jude"; b.shortCode="Jud"; b.testament="NT"; b.description="Contend for the faith, false teachers";
 addV(b,1,3,"Beloved, when I gave all diligence to write unto you of the common salvation, it was needful for me to write unto you, and exhort you that ye should earnestly contend for the faith.");
 addV(b,1,24,"Now unto him that is able to keep you from falling, and to present you faultless before the presence of his glory with exceeding joy,"); g_bible.addBook(std::move(b));}

{BibleBook b; b.name="Revelation"; b.shortCode="Rev"; b.testament="NT"; b.description="Apocalypse, seven seals, new Jerusalem";
 addV(b,1,7,"Behold, he cometh with clouds; and every eye shall see him, and they also which pierced him.");
 addV(b,1,8,"I am Alpha and Omega, the beginning and the ending, saith the Lord, which is, and which was, and which is to come, the Almighty.");
 addV(b,3,20,"Behold, I stand at the door, and knock: if any man hear my voice, and open the door, I will come in to him, and will sup with him, and he with me.");
 addV(b,19,16,"And he hath on his vesture and on his thigh a name written, KING OF KINGS, AND LORD OF LORDS.");
 addV(b,21,1,"And I saw a new heaven and a new earth: for the first heaven and the first earth were passed away; and there was no more sea.");
 addV(b,21,3,"And I heard a great voice out of heaven saying, Behold, the tabernacle of God is with men, and he will dwell with them.");
 addV(b,21,4,"And God shall wipe away all tears from their eyes; and there shall be no more death, neither sorrow, nor crying, neither shall there be any more pain: for the former things are passed away.");
 addV(b,21,5,"And he that sat upon the throne said, Behold, I make all things new.");
 addV(b,22,12,"And, behold, I come quickly; and my reward is with me, to give every man according as his work shall be.");
 addV(b,22,13,"I am Alpha and Omega, the beginning and the end, the first and the last.");
 addV(b,22,20,"He which testifieth these things saith, Surely I come quickly. Amen. Even so, come, Lord Jesus.");
 addV(b,22,21,"The grace of our Lord Jesus Christ be with you all. Amen."); g_bible.addBook(std::move(b));}
}

// ============================================================
//  SECTION 5: DISPLAY — READ A BOOK (BST traversal)
// ============================================================
void displayBook(const BibleBook& bk){
    clearScreen();
    std::string test=(bk.testament=="OT")?"Old Testament":"New Testament";
    std::cout<<"\n";
    printLine("  +========================================================================+",YELLOW,true);
    std::cout<<BOLD<<BRIGHT_YELLOW<<"  |  "<<BRIGHT_WHITE<<bk.name<<RESET
             <<BRIGHT_CYAN<<"  --  "<<test<<"\n"<<RESET;
    std::cout<<BOLD<<BRIGHT_YELLOW<<"  |  "<<DIM<<WHITE<<bk.description<<"\n"<<RESET;
    // Stats via MapReduce
    std::cout<<BOLD<<BRIGHT_YELLOW<<"  |  "<<RESET<<DIM<<CYAN
             <<bk.totalChapters()<<" chapters  |  "<<bk.totalVerses()<<" verses stored\n"<<RESET;
    printLine("  +========================================================================+",YELLOW,true);

    // Walk BST (std::map in-order = sorted chapters)
    for(auto& [chNum,verses]:bk.chapters){
        std::cout<<"\n"<<BOLD<<BRIGHT_YELLOW<<"  === Chapter "<<chNum<<" ===\n"<<RESET;
        for(auto& v:verses){
            std::cout<<BOLD<<BRIGHT_CYAN<<"  "<<chNum<<":"<<v.verse<<"  "<<RESET;
            auto lns=wordWrap(v.text,64);
            for(size_t li=0;li<lns.size();++li){
                if(li>0)std::cout<<"        ";
                std::cout<<BRIGHT_WHITE<<lns[li]<<"\n"<<RESET;
                sleep_ms(5);
            }
        }
        sleep_ms(20);
    }
    std::cout<<"\n";
    printLine("  +========================================================================+",YELLOW,true);
}

void browseBooksMenu(){
    while(true){
        clearScreen();
        sectionHeader("READ A BOOK OF THE BIBLE  (all 66)","B");
        auto& otIdx=g_bible.testamentIndex["OT"];
        auto& ntIdx=g_bible.testamentIndex["NT"];

        std::cout<<BOLD<<BRIGHT_YELLOW<<"  -- OLD TESTAMENT ("<<otIdx.size()<<" books) --\n"<<RESET;
        printRule(74,YELLOW);
        for(int i=0;i<(int)otIdx.size();++i){
            auto& bk=g_bible.books[otIdx[i]];
            std::cout<<BOLD<<BRIGHT_CYAN<<"  "<<std::setw(2)<<(i+1)<<". "
                     <<BRIGHT_WHITE<<std::left<<std::setw(18)<<bk.name
                     <<BRIGHT_YELLOW<<" ["<<std::setw(3)<<bk.totalChapters()<<"ch]  "
                     <<DIM<<WHITE<<bk.description.substr(0,40)<<RESET<<"\n";
        }
        std::cout<<"\n"<<BOLD<<BRIGHT_MAGENTA<<"  -- NEW TESTAMENT ("<<ntIdx.size()<<" books) --\n"<<RESET;
        printRule(74,MAGENTA);
        for(int i=0;i<(int)ntIdx.size();++i){
            auto& bk=g_bible.books[ntIdx[i]];
            std::cout<<BOLD<<BRIGHT_CYAN<<"  "<<std::setw(2)<<(i+40)<<". "
                     <<BRIGHT_WHITE<<std::left<<std::setw(18)<<bk.name
                     <<BRIGHT_MAGENTA<<" ["<<std::setw(3)<<bk.totalChapters()<<"ch]  "
                     <<DIM<<WHITE<<bk.description.substr(0,40)<<RESET<<"\n";
        }
        std::cout<<BOLD<<RED<<"\n  [0] Back to Main Menu\n"<<RESET;
        std::cout<<BRIGHT_YELLOW<<BOLD<<"\n  Choose book (1-66): "<<RESET;

        std::string inp; std::getline(std::cin,inp);
        int idx=-1; try{idx=std::stoi(inp)-1;}catch(...){}
        if(idx<0)return;
        if(idx<(int)g_bible.books.size()){
            displayBook(g_bible.books[idx]); pressAnyKey();
        }
    }
}

// ============================================================
//  SECTION 6: SEARCH (MapReduce grep)
// ============================================================
void searchBible(){
    clearScreen(); sectionHeader("SEARCH THE HOLY SCRIPTURES","?");
    printLine("  Type any word or phrase to search all 66 books.\n",BRIGHT_CYAN,true);
    std::cout<<BRIGHT_YELLOW<<BOLD<<"  Keyword: "<<RESET;
    std::string kw; std::getline(std::cin,kw);
    if(kw.empty())return;
    spinLoader("Searching the Scriptures",900);
    auto hits=g_bible.search(kw);
    clearScreen(); sectionHeader("RESULTS FOR: \""+kw+"\"","?");
    if(hits.empty()){printLine("  No results found. Try another word.",BRIGHT_RED,true);}
    else{
        std::cout<<BRIGHT_CYAN<<"  Found "<<hits.size()<<" result(s):\n\n"<<RESET;
        for(auto& [bk,ch,vs,txt]:hits){
            std::cout<<BOLD<<BRIGHT_YELLOW<<"  "<<bk<<" "<<ch<<":"<<vs<<"\n"<<RESET;
            for(auto& l:wordWrap(txt,65))
                std::cout<<"  "<<BRIGHT_WHITE<<l<<"\n"<<RESET;
            std::cout<<"\n"; sleep_ms(15);
        }
    }
    pressAnyKey();
}

// ============================================================
//  SECTION 7: STATS (MapReduce analytics)
// ============================================================
void showBibleStats(){
    clearScreen(); sectionHeader("BIBLE STATISTICS  (MapReduce Analytics)","#");
    spinLoader("Analyzing all 66 books",700);
    std::cout<<"\n";
    int totalV=g_bible.totalVerses();
    int totalC=g_bible.totalChapters();
    int totalB=(int)g_bible.books.size();
    // Map each book to its verse count, then find max/min (reduce)
    std::string longest,shortest; int maxV=0,minV=999999;
    for(auto& bk:g_bible.books){
        int v=bk.totalVerses();
        if(v>maxV){maxV=v;longest=bk.name;}
        if(v<minV){minV=v;shortest=bk.name;}
    }
    int otV=0,ntV=0;
    for(auto i:g_bible.testamentIndex["OT"])otV+=g_bible.books[i].totalVerses();
    for(auto i:g_bible.testamentIndex["NT"])ntV+=g_bible.books[i].totalVerses();

    printLine("  +--------------------------------------------------------------------+",BRIGHT_YELLOW,true);
    auto row=[](const std::string& lbl,const std::string& val){
        std::cout<<BOLD<<BRIGHT_YELLOW<<"  |  "<<RESET
                 <<BRIGHT_CYAN<<std::left<<std::setw(36)<<lbl
                 <<BOLD<<BRIGHT_WHITE<<val<<RESET<<"\n";
    };
    row("Total Books:",          std::to_string(totalB));
    row("Old Testament Books:",  "39");
    row("New Testament Books:",  "27");
    row("Total Chapters:",       std::to_string(totalC));
    row("Total Verses (stored):",std::to_string(totalV));
    row("OT Verses:",            std::to_string(otV));
    row("NT Verses:",            std::to_string(ntV));
    row("Largest book (verses):",longest+" ("+std::to_string(maxV)+")");
    row("Smallest book (verses):",shortest+" ("+std::to_string(minV)+")");
    row("Tree structure:",       "std::map<int,vector<Verse>> per book");
    row("Index structure:",      "std::map<name,idx>  O(log n) lookup");
    row("Search algorithm:",     "MapReduce grep  O(n) all verses");
    row("MapReduce filter:",      "filterVerses(predicate) per book");
    printLine("  +--------------------------------------------------------------------+",BRIGHT_YELLOW,true);
    pressAnyKey();
}

// ============================================================
//  SECTION 8: PSALMS BROWSER (BST chapter lookup)
// ============================================================
void browsePsalms(){
    auto* ps=g_bible.find("Psalms"); if(!ps)return;
    while(true){
        clearScreen(); sectionHeader("THE BOOK OF PSALMS -- 150 Songs","~");
        printLine("  Enter a Psalm number to read its key verses.\n",BRIGHT_CYAN,true);
        // List available Psalms from BST keys
        int col=0;
        for(auto& [ch,_]:ps->chapters){
            std::cout<<BRIGHT_CYAN<<"  Ps."<<std::setw(3)<<ch<<RESET;
            if(++col%12==0)std::cout<<"\n";
        }
        std::cout<<"\n"<<BOLD<<RED<<"\n  [0] Back\n"<<RESET;
        std::cout<<BRIGHT_YELLOW<<BOLD<<"\n  Psalm number: "<<RESET;
        std::string s; std::getline(std::cin,s);
        int n=-1; try{n=std::stoi(s);}catch(...){}
        if(n<=0)return;
        auto it=ps->chapters.find(n); // O(log n) BST lookup
        if(it==ps->chapters.end()){
            std::cout<<BRIGHT_RED<<"  Psalm "<<n<<" not in this edition.\n"<<RESET;
            sleep_ms(1200); continue;
        }
        clearScreen(); std::cout<<"\n";
        printLine("  +========================================================================+",BRIGHT_YELLOW,true);
        std::cout<<BOLD<<BRIGHT_YELLOW<<"  |  PSALM "<<n<<"\n"<<RESET;
        printLine("  +========================================================================+",BRIGHT_YELLOW,true);
        for(auto& v:it->second){
            std::cout<<BOLD<<BRIGHT_YELLOW<<"  v"<<v.verse<<"  "<<RESET;
            for(auto& l:wordWrap(v.text,64))
                std::cout<<ITALIC<<BRIGHT_WHITE<<l<<"\n"<<RESET;
            std::cout<<"\n"; sleep_ms(35);
        }
        printLine("  +========================================================================+",BRIGHT_YELLOW,true);
        pressAnyKey();
    }
}

// ============================================================
//  SECTION 9: VERSICLES
// ============================================================
struct Versicle{ std::string title,ref; std::vector<std::pair<std::string,std::string>> lines; };
const std::vector<Versicle> versicles={
{"The Grace","2 Corinthians 13:14",
{{"L:","The grace of the Lord Jesus Christ,"},{"R:","And the love of God,"},
 {"L:","And the fellowship of the Holy Spirit,"},{"R:","Be with you all."},{"ALL:","Amen."}}},
{"Opening Versicle","Psalm 51:15 / Psalm 70:1",
{{"L:","O Lord, open thou our lips."},{"R:","And our mouth shall show forth thy praise."},
 {"L:","O God, make speed to save us."},{"R:","O Lord, make haste to help us."},
 {"L:","Glory be to the Father, and to the Son,"},{"R:","And to the Holy Ghost."},
 {"L:","As it was in the beginning, is now, and ever shall be,"},{"R:","World without end. Amen."}}},
{"The Apostles' Creed","Ancient Confession of Faith",
{{"ALL:","I believe in God, the Father Almighty, Creator of heaven and earth."},
 {"ALL:","I believe in Jesus Christ, His only Son, our Lord,"},
 {"ALL:","Who was conceived by the Holy Spirit, born of the Virgin Mary,"},
 {"ALL:","Suffered under Pontius Pilate, was crucified, died, and was buried."},
 {"ALL:","On the third day He rose again; He ascended into heaven,"},
 {"ALL:","And is seated at the right hand of the Father."},
 {"ALL:","He will come again to judge the living and the dead."},
 {"ALL:","I believe in the Holy Spirit, the holy universal Church,"},
 {"ALL:","The communion of saints, the forgiveness of sins,"},
 {"ALL:","The resurrection of the body, and the life everlasting. Amen."}}},
{"The Doxology","Thomas Ken, 1674",
{{"ALL:","Praise God, from Whom all blessings flow;"},
 {"ALL:","Praise Him, all creatures here below;"},
 {"ALL:","Praise Him above, ye heavenly host;"},
 {"ALL:","Praise Father, Son, and Holy Ghost. Amen."}}},
{"Kyrie Eleison","Ancient Litany",
{{"L:","Lord, have mercy upon us."},{"R:","Lord, have mercy upon us."},
 {"L:","Christ, have mercy upon us."},{"R:","Christ, have mercy upon us."},
 {"L:","Lord, have mercy upon us."},{"R:","Lord, have mercy upon us."}}},
{"Gloria in Excelsis","Luke 2:14",
{{"ALL:","Glory to God in the highest, and on earth peace, good will toward men."},
 {"L:","We praise thee, we bless thee, we worship thee, we glorify thee."},
 {"ALL:","O Lord God, heavenly King, God the Father Almighty. Amen."}}},
{"Sursum Corda","Ancient Eucharistic Dialogue",
{{"L:","The Lord be with you."},{"R:","And also with you."},
 {"L:","Lift up your hearts."},{"R:","We lift them up to the Lord."},
 {"L:","Let us give thanks to the Lord our God."},{"R:","It is right to give him thanks and praise."}}},
{"Agnus Dei","John 1:29",
{{"ALL:","O Lamb of God, that takest away the sins of the world,"},{"ALL:","Have mercy upon us."},
 {"ALL:","O Lamb of God, that takest away the sins of the world,"},{"ALL:","Have mercy upon us."},
 {"ALL:","O Lamb of God, that takest away the sins of the world,"},{"ALL:","Grant us thy peace. Amen."}}},
{"Sanctus","Isaiah 6:3",
{{"ALL:","Holy, holy, holy, Lord God of hosts,"},{"ALL:","Heaven and earth are full of thy glory."},
 {"ALL:","Glory be to thee, O Lord Most High."},{"ALL:","Hosanna in the highest. Amen."}}},
{"Benediction Response","Numbers 6:24-26",
{{"L:","The Lord bless you and keep you."},{"R:","Thanks be to God."},
 {"L:","The Lord make his face shine upon you and be gracious to you."},{"R:","Thanks be to God."},
 {"L:","The Lord turn his face toward you and give you peace."},{"ALL:","Amen. Thanks be to God!"}}},
};

void showVersiclesMenu(){
    clearScreen(); sectionHeader("VERSICLES & RESPONSIVE READINGS","R");
    for(size_t i=0;i<versicles.size();++i)
        std::cout<<BOLD<<BRIGHT_CYAN<<"  ["<<(i+1)<<"] "<<RESET
                 <<BRIGHT_WHITE<<versicles[i].title<<DIM<<CYAN<<"  ("<<versicles[i].ref<<")\n"<<RESET;
    std::cout<<BOLD<<RED<<"\n  [0] Back\n"<<RESET;
    std::cout<<BRIGHT_YELLOW<<BOLD<<"\n  Choose (1-"<<versicles.size()<<"): "<<RESET;
}
void displayVersicle(const Versicle& v){
    clearScreen(); sectionHeader(v.title+"  --  "+v.ref,"R");
    std::cout<<"  "<<BOLD<<BRIGHT_CYAN<<"L:"<<RESET<<" Leader    "
             <<BOLD<<BRIGHT_GREEN<<"R:"<<RESET<<" Response    "
             <<BOLD<<BRIGHT_YELLOW<<"ALL:"<<RESET<<" Congregation\n\n";
    for(auto& [role,text]:v.lines){
        sleep_ms(100);
        std::string col=(role=="L:")?BRIGHT_CYAN:(role=="R:")?BRIGHT_GREEN:BRIGHT_YELLOW;
        std::cout<<"\n  "<<BOLD<<col<<std::setw(5)<<role<<"  "<<RESET;
        typewrite(text,18,ITALIC+std::string(BRIGHT_WHITE)); std::cout<<"\n";
    }
    std::cout<<"\n"; pressAnyKey();
}

// ============================================================
//  SECTION 10: OTHER CONTENT
// ============================================================
void showLordsPrayer(){
    clearScreen(); sectionHeader("THE LORD'S PRAYER","~");
    printLine("              Matthew 6:9-13  |  Luke 11:2-4\n",BRIGHT_CYAN,true);
    const std::vector<std::pair<std::string,std::string>> pr={
        {"Address",    "Our Father, who art in heaven, hallowed be Thy name."},
        {"Kingdom",    "Thy Kingdom come, Thy will be done, on earth as it is in heaven."},
        {"Provision",  "Give us this day our daily bread,"},
        {"Forgiveness","And forgive us our trespasses, as we forgive those who trespass against us."},
        {"Protection", "And lead us not into temptation, but deliver us from evil."},
        {"Doxology",   "For Thine is the Kingdom, the Power, and the Glory, forever and ever."},
        {"",           "Amen."},
    };
    printLine("  +----------------------------------------------------------+",MAGENTA,true);
    for(auto& [lbl,line]:pr){
        std::cout<<MAGENTA<<BOLD<<"  |  "<<RESET;
        typewrite(line,16,BRIGHT_WHITE);
        if(!lbl.empty())std::cout<<DIM<<CYAN<<"  <- "<<lbl<<RESET;
        std::cout<<"\n"; sleep_ms(80);
    }
    printLine("  +----------------------------------------------------------+",MAGENTA,true);
    pressAnyKey();
}

void showCommandments(){
    clearScreen(); sectionHeader("THE TEN COMMANDMENTS","=");
    printLine("         Exodus 20:1-17\n",BRIGHT_CYAN,true);
    const std::vector<std::string> cmds={
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
    printLine("  +------------------------------------------------------------+",BRIGHT_WHITE,true);
    printLine("  |            * THE LAW OF GOD (Exodus 20) *                  |",YELLOW,true);
    printLine("  +------------------------------------------------------------+",BRIGHT_WHITE,true);
    for(auto& c:cmds){
        std::cout<<BRIGHT_WHITE<<BOLD<<"  |  "<<RESET;
        typewrite(c,14,BRIGHT_YELLOW);
        std::cout<<std::string(std::max(0,54-(int)c.size()),' ')<<BRIGHT_WHITE<<BOLD<<"  |\n"<<RESET;
        sleep_ms(60);
    }
    printLine("  +------------------------------------------------------------+",BRIGHT_WHITE,true);
    pressAnyKey();
}

void verseOfTheDay(){
    clearScreen(); sectionHeader("VERSE OF THE DAY","*");
    srand((unsigned)time(nullptr));
    // Pick random book, random chapter, random verse from the tree
    auto& bks=g_bible.books;
    int bi=rand()%(int)bks.size();
    auto& bk=bks[bi];
    auto it=bk.chapters.begin();
    std::advance(it,rand()%bk.chapters.size());
    auto& vs=it->second;
    auto& v=vs[rand()%vs.size()];
    std::string ref=bk.name+" "+std::to_string(v.chapter)+":"+std::to_string(v.verse);
    std::cout<<"\n"; verseBox(ref,"\""+v.text+"\"",BRIGHT_YELLOW,BRIGHT_WHITE,64);
    pressAnyKey();
}

void showTimeline(){
    clearScreen(); sectionHeader("BIBLE TIMELINE -- Creation to Revelation","T");
    struct E{std::string date,desc,ref;};
    const std::vector<E> ev={
        {"~4000 BC","Creation of the world","Genesis 1"},
        {"~2350 BC","Noah's Flood","Genesis 6-9"},
        {"~2091 BC","God calls Abraham","Genesis 12"},
        {"~1526 BC","Birth of Moses","Exodus 2"},
        {"~1446 BC","The Exodus from Egypt","Exodus 12-14"},
        {"~1406 BC","Israel enters Canaan","Joshua 3"},
        {"~1010 BC","David becomes king","2 Samuel 5"},
        {"~970 BC", "Solomon builds the Temple","1 Kings 6"},
        {"~722 BC", "Fall of Northern Kingdom","2 Kings 17"},
        {"~586 BC", "Babylon destroys Jerusalem","2 Kings 25"},
        {"~538 BC", "Return from exile","Ezra 1"},
        {"~5 BC",   "Birth of Jesus in Bethlehem","Luke 2"},
        {"~30 AD",  "Crucifixion and Resurrection","John 19-20"},
        {"~33 AD",  "Pentecost -- birth of the Church","Acts 2"},
        {"~47 AD",  "Paul's missionary journeys","Acts 13"},
        {"~95 AD",  "John writes Revelation on Patmos","Revelation 1"},
    };
    std::cout<<"\n";
    for(size_t i=0;i<ev.size();++i){
        std::cout<<BOLD<<BRIGHT_YELLOW<<"  "<<std::setw(9)<<ev[i].date<<"  "<<RESET
                 <<BRIGHT_CYAN<<"--> "<<RESET
                 <<BRIGHT_WHITE<<BOLD<<ev[i].desc<<RESET
                 <<DIM<<CYAN<<"  ["<<ev[i].ref<<"]\n"<<RESET;
        if(i<ev.size()-1)std::cout<<"               |\n";
        sleep_ms(35);
    }
    std::cout<<"\n";
    printLine("  'All scripture is given by inspiration of God.' -- 2 Tim 3:16",BRIGHT_CYAN,true);
    pressAnyKey();
}

void showFruitOfSpirit(){
    clearScreen(); sectionHeader("THE FRUIT OF THE HOLY SPIRIT","F");
    printLine("  Galatians 5:22-23\n",BRIGHT_CYAN,true);
    verseBox("Galatians 5:22-23",
        "The fruit of the Spirit is love, joy, peace, longsuffering, gentleness, "
        "goodness, faith, meekness, temperance: against such there is no law.",
        BRIGHT_GREEN,BRIGHT_WHITE,60);
    std::cout<<"\n";
    const std::vector<std::tuple<std::string,std::string,std::string>> f={
        {"LOVE",         "Agape",      "Unconditional love -- the very nature of God."},
        {"JOY",          "Chara",      "Deep gladness that transcends all circumstances."},
        {"PEACE",        "Eirene",     "Wholeness and harmony with God and others."},
        {"LONGSUFFERING","Makrothumia","Patient endurance -- bearing with others graciously."},
        {"GENTLENESS",   "Chrestotes", "Kindness in action -- treating others with care."},
        {"GOODNESS",     "Agathosune", "Moral excellence -- doing right because it is right."},
        {"FAITH",        "Pistis",     "Faithfulness -- reliability and trustworthiness."},
        {"MEEKNESS",     "Prautes",    "Strength under control -- holy humility before God."},
        {"TEMPERANCE",   "Enkrateia",  "Self-control -- mastery of desires and passions."},
    };
    for(auto& [nm,gr,dc]:f){
        std::cout<<"  "<<BOLD<<BRIGHT_GREEN<<std::left<<std::setw(16)<<nm
                 <<DIM<<CYAN<<std::setw(16)<<gr<<"\n"<<RESET
                 <<"  "<<BRIGHT_WHITE<<dc<<"\n\n"<<RESET; sleep_ms(35);
    }
    pressAnyKey();
}

void showPrayerCorner(){
    clearScreen(); sectionHeader("THE PRAYER CORNER","~");
    printLine("  'Pray without ceasing.' -- 1 Thessalonians 5:17\n",BRIGHT_CYAN,true);
    const std::vector<std::pair<std::string,std::string>> prayers={
        {"Morning Prayer",
         "Lord, as I rise this day, I thank You for the gift of life. Guide my steps, guard my heart, "
         "let my words bring glory to You. Fill me with Your Spirit and keep me in Your peace. Amen."},
        {"Evening Prayer",
         "Heavenly Father, as the day ends I come in gratitude. Forgive me where I fell short. "
         "Your mercies are new every morning. Watch over me and all I love tonight. In Jesus' name, Amen."},
        {"Prayer for Strength",
         "O Lord, I am weak but You are strong. When my burdens feel too heavy remind me Your yoke is easy. "
         "Be my strength and shield. I can do all things through Christ who strengthens me. Amen."},
        {"Prayer for Others",
         "Father, I lift up all who are suffering, sick, lonely, or afraid. Let Your healing hand touch them. "
         "Give comfort to the grieving and hope to the hopeless. Let Your kingdom come in their lives. Amen."},
        {"Prayer of Thanksgiving",
         "Thank You, Lord, for every good and perfect gift -- for life, for Your Son's sacrifice, "
         "for Your Word, and for the hope of heaven. May gratitude be the song of my life. Amen."},
    };
    for(auto& [nm,text]:prayers){
        std::cout<<BOLD<<BRIGHT_MAGENTA<<"\n  +  "<<nm<<"\n"<<RESET;
        printLine("  +------------------------------------------------------------------+",MAGENTA,true);
        for(auto& l:wordWrap(text,64))
            std::cout<<MAGENTA<<BOLD<<"  |  "<<RESET<<ITALIC<<BRIGHT_WHITE<<l<<"\n"<<RESET;
        printLine("  +------------------------------------------------------------------+",MAGENTA,true);
        sleep_ms(25);
    }
    pressAnyKey();
}

void showNamesOfJesus(){
    clearScreen(); sectionHeader("THE NAMES & TITLES OF JESUS CHRIST","J");
    const std::vector<std::tuple<std::string,std::string,std::string>> names={
        {"Jesus",             "Matthew 1:21",     "YHWH saves -- His personal name given by the angel"},
        {"Christ / Messiah",  "Matthew 16:16",    "The Anointed One of Israel"},
        {"Son of God",        "John 1:34",        "The eternal Son, one with the Father"},
        {"Emmanuel",          "Matthew 1:23",     "God with us -- the Incarnate God"},
        {"The Word",          "John 1:1",         "The eternal Logos -- God's self-expression"},
        {"Lamb of God",       "John 1:29",        "The sacrifice who takes away sin"},
        {"Good Shepherd",     "John 10:11",       "He lays down His life for the sheep"},
        {"Bread of Life",     "John 6:35",        "He satisfies the soul's deepest hunger"},
        {"Light of the World","John 8:12",        "He scatters all spiritual darkness"},
        {"Resurrection & Life","John 11:25",      "Death has no power over Him or those in Him"},
        {"Way, Truth, Life",  "John 14:6",        "The only path to the Father"},
        {"King of Kings",     "Revelation 19:16", "Lord over all rulers and powers"},
        {"Alpha and Omega",   "Revelation 1:8",   "First and Last -- encompasses all eternity"},
        {"Prince of Peace",   "Isaiah 9:6",       "His kingdom brings ultimate and lasting peace"},
        {"Wonderful Counselor","Isaiah 9:6",      "His wisdom surpasses all human understanding"},
        {"Savior",            "Luke 2:11",        "He saves His people from their sins"},
        {"High Priest",       "Hebrews 4:14",     "He intercedes for us before the Father"},
        {"Lion of Judah",     "Revelation 5:5",   "He is sovereign, powerful, and victorious"},
    };
    std::cout<<"\n";
    for(auto& [nm,verse,meaning]:names){
        std::cout<<"  "<<BOLD<<BRIGHT_YELLOW<<std::left<<std::setw(22)<<nm
                 <<BRIGHT_CYAN<<std::setw(20)<<verse<<"\n"<<RESET
                 <<"  "<<DIM<<WHITE<<meaning<<"\n\n"<<RESET; sleep_ms(28);
    }
    pressAnyKey();
}

void showSalvationPlan(){
    clearScreen(); sectionHeader("THE PLAN OF SALVATION -- The Roman Road","S");
    const std::vector<std::tuple<std::string,std::string,std::string>> steps={
        {"1. ALL HAVE SINNED",      "Romans 3:23","For all have sinned, and come short of the glory of God."},
        {"2. SIN'S PENALTY",        "Romans 6:23","The wages of sin is death; but the gift of God is eternal life through Jesus Christ."},
        {"3. GOD'S LOVE",           "Romans 5:8", "While we were yet sinners, Christ died for us."},
        {"4. CONFESS & BELIEVE",    "Romans 10:9","If thou shalt confess with thy mouth the Lord Jesus, and believe in thine heart, thou shalt be saved."},
        {"5. CALL ON HIS NAME",     "Romans 10:13","Whosoever shall call upon the name of the Lord shall be saved."},
        {"6. NO CONDEMNATION",      "Romans 8:1", "There is therefore now no condemnation to them which are in Christ Jesus."},
        {"7. NOTHING SEPARATES US", "Romans 8:38","Nothing shall be able to separate us from the love of God, which is in Christ Jesus."},
    };
    for(auto& [step,ref,text]:steps){
        std::cout<<"\n  "<<BOLD<<BRIGHT_GREEN<<step<<"\n"<<RESET
                 <<"  "<<BOLD<<BRIGHT_YELLOW<<ref<<"\n"<<RESET;
        for(auto& l:wordWrap("\""+text+"\"",65))
            std::cout<<"  "<<ITALIC<<BRIGHT_WHITE<<l<<"\n"<<RESET;
        sleep_ms(50);
    }
    std::cout<<"\n\n"; printRule(74,BRIGHT_GREEN);
    printLine("  To receive Jesus: confess, believe, and accept His gift of eternal life.",BRIGHT_WHITE,true);
    printRule(74,BRIGHT_GREEN);
    pressAnyKey();
}

void goodbyeScreen(){
    clearScreen(); std::cout<<"\n"; printRule(74,YELLOW);
    typewrite("\n       May the Lord bless you and keep you;\n",18,BRIGHT_WHITE);
    typewrite("       May the Lord make His face shine on you and be gracious to you;\n",18,BRIGHT_WHITE);
    typewrite("       May the Lord turn His face toward you and give you peace.\n",18,BRIGHT_WHITE);
    printLine("\n                        -- Numbers 6:24-26\n",BRIGHT_YELLOW,true);
    printRule(74,YELLOW); std::cout<<"\n";
    animFlame(3); std::cout<<"\n";
    typewrite("  + God bless you. Come back and read His Word again!  +\n\n",22,BRIGHT_YELLOW);
}

// ============================================================
//  OPENING + MAIN MENU
// ============================================================
void openingCeremony(){
    clearScreen(); std::cout<<HIDE_CURSOR<<"\n";
    starField(4,74,5);
    std::cout<<BRIGHT_WHITE<<BOLD;
    std::cout<<"               ___     _,--'.`-.    \n";
    std::cout<<"           .-'   `--=/     `    \\   \n";
    std::cout<<"          /        /            |   \n";
    std::cout<<"         |   , .__/ `._,  \\     |   \n";
    std::cout<<"          \\  `'         `--'`--'/   \n";
    std::cout<<"           `.     ~ Holy Spirit ~  .'\n";
    std::cout<<"             `-._          _.-'     \n";
    std::cout<<"                  `------''          \n"<<RESET;
    sleep_ms(400);
    animRule(74,YELLOW,5);
    drawBibleTitle();   // <-- original block-letter ASCII title
    animRule(74,YELLOW,5);
    std::cout<<"\n";
    printLine("      ** Complete Holy Bible Explorer -- King James Version **\n",BRIGHT_CYAN,true);
    spinLoader("Loading BibleTree (66 books, BST + MapReduce)",1400);
    sleep_ms(400);
    std::cout<<SHOW_CURSOR;
}

void showMenu(){
    clearScreen(); std::cout<<"\n";
    drawCross(); std::cout<<"\n";
    printLine("  +=========================================================================+",YELLOW,true);
    printLine("  |          +  COMPLETE HOLY BIBLE EXPLORER -- KJV Edition  +             |",BRIGHT_WHITE,true);
    printLine("  +====================================+====================================+",YELLOW,true);
    printLine("  |  B  1. Read Any Book (all 66)      |  ~  9. Browse Psalms (BST lookup) |",BRIGHT_CYAN,true);
    printLine("  |  *  2. Verse of the Day (random)   |  R 10. Versicles & Responses      |",BRIGHT_CYAN,true);
    printLine("  |  ?  3. Search the Scriptures        |  ~  11. The Lord's Prayer         |",BRIGHT_CYAN,true);
    printLine("  |  #  4. Bible Statistics (MapReduce) |  =  12. Ten Commandments          |",BRIGHT_CYAN,true);
    printLine("  |  T  5. Bible Timeline               |  F  13. Fruit of the Holy Spirit  |",BRIGHT_CYAN,true);
    printLine("  |  ~  6. Prayer Corner                |  J  14. Names & Titles of Jesus   |",BRIGHT_CYAN,true);
    printLine("  |  ~  7. Candlelight Meditation        |  S  15. Plan of Salvation         |",BRIGHT_CYAN,true);
    printLine("  |                                    |                                   |",BRIGHT_CYAN,true);
    printLine("  +====================================+====================================+",YELLOW,true);
    printLine("  |                      X  0. Exit & God Bless                            |",RED,true);
    printLine("  +=========================================================================+",YELLOW,true);
    std::cout<<"\n  "<<BRIGHT_YELLOW<<BOLD<<"+  Your choice (0-15): "<<RESET;
}

// ============================================================
//  MAIN
// ============================================================
int main(){
    buildBibleTree();
    openingCeremony();
    sleep_ms(400);

    bool running=true;
    while(running){
        showMenu();
        std::string inp; std::getline(std::cin,inp);
        if(inp.empty())std::getline(std::cin,inp);
        int ch=-1; try{ch=std::stoi(inp);}catch(...){}

        switch(ch){
        case 1:  browseBooksMenu(); break;
        case 2:  verseOfTheDay(); break;
        case 3:  searchBible(); break;
        case 4:  showBibleStats(); break;
        case 5:  showTimeline(); break;
        case 6:  showPrayerCorner(); break;
        case 7:{
            clearScreen(); sectionHeader("CANDLELIGHT MEDITATION","~");
            printLine("  Take a quiet moment. Still your heart before the Lord.\n",BRIGHT_WHITE,true);
            animFlame(4); std::cout<<"\n";
            verseBox("Psalm 46:10","Be still, and know that I am God.",BRIGHT_YELLOW,BRIGHT_WHITE,50);
            pressAnyKey(); break;}
        case 8:  break; // unused slot
        case 9:  browsePsalms(); break;
        case 10:{
            showVersiclesMenu();
            std::string s; std::getline(std::cin,s);
            int i=-1; try{i=std::stoi(s)-1;}catch(...){}
            if(i>=0&&i<(int)versicles.size())displayVersicle(versicles[i]);
            break;}
        case 11: showLordsPrayer(); break;
        case 12: showCommandments(); break;
        case 13: showFruitOfSpirit(); break;
        case 14: showNamesOfJesus(); break;
        case 15: showSalvationPlan(); break;
        case 0:  goodbyeScreen(); running=false; break;
        default:
            std::cout<<"\n  "<<BRIGHT_RED<<"X Please choose 0-15.\n"<<RESET;
            sleep_ms(700);
        }
    }
    std::cout<<SHOW_CURSOR<<RESET;
    return 0;
}
