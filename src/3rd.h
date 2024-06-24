//-----------------------------------------------------------------------------
// 3rd party libs

#define KISSDB_IMPLEMENTATION
#include "3rd_kissdb.h"

#define TIGR_C
//#define TIGR_DO_NOT_PRESERVE_WINDOW_POSITION // @fixme: make it centered
#define run run2
#define border border2
#include "3rd_tigr.h"
#include "3rd_tigrobjc.h"
#include "3rd_tigrmousecursor.h"
#include "3rd_tigrdragndrop.h"
#undef border
#undef run

#if 0
#define LUA_IMPL                              // lua544
#define TK_END TK_END2
#define TK_RETURN TK_RETURN2
#define block block2
#include "3rd_lua.h"
#undef TK_END
#undef TK_RETURN
#endif

#define SOKOL_AUDIO_IMPL
#include "3rd_sokolaudio.h"

#define DEFLATE_C
#include "3rd_deflate.h"
#include "3rd_zlib.h" // for zlib streams, like CSW2
#include "3rd_gz.h" // gunzip
#define ZIP_C
#include "3rd_zip.h"
#include "3rd_rar.h"
#define DIR_C
#include "3rd_dir.h"
#define STB_IMAGE_IMPLEMENTATION
#include "3rd_stbimage.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "3rd_stbimage_resize2.h"
#define THREAD_IMPLEMENTATION
#include "3rd_thread.h"
#include "3rd_bin.h"

#if 1
#define TFD_IMPLEMENTATION
//#define GetForegroundWindow GetActiveWindow
#include "3rd_tfd.h"
//#undef  GetForegroundWindow
#else
#include "3rd_osdialog.h"
#include "3rd_osdialog.c"
#  if defined __APPLE__
#include "3rd_osdialog_mac.m"
#elif defined _WIN32
#include "3rd_osdialog_win.c"
#pragma comment(lib, "comdlg32")
#else
//#include "3rd_osdialog_gtk2.c"
#include "3rd_osdialog_gtk3.c"
#endif
#endif
