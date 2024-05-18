// ttf2mono font converter v0.2, dirty hack :)
// r-lyeh, public domain

#include <stdint.h>
#include <stdio.h>

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <map>

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"

const char* codepoint_to_utf8(unsigned c) { //< @r-lyeh
    static char s[4+1];
    memset(s, 0, 5);
    /**/ if (c <     0x80) s[0] = c, s[1] = 0;
    else if (c <    0x800) s[0] = 0xC0 | ((c >>  6) & 0x1F), s[1] = 0x80 | ( c        & 0x3F), s[2] = 0;
    else if (c <  0x10000) s[0] = 0xE0 | ((c >> 12) & 0x0F), s[1] = 0x80 | ((c >>  6) & 0x3F), s[2] = 0x80 | ( c        & 0x3F), s[3] = 0;
    else if (c < 0x110000) s[0] = 0xF0 | ((c >> 18) & 0x07), s[1] = 0x80 | ((c >> 12) & 0x3F), s[2] = 0x80 | ((c >>  6) & 0x3F), s[3] = 0x80 | (c & 0x3F), s[4] = 0;
    return s;
}

char ttf_buffer[1<<25];
stbtt_fontinfo font;

int main(int argc, char **argv)
{
    if( argc < 2 ) {
        std::cout << "usage: " << argv[0] << " font.ttf [MAX height in px (8)]" << std::endl;
        return -1;
    }

    std::string filename = argv[1];
    std::string title = filename;
    title = title.substr( title.find_last_of('/') + 1 );
    title = title.substr( title.find_last_of('\\') + 1 );
    title = title.substr( 0, title.find_last_of('.') );

    FILE *fp = fopen(filename.c_str(), "rb");
    fread(ttf_buffer, 1, 1<<25, fp);
    fclose(fp);

    // max pixels per char (defaults to 8)
    int MAX = argc >= 3 ? atoi(argv[2]) : 8;
    if( MAX <= 4 ) return -1;

    stbtt_InitFont(&font, (const unsigned char *)ttf_buffer, stbtt_GetFontOffsetForIndex((const unsigned char *)ttf_buffer,0));

    auto mk_monospace = [&]( std::vector<int> &bitfont, std::vector<int> &toc, int c) {
        int w, h, xoff, yoff;
        unsigned char *bitmap = 0;

        // this is used to determinate if given 'c' is valid or invalid codepoint/unsupported typeface
        static const unsigned char *invalid = 0;
        static int size = 0;
        if( !invalid ) {
            int invalid_codepoint = 0xC0C1;
            bitmap = stbtt_GetCodepointBitmap(&font, 0,stbtt_ScaleForPixelHeight(&font, MAX), c, &w, &h, &xoff, &yoff);
            invalid = (unsigned char *)malloc( size = w * h );
            memcpy( (unsigned char *)invalid, bitmap, size );
            for( unsigned it = 0; it < size; ++it ) {
                // printf( &"\"\\x%02x%s"[ !!(it % w) ], invalid[it], &"\"},\n\0\"}};\n"[ (1+it) < size ? 4 * !!((1+it) % w) : 5 ] );
            }
        }

        bitmap = stbtt_GetCodepointBitmap(&font, 0,stbtt_ScaleForPixelHeight(&font, MAX), c, &w, &h, &xoff, &yoff);

        if( w*h == size && !memcmp(invalid, bitmap, size) ) {
            return 0;
        }

        toc.push_back( c );

        printf("%s", codepoint_to_utf8(c));

#if 1
--yoff; // fix descent final line in BESCII ttf
#endif

        std::vector<int> font;
        fprintf(stderr, "// u+%04x %d %d %d %d\n", c, w, h, xoff, yoff);
        for(int j = 0; j < MAX; ++j) {
            fprintf(stderr, "// ");
            font.push_back( 0 );
            if( j >= MAX + yoff && j < (MAX + yoff) + h ) {
                for( int i=0; i < MAX; ++i ) {
                    char ch = i >= xoff && i < xoff + w ? ".x"[(bitmap[(j-(MAX+yoff))*w+i-xoff]>>5) > 0] : '.';
                    font.back() |= (ch != '.') << (MAX - 1 - i);
                    fprintf(stderr, "%c", ch );
                }
            } else {
                fprintf(stderr, "%s", std::string(MAX, '.').c_str() );
            }
            fprintf(stderr, " 0x%02x [%d]\n", font.back(), c*MAX+j );
        }
        for( auto &it : font ) {
            bitfont.push_back( it );
        }
        return 1;
    };

    printf("/* generated by ttf2mono.cc v0.2;                         https://github.com/r-lyeh\n");
    printf("   %s.ttf specimen:\n", title.c_str());

    std::vector<int> bitfont, toc;
    for( uint32_t x = 0, count = 0; x <= 0xfffff; ++x ) {
        if( mk_monospace(bitfont, toc, x) ) {
            if( (count++ % 80) == 0 ) printf("\n   ");
        }
    }

    printf("\n*/\n");

    printf("const unsigned char %s_ttf[] =\n", title.c_str() );
    for( auto end = bitfont.size(), it = end - end; it < end; ++it ) {
        printf( &"\"\\x%02x%s"[ !!(it % 16) ], bitfont[it], &"\"\n\0\";\n"[ (1+it) < end ? 2 * !!((1+it) % 16) : 3 ] );
    }

    printf("const int %s_ttf_toc[] = {\n", title.c_str() );
    for( auto end = toc.size(), it = end - end; it < end; ++it ) {
        printf( "%6d%s", toc[it], &",\0,\n\0};\n"[ (1+it) >= end ? 5 : 2 * !((1+it) % 16) ] );
    }

    printf(
    "/*dichotomic binary search (container must be sorted && supporting sequential access); r-lyeh, public domain */\n"
    "static inline unsigned %s_toc_find( const int *begin, const int *end, int x ) {\n"
    "    unsigned min = 0, max = unsigned( (end - begin) / sizeof(int));\n"
    "    while( min < max ) {\n"
    "        unsigned mid = min + ( max - min ) / 2;\n"
    "        /**/ if( x == begin[mid] ) return mid;\n"
    "        else if( x < begin[mid] ) max = mid;\n"
    "        else min = mid + 1;\n"
    "    }\n"
    "    return ~0u;\n"
    "};\n", title.c_str());

    printf("\n");

    if( MAX > 8 ) return 0; // wont fit in 64bits below otherwise

    char buf[256];
    printf("const uint64_t %s_bit[][2] = {\n", title.c_str() );
    for( auto end = bitfont.size(), it = end - end; it < end; ++it ) {
        char *ptr = buf;
        if( 0 == (it%8) ) {
            ptr += sprintf(ptr, "{0x%04x,", toc[it/8] );
        }
        ptr += sprintf(ptr, &"0x%02x"[ 2 * !!(it % 8) ], bitfont[it] );
        if( (1+it) >= end ) {
            ptr += sprintf(ptr, "},\n{0,0}\n};\n" );
        } else {
            if( !((1+it) % 8) ) ptr += sprintf(ptr, "%s", "},");
            if( !((1+it) %32) ) ptr += sprintf(ptr, "%s", "\n");
        }
        printf( "%s", buf );
    }

    printf("%s", "\n");

    return 0;
}
