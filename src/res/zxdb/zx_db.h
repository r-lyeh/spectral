// header

typedef struct zxdb {
    char *id;           // "#id|year|title|alias|publisher|max_players|score|genre"
    char *author[16];   // "@role author(team)"
    char *download[64]; // "/file_link|release_seq|filetype_id|filetype"
} zxdb;

zxdb zxdb_search(const char *entry); // either "#id", "*text*search*", or "/file.ext"
zxdb  zxdb_print(const zxdb z);
char* zxdb_pickurl(const zxdb z, const char *hint); // @todo:, unsigned release_seq);
char* zxdb_download(const char *url, int *len);
void zxdb_free(struct zxdb z);

// impl
#include <string.h>

zxdb zxdb_print(const zxdb z) {
    z.id && z.id[0] && puts(z.id);
    for( int i = 0; z.author[i]; ++i) printf("%s", z.author[i]); z.author[0] && puts("");
    for( int i = 0; z.download[i]; ++i) puts(z.download[i]);
    return z;
}

void zxdb_free(struct zxdb z) {
    free(z.id);
    for(int i = 0; z.author[i]; ++i) free(z.author[i]);
    for(int i = 0; z.download[i]; ++i) free(z.download[i]);
}

char* zxdb_pickurl(const zxdb z, const char *hint) {
    /**/ if( strstri(hint, "runn") ) hint = "|0|2|";
    else if( strstri(hint, "screen") ) hint = "|0|1|";
    else if( strstri(hint, "inlay") && strstri(hint, "side")) hint = "|0|4|";
    else if( strstri(hint, "inlay") && strstri(hint, "back")) hint = "|0|6|";
    else if( strstri(hint, "inlay")) hint = "|0|5|";
//  else if( strstri(hint, "tape")) hint = "|0|22|";
    else if( strstri(hint, "tape")) hint = "|0|8|";
    else if( strstri(hint, "snap")) hint = "|0|10|";
    else if( strstri(hint, "disk")) hint = "|0|11|";
    else if( strstri(hint, "rom")) hint = "|0|17|";
    else if( strstri(hint, "scanned")) hint = "|0|29|";
    else if( strstri(hint, "instr")) hint = "|0|28|";
    else if( strstri(hint, "overlay")) hint = "|0|30|";
    else if( strstri(hint, "map")) hint = "|0|31|";
    else if( strstri(hint, "comic")) hint = "|0|59|";
    else if( strstri(hint, "rzx")) hint = "|0|63|";
    else if( strstri(hint, "pok")) hint = "|0|74|";

    for( int i = 0; z.download[i]; ++i ) {
        if( strstri(z.download[i], hint) ) {
            char *url = va("%.*s", strchr(z.download[i],'|') - z.download[i], z.download[i]);
            return url;
        }
    }

    return NULL;
}

char* zxdb_download(const char *url, int *len) {
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

    // extract (year)(publisher)
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

    // convert non-alpha into spaces (@fixme: utf8 games; russian? spanish? czech?)
    for( int i = 0; s[i]; ++i ) s[i] = isalnum(s[i]) ? s[i] : '*';

    return s;
}

void zxdb_add_id(zxdb *z, const char *id) {
    z->id = strdup(id);
}

void zxdb_add_authors(zxdb *z, const char **authors) {
    for( ; authors; authors = 0) {
        for( int i = 0; authors[i][0]; ++i)
        {
            if( i >= countof(z->author) ) continue;

            z->author[i] = malloc(strlen(authors[i])+3);
            sprintf(z->author[i]+0, "@%c%s)", authors[i][0] == '|' ? '?' : authors[i][0], authors[i]+2-(authors[i][0] == '|'));
            char *div = strchr(z->author[i], '|');
            div[0] = div[1] == ')' ? '\0' : '(';
        }
    }
}

void zxdb_add_downloads(zxdb *z, const char **downloads) {
    for( int i = 0, j = 0; downloads[i][0]; ++i)
    {
        if( j >= countof(z->download) ) continue;

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
        z->download[j++] = strdup(downloads[i] + 6 * !strncmp(downloads[i], "http:/", 6) + 7 * !strncmp(downloads[i], "https:/", 7) );
    }
}

void zxdb_init(const char *dbfile) {
    do_once db_open(dbfile), atexit(db_close_);
            //db_query("pragma journal_mode = WAL;"),
            //db_query("pragma synchronous = normal;"),
            //db_query("pragma journal_size_limit = 6144000;");

    // db_print(db_query("select * from aliases limit 1;"));
    // db_print(db_query("select * from entries where title like 'myth%' limit 1;"));
    // db_print(db_query(sql_findgame(argc > 1 ? argv[1] : "myth*", 1)));
}

zxdb zxdb_search_by_expr(const char *expr) {
    // create _tags temp table
    do_once
        db_query("create temp table _tags0 as select entry_id,(select name from tags where id = M.tag_id) as tag from members M;"),
        db_query("create temp table _tags as select entry_id, concat('#',group_concat(tag,'#')) as tags from ( select entry_id, tag from _tags0 order by entry_id, tag) group by entry_id;");

    // returns query "#id|year|title|alias|publisher|max_players|score|genre|tags"
    // sorted by descending score, so first pick in list is likely the right one (see: multiple Eliminator titles)
    #define ZXDB_SEARCH \
    "select id,min(9999, " \
    "   ( " \
    "   select coalesce(release_year,9999) from releases where entry_id = E.id and releases.release_seq = 0 " \
    "   UNION " \
    "   select coalesce(release_year,9999) from downloads where entry_id = E.id and downloads.release_seq = 0 " \
    "   UNION " \
    "   select coalesce(release_year,9999) from releases where entry_id = (select container_id from contents where entry_id = E.id) " \
    "   UNION " \
    "   select coalesce(release_year,9999) from releases where entry_id = (select book_id from booktypeins where entry_id = E.id) " \
    "   UNION " \
    "   select coalesce(date_year,9999) from issues where id = (select issue_id from magrefs where entry_id = E.id) " \
    "   ) " \
    ") as year,title,(select title from aliases where entry_id = E.id) as alias," \
    "(select name from labels where id = (select label_id from publishers where release_seq = 0 and entry_id = E.id limit 1)) as publisher," \
    "concat(availabletype_id,replace(is_xrated,'1','X'),machinetype_id,',',(select text from machinetypes where id = E.machinetype_id)) as type," \
    "(select score from scores where entry_id = E.id) as score," \
    "concat(genretype_id,(select text from genretypes where id = E.genretype_id)) as genre," \
    "(select tags from _tags where entry_id=E.id) as tags " \
    "FROM entries E WHERE " \
    "(machinetype_id <= 10 OR machinetype_id = 14) AND " /*16,16/48,48,48/128,128,+2,+3,+2a/+3,+2b or pentagon*/ \
    "(genretype_id <= 32 OR (genretype_id >= 72 AND genretype_id <= 78)) AND " /*games or demos*/ \
    "%s ORDER BY score DESC LIMIT %d;", \
        expr, 1

    // returns query "/file_link|release_seq|filetype_id|filetype". exclude 38 original artwork, 40 additional materials
    #define ZXDB_DOWNLOADS \
    "select file_link,release_seq,filetype_id,(select text from filetypes where id = D.filetype_id) as filetype from downloads D " \
    " where entry_id=%d AND filetype_id<>38 AND filetype_id<>40 order by filetype_id,release_seq limit 63;", atoi(z.id)

    // returns query "@role|author|team"
    #define ZXDB_AUTHORS \
    "select "\
    " (select roletype_id from roles where label_id=E.label_id AND entry_id=E.entry_id) as role," \
    " (select name from labels where id=E.label_id) as author,"\
    " (select name from labels where id=E.team_id) as team from authors E where entry_id=%d limit 15;", atoi(z.id)

//    "tag_id|entry_id|variant|category_id|member_seq|tag"
//    "select *,(select name from tags where id = E.tag_id) as tag from members E where entry_id = 5525 limit 10;"
//     SELECT Column1, group_concat(Column2) FROM Table GROUP BY Column1

    zxdb z = {0};

    // puts(va(ZXDB_SEARCH));
    for( char **games = db_query(va(ZXDB_SEARCH)); games; games = 0 ) {
        zxdb_add_id(&z, games[0]);

        char **authors = db_query(va(ZXDB_AUTHORS));
        zxdb_add_authors(&z, authors);

        char **downloads = db_query(va(ZXDB_DOWNLOADS));
        zxdb_add_downloads(&z, downloads);
    }

    return z;
}
zxdb zxdb_search_by_id(unsigned id) {
    return zxdb_search_by_expr( va("id=%u", id) );
}
zxdb zxdb_search_by_name(const char *title) { // game title can use '*' wildcards
    // convert wildcards to sql format
    title = (const char *)replace(va("title LIKE '%s'", title), "*", "%");

    // regular search
    zxdb z = zxdb_search_by_expr( title );

    // search in aliases table alternatively
    if( !z.id || !z.id[0] )
    for( char **games = db_query(va("select entry_id from aliases where %s limit 1;", title)); games; games = 0 ) {
        if( games[0][0] ) return zxdb_search_by_id(atoi(games[0]));
    }

    return z;
}
zxdb zxdb_search_by_filename(const char *filename) {
    char *s = zxdb_filename2title(filename);

    // search query
    zxdb z = zxdb_search_by_name( s );

    // if it fails, try again replacing trailing 2>"% II", 3>"% III", 4>"% IV", 5>"% V"
    // lines ago, we did pre-allocate room space for this patch.
    if( (!z.id || !z.id[0]) && strendi(s, "*2") ) strcpy((char*)strendi(s,"*2"),"* II"), z = zxdb_search_by_name(s);
    if( (!z.id || !z.id[0]) && strendi(s, "*3") ) strcpy((char*)strendi(s,"*3"),"* III"), z = zxdb_search_by_name(s);
    if( (!z.id || !z.id[0]) && strendi(s, "*4") ) strcpy((char*)strendi(s,"*4"),"* IV"), z = zxdb_search_by_name(s);
    if( (!z.id || !z.id[0]) && strendi(s, "*5") ) strcpy((char*)strendi(s,"*5"),"* V"), z = zxdb_search_by_name(s);
    return z;
}

zxdb zxdb_search(const char *entry) { // generic interface: either "#id", "*text*search*", or "/file.ext"
    /**/ if( entry[0] == '#' && strspn(entry+1, "0123456789") == strlen(entry+1) ) {
        return zxdb_search_by_id(atoi(entry+1));
    }
    else if( strchr(entry, '*') ) {
        return zxdb_search_by_name(entry);
    }
    else {
        return zxdb_search_by_filename(entry);
    }
}
