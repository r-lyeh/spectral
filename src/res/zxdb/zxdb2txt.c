#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SQLITE_C 1
#include "sqlite3.h"

#ifndef __thread
#define __thread __declspec(thread)
#endif

#ifndef do_once
#define do_once static int once = 1; for(;once;once=0)
#endif

#undef countof
#define countof(x) ((int)(sizeof(x) / sizeof(0[x])))

#include "../../3rd.h"
#include "../../sys_string.h"
#include "sys_db.h"
#include "../../sys_network.h"
#include "../../sys_sleep.h"

#include "zx_db.h"

int main(int argc, char **argv) {
#if 0
    zxdb_free(zxdb_print(zxdb_search_by_name("myth*")));
    zxdb_free(zxdb_print(zxdb_search_by_name("tarzan")));
    zxdb_free(zxdb_print(zxdb_search_by_name("total recall")));
#endif
    if( argc > 1 ) {

        zxdb_init("ZXDB.sqlite");

        //
        for( FILE *fp = fopen(argv[1], "rb"); fp; fclose(fp), fp = 0 ) {
            zxdb_free(zxdb_print(zxdb_search_by_filename(argv[1])));
        }

        /**/ if( strstr(argv[1], "..") ) { // range of ids 0..65535
            uint64_t t = -time_ns();

            for( int i = atoi(argv[1]), end = atoi(strstr(argv[1],"..")+2), pct_ = (end - i) / 100, pct = pct_ + !pct_; i < end; ++i ) {
                if( !(i % pct) ) fprintf(stderr, "\r%d%%", (int)(i / pct) + 1);
                zxdb_free(zxdb_print(zxdb_search_by_id(i)));
            }

            t += time_ns();
            fprintf(stderr, "%dm%ds\n", (int)(t / 1e9)/60, (int)(t / 1e9)%60);
        }
        else if( argv[1][0] == '/' ) {
            for( FILE *out = fopen(strrchr(argv[1],'/')+1, "a+b"); out; fclose(out), out = 0) {
                int len; char *data = zxdb_download(argv[1], &len);
                fwrite(data, len, 1, out);
            }
        }
        else { // #number, "*text*search*", or "/file.ext"
            zxdb_free(zxdb_print(zxdb_search(argv[1])));
        }

        exit(0);
    }
    return -1;
}

// # tests
// 1942 Mission|5898                    #
// 1943|9298                            # release
// Fort Apache                          #
// Apaches|26375                        #
// Fort Apache|12624                    #
// 11-A-Side|9296                       # compilation
// Benny Bunny|24323                    # type-in v1
// Danger Dynamite|36221                # magref
// Perigo!|39393                        # type-in v2

// select *,(select score from scores where entry_id = E.id) as score from entries E order by score desc limit 10;
