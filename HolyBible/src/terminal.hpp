// terminal.hpp - colors, screen size and small text helpers for the console.
#pragma once
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#ifdef _WIN32
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
  #include <io.h>
#else
  #include <sys/ioctl.h>
  #include <unistd.h>
#endif

namespace term {

inline bool g_color = true;
inline bool g_tty   = true;

// Prepare the console: UTF-8 output and ANSI escape codes (also on Windows 10+).
inline void init(bool forceNoColor = false) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    g_tty = _isatty(_fileno(stdout)) != 0;
    if (GetConsoleMode(h, &mode)) {
        if (!SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) g_color = false;
    }
#else
    g_tty = isatty(STDOUT_FILENO) != 0;
#endif
    if (!g_tty) g_color = false;
    if (std::getenv("NO_COLOR")) g_color = false;
    if (std::getenv("HOLYBIBLE_FORCE_COLOR")) g_color = true;
    if (forceNoColor) g_color = false;
}

// ---- 256-color palette helpers -------------------------------------------
inline std::string fg(int n) { return g_color ? "\x1b[38;5;" + std::to_string(n) + "m" : ""; }
inline std::string bg(int n) { return g_color ? "\x1b[48;5;" + std::to_string(n) + "m" : ""; }
inline std::string bold()      { return g_color ? "\x1b[1m" : ""; }
inline std::string dim()       { return g_color ? "\x1b[2m" : ""; }
inline std::string italic()    { return g_color ? "\x1b[3m" : ""; }
inline std::string underline() { return g_color ? "\x1b[4m" : ""; }
inline std::string reset()     { return g_color ? "\x1b[0m" : ""; }

inline int cols() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    winsize ws{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) return ws.ws_col;
#endif
    if (const char* c = std::getenv("COLUMNS")) { int v = std::atoi(c); if (v > 20) return v; }
    return 80;
}

inline int rows() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
    winsize ws{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0) return ws.ws_row;
#endif
    if (const char* r = std::getenv("LINES")) { int v = std::atoi(r); if (v > 8) return v; }
    return 30;
}

inline void clear() {
    if (g_tty) std::cout << "\x1b[2J\x1b[H";
    else       std::cout << "\n";
}

// Number of printed columns (skips ANSI escapes, counts UTF-8 code points).
inline int visibleLen(const std::string& s) {
    int n = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = (unsigned char)s[i];
        if (c == 0x1b) {                     // skip ESC [ ... letter
            ++i;
            while (i < s.size() && !std::isalpha((unsigned char)s[i])) ++i;
            continue;
        }
        if ((c & 0xC0) != 0x80) ++n;          // not a UTF-8 continuation byte
    }
    return n;
}

inline std::string rep(const std::string& s, int n) {
    std::string out;
    for (int i = 0; i < n; ++i) out += s;
    return out;
}
inline std::string spaces(int n) { return n > 0 ? std::string((size_t)n, ' ') : std::string(); }

inline std::string padRight(const std::string& s, int w) { return s + spaces(w - visibleLen(s)); }
inline std::string center(const std::string& s, int w) {
    int pad = (w - visibleLen(s)) / 2;
    return spaces(pad) + s;
}

} // namespace term
