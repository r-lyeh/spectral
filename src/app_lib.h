rgba* thumbnail(byte *VRAM_, int len, unsigned downfactor) {
    int w = 256 / downfactor, h = 192 / downfactor;

    rgba *texture = malloc( w * h * 4 ), *cpy = texture;
    if( len != 6912 ) return texture; // @fixme: .ifl/.mlt

    #define SCANLINE_(y) \
        ((((((y)%64) & 0x38) >> 3 | (((y)%64) & 0x07) << 3) + ((y)/64) * 64) << 5)

    for( int y = 0; y < 192; y += downfactor ) {
        // paper
        byte *pixels=VRAM_+SCANLINE_(y);
        byte *attribs=VRAM_+6144+((y&0xF8)<<2);
        rgba *bak = texture;

        for(int x = 0; x < 32; ++x ) {
            byte attr = *attribs;
            byte pixel = *pixels, fg, bg;

            // @fixme: make section branchless

            pixel ^= (attr & 0x80) && ZXFlashFlag ? 0xff : 0x00;
            fg = (attr & 0x07) | ((attr & 0x40) >> 3);
            bg = (attr & 0x78) >> 3;

            if( downfactor == 1 ) {
            texture[0]=ZXPalette[pixel & 0x80 ? fg : bg];
            texture[1]=ZXPalette[pixel & 0x40 ? fg : bg];
            texture[2]=ZXPalette[pixel & 0x20 ? fg : bg];
            texture[3]=ZXPalette[pixel & 0x10 ? fg : bg];
            texture[4]=ZXPalette[pixel & 0x08 ? fg : bg];
            texture[5]=ZXPalette[pixel & 0x04 ? fg : bg];
            texture[6]=ZXPalette[pixel & 0x02 ? fg : bg];
            texture[7]=ZXPalette[pixel & 0x01 ? fg : bg];
            texture += 8;
            }
            else if( downfactor == 2 ) {
            texture[0]=ZXPalette[pixel & 0x80 ? fg : bg];
            texture[1]=ZXPalette[pixel & 0x20 ? fg : bg];
            texture[2]=ZXPalette[pixel & 0x08 ? fg : bg];
            texture[3]=ZXPalette[pixel & 0x02 ? fg : bg];
            texture += 4;
            }
            else if( downfactor == 4 ) {
            texture[0]=ZXPalette[pixel & 0x80 ? fg : bg];
            texture[1]=ZXPalette[pixel & 0x08 ? fg : bg];
            texture += 2;
            }
            else if( downfactor == 8 ) {
            texture[0]=ZXPalette[pixel & 0x80 ? fg : bg];
            texture += 1;
            }

            pixels++;
            attribs++;
        }

        texture = bak + w;
    }

    return cpy;
}



static
const char *tab;

static
int zxdb_compare_by_name(const void *arg1, const void *arg2) { // @fixme: roman
    char **a = (char**)*(VAL**)arg1; char *entry = *a;
    char **b = (char**)*(VAL**)arg2; char *other = *b;

    char *year1  = strchr(entry,  '|')+1;
    char *title1 = strchr(year1,  '|')+1;
    char *alias1 = strchr(title1, '|')+1;

    char *year2  = strchr(other,  '|')+1;
    char *title2 = strchr(year2,  '|')+1;
    char *alias2 = strchr(title2, '|')+1;

    if( *tab == '#' ) {
        if( *alias1 != '|' && (isdigit(*alias1) || ispunct(*alias1)) ) title1 = alias1;
        if( *alias2 != '|' && (isdigit(*alias2) || ispunct(*alias2)) ) title2 = alias2;
    } else {
        if( *title1 != *tab && *alias1 == *tab ) title1 = alias1;
        if( *title2 != *tab && *alias2 == *tab ) title2 = alias2;
    }

    return strcmpi(title1, title2);
}

char *zxdb_screen(const char *id, int *len) {
    if( id && id[0] && strcmp(id, "0") && strcmp(id, "#") ) {
        ZXDB = zxdb_search( id );

        if( ZXDB.ids[0] ) {
            static char *data = 0;
            if( data ) free(data), data = 0;
            if(!data ) data = zxdb_download(zxdb_url(ZXDB, "screen"), len);
            if(!data ) data = zxdb_download(zxdb_url(ZXDB, "running"), len);
            return data;
        }
    }
    return NULL;
}

bool zxdb_load(const char *id) {
    if( id && id[0] && strcmp(id, "0") && strcmp(id, "#") ) {
        ZXDB = zxdb_search( id );

        if( ZXDB.ids[0] ) {
            int len;

            static char *data = 0;
            if( data ) free(data), data = 0;
            if(!data ) data = zxdb_download(zxdb_url(ZXDB, "play"), &len);
            if( data ) {
        #if 0
                loadbin(data, len, true);
        #else
                // this temp file is a hack for now. @fixme: move the zip/rar/fdi loaders into loadbin()
                for( FILE *fp = fopen("spectral.$$2", "wb"); fp; fwrite(data, len, 1, fp), fclose(fp), fp = 0) {
                }
                loadfile("spectral.$$2", 1);
                unlink("spectral.$$2");
        #endif
            }

            return true; // @fixme: verify that previous step went right
        }
    }
    return false;
}







#pragma pack(push, 1)
typedef struct cache_t {
    uint16_t likes : 4;
    uint16_t flags : 4;
    uint16_t reserved : 8;
} cache_t;
#pragma pack(pop)

typedef int static_assert_cache_t[sizeof(cache_t) == 2];

enum {
    CACHE_TRANSFER = 2, // if download is in progress

    CACHE_MP3 = 1,
    CACHE_TXT = 2,
    CACHE_POK = 4,
    CACHE_SCR = 8,
    CACHE_SNA = 16,
    CACHE_JPG = 32,
    CACHE_MAP = 64,
};

uint16_t *cache;

void cache_load() {
    if(!cache) {
        cache = calloc(2, 65536); // zxdb_count());
        for( FILE *fp = fopen(".Spectral/Spectral.db", "rb"); fp; fclose(fp), fp = 0) {
            fread(cache, 2 * 65536, 1, fp);
        }
    }
}
void cache_save() {
    for( FILE *fp = fopen(".Spectral/Spectral.db", "wb"); fp; fclose(fp), fp = 0) {
        fwrite(cache, 2 * 65536, 1, fp);
    }
}
uint16_t cache_get(unsigned zxdb) {
    cache_load();
    return cache[zxdb];
}
uint16_t cache_set(unsigned zxdb, uint16_t v) {
    cache_load();
    int changed = cache[zxdb] ^ v;
    cache[zxdb] = v;
    if( changed ) cache_save();
    return v;
}

byte* screens[65536][4]; // as-is, 2:1 shrink, 4:1 shrink, 8:1 shrink
unsigned short screens_len[65536];


thread_ptr_t worker;
thread_queue_t queue;
void *queue_values[65536];
int worker_fn( void* userdata ) {
    for(;;) {
        void* item = thread_queue_consume(&queue, THREAD_QUEUE_WAIT_INFINITE);
        char* str = (char*)item;
        printf("recv %s\n", str);

        int id = atoi(str), len = 0;
        if( screens_len[id] == 1 ) {
            str = strchr(str, '|')+1;
            str = zxdb_download(str, &len);
            if( str && len ) {
                screens[id][0] = str; // as-is
                screens[id][1] = str;
                screens[id][2] = str;
                screens[id][3] = str;

                for( int i = 1; i < 4; ++i ) {
                    int factors[] = { 1,2,4,8 }, factor = factors[i];
                    int f256 = 256/factor, f192 = 192/factor;

                    rgba *bitmap = ui_image(str,len,f256,f192,1);
                    if( bitmap ) {
                        screens[id][i] = (byte*)bitmap;
                    } else {
                        bitmap = thumbnail(str,len,1);
                        screens[id][i] = (byte*)ui_resize(bitmap,256,192,f256,f192,1);
                        free(bitmap);
                    }
                }

                screens_len[id] = len;
            }
        }

        free(item);
    }
    return 0;
}
const char *worker_push(const char *url) {
    // init
    if(!worker) thread_queue_init(&queue, sizeof(queue_values) / sizeof(queue_values[0]), queue_values, 0);
    if(!worker) thread_detach( worker = thread_init(worker_fn, NULL, "worker_fn", THREAD_STACK_SIZE_DEFAULT) );
    // 
    printf("sent %s\n", url);
    thread_queue_produce(&queue, strdup(url), THREAD_QUEUE_WAIT_INFINITE );
    return url;
}


char *zxdb_screen_async(const char *id, int *len, int factor) {
    if( id && id[0] && strcmp(id, "0") && strcmp(id, "#") ) {
        ZXDB = zxdb_search( id );

        if( ZXDB.ids[0] ) {
            int zxdb_id = atoi(ZXDB.ids[0]);
            if( screens_len[zxdb_id] == 0 ) {
                screens_len[zxdb_id] = 1;
                worker_push(va("%d|%s", zxdb_id, zxdb_url(ZXDB, "screen")));
                worker_push(va("%d|%s", zxdb_id, zxdb_url(ZXDB, "running")));
            }
            if( screens_len[zxdb_id] > 1 ) {
                return *len = screens_len[zxdb_id], screens[zxdb_id][factor & 3];
            }
        }
    }
    return NULL;
}









int active; // @todo: rename to browser_active, or library_active

extern Tigr *app, *ui;
extern char *last_load;

char **games;
int *dbgames;
int numgames;
int numok,numwarn,numerr; // stats
void rescan(const char *folder) {
    if(!folder) return;
    if(ZX_PLAYER) return; // zxplayer has no library

    // clean up
    while( numgames ) free(games[--numgames]);
    games = realloc(games, 0);

    // refresh stats
    {
        numok=0,numwarn=0,numerr=0;

        for( dir *d = dir_open(folder, "r"); d; dir_close(d), d = NULL ) {
            for( unsigned count = 0, end = dir_count(d); count < end; ++count ) {
                if( !dir_file(d, count) ) continue;

                const char *fname = dir_name(d, count);
                if( strendi(fname, ".db") ) {
                    for(FILE *fp2 = fopen(fname, "rb"); fp2; fclose(fp2), fp2=0) {
                        int ch;
                        fscanf(fp2, "%d", &ch); ch &= 0xFF;
                        numok += ch == 1;
                        numerr += ch == 2;
                        numwarn += ch == 3;
                    }
                }
            }
            for( unsigned count = 0, end = dir_count(d); count < end; ++count ) {
                if( !dir_file(d, count) ) continue;

                const char *fname = dir_name(d, count);
                if( file_is_supported(fname,ALL_FILES) ) {
                    // append
                    ++numgames;
                    games = realloc(games, numgames * sizeof(char*) );
                    games[numgames-1] = strdup(fname);
                    //
                    dbgames = realloc(dbgames, numgames * sizeof(char*) );
                    dbgames[numgames-1] = db_get(fname);
                }
            }
        }
    }

    printf("%d games\n", numgames);
}
void draw_compatibility_stats(window *layer) {
    // compatibility stats
    int total = numok+numwarn+numerr;
    if(total && active) {
    TPixel white = {255,255,255,255}, black = {0,0,0,255}, *bar = &ui->pix[0 + _239 * _320];
    int num1 = (numok * (float)_319) / total;
    int num2 = (numwarn * (float)_319) / total;
    int num3 = (numerr * (float)_319) / total; if((num1+num2+num3)<_319) num1 += _319 - (num1+num2+num3);
    for( int x = 0; x <= num1; ++x ) bar[x-320]=bar[x] = tigrRGB(64,255,64);
    for( int x = 0; x <= num2; ++x ) bar[x+num1-320]=bar[x+num1] = tigrRGB(255,192,64);
    for( int x = 0; x <= num3; ++x ) bar[x+num1+num2-320]=bar[x+num1+num2] = tigrRGB(255,64,64);
    static char compat[64];
    snprintf(compat, 64, "  OK:%04.1f%%     ENTER:128, +SHIFT:48, +CTRL:Try turbo", (total-numerr) * 100.f / (total+!total));
    window_printxy(layer, compat, 0,(_240-12.0)/11);
    }
}

int selected, scroll;
char* game_browser_v1() {
//  tigrBlitTint(app, app, 0,0, 0,0, _320,_240, tigrRGB(128,128,128));

    enum { ENTRIES = (_240/11)-4 };
    static char *buffer = 0; if(!buffer) { buffer = malloc(65536); /*rescan();*/ }
    if (!numgames) return 0;
    if( scroll < 0 ) scroll = 0;
    for( int i = scroll; i < numgames && i < scroll+ENTRIES; ++i ) {
        const char starred = dbgames[i] >> 8 ? (char)(dbgames[i] >> 8) : ' ';
        sprintf(buffer, "%c %3d.%s%s\n", starred, i+1, i == selected ? " > ":" ", 1+strrchr(games[i], DIR_SEP) );
        window_printxycol(ui, buffer, 1, 3+(i-scroll-1),
            (dbgames[i] & 0x7F) == 0 ? tigrRGB(255,255,255) : // untested
            (dbgames[i] & 0x7F) == 1 ? tigrRGB(64,255,64) :   // ok
            (dbgames[i] & 0x7F) == 2 ? tigrRGB(255,64,64) : tigrRGB(255,192,64) ); // bug:warn
    }

    int up = 0, pg = 0;

    static int UPcnt = 0; UPcnt *= !!window_pressed(app, TK_UP);
    if( window_pressed(app, TK_UP) && (UPcnt++ == 0 || UPcnt > 32) ) {
        up = -1;
    } UPcnt *= !!window_pressed(app, TK_UP);
    static int DNcnt = 0; DNcnt *= !!window_pressed(app, TK_DOWN);
    if( window_pressed(app, TK_DOWN) && (DNcnt++ == 0 || DNcnt > 32) ) {
        up = +1;
    } DNcnt *= !!window_pressed(app, TK_DOWN);
    static int PGUPcnt = 0; PGUPcnt *= !!window_pressed(app, TK_PAGEUP);
    if( window_pressed(app, TK_PAGEUP) && (PGUPcnt++ == 0 || PGUPcnt > 32) ) {
        pg = -1;
    } PGUPcnt *= !!window_pressed(app, TK_PAGEUP);
    static int PGDNcnt = 0; PGDNcnt *= !!window_pressed(app, TK_PAGEDN);
    if( window_pressed(app, TK_PAGEDN) && (PGDNcnt++ == 0 || PGDNcnt > 32) ) {
        pg = +1;
    } PGDNcnt *= !!window_pressed(app, TK_PAGEDN);

    // issue browser
    if( window_trigger(app, TK_LEFT)  ) { for(--up; (selected+up) >= 0 && (dbgames[selected+up]&0xFF) <= 1; --up ) ; }
    if( window_trigger(app, TK_RIGHT) ) { for(++up; (selected+up) < numgames && (dbgames[selected+up]&0xFF) <= 1; ++up ) ; }

    for(;up < 0;++up) {
        --selected;
        if( selected < scroll ) --scroll;
    }
    for(;up > 0;--up) {
        ++selected;
        if( selected >= (scroll+ENTRIES) ) ++scroll;
    }
    for(;pg < 0;++pg) {
        if( selected != scroll ) selected = scroll;
        else scroll -= ENTRIES, selected -= ENTRIES;
    }
    for(;pg > 0;--pg) {
        if( selected != scroll+ENTRIES-1 ) selected = scroll+ENTRIES-1;
        else scroll += ENTRIES, selected += ENTRIES;
    }

    scroll = scroll < 0 ? 0 : scroll >= numgames - ENTRIES ? numgames-ENTRIES-1 : scroll;
    selected = selected < scroll ? scroll : selected >= (scroll + ENTRIES + 1) ? scroll + ENTRIES : selected;
    selected = selected < 0 ? 0 : selected >= numgames ? numgames-1 : selected;


        static int chars[16] = {0}, chars_count = -1;
        #define RESET_INPUTBOX() do { memset(chars, 0, sizeof(int)*16); chars_count = -1; } while(0)
        int any = 0;
        // Grab any chars and add them to our buffer.
        for(;;) {
            int c = tigrReadChar(app);
            if (c == 0) break;
            if( window_pressed(app,TK_CONTROL)) break;
            if( c == 8 ) { RESET_INPUTBOX(); break; } // memset(chars, 0, sizeof(int)*16); chars_count = -1; break; }
            if( c == '\t' && chars_count > 0 ) { any = 1; break; }
            if( c <= 32 ) continue;
            else any = 1;
            chars[ chars_count = min(chars_count+1, 15) ] = c;
        }
        // Print out the character buffer too.
        char tmp[1+16*6], *p = tmp;
        for (int n=0;n<16;n++)
            p = tigrEncodeUTF8(p, chars[n]);
        *p = 0;
        char tmp2[16+16*6] = "Find:"; strcat(tmp2, tmp);
        window_printxycol(ui, tmp2, 3,1, tigrRGB(0,192,255));
        if( any ) {
            static char lowercase[1024];
            for(int i = 0; tmp[i]; ++i) tmp[i] |= 32;
            int found = 0;
            if(!found)
            for( int i = scroll+1; i < numgames; ++i ) {
                if (i < 0) continue;
                for(int j = 0; games[i][j]; ++j) lowercase[j+1] = 0, lowercase[j] = games[i][j] | 32;
                if( strstr(lowercase, tmp) ) {
                    scroll = selected = i;
                    found = 1;
                    break;
                }
            }
            if(!found)
            for( int i = 0; i < scroll; ++i ) {
                for(int j = 0; games[i][j]; ++j) lowercase[j+1] = 0, lowercase[j] = games[i][j] | 32;
                if( strstr(lowercase, tmp) ) {
                    scroll = selected = i;
                    found = 1;
                    break;
                }
            }
        }

    if( window_pressed(app, TK_CONTROL) || window_trigger(app, TK_SPACE) ) {
        int update = 0;
        int starred = dbgames[selected] >> 8;
        int color = dbgames[selected] & 0xFF;
        if( window_trigger(app, TK_SPACE) ) color = (color+1) % 4, update = 1;
        if( window_trigger(app, 'D') ) starred = starred != 'D' ? 'D' : 0, update = 1; // disk error
        if( window_trigger(app, 'T') ) starred = starred != 'T' ? 'T' : 0, update = 1; // tape error
        if( window_trigger(app, 'I') ) starred = starred != 'I' ? 'I' : 0, update = 1; // i/o ports error
        if( window_trigger(app, 'R') ) starred = starred != 'R' ? 'R' : 0, update = 1; // rom/bios error
        if( window_trigger(app, 'E') ) starred = starred != 'E' ? 'E' : 0, update = 1; // emulation error
        if( window_trigger(app, 'Z') ) starred = starred != 'Z' ? 'Z' : 0, update = 1; // zip error
        if( window_trigger(app, 'S') ) starred = starred != 'S' ? 'S' : 0, update = 1; // star
        if( window_trigger(app, '3') ) starred = starred != '3' ? '3' : 0, update = 1; // +3 only error
        if( window_trigger(app, '4') ) starred = starred != '4' ? '4' : 0, update = 1; // 48K only error
        if( window_trigger(app, '1') ) starred = starred != '1' ? '1' : 0, update = 1; // 128K only error
        if( window_trigger(app, '0') ) starred = starred != '0' ? '0' : 0, update = 1; // USR0 only error
        if( window_trigger(app, 'A') ) starred = starred != 'A' ? 'A' : 0, update = 1; // ay/audio error
        if( window_trigger(app, 'V') ) starred = starred != 'V' ? 'V' : 0, update = 1; // video/vram error
        if( window_trigger(app, 'H') ) starred = starred != 'H' ? 'H' : 0, update = 1; // hardware error
        if( window_trigger(app, 'M') ) starred = starred != 'M' ? 'M' : 0, update = 1; // mem/multiload error
        if(update) {
            dbgames[selected] = color + (starred << 8);
            db_set(games[selected], dbgames[selected]);
        }
    }

    if( window_trigger(app, TK_RETURN) ) {
        RESET_INPUTBOX();
        return games[selected];
    }

    return NULL;
}



char* game_browser_v2() {
    // decay to local file browser if no ZXDB is present
    if( !zxdb_loaded() ) return ZX_BROWSER = 1, NULL;

#if 0
    if (!numgames) return 0;
    if( scroll < 0 ) scroll = 0;
#endif

    // handle input
    struct mouse m = mouse();
    int up = window_keyrepeat(app, TK_UP);
    int down = window_keyrepeat(app, TK_DOWN);
    int left = window_keyrepeat(app, TK_LEFT);
    int right = window_keyrepeat(app, TK_RIGHT);
    int page_up = window_keyrepeat(app,TK_PAGEUP);
    int page_down = window_keyrepeat(app,TK_PAGEDN);

    // constants

    const int LINE_HEIGHT = 11;
    const int UPPER_SPACING = 2;
    const int BOTTOM_SPACING = (DEV ? 5 : 2) * LINE_HEIGHT;

    // upper tabs

    ui_at(ui, -6+2+5+2, UPPER_SPACING);

    static int page = 0;
    static int thumbnails = 0; // 0 text, 3 (3x3), 6 (6x6), 12 (12x12)
    static char *notify = 0;

//  static const char *tab = 0;
    static const char *tabs = "#ABCDEFGHIJKLMNOPQRSTUVWXYZ\x12\x18\x17\x19";
    for(int i = 0; tabs[i]; ++i) {
        if( (ui_at(ui, ui_x+5, ui_y), ui_click(NULL, va("%c%c", (tab && tabs[i] == *tab) ? 5 : 7, tabs[i])) ))
            tab = tabs + i;
    }

    static VAL **list = 0;
    static int list_num = 0;

    if( left )  if(!tab) tab = tabs; else if(*tab-- == '#')    tab = tabs + 27;
    if( right ) if(!tab) tab = tabs; else if(*tab++ == '\x12') tab = tabs + 00;

    static const char *prev = 0;
    if( tab && prev != tab ) {

        if( *tab == '\x18' ) {
            if( zxdb_load(prompt("Game title or ZXDB #ID", "Game title or ZXDB #ID", "0")) ) {
                active = 0;

                tab = 0;
                prev = 0;
                //list = 0;
                //list_num = 0;
                return NULL;
            }
        }
        else
        if( *tab == '\x17' ) {
            extern int cmdkey;
            cmdkey = 'SCAN';

            ZX_BROWSER = 1; // decay to file browser

            tab = 0;
            prev = 0;
            //list = 0;
            //list_num = 0;
            return NULL;
        }
        else
        if( *tab == '\x19' ) {
            int next[] = { [0]=3,[3]=6,[6]=12,[12]=0 };

            thumbnails = next[thumbnails];

            tab = prev;
            prev = prev;
            //list = 0;
            //list_num = 0;
            //return NULL;
        }
        else {
            page = 0;
        }

        // search & sort
        free(list), list = 0;
        if( *tab != '\x12' ) {
            list = map_multifind(&zxdb2, va("%c*", *tab == '#' ? '?' : *tab), &list_num);
        }
        else {
            // bookmarks
            list_num = 0;
            list = realloc(list, 65535 * sizeof(VAL*));
            for( int i = 0; i < 65535; ++i) {
                if( cache_get(i) & 0x0f ) {
                    list[ list_num++ ] = map_find(&zxdb2, va("#%d", i));
                }
            }
            // list = realloc(list, list_num * sizeof(VAL*));
        }
        if( list_num ) qsort(list, list_num, sizeof(VAL*), zxdb_compare_by_name);

        // remove dupes (like aliases)
        for( int i = 1; i < list_num; ++i ) {
            if( *(char**)list[i-1] == *(char**)list[i] ) {
                memmove(list + i - 1, list + i, ( list_num - i ) * sizeof(list[0]));
                --list_num;
            }
        }

        // exclude XXX games
        // exclude For(S)ale,(N)everReleased,Dupes(*),MIA(?) [include (A)vailable,(R)ecovered,(D)enied games]
        // exclude demos 72..78
        for( int i = 0; i < list_num; ++i ) {
            char *zx_id = (char*)*list[i];
            char *years = strchr(zx_id, '|')+1; int zx_id_len = years-zx_id-1;
            char *title = strchr(years, '|')+1; int years_len = title-years-1;
            char *alias = strchr(title, '|')+1; int title_len = alias-title-1;
            char *brand = strchr(alias, '|')+1; int alias_len = brand-alias-1;
            char *avail = strchr(brand, '|')+1; int brand_len = avail-brand-1;
            char *score = strchr(avail, '|')+1; int avail_len = score-avail-1;
            char *genre = strchr(score, '|')+1; int score_len = genre-score-1;
            char *tags_ = strchr(genre, '|')+1; int genre_len = tags_-genre-1;

            if( avail[1] == 'X' || !strchr("ARD", avail[0]) || atoi(genre) >= 72 ) {
                memmove(list + i, list + i + 1, ( list_num - i - 1 ) * sizeof(list[0]));
                --list_num;
                --i;
            }
        }

        prev = tab;
    }

    if( notify ) free(notify), notify = 0;

    // main content

    if( !tab ) return NULL;

    ui_at(ui, 0, UPPER_SPACING+2*LINE_HEIGHT);

    int ENTRIES_PER_PAGE = (_240-ui_y-BOTTOM_SPACING)/LINE_HEIGHT;

    if( thumbnails == 3 ) ENTRIES_PER_PAGE = 3*3;
    if( thumbnails == 6 ) ENTRIES_PER_PAGE = 6*6;
    if( thumbnails == 12 ) ENTRIES_PER_PAGE = 12*12;

    int NUM_PAGES = list_num / ENTRIES_PER_PAGE;
    int trailing_page = list_num % ENTRIES_PER_PAGE;
    NUM_PAGES -= NUM_PAGES && !trailing_page;

    if( page > NUM_PAGES ) page = NUM_PAGES - 1;
    if( page < 0 ) page = 0;

    if( up || page_up )     if(--page < 0) page = 0;
    if( down || page_down ) if(++page >= NUM_PAGES) page = NUM_PAGES;

    static byte frame4 = 0; frame4 = (frame4 + 1) & 3; // 4-frame anim
    static byte frame8 = 0; frame8 = (frame8 + 1) & 7; // 8-frame anim

    if( list )
    for( int len, cnt = 0, i = page * ENTRIES_PER_PAGE, end = (page + 1) * ENTRIES_PER_PAGE;
        i < end && i < list_num; ++i, ++cnt ) {

        char *zx_id = (char*)*list[i];
        char *years = strchr(zx_id, '|')+1; int zx_id_len = years-zx_id-1;
        char *title = strchr(years, '|')+1; int years_len = title-years-1;
        char *alias = strchr(title, '|')+1; int title_len = alias-title-1;
        char *brand = strchr(alias, '|')+1; int alias_len = brand-alias-1;
        char *avail = strchr(brand, '|')+1; int brand_len = avail-brand-1;
        char *score = strchr(avail, '|')+1; int avail_len = score-avail-1;
        char *genre = strchr(score, '|')+1; int score_len = genre-score-1;
        char *tags_ = strchr(genre, '|')+1; int genre_len = tags_-genre-1;

        // replace title if alias is what we're looking for
        if( *tab == '#' ) {
        if( *alias != '|' && (isdigit(*alias) || ispunct(*alias)) ) title = alias, title_len = alias_len;
        } else {
        if( title[0] != *tab && alias[0] == *tab ) title = alias, title_len = alias_len;
        }

        // replace year if title was never released
        if( years[0] == '9' ) years = "?", years_len = 1; // "9999"

        // replace brand if no brand is given. use 1st author if possible
        if( brand[0] == '|' ) {
            char *next = strchr(zx_id, '\n');
            if( next && next[1] == '@' ) { // x3 skips: '\n' + '@' + 'R'ole
                brand = next+1+1+1, brand_len = strcspn(brand, "@\r\n");
            }
        }

        // stars, user-score
        const char *stars[] = {
            /*"\2"*/"\f\x10\f\x10\f\x10", // 0 0 0
            /*"\2"*/"\f\x11\f\x10\f\x10", // 0 0 1
            /*"\2"*/"\f\x12\f\x10\f\x10", // 0 1 0
            /*"\2"*/"\f\x12\f\x11\f\x10", // 0 1 1
            /*"\2"*/"\f\x12\f\x12\f\x10", // 1 0 0
            /*"\2"*/"\f\x12\f\x12\f\x11", // 1 0 1
            /*"\2"*/"\f\x12\f\x12\f\x12", // 1 1 0
            /*"\2"*/"\f\x12\f\x12\f\x12", // 1 1 1
        };
        static const char *colors = "\7\2\6\4";
        int dbid = atoi(zx_id);
        int vars = cache_get(dbid);
        int star = (vars >> 0) & 0x0f; assert(star <= 3);
        int flag = (vars >> 4) & 0x0f; assert(flag <= 3);

        // build title and clean it up
        char full_title[128];
        snprintf(full_title, sizeof(full_title), " %.*s (%.*s)(%.*s)\n", title_len, title, years_len, years, brand_len, brand);
        for( int i = 1; full_title[i]; ++i )
            if( i == 1 || full_title[i-1] == '.' )
                full_title[i] = toupper(full_title[i]);

        full_title[0] = colors[flag];

        if( !thumbnails ) {

            ui_label(va("  %3d. ", i+1));

            if( ui_click("-Likes-", va("%c\f", "\x10\x12"[!!star])) )
                star = !star;

            if( ui_click("-Flags-", va("%c%s", colors[flag], flag == 0 || flag == 3 ? "":"")) )
                flag = (flag + 1) % 4;

            cache_set(dbid, (vars & 0xff00) | (flag << 4) | star);

            ui_label(" ");

            ui_monospaced = 0;
            if( ui_button(NULL, full_title) ) {
                
                if( ui_hover )
                for( char *data = zxdb_screen_async(va("#%.*s", zx_id_len, zx_id), &len, 0); data; data = 0 ) {
                    // loadbin(data, len, false);
                    if( len == 6912 ) {
                        memcpy(VRAM, data, len);
                    }
                }

                if( ui_click ) {
                    zxdb_load(va("#%.*s", zx_id_len, zx_id));

                    active = 0;

#if 0
        //          tab = 0;
                    prev = 0;
                    list = 0;
                    list_num = 0;
#endif
                    return NULL;
                }
            }
        }
        else {
            int t = thumbnails;
            int w = _320/t, h = (_240-16)/t;
            int x = (cnt % t) * w, y = 16 + (cnt / t) * h;
            int clicked = 0;
            int has_thumb = 0;

            int factor = t == 3 ? 1 : t == 6 ? 2 : 3;
            char *data = zxdb_screen_async(va("#%.*s", zx_id_len, zx_id), &len, factor);
            if( data && len ) {
                // blit
                rgba *bitmap = (rgba*)data;
                int f256 = 256/(1<<factor), f192 = 192/(1<<factor);
                for(int i = 0; i < f192; ++i) {
                    memcpy(&ui->pix[x+(y+i)*_320], bitmap + (0+i*f256), f256*4);
                }

                has_thumb = 1;
            }

            if( m.x >= x && m.x < (x+w) && m.y >= y && m.y < (y+h) ) {
                ui_rect(ui, x,y, x+w-1,y+h-1);

                ui_at(ui, x+2, y+2);

                if( ui_click(NULL, va("%c\f", "\x10\x12"[!!star])) )
                    star = !star;

                if( ui_click(NULL, va("%c%s", colors[flag], flag == 0 || flag == 3 ? "":"")) )
                    flag = (flag + 1) % 4;

                cache_set(dbid, (vars & 0xff00) | (flag << 4) | star);

                if( m.x <= (x+10) && m.y <= (y+11) ) {
                    notify = strdup("-Likes-");
                }
                else
                if( m.x <= (x+10+10-4) && m.y <= (y+11) ) {
                    notify = strdup("-Flags-");
                }
                else {
                    mouse_cursor(2);
                    notify = strdup(full_title);
                    clicked = m.lb;
                }
            }
            else {
                ui_at(ui, x, y);

                if( !has_thumb ) {
                    const char *anims8[] = {
                        "","","","",
                        "","","","",
                    };
                    const char *anims4[] = {
                        "","",
                        "","",
                    };
                    const char *id = va("%.*s\n%s", zx_id_len, zx_id, anims4[ (atoi(zx_id) + frame4) & 3 ] );
                    if( ui_click(id, id) ) clicked = 1;
                }
            }

            if( clicked ) {
                zxdb_load(va("#%.*s", zx_id_len, zx_id));

                active = 0;

#if 0
    //          tab = 0;
                prev = 0;
                list = 0;
                list_num = 0;
#endif
                return NULL;
            }
        }
    }

    if( notify ) ui_notify(notify);

    return NULL;
}
