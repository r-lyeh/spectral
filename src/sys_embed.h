// find Nth embedded media in executable
// medias could be a zxdb files or games

char *embedded(unsigned id, int *length) {
    static int binlen = 0;
    static char *bin = 0;
    if(!bin) bin = readfile(__argv[0], (int*)&binlen);
    if(!bin) die("cannot read argv[0]");

    // find num embedded medias.
    char watermark[] = "spectralEmBeDdEd";
    watermark[0] -= 32;

    int len = binlen;
    char *eof = bin + len;
    char *found = bin;

    while( found && len >= 16 ) {
        found = (char*)memmem(found, len, watermark, 16);
        if( found ) {
            found += 16;

            char *found2 = (char*)memmem(found, eof - found, watermark, 16);
            len = (found2 ? found2 - 1 : eof) - found;

            if( id-- == 0 ) {
                if( length ) *length = len;
                return len ? found : NULL;
            }

            found += len + 1;
            len = eof - found;
        }
    }

    return NULL;
}
