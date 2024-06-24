#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char **argv) {
#ifdef _WIN32
    if( argc != 3 ) puts("embed output.file \"@string\"");
#else
    if( argc != 3 ) puts("embed output.file \'@string\'");
#endif
    if( argc != 3 ) puts("embed output.file #hex_sequence");
    if( argc != 3 ) puts("embed output.file resource_file"), exit(1);

    int count = 0, ok = 0;

    if( argv[2][0] == '#' )
    for( FILE *out = fopen(argv[1], "a+b"); out; fclose(out), out = 0, ok = 1) {
        count = strlen(argv[2]+1) / 2;
        for( unsigned bytes = 0, hex = 0; bytes < count; ++bytes ) {
            ok = sscanf(argv[2]+1+bytes*2, "%02x", &hex) == 1;
            ok && printf("hex: %02x%c", hex, (bytes+1) < count ? ',' : '\n');
            ok && (fputc(hex & 255, out), hex >>= 8);
        }
    }
    else
    if( argv[2][0] == '@' )
    for( FILE *out = fopen(argv[1], "a+b"); out; fclose(out), out = 0) { 
        ok = fwrite(argv[2]+1, count = strlen(argv[2]+1), 1, out) == 1
            && printf("%d bytes string\n", count);
    }
    else
    for( FILE *in = fopen(argv[2], "rb"); in; fclose(in), in = 0 )
    for( FILE *out = fopen(argv[1], "a+b"); out; fclose(out), out = 0, ok = 1) { 
        int ch; do fputc(ch = fgetc(in), out), count += ch != EOF; while(ch != EOF);
        printf("%d bytes file\n", count);
    }

#if 0
    if( ok && count )
    for( FILE *out = fopen(argv[1], "a+b"); out; fclose(out), out = 0) { 
        fputc((count >>  0) & 255, out);
        fputc((count >>  8) & 255, out);
        fputc((count >> 16) & 255, out);
        fputc((count >> 24) & 255, out);
    }
#endif

    puts(ok ? "Ok" : "Error");
    exit(-!ok);
}
