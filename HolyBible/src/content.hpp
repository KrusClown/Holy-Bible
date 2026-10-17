// content.hpp - curated topics, famous passages and verses of the day.
#pragma once
#include <vector>

namespace bible {

struct Collection {
    const char* title;
    const char* subtitle;
    std::vector<const char*> refs;
};

// "Verses for what I need" - pick a feeling or need, receive the Word.
inline const std::vector<Collection> kTopics = {
    {"Anxiety & Fear", "when your heart is troubled",
        {"Isaiah 41:10", "Philippians 4:6-7", "1 Peter 5:7", "2 Timothy 1:7", "Psalm 56:3", "Matthew 6:34", "John 14:27", "Psalm 23:4"}},
    {"Peace", "rest for the soul",
        {"John 14:27", "Isaiah 26:3", "Matthew 11:28-30", "Romans 5:1", "Philippians 4:7", "Psalm 4:8", "Colossians 3:15"}},
    {"Strength", "when you feel weak",
        {"Philippians 4:13", "Isaiah 40:29-31", "Psalm 46:1", "2 Corinthians 12:9", "Nehemiah 8:10", "Psalm 73:26", "Ephesians 6:10"}},
    {"Hope", "for the days ahead",
        {"Jeremiah 29:11", "Romans 15:13", "Lamentations 3:22-24", "Romans 8:28", "Psalm 42:11", "Hebrews 6:19", "Romans 5:3-5"}},
    {"Love", "God's love and loving others",
        {"John 3:16", "1 Corinthians 13:4-7", "1 John 4:7-8", "1 John 4:18-19", "Romans 8:38-39", "John 15:13", "Romans 5:8"}},
    {"Faith", "trusting what is unseen",
        {"Hebrews 11:1", "Hebrews 11:6", "Proverbs 3:5-6", "Romans 10:17", "Mark 11:22-24", "2 Corinthians 5:7", "Matthew 17:20"}},
    {"Forgiveness", "receiving and giving mercy",
        {"1 John 1:9", "Ephesians 4:32", "Colossians 3:13", "Matthew 6:14-15", "Psalm 103:10-12", "Isaiah 1:18", "Micah 7:18-19"}},
    {"Grief & Comfort", "when you are mourning",
        {"Matthew 5:4", "Psalm 34:18", "Revelation 21:4", "John 11:25-26", "2 Corinthians 1:3-4", "Psalm 147:3", "1 Thessalonians 4:13-14"}},
    {"Salvation", "the good news",
        {"John 3:16-17", "Romans 3:23", "Romans 6:23", "Romans 10:9-10", "Ephesians 2:8-9", "John 14:6", "Acts 4:12"}},
    {"Wisdom & Guidance", "when you must decide",
        {"Proverbs 3:5-6", "James 1:5", "Psalm 32:8", "Psalm 119:105", "Proverbs 16:9", "Isaiah 30:21", "Proverbs 2:6"}},
    {"Prayer", "talking with God",
        {"Matthew 6:9-13", "Philippians 4:6", "1 Thessalonians 5:16-18", "Matthew 7:7-8", "James 5:16", "1 John 5:14-15", "Jeremiah 33:3"}},
    {"Gratitude & Praise", "a thankful heart",
        {"Psalm 100", "1 Thessalonians 5:18", "Psalm 107:1", "Colossians 3:17", "Psalm 103:1-5", "Psalm 150"}},
    {"Healing", "for body and soul",
        {"Jeremiah 17:14", "Isaiah 53:5", "James 5:14-15", "Psalm 103:2-3", "Exodus 15:26", "Psalm 30:2"}},
    {"Loneliness", "you are never alone",
        {"Deuteronomy 31:6", "Hebrews 13:5", "Psalm 139:7-10", "Matthew 28:20", "Psalm 68:6", "Isaiah 43:1-2"}},
    {"Patience & Waiting", "when the answer is slow",
        {"Psalm 27:14", "Isaiah 40:31", "James 1:2-4", "Romans 12:12", "Galatians 6:9", "Psalm 37:7", "Lamentations 3:25-26"}},
    {"Temptation", "standing firm",
        {"1 Corinthians 10:13", "James 4:7", "Matthew 26:41", "Hebrews 4:15-16", "James 1:12", "Psalm 119:11"}},
    {"Family & Marriage", "home and those you love",
        {"Genesis 2:24", "Ephesians 5:25", "Proverbs 22:6", "Ephesians 6:1-4", "Joshua 24:15", "Ecclesiastes 4:9-12"}},
    {"Provision & Money", "God supplies",
        {"Philippians 4:19", "Matthew 6:31-33", "Hebrews 13:5", "1 Timothy 6:6-10", "Proverbs 3:9-10", "Malachi 3:10"}},
    {"Purpose & Work", "why you are here",
        {"Ephesians 2:10", "Colossians 3:23-24", "Jeremiah 1:5", "Romans 12:1-2", "Micah 6:8", "Ecclesiastes 12:13"}},
    {"Courage", "be strong and of good courage",
        {"Joshua 1:9", "Deuteronomy 31:8", "Psalm 27:1", "Isaiah 43:2", "Psalm 118:6", "1 Corinthians 16:13"}},
};

inline const std::vector<Collection> kFamous = {
    {"Creation",                      "In the beginning",     {"Genesis 1:1-2:3"}},
    {"The Ten Commandments",          "Sinai",                {"Exodus 20:1-17"}},
    {"The Priestly Blessing",         "The LORD bless thee",  {"Numbers 6:24-26"}},
    {"The Shema",                     "Hear, O Israel",       {"Deuteronomy 6:4-9"}},
    {"The Shepherd Psalm",            "Psalm 23",             {"Psalm 23"}},
    {"Under His Wings",               "Psalm 91",             {"Psalm 91"}},
    {"Search Me, O God",              "Psalm 139",            {"Psalm 139"}},
    {"A Time for Everything",         "Ecclesiastes 3",       {"Ecclesiastes 3:1-8"}},
    {"The Suffering Servant",         "Isaiah 53",            {"Isaiah 53"}},
    {"Mount Up with Wings as Eagles", "Isaiah 40",            {"Isaiah 40:28-31"}},
    {"The Valley of Dry Bones",       "Ezekiel 37",           {"Ezekiel 37:1-14"}},
    {"The Beatitudes",                "Blessed are...",       {"Matthew 5:3-12"}},
    {"The Lord's Prayer",             "Our Father",           {"Matthew 6:9-13"}},
    {"The Sermon on the Mount",       "Matthew 5-7",          {"Matthew 5-7"}},
    {"The Great Commission",          "Go ye therefore",      {"Matthew 28:18-20"}},
    {"The Birth of Jesus",            "Luke 2",               {"Luke 2:1-20"}},
    {"The Good Samaritan",            "Who is my neighbour?", {"Luke 10:25-37"}},
    {"The Prodigal Son",              "He was lost, and is found", {"Luke 15:11-32"}},
    {"The Word Became Flesh",         "John 1",               {"John 1:1-14"}},
    {"God So Loved the World",        "John 3",               {"John 3:16-21"}},
    {"The Good Shepherd",             "John 10",              {"John 10:1-18"}},
    {"I Am the Way",                  "John 14",              {"John 14:1-6"}},
    {"The True Vine",                 "John 15",              {"John 15:1-17"}},
    {"Pentecost",                     "Acts 2",               {"Acts 2:1-21"}},
    {"More Than Conquerors",          "Romans 8",             {"Romans 8:28-39"}},
    {"The Love Chapter",              "1 Corinthians 13",     {"1 Corinthians 13"}},
    {"The Resurrection",              "O death, where is thy sting?", {"1 Corinthians 15:51-58"}},
    {"The Fruit of the Spirit",       "Galatians 5",          {"Galatians 5:22-23"}},
    {"The Whole Armour of God",       "Ephesians 6",          {"Ephesians 6:10-18"}},
    {"The Mind of Christ",            "Philippians 2",        {"Philippians 2:5-11"}},
    {"The Hall of Faith",             "Hebrews 11",           {"Hebrews 11"}},
    {"All Things New",                "Revelation 21",        {"Revelation 21:1-7"}},
};

inline const std::vector<const char*> kVerseOfTheDay = {
    "John 3:16", "Psalm 23:1", "Philippians 4:13", "Jeremiah 29:11", "Romans 8:28", "Proverbs 3:5-6",
    "Isaiah 40:31", "Joshua 1:9", "Psalm 46:1", "Matthew 11:28", "Psalm 119:105", "2 Corinthians 5:17",
    "Galatians 2:20", "Romans 12:2", "Psalm 27:1", "Isaiah 41:10", "John 14:6", "Hebrews 11:1",
    "Psalm 37:4", "Matthew 6:33", "Romans 5:8", "1 John 4:19", "Psalm 121:1-2", "Lamentations 3:22-23",
    "Ephesians 2:8", "Psalm 91:1-2", "John 16:33", "2 Timothy 1:7", "Philippians 4:6-7", "Psalm 34:8",
    "Micah 6:8", "Matthew 5:16", "Psalm 51:10", "Isaiah 26:3", "1 Corinthians 16:14", "Colossians 3:23",
    "Psalm 118:24", "Zephaniah 3:17", "Numbers 6:24-26", "Deuteronomy 31:8", "Psalm 139:14", "John 8:12",
    "John 11:25", "Romans 15:13", "Hebrews 13:8", "James 1:17", "1 Peter 5:7", "Psalm 16:11",
    "Matthew 28:20", "Revelation 21:4", "John 1:1", "Genesis 1:1", "Psalm 100:4", "Psalm 19:14",
    "Isaiah 9:6", "John 10:10", "Psalm 62:1-2", "2 Chronicles 7:14", "Psalm 145:18", "Romans 8:1",
};

} // namespace bible
