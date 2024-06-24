// known issues:

// starts in turborom mode ??? see battle valley

// runahead
// - bonanza bros.dsk

// tape buttons

// tape
// - red/cyan tape freq bars

// quickload
// - crash by pressing F12 after main window is displayed

// media
// scl not being saved on exit (borderbreak)

// sav
// - .db2 in .sav or binary .ini
// - no settings in .sav
// - add setting: games folder, 

// gui
// - utf8 no czech/hungarian/slovak: ČĎĚŇŘŠŤŽ čďěňřšťž Ýý ÔôŐő ŰűŮů Ĺĺ Ľľ Ŕŕ (see tabs: d,k,m,p,r,t)
// - no selector (pok)
// - no inputbox
// - no notify black panel
// - no notify scroll

// gallery
// - cadaverion,coliseum: no zxdb info
// - champions of cricket + world cup cricket : crash (html)
// - no proper model reset (see return null)
// - text mode corrupts vram
// - no download bar
// - no flashcolor x2
// - no ifl/mlt
// - wrong cache path (no zxdb id) .Spectral/Z/Zizala-#88.zip

// zxdb
// - infos get lost between different .sav sessions
// - overlay: no scroll (mouse/key), no zooming
// - JACKNIP.TAP ; difficult to get it right without hyphenation. best we could do for now is search JACKNIP%
// - reset does not clear zxdb
// - instructions: wordwrap, utf8 bom
// - no mp3s
// - no ays
// - maps: battle valley

// zxplayer
// - cant load zip/rar files because FILE *fp is not pointing at seek pos

// trdos+L mode

// 128/+2:
// - parapshock should break; it doesnt
// - parapshock + turborom
// .ay files:
// - could use left/right keys to change tracks
// - could fill vram so it displays author, filename, etc datas
// - felt the 0x8000/0x10000 magic numbers in zx_ay can be removed if we switch vars to be unsigned
// .dsk files:
// - no offset-info\r\n extension. see: mercs(samdisk), paperboy2
// .sav files:
// - may be corrupt. repro steps: many save/loads (see: jacknipper2 menu, RAIDERS.ROM screen$)
// - may be corrupt. repro steps: save during tape load or disk load
// - tape marker not very exact in turborom medias
// altroms:
// - donkeykong(erbe) (128): no ay (wrong checksum?)
// - bloodbrothers (128): no ay (wrong checksum?)
// - travelwithtrashman (128): crash (wrong checksum?)
// - r-type128-km-nocheats (lg18v07 + usr0): r tape err (no turbo) or crash (turbo)
// - lg18,gw03,sebasic fail in the games above. jgh v0.77 seems to work. old sebasic partially too
// auto-locale:
// - will corrupt medias with checksums (like tap files)
// ay:
// - Fuller Box AY chip is clocked at 1.63819 MHz
// - ABC/ACB modes when stereo is enabled
// beeper:
// - fix: indiana jones last crusade
// - p-47 in-game click sound (ayumi? beeper?). double check against real zx spectrum
// - OddiTheViking128
// crash (vsync/int line?):
// - hostages(erbe) +2, +3 versions. 48 is ok.
// disciple/plusd/mgt (lack of):
// - zxoom.z80
// floating:
// - not working yet
// - gloop (probably)
// keyboard:
// - modern typing. eg, shift+2 for quotes, not symb+P.
// lightgun:
// - gunstick + (kempston || kmouse) conflicts
// mouse:
// - kms window focus @ MMB+tigr, kms wrap win borders, kms fullscreen, kms coord wrap (inertia, r-type128-km)
// - load rtype-km, load another game or reset, notice no mouse cursor
// pentagon:
// - AY should clock at 1.75 MHz exact, CPU at 3.50 MHz exact
// - no autoboot (autoboot is activated by paging in TR-DOS rom and jumping to 0)
//   "Unreal Speccy Portable checks for a boot.B file in firsts sectors. If not found, send two keystrokes (Enter). That only makes sense on Pentagon machines with the reset service rom (gluck), the first Enter show a list of files and the latter selects the first file."
// ports:
// - linux: no mouse clip
// - osx: no mouse clip
// - osx: retina too heavy?
// timing:
// - border: sentinel, defenders of earth, dark star, gyroscope II, super wonder boy (pause)
// - border: aquaplane, olympic games, vectron, mask3, jaws, platoon
// - multicolor: mask3, shock megademo, MDA, action force 2, old tower, hardy
// - contended: exolon main tune requires contended memory, otherwise it's too fast +15% tempo
// - floating: arkanoid (original), cobra (original), sidewize, short circuit
// trdos
// - page in .z80 not handled
// turborom:
// - x4,x6 modes not working anymore. half bits either.
// tzx:
// - flow,gdb

FILE *printer;

int do_audio = 1;

// ZX

int ZX_TS;
int ZX_FREQ;
int ZX_RF = !DEV;
int ZX_CRT = !DEV;
int ZX; // 16, 48, 128, 200 (+2), 210 (+2A), 300 (+3)
int ZX_AY = 1; // 0: no, 1: ayumi, 2: flooh's, 3: both (ZX_AY is a mask)
int ZX_TURBOROM = 0; // 0: no, 1: patch rom so loading standard tape blocks is faster (see: .tap files)
int ZX_JOYSTICK = 3; // 0: no, 1: sinclair1, 2: sinclair2, 3: cursor/fuller/kempston/protek, 4..: other mappings
int ZX_AUTOPLAY = 0; // yes/no: auto-plays tapes based on #FE port reads
int ZX_AUTOSTOP = 0; // yes/no: auto-stops tapes based on #FE port reads
int ZX_RUNAHEAD = 0; // yes/no: improves input latency
int ZX_MOUSE = 1; // yes/no: kempston mouse
int ZX_ULAPLUS = 1; // yes/no: ulaplus 64color mode
int ZX_GUNSTICK = 0; // yes/no: gunstick/lightgun @fixme: conflicts with kempston & kmouse
int ZX_FPS = 100; // fps multiplier: 0 (max), x100 (50 pal), x120 (60 ntsc), x200 (7mhz), x400 (14mhz)
int ZX_AUTOLOCALE = 0; // yes/no: automatically patches games from foreign languages into english
int ZX_FASTCPU = 0; // yes/no: max cpu speed
int ZX_FASTTAPE = 1; // yes/no: max tape speed

int ZX_PENTAGON = 0; // DEV; // whether the 128 model emulates the pentagon or not

int ZX_KLMODE = 0; // 0:(K mode in 48, default), 1:(L mode in 48)
int ZX_KLMODE_PATCH_NEEDED = 0; // helper internal variable, must be init to match ZX_KLMODE

int ZX_PLAYER = 0; // 0:app is full featured emulator, 1:app is a small zx player with reduced functionality

int ZX_BROWSER = 2; // game browser version to use, currently only v1 is supported

int ZX_DEVTOOLS = 0; // 0: regular, 1: development tools (@todo: profiler,analyzer)
int ZX_DEBUG = 0; // status bar, z80 disasm

const
int ZX_ALTROMS = 0; // 0:(no, original), 1:(yes, custom)

int ZX_INPUT = 1;

void logport(word port, byte value, int is_out);
void outport(word port, byte value);
byte inport(word port);

void port_0x00fe(byte value);
void port_0x1ffd(byte value);
void port_0x7ffd(byte value);
void port_0xbffd(byte value);
void port_0xfffd(byte value);

enum { KEEP_MEDIA = 1, QUICK_RESET = 2 };
void boot(int model, unsigned flags);
void reset(unsigned flags);
void eject();

void run(unsigned TS);

void frame_new();
void frame(int drawmode, int do_sim); // no render (<0), whole frame (0), scanlines (1)

void* quicksave(unsigned slot);
void* quickload(unsigned slot);

unsigned border[71680];
unsigned border_num;
uint64_t border_org;

// z80
z80_t cpu;
uint64_t pins;
int int_counter;

// mem
    byte *MEMr[4]; //solid block of 16*4 = 64kb for reading
    byte *MEMw[4]; //solid block of 16*4 = 64kb for writing
    #define RAM_BANK(n)   (mem + (n) * 0x4000)
    #define ROM_BANK(n)   (rom + (n) * 0x4000)
    #define DUMMY_BANK(n) (dum + (0) * 0x4000)
    #define VRAM RAM_BANK(5 + ((page128 & 8) >> 2)) // branchless ram bank 5 or 7. equivalent to RAM_BANK(page128 & 8 ? 7 : 5)

    #define GET_MAPPED_ROMBANK() ((page128 & 16 ? 1 : 0) | ((page2a & 5) == 4 ? 2 : 0)) // 0, 0/1 or 0/1/2/3
    #define GET_BASIC_ROMBANK() ((ZX>=128) + 2*(ZX>=210)) // 0 (16/48), 1 (128/+2) or 3 (+2A/+3)
    #define GET_EDITOR_ROMBANK() (!(ZX<=200)) // 0 (128/+2) or 1 (+2A/+3)
    #define GET_3DOS_ROMBANK() (2 * (ZX >= 210)) // 2 (+2A/+3)
    #define ROM_BASIC()  ROM_BANK(GET_BASIC_ROMBANK())
    #define ROM_EDITOR() ROM_BANK(GET_EDITOR_ROMBANK())
    #define ROM_3DOS()   ROM_BANK(GET_3DOS_ROMBANK())

    int vram_contended;
    int vram_accesses;
    //byte contended[70908];
    unsigned short floating48[70908], floating128[70908], *floating_bus;

    byte *mem; // 48k
    byte *rom;
    byte *dum;

    // 16/48/128/+2: pages 1,3,5,7 are contended (1), 0,2,4,6 not contended (0) -> so mask is 0001 (1)
    // +2A/+3:       pages 4,5,6,7 are contended (1), 0,1,2,3 not contended (0) -> so mask is 0100 (4)
    //#define IS_CONTENDED_PAGE(addr) !!((addr >> 14) & (ZX >= 210 ? 4 : 1))

    #define ADDR8(a)     ((byte *)&MEMr[(a)>>14][(a)&0x3FFF])
    #define READ8(a)     (/*vram_accesses += vram_contended && ((a)>>14==1),*/ *(byte *)&MEMr[(a)>>14][(a)&0x3FFF])
    #define READ16(a)    (/*vram_accesses += vram_contended && ((a)>>14==1),*/ *(word *)&MEMr[(a)>>14][(a)&0x3FFF])
    #define WRITE8(a,v)  (/*vram_accesses += vram_contended && ((a)>>14==1),*/ *(byte *)&MEMw[(a)>>14][(a)&0x3FFF]=(v))
    #define WRITE16(a,v) (/*vram_accesses += vram_contended && ((a)>>14==1),*/ *(word *)&MEMw[(a)>>14][(a)&0x3FFF]=(v))

// ula
int  ZXFlashFlag;
rgba ZXPalette[64];
byte ZXBorderColor;

// 128
byte page128;

// plus3/2a
byte page2a;

// betadisk
byte beta128;

// keyboard
int issue2;
int keymap[5][5];

// joysticks
byte kempston,fuller;

// mouse
byte kempston_mouse;

// beeper
#define TAPE_VOLUME 0.15f // relative to ~~buzz~~ ay
#define BUZZ_VOLUME 0.25f // relative to ay
beeper_t buzz;
byte ear;
byte spk;

// ay
ay38910_t ay;
struct ayumi ayumi;
byte ay_current_reg;
int/*byte*/ ay_registers[ 16 ];
void ay_reset() {
    memset(ay_registers,0,sizeof(ay_registers));
    ay_current_reg=0;
}

// vsync
byte zx_int;

// ticks
uint64_t ticks, TS;

// tape
int tape_hz; // for vis
uint64_t tape_ticks;

// autoplay vars
int autoplay;
int autostop;
int autoplay_freq;
uint64_t autoplay_last;
unsigned autoplay_numreads;

// wd1793
WD1793 wd;
FDIDisk fdd[NUM_FDI_DRIVES];

// medias (tap,tzx,dsk...)
struct media_t {
    int len;
    byte *bin;
    double pos;
} media[16];
int medias;
void media_reset() { medias = 0; for(int i=0;i<16;++i) media[i].bin = realloc(media[i].bin, media[i].len = media[i].pos = 0); }
void media_mount(const byte *bin, int len) { media[medias].bin = memcpy(realloc(media[medias].bin, media[medias].len = len), bin, len), media[medias].pos = 0, medias++; }

enum { ALL_FILES = 0, GAMES_ONLY = 6*4, TAPES_AND_DISKS_ONLY = 9*4, DISKS_ONLY = 12*4 };
int file_is_supported(const char *filename, int skip) {
    const char *ext = strrchr(filename ? filename : "", '.');
    return ext && strstri(skip+".gz .zip.rar.pok.scr.ay .rom.sna.z80.tap.tzx.csw.dsk.img.mgt.trd.fdi.scl.$b.$c.$d.", ext);
}

#include "zx_ay.h"
#include "zx_dis.h"
#include "zx_rom.h"
#include "zx_dsk.h"
#include "zx_tap.h" // requires page128
#include "zx_tzx.h"
#include "zx_sna.h" // requires page128, ZXBorderColor
#include "zx_ula.h"
#include "zx_db.h"

// 0: cannot load, 1: snapshot loaded, 2: tape loaded, 3: disk loaded
int loadbin_(const byte *ptr, int size, int preloader) {
    if(!(ptr && size > 10))
        return 0;

    if( preloader ) {
        int model = guess(ptr, size);
        ZX_PENTAGON = model < 0; model = abs(model);
        if( ZX != model ) boot(model, ~0u);
        else reset(ZX);
    }

    ZX_AUTOPLAY = 1;
    ZX_AUTOSTOP = 0;

    #define preload_snap(blob,len) ( sna_load(blob,len) || z80_load(blob,len) )

    // pre-loaders
    const byte*    bins[] = {
        ld16bas, ld48bas, ld128bas, ldplus2bas, ldplus2abas, ldplus3bas,
        ld16bin, ld48bin, ld128bin, ldplus2bin, ldplus2abin, ldplus3bin,
    };
    const unsigned lens[] = {
        sizeof ld16bas, sizeof ld48bas, sizeof ld128bas, sizeof ldplus2bas, sizeof ldplus2abas, sizeof ldplus3bas,
        sizeof ld16bin, sizeof ld48bin, sizeof ld128bin, sizeof ldplus2bin, sizeof ldplus2abin, sizeof ldplus3bin,
    };

    if( load_ay(ptr, (int)size) ) {
        return 1;
    }

    // dsk first
    if(!memcmp(ptr, "MV - CPC", 8) || !memcmp(ptr, "EXTENDED", 8)) {
        if(preloader) preload_snap(ldplus3, sizeof(ldplus3));
        return dsk_load(ptr, size), 3;
    }

    // tapes first
    if( tzx_load(ptr, (int)size) ) {
        int slots[] = { [1]=0,[3]=1,[8]=2,[12]=3,[13]=4,[18]=5 };
        int is_bin = tape_type == 3, choose = slots[ZX/16] + 6 * is_bin;
        if(preloader) preload_snap(bins[choose], lens[choose]);
        //if(tape_has_turbo) rom_restore(); // rom_restore(), rom_patch(tape_has_turbo ? 0 : do_rompatch);
        ZX_AUTOSTOP = tape_num_stops > 1 ? 0 : size > 65535;
        //alert(va("numstops:%d", tape_num_stops));
        return 2;
    }
    if( tap_load(ptr,(int)size) ) {
        int slots[] = { [1]=0,[3]=1,[8]=2,[12]=3,[13]=4,[18]=5 };
        int is_bin = tape_type == 3, choose = slots[ZX/16] + 6 * is_bin;
        if(preloader) preload_snap(bins[choose], lens[choose]);
        ZX_AUTOSTOP = size > 65535;
        return 2;
    }
    if( csw_load(ptr,(int)size) ) {
        int slots[] = { [1]=0,[3]=1,[8]=2,[12]=3,[13]=4,[18]=5 };
        int is_bin = tape_type == 3, choose = slots[ZX/16] + 6 * is_bin;
        if(preloader) preload_snap(bins[choose], lens[choose]);
        rom_restore(); // rom_restore(), rom_patch(tape_has_turbo ? 0 : do_rompatch);
        ZX_AUTOSTOP = 1;
        return 2;
    }

    // headerless fixed-size formats now, sorted by ascending file size.
    if( scr_load(ptr, size) ) {
        return 1;
    }
    if( rom_load(ptr, size) ) {
        return 1;
    }
    if( sna_load(ptr, size) ) {
        return regs("load .sna"), 1;
    }

    // headerless variable-size formats now
    if( *ptr == 'N' && pok_load(ptr, size) ) {
        return 1;
    }
    if( size > 87 && z80_load(ptr, size) ) {
        return regs("load .z80"), 1;
    }

    puts("unknown file format");
    return 0;
}

int loadbin(const byte *ptr, int size, int preloader) {
    if(!(ptr && size))
        return 0;

    int ret = loadbin_(ptr, size, preloader);
    if( ret > 1 ) media_mount(ptr, size);
    return ret;
}


static byte* merge;
static size_t merge_len;
void glue_init() {
    free(merge); merge = 0;
    merge_len = 0;
}
void *glue(const byte *data, size_t len) {
    merge = realloc(merge, merge_len + len);
    memcpy(merge + merge_len, data, len);
    merge_len += len;
    return merge;
}
size_t glue_len() {
    return merge_len;
}
void *zip_read(const char *filename, size_t *size) {
    glue_init();

    // up to 64 tapes per zip (usually up to 4 max)
    char *alpha_tapes[64] = {0};
    int count_tapes = 0;

    // quick rar test
    rar *r = rar_open(filename, "rb");
    if( r ) {
        for( unsigned i = 0 ; count_tapes < 64 && i < rar_count(r); ++i ) {
            if( !file_is_supported(rar_name(r,i), ALL_FILES) ) continue;

            printf("  %d) ", i+1);
            printf("[%08X] ", rar_hash(r,i));
            printf("$%02X ", rar_codec(r,i));
            printf("%s ", rar_modt(r,i));
            printf("%5s ", rar_file(r,i) ? "" : "<dir>");
            printf("%11u ", rar_size(r,i));
            printf("%s ", rar_name(r,i));
            void *data = rar_extract(r,i);
            printf("\r%c\n", data ? 'Y':'N'); // %.*s\n", rar_size(r,i), (char*)data);

            // append data to merge
            glue(data, rar_size(r,i));
            if(size) *size = merge_len;
            rar_free(r,data);

            // keeping glueing tapes
            if(strstr(rar_name(r,i),".tap") || strstr(rar_name(r,i),".tzx")) {
            continue;
            }
            break;
        }

        rar_close(r);
        return merge;
    }


    // test contents of file
    zip *z = zip_open(filename, "rb");
    if( z ) {
        for( unsigned i = 0 ; count_tapes < 64 && i < zip_count(z); ++i ) {
            void *data = file_is_supported(zip_name(z,i),ALL_FILES) ? zip_extract(z,i) : 0;
            if(!data) continue;
            free(data);

            alpha_tapes[count_tapes++] = zip_name(z,i);
        }

        if( count_tapes )
        qsort(alpha_tapes, count_tapes, sizeof(char *), qsort_strcmp);

        for( unsigned j = 0 ; j < count_tapes; ++j ) {
            int i = zip_find(z, alpha_tapes[j]); // convert entry to index. returns <0 if not found.

            printf("  %d) ", i+1);
            printf("[%08X] ", zip_hash(z,i));
            printf("$%02X ", zip_codec(z,i));
            printf("%s ", zip_modt(z,i));
            printf("%5s ", zip_file(z,i) ? "" : "<dir>");
            printf("%11u ", zip_size(z,i));
            printf("%s ", zip_name(z,i));
            void *data = zip_extract(z,i);
            printf("\r%c\n", data ? 'Y':'N'); // %.*s\n", zip_size(z,i), (char*)data);

            // append data to merge
            glue(data, zip_size(z,i));
            if(size) *size = merge_len;
            free(data);

            // keeping glueing tapes
            if(strstr(zip_name(z,i),".tap") || strstr(zip_name(z,i),".tzx")) {
            continue;
            }
            break;
        }

        zip_close(z);
        return merge;
    } else {
        printf("no zip/rar contents in %s archive\n", filename);
    }
    return 0;
}

static struct zxdb ZXDB;
static char *last_load;
int loadfile(const char *file, int preloader) {
    if( !file ) return 0;

    const char *bak = file;
    file = last_load = (free(last_load), strdup(file));

#if TESTS
    printf("\n\n%s\n-------------\n\n", file);
#endif

    char *ptr = 0; size_t size = 0;
#if 0
    if( strstr(file, ".zip") || strstr(file,".rar") ) {
        ptr = zip_read(file, &size); // @leak
#else
    ptr = zip_read(file, &size); // @leak
    if( 1 ) {
#endif
        if( ptr ) {
            // update file from archived filename. use 1st entry if multiple entries on zipfile are found
            for( zip *z = zip_open(file, "rb"); z; zip_close(z), z = 0 )
            for( unsigned i = 0 ; i < zip_count(z); ++i ) {
                if( file_is_supported(zip_name(z,i), GAMES_ONLY) ) {
                    free(last_load);
                    file = last_load = strdup(zip_name(z,i));
                    break;
                }
            }
            // update file from archived filename. use 1st entry if multiple entries on zipfile are found
            for( rar *r = rar_open(file, "rb"); r; rar_close(r), r = 0 )
            for( unsigned i = 0 ; i < rar_count(r); ++i ) {
                if( file_is_supported(rar_name(r,i), GAMES_ONLY) ) {
                    free(last_load);
                    file = last_load = strdup(rar_name(r,i));
                    break;
                }
            }
        }
    }

    if(!ptr)
    for( FILE *fp = fopen(file,"rb"); fp; fclose(fp), fp = 0) {
        fseek(fp, 0L, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        ptr = malloc(size); // @leak
        fread(ptr, 1, size, fp);
    }

#if 1 // gunzip
    if( ptr && !memcmp(ptr, "\x1f\x8b\x08", 3) ) {
        static char *unc = 0;
        unsigned unclen;
        unc = gunzip(ptr, size, &unclen);
        if( unc ) {
            free( ptr );
            ptr = unc;
            size = unclen;
        }
    }
#endif

    if( ZX_AUTOLOCALE ) {
        translate(ptr, size, 'en');
    }

    int ok = 0;
    const char *ext = strrchr(file, '.');
    const char *extensions = ".img.mgt.trd.fdi.scl.$b .$c ."; // order must match FMT enum definitions in FDI header
    const char *extfound = ext ? strstri(extensions, ext) : NULL;
    if( !extfound ) {
        // regular load
        ok = loadbin(ptr, size, preloader);
    } else {
        // betadisk load
        printf("found ext: %d\n", (int)(extfound - extensions) );
        int format = FMT_IMG + ( extfound - extensions ) / 4;
        if( format > FMT_HOBETA ) format = FMT_HOBETA;
        printf(
            "FMT_IMG = %d\n"
            "FMT_MGT = %d\n"
            "FMT_TRD = %d\n"
            "FMT_FDI = %d\n"
            "FMT_SCL = %d\n"
            "FMT_HOB = %d\n" 
            "format = %d\n", FMT_IMG, FMT_MGT, FMT_TRD, FMT_FDI, FMT_SCL, FMT_HOBETA, format
        );

        // this temp file is a hack for now. @fixme: implement a proper file/stream abstraction in FDI library
        for( FILE *fp = fopen("spectral.$$$", "wb"); fp; fwrite(ptr, size, 1, fp), fclose(fp), fp = 0);
        ok = LoadFDI(&fdd[0], "spectral.$$$", format);
        unlink("spectral.$$$");

        if( ok ) {
            ZX_PENTAGON = 1;
            ZX = 128;
            boot(ZX, KEEP_MEDIA);

            // type RUN+ENTER if bootable disk found
            extern window *app;
            if( !window_pressed(app, TK_SHIFT) )
            if( memmem(ptr, size, "boot    B", 9) ) {
                loadbin(ldtrdos, ldtrdos_length, 0);
            }
        }
    }

    if( ok && preloader ) { // probably a game, so use ZXDB
        // ZXDB = zxdb_free(ZXDB);
        zxdb_print( ZXDB = zxdb_search(file) );

        if( !ZXDB.tok )
        zxdb_print( ZXDB = zxdb_search(bak) );
    }

    if( !ok ) {
        if( ptr ) free(ptr);
    }

    return ok;
}


enum SpecKeys {
    ZX_0,ZX_1,ZX_2,ZX_3,ZX_4,ZX_5,  ZX_6,ZX_7,ZX_8,ZX_9,ZX_A,ZX_B,  ZX_C,ZX_D,ZX_E,ZX_F,ZX_G,ZX_H,
    ZX_I,ZX_J,ZX_K,ZX_L,ZX_M,ZX_N,  ZX_O,ZX_P,ZX_Q,ZX_R,ZX_S,ZX_T,  ZX_U,ZX_V,ZX_W,ZX_X,ZX_Y,ZX_Z,
    ZX_SPACE,ZX_ENTER,ZX_SHIFT,ZX_SYMB,ZX_CTRL
};
#define ZXKey(a) ( keymap[ keytbl[a][0] ][ keytbl[a][1] ] &= keytbl[a][2] )
#define ZXKeyboardClear() \
    keymap[1][1] = keymap[1][2] = \
    keymap[2][1] = keymap[2][2] = \
    keymap[3][1] = keymap[3][2] = \
    keymap[4][1] = keymap[4][2] = 0xFF;

const unsigned char keytbl[256][3] = {
    {1, 2, 0xFE}, {1, 1, 0xFE}, {1, 1, 0xFD}, /* 0|1|2 */
    {1, 1, 0xFB}, {1, 1, 0xF7}, {1, 1, 0xEF}, /* 3|4|5 */
    {1, 2, 0xEF}, {1, 2, 0xF7}, {1, 2, 0xFB}, /* 6|7|8 */
    {1, 2, 0xFD}, {3, 1, 0xFE}, {4, 2, 0xEF}, /* 9|a|b */
    {4, 1, 0xF7}, {3, 1, 0xFB}, {2, 1, 0xFB}, /* c|d|e */
    {3, 1, 0xF7}, {3, 1, 0xEF}, {3, 2, 0xEF}, /* f|g|h */
    {2, 2, 0xFB}, {3, 2, 0xF7}, {3, 2, 0xFB}, /* i|j|k */
    {3, 2, 0xFD}, {4, 2, 0xFB}, {4, 2, 0xF7}, /* l|m|n */
    {2, 2, 0xFD}, {2, 2, 0xFE}, {2, 1, 0xFE}, /* o|p|q */
    {2, 1, 0xF7}, {3, 1, 0xFD}, {2, 1, 0xEF}, /* r|s|t */
    {2, 2, 0xF7}, {4, 1, 0xEF}, {2, 1, 0xFD}, /* u|v|w */
    {4, 1, 0xFB}, {2, 2, 0xEF}, {4, 1, 0xFD}, /* x|y|z */
    {4, 2, 0xFE}, {3, 2, 0xFE}, {4, 1, 0xFE}, /* SPC|ENT|SHF */
    {4, 2, 0xFD}, {1, 2, 0xEF},               /* SYMB|CTRL */
};

// 16/48
void port_0x00fe(byte value) {
    // border color
    ZXBorderColor = (value & 0x07);

    border[border_num++] = (ZXBorderColor << 24) | (ticks - border_org);

    // speaker
    spk = value;
}

// zx128
void port_0x7ffd(byte value) {
    if(ZX >= 128)
    if(!(page128 & 32)) { //if bit5 not locked
        // check bit 2-0: RAM0/7 -> 0xc000-0xffff
        MEMw[3]=MEMr[3]=RAM_BANK(value&7);

        //16/48/128/+2: pages 1,3,5,7 are contended (1), 0,2,4,6 not contended (0) -> so mask is 0001 (1)
        //+2A/+3:       pages 4,5,6,7 are contended (1), 0,1,2,3 not contended (0) -> so mask is 0100 (4)
//MEMc[3]=value & contended_mask;

        //128:    check bit 4 : rom selection (ROM0/1 -> 0x0000-0x3FFF)
        //+2A/+3: check high bit of rom selection too (same as offset ROM selection to 0x8000) 1x0 101
        MEMr[0]=ROM_BANK((value & 16 ? 1 : 0) | ((page2a & 5) == 4 ? 2 : 0));

        page128=value;
    }
}

void ZXJoysticks(int up, int down, int left, int right, int fire) {
    // kempston/i2l + fuller, then either sinclair1 or sinclair2 or cursor/protek/agf
    int joy[][6] = {
        { 0 },
        { ZX_1,ZX_2,ZX_4,ZX_3,ZX_5,ZX_5 },
        { ZX_6,ZX_7,ZX_9,ZX_8,ZX_0,ZX_0 },
        { ZX_5|0x80,ZX_8|0x80,ZX_7|0x80,ZX_6|0x80,ZX_0|0x80,ZX_0|0x80 },
        //OPQAM/SP,OP1QM/SP,KLAZM/SP,ZXPL0/SP,QABNM/SP,QZIPM/SP,1ZIPM/SP,670OM/SP,QWOKC/SP
        { ZX_O,ZX_P,ZX_Q,ZX_A,ZX_M,ZX_SPACE },
        { ZX_O,ZX_P,ZX_1,ZX_Q,ZX_M,ZX_SPACE },
        { ZX_K,ZX_L,ZX_A,ZX_Z,ZX_M,ZX_SPACE },
        { ZX_Z,ZX_X,ZX_P,ZX_L,ZX_0,ZX_SPACE },
        { ZX_B,ZX_N,ZX_Q,ZX_A,ZX_M,ZX_SPACE },
        { ZX_I,ZX_P,ZX_Q,ZX_Z,ZX_M,ZX_SPACE },
        { ZX_I,ZX_P,ZX_1,ZX_Z,ZX_M,ZX_SPACE },
        { ZX_6,ZX_7,ZX_0,ZX_O,ZX_M,ZX_SPACE },
        { ZX_Q,ZX_W,ZX_O,ZX_K,ZX_C,ZX_SPACE },
    }, *j = joy[ZX_JOYSTICK];
    kempston=0; fuller=0xff;
    if( ZX_JOYSTICK ) {
    if(left)  { kempston|=2;  fuller&=0xFF-4;   ZXKey(j[0] & 0x7f); if(j[0] & 0x80) ZXKey(ZX_SHIFT); }
    if(right) { kempston|=1;  fuller&=0xFF-8;   ZXKey(j[1] & 0x7f); if(j[1] & 0x80) ZXKey(ZX_SHIFT); }
    if(up)    { kempston|=8;  fuller&=0xFF-1;   ZXKey(j[2] & 0x7f); if(j[2] & 0x80) ZXKey(ZX_SHIFT); }
    if(down)  { kempston|=4;  fuller&=0xFF-2;   ZXKey(j[3] & 0x7f); if(j[3] & 0x80) ZXKey(ZX_SHIFT); }
    if(fire)  { kempston|=16; fuller&=0xFF-128; ZXKey(j[4] & 0x7f); if(j[4] & 0x80) ZXKey(ZX_SHIFT); }
    }
}

// fdc
// fdc_ports.c -----------------------------------------------------------------

// control of port 0x1ffd (+2a/+3 -> mem/fdc/ptr)
void port_0x1ffd(byte value) {
    if(ZX < 210) return; //if not in +2a/+3 mode, return

    page2a = value; // value & 0x1f;    //save bits 0-4

    // check bit 1: special RAM banking or not

    if(value & 1)
    {
        switch(value & 7)
        {
            case 1 : //001=ram:0123
                MEMw[3]=MEMr[3]=RAM_BANK(3); // MEMc[3]=0;
                MEMw[2]=MEMr[2]=RAM_BANK(2); // MEMc[2]=0;
                MEMw[1]=MEMr[1]=RAM_BANK(1); // MEMc[1]=0;
                MEMw[0]=MEMr[0]=RAM_BANK(0); // MEMc[0]=0;
                break;

            case 3 : //011=ram:4567
                MEMw[3]=MEMr[3]=RAM_BANK(7); // MEMc[3]=1;
                MEMw[2]=MEMr[2]=RAM_BANK(6); // MEMc[2]=1;
                MEMw[1]=MEMr[1]=RAM_BANK(5); // MEMc[1]=1;
                MEMw[0]=MEMr[0]=RAM_BANK(4); // MEMc[0]=1;
                break;

            case 5 : //101=ram:4563
                MEMw[3]=MEMr[3]=RAM_BANK(3); // MEMc[3]=0;
                MEMw[2]=MEMr[2]=RAM_BANK(6); // MEMc[2]=1;
                MEMw[1]=MEMr[1]=RAM_BANK(5); // MEMc[1]=1;
                MEMw[0]=MEMr[0]=RAM_BANK(4); // MEMc[0]=1;
                break;

            case 7 : //111=ram:4763
                MEMw[3]=MEMr[3]=RAM_BANK(3); // MEMc[3]=0;
                MEMw[2]=MEMr[2]=RAM_BANK(6); // MEMc[2]=1;
                MEMw[1]=MEMr[1]=RAM_BANK(7); // MEMc[1]=1;
                MEMw[0]=MEMr[0]=RAM_BANK(4); // MEMc[0]=1;
                break;
        }
    } else {
        port_0x7ffd(page128);

        //restart locked_in_128_model values if touched by +2A or +3 emulation previously (see value & 1)
        MEMw[2]=MEMr[2]=RAM_BANK(2); //MEMc[2]=0;
        MEMw[1]=MEMr[1]=RAM_BANK(5); //MEMc[1]=1;
        MEMw[0]=DUMMY_BANK(0);       //MEMc[0]=0;
    }

    //bit 3: motor on/off
    if(ZX == 300) fdc_motor(value & 8);

    //bit 4: printer strobe
}


// control of port 0x2ffd (fdc in)
byte inport_0x2ffd(void) {
    return ZX == 300 ? fdc_read_status() : 0xFF;
}

// control of port 0x3ffd (fdc out & in)
byte inport_0x3ffd(void) {
    return ZX == 300 ? fdc_read_data() : 0xFF;
}

void port_0x3ffd(byte value) {
    if( ZX == 300 ) fdc_write_data(value);
}


void config(int ZX) {
    if(ZX >= 16) {
        MEMr[3]=DUMMY_BANK(0);
        MEMr[2]=DUMMY_BANK(0);
        MEMr[1]=RAM_BANK(5);
        MEMr[0]=ROM_BANK(0);

        MEMw[3]=RAM_BANK(0);
        MEMw[2]=RAM_BANK(2);
        MEMw[1]=RAM_BANK(5);
        MEMw[0]=DUMMY_BANK(0);

        issue2=1;
        page128=32; // -1
        page2a=128; // -1
        //contended_mask=1;

        ZX_TS = 69888;
        ZX_FREQ = 3500000;
    }

    if(ZX >= 48) {
        issue2=0; // can be =1 as well
        MEMr[3]=RAM_BANK(0);
        MEMr[2]=RAM_BANK(2);
    }

    if( ZX >= 128) {
        issue2=0; // cant be 1
        page128=0;
        port_0x7ffd(page128);

        ZX_TS = 70908;
        ZX_FREQ = 3546894;
    }

    if( ZX_PENTAGON ) { // ZX == 128
        ZX_TS = 71680;
        ZX_FREQ = 3500000;
    }

    if( ZX >= 210 ) { // +2A/+3
        page2a=0;
        port_0x1ffd(page2a);
    }

    // floating bus [16,48,128,+2]
    //
    // [ref] https://sinclair.wiki.zxnet.co.uk/wiki/Floating_bus
    // [ref] https://github.com/jsmolina/z88dk-tutorial-sp1/blob/master/floating-bus.md
    // [ref] https://softspectrum48.weebly.com/notes/category/floating-bus
    //
    // note: for +2A/+3 models, it works this way:
    // While the effect is no longer present on unused ports, it is still evident when reading from ports
    // which match a particular addressing pattern, expressed as (1+n*4), or in binary, 0000 xxxx xxxx xx01.
    // This is only evident when the memory paging ports are unlocked, otherwise they will always return FF.
    // In detail:
    // 1. It is only found on ports that follow the pattern (1 + (4 * n) && n < 0x1000) (that is, ports 1, 5, 9, 13 . . . 4093).
    // 2. The bus always returns 0xFF if bit 5 of port 32765 is set (i.e. paging is disabled), so it won’t work in 48K mode.
    // 3. Otherwise, the value returned is the value currently read by the ULA ORed with 1 (i.e. Bit 0 is always set).
    // 4. During idling intervals (i.e. when the ULA is drawing the border or in between fetching the bitmap/attribute byte), the bus latches onto the last value that was written to, or read from, contended memory, and not strictly 0xFF. This is crucial to keep in mind.

    // @fixme: for floatspy.tap to be stable
    // IM2 t_offs needs to be 25->22 (128K), 29->30 (48K)

    do_once
    for( int model = 48; model <= 128; model += 80 ) {

        if( model == 128 )
        memset(floating_bus = floating128, 0, sizeof(floating128));
        else
        memset(floating_bus = floating48, 0, sizeof(floating48));

        enum { TIMING_48  = 14340 }; // others: 14338 (faq), ramsoft: 14347(+9)
        enum { TIMING_128 = 14366 }; // others: 14364 (faq), ramsoft: 14368(+4)
        int timing = model < 128 ? TIMING_48 : TIMING_128, inc = model < 128 ? 96 : 100;
        int cycles = model < 128 ? 69888 : 70908;
        int adjust = model < 128 ? 0 : 0; // adjust

        for( int k = timing + adjust, y = 0; y < 192; ++y, k += inc ) {
            int SCANLINE = ((((((y)%64) & 0x38) >> 3 | (((y)%64) & 0x07) << 3) + ((y)/64) * 64) << 5);
            int pixel = 0x4000 + SCANLINE, attr = 0x5800 + 32*(y/8);

            for( int x = 0; x < 16; ++x ) { //16*8=128 T states per line (main screen)
                floating_bus[k++]=pixel++; // = 1;
                floating_bus[k++]=attr++;  // = 2;
                floating_bus[k++]=pixel++; // = 3;
                floating_bus[k++]=attr++;  // = 4;
                floating_bus[k++]=0x0000;  // = 5;
                floating_bus[k++]=0x0000;  // = 6;
                floating_bus[k++]=0x0000;  // = 7;
                floating_bus[k++]=0x0000;  // = 8;
            }
        }
    }

    floating_bus = ZX <= 48 ? floating48 : ZX <= 200 ? floating128 : NULL;
    if( ZX_PENTAGON ) floating_bus = NULL;
}



// ay
void port_0xfffd(byte value) {
    // select ay register
    ay_current_reg=(value&15);
    // floooh's
    ay38910_iorq(&ay, AY38910_BDIR|AY38910_BC1|ay_current_reg<<16);
}
void port_0xbffd(byte value) {
    // update ay register
    ay_registers[ay_current_reg]=value;
    // floooh's
    ay38910_iorq(&ay, AY38910_BDIR|value<<16);
    // ayumi
    int *r = ay_registers;
    switch (ay_current_reg)
    {
    case 0:
    case 1:
        ayumi_set_tone(&ayumi, 0, (r[1] << 8) | r[0]);
        break;
    case 2:
    case 3:
        ayumi_set_tone(&ayumi, 1, (r[3] << 8) | r[2]);
        break;
    case 4:
    case 5:
        ayumi_set_tone(&ayumi, 2, (r[5] << 8) | r[4]);
        break;
    case 6:
        ayumi_set_noise(&ayumi, r[6]);
        break;
    case 8:
        ayumi_set_mixer(&ayumi, 0, r[7] & 1, (r[7] >> 3) & 1, r[8] >> 4);
        ayumi_set_volume(&ayumi, 0, r[8] & 0xf);
        break;
    case 9:
        ayumi_set_mixer(&ayumi, 1, (r[7] >> 1) & 1, (r[7] >> 4) & 1, r[9] >> 4);
        ayumi_set_volume(&ayumi, 1, r[9] & 0xf);
        break;
    case 10:
        ayumi_set_mixer(&ayumi, 2, (r[7] >> 2) & 1, (r[7] >> 5) & 1, r[10] >> 4);
        ayumi_set_volume(&ayumi, 2, r[10] & 0xf);
        break;
    case 7:
        ayumi_set_mixer(&ayumi, 0, r[7] & 1, (r[7] >> 3) & 1, r[8] >> 4);
        ayumi_set_mixer(&ayumi, 1, (r[7] >> 1) & 1, (r[7] >> 4) & 1, r[9] >> 4);
        ayumi_set_mixer(&ayumi, 2, (r[7] >> 2) & 1, (r[7] >> 5) & 1, r[10] >> 4);
        break;
    case 11:
    case 12:
        ayumi_set_envelope(&ayumi, (r[12] << 8) | r[11]);
        break;
    case 13:
        if (r[13] != 255) //< 255 to continue current envelope
        ayumi_set_envelope_shape(&ayumi, r[13]);
        break;
    }
}
byte inport_0xfffd(void) {
//  return ay38910_iorq(&ay, 0);
    unsigned char ay_registers_mask[ 16 ] = {
        0xff, 0x0f, 0xff, 0x0f, 0xff, 0x0f, 0x1f, 0xff,
        0x1f, 0x1f, 0x1f, 0xff, 0xff, 0x0f, 0xff, 0xff,
    };

    if( ZX_GUNSTICK ) // magnum lightgun
    if( ay_current_reg == 14 ) {

        // Magnum Light Phaser / Defender Light Gun for Spectrum >= 128
        // 1101 1111 bit5: trigger button (0=Pressed, 1=Released)
        // 1110 1111 bit4: light sensor   (0=None, 1=Light)

        byte gunstick(byte);
        struct mouse m = mouse();
        byte v = m.lb ? 0xDF : 0xEF; // set button
        v |= 0x10 * (gunstick(0xFF) < 0xFE); // set light

        ay_registers[14] = v;

        //printf("\t\t\tmagnum (%02x vs %02x)\n", gunstick(0xFF), v);
        return v;
    }

    /* The AY I/O ports return input directly from the port when in
    input mode; but in output mode, they return an AND between the
    register value and the port input. So, allow for this when
    reading R14... */

    if( ay_current_reg == 14 )
    return (ay_registers[7] & 0x40 ? 0xbf & ay_registers[14] : 0xbf);

    /* R15 is simpler to do, as the 8912 lacks the second I/O port, and
    the input-mode input is always 0xff */

    if( ay_current_reg == 15 && !( ay_registers[7] & 0x80 ) ) return 0xff;

    /* Otherwise return register value, appropriately masked */
    return ay_registers[ay_current_reg] & ay_registers_mask[ay_current_reg];
}

uint64_t tick1(int num_ticks, uint64_t pins, void* user_data) {

#if DEV
    if( cpu.step == 0 ) {
        unsigned pc = Z80_GET_ADDR(pins);

#if PRINTER
        // traps rom print routine. useful for automation tests

        // check no int pending, +2a/+3, or rom1
        // if( !IFF1(cpu) && ZX <= 200 && !(page128 & 16) )

        if (pc == 0x09F4 /*&& SP(cpu) < 0x4000*/) {
            uint8_t channel_k = READ8( IY(cpu) + 0x30 ) & 16; // FLAGS2
            uint8_t is_lower_screen = READ8( IY(cpu) + 2 ) & 1; // TV_FLAG
            if (!channel_k /*&& !is_lower_screen*/ ) {
                char ch = cpu.a; // & 0x7f;
                // if(ch >= 0x7F ) printf(printer, "%c", "© ▝▘▀▗▐▚▜▖▞▌▛▄▟▙█"[ch - 0x7f]);
                // else
                if(ch == 0x0D || ch >= 32) {
                    fprintf(printer, "%c", ch < 32 ? '\n' : ch);
                    fflush(printer);
                }
            }
        }
#endif

        // disasm
        int do_disasm = pc >= 0x38 && pc < 0x45; // mouse().rb;
        if( 0 )
        if( do_disasm ) puts(dis(pc, 1));
    }
#endif

    // mem
    if( pins & Z80_MREQ ) {
        if( pins & Z80_RD ) {
            uint16_t addr = Z80_GET_ADDR(pins);

            // Install shadow TRDOS ROM when the PC is in ROM1 + [0x3D00..0x3DFF] range
            // Uninstall it as soon as PC is out of ROM space
            if( ZX_PENTAGON/*beta128*/ ) {
                unsigned pc = PC(cpu);
                if( ((pc & 0xFF00) == 0x3D00) && (MEMr[0] == (rom + 0x4000 * 1)) ) {
                    MEMr[0] = rom + 0x4000 * 2; // install shadow rom
                }
                else
                if( pc >= 0x4000 && (MEMr[0] == (rom + 0x4000 * 2)) ) {
                    MEMr[0] = rom + 0x4000 * (page128 & 16 ? 1 : 0); // * 1; restore rom1
                }
            }

            uint8_t value = READ8(addr);
            Z80_SET_DATA(pins, value);
        }
        else if( pins & Z80_WR ) {
            uint16_t addr = Z80_GET_ADDR(pins);
            uint8_t value = Z80_GET_DATA(pins);
            WRITE8(addr, value);
        }
    }
    // ports
    else if( pins & Z80_IORQ ) {

#if NEWCORE
        if (pins & Z80_M1) {
            // put an interrupt vector on the data bus
            uint16_t intvec = (I(cpu)<<8) | 0xFF;
            Z80_SET_DATA(pins, intvec);
        }
        else
#endif

        if( pins & Z80_RD ) {
            uint16_t addr = Z80_GET_ADDR(pins);
            uint8_t value = inport(addr);
            Z80_SET_DATA(pins, value);
        }
        else if( pins & Z80_WR ) {
            uint16_t addr = Z80_GET_ADDR(pins);
            uint8_t value = Z80_GET_DATA(pins);
            outport(addr, value);
        }
    }

#if NEWCORE
    sys_audio();
    return pins;
#endif

    for(int i = 0; i < num_ticks; ++i)
    sys_audio();

    // vsync
    return pins | (zx_int ? zx_int = 0, Z80_INT : 0);
}


void frame_new() {
    // logic that ticks every new frame
    // this section has x50 faster rate than next section.
    if( 1 ) {
        // vsync (once per frame)
        zx_int = 1;
        //border_num = 0;
    }
    if( tape_inserted() ) {
        // auto-plays tape
        if( autoplay && !tape_playing() ) {
            if( ZX_AUTOPLAY ) {
                printf("auto-play tape (%u Hz, %u reads)\n", tape_hz, autoplay_numreads),
                tape_play(1);
            }
            else
            if( PC(cpu) < 0x4000 && voc_pos == 0 ) { // if( (PC(cpu) & 0xff00) == 0x0500 && GET_MAPPED_ROMBANK() == GET_BASIC_ROMBANK() ) {
                // kick off the initial tape playing automatically.
                // user will need to pause/resume tape manually (via F2 key) from this point, as AUTOPLAY=off
                // if( 1 ) { // tape_inserted() && !tape_playing() ) {
                if( !tape_playing() ) {
                    puts("auto-play tape (rom)"),
                    tape_play(1);
                }
            }
        }

        autoplay = 0;
        autostop = 0;
    }
    // logic that ticks once per second
    static int frame = 0; ++frame;
    if( !(frame % 50) ) {
        // auto-stops tape. stopping a tape is a risky action and load will fail if not done at the right moment.
        // so, the thinking for this block has a x50 slower rate than playing a tape. we have to be sure we dont
        // want tape to be playing instead.
        if( autostop && tape_playing() ) {
            if( ZX_AUTOSTOP ) {
                printf("auto-stop tape (%u Hz, %u reads)\n", tape_hz, autoplay_numreads),
                tape_stop();
            }
        }

        autoplay = 0;
        autostop = 0;
    }
}

void run(unsigned TS) {
    // if(TS<0)return;

#if NEWCORE

    for( int tick = 0; tick < TS; ++tick) {

        if (zx_int /*  && cpu.step == 2 && IFF1(cpu) */ ) { // adjust
            zx_int = 0;

            // request vblank interrupt
            z80_interrupt(&cpu, 1);

            // hold the INT pin for 32 ticks
            int_counter = 32; // - (4 - (tick & 3)); // 23?
        }
        
        // clear INT pin after 32 ticks
        if (int_counter > 0) {
            if(!--int_counter) z80_interrupt(&cpu, 0);
        }

        pins = z80_tick(&cpu, pins);
        ++ticks;
        tape_ticks += !!mic_on;

        pins = tick1(1,pins,0);
    }
#else
    z80_exec(&cpu, TS);
    ticks += TS;
    tape_ticks += !!mic_on * TS;
#endif

    fdc_tick(TS);
}

void sys_audio() {
    // tick the beeper (full frequency)
    bool sample_ready = beeper_tick(&buzz);
    // queue next sample
    if( sample_ready ) {
#if 1
    // mic/ear speaker
    // ref: https://piters.tripod.com/cassport.htm
    // ref: https://retrocomputing.stackexchange.com/a/27539
    // The threshold voltage for reading the input state (at the pin) of the ULA is 0.7V (*)
    // BIT4 EAR     BIT3 MIC     PIN 28 VOLTAGE  BIT6 IN READ
    //        0            0               0.34             0
    //        0            1               0.66             0 (borderline (*))
    //        1            0               3.56             1
    //        1            1               3.70             1
    //

    // test: ensure that: SAVE "P" : BEEP 1,1 : LOAD "" shall output sound
    // test: Cobra's Arc has menu music and in-game speech
    // test: Parapshock has menu music

    const float beeper_volumes[] = {
        TAPE_VOLUME, // rest volume
        TAPE_VOLUME, // tape volume (??%) see diagram above (*)
        0.96f, // beeper volume (96%)
        1.00f  // tape+beeper volume (100%)
    };

    int save_audio = spk & 0x8;
    int load_audio = ear;
    int tape = !!load_audio ^ !!save_audio;

    int beeper = !!(spk & 0x10);
    int combined = !!beeper * 2 + !!tape;

    beeper_set_volume(&buzz, beeper_volumes[combined]);
    beeper_set(&buzz, beeper | tape );
#endif
    }

    // tick the AY (half frequency)
    static float ay_sample1 = 0, ay_sample2 = 0; enum { ayumi_fast = 0 };
    static byte even = 255; ++even;
    if( ZX_AY & 1 ) if(!(even & 0x7F)) ay_sample1 = ayumi_render(&ayumi, ayumi_fast, 1) * 2; // 2/256 freq. even == 0 || even == 0x80
    if( ZX_AY & 2 ) if( even & 1 ) ay38910_tick(&ay), ay_sample2 = ay.sample; // half frequency

    if( ZX_AY == 0 ) ay_sample1 = ay_sample2 = 0; // no ay
    if( ZX_AY == 1 ) ay_sample2 = ay_sample1;     // ayumi only
    if( ZX_AY == 2 ) ay_sample1 = ay_sample2;     // floooh's only
    float ay_sample = (ay_sample1 + ay_sample2) * 0.5f; // both

    if( do_audio && sample_ready ) {
        float master = 0.98f * !!ZX_AY; // @todo: expose ZX_AY_VOLUME / ZX_BEEPER_VOLUME instead
        float sample = (buzz.sample * 0.75f + ay_sample * 0.25f) * master;

        float digital = mix(0.5); // 70908/44100. 69888/44100. // ZX_TS/s); 22050 11025 44100
        sample += digital * 1; // increase volume

        audio_queue(sample);
    }
}


void logport(word port, byte value, int is_out) {
    return;

    // [ref] https://worldofspectrum.org/faq/reference/ports.htm
    // sorted by xxNN byte, then NNxx byte
    if( 0
        || port == 0xfefe // keyboard
        || port == 0xfdfe // keyboard
        || port == 0xfbfe // keyboard
        || port == 0xf7fe // keyboard, sinclair2
        || port == 0xeffe // keyboard, sinclair1
        || port == 0xdffe // keyboard
        || port == 0xbffe // keyboard
        || port == 0x7ffe // keyboard

        || port == 0x00fe // ear/mic border/beeper

        || port == 0xfffd // ay
        || port == 0xbffd // ay
        || port == 0x7ffd // mem128
        || port == 0x3ffd // fdc
        || port == 0x2ffd // fdc
        || port == 0x1ffd // mem+2a/fdcmotor
        || port == 0x0ffd // parallel

        || port == 0xffdf // kempston mouse
        || port == 0xfbdf // kempston mouse
        || port == 0xfadf // kempston mouse

        || port == 0x00df // kempston joystick

        ||(port == 0x007f && !ZX_PENTAGON/*!beta128*/ ) // fuller joystick
        ||(port == 0x005f && !ZX_PENTAGON/*!beta128*/ ) // fuller ay
        ||(port == 0x003f && !ZX_PENTAGON/*!beta128*/ ) // fuller ay

        || port == 0xff3b // ulaplus
        || port == 0xbf3b // ulaplus
    )
        return;

    if( (port & 0xff) == 0xfe ) return; // ula

    unsigned pc = PC(cpu); // Z80_GET_ADDR(pins);
    if( is_out )
    printf("%04x OUT %4X, %X\n", pc, port, value);
    else
    printf("%04x IN  %4X (%X)\n", pc, port, value);
}


void outport(word port, byte value) {
#if NDEBUG <= 0
    logport(port, value, 1);
#endif

    // XXXXXXXX ---1 1111 when trdos is enabled. ports: 1F,3F,5F,7F,FF
    if( (port & 3) == 3 ) { // (port & 0xff1f) == 0x1f) { // 
        int trdos = ZX_PENTAGON/*beta128*/ && MEMr[0] == (rom + 0x4000 * 2);
        if( trdos ) {
            // command(0)/track(1)/sector(2)/data(3)/system(4) WD write registers

            int index;
#if 0
            // convert 1F,3F,5F,7F,FF ports into 0,1,2,3,4 index
            index = ((port & 0x80 ? 0xA0-1 : port) >> 5) & 7;
#else
            if(port & 0x80) index = WD1793_SYSTEM;
            else if( ( port & 0x60 ) == 0x60 ) index = WD1793_DATA;
            else if( ( port & 0x60 ) == 0x40 ) index = WD1793_SECTOR;
            else if( ( port & 0x60 ) == 0x20 ) index = WD1793_TRACK;
            else if( ( port & 0x60 ) == 0x00 ) index = WD1793_COMMAND;
#endif

            if( !index ) {
                int drive_state = ((value >> 5) & 7) < 4; // on, off
            }
            // Commands Register for index #0 (0x1F)
            // 0 0 0 0  h v x x  Init 0X
            // 0 0 0 1  h v x x  Seek 1X
            // 0 0 1 i  h v x x  Step 2X 3X
            // 0 1 0 i  h v x x  Step forward 4X 5X
            // 0 1 1 i  h v x x  Step back 6X 7X
            // 1 0 0 m  s e c 0  Read sectors 8X
            // 1 0 1 m  s e c a  Write sectors AX BX
            // 1 1 0 0  0 e 0 0  Read address CX
            // 1 1 0 1 J3J2J1J0  Forced Interrupt DX
            // 1 1 1 0  0 e 0 0  Read track EX
            // 1 1 1 1  0 e 0 0  Format track FX
            Write1793(&wd, index, value);
            return;
        }
    }


#if 0
    if(contended_mask & 4) //if +2a or +3 then apply (fixes fairlight games)
#else
    // +2a/+3
    if( ZX >= 210 )
#endif
    {
        //if (port & (0xFFFF^0x1FFD)) { port_0x1ffd(value); return; }
        //if (port & (0xFFFF^0x3FFD)) { port_0x3ffd(value); return; }

      //if((port & 0xf002) == 0x0000) { port_0x0ffd(value); return; } // 0000xxxx xxxxxx0x   parallel port
        if((port & 0xf002) == 0x1000) { port_0x1ffd(value); return; } // 0001xxxx xxxxxx0x   mem pagination
      //if((port & 0xf002) == 0x2000) { port_0x2ffd(value); return; } // 0010xxxx xxxxxx0x   fdc status
        if((port & 0xf002) == 0x3000) { port_0x3ffd(value); return; } // 0011xxxx xxxxxx0x   fdc data
        if((port & 0xc002) == 0xa000) { port_0x7ffd(value); return; } // 01xxxxxx xxxxxx0x   mem pagination
        if((port & 0xc002) == 0xb000) { port_0xbffd(value); return; } // 10xxxxxx xxxxxx0x   ay data
        if((port & 0xc002) == 0xc000) { port_0xfffd(value); return; } // 11xxxxxx xxxxxx0x   ay data register
    }

    // sorted ports
    if (ZX >= 128) if(!(port & (0xFFFF^0x7FFD))) { port_0x7ffd(value); return; }


    // ula+ register port
    if( ZX_ULAPLUS )
    if (!(port & (0xFFFF^0xBF3B))) {
        ulaplus_data = value & 0x3f;
        ulaplus_mode = value >> 6;
    }

    // ay
    if( ZX >= 128 )
    if (!(port & (0xFFFF^0xBFFD)))     { port_0xbffd(value); return; } // ay

    // ula+ data port
    if( ZX_ULAPLUS )
    if (!(port & (0xFFFF^0xFF3B))) {
        if( ulaplus_mode == 0 ) {
            ulaplus_registers[ulaplus_data] = value;
            unsigned b = (value & 0x03) << 6; b |= !!b << 5;
            unsigned r = (value & 0x1c) << 3;
            unsigned g = (value & 0xe0);
            ZXPalette[ulaplus_data] = ulaplus_grayscale ? rgb(value,value,value) : rgb(r,g,b);
        }
        if( ulaplus_mode != 0 ) {
            ulaplus_registers[64] = value;
            ulaplus_enabled = value & 1;
            ulaplus_grayscale = !!(value & 2);
        }
    }

    // ay
    if( ZX >= 128 )
    if (!(port & (0xFFFF^0xFFFD)))     { port_0xfffd(value); return; } // ay

    // fuller audio box emulation
    if (!(port & (0xFF^0x3F)))         { port_0xfffd(value); return; } // ay
    if (!(port & (0xFF^0x5F)))         { port_0xbffd(value); return; } // ay

    // default
    if( !(port & (0xFF^0xFE))) {
        port_0x00fe(value);
        return;
    }
}
byte gunstick(byte code) { // out: FF(no), FE(fire), FB(lit)
    // @fixme, lmb should last at least 2 frames (see Guillermo Tell)
    enum { h_border = _32 };
    enum { v_border = _24 };

    struct mouse m = mouse();

    // fire
    if (m.lb || m.rb) {
        code &= 0xFE;
        goto trig;
    }

        // estimate X,Y position based on TS and check against real mouse pos
        int scanline = (ticks % 70908) / 311;
        if( scanline <= 64 || scanline > (64+192) ) return 0xFF;
        int raster = (ticks % 70908) % 228;
        if( raster > 128 ) return 0xFF;
#if 0
        m.x -= h_border; // 0..256
        m.y -= v_border; // 0..191
        raster *= 2; // 0..256
        scanline -= 64; // 0..191
#endif
        if( ((m.x-raster)*(m.x-raster)) < 64 )
            if( ((m.y-scanline)*(m.y-scanline)) < 64 )
                code &= 0xFB;

        return code;

        // debug
        printf("gun(%3d,%3d) ask(%3d,%3d) %02x\n", m.x,m.y, raster,scanline, code);

        // debug
        if( 1 )
        {
            raster *= 2; // 0..256
            raster += _32; // 32..288

            scanline -= 64; // 0..191
            scanline += _24; // 24..215

            int x = 0; // raster > _319 ? _319 : raster;
            int y = scanline > _239 ?  _239 : scanline;

            extern window *app;
            int width = _32+256+_32;
            rgba *texture = &((rgba*)app->pix)[0];
            texture[ x + y * width ] = rgb(255,0,255);
        }
        return code;

trig:;

    // light
    // check whether attribute is white
    if (m.x > h_border && (m.x - h_border < 256))
    if (m.y > v_border && (m.y - v_border < 192)) {
        int x = (m.x - h_border) / 8;
        int y = (m.y - v_border) / 8;

        int cell = x;
        int line = m.y - v_border;
        int offset = ((line & 7) << 8) + ((line & 0x38) << 2) + ((line & 0xC0) << 5) + cell;
        byte *pixel = VRAM + offset;
        byte *attr = VRAM + 6144 + 32 * y + x;
        int ink = *attr & 7;
        int paper = (*attr >> 3) & 7;
        int bright = *attr & 0x40;

        #if 0 // find bright luma pixels (green 4, cyan 5, yellow 6, white 7)
        if( *pixel ? ink >= 4 : paper >= 4 ) code &= 0xFB;
        #else // find white attribs
        if (ink == 0x07 || paper == 0x07) code &= 0xFB;
        #endif

        #if 0 // debug
        printf("gun(%d,%d,%d) attr(%d,%d,%02x) val:%02x\n", m.x,m.y,m.lb, x,y,(byte)*attr, code);
        *pixel = 0xFF, *attr = 0x38;
        #endif
    }
    return code;
}


byte inport_(word port) {
    // if(port != 0x1ffd && port != 0x2ffd && port != 0x3ffd && port != 0x7ffe) { puts(regs("inport")); printf(" in port[%04x]\n", port); }
    //[0102] alien syndrome.dsk ??
    //[0002] alien syndrome.dsk ??
    //[fefe] alien syndrome.dsk


    // XXXXXXXX ---1 1111 when trdos is enabled. ports: 1F,3F,5F,7F,FF
    if( (port & 3) == 3 ) { //  (port & 0xff1f) == 0x1f) { // 
        int trdos = ZX_PENTAGON/*beta128*/ && MEMr[0] == (rom + 0x4000 * 2);
        if( trdos ) {
            // status(0)/track(1)/sector(2)/data(3)/ready(4) WD read registers

            int index;
#if 0
            // convert 1F,3F,5F,7F,FF ports into 0,1,2,3,4 index
            index = ((port & 0x80 ? 0xA0-1 : port) >> 5) & 7;
#else
            if(port & 0x80) index = WD1793_SYSTEM;
            else if( ( port & 0x60 ) == 0x60 ) index = WD1793_DATA;
            else if( ( port & 0x60 ) == 0x40 ) index = WD1793_SECTOR;
            else if( ( port & 0x60 ) == 0x20 ) index = WD1793_TRACK;
            else if( ( port & 0x60 ) == 0x00 ) index = WD1793_STATUS;
#endif

            // read from reg 0xFF(4) system 
            // D0, D1   - diskdrive select. 00 for drive A, 01 for drive B 10 for drive C, 11 for drive D
            // D2       - hardware microcontroller reset. by resetting and then setting this bit again, we can form impulse of microcontroller reset. usually this reset happens in very begin of TR-DOS session. 
            // D3       - this digit blocks signal HLT of microcontroller. For normal work must contain '1'. 
            // D4       - Diskdrive head select. contents of this digit translates directly to diskdrive. 0 means first head or 'bottom' side of disk, 1 - second head/'top' side of disk.
            // D5       - Density select. reset of this digit makes microcontroller works in FM mode, seted digit - MFM.
            // D6 DRQ   - signal showing request of data by microcontroller 
            // D7 INTRQ - signal of completion of execution of command.
            return Read1793(&wd, index);
        }
    }


    // +2a/+3
    if( ZX >= 210 )
    {
        if((port & 0xf002) == 0x2000) { return inport_0x2ffd(); } // 0010xxxx xxxxxx0x   fdc status
    }

    if( ZX_MOUSE )
    {
        // kempston mouse detection
        if(port == 0xFADF) kempston_mouse |= 1;
        if(port == 0xFBDF) kempston_mouse |= 2;
        if(port == 0xFFDF) kempston_mouse |= 4;

        // kempston mouse
        if( kempston_mouse == 7 ) {
            struct mouse m = mouse();
            if(!(port & (0xFFFF^0xFADF))) return mouse_clip(1), mouse_cursor(0), 0xFF&~(1*m.rb+2*m.lb+4*m.mb); // ----.--10.--0-.----  =  Buttons: D0 = RMB, D1 = LMB, D2 = MMB
            if(!(port & (0xFFFF^0xFBDF))) return mouse_clip(1), mouse_cursor(0),  m.x;                         // ----.-011.--0-.----  =  X-axis:  $00 … $FF
            if(!(port & (0xFFFF^0xFFDF))) return mouse_clip(1), mouse_cursor(0), -m.y;                         // ----.-111.--0-.----  =  Y-axis:  $00 … $FF
        }
    }

    // kempston port (31) @fixme: also seen mappings on port 55 and 95 (?)
    if( kempston_mouse != 7 ) {
        // gunstick (bestial warrior)
        if(ZX_GUNSTICK)
        if(!(port & (0xFF^0xDF))) return /*puts("gunstick kempston"),*/ gunstick(0xFF);

        // kempston joystick
        if(!(port & (0xFF^0xDF))) return kempston; //bit 5 low = reading kempston (as told pera putnik)
        //if(!(port & 0xE0)) return kempston;          //bit 7,6,5 low = reading kempston (as floooh)
    }

    // +2a/+3
    if( ZX >= 210 ) {
    if(!(port & (0xFFFF^0x2FFD))) return inport_0x2ffd();
    if(!(port & (0xFFFF^0x3FFD))) return inport_0x3ffd();
    }

    // ulaplus read data
    if( ZX_ULAPLUS )
    if (!(port & (0xFFFF^0xFF3B))) {
        if( ulaplus_mode == 0 ) {
            return ulaplus_registers[ulaplus_data];
        }
        if( ulaplus_mode != 0 ) {
            return ulaplus_registers[64];
        }
    }

    // gunstick 0x7ffe,0xbffe,0xdffe,0xeffe,0xf7fe,0xfefe,0xfbfe,0xfdfe
    // @fixme: add SINCLAIR2 port too
    if( ZX_GUNSTICK )
    if (!(port & (0xFFFF^0xEFFE))) {
        int code = 0xFF;

        // prevent emulator input being passed to zx
        extern int active;
        if(!active) {
        if (!(port & 0x0100)) code &= keymap[4][1];
        if (!(port & 0x0200)) code &= keymap[3][1];
        if (!(port & 0x0400)) code &= keymap[2][1];
        if (!(port & 0x0800)) code &= keymap[1][1];
        if (!(port & 0x1000)) code &= keymap[1][2];
        if (!(port & 0x2000)) code &= keymap[2][2];
        if (!(port & 0x4000)) code &= keymap[3][2];
        if (!(port & 0x8000)) code &= keymap[4][2];
        }

        return /*puts("gunstick sinclair"),*/ gunstick(code);
    }

    // ay
    if( ZX >= 128)
    if(!(port & (0xFFFF^0xFFFD))) return inport_0xfffd();

    // fuller joystick
    if(!(port & (0xFF^0x7F))) return fuller;

    // keyboard
    if(!(port & (0xFF^0xFE))) {
        byte code = 0xFF;

        // prevent emulator input being passed to zx
        extern int active;
        if(!active) {

        // test: Without this matrix behaviour Zynaps, for instance, 
        // won't pause when you press 5,6,7,8 and 0 simultaneously. @fixme
        if (!(port & 0x0100)) code &= keymap[4][1];
        if (!(port & 0x0200)) code &= keymap[3][1];
        if (!(port & 0x0400)) code &= keymap[2][1];
        if (!(port & 0x0800)) code &= keymap[1][1];
        if (!(port & 0x1000)) code &= keymap[1][2];
        if (!(port & 0x2000)) code &= keymap[2][2];
        if (!(port & 0x4000)) code &= keymap[3][2];
        if (!(port & 0x8000)) code &= keymap[4][2];

        }

        // issue2/3 handling
        // issue2 keyboard emulation (16K/48K only). bits 765 set. 560,000 48k sold models.
        // issue3 keyboard emulation (>=128 models). bits 7 5 set, 6 reset. 3,000,000 48k sold models.
        // On an Issue 3, an OUT 254 with bit 4 reset will give a reset bit 6 from IN 254.
        // On an Issue 2, both bits 3 and 4 must be reset for the same effect to occur.
        // test from css faq:
        // 10 OUT 254,BIN 11101111
        // 20 PRINT IN 254
        // 30 OUT 254,BIN 11111111
        // 40 PRINT IN 254
        // 50 GOTO 10
        // For a correct test do not press any key while running, and have no EAR input.
        // If the output is 191,255,191,255 etc, you are on real Spectrum Issue 3.
        // If output is always 191 or always 255, change the value in line 10 to BIN 11100111.
        // If output is then 191,255,191,255 etc, then you are on Spectrum Issue 2.
        // If output is still always 191 or always 255 you are on Spectrum emulator.

        if( (spk & ( issue2 ? 0x18 : 0x10 )) == 0 )
            code &= ~(1 << 6);

        // Bit 6 of IN-Port 0xfe is the EAR input bit
        // Note: Status of MIC line (block below), can alter the behavior of the last code assignment.
        // See: AbuSimbelProfanation(Gremlin) + polarity of ending TZX pause/stop blocks.

        // @todo: check this comment
        // Patrik Rak "In case there is no tape input, bit 6 depends on what was output
        // to bits 3 and 4. Issue 2 maps 00 to 0, issue 3 both 00 and 01 to 0.
        // We stick to issue 2 behavior as it is more compatible of the two (when it matters).
        // The +2A/+3 models however always return 0, so we honor that."

        if( 1 ) {
            ear = mic_read(tape_ticks);
            code = code ^ ear;
        }

        // issue2/3 keyboard tests:
        // - LOAD"" shall work for all issue2/3 models.
        // - keys shall work in BASIC for all issue2/3 models.
        // - AbuSimbelProfanation(Gremlin): issue2 can play the game
        // - Rasputin48k: issue3 can play the game. issue2 sounds like game crashed.
        // issue2/3 polarity tests:
        // - Lonewolf3-Side128 (+) shall load only in 128 models (issue3)
        // - Lonewolf3-Side48 (+) shall load only in 48 models (issue3 only)
        // - ForbiddenPlanet (-), V1 (-) and V2 (-) (issue2 only)
        // - Wizball.tap (+) (from PZXtools) shall load in 48/128 models (issue3)
        // - Basil Mouse Detective (+) and MASK (+) shall load on 48/128 issue3 models only
        // - Aforementioned games can still load if you toggle tape polarity; which is similar to switching issue2/3 behavior

        // now that we have read tape, let's update autoplay/autostop heuristics

        {
            // auto-tape tests [* issue]
            // [x] rom: pilot loader, acid killer.tzx
            // [x] full suite: gauntlet, *p-47 thunderbolt, munsters 48k, myth,
            // [x] glued: batman caped crusader
            // [x] ay stopper: madmix2 (symb), mortadelo y filemon 2 (spc), wizard warz,
            // [x] press-any-key: the egg, cauldron2.tap
            // [x] loading pauses: diver, doctum, coliseum.tap, barbarian(melbourne), indiana jones last crusade
            // [ ] *viaje al centro de la tierra, tuareg, desperado
            // [ ] *cauldron (silverbird),
            // [ ] *turrican, turrican2, un squadron,
            // [x] tusker, predator,
            // [x] last ninja 2, karnov,
            // [x] gryzor 48k, rainbow islands 48k
            // [?] twilight, wild streets, 
            // [?] time scanner, tmnt coin op, super wonder boy, strider 2, street hassle,
            // [?] st dragon, spherical, real ghostbusters, spherical, perico delgado,
            // [?] pang, outrun, laser squad, koronis rift,
            // [?] gauntlet 2, line of fire, mandragore, the deep, 
            // [?] galaxy force, flash gordon, dragons lair, dragons lair 2, dragon breed,
            // [?] double dragons, desolator, final assault, forgotten worlds, crack down,
            // [?] alien syndrome, altered beast,

            // stable tape heuristics I used for a long time:
            // - hint play if loading from rom && mic has tape
            // newer heuristics that worked ok-ish:
            // - hint stop if strchr("uo",voc[voc_pos].debug) ; pa(u)se st(o)p
            // - hints based on IN FE port activity
            //   - 70908 ts/s / 8 ts/portread = 8863 portreads/s. theory limit of 8863 Hz.
            //     very high frequency polling are press-any-key pollings (>1700 Hz)
            //     high frequency polling are data or pilot loaders (~500..1200 Hz)
            //     low frequency polling are keyboard handlers (0..8 Hz)
            //   - hint stop if keyboard is being read (<300hz); else hint loader. myth will work; madmix2 (shift) wont.
            //   - hint stop if press-any-key tight loops (>1700hz); else hint loader. madmix2 will work; myth wont.
            // newer heuristics that I tried and didnt fully work:
            // - hint stop if num_written_ports(ay||spk) is significant, then we are playing; else hint loader. madmix2 will work; gauntlet wont.
            // - hint stop if hash of AY regs[0..13] changes every second. 
            // - hint stop if black/blue borders; else hint loader. gauntlet will work; p47 wont.
            // - hint stop if IN FE from slow page; else hint loader. firelord wont work.
            // @todo: more hints I didnt try
            // - hint stop if reading mouse port (~playing)
            // - hint stop if reading kempston(s) ports (~playing)
            // - hint stop if reading keyboard matrix other than SPACE or BREAK (~playing)
            // - hint stop if long pause (>=5s) found after tape progress is >30% (see: myth)
            // - hint stop if long pause (>=5s) found between turbo blocks (see: myth or gauntlet)

            // what I'm using now:
            // inspect how the game processes the IN FE byte. if byte is RRA or checked against 0x20 then byte is a loading one.
            // count the amount of loading bytes within a second: hint stop if count == 0; hint play if count > 200.

            if( tape_inserted() )
            {
                unsigned pc = PC(cpu) - 2;
                byte *addr = ADDR8(pc);

                if( 0
                    || !memcmp(addr,  "\xDB\xFE\x1F", 3) // Common
                    || !memcmp(addr,  "\xDB\xFE\xE6\x40", 4) // Abadia del crimen
                    || !memcmp(addr,  "\xDB\xFE\xA9\xE6\x40", 5) // P-47, BloodBrothers
                    || !memcmp(addr,  "\xDB\xFE\xA0\xC2\x88\xFD", 6) // lonewolf3 v1
                    || !memcmp(addr,  "\xDB\xFE\xA0\xCA\x48\xFD", 6) // lonewolf3 v2
                    || !memcmp(addr,  "\xED\x78\xA8\xE6\x40", 5) // trivial pursuits' questions
                    || !memcmp(addr-1,"\x2C\xDB\xFE\xA4\xC2", 5) // gremlin2: Basil the Mouse Detective (10 Great Games 2),  Mask,  Mask (10 Great Games 2)
                    || !memcmp(addr-1,"\x2C\xDB\xFE\xA4\xCA", 5) // gremlin2: Basil the Mouse Detective (10 Great Games 2),  Mask,  Mask (10 Great Games 2)
                )
                autoplay_numreads++;

                // measure how large the slice of samples is. SLICE used to be 0.1, so it was a tenth (or slice) of a frame
                // it is 1 second now, so autoplay/autostop variables get updated once per second
                const float SLICE = 1.0; 

                ++autoplay_freq;

                if( (ticks - autoplay_last) > 69888*SLICE ) {
                    tape_hz = autoplay_freq/SLICE;
                    autoplay_last = ticks, autoplay_freq = 0;

                    // @fixme: wont stop tape or will burst CPU at max speed after load
                    // *viaje al centro de la tierra, *dynamite dux, *outrun europa (stop:228Hz)

                    if( autoplay_numreads <= 9 ) { // 9Hz turrican, 228Hz outrun europa
                        ++autostop;
                    }
                    if( autoplay_numreads > 200 ) { // 273Hz turrican
                        ++autoplay;
                    }

                    autoplay_numreads = 0;
                }
            }
        }

        return code;
    }

    // unattached port, read from floating bus. +2A/+3 returns 0xFF under most circumstances.
    // When the Z80 reads from an unattached port it will read the data present on the ULA bus, which will be a display byte being transferred to the video circuits or 0xFF when idle, such as periods spent building the border.
    // A number of commercial games used the floating bus effect, generally to synchronise with the display and allow for flicker-free drawing. Games known to use the floating bus include:
    // - Arkanoid (original release only, the Hit Squad re-release does not use the floating bus)
    // - Cobra (original release only, the Hit Squad re-release does not use the floating bus)
    // - Sidewize
    // - Short Circuit
    if ((port & 0xFF) == 0xFF) {
        if( !floating_bus ) return 0xFF;
        unsigned tstate = ticks % (ZX < 128 ? 69888 : 70908);
        unsigned value = floating_bus[tstate] ? READ8(floating_bus[tstate]) : 0xFF;
        // debug: value = floating_bus[tstate] ? floating_bus[tstate] : 0xFF ; 
        // debug: printf("%d %d\n", tstate, value); // adjust
        return value;
    } // 48: 56 << 8 + 3..6 (0) = 14339..45 (vs 14336) // 128: 56 << 8 + 30 (28) = 14366 (vs 14364)

    return 0xFF;
    // return port & 1 ? 0xFF : 0x00;
}

byte inport(word port) {
    byte v = inport_(port);
#if NDEBUG <= 0
    logport(port, v, 0);
#endif
    return v;
}

#include "zx_sav.h"

#define FULL_QUICKSAVES 1 // 0 breaks run-a-head

struct quicksave {
    // cpu
    z80_t cpu;
    uint64_t pins;
    int int_counter;

    // rom patch
    int rom_patches;
    // control flags
    int ZX_TS;
    int ZX_FREQ;
    int ZX;
    // audio
    //int audio_pos;
    //float audio_buffer[AUDIO_BUFFER];
    // memory
    byte *MEMr[4]; //solid block of 16*4 = 64kb for reading
    byte *MEMw[4]; //solid block of 16*4 = 64kb for writing
    int vram_contended;
    int vram_accesses;

#if FULL_QUICKSAVES
    unsigned short *floating_bus;

    byte dum[16384];    // dummy page
    byte rom[16384*4];  // +3
    byte mem[16384*16]; // scorpion
#endif

    // screen
    int  ZXFlashFlag;
    byte ZXBorderColor;
    rgba ZXPalette[64];
    // input
    int keymap[5][5];
    int issue2;
    // joysticks
    byte kempston,fuller;
    // mouse
    byte kempston_mouse;
    // ear
    byte ear;
    byte spk;
    // int
    byte zx_int;
    // ticks
    uint64_t ticks, TS;
    // plus3/2a
    byte page2a;
    // zx128
    byte page128;
    // audio
#if FULL_QUICKSAVES
    beeper_t buzz;
    struct ayumi ayumi;
    ay38910_t ay;
#endif
    // ay
    byte ay_current_reg;
    int ay_registers[ 16 ];
    // ula+
    byte ulaplus_mode;
    byte ulaplus_data;
    byte ulaplus_enabled;
    byte ulaplus_grayscale;
    byte ulaplus_registers[64+1];

    // @todo: pointers needed?
    //#include "dsk.c"
    FDC_DEFINES
#if FULL_QUICKSAVES
    WD1793 wd;
    FDIDisk fdd[NUM_FDI_DRIVES];
#endif

    VOC_DEFINES

    // tape
    int tape_hz;
    uint64_t tape_ticks;
    // autoplay
    int autoplay;
    int autostop;
    int autoplay_freq;
    uint64_t autoplay_last;
    unsigned autoplay_numreads;

} quicksaves[10+1] = {0}; // user[0..9], sys/run-ahead reserved[10]

enum { quicksave_len = sizeof(struct quicksave) };

#define EXPORT(f) (c->f = f)
#define IMPORT(f) (f = c->f)

void* quicksave(unsigned slot) {
    if( slot >= 11) return 0;

    struct quicksave *c = &quicksaves[slot];

    #define $(member) \
        printf("%x..%x %s;\n", (int)offsetof(struct quicksave, member), (int)offsetof(struct quicksave, member) + (int)(sizeof( ((struct quicksave *)0)->member )), #member);

    int rampages = 16; // ZX > 300 ? 16 : ZX > 48 ? 8 : ZX > 16 ? 3 : 1;

    // cpu
    c->cpu = cpu;
    c->pins = pins;
    c->int_counter = int_counter;
    // rom patch
    c->rom_patches = rom_patches;
    // control flags
    c->ZX_TS = ZX_TS;
    c->ZX_FREQ = ZX_FREQ;
    c->ZX = ZX;
    // audio
//  c->audio_pos = audio_pos;
//  float audio_buffer[AUDIO_BUFFER];
    // memory
    for( int i = 0; i < 4; ++i ) c->MEMr[i] = MEMr[i];
    for( int i = 0; i < 4; ++i ) c->MEMw[i] = MEMw[i];
    c->vram_contended = vram_contended;
    c->vram_accesses = vram_accesses;
#if FULL_QUICKSAVES
//  byte contended[70908];
//  byte floating_bus[70908];
    c->floating_bus = floating_bus;
//    memcpy(c->dum, dum, 16384);
//    memcpy(c->rom, rom, 16384*4);
    memcpy(c->mem, mem, 16384*rampages); //16);
#endif
    // screen
    c->ZXFlashFlag = ZXFlashFlag;
    c->ZXBorderColor = ZXBorderColor;
    memcpy(c->ZXPalette, ZXPalette, sizeof(rgba)*64);
    // input
    memcpy(c->keymap, keymap, sizeof(int)*5*5);
    c->issue2 = issue2;
    // joysticks
    c->kempston = kempston;
    c->fuller = fuller;
    // mouse
    c->kempston_mouse = kempston_mouse;
    // beeper
    c->ear = ear;
    c->spk = spk;
    // vsync
    c->zx_int = zx_int;
    // ticks
    c->ticks = ticks;
    c->TS = TS;
    // plus3/2a
    c->page2a = page2a;
    // zx128
    c->page128 = page128;
    // audio
#if FULL_QUICKSAVES
    c->buzz = buzz;
    c->ay = ay;
    c->ayumi = ayumi;
#endif
    // ay
    c->ay_current_reg = ay_current_reg;
    memcpy(c->ay_registers, ay_registers, sizeof(ay_registers));
    // ula+
    c->ulaplus_mode = ulaplus_mode;
    c->ulaplus_data = ulaplus_data;
    c->ulaplus_enabled = ulaplus_enabled;
    c->ulaplus_grayscale = ulaplus_grayscale;
    memcpy(c->ulaplus_registers, ulaplus_registers, sizeof(ulaplus_registers[0])*(64+1));
    // @todo
    //#include "dsk.c"
    FDC_EXPORT
#if FULL_QUICKSAVES
    c->wd = wd;
    memcpy(c->fdd, fdd, sizeof(fdd[0]) * 4); // sizeof(fdd));
#endif

    // @todo
    //#include "tap.c"
    VOC_EXPORT

    // tape
    c->tape_hz = tape_hz;
    c->tape_ticks = tape_ticks;
    // autoplay
    c->autoplay = autoplay;
    c->autostop = autostop;
    c->autoplay_freq = autoplay_freq;
    c->autoplay_last = autoplay_last;
    c->autoplay_numreads = autoplay_numreads;

    return c;
}
void* quickload(unsigned slot) {
    if( slot >= 11) return 0;

    struct quicksave *c = &quicksaves[slot];

    /*config*/(ZX = c->ZX);

    // control flags
    ZX_TS = c->ZX_TS;
    ZX_FREQ = c->ZX_FREQ;

    // zx128
    port_0x7ffd((page128 = c->page128) & ~32);
    // plus3/2a
    port_0x1ffd(page2a = c->page2a);

    // cpu
    cpu = c->cpu;
    pins = c->pins;
    // pins = z80_prefetch(&cpu, PC(cpu));
    int_counter = c->int_counter;

    // rom patch
    rom_patches = c->rom_patches;

    int rampages = 16; // ZX > 300 ? 16 : ZX > 48 ? 8 : ZX > 16 ? 3 : 1;

    // audio
//  audio_pos = c->audio_pos;
//  float audio_buffer[AUDIO_BUFFER];
//  memset(audio_buffer, 0, sizeof(audio_buffer[0])*AUDIO_BUFFER);
    // memory
    for( int i = 0; i < 4; ++i ) MEMr[i] = c->MEMr[i];
    for( int i = 0; i < 4; ++i ) MEMw[i] = c->MEMw[i];
    vram_contended = c->vram_contended;
    vram_accesses = c->vram_accesses;
#if FULL_QUICKSAVES
//  byte contended[70908];
    floating_bus = c->floating_bus;
//  memcpy(dum, c->dum, 16384);
//  memcpy(rom, c->rom, 16384*4);
    memcpy(mem, c->mem, 16384*rampages); //16);
#endif
    // screen
    ZXFlashFlag = c->ZXFlashFlag;
    ZXBorderColor = c->ZXBorderColor;
    memcpy(ZXPalette, c->ZXPalette, sizeof(rgba)*64);
    // input
    memcpy(keymap, c->keymap, sizeof(int)*5*5);
    issue2 = c->issue2;
    // joysticks
    kempston = c->kempston;
    fuller = c->fuller;
    // mouse
    kempston_mouse = c->kempston_mouse;
    // beeper
    ear = c->ear;
    spk = c->spk;
    // vsync
    zx_int = c->zx_int;
    // ticks
    ticks = c->ticks;
    TS = c->TS;
    // audio
#if FULL_QUICKSAVES
    buzz = c->buzz;
    ay = c->ay;
    ayumi = c->ayumi;
#endif
    // ay
    ay_current_reg = c->ay_current_reg;
    memcpy(ay_registers, c->ay_registers, sizeof(ay_registers));
    // ula+
    ulaplus_mode = c->ulaplus_mode;
    ulaplus_data = c->ulaplus_data;
    ulaplus_enabled = c->ulaplus_enabled;
    ulaplus_grayscale = c->ulaplus_grayscale;
    memcpy(ulaplus_registers, c->ulaplus_registers, sizeof(ulaplus_registers[0])*(64+1));
    // @todo
    //#include "dsk.c"
    FDC_IMPORT
#if FULL_QUICKSAVES
    wd = c->wd;
    memcpy(fdd, c->fdd, sizeof(fdd[0]) * 4); // sizeof(fdd));
#endif

    //#include "tap.c"
    VOC_IMPORT

    // tape
    tape_hz = c->tape_hz;
    tape_ticks = c->tape_ticks;
    // autoplay
    autoplay = c->autoplay;
    autostop = c->autostop;
    autoplay_freq = c->autoplay_freq;
    autoplay_last = c->autoplay_last;
    autoplay_numreads = c->autoplay_numreads;

    return c;
}



// https://worldofspectrum.org/forums/discussion/comment/539714
// warm reset: IR set to 0000h. All other registers keep their previous values.
// cold reset: IR set to 0000h. All other registers are set to FFFFh.

void z80_quickreset(int warm) {
    if( warm ) {

        #if 0
            // http://www.z80.info/interrup.htm
            IFF1(cpu) = IFF2(cpu) = IM(cpu) = 0;
            PC(cpu) = I(cpu) = R(cpu) = 0;
            SP(cpu) = AF(cpu) = 0xffff;
            return;
        #endif

            PC(cpu) = I(cpu) = R(cpu) = 0;

    } else {
        #if 0
            #if NEWCORE
                pins = z80_reset(&cpu);
            #else
                z80_reset(&cpu);
            #endif
            return;
        #endif

            PC(cpu) = I(cpu) = R(cpu) = 0;
            
            AF(cpu) = BC(cpu) = DE(cpu) = HL(cpu) =
            WZ(cpu) = SP(cpu) = IX(cpu) = IY(cpu) =
            AF2(cpu) = BC2(cpu) = DE2(cpu) = HL2(cpu) = 0xFFFF;
    }
}

void eject() {
    Reset1793(&wd,fdd,WD1793_EJECT);
    fdc_reset();
    tape_reset();
    media_reset();
    ZXDB = zxdb_free(ZXDB);
}

void reset(unsigned FLAGS) {
#if 1
    ZX_KLMODE_PATCH_NEEDED = !!ZX_KLMODE;
#else
    ZX_KLMODE = 0, ZX_KLMODE_PATCH_NEEDED = 0;
#endif

    if( ZX != 128 ) ZX_PENTAGON = 0;

    page2a = ZX < 210 ? 128 : 0; port_0x1ffd(page2a);
    page128 = ZX < 128 ? 32 : 0; port_0x7ffd(page128);

    z80_quickreset(1);

    #if NEWCORE
        pins = z80_prefetch(&cpu, PC(cpu));
    #endif
        ticks=0;

    /*page128 = 0;*/
    //if(ZX>=128) port_0x7ffd(page128 = 0); // 128
    //if(ZX>=210) port_0x1ffd(page2a = 0); // +2a/+3

    ear = 0;
    spk = 0;
    beeper_reset(&buzz);

    ay_reset();
    ayumi_reset(&ayumi);
    ay38910_reset(&ay);

    mixer_reset();

    ula_reset();

    mouse_clip(0);
    mouse_cursor(1);
    kempston_mouse = 0;

    tape_hz = 0;
    tape_ticks = 0;
    autoplay = 0;
    autostop = 0;

    tape_rewind();

    if( !(FLAGS & KEEP_MEDIA) ) {
        eject();
    } else {
        Reset1793(&wd,fdd,WD1793_KEEP);
    }

    border_num = 0;
    border_org = ticks;
    border[border_num] = 0;
}

void boot(int model, unsigned FLAGS) {
    if(model) ZX = model;

    dum = (char*)realloc(dum, 16384);    // dummy page
    rom = (char*)realloc(rom, 16384*4);  // +3
    mem = (char*)realloc(mem, 16384*16); // scorpion

    if( ZX != 128 ) ZX_PENTAGON = 0;

    config(ZX);

#if NEWCORE
    pins = z80_init(&cpu);
#else
    z80_desc_t desc = { tick1, NULL };
    z80_init(&cpu, &desc);
#endif

    beeper_desc_t bdesc = {0};
    bdesc.tick_hz = ZX_FREQ; // ZX_FREQ_48K
    bdesc.sound_hz = AUDIO_FREQUENCY;
    bdesc.base_volume = BUZZ_VOLUME;
    beeper_init(&buzz, &bdesc);

    // ay
    ay_reset();

    // floooh's
    ay38910_desc_t ay_desc = {0};
    ay_desc.type = AY38910_TYPE_8912;
    ay_desc.tick_hz = ZX_FREQ / 2;
    ay_desc.sound_hz = AUDIO_FREQUENCY; // * 0.75; // fix: -25% speed
    ay_desc.magnitude = 1.0f;
    ay38910_init(&ay, &ay_desc);

    // ayumi
    const int is_ym = 1; // should be 0, but 1 sounds more Speccy to me somehow (?)
    const int eqp_stereo_on = 0;
    const double pan_modes[7][3] = { // pan modes, 7 stereo types
      {0.50, 0.50, 0.50}, // MONO, original
      {0.10, 0.50, 0.90}, // ABC, common in west-Europe
      {0.10, 0.90, 0.50}, // ACB, common in east-Europe
      {0.50, 0.10, 0.90}, // BAC
      {0.90, 0.10, 0.50}, // BCA
      {0.50, 0.90, 0.10}, // CAB
      {0.90, 0.50, 0.10}, // CBA
    };

    if (!ayumi_configure(&ayumi, is_ym, 2000000 * (2000000.0 / (ZX_FREQ / 2.0)), AUDIO_FREQUENCY)) { // ayumi is AtariST based, i guess. use 2mhz clock instead
        die("ayumi_configure error (wrong sample rate?)");
    }
    const double *pan = pan_modes[0];   // @fixme: ACB, mono for now
    if(ZX_PENTAGON) pan = pan_modes[0]; // @fixme: ABC, mono for now
    ayumi_set_pan(&ayumi, 0, pan[0], eqp_stereo_on);
    ayumi_set_pan(&ayumi, 1, pan[1], eqp_stereo_on);
    ayumi_set_pan(&ayumi, 2, pan[2], eqp_stereo_on);

    fdc_reset();

    do_once Reset1793(&wd,fdd,WD1793_INIT);

    rom_restore();

    reset(FLAGS);
    z80_quickreset(0);

//    memset(mem, 0x00, 16384*16);
}
