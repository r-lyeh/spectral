typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned int   dword;
typedef unsigned int   rgba;

#define joint(a,b) a##b
#define join(a,b)  joint(a,b)

#define do_once  static int join(once,__LINE__) = 1; for( ; join(once,__LINE__) ; join(once,__LINE__) = 0 )
#define __thread __declspec(thread)
#define FIXME(msg) printf("FIXME: " msg " (%s:%d)\n", __FILE__, __LINE__)

#include "sys_file.h"
#include "sys_string.h"
#include "sys_sleep.h"
#include "sys_video.h"
#include "sys_audio.h"
#include "sys_window.h"
#include "sys_db.h"
#include "sys_ui.h"

#ifndef _WIN32
#define min(a,b) ((a)<(b)?(a):(b))
#define FALSE 0
#define ShowCursor(...)     (void)0
#define ClipCursor(...)     (void)0
#define SetWindowTextA(...) (void)0
#define GetWindowRect(...)  (void)0
typedef int RECT;
#define sys_sleep(x)        usleep((x)*1000)
#define _popen popen
#define _pclose pclose
#ifdef __GNUC__ // also, clang
    int __argc;
    char **__argv;
    __attribute__((constructor)) void init_argcv(int argc, char **argv) {
        __argc = argc;
        __argv = argv;
    }
#endif
#endif
