// ansi console utilities,
// - rlyeh, public domain

#if 0
#define ANSI_BLINK      "\x1B[5m"
#define ANSI_BLINK_OFF  "\x1B[25m"
#define ANSI_CLEAR_LINE "\x1B[2K"
#endif

#define ANSI_BLUE   "\x1B[34;1m" // bright: on(;1m)
#define ANSI_RED    "\x1B[31;1m" // bright: on(;1m)
#define ANSI_PURPLE "\x1B[35;1m" // bright: on(;1m)
#define ANSI_GREEN  "\x1B[32;1m" // bright: on(;1m)
#define ANSI_CYAN   "\x1B[36;1m" // bright: on(;1m)
#define ANSI_YELLOW "\x1B[33;1m" // bright: on(;1m)
#define ANSI_WHITE  "\x1B[37;1m" // bright: on(;1m)
#define ANSI_GREY   "\x1B[30;1m" // bright: on(;1m)
#define ANSI_RESET  "\x1B[m"     // "\x1B[0m"

#ifdef _WIN32
#include <io.h>
#endif

void ansi(void) {
#ifdef _WIN32
    static int counter = 0;
    if( counter++ ) {
        (printf)(ANSI_RESET);
    } else {
        void *handle;
        DWORD mode;
        if (GetConsoleMode(handle = GetStdHandle(STD_OUTPUT_HANDLE), &mode)) {
            SetConsoleMode(handle, mode | 4); /* ENABLE_VIRTUAL_TERMINAL_PROCESSING. ignore errors */
        }
        _pclose(_popen("chcp 65001 >nul", "r")); // enable unicode
        atexit(ansi);
    }
#endif
}

#define cprintf(fmt, ...) cprintf(va(fmt, __VA_ARGS__))

int (cprintf)(const char *x) {
    do_once ansi();
    const char *colors[] = {
        ANSI_BLUE,
        ANSI_RED,
        ANSI_PURPLE,
        ANSI_GREEN,
        ANSI_CYAN,
        ANSI_YELLOW,
        ANSI_GREY,
//      ANSI_WHITE,
    };
    int bytes = 0;
    for ( int i = 0; x[i]; ++i ) {
        if( (unsigned)x[i] < 8 ) (printf)("%s", colors[x[i]-1]);
        else (printf)("%c", i[x]), ++bytes;
    }
    return bytes;
}

int cputs(const char *x) {
    return !!cprintf("%s\n", x);
}
