//#pragma GCC warning "warning message"         // gcc/clang only pragma
//#pragma message ( "your warning text here" )  // msc pragma which works in gcc too, albeit not as intended

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned int   dword;
typedef unsigned int   rgba;

#define countof(x) ((int)(sizeof(x) / sizeof(0[x])))

#define joint(a,b) a##b
#define join(a,b)  joint(a,b)

#define do_once  static int join(once,__LINE__) = 1; for( ; join(once,__LINE__) ; join(once,__LINE__) = 0 )
#define FIXME(msg) printf("FIXME: " msg " (%s:%d)\n", __FILE__, __LINE__)

#ifdef _MSC_VER
#define bswap16  _byteswap_ushort
#define __thread __declspec(thread)
#else
#define bswap16 __builtin_bswap16
#endif

#ifdef _WIN32
#define mkdir(p,m)    mkdir(p)
#else
#include <limits.h>
#define MAX_PATH PATH_MAX // (defined in limits.h)

#define GetAsyncKeyState(vk) 0
#define VK_SNAPSHOT 0
#define strcmpi       strcasecmp
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define FALSE 0
#define ShowCursor(...)     (void)0
#define ClipCursor(...)     (void)0
#define GetWindowRect(...)  (void)0
typedef int RECT;
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

#ifndef __GNUC__
#include <immintrin.h>
#define __builtin_clz(x)   _lzcnt_u32(x)
#define __builtin_clzll(x) _lzcnt_u64(x)
#define __builtin_ctz(x)   _tzcnt_u32(x)
#define __builtin_ctzll(x) _tzcnt_u64(x)
#endif
