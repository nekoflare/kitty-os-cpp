#pragma once

// Reset
#define ANSI_RESET "\033[0m"

// Text Styles
#define ANSI_BOLD "\033[1m"
#define ANSI_DIM "\033[2m"
#define ANSI_ITALIC "\033[3m"
#define ANSI_UNDERLINE "\033[4m"
#define ANSI_BLINK "\033[5m"
#define ANSI_REVERSE "\033[7m"
#define ANSI_HIDDEN "\033[8m"
#define ANSI_STRIKETHROUGH "\033[9m"

// Text Colors
#define ANSI_BLACK "\033[30m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_CYAN "\033[36m"
#define ANSI_WHITE "\033[37m"

// Bright Text Colors
#define ANSI_BRIGHT_BLACK "\033[90m"
#define ANSI_BRIGHT_RED "\033[91m"
#define ANSI_BRIGHT_GREEN "\033[92m"
#define ANSI_BRIGHT_YELLOW "\033[93m"
#define ANSI_BRIGHT_BLUE "\033[94m"
#define ANSI_BRIGHT_MAGENTA "\033[95m"
#define ANSI_BRIGHT_CYAN "\033[96m"
#define ANSI_BRIGHT_WHITE "\033[97m"

// Background Colors
#define ANSI_BG_BLACK "\033[40m"
#define ANSI_BG_RED "\033[41m"
#define ANSI_BG_GREEN "\033[42m"
#define ANSI_BG_YELLOW "\033[43m"
#define ANSI_BG_BLUE "\033[44m"
#define ANSI_BG_MAGENTA "\033[45m"
#define ANSI_BG_CYAN "\033[46m"
#define ANSI_BG_WHITE "\033[47m"

// Bright Background Colors
#define ANSI_BG_BRIGHT_BLACK "\033[100m"
#define ANSI_BG_BRIGHT_RED "\033[101m"
#define ANSI_BG_BRIGHT_GREEN "\033[102m"
#define ANSI_BG_BRIGHT_YELLOW "\033[103m"
#define ANSI_BG_BRIGHT_BLUE "\033[104m"
#define ANSI_BG_BRIGHT_MAGENTA "\033[105m"
#define ANSI_BG_BRIGHT_CYAN "\033[106m"
#define ANSI_BG_BRIGHT_WHITE "\033[107m"

// Cursor Controls
#define ANSI_CURSOR_UP(n) "\033[" #n "A"
#define ANSI_CURSOR_DOWN(n) "\033[" #n "B"
#define ANSI_CURSOR_FORWARD(n) "\033[" #n "C"
#define ANSI_CURSOR_BACK(n) "\033[" #n "D"
#define ANSI_CURSOR_NEXT_LINE(n) "\033[" #n "E"
#define ANSI_CURSOR_PREV_LINE(n) "\033[" #n "F"
#define ANSI_CURSOR_COLUMN(n) "\033[" #n "G"
#define ANSI_CURSOR_POSITION(x, y) "\033[" #y ";" #x "H"

// Screen Controls
#define ANSI_CLEAR_SCREEN "\033[2J"
#define ANSI_CLEAR_LINE "\033[2K"
#define ANSI_CLEAR_LINE_TO_END "\033[K"
#define ANSI_CLEAR_LINE_TO_START "\033[1K"

// Save/Restore Cursor
#define ANSI_SAVE_CURSOR "\033[s"
#define ANSI_RESTORE_CURSOR "\033[u"

// Other
#define ANSI_HIDE_CURSOR "\033[?25l"
#define ANSI_SHOW_CURSOR "\033[?25h"
