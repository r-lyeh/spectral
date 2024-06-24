// # build (windows)
// cl app.c /FeSpectral.exe /O2 /MT /DNDEBUG=3 /GL /GF /arch:AVX2
//
// # build (linux, debian)
// sudo apt-get install mesa-common-dev libx11-dev gcc libgl1-mesa-dev libasound2-dev
// gcc app.c -o Spectral -O3 -DNDEBUG=3 -Wno-unused-result -Wno-format -Wno-multichar -lm -ldl -lX11 -lGL -lasound -lpthread
//
// # done
// cpu, ula, mem, rom, 48/128, key, joy, ula+, tap, ay, beep, sna/128, fps, tzx, if2, zip, rf, menu, kms, z80, scr,
// key2/3, +2a/+3, fdc, dsk, autotape, gui, KL modes, load "" code, +3 fdc sounds, +3 speedlock, issue 2/3,
// pentagon, trdos, trdos (boot), translate game menus, 50/60 hz, game2exe,
// zxdb, custom tiny zxdb fmt, embedded zxdb, zxdb cache, zxdb download on demand,
// ay player,
// glue sequential tzx/taps in zips (side A) -> side 1 etc)
// sequential tzx/taps/dsks do not reset model

#define SPECTRAL "v1.0 wip"

#define README \
"Spectral can be configured with a mouse.\n\n" \
"Here are some keyboard shortcuts, though:\n" \
"- ESC: Game browser\n" \
"- F1: CPU throttle (hold)\n" \
"- F2: Start/stop tape\n" \
"- F3/F4: Rewind/advance tape\n" \
"- F5: Reload game\n" \
"- F6: Toggle input latency (Run-a-head)\n" \
"- F7: Toggle keyboard issue 2/3\n" \
"- F8: Toggle tape speed\n" \
"- F9: Toggle TV/RF (4 modes)\n" \
"- F9+SHIFT: Toggle AY core (2 modes)\n" \
"- F11/F12: Quick save/load\n" \
"- ALT+ENTER: Fullscreen\n" \
"- TAB+CURSORS: Joysticks\n"

#if NDEBUG >= 2
#define DEV 0
#else
#define DEV 1
#endif

#define ALT_BORDER 0 // `1` not ready yet


// ref: http://www.zxdesign.info/vidparam.shtml
// https://worldofspectrum.org/faq/reference/48kreference.htm
// https://faqwiki.zxnet.co.uk/wiki/ULAplus
// https://foro.speccy.org/viewtopic.php?t=2319

// @todo:
// [ ] ui_inputbox(). remove prompt()
// [ ] widescreen fake borders
// [ ] animated states
// [ ] auto-saves, then F11 to rewind. use bottom bar
// [ ] scan folder if dropped or supplied via cmdline
// [ ] live coding disasm (like bonzomatic)
// [ ] convert side-b/mp3s into voc/pulses
// [ ] db interface [optionally: y/n/why]
//     [hearts] NUM. Title(F2 to rename)   [ghost-load*][ghost-run*][ghost-play*][ghost-snd*] [*:white,red,yellow,green]
//     on hover: show animated state if exists. show loading screen otherwise.
//
// idea: when stop-block is off
// - turn autoplay=off
// - wait silently for any key to be pressed, then turn autoplay=on again
//
// profiler (auto)
// -           []
// -   []      [][]
// - [][][][][][][][][][] ...
// - 4000 4800 5000 5800  ...
// profiler (instrumented)
// IN TS
// OUT PROFILER+0, COLOR; OUT PROFILER+1, TS_DIFF
// profiler (instrumented, other; ticks get accumulated within a second; border is not rendered as usual)
// OUT 254, COLOR // on
// OUT 254, 0 // off
// analyzer
// https://www.youtube.com/watch?v=MPUL0LAUsck&ab_channel=theALFEST
// "Stuff that uses the keyboard" and "What wrote this pixel/attribute" is just awesome.
//
// idea: embed bittorrent client to mirror the WOS/ZXDB/NVG/Archive.org distros

// todo (tapes)
// [ ] overlay ETA
// [ ] auto rewind
// [ ] auto-rewind at end of tape if multiload found (auto-stop detected)
// [ ] auto-insert next tape at end of tape (merge both during tzx_load! argv[1] argv[2])
// [ ] when first stop-the-tape block is reached, trim everything to the left, so first next block will be located at 0% progress bar
// [ ] trap rom loading, edge detection
// [ ] glue consecutive tzx/taps in disk
// [ ] glue: do not glue two consecutive tapes if size && hash match (both sides are same)
// [ ] glue: if tape1 does not start with a basic block, swap tapes
// [ ] prefer programs in tape with "128" string on them (barbarian 2; dragon ninja)
//     - if not found, and a "48" string is found, switch model to 48k automatically
// score128: 128 in filename + memcmp("19XY") X <= 8 Y < 5 + sizeof(tape) + memfind("in ayffe") + side b > 48k + program name "128" + filename128  -> load as 128, anything else: load as usr0
//      if single bank > 49k (navy seals), if size(tap)>128k multiload (outrun)
// test autotape with: test joeblade2,atlantis,vegasolaris,amc
// find 1bas-then-1code pairs within tapes. provide a prompt() call if there are more than 1 pair in the tape
//     then, deprecate 0x28 block

// notes about TESTS mode:
// - scans src/tests/ folder
// - creates log per test
// - 48k
// - exits automatically
// - 50% frames not drawn
// - 50% drawn in fastest mode
// @todo: tests
// - send keys via cmdline: "--keys 1,wait,wait,2"
// - send termination time "--maxidle 300"

// try
// https://github.com/anotherlin/z80emu
// https://github.com/kspalaiologos/tinyz80/
// https://github.com/jsanchezv/z80cpp/

// try
// https://damieng.com/blog/2020/05/02/pokes-for-spectrum/
// test(48): RANDOMIZE USR 46578
// test(2Aes): RANDOMIZE USR 20000

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#if !DEV // disable console logging in release builds (needed for linux/osx targets)
#define printf(...) 0
#define puts(...)   1
#endif

// 384x304 to fit border_break.trd resolution
enum { _320 = 384, _319 = _320-1, _321 = _320+1, _32 = (_320-256)/2 };
enum { _240 = 304, _239 = _240-1, _241 = _240+1, _24 = (_240-192)/2 };

#include "3rd.h"
#include "emu.h"
#include "sys.h"
#include "zx.h"
#include "app.h"

int screenshot(const char *filename) {
    extern window *app;
    static uint16_t counter = 0xFFFF; counter = (counter + 1);

#if 0
    time_t timer = time(NULL);
    struct tm* tm_info = localtime(&timer);

    char stamp[32];
    strftime(stamp, 32, "%Y%m%d %H%M%S", tm_info);

    extern window* app;
    int ok1 = writefile(va("%s %s %04x.scr", filename, stamp, counter), VRAM, 6912);
    int ok2 = tigrSaveImage(va("%s %s %04x.png", filename, stamp, counter), app);
#else
    int ok1 = writefile(va("%s-%04x.scr", filename, counter), VRAM, 6912);
    int ok2 = tigrSaveImage(va("%s-%04x.png", filename, counter), app);
#endif

    return ok1 && ok2;
}




// command keys: sent either physically (user) or virtually (ui)
int cmdkey;
char *cmdarg;

void input() {
    // keyboard
    ZXKeyboardClear();

    if( !ZX_DEVTOOLS ) {
        // joysticks
        int up   = window_pressed(app, TK_UP),   down = window_pressed(app, TK_DOWN);
        int left = window_pressed(app, TK_LEFT), right = window_pressed(app, TK_RIGHT);
        int fire = window_pressed(app, TK_TAB);
        ZXJoysticks(up,down,left,right,fire);

        // keyboard
        #define KEYS(k) \
            k(0)k(1)k(2)k(3)k(4)k(5)k(6)k(7)k(8)k(9)\
            k(A)k(B)k(C)k(D)k(E)k(F)k(G)k(H)k(I)k(J)\
            k(K)k(L)k(M)k(N)k(O)k(P)k(Q)k(R)k(S)k(T)\
            k(U)k(V)k(W)k(X)k(Y)k(Z)
        #define K(x) if(window_pressed(app, 0[#x])) ZXKey(ZX_##x);
        KEYS(K);
        if(window_pressed(app, TK_SPACE))      {ZXKey(ZX_SPACE); /*if(mic_on) mic_on = 0, tap_prev();*/ }
        if(window_pressed(app, TK_BACKSPACE))  {ZXKey(ZX_SHIFT); ZXKey(ZX_0);}
        if(window_pressed(app, TK_RETURN))      ZXKey(ZX_ENTER);
        if(window_pressed(app, TK_SHIFT))       ZXKey(ZX_SHIFT);
        if(window_pressed(app, TK_CONTROL))     ZXKey(ZX_SYMB);
        if(window_pressed(app, TK_ALT))         ZXKey(ZX_CTRL);
    }


    // prepare command keys
    if( window_trigger(app, TK_ESCAPE) ) cmdkey = 'ESC';
    if( window_pressed(app, TK_F1) )     cmdkey = 'F1';
    if( window_trigger(app, TK_F2) )     cmdkey = 'F2';
    if( window_trigger(app, TK_F3) )     cmdkey = 'prev';
    if( window_trigger(app, TK_F4) )     cmdkey = 'next';
    if( window_trigger(app, TK_F5) )     cmdkey = 'F5';
    if( window_trigger(app, TK_F6) )     cmdkey = 'RUN';
    if( window_trigger(app, TK_F7) )     cmdkey = 'ISSU';
    if( window_trigger(app, TK_F8) )     cmdkey = 'ffwd';
    if( window_trigger(app, TK_F9) )     cmdkey = window_pressed(app, TK_SHIFT) ? 'AY' : 'F9';
    if( window_trigger(app, TK_F11) )    cmdkey = 'F11';
    if( window_trigger(app, TK_F12) )    cmdkey = 'F12';

    if( window_trigger(app, TK_PRINT) )  cmdkey = 'PIC1';
}






enum { OVERLAY_ALPHA = 96 };
window *app, *ui, *dbg, *overlay; int do_overlay;
int do_disasm;
float fps;







void help() {
    int total = numok+numwarn+numerr;
    char *help = va(
        "Spectral " SPECTRAL " (Public Domain).\n"
        "https://github.com/r-lyeh/Spectral\n\n"
        "Library: %d games found (%d%%)\n\n"
        README "\n", numgames, 100 - (numerr * 100 / (total + !total)));
    (alert)("Spectral " SPECTRAL, help);
}

void titlebar(const char *filename) {
    const char *models[] = { [1]="16",[3]="48",[8]="128",[12]="+2",[13]="+2A",[18]="+3" };
    const char *title = strrchr(filename ? filename : "", DIR_SEP); title = title ? title + 1 : "";
    window_title(app, ZX_PLAYER ? __argv[0] : va("Spectral%s %s%s%s", DEV ? " DEV" : "", models[ZX/16], title[0] ? " - " : "", title));
}

void draw_ui() {

    // draw_compatibility_stats(ui);

    // ui
    int UI_LINE1 = (ZX_CRT ? 2 : 0); // first visible line

    struct mouse m = mouse();
    if( m.cursor == 0 ) {
        m.x = _320/2, m.y = _240/2; // ignore mouse; already clipped & hidden (in-game)
    } else {
        if( !active ) mouse_cursor(1);
    }

    // ui animation
    enum { _60 = 58 };
    int hovering_border = !active && !do_overlay && (m.x > (_320 - _60) || m.x < _60 );
    static float smooth; do_once smooth = hovering_border;
    smooth = smooth * 0.75 + hovering_border * 0.25;
    // left panel: game options
    if( smooth > 0.1 )
    {
        {
            // draw black panel
            TPixel transp = { 0,0,0, 192 * smooth };
            tigrFillRect(ui, -1,-1, smooth * _60, _240+2, transp);
            tigrLine(ui, smooth * _60-2,-1,smooth * _60-2,_240+2, ((TPixel){255,255,255,240*smooth}));
        }

        // left panel
        float chr_x = REMAP(smooth,0,1,-6,0.5) * 11, chr_y = REMAP(smooth,0,1,-3,2.5) * 11;
        int right = chr_x+8*4-4;
//        int bottom = chr_y+8*31.0-1;
        int bottom = _240-11*4+chr_y; // +8*31.0-1;

        ui_at(ui,chr_x,chr_y);

        // sample sketch

        if( ZXDB.ids[0] ) {

        // update titlebar
        titlebar( ZXDB.ids[2] );

        // zxdb
        if( ui_click(va("- %s -", ZXDB.ids[2]), "ZXDB\n"));
        if( ui_click(va("- %s -", ZXDB.ids[1]), "Year\n"));
        if( ui_click(va("- %s -", ZXDB.ids[4]), "Brand\n"));
        if( ui_click(va("- %s -", ZXDB.ids[7] + strspn(ZXDB.ids[7],"0123456789")), "Genre\n"));
        if( ZXDB.ids[6][0] && ui_click(va("- %s -", ZXDB.ids[6]), "Score\n"));

        int len;

        if( zxdb_url(ZXDB, "inlay") && ui_click(va("- Toggle Inlay -"), "Inlays\n")) { // @todo: include scanned instructions and tape scan, and mp3s
            for( char *data = zxdb_download(zxdb_url(ZXDB, "inlay"), &len); data; free(data), data = 0 ) {
                do_overlay ^= 1;
                tigrClear(overlay, !do_overlay ? tigrRGBA(0,0,0,0) : tigrRGBA(0,0,0,OVERLAY_ALPHA));
                if( do_overlay ) {
                    rgba *bitmap = ui_image(data,len, _320,_240, 0);
                    if( bitmap ) {
                        memcpy(overlay->pix, bitmap, _320 * _240 * 4);
                        free( bitmap );
                    }
                }
            }
        }
        if( zxdb_url(ZXDB, "screen") && ui_click(va("- Toggle Screen$ -"), "Screen\n")) {
            for( char *data = zxdb_download(zxdb_url(ZXDB, "screen"), &len); data; free(data), data = 0 ) {
                // loadbin(data, len, false);
                if( len == 6912 ) memcpy(VRAM, data, len);
            }
        }
        if( zxdb_url(ZXDB, "instructions") && ui_click(va("- Toggle Instructions -"), "Help\n")) { // @todo: word wrap. mouse panning. rmb close
            for( char *data = zxdb_download(zxdb_url(ZXDB, "instructions"), &len); data; free(data), data = 0 ) {
                do_overlay ^= 1;
                tigrClear(ui, !do_overlay ? tigrRGBA(0,0,0,0) : tigrRGBA(0,0,0,OVERLAY_ALPHA));
                if( do_overlay ) ui_monospaced = 0, ui_print(overlay, 4,4, ui_colors, as_utf8(replace(data, "\t", " ")));
            }
        }
        if( zxdb_url(ZXDB, "map") && ui_click(va("- Toggle Game Map -"), "Maps\n")) {
            for( char *data = zxdb_download(zxdb_url(ZXDB, "map"), &len); data; free(data), data = 0 ) {
                tigrClear(overlay, !do_overlay ? tigrRGBA(0,0,0,0) : tigrRGBA(0,0,0,OVERLAY_ALPHA));
                do_overlay ^= 1;
                if( do_overlay ) {
                    rgba *bitmap = ui_image(data,len, _320,_240, 0);
                    if( bitmap ) {
                        memcpy(overlay->pix, bitmap, _320 * _240 * 4);
                        free( bitmap );
                    }
                }
            }
        }
        if( zxdb_url(ZXDB, "mp3") && ui_click(va("- Toggle Bonus Track -"), "Bonus\n")) {
            for( char *data = zxdb_download(zxdb_url(ZXDB, "mp3"), &len); data; free(data), data = 0 ) {
                // loadbin(data, len, false);
                // if( len == 6912 ) memcpy(VRAM, data, len);
            }
        }
        if( zxdb_url(ZXDB, "poke") && ui_click("- Cheats -", "Cheats\n") ) { // @todo: selector
            for( char *data = zxdb_download(zxdb_url(ZXDB, "poke"), &len); data; free(data), data = 0 ) {
                loadbin(data, len, false);
            }
        }

        const char *roles[] = {
            ['?'] = "",
            ['C'] = "Code: ",
            ['D'] = "Design: ",
            ['G'] = "Graphics: ",
            ['A'] = "Inlay: ",
            ['V'] = "Levels: ",
            ['S'] = "Screen: ",
            ['T'] = "Translation: ",
            ['M'] = "Music: ",
            ['X'] = "Sound Effects: ",
            ['W'] = "Story Writing: ",
        };

        for( int i = 0; i < 9/*countof(ZXDB.authors)*/; ++i )
        if( ZXDB.authors[i] )
        if( ui_click(va("- %s%s -", roles[ZXDB.authors[i][0]], ZXDB.authors[i]+1), "Author\n"));

        if( ZXDB.ids[8] )
        if( ui_click(va("- %s -", ZXDB.ids[8]), "Tags\n"));
//          if( ui_click("- AY Sound -", "Feat.\n"));
//          if( ui_click("- Multicolour (Rainbow Graphics) -", "Feat.\n"));

        }

        // mags reviews
        // netplay lobby
        // #tags
    }
    // right panel: emulator options
    if( 1 )
    {
        int chr_x = REMAP(smooth,0,1,_320+11,_320-6*11) + 0, chr_y = REMAP(smooth,0,1,-4,2.5) * 11;
        int right = chr_x+8*4-4;
        int bottom = _240-11*4+chr_y; // +8*31.0-1;

        {
            // draw black panel
            TPixel transp = { 0,0,0, 192 * smooth };
            tigrFillRect(ui, REMAP(smooth,0,1,_320,_320-_60), -1, _320*1/2, _240+2, transp);
            tigrLine(ui, REMAP(smooth,0,1,_320,_320-_60)+1,-1,REMAP(smooth,0,1,_320,_320-_60)+1,_240+2, ((TPixel){255,255,255,240*smooth}));
        }

        ui_at(ui,chr_x - 8,chr_y-11);
        if( ui_click("-Take Picture-", "%c", SNAP_CHR) ) cmdkey = 'PIC1'; // send screenshot command

        ui_at(ui,chr_x,chr_y-11);
        if( ui_press("-Full Throttle-", "%c\b\b\b%c\b\b\b%c%d\n\n", PLAY_CHR,PLAY_CHR,PLAY_CHR,(int)fps) ) cmdkey = 'F1';

        const char *models[] = { [1]="16",[3]="48",[8]="128",[12]="+2",[13]="+2A",[18]="+3" };
        if( ui_click("-Reload Model-", "\f%s%s",models[ZX/16],ZX_ALTROMS ? "!":"") ) cmdkey = 'SWAP';
        //if( ui_click("-NMI-", "") ) cmdkey = 'NMI';
        if( ui_click("-Reset-", "\n") ) cmdkey = 'BOMB';

        if( ui_click("-Toggle TV mode-", "▒\f%d\n", 1+(ZX_CRT << 1 | ZX_RF)) ) cmdkey = 'F9';
        if( ui_click("-Toggle AY core-", "♬\f%d\n",ZX_AY) ) cmdkey = 'AY';
        if( ui_click("-Toggle ULAplus-", "%c\f%d\n", ZX_ULAPLUS ? 'U':'u'/*CHIP_CHR '+'*/, ZX_ULAPLUS) ) cmdkey = 'PLUS';

        if( ui_click("-Toggle Joysticks-", "%c\f%d\n", JOYSTICK_CHR, ZX_JOYSTICK)) cmdkey = 'JOY';
        if( ui_click("-Toggle Mouse-", "\x9\f%d\n", ZX_MOUSE) ) cmdkey = 'MICE';
        if( ui_click("-Toggle Lightgun-", "\xB\f%d\n", ZX_GUNSTICK) ) cmdkey = 'GUNS';

        if( ui_click("-Toggle RunAHead-", !ZX_RUNAHEAD ? "🯆\f0\n" : "🯇\f1\n") ) cmdkey = 'RUN';
        if( ui_click("-Toggle TurboROM-", !ZX_TURBOROM ? "\f0\n" : "\f1\n")) cmdkey = 'TROM';
        if( ui_click("-Toggle FastTape-", "\b\b%c\b\b\b%c\b%d\n", PLAY_CHR,PLAY_CHR,ZX_FASTTAPE )) cmdkey = 'ffwd';

        if( ui_click("-Translate game menus-", "T\f%d\n", ZX_AUTOLOCALE)) cmdkey = 'ALOC';

        // if( ui_click("-Toggle Speed-", "\f%d\n", (int)(ZX_FPS*50.)) ) cmdkey = 'FPS';
        if( ui_click("-Toggle 50/60 Hz-", "\f%d\n", ZX_FPS > 100) ) cmdkey = 'FPS';

        if( ui_click("-Toggle letter mode-", "~%c~\f%d\n", ZX_KLMODE ? 'L' : 'K', ZX_KLMODE) ) cmdkey = 'KL';

        //if( ui_click("-Toggle TapePolarity-", "%c\f%d\n", mic_low ? '+':'-', !mic_low) ) cmdkey = 'POLR';
        // if( ZX < 128 )
        if( ui_click("-Toggle Keyboard Issue 2/3-", "i\f%d\n", issue2 ? 2 : 3)) cmdkey = 'ISSU';

        if( DEV )
        if( ui_click("-Toggle DevTools -", "m\f%d\n", ZX_DEVTOOLS)) cmdkey = 'DEVT';

        if( ZX == 128 )
        if( ui_click("-Toggle Pentagon-", "%c\f%d\n", ZX_PENTAGON ? 'P':'p',/*beta128+*/ZX_PENTAGON)) {
            ZX_PENTAGON ^= 1;
            rom_restore();

            if( GET_MAPPED_ROMBANK() == GET_EDITOR_ROMBANK() )
                reset(ZX);
        }

        ui_at(ui,chr_x - 8,bottom+1);
        if( ui_click(NULL, "") ) cmdkey = 'HELP';

        ui_at(ui,right,bottom);
        if( ui_click("-Debug-", "") ) cmdkey = 'DEV'; // send disassemble command
    }

    // tape progress
    float pct = tape_tellf();
    int visible = !active && !do_overlay ? ( pct <= 1. && mic_on/*tape_playing()*/ ) || (m.y > -10 && m.y < _240/10) : 0;
    static float smoothY; do_once smoothY = visible;
    smoothY = smoothY * 0.75 + visible * 0.25;
    if( smoothY > 0.01 )
    {
        int y = REMAP(smoothY,0,1,-10,UI_LINE1);

        TPixel white = {255,255,255,255}, black = {0,0,0,255}, *bar = &ui->pix[0 + y * _320];

        // bars & progress
        unsigned mark = pct * _320;
        if(y>= 0) for( int x = 0; x < _320; ++x ) bar[x] = white;
        if(y>=-2) for( int x = 0; x < _320; ++x ) bar[x+2*_320] = white;
        if(y>= 1) for( int x = 0; x<=mark; ++x )  bar[x+_320] = white;
        if(y>=-2) for( int x = 1; x<=mark; ++x )  bar[-1+2*_320] = black;
        if(y>=-1) for( int x = 0; x < _320; ++x ) {
            if(tape_preview[x]) bar[x+1*_320] = white;
        }
        // triangle marker (top)
        if(y>=-4) bar[mark+4*_320] = white;
        if(y>=-5) for(int i = -1; i <= +1; ++i) if((mark+i)>=0 && (mark+i)<_320) bar[mark+i+5*_320] = white;
        if(y>=-6) for(int i = -2; i <= +2; ++i) if((mark+i)>=0 && (mark+i)<_320) bar[mark+i+6*_320] = white;
        // mouse seeking
        if( m.y > 0 && m.y < 11 ) {
            mouse_cursor(2);
            if( m.buttons ) {
                m.x = m.x < 0 ? 0 : m.x > _320 ? _320 : m.x;
                tape_seekf(m.x / (float)_320);
            }
        }
    }

    ui_at(ui, 1*11, 1*11);
    if( numgames ) {
        if( ui_click(NULL, "%c", !active ? PLAY_CHR : PAUSE_CHR) ) active ^= 1;
    }
    else {
        if( ui_click("-Scan games folder-", "%c\n", FOLDER_CHR) ) cmdkey = 'SCAN';
    }

    // manual tape handling
    if( 1 ) {
        int visible = !active && !do_overlay ? (m.y > -10 && m.y < _240/10) : 0;
        static float smoothY; do_once smoothY = visible;
        smoothY = smoothY * 0.75 + visible * 0.25;

        int y = REMAP(smoothY,0,1,-10,1*11);
        ui_at(ui, ui_x, y+1 );
#if 0
        if( ui_click(NULL, "\xf\b\b\b\xf") ) cmdkey = 'prev';
        if( ui_click(NULL, "%c\b\b\b%c", PLAY_CHR, PLAY_CHR) ) cmdkey = 'next';
        if( ui_click(NULL, "■") ) cmdkey = 'stop';

        ui_x += 2;
        if( ui_click(NULL, "\xe") ) active ^= 1;
        ui_x += 1;
        if( ZX_AUTOPLAY )
        if( ui_click(NULL, "P%d,", autoplay));
        if( ZX_AUTOSTOP )
        if( ui_click(NULL, "S%d", autostop));
#endif
    }

    // bottom slider. @todo: rewrite this into a RZX player/recorder
    if( ZX_DEBUG )
    if( !active && !do_overlay ) {
        static float my_var = 0; // [-2,2]

        TPixel white = {255,255,255,255}, black = {0,0,0,255}, *bar = &ui->pix[0 + (_240-7) * _320];
        unsigned mark = REMAP(my_var, -2,2, 0,1) * _320;
        // triangle marker (bottom)
        for(int i = -2; i <= +2; ++i) if((mark+i)>=0 && (mark+i)<_320) bar[mark+i+0*_320] = white;
        for(int i = -1; i <= +1; ++i) if((mark+i)>=0 && (mark+i)<_320) bar[mark+i+1*_320] = white;
        bar[mark+2*_320] = white;
        bar += _320 * 4;
        // bars & progress
        for( int x = 0; x < _320; ++x ) bar[x] = bar[x+2*_320] = white;
        for( int x = 0; x<=mark; ++x ) bar[x+_320] = white; bar[_320-1+_320] = white;
        // mouse seeking
        if( m.y >= (_240-11) && m.y < _240 ) {
            mouse_cursor(2);
            if( m.buttons/*&4*/ ) {
                m.x = m.x < 0 ? 0 : m.x > _320 ? _320 : m.x;
                float target = REMAP(m.x, 0,_320, 0.98,1.2);
                my_var = my_var * 0.50f + target * 0.50f ; // animate seeking
                // print my_var value
                char text[32]; sprintf(text, "%.4f", my_var);
                window_printxy(ui, text, (mark+5)/11.f,(_240-12.0)/11);
            }
        }
    }
}


char* game_browser(int version) { // returns true if loaded
    // scan files
    do_once
    {
        uint64_t then = time_ns();
        const char *folder = "./games/";
#if TESTS
        folder = "./src/tests/";
#endif
        rescan(folder);
        printf("%5.2fs rescan\n", (time_ns() - then)/1e9);
    }

    if( !numgames && !zxdb_loaded() ) return 0;

    if( !active ) return 0;

    // game browser
    if( active ) {
        // disable overlay
        if( do_overlay ) tigrClear(overlay, tigrRGBA(0,0,0,0));
        do_overlay = 0;

        // restore mouse interaction in case it is being clipped (see: kempston mouse)
        mouse_clip(0);
        mouse_cursor(1);
    }

    if( version == 1 ) return game_browser_v1();
    if( version == 2 ) return game_browser_v2();

    return NULL;
}

void logo(void) {
    cputs("\3 \3 \3 \3 \3 \3 \3 \3 \2 \2 \2 \2 \2 \2 \2 \2 \2 \2 \2 \2 \2 \2 \2 \2 \6 \6 \6 \6 \6 \6 \6 \6 \6 \6 \6 \6 \6 \6 \6 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \1 \1 \1 \1 \1█");
    cputs("\3█\3▀\3▀\3▀\3▀\3▀\2▀\2▀\2▀\2▀\2 \2█\2▀\2▀\2▀\2▀\2▀\2▀\2▀\2▀\2█\2 \6█\6▀\6▀\6▀\6▀\6▀\6▀\6▀\6▀\6▀\6 \6█\6▀\4▀\4▀\4▀\4▀\4▀\4▀\4▀\4▀\4 \4▀\4▀\4▀\4▀\4█\4▀\4▀\4▀\4▀\5▀\5 \5█\5▀\5▀\5▀\5▀\5▀\5▀\5▀\5▀\5▀\5 \5▀\5▀\5▀\5▀\5▀\1▀\1▀\1▀\1▀\1█\1 \1█");
    cputs("\3▀\3▀\3▀\3▀\2▀\2▀\2▀\2▀\2▀\2█\2 \2█\2 \2 \2 \2 \2 \2 \2 \2 \6█\6 \6█\6▀\6▀\6▀\6▀\6▀\6▀\6▀\6▀\6▀\6 \4█\4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4█\4 \4 \5 \5 \5 \5 \5█\5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5█\5▀\5▀\1▀\1▀\1▀\1▀\1▀\1▀\1█\1 \1█");
    cputs("\3▀\3▀\2▀\2▀\2▀\2▀\2▀\2▀\2▀\2▀\2 \2█\2▀\2▀\2▀\2▀\2▀\2▀\6▀\6▀\6▀\6 \6▀\6▀\6▀\6▀\6▀\6▀\6▀\6▀\6▀\4▀\4 \4▀\4▀\4▀\4▀\4▀\4▀\4▀\4▀\4▀\4▀\4 \4 \4 \4 \4 \5▀\5 \5 \5 \5 \5 \5 \5▀\5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5▀\1▀\1▀\1▀\1▀\1▀\1▀\1▀\1▀\1▀\1 \1▀" "\007 " SPECTRAL);
    cputs("\3 \2 \2 \2 \2 \2 \2 \2 \2 \2 \2 \2█\2 \6 \6 \6 \6 \6 \6 \6 \6 \6 \6 \6 \6 \6 \6 \6 \6 \6 \6 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \4 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \5 \1 \1 \1 \1 \1 \1 \1 \1 \1 \1 \1" ANSI_RESET);

    // works on win, lin, osx. wont work on lubuntu/lxterminal, though.
    //cputs("\3┏\2┓\2 \6 \6 \6 \4 \4 \4 \5 \5 \5 \1┓");
    //cputs("\2┗\2┓\6┏\6┓\6┏\4┓\4┏\4╋\5┏\5┓\5┏\1┓\1┃");
    //cputs("\2┗\6┛\6┣\6┛\4┗\4 \4┗\5┗\5┛\5 \1┗\1┻\1┗" "\007" SPECTRAL);
    //cputs("\6 \6 \4┛\4 \4 \4 \5 \5 \5 \1 \1 \1 \1 " ANSI_RESET);

    //puts("┌┐          ┐");
    //puts("└┐┌┐┌┐┌┼┌┐┌┐│");
    //puts("└┘├┘└ └└┘ └┴└");
    //puts("  ┘          ");
    //puts("┏┐          ┐");
    //puts("└┐┏┐┏┐┏┼┏┐┏┐┃");
    //puts("└┛├┛└ └└┛ └┻└");
    //puts("  ┛          ");
    //puts("┏┓          ┓");
    //puts("┗┓┏┓┏┓┏╋┏┓┏┓┃");
    //puts("┗┛┣┛┗ ┗┗┛ ┗┻┗");
    //puts("  ┛          ");
}

int main() {

#ifdef _WIN32 // 3rd_tfd.h requires this
    HRESULT lHResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
#endif

    logo();

    // install icon hooks for any upcoming window or modal creation
    window_override_icons();

    // convert relative paths
    for( int i = 1; i < __argc; ++i ) {
        if( __argv[i][0] != '-' ) {
            char full[MAX_PATH] = {0};
            realpath(__argv[i], full);
            __argv[i] = strdup(full); // @leak
        }
    }

    // initialize tests
    printer = stdout;
#if TESTS
    {
        if( __argc <= 1 ) die("error: no test file provided");
        printer = fopen(va("%s.txt", __argv[1]), "wt"); //"a+t");
        if(!printer) die("cant open file for logging");

        ZX_FASTCPU = 1;
    }
#endif

    // relocate to exedir
    cwdexe();
    // normalize argv[0] extension
#ifdef _WIN32
    __argv[0] = strdup(va("%s%s", __argv[0], strendi(__argv[0], ".exe") ? "" : ".exe" ));
#endif

    // init external zxdb (higher precedence)
    zxdb_init("Spectral.db");

    // init embedded zxdb (lower precedence)
    // also change app behavior to ZX_PLAYER if embedded games are detected.
    {
        int embedlen; const char *embed;
        for( unsigned id = 0; (embed = embedded(id, &embedlen)); ++id ) {
            if(!memcmp(embed + 0000, "\x1f\x8b\x08",3))
            if(!memcmp(embed + 0x0A, "Spectral.db",11)) {
                zxdb_initmem(embed, embedlen);
                continue;
            }
            if( embedlen ) ZX_PLAYER |= 1;
        }
    }

    // fixed settings on zxplayer builds
    if( ZX_PLAYER ) {
        ZX_RF = 1;
        ZX_CRT = 1;
    }

    // prepare title based on argv[0]
    char *apptitle = va("%s", __argv[0]);
    while( strrchr(apptitle,'\\') ) apptitle = strrchr(apptitle,'\\') + 1;
    while( strrchr(apptitle, '/') ) apptitle = strrchr(apptitle, '/') + 1;
    if( strrchr(apptitle, '.') ) *strrchr(apptitle, '.') = '\0';
    apptitle = va("%s%s", apptitle, DEV ? " DEV" : "");

    // app
    app = window_open(_32+256+_32, _24+192+_24, apptitle);
    ui = window_bitmap(_320, _240);
    dbg = window_bitmap(_320, _240);
    overlay = window_bitmap(_320, _240);

    // postfx
    crt(ZX_CRT);

    // must be as close to frame() as possible
    audio_init();

    // zx
    boot(128, 0);

    // load embedded games (if any)
    {
        int embedlen; const char *embed;
        for( unsigned id = 0; (embed = embedded(id, &embedlen)); ++id ) {
            if(!memcmp(embed + 0000, "\x1f\x8b\x08",3))
            if(!memcmp(embed + 0x0A, "Spectral.db",11))
                continue;
            loadbin(embed, embedlen, 1);
        }
    }

    // zxplayer does not load/save state
    if(!ZX_PLAYER)

    // import state
    for( FILE *state = fopen("Spectral.sav","rb"); state; fclose(state), state = 0) {
        if( import_state(state) )
            pins = z80_prefetch(&cpu, cpu.pc),
            titlebar(0);
    }

    if(0)
    for( FILE *state = fopen("Spectral.cpu","rb"); state; fclose(state), state = 0) {
        fread(&cpu, 1, sizeof(cpu), state);
        pins = z80_prefetch(&cpu, cpu.pc);
    }


    // main loop
    do {
#if 1
        // 4 parameters in our shader. we use parameters[0] to track time
        if( ZX_CRT )
        tigrSetPostFX(app, (ticks / (69888 * 50.)), 0, 0, 0);
        else
        tigrSetPostFX(app, 0, 0, 0, 1);
#endif

        ui_frame();
        input();
        if(do_overlay) ZXKeyboardClear(); // do not submit keys to ZX while overlay is drawn on top

        int likely_loading = (PC(cpu) & 0xFF00) == 0x0500 ? 1 : tape_hz > 40;

        int tape_accelerated = ZX_FASTCPU ? 1
            : tape_inserted() && tape_peek() == 'o' ? 0 
            : tape_playing() && likely_loading && ZX_FASTTAPE;
        if( active ) tape_accelerated = 0;

        // z80, ula, audio, etc
        // static int frame = 0; ++frame;
        int do_sim = active ? 0 : 1;
        int do_drawmode = 1; // no render (<0), full frame (0), scanlines (1)
        int do_flashbit = tape_accelerated ? 0 : 1;
        int do_runahead = tape_accelerated ? 0 : ZX_RUNAHEAD;

#if TESTS
        // be fast. 50% frames not drawn. the other 50% are drawn in the fastest mode
        static byte even = 0; ++even;
        do_drawmode = even & 1; 

        // monitor test for completion
        static byte check_tests = 0;
        if( !check_tests++ )
        {
            static unsigned prev = 0;
            static unsigned stalled = 0;

            struct stat st;
            if( fstat(fileno(printer), &st) == 0 ) {
                if( prev == st.st_size ) ++stalled;
                else prev = st.st_size, stalled = 0;
            }

            // finish test after being idle for 15,000,000 frames
            if( stalled >= (50*300000/256) ) {
                fprintf(printer, "Quitting test because of inactivity.\n");
                exit(0);
            }
        }
#endif

        if( ZX_TURBOROM )
        rom_patch_turbo();
        rom_patch_klmode();

        static byte counter = 0; // flip flash every 16 frames @ 50hz
        if( !((++counter) & 15) ) if(do_flashbit) ZXFlashFlag ^= 1;

if( do_runahead == 0 ) {
        do_audio = 1;
        frame(do_drawmode, do_sim); //tape_accelerated ? (frame%50?0:1) : 1 );
} else {
        // runahead:
        // - https://near.sh/articles/input/run-ahead https://www.youtube.com/watch?v=_qys9sdzJKI // https://docs.libretro.com/guides/runahead/

        do_audio = 1;
        frame(-1, do_sim);

        quicksave(10);

        do_audio = 0;
        frame(do_drawmode, do_sim);

        quickload(10);
}

        // screenshots: after drawn frame, before UI
        if( cmdkey == 'PIC2' ) {
            screenshot( window_title(app, NULL) );
            play('cam', 1);
        }

        static char status[128] = "";
        char *ptr = status;
        ptr += sprintf(ptr, "%dm%02ds ", (unsigned)(timer) / 60, (unsigned)(timer) % 60);
        ptr += sprintf(ptr, "%5.2ffps%s %d mem%s%d%d%d%d ", fps, do_runahead ? "!":"", ZX, rom_patches ? "!":"", GET_MAPPED_ROMBANK(), (page128&8?7:5), 2, page128&7);
        ptr += sprintf(ptr, "%02X%c%02X %04X ", page128, page128&32?'!':' ', page2a, PC(cpu));
        ptr += sprintf(ptr, "%c%c %4dHz ", "  +-"[tape_inserted()*2+tape_level()], toupper(tape_peek()), tape_hz);

        tigrClear(ui, !active && !do_overlay ? tigrRGBA(0,0,0,0) : tigrRGBA(0,0,0,128));

        if( DEV ) {
            float x = 0.5, y = 25.5;
            ui_print(ui,  x*11, y*11, ui_colors, status);
        }

        if( ZX_DEBUG ) {
            tigrClear(dbg, tigrRGBA(0,0,0,128));

            float x = 0.5, y = 2;

            ui_print(dbg, x*11, y*11, ui_colors, status), y += 1.5;

            ui_print(dbg, x*11, y*11, ui_colors, regs(0)), y += 5;

            ui_print(dbg, x*11, y*11, ui_colors, dis(PC(cpu), 22)), y += 22;
        }
            
        char* game = game_browser(ZX_BROWSER);
        if( game ) {
            active = 0;

            bool insert_next_disk_or_tape = false;
            if( last_load ) {
                if( 0 != strcmp(game, last_load) ) {
                    const char *a1 = game, *a2 = last_load;

                    // basenames and their lengths
                    const char *b1 = strrchr(a1, '/') ? strrchr(a1, '/')+1 : a1; int l1 = strlen(b1);
                    const char *b2 = strrchr(a2, '/') ? strrchr(a2, '/')+1 : a2; int l2 = strlen(b2);
                    // printf("%s(%d) %s(%d)\n", b1,l1, b2,l2);

                    // multi-load tapes and disks are well named (eg, Mutants - Side 1.tzx). 
                    // following oneliner hack prevents some small filenames to be catched in the 
                    // diff trap below. eg, 1942.tzx / 1943.tzx; they do not belong to each other
                    // albeit their ascii diff is exactly `1`.
                    if( l1 > 8 )

                    if( l1 == l2 ) {
                        int diff = 0;
                        for( int i = 0; i < l1; ++i ) {
                            diff += b1[i] - b2[i];
                        }
                        insert_next_disk_or_tape = diff == 1;
                    }
                }
            }

            int model = strstri(game, ".dsk") ? 300 : window_pressed(app, TK_SHIFT) ? 48 : 128;
            int must_clear = insert_next_disk_or_tape || strstr(game, ".pok") || strstr(game, ".POK") ? 0 : 1;
            int must_turbo = window_pressed(app,TK_CONTROL) || ZX_TURBOROM ? 1 : 0;
            int use_preloader = must_clear ? 1 : 0;

            if( must_clear ) boot(model, 0);
            if( must_turbo ) rom_patch_turbo();

            if( loadfile(game,use_preloader) ) {
                titlebar(game);

                // clear window keys so the current key presses are not being sent to the 
                // next emulation frame. @fixme: use ZXKeyboardClear(); instead
                memset(tigrInternal(app)->keys, 0, sizeof(tigrInternal(app)->keys));
                memset(tigrInternal(app)->prev, 0, sizeof(tigrInternal(app)->prev));
            }
        }

        // measure time & frame lock (50.01 fps)
        int max_speed = tape_accelerated || !ZX_FPS || ZX_FASTCPU; // max speed if tape_accelerated or no fps lock
        if( max_speed ) {
            dt = tigrTime();
            // constant time flashing when loading accelerated tapes (every 16 frames @ 50hz)
            static float accum = 0; accum += dt;
            if( accum >= 0.32f ) accum = 0, ZXFlashFlag ^= 1;
        } else {
#if 0 // no lock
            dt = tigrTime();
#elif 0 // naive
            sys_sleep(1000/50.f); // 50 -> 39 fps
            dt = tigrTime();
#elif 0 // less naive
            dt = tigrTime();
            if( dt < (1000/50.f) ) sys_sleep( (1000/50) - dt );
#else // accurate (beware of CPU usage)
            float target = (ZX_FPS/100.0) * (ZX < 128 ? 50.08:50.01);

            // be nice to os
            sys_sleep(ZX_FPS > 120 ? 1 : 5);
            // complete with shortest sleeps (yields) until we hit target fps
            dt = tigrTime();
            for( float target_fps = 1.f/(target+!target); dt < target_fps; ) {
                sys_yield();
                dt += tigrTime();
            }
#endif
        }

        // zxplayer quits loop early, so...
        // it wont be drawing UI
        // it wont be drawing game browser
        // it wont be processing cmdline/FN-keys commands
        // it wont be processing drag 'n drops
        if( ZX_PLAYER ) {
            window_update(app);
            continue;
        }

        // calc fps
        static int frames = 0; ++frames;
        static double time_now = 0; time_now += dt;
        if( time_now >= 1 ) { fps = frames / time_now; time_now = frames = 0; }

        // tape timer
        if(tape_playing()) timer += dt;

        // stats & debug
        draw_ui();

        if( ZX_DEBUG )
        tigrBlitAlpha(app, dbg, 0,0, 0,0, _320,_240, 1.0f);

        // draw ui on top
        tigrBlitAlpha(app, ui, 0,0, 0,0, _320,_240, 1.0f);

        // draw overlay on top
        if( do_overlay ) {

        static int x0 = 0, y0 = 0; static int x = 0, y = 0;
        static struct mouse prev = {0};
        struct mouse now = mouse();
        if( now.lb && !prev.lb ) x0 = now.x, y0 = now.y;
        if( now.lb ) x = (now.x - x0), y = (now.y - y0);
        else x *= 0.95, y *= 0.95;
        prev = now;

        tigrBlitAlpha(app, overlay, x,y, 0,0, _320,_240, 1.0f);
        if( mouse().rb || window_pressed(app,TK_ESCAPE) ) do_overlay = 0, tigrClear(overlay, tigrRGBA(0,0,0,0));
        }

        // flush
        window_update(app);

        #define LOAD(ZX,TURBO,file) if(file) do { \
                boot(ZX, 0); if(TURBO || window_pressed(app,TK_CONTROL)) rom_patch_turbo(); \
                if( !loadfile(file,1) ) { \
                    if( !load_shader( file ) ) { \
                        if( is_folder(file) ) cmdkey = 'SCAN', cmdarg = file; \
                        else alert(va("cannot open '%s' file\n", file)); \
                    } \
                } \
            } while(0)

        // parse drag 'n drops. reload if needed
        for( char **list = tigrDropFiles(app,0,0); list; list = 0)
        for( int i = 0; list[i]; ++i ) {
            #if TESTS
            LOAD(48,1,list[i]);
            #else
            LOAD(ZX,ZX_TURBOROM,list[i]);
            #endif
        }

        // parse cmdline. reload if needed
        do_once
        for( int i = 1; i < __argc; ++i )
        if( __argv[i][0] != '-' ) {
            #if TESTS
            LOAD(48,1,__argv[i]);
            #else
            LOAD(ZX,ZX_TURBOROM,__argv[i]);
            #endif
        }
        else if( __argv[i][1] == 'v' ) cmdkey = 'HELP';


        // clear command
        int cmdkey_ = cmdkey; cmdkey = 0;
        char *cmdarg_ = cmdarg; cmdarg = 0;

        // parse commands
        ZX_FASTCPU = 0;
        switch(cmdkey_) { default:
            break; case 'ESC':   if( numgames ) active ^= 1;
            break; case  'F1':   ZX_FASTCPU = 1; // fast-forward cpu
            break; case  'F2':   if(!tape_inserted()) active ^= 1; else tape_play(!tape_playing()); // open browser if start_tape is requested but no tape has been ever inserted
            break; case 'prev':  tape_prev();
            break; case 'next':  tape_next();
            break; case 'stop':  tape_stop();
            break; case 'ffwd':  ZX_FASTTAPE ^= 1;
            // cycle tv modes
            break; case  'F9':   { static int mode = 0; do_once mode = ZX_CRT << 1 | ZX_RF; mode = (mode + 1) & 3; ZX_RF = mode & 1; crt(ZX_CRT = !!(mode & 2) ); }
            break; case 'F11':   quicksave(0);
            break; case 'F12':   quickload(0);

            // cycle AY cores
            break; case  'AY':   { const int table[] = { 1,2,0,0 }; ZX_AY = table[ZX_AY]; }

            break; case 'PIC1':   cmdkey = 'PIC2'; // resend screenshot cmd

            break; case 'TROM':  ZX_TURBOROM ^= 1; boot(ZX, 0); loadfile(last_load,1); // toggle turborom and reload
            break; case 'F5':    reset(0); loadfile(last_load, 1);
            break; case 'NMI':   if( pins & Z80_NMI ) pins &= ~Z80_NMI; else pins |= Z80_NMI; // @todo: verify
            break; case 'BOMB':  reset(KEEP_MEDIA/*|QUICK_RESET*/); // if(last_load) free(last_load), last_load = 0;
            break; case 'SWAP':  // toggle model and reload
            {
                static int modes[] = { 128, 48, 200, 210, 300, 16 };
                static byte mode = 0;
                while(modes[mode] != ZX)
                mode = (mode + 1) % 6;
                mode = (mode + 1) % 6;
                ZX = modes[ mode ];
                boot(ZX, 0); loadfile(last_load,1); // toggle rom
                titlebar(last_load); // refresh titlebar
            }
            break; case 'JOY':   ZX_JOYSTICK--; ZX_JOYSTICK += (ZX_JOYSTICK<0)*4; if(ZX_JOYSTICK) ZX_GUNSTICK = 0; // cycle Cursor/Kempston/Fuller,Sinclair1,Sinclair2
            break; case 'GUNS':  ZX_GUNSTICK ^= 1;   if(ZX_GUNSTICK) ZX_MOUSE = 0, ZX_JOYSTICK = 0; // cycle guns
            break; case 'MICE':  ZX_MOUSE ^= 1;      if(ZX_MOUSE) ZX_GUNSTICK = 0;                  // cycle kempston mouse(s)
            break; case 'PLUS':  ZX_ULAPLUS ^= 1;    // cycle ulaplus
            break; case 'RUN':   ZX_RUNAHEAD ^= 1;   // cycle runahead mode
            break; case 'DEV':   ZX_DEBUG ^= 1;
            break; case 'KL':    ZX_KLMODE ^= 1, ZX_KLMODE_PATCH_NEEDED = 1;

            // break; case 'POLR':  mic_low ^= 64;
            break; case 'ISSU':
                                    issue2 ^= 1;

                                    int do_reset = tape_playing() && q.debug && !strchr("uol", q.debug);
                                    if( do_reset ) {
                                        reset(KEEP_MEDIA), loadfile(last_load,1);
                                    }

            //break; case 'FPS':   { const float table[] = { [0]=100,[10]=120,[12]=200,[20]=400,[40]=0 }; ZX_FPS = table[(int)(ZX_FPS*10)]; }
            break; case 'FPS':   { const float table[] = { [10]=120,[12]=100 }; ZX_FPS = table[(int)(ZX_FPS/10)]; }

            break; case 'ALOC':  ZX_AUTOLOCALE ^= 1; if(ZX_AUTOLOCALE) translate(mem, 0x4000*16, 'en');
            break; case 'HELP':  help();

            break; case 'SCAN':  for( const char *f = cmdarg_ ? cmdarg_ : app_selectfolder("Select games folder"); f ; f = 0 )
                                    rescan( f ), active = !!numgames;

            break; case 'DEVT': ZX_DEVTOOLS ^= 1;
        }

    } while( window_alive(app) );

    // zxplayer does not save state
    if(!ZX_PLAYER)

    // export state
    {
        #if NEWCORE
        while( !z80_opdone(&cpu) ) pins = z80_tick(&cpu, pins);
        #endif
        if(medias) media[0].pos = voc_pos / (double)(voc_len+!voc_len);
        for( FILE *state = fopen("Spectral.sav","wb"); state; fclose(state), state = 0) {
            if( !export_state(state) )
                alert("Error exporting state");
        }

        if(0)
        for( FILE *state = fopen("Spectral.cpu","wb"); state; fclose(state), state = 0) {
            fwrite(&cpu, 1, sizeof(cpu), state);
        }
    }

    window_close(app);
    return 0;
}
