// header

typedef struct zxdb {
    char *copy, *tok;
    char *ids[9];        // "#id|year|title|alias|publisher|type|score|genre|#tags"
    char *authors[16];   // "@role author(team)"
    char *downloads[64]; // "/file_link|release_seq|filetype_id|filetype"
} zxdb;

bool  zxdb_init(const char *zxdbfile);
bool  zxdb_initmem(const char *blob, const int len);
zxdb   zxdb_search(const char *entry); // either "#id", "*text*search*", or "/file.ext"
zxdb    zxdb_print(const zxdb);
char*   zxdb_url(const zxdb, const char *hint); // @todo:, unsigned release_seq);
zxdb    zxdb_free(zxdb);
char*  zxdb_download(const char *url, int *len); // must free() after use
int    zxdb_pathfile(const char *url, char *path, int pathlen); // constructs path to local file. returns true if file exists.
bool   zxdb_loaded();
int    zxdb_count();

// impl
#include <string.h>

struct map zxdb2;
char *zxdb_alloc;

int zxdb_count() {
    return map_count(&zxdb2);
}

bool zxdb_loaded() {
    return !!zxdb_alloc;
}

bool zxdb_init(const char *zxdbfile) {
    int len; char *blob = readfile(zxdbfile, &len);
    if( blob && len && !zxdb_initmem(blob, len) ) return free(blob), false;
    return true;
}

bool zxdb_initmem(const char *blob, const int len) {
    // allocate db
    if(zxdb_alloc) return false;
    if(!(blob && len)) return false;

    // ensure it's our gzipped database
    if( memcmp(blob + 0000, "\x1f\x8b\x08",3) ) return false;
    if( memcmp(blob + 0x0A, "Spectral.db",11) ) return false;

    // uncompress
    unsigned unclen;
    char *unc = gunzip(blob, len, &unclen);
    if( !unc ) { alert("cannot uncompress .db file"); return false; }

    // parse db. insert every entry
    for( char *ptr = unc; (ptr < (unc+unclen)) && *ptr; ) {
        char *entry = ptr;

        char *lf;
        for( ;; ) {
            lf = strchr(ptr, '\n');
            if(!lf) { ptr += strlen(ptr); break; } // go to eof
            ptr = ++lf;
            if(ptr >= (unc+unclen)) break;
            if(*ptr != '@' && *ptr != '/') break; // break at new entry
        }

        // inscribe entry, which goes from [entry,ptr)
        ptr[-1] = '\0';
        char *zxid;
        char *year  = strchr(entry, '|')+1; assert(year);  zxid  = va("#%.*s", year-entry-1, entry);
        char *title = strchr(year,  '|')+1; assert(title);
        char *alias = strchr(title, '|')+1; assert(alias); title = va("%.*s", alias-title-1, title);
        char *brand = strchr(alias, '|')+1; assert(brand); alias = va("%.*s", brand-alias-1, alias);
//      char *avail = strchr(brand, '|')+1; assert(avail);
//      char *score = strchr(avail, '|')+1; assert(score);
//      char *genre = strchr(score, '|')+1; assert(genre);
//      char *ttags = strchr(genre, '|')+1; assert(ttags);

        map_insert(&zxdb2, zxid, entry);
        map_insert(&zxdb2, title, entry);
        if( alias[0] ) map_insert(&zxdb2, alias, entry);

        char *roman = romanize(title);
        if( strcmp(roman, title) )
            map_insert(&zxdb2, roman, entry);
    }

    printf("%d ZXDB entries\n", map_count(&zxdb2));

    // do not free(). keep pointers alive
    zxdb_alloc = unc;
    return true;
}

static
void zxdb_add_ids(zxdb *z, char **ids) {
    for( int i = 0, j = 0; ids[i]; ++i) {
        j = i;
        if( j >= countof(z->ids) ) continue;
        z->ids[j++] = ids[i];
    }
}

static
void zxdb_add_authors(zxdb *z, char **authors) {
    for( int i = 0, j = 0; authors[i]; ++i) {
        if( j >= countof(z->authors) ) continue;
        z->authors[j++] = authors[i];
    }
}

static
void zxdb_add_downloads(zxdb *z, char **downloads) {
    for( int i = 0, j = 0; downloads[i]; ++i) {
        if( j >= countof(z->downloads) ) continue;

        // .headers on; select * from filetypes ORDER BY id;
        const char accepted_types[] = {
            1,  // Loading screen
            2,  // Running screen
            4,  // Inlay - Side
            5,  // Inlay - Front
            6,  // Inlay - Back
            7,  // Media scan
            8,  // Tape image
            10, // Snapshot image
            11, // Disk image
            17, // Computer/ZX Interface 2 cartridge ROM image dump
            18, // DOCK cartridge ROM image dump
            22, // BUGFIX tape image
            27, // Bonus soundtrack(s) in MP3 format
            28, // Instructions
            29, // Scanned instructions
            30, // Keyboard overlay
            31, // Game map
            33, // Code sheet
            59, // Comic
            63, // RZX playback file
            67, // ZX Interface 2 cartridge box scan
            74, // POK pokes file
            0,
        };

        // exclude types we are not likely going to ever use.
        char download_type = atoi(strchr(strchr(downloads[i],'|')+1,'|')+1);
        if( !strchr(accepted_types, download_type) ) continue;

        // exclude .pdf instructions. i dont really want to support this right now
        char *ext = strrchr(downloads[i], '.');
        if( ext && !memcmp(ext, ".pdf", 4) ) continue;

        // add download
        z->downloads[j++] = (downloads[i] + 6 * !strncmp(downloads[i], "http:/", 6) + 7 * !strncmp(downloads[i], "https:/", 7) );
    }
}


static
int zxdb_compare(const void *arg1, const void *arg2) {
    // the zxdb database is sorted by game id (#), there are no collisions there.
    // however, we use title and alias strings for our lookups. there are like 2,000
    // conflicting names in the database, among titles and aliases. here we try to choose
    // which conflicting part should be kept. see: barbarian, eliminator, etc.
    //
    // for now, using the `brand vs brandless` heuristic and the `higher score` one.
    //
    // other heuristics we could use:
    // recent vs old
    // known brand vs unknown brand

    char **a = (char**)*(VAL**)arg1; char *entry = *a;
    char **b = (char**)*(VAL**)arg2; char *other = *b;

    char *year1  = strchr(entry,  '|')+1;
    char *title1 = strchr(year1,  '|')+1;
    char *alias1 = strchr(title1, '|')+1;
    char *brand1 = strchr(alias1, '|')+1;
    char *avail1 = strchr(brand1, '|')+1;
    char *score1 = strchr(avail1, '|')+1;
    char *genre1 = strchr(score1, '|')+1;
    char *ttags1 = strchr(genre1, '|')+1;

    char *year2  = strchr(other,  '|')+1;
    char *title2 = strchr(year2,  '|')+1;
    char *alias2 = strchr(title2, '|')+1;
    char *brand2 = strchr(alias2, '|')+1;
    char *avail2 = strchr(brand2, '|')+1;
    char *score2 = strchr(avail2, '|')+1;
    char *genre2 = strchr(score2, '|')+1;
    char *ttags2 = strchr(genre2, '|')+1;

    if( !!brand1[0] ^ !!brand2[0] ) // brand vs brandless: prefer brand
        return brand2[0] - brand1[0];

    if( atof(score2) - atof(score1) ) // score: prefer higher
        return (int)(atof(score2) - atof(score1) * 100);

    return 0;
}

static
zxdb zxdb_search_by_name(const char *name) {
    if(name[0] != '#') puts(name);

    zxdb z = {0};

    VAL *found = 0;

    // search query
    int matches;
    VAL **multi = map_multifind(&zxdb2, name, &matches);
    if( matches ) {
        qsort(multi, matches, sizeof(VAL*), zxdb_compare);
#if DEV
        if( matches > 1 ) ; // alert(va("%d matches for `%s`", matches, name));
        if( matches > 1 )
        for( int i = 0; i < matches; ++i ) {
            puts(*multi[i]);
        }
#endif
        found = multi[0];
    }

    // process query
    if( found ) {
        z.copy = strdup(*found);
        z.tok = strdup(*found);

        char *lines[128] = {0}, **line = lines, *sep;

        // split tok into lines. ignore entry strings (\r\n)
        sep = z.tok;
        for( char *ptr = strsep(&sep, "\r\n" ); ptr; ptr = strsep(&sep, "\r\n") ) {
            if( ptr[0] ) *line++ = ptr;
        }

        // inscribe download lines [2..]
        if( (lines[1] && lines[1][0] == '/') || (lines[1] && lines[2] && lines[2][0] == '/') )
        zxdb_add_downloads(&z, &lines[1 + (lines[1][0] == '@')]); // pick line [1] or [2] depending on line[1] being an @authors line

        // inscribe authors' line [1]. ignore entry strings (\r\n)
        if( lines[1] && lines[1][0] == '@' ) {
            char **authors = line;
            sep = lines[1];
            for( char *ptr = strsep(&sep, "@\r\n" ); ptr; ptr = strsep(&sep, "@\r\n" ) ) {
                if( ptr[0] ) *line++ = ptr;
            }
            zxdb_add_authors(&z, authors);
        }

        // inscribe main line [0]. include empty strings (1983||Manic|)
        {
            char **ids = line;
            sep = lines[0];
            for( char *ptr = strsep(&sep, "|\r\n" ); ptr; ptr = strsep(&sep, "|\r\n" ) ) {
                *line++ = ptr;
            }
            zxdb_add_ids(&z, ids);
        }
    }

    free(multi);
    return z;
}

zxdb zxdb_print(const zxdb z) {
    z.copy && puts(z.copy);
    return z;
}

zxdb zxdb_free(zxdb z) {
    zxdb zero = {0};
    free(z.copy);
    free(z.tok);
    return zero;
}

static char *zxdb_filename2title(const char *filename) {
    // find basename
    const char *win = strrchr(filename, '\\');
    const char *unx = strrchr(filename, '/');

    // extract basename
    char *s = va("%s", win > unx ? win + 1 : unx > win ? unx + 1 : filename);

    // remove extensions
    // @todo: filter DB search by extension. ie, search exclusively for +3 disks if .dsk extension is found
    char *ext;
    do {
        ext = strlen(s) > 4 ? s + strlen(s) - 4 : 0;
        /**/ if( ext && ext[0] == '.' && !strchr(ext+1, '.') ) ext[0] = '\0'; // .rar .zip .tap .tzx etc
        else if( ext && ext[1] == '.' && !strchr(ext+1, '.') ) ext[1] = '\0'; // .gz
        else ext = 0;
    } while(ext);

    // extract (year)(publisher)(side 1)[48-128K]
    if( strchr(s, '(') ) *strchr(s, '(') = '\0';

    // convert case edges into spaces (@fixme: utf8 games; russian? spanish? czech?)
    // there x2 room for worst case: aBcDeF>a*B*c*D*e*F, and also extra room for further string patches
    char *spaced = va("%*.s", strlen(s)*2+5, "");
    for( int i = 0, j = 0; s[i]; ++i ) {
        if( i > 1 ) {
            int upper = islower(s[i-1]) && isupper(s[i]);
            int digit = isalpha(s[i-1]) && isdigit(s[i]);
            if( upper || digit ) spaced[j++] = '*';
        }
        spaced[j++] = s[i];
    }
    s = spaced;

    // trim lead/final spaces and double spaces
    replace(s, " - ", "   ");
    replace(s, "_-_", "   ");
    while( s[0] == ' ' ) ++s;
    while( s[strlen(s)-1] == ' ' ) s[strlen(s)-1] = '\0';
    while( strstr(s, "  ") ) replace(s, "  ", " ");

    // convert to uppercase (@fixme: utf8 games; russian? spanish? czech?)
    for( int i = 0; s[i]; ++i ) s[i] = toupper(s[i]);

    // reorder prefixes written as suffixes
    /**/ if( strendi(s, ", THE") ) memmove(s+4,s,strlen(s+4)), memcpy(s, "THE ", 4), s[strlen(s)-1] = '\0';
    else if( strendi(s, ", LOS") ) memmove(s+4,s,strlen(s+4)), memcpy(s, "LOS ", 4), s[strlen(s)-1] = '\0';
    else if( strendi(s, ", LAS") ) memmove(s+4,s,strlen(s+4)), memcpy(s, "LAS ", 4), s[strlen(s)-1] = '\0';
    else if( strendi(s, ", EL")  ) memmove(s+3,s,strlen(s+3)), memcpy(s, "EL ", 3), s[strlen(s)-1] = '\0';
    else if( strendi(s, ", LA")  ) memmove(s+3,s,strlen(s+3)), memcpy(s, "LA ", 3), s[strlen(s)-1] = '\0';
    else if( strendi(s, ", IL")  ) memmove(s+3,s,strlen(s+3)), memcpy(s, "IL ", 3), s[strlen(s)-1] = '\0';
    else if( strendi(s, ", A")   ) memmove(s+2,s,strlen(s+2)), memcpy(s, "A ", 2), s[strlen(s)-1] = '\0';
    else if( strendi(s, ", O")   ) memmove(s+2,s,strlen(s+2)), memcpy(s, "O ", 2), s[strlen(s)-1] = '\0';

    // remove: gonzzalezz - side 1, Toi Acid Game - Side 2
    // remove: Outrun - Tape 2 - Side 1, 5_Exitos_De_Opera_Soft_Tape_1_-_Side_1,
    const char *tags[] = {
        " TAPE 1",
        " TAPE 2",
        " PART 1",
        " PART 2",
        " PART II",
        " SIDE 1",
        " SIDE 2",
        " SIDE A",
        " SIDE B",
        " SIDEA-V1",
        " SIDEA-V2",
        " SIDEB-V1",
        " SIDEB-V2",
        " RELEASE 1",
        " RELEASE 2",
        " RELEASE 3",
        " SMALL CASE",
        " SMALL CARDBOARD CASE",
        " MEDIUM CASE",
        " BUGFIX",
        " DEMONSTRATOR",
        " STANDARD",
        " EXPERT",
        0
    };
    for( int i = 0; tags[i]; ++i ) {
        if( strstr(s, tags[i]) ) {
            strstr(s, tags[i])[0] = '\0';
        }
    }

    // convert non-alpha into spaces (@fixme: utf8 games; russian? spanish? czech?)
    for( int i = 0; s[i]; ++i ) s[i] = isalnum(s[i]) ? s[i] : '*';

    // final touch, remove 48 and 128 from filenames: renegade128.tzx, rasputin48.sna, etc
    if( strstr(s, "*48K") ) strstr(s, "*48K")[1] = '\0'; // memcpy(strstr(s, "*48K"), "****",  4);
    if( strstr(s,"*128K") ) strstr(s,"*128K")[1] = '\0'; // memcpy(strstr(s,"*128K"), "*****", 5);
    if( strstr(s,  "*48") ) strstr(s,  "*48")[1] = '\0'; // memcpy(strstr(s,  "*48"), "***",   3);
    if( strstr(s, "*128") ) strstr(s, "*128")[1] = '\0'; // memcpy(strstr(s, "*128"), "****",  4);

    return s;
}

zxdb zxdb_search(const char *id) { // game.tap or #13372
    // search by id
    if( id[0] == '#' ) return zxdb_search_by_name(id);

    // search by filename
    char *s = zxdb_filename2title(id);
    zxdb z = zxdb_search_by_name( s );

    // if it fails, try again replacing trailing 2>"% II", 3>"% III", 4>"% IV", 5>"% V"
    // lines ago, we did pre-allocate room space for this patch.
    if( !z.ids[0] && strstri(s,   "*2") ) strcpy((char*)strstri(s,  "*2"), "* II*"), z = zxdb_search_by_name(s);
    if( !z.ids[0] && strstri(s,   "*3") ) strcpy((char*)strstri(s,  "*3"),"* III*"), z = zxdb_search_by_name(s);
    if( !z.ids[0] && strstri(s,   "*4") ) strcpy((char*)strstri(s,  "*4"), "* IV*"), z = zxdb_search_by_name(s);
    if( !z.ids[0] && strstri(s,   "*5") ) strcpy((char*)strstri(s,  "*5"),  "* V*"), z = zxdb_search_by_name(s);
    if( !z.ids[0] && strstri(s, "*III") ) strcpy((char*)strstri(s,"*III"),  "* 3*"), z = zxdb_search_by_name(s);
    if( !z.ids[0] && strstri(s,  "*II") ) strcpy((char*)strstri(s, "*II"),  "* 2*"), z = zxdb_search_by_name(s);
    if( !z.ids[0] && strstri(s,  "*IV") ) strcpy((char*)strstri(s, "*IV"),  "* 4*"), z = zxdb_search_by_name(s);
    if( !z.ids[0] && strstri(s,   "*V") ) strcpy((char*)strstri(s,  "*V"),  "* 5*"), z = zxdb_search_by_name(s);
    return z;
}

char* zxdb_url(const zxdb z, const char *hint) {

    if( strstri(hint, "play") ) {
        char *media = 0;
        if(!media) media = zxdb_url(z, "disk");
        if(!media) media = zxdb_url(z, "snap");
        if(!media) media = zxdb_url(z, "rom");
        if(!media) media = zxdb_url(z, "bugfix");
        if(!media) media = zxdb_url(z, "tape");
        return media;
    }

    int check_media = 0;

    /**/ if( strstri(hint, "runn") ) hint = "|2|R"; // .scr, .gif, .png, .jpg, .mc/.mlt, .ifl (multicolor8x2 9216 = 6144+768*4)
    else if( strstri(hint, "screen") ) hint = "|1|L"; // .scr, .gif, .png, .jpg, .mc/.mlt (multicolor8x1 12288 = 6144+768*8)
    else if( strstri(hint, "inlay") && strstri(hint, "side")) hint = "|4|I";
    else if( strstri(hint, "inlay") && strstri(hint, "back")) hint = "|6|I";
    else if( strstri(hint, "inlay")) hint = "|5|I";
    else if( strstri(hint, "bugfix")) hint = "|22|B", check_media = 1;
    else if( strstri(hint, "tape")) hint = "|8|T", check_media = 1;
    else if( strstri(hint, "snap")) hint = "|10|S", check_media = 1;
    else if( strstri(hint, "disk")) hint = "|11|D", check_media = 1;
    else if( strstri(hint, "rom")) hint = "|17|C", check_media = 1;
    else if( strstri(hint, "mp3")) hint = "|27|B";
    else if( strstri(hint, "scanned")) hint = "|29|S";
    else if( strstri(hint, "instr")) hint = "|28|I";
    else if( strstri(hint, "overlay")) hint = "|30|K"; // @todo: addme
    else if( strstri(hint, "map")) hint = "|31|G";
    else if( strstri(hint, "comic")) hint = "|59|C";
    else if( strstri(hint, "rzx")) hint = "|63|R";
    else if( strstri(hint, "pok")) hint = "|74|P";

    for( int i = 0; z.downloads[i]; ++i ) {
        if( strstri(z.downloads[i], hint) ) {
            char *url = va("%.*s", strchr(z.downloads[i],'|') - z.downloads[i], z.downloads[i]);
            if( strstri(url, ".szx") || strstri(url, ".slt") ) continue;
            return url;
        }
    }

    return NULL;
}

static
char* zxdb_download_(const char *url, int *len) {
    if( !url ) return 0;

    if( url[0] == '/' ) {
        const char *mirror = 0;
        /**/ if( !strncmp(url, "/nvg/", 5) ) {
            mirror = "https://archive.org/download/mirror-ftp-nvg/Mirror_ftp_nvg.zip/";
            url += 5;
        }
        else if( !strncmp(url, "/pub/", 5) ) {
            // ok, redirection
            mirror = "https://archive.org/download/World_of_Spectrum_June_2017_Mirror/World%20of%20Spectrum%20June%202017%20Mirror.zip/World%20of%20Spectrum%20June%202017%20Mirror/";
            url += 5;
        }
        else if( !strncmp(url, "/zxdb/", 6) ) {
            // ok
            mirror = "https://spectrumcomputing.co.uk/";
            url += 1;
        }
        else {
            // try https:// as a fallback in case it is a full url
            mirror = "https://";
            url += 1;
        }
        return mirror ? download(va("%s%s", mirror, url), len) : 0;
    }
    // puts(url);
    return download(url, len);
}

int zxdb_pathfile(const char *url, char *path, int pathlen) {
    if( !url ) return 0;

    // @fixme: this depends on a perfect hash function right now. so far, fnv1a is a pass; crc32 is not.
    uint64_t hash = fnv1a(url, strlen(url));
    uint32_t hash1 = hash >> 32ULL;
    uint32_t hash2 =(hash << 32ULL) >> 32ULL;

    do_once mkdir(".Spectral", 0777);
    snprintf(path, pathlen, ".Spectral/%02X", hash1 >> 24); mkdir(path, 0777);
    snprintf(path, pathlen, ".Spectral/%02X/%06X", hash1 >> 24, (hash1 << 8) >> 8); mkdir(path, 0777);
    snprintf(path, pathlen, ".Spectral/%02X/%06X/%08X", hash1 >> 24, (hash1 << 8) >> 8, hash2);

    return is_file(path);
}

char* zxdb_download(const char *url, int *len) {
    if( !url ) return NULL;

    char path[128];
    zxdb_pathfile(url, path, 128);

    char *cache = readfile(path, len);
    if( !cache ) {
        cache = zxdb_download_(url, len);
        if( cache ) {
            writefile(path, cache, *len);
        }
    }
    return cache;
}
