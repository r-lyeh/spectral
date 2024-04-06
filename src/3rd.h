//-----------------------------------------------------------------------------
// 3rd party libs

#define TIGR_C
#define TIGR_DO_NOT_PRESERVE_WINDOW_POSITION
#include "3rd_tigr.h"

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
#define ZIP_C
#include "3rd_zip.h"
#define DIR_C
#include "3rd_dir.h"