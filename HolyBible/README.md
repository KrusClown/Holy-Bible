# † Holy Bible — for the console

The complete **King James Version** (all 66 books, 1,189 chapters, 31,102 verses) in a colorful
terminal app written in C++17. Read, search, and find the Word for what you need.

## Build

**Windows** — open a terminal in this folder and run `build.bat` (works with MinGW `g++` or Visual Studio `cl`).
Use **Windows Terminal** for the best colors.

**Linux / macOS** — `sh build.sh`

**CMake (any OS)**

```
cmake -B build
cmake --build build --config Release
```

Keep the `data` folder (with `kjv.tsv`) next to the program.

## Use

Run `holybible` and pick from the menu — or just type:

| You type | You get |
|---|---|
| `John 3:16` | a verse |
| `Psalm 23`, `Ps 91` | a whole psalm or chapter |
| `1 Cor 13:4-7` | a range of verses |
| `Gen 1:1-2:3` | a passage across chapters |
| `Juan 3,16`, `Salmos 23` | Spanish book names work too |
| `love one another` | anything else is searched |

### Search
| Query | Meaning |
|---|---|
| `faith hope charity` | verses containing all the words |
| `"the lord is my shepherd"` | exact phrase |
| `bless*` | bless, blessed, blessing, blesseth… |
| `mercy -judgment` | exclude a word |
| `peace in:psalms` | filter: `in:ot` `in:nt` `in:gospels` `in:law` `in:prophets` `in:epistles` `in:<book>` |
| `fiath` | typos are corrected automatically |

Results are ranked by relevance (BM25, like real search engines); press `o` to switch to Bible order,
or type a result number to read it in context with the words highlighted.

### Reading
`Enter` next page · `-` previous page · `n` / `p` next / previous chapter · `12` jump to chapter 12 ·
`v 16` highlight verse 16 · `b 16` bookmark it · `s grace` search · `q` back

### Also inside
- **Verses for what I need** — 20 topics: anxiety, peace, strength, hope, grief, forgiveness, healing…
- **Famous passages** — Creation, the Ten Commandments, Psalm 23, the Beatitudes, the Lord's Prayer, 1 Corinthians 13…
- **Verse of the day**, **random verse**, **random Psalm**
- **Daily reading plan** — the whole Bible in one year, with progress
- **Bookmarks** and **continue reading** (saved in `~/.holybible`)

### Command line
```
holybible John 3:16
holybible -s "love your enemies"
holybible --votd
holybible --no-color
```

## How it works
- `src/bible.hpp` — loads the text, chapter navigation, reference parser (abbreviations, ordinals like "First John", Spanish names, typo-tolerant book names)
- `src/search.hpp` — in-memory inverted index (≈12,500 distinct words, built in ~0.1 s), BM25 ranking, phrases, wildcards, filters, Damerau-Levenshtein spelling correction
- `src/main.cpp` — the colorful console interface (256-color ANSI, UTF-8 box drawing)
- `data/kjv.tsv` — the King James Version (1769), public domain

*"Thy word is a lamp unto my feet, and a light unto my path."* — Psalm 119:105
