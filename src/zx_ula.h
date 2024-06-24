#ifndef ALT_BORDER
#define ALT_BORDER 1
#endif

static int SKIP_PAPER;
static int SKIP_X = 0, SKIP_Z = 0;
static int SKIP_Y = 24, SKIP_B = 64; // PENTAGON


rgba ZXPaletteDef[64] = { // 16 regular, 64 ulaplus
#if 0 // check these against SHIFT-SPC during reset
    rgb(0x00,0x00,0x00),
    rgb(0x00,0x00,0xCD),
    rgb(0xCD,0x00,0x00),
    rgb(0xCD,0x00,0xCD),
    rgb(0x00,0xCD,0x00),
    rgb(0x00,0xCD,0xCD),
    rgb(0xCD,0xCD,0x00),
    rgb(0xD4,0xD4,0xD4),

    rgb(0x00,0x00,0x00),
    rgb(0x00,0x00,0xFF),
    rgb(0xFF,0x00,0x00),
    rgb(0xFF,0x00,0xFF),
    rgb(0x00,0xFF,0x00),
    rgb(0x00,0xFF,0xFF),
    rgb(0xFF,0xFF,0x00),
    rgb(0xFF,0xFF,0xFF),
#elif 0 // my fav, i guess
    rgb(0x00,0x00,0x00), // normal: black,blue,red,pink,green,cyan,yellow,white
    rgb(0x00,0x00,0xC0), // note: D7 seems fine too
    rgb(0xC0,0x00,0x00),
    rgb(0xC0,0x00,0xC0),
    rgb(0x00,0xC0,0x00),
    rgb(0x00,0xC0,0xC0),
    rgb(0xC0,0xC0,0x00),
    rgb(0xC0,0xC0,0xC0),

    rgb(0x00,0x00,0x00), // bright: black,blue,red,pink,green,cyan,yellow,white
    rgb(0x00,0x00,0xFF),
    rgb(0xFF,0x00,0x00),
    rgb(0xFF,0x00,0xFF),
    rgb(0x00,0xFF,0x00),
    rgb(0x00,0xFF,0xFF),
    rgb(0xFF,0xFF,0x00),
    rgb(0xFF,0xFF,0xFF),
#elif 0
    // Richard Atkinson's colors (zx16/48)
    rgb(0x06,0x08,0x00),
    rgb(0x0D,0x13,0xA7),
    rgb(0xBD,0x07,0x07),
    rgb(0xC3,0x12,0xAF),
    rgb(0x07,0xBA,0x0C),
    rgb(0x0D,0xC6,0xB4),
    rgb(0xBC,0xB9,0x14),
    rgb(0xC2,0xC4,0xBC),

    rgb(0x06,0x08,0x00),
    rgb(0x16,0x1C,0xB0),
    rgb(0xCE,0x18,0x18),
    rgb(0xDC,0x2C,0xC8),
    rgb(0x28,0xDC,0x2D),
    rgb(0x36,0xEF,0xDE),
    rgb(0xEE,0xEB,0x46),
    rgb(0xFD,0xFF,0xF7)
#elif 1 // vivid
    rgb(84/3,77/3,84/3), // made it x3 darker
    // rgb(0x06,0x08,0x00), // normal: black,blue,red,pink,green,cyan,yellow,white
    rgb(0x00,0x00,0xAB), // D8 and 96 looked fine
    rgb(0xAB,0x00,0x00),
    rgb(0xAB,0x00,0xAB),
    rgb(0x00,0xAB,0x00),
    rgb(0x00,0xAB,0xAB),
    rgb(0xAB,0xAB,0x00),
    rgb(0xAB,0xAB,0xAB),

    rgb(84/3,77/3,84/3), // made it x3 darker
    // rgb(0x06,0x08,0x00), // bright: black,blue,red,pink,green,cyan,yellow,white
    rgb(0x00,0x00,0xFF),
    rgb(0xFF,0x00,0x00),
    rgb(0xFF,0x00,0xFF),
    rgb(0x00,0xFF,0x00),
    rgb(0x00,0xFF,0xFF),
    rgb(0xFF,0xFF,0x00), // rgb(0xEE,0xEB,0x46), brighter yellow because jacknipper2 looks washed
    rgb(0xFF,0xFF,0xFF)
#elif 0 // latest
    rgb(0x00,0x00,0x00), // normal: black,blue,red,pink,green,cyan,yellow,white
    rgb(0x00,0x00,0xC0), // note: D7 seems fine too
    rgb(0xC0,0x00,0x00),
    rgb(0xC0,0x00,0xC0),
    rgb(0x00,0xC0,0x00),
    rgb(0x00,0xC0,0xC0),
    rgb(0xC0,0xC0,0x00),
    rgb(0xC0,0xC0,0xC0),

    rgb(0x06,0x08,0x00), // bright: black,blue,red,pink,green,cyan,yellow,white
    rgb(0x16,0x1C,0xB0), // Richard Atkinson's bright colors
    rgb(0xCE,0x18,0x18),
    rgb(0xDC,0x2C,0xC8),
    rgb(0x28,0xDC,0x2D),
    rgb(0x36,0xEF,0xDE),
    rgb(0xEE,0xEB,0x00), // rgb(0xEE,0xEB,0x46), brighter yellow because jacknipper2 looks washed
    rgb(0xFD,0xFF,0xF7)
#elif 0 // mrmo's goblin22 adapted. just for fun
    rgb(84/3,77/3,84/3), // made it x3 darker
    rgb(37,47,64),
    rgb(99,37,14),
    rgb(99,42,123),
    rgb(78,131,87),
    rgb(71,143,202),
    rgb(216,121+121/2,69), // original: 216,121,69
    rgb(160,154,146),

    rgb(84/3,77/3,84/3), // made it x3 darker
    rgb(47,88,141),
    rgb(158,50,39),
    rgb(194,71,184),
    rgb(137,170,85),
    rgb(100,213,223),
    rgb(244,220,109),
    rgb(245,238,228)
#endif
};

byte ulaplus_mode = 0; // 0:pal,1:mode,else:undefined
byte ulaplus_data = 0;
byte ulaplus_enabled = 0;
byte ulaplus_grayscale = 0;
byte ulaplus_registers[64+1] = {0};

void ula_reset() {
    ulaplus_mode = 0;
    ulaplus_data = 0;
    ulaplus_enabled = 0;
    ulaplus_grayscale = 0;
    memset(ulaplus_registers, 0, sizeof(ulaplus_registers));
    memcpy(ZXPalette, ZXPaletteDef, sizeof(ZXPalette[0]) * 64);

#if 0
    // contended lookups
    memset(contended, 0, sizeof(contended));
    for( int i = 63, c = 6; i <= (63+192); ++i, --c ) {
        contended[i*228]=c<0?0:c; // initial: 64*224 (48K)
        if(c<0) c=6;
    }
#endif

    // colorize. this is especially needed on Richard Atkinson's palette (imho)
    // also, for the RF effect, colors are saturated right here instead of during bitmap blits
    for( int i = 0; i < 16; ++i) {
        unsigned r,g,b; byte h,s,v;
        rgb_split(ZXPalette[i],r,g,b);
        rgb2hsv(r,g,b,&h,&s,&v);

                    //unsigned luma = (unsigned)((r * 0.299 + g * 0.587 + 0.114));
                    //luma *= 1.25; if( luma > 255 ) luma = 255;
                    //ZXPalette[i] = rgb(luma,luma,luma);

        // saturated; (h)ue bw-to-(s)aturation (b)rightness
                 // s = s*1.125 < 255 ? s*1.125 : 255; // extra saturated
                    v = v*1.125 < 255 ? v*1.125 : 255; // extra brightness
                    ZXPalette[i] = as_rgb(h,s,v);
                    continue;

        // bw
                    s = 0;
                    v = v*0.98;
                    ZXPalette[i] = as_rgb(h,s,v);
    }
}


#define CLAMP(v, minv, maxv) ((v) < (minv) ? (minv) : (v) > (maxv) ? (maxv) : (v))
#define REMAP(var, src_min, src_max, dst_min, dst_max) \
    (dst_min + ((CLAMP(var, src_min, src_max) - src_min) / (float)(src_max - src_min)) * (dst_max - dst_min))

static float dt;
static double timer;
static int flick_frame;
static int flick_hz;

int tick2offset(int tick, int TS, int *col) {
    // 352x288 phys, _320x_240 virt, bottom/left lines more important than top/upper lines    

tick -= 36; // +8;

    int tx = tick % (TS/2);
    int ty = tick / TS; //ty -= (288 - _240) / 2; if(ty < 0) continue;

    // skip invisible lines (vsync & overscan)
    ty -= SKIP_Y; //(288 - _240) / 2;
    if( ty < 0 ) return -1;
    if( ty >= _240 ) return -1;

    int is_paper_line = ty >= abs(_240-192)/2 && ty < _240-abs(_240-192)/2;

*col = (tx < TS/2) + 2;

if( !is_paper_line ) tx *= 2;
else if( tx >= 32 ) tx += (256+16);

    return tx + ty * _320;
}

void draw_border() {
    extern window *app;
    int width = _32+256+_32;
    rgba *begin = &((rgba*)app->pix)[0 + 0 * width];
    rgba *end = &((rgba*)app->pix)[(app->w-1) + (app->h-1) * (app->w)];

    int TS = (ZX >= 128 ? 228 : 224) - (ZX_PENTAGON * 4);

    if( ZX_DEVTOOLS )
    {
        if( GetFocus() ) {
        if( GetAsyncKeyState('X') & 0x1 ) SKIP_PAPER^= 1;
        if( GetAsyncKeyState('Q') & 0x1 ) SKIP_X++;
        if( GetAsyncKeyState('A') & 0x1 ) SKIP_X--; //if(SKIP_X < 0) SKIP_X = 0;
        if( GetAsyncKeyState('W') & 0x1 ) SKIP_Y++;
        if( GetAsyncKeyState('S') & 0x1 ) SKIP_Y--; if(SKIP_Y < 0) SKIP_Y = 0;
        if( GetAsyncKeyState('E') & 0x1 ) SKIP_Z++;
        if( GetAsyncKeyState('D') & 0x1 ) SKIP_Z--; //if(SKIP_Z < 0) SKIP_Z = 0;
        if( GetAsyncKeyState('R') & 0x1 ) SKIP_B++;
        if( GetAsyncKeyState('F') & 0x1 ) SKIP_B--; //if(SKIP_B < 0) SKIP_B = 0;
        }

        char buf[128];
        sprintf(buf, "TS=%d(%03d,%03d) SKIP=%d,%d,%d,%d [%d|%d], [%d|%d], [%d|%d], [%d|%d], [%d|%d]",
            TS, mouse().x, mouse().y, SKIP_X, SKIP_Y, SKIP_Z, SKIP_B,
            border[0] >> 24, border[0] & 0x00FFFFFF,
            border[1] >> 24, border[1] & 0x00FFFFFF,
            border[2] >> 24, border[2] & 0x00FFFFFF,
            border[3] >> 24, border[3] & 0x00FFFFFF,
            border[4] >> 24, border[4] & 0x00FFFFFF
        );
        window_title(app, buf);
    }

    rgba *texture = 0;
    rgba *origin = 0;

    memset32(begin, ZXPalette[ZXBorderColor], end - begin);

    for( int i = 1; i < border_num; ++i ) { // -1; i >= 1; --i ) {
        int color = border[i-1] >> 24;
        int tick = border[i-1] & 0x00FFFFFF;
        int next = border[i] & 0x00FFFFFF;

        int col1, col2;

        int offset1 = tick2offset(tick, TS, &col1);
        if( offset1 < 0 ) continue;

        assert( tick < next);
        int offset2 = tick2offset(next, TS, &col2);
        if( offset2 < 0 ) continue;

        if( ZX_DEVTOOLS ) color = color ? col1 : color;

        texture = begin + (offset1 % _320) + (offset1 / _320) * _320;
        if(!origin) origin = texture;

        int pixels = abs(offset2 - offset1) * 1;

        if( (texture + pixels) >= end ) {
            int pixels2 = end - texture; assert( end > texture );
            memset32(texture, ZXPalette[color], pixels2);
            break;
        }
        if( pixels ) {
            memset32(texture, ZXPalette[color], pixels);
        }
    }

    // dupe x2
    for( int tick = 0; tick < ZX_TS; tick += TS ) {
        int ty = tick / TS;

        ty -= SKIP_Y;
        if( ty < 0 ) continue;
        if( ty >= _240 ) continue;

        int is_paper_line = ty >= abs(_240-192)/2 && ty < _240-abs(_240-192)/2;
        if(!is_paper_line) continue;

        unsigned *p = &begin[ 0 + ty * _320 ];
        unsigned *q = p + 256+16+48; // 256+16;
        for( int x = 32+1; --x > 0; ) {
            p[x*2-1] = p[x*2-2] = p[x];
            q[x*2-1] = q[x*2-2] = q[x];
        }
    }
}

void draw(window *win, int y /*0..311 tv scanline*/) {
    int width = _32+256+_32;
    rgba *texture = &((rgba*)win->pix)[0 + y * width];

    // int third = y / 64; int y64 = y % 64;
    // int bit3swap = (y64 & 0x38) >> 3 | (y64 & 0x07) << 3;
    // int scanline = (bit3swap + third * 64) << 5;
    #define SCANLINE(y) \
        ((((((y)%64) & 0x38) >> 3 | (((y)%64) & 0x07) << 3) + ((y)/64) * 64) << 5)

#if ALT_BORDER
    texture += SKIP_B/* _32 */;
#else
    // border left
    for(int x=0;x<_32;++x) *texture++=ZXPalette[ZXBorderColor];
#endif

    // paper
    if( y >= (0+_24) && y < (192+_24) )
    {
        y -= _24;
        byte *pixels=VRAM+SCANLINE(y);
        byte *attribs=VRAM+6144+((y&0xF8)<<2);

// RF: misalignment
*texture = ZXPalette[ZXBorderColor];
enum { BAD = 8, POOR = 32, DECENT = 256 };
int shift0 = !ZX_RF ? 0 : (rand()<(RAND_MAX/(ZX_FASTCPU?2:POOR))); // flick_frame * -(!!((y+0)&0x18))
texture += shift0;

        for(int x = 0; x < 32; ++x) {
            byte attr = *attribs;
            byte pixel = *pixels, fg, bg;

            // @fixme: make section branchless

            if (ulaplus_enabled) {
                fg = ((attr & 0xc0) >> 2) | ((attr & 0x07));
                bg = ((attr & 0xc0) >> 2) | ((attr & 0x38) >> 3) | 8;
            } else {
                pixel ^= (attr & 0x80) && ZXFlashFlag ? 0xff : 0x00;
                fg = ((attr >> 3) & 0x08) | (attr & 0x07); // ((attr & 0x40) >> 3); // 0x40 typo?
                bg = ((attr >> 3) & 0x0F);
            }

            // @fixme: make section branchless

            texture[0]=ZXPalette[pixel & 0x80 ? fg : bg];
            texture[1]=ZXPalette[pixel & 0x40 ? fg : bg];
            texture[2]=ZXPalette[pixel & 0x20 ? fg : bg];
            texture[3]=ZXPalette[pixel & 0x10 ? fg : bg];
            texture[4]=ZXPalette[pixel & 0x08 ? fg : bg];
            texture[5]=ZXPalette[pixel & 0x04 ? fg : bg];
            texture[6]=ZXPalette[pixel & 0x02 ? fg : bg];
            texture[7]=ZXPalette[pixel & 0x01 ? fg : bg];

            texture += 8;

            pixels++;
            attribs++;
        }

texture -= shift0;

    }

#if ALT_BORDER
    texture += 256;
#else
    // top/bottom border
    else {
    for(int x=0;x<256;++x) *texture++=ZXPalette[ZXBorderColor];
    }
#endif

#if ALT_BORDER
    texture += _32;
#else
    // border right
    for(int x=0;x<_32;++x) *texture++=ZXPalette[ZXBorderColor];
#endif

#ifdef DEBUG_SCANLINE
    memset(texture, 0xFF, width * 4);
    //    sys_sleep(1000/60.);
#endif
}

void blur(window *win) {
    int height = _24+192+_24;
    int width = _32+256+_32;

    // screen
    static int j = 0;
    static byte jj = 0;

    for( int y = _24; y < _240-_24; ++y ) {
        rgba *texture = &((rgba*)win->pix)[_32 + y * width];

        for(int x=0;x<_32;x++) {

        int shift = (++jj)&1;
        texture += shift;

                // RF: hue shift
                for(int i = 8-1; i >= 0; --i) {
                    unsigned pix0 = texture[i-0];
                    unsigned pix1 = texture[i-1];
                    unsigned pix2 = texture[i-2];
                    unsigned r0,g0,b0; rgb_split(pix0,r0,g0,b0);
                    unsigned r1,g1,b1; rgb_split(pix1,r1,g1,b1);
                    unsigned r2,g2,b2; rgb_split(pix2,r2,g2,b2);
                    if(i&1)
                    texture[i] = rgb((r0+r0+r0+r0)/4,(g0+g0+g0+g0)/4,(b2+b2+b2)/3); // yellow left
                    else
                    texture[i] = rgb((r0+r0+r0+r1)/4,(g0+g0+g0+g1)/4,(b1+b1+b1+b2)/4); // blue left
                    //else
                    //texture[i] = rgb((r0+r1+r1+r1)/4,(g0+g1+g1+g1)/4,(b0+b1+b1)/3);

                    continue;
if(ZX_RF) {
                    // saturate aberrations (very slow)
                    pix0 = texture[i];
                    rgb_split(pix0,r0,g0,b0);
                    byte h0,s0,v0; rgb2hsv(r0,g0,b0,&h0,&s0,&v0);
                    if( s0 * 1.5 > 255 ) s0 = 255; else s0 *= 1.5;
                    if( v0 * 1.01 > 255 ) v0 = 255; else v0 *= 1.01;
                    texture[i] = as_rgb(h0,s0,v0);
}
                }

                // RF: jailbars
                if(x%2)
                for(int i = 0; i < 8; ++i) {
                    unsigned pix0 = texture[i-0];
                    byte r0,g0,b0; rgb_split(pix0,r0,g0,b0);
                    byte h0,s0,v0; rgb2hsv(r0,g0,b0,&h0,&s0,&v0);
                    texture[i] = as_rgb(h0,s0,v0*0.99);
                }

                // RF: interferences
                // interesting tv effects (j): 13, 19, 23, 27, 29, 33
                // note: since CRT shader was introduced this RF effect became less
                // aparent (because of the bilinear smoothing). and that's why we're
                // using 13 now, since the screen stripes it creates are way more visible.
                // used to be 33 all the time before.
                // @fixme: apply this effect to the upper and bottom border as well
                for(int i = 0; i < 8; ++i) { ++j; j%=13; // was 33 before
                    unsigned pix0 = texture[i-0];
                    byte r0,g0,b0; rgb_split(pix0,r0,g0,b0);
                    byte h0,s0,v0; rgb2hsv(r0,g0,b0,&h0,&s0,&v0);
                    if(j<1) texture[i] = as_rgb(h0,s0,v0*0.95);
                    else
                    if(j<2) texture[i] = as_rgb(h0,s0,v0*0.95);
                    else
                            texture[i] = rgb(r0,g0,b0);
                }

        texture -= shift;

            texture += 8;
        }
    }
}

void scanlines(window *win) {
    // hsv: slow. may be better in a shader
    int height = _24+192+_24;
    int width = _32+256+_32;
    for(int y = 0; y < _240; y+=2) {
        rgba *texture = &((rgba*)win->pix)[0 + y * width];
        for(int x = 0; x < width; ++x) {
            unsigned pix0 = texture[x];
            byte r0,g0,b0; rgb_split(pix0,r0,g0,b0);
            byte h0,s0,v0; rgb2hsv(r0,g0,b0,&h0,&s0,&v0);
            texture[x] = as_rgb(h0*0.99,s0,v0*0.98);
        }
    }
}

void frame(int drawmode, int do_sim) { // no render (<0), whole frame (0), scanlines (1)
    extern window *app;

    if( !ZX_DEVTOOLS ) {
        if( ZX <= 48 ) {
            SKIP_Y = 8;
        }
        if( ZX >= 128 ) {
            SKIP_Y = 6;
        }
        if( ZX_PENTAGON ) {
            SKIP_Y = 24, SKIP_B = 64;
        }
    }

    // notify new frame
    if(do_sim) frame_new();

    // NO RENDER
    if( drawmode < 0 ) {
        if(do_sim) run(ZX_TS);
        return;
    }

if( !SKIP_PAPER ) {
#if ALT_BORDER
    // we are drawing border from previous frame
    // technically, we should draw border *after* the frame has been emulated, not before.
    draw_border();
#endif
    border_num = 0;
    border_org = ticks;
    border[border_num] = ZXBorderColor << 24 | 0;
}

    // FRAME RENDER
    if( drawmode == 0 ) {
        if(do_sim) run(ZX_TS);
        for( int y = 0; y < 24+192+24; ++y ) draw(app, y);
        return;
    }

    // most models start in late_timings, and convert into early_timings as they heat
    bool early_timings = ZX != 200; // however, +2 is always late_timings

    // SCANLINE RENDER
    if( ZX_PENTAGON ) {
        // Pentagon: see https://worldofspectrum.net/rusfaq/index.html
        // 320 scanlines = 16 vsync + 64  upper + 192 paper + 48 bottom
        // each scanline = 32 hsync + 36 border + 128 paper + 28 border = 224 TS/scanline
        // total = (16+64+192+48) * 224 = 320 * (32+36+128+28) = 320 * 224 = 71680 TS
        const int TS = 224;
        for( int y = 0; y <  16; ++y ) { if(do_sim) zx_int = !y, run(TS); }
        for( int y = 0; y <  64; ++y ) { if(do_sim) run(TS); if(y>(64-_24-1)) draw(app, y-(64-_24)); }
        for( int y = 0; y < 192; ++y ) { if(do_sim) run(TS);                  draw(app, _24+y); }
        for( int y = 0; y <  48; ++y ) { if(do_sim) run(TS); if(y<_24)        draw(app, _24+192+y); }
    }
    else if( ZX < 128 ) {
        // 48K: see https://wiki.speccy.org/cursos/ensamblador/interrupciones http://www.zxdesign.info/interrupts.shtml
        // 312 scanlines = 16 vsync + 48  upper + 192 paper + 56 bottom
        // each scanline = 48 hsync + 24 border + 128 paper + 24 border = 224 TS/scanline
        // total = (16+48+192+56) * 224 = 312 * (48+24+128+24) = 312 * 224 = 69888 TS
        const int TS = 224;
        if( do_sim ) zx_int = 0, run(24-early_timings), zx_int = 1, run(TS-24-early_timings);
        for( int y = 1; y <  64; ++y ) { if(do_sim) run(TS); if(y>(64-_24-1)) draw(app, y-(64-_24)); }
        for( int y = 0; y < 192; ++y ) { if(do_sim) run(TS);                  draw(app, _24+y); }
        for( int y = 0; y <  56; ++y ) { if(do_sim) run(TS); if(y<_24)        draw(app, _24+192+y); }
    } else {
        // 128K:https://wiki.speccy.org/cursos/ensamblador/interrupciones https://zx-pk.ru/threads/7720-higgins-spectrum-emulator/page4.html
        // 311 scanlines = 15 vsync + 48  upper + 192 paper + 56 bottom
        // each scanline = 48 hsync + 26 border + 128 paper + 26 border = 228 TS/scanline
        // total = (63+192+56) * 228 = 311 * (48+26+128+26) = 311 * 228 = 70908 TS
        const int TS = 228;
        if( do_sim ) zx_int = 0, run(26-early_timings), zx_int = 1, run(TS-26-early_timings);
        for( int y = 2; y <  64; ++y ) { if(do_sim) run(TS); if(y>(64-_24-1)) draw(app, y-(64-_24)); }
        for( int y = 0; y < 192; ++y ) { if(do_sim) run(TS);                  draw(app, _24+y); }
        for( int y = 0; y <  56; ++y ) { if(do_sim) run(TS); if(y<_24)        draw(app, _24+192+y); }
    }

if( SKIP_PAPER ) {
#if ALT_BORDER
    // we are drawing border from previous frame
    // technically, we should draw border *after* the frame has been emulated, not before.
    draw_border();
#endif
    border_num = 0;
    border_org = ticks;
    border[border_num] = ZXBorderColor << 24 | 0;
}

#if 0
    // @todo
    // PIXEL RENDER
    if( drawmode == 2 ) {
        TS = 0;

        // 128K
        // There are 63 scanlines before the television picture, as opposed to 64.
        // To modify the border at the position of the first byte of the screen (see the 48K ZX Spectrum section for details), the OUT must finish after 14365, 14366, 14367 or 14368 T states have passed since interrupt. As with the 48K machine, on some machines all timings (including contended memory timings) are one T state later.
        // Note that this means that there are 70908 T states per frame, and the '50 Hz' interrupt occurs at 50.01 Hz, as compared with 50.08 Hz on the 48K machine. The ULA bug which causes snow when I is set to point to contended memory still occurs, and also appears to crash the machine shortly after I is set to point to contended memory.
        // 228 TS/scanline, 311 scanlines

        // A single ZX Spectrum display row takes 224 T-States, including the horizontal flyback. For every T-State 2 pixels are written to the display, so 128 T-States pass for the 256 pixels in a display row. The ZX Spectrum is clocked at 3.5 MHz, so if 2 pixels are written in a single CPU clock cycle, the pixel clock of our display must be 7 MHz. A single line thus takes 448 pixel clock cycles.
        // The left and right border areas can be shown to be 48 pixels wide, which gives a visible row of 352 pixels (and the border equivalent) in total. That would take 352 / 2 = 176 T-States to display, and we know that a display row takes 224 T-States, so the lost 96 T-States must be used during the horizontal flyback of the electron beam to the start of the new row.

        static int vs = 0;
        if(window_trigger(app,'T')) { vs+=1; printf("%d\n", vs); }
        if(window_trigger(app,'G')) { vs-=1; printf("%d\n", vs); }

        for( int tv = 0; tv < 311; ++tv ) {
            int y = (tv - 24);
            zx_int = (tv == vs);
            if(y < 0 || y >= 240) run(228);
            else {
                // 16px left extra
                run(2*4);
                // 32px left
                run(4*4);
                draw(app, y, 0, 4);
                // 256px main
                int excess = 0;
                for( int x = 0; x < 32; ++x ) {
                    run(4 - excess);
                    excess = 0;
                    excess += !!vram_accesses * contended[TS/4];
                    vram_accesses = 0;
                    draw(app, y, 32+x*8, 1);
                }
                // 32px right
                run(4*4);
                draw(app, y, 32+256, 4);
                // 16px right extra
                run(2*4);
                // retrace
                run(52);
            }
        }
    }
#endif

#if 1
    // detect ZX_RF flip-flop
    static int ZX_RF_old = 1;
    int refresh = ZX_RF ^ ZX_RF_old;
    ZX_RF_old = ZX_RF;

    if(ZX_RF) {
        static window *blend = 0;
        if(!blend) blend = window_bitmap(_320, _240);

        // reset bitmap contents when re-enabling ZX_RF
        if( refresh ) tigrBlit(blend, app, 0,0, 0,0, _320,_240);

        // ghosting
        tigrBlitAlpha(blend, app, 0,0, 0,0, _320,_240, 0.5f); //0.15f);
        tigrBlit(app, blend, 0,0, 0,0, _320,_240);

        // scanlines
        scanlines(app);

        // aberrations
        flick_hz ^= 1; // &1; // (rand()<(RAND_MAX/255));
        flick_frame = cpu.r; // &1; // (rand()<(RAND_MAX/255));
        blur(app);
    }
#endif
}

const char *shader = 
#if 0
"/* HSV from/to RGB conversion functions by Inigo Quilez. https://www.shadertoy.com/view/lsS3Wc (MIT licensed)*/\n"
"const float eps = 0.0000001;\n"
"vec3 hsv2rgb( vec3 c ) {\n"
"    vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );\n"
"    return c.z * mix( vec3(1.0), rgb, c.y);\n"
"}\n"
"vec3 rgb2hsv( vec3 c) {\n"
"    vec4 k = vec4(0.0, -1.0/3.0, 2.0/3.0, -1.0);\n"
"    vec4 p = mix(vec4(c.zy, k.wz), vec4(c.yz, k.xy), (c.z<c.y) ? 1.0 : 0.0);\n"
"    vec4 q = mix(vec4(p.xyw, c.x), vec4(c.x, p.yzx), (p.x<c.x) ? 1.0 : 0.0);\n"
"    float d = q.x - min(q.w, q.y);\n"
"    return vec3(abs(q.z + (q.w - q.y) / (6.0*d+eps)), d / (q.x+eps), q.x);\n"
"}\n"

"/* YUV conversions by icalvin102 https://www.shadertoy.com/view/3lycWz */\n"
"vec3 rgb2yuv(vec3 rgb){\n"
"    float y = 0.299*rgb.r + 0.587*rgb.g + 0.114*rgb.b;\n"
"    return vec3(y, 0.493*(rgb.b-y), 0.877*(rgb.r-y));\n"
"}\n"
"vec3 yuv2rgb(vec3 yuv){\n"
"    float y = yuv.x;\n"
"    float u = yuv.y;\n"
"    float v = yuv.z;   \n"
"    return vec3(\n"
"        y + 1.0/0.877*v,\n"
"        y - 0.39393*u - 0.58081*v,\n"
"        y + 1.0/0.493*u\n"
"    );\n"
"}\n"
#endif
    "/* based on code by lalaoopybee https://www.shadertoy.com/view/DlfSz8 */\n"
    "#define CURVATURE 8.2\n"
    "#define BLUR .01\n"
    "#define CA_AMT 1.0024\n"
    "void fxShader(out vec4 fragColor, in vec2 uv){\n"
    "    vec2 fragCoord=uv*vec2(352.0*3.0,288.0*3.0);\n"
#if 1
    "    /* curvature */\n"
    "    vec2 crtUV=uv*2.-1.;\n"
    "    vec2 offset=crtUV.yx/CURVATURE;\n"
    "    crtUV+=crtUV*offset*offset;\n"
    "    crtUV=crtUV*.5+.5;\n"

    "    /* edge blur */\n"
    "    vec2 edge=smoothstep(0., BLUR, crtUV)*(1.-smoothstep(1.-BLUR, 1., crtUV));\n"
#else
    "    vec2 crtUV = uv;\n"
    "    vec2 edge = vec2(1.,1.);\n"
#endif

#if 1
    "    /* chromatic aberration */\n"
    "    vec3 color = vec3(\n"
    "        texture(image, (crtUV-.5)*CA_AMT+.5).r,\n"
    "        texture(image, crtUV).g,\n"
    "        texture(image, (crtUV-.5)/CA_AMT+.5).b\n"
    "    );\n"
#endif

#if 1
    "    /* tv refresh line*/\n"
    "    float tvline = mod(parameters.x / -12.5, 1) - uv.y; // params.x @ 50hz, make it x12.5 slower so it's less distracting; neg sign for upwards dir\n"
    "    if(tvline > 0 && tvline < 0.03f) color.rgb -= tvline;\n"
#endif

    "    /* mix up */\n"
    "    fragColor.rgb = color * edge.x * edge.y;\n"

#if 0
    "    /* diodes */\n"
    "    if(mod(fragCoord.y, 2.)<1.) fragColor.rgb*=.95;\n"
    "    else if(mod(fragCoord.x, 3.)<1.) fragColor.rgb*=.95;\n"
    "    else fragColor*=1.05;\n"
#endif
    "}\n";

int load_shader(const char *filename) {
    char *data = readfile(filename, NULL);
    if(data) if(strstr(data, " fxShader")) return shader = strdup(data), 1; // @leak
    return 0;
}

void crt(int enable) {
    extern window *app;
    if( enable )
    tigrSetPostShader(app, shader, strlen(shader));
    else
    tigrSetPostShader(app, tigr_default_fx_gl_fs, strlen(tigr_default_fx_gl_fs));
}
