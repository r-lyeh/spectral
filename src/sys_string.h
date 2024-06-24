// string utilities,
// - rlyeh, public domain

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
char* va(const char *fmt, ...) {
    static char buf[32][1024+512];
    static int l = 0; l = (l+1) % 32;
    va_list vl;
    va_start(vl,fmt);
    int rc = vsnprintf(buf[l], 1024+512-1, fmt, vl);
    va_end(vl);
    buf[l][rc<0?0:rc] = 0;
    return buf[l];
}
#include <string.h>
char *replace( char *copy, const char *target, const char *replacement ) {
    // replaced only if new text is shorter than old one
    int rlen = strlen(replacement), diff = strlen(target) - rlen;
    if( diff < 0 ) return 0;
    for( char *s = copy, *e = s + strlen(copy); /*s < e &&*/ 0 != (s = strstr(s, target)); ) {
        if( rlen ) s = (char*)memcpy( s, replacement, rlen ) + rlen;
        if( diff ) memmove( s, s + diff, (e - (s + diff)) + 1 );
    }
    return copy;
}
const char *strstri(const char *a, const char *b) {
    char *A, *B, *M;
    for(char *p = (char*)(A = va("%s",a)); *p; ++p) *p = toupper(*p);
    for(char *p = (char*)(B = va("%s",b)); *p; ++p) *p = toupper(*p);
    return M = strstr(A, B), M ? a + (M - A) : NULL;
}
const char *strendi(const char *src, const char *sub) { // returns true if both strings match at end. case insensitive
    int srclen = strlen(src);
    int sublen = strlen(sub);
    if( sublen > srclen ) return 0;
    return !strcmpi(src + srclen - sublen, sub) ? src + srclen - sublen : NULL;
}
int qsort_strcmp(const void * a, const void * b ) {
    // smart strcmp which does:
    // Narc - 128k.tzx < Narc - 128k - Levels.tzx, and also:
    // Academy - Side 1.tzx < Academy - Side 2.tzx < Academy - Side 3.tzx < Academy - Side 4.tzx, and also:
    // Outrun - Tape 1 - Side 1.tzx < Outrun - Tape 2 - Side 1.tzx < Outrun - Tape 2 - Side 2.tzx
    const char *pa = *(const char**)a;
    const char *pb = *(const char**)b;
    int la = strlen(pa), lb = strlen(pb);
    if( la < lb ) return -1;
    if( la > lb ) return +1;
    return strcmp(pa,pb);
}

// find a mem blob in a mem section; similar to strstr()
#define memmem memmem2 // archlinux
const void *memmem(const void *block, size_t blocklen, const void * const bits, const size_t bitslen) {
    if((uintptr_t)block * blocklen * (uintptr_t)bits * bitslen)
    for (const char *h = block; blocklen >= bitslen; ++h, --blocklen) {
        if (!memcmp(h, bits, bitslen)) {
            return h;
        }
    }
    return NULL;
}

const void *memstr(const void *block, size_t blocklen, const char* str) {
    return memmem(block, blocklen, str, strlen(str));
}

// memset words instead of chars
void *memset32(void *dst, unsigned ch, int words) {
    unsigned *ptr = (unsigned*)dst;
    if( words > 0 )
    while( words-- ) {
        *ptr++ = ch;
    }
    return dst;
}

uint64_t fnv1a(const void* ptr_, unsigned len) {
    const uint8_t *ptr = (const uint8_t *)ptr_;
    uint64_t hash = 14695981039346656037ULL; // hash(0),mul(131) faster than fnv1a, a few more collisions though
    while( len-- ) hash = ( *ptr++ ^ hash ) * 0x100000001b3ULL;
    return hash;
}

unsigned crc32(unsigned h, const void *ptr_, unsigned len) {
    // based on public domain code by Karl Malbrain
    const uint8_t *ptr = (const uint8_t *)ptr_;
    if (!ptr) return 0;
    const unsigned tbl[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };
    for(h = ~h; len--; ) { uint8_t b = *ptr++; h = (h >> 4) ^ tbl[(h & 15) ^ (b & 15)]; h = (h >> 4) ^ tbl[(h & 15) ^ (b >> 4)]; }
    return ~h;
}

#ifdef _WIN32 // better than strtok(). preserves empty strings within delimiters
char *strsep(char **sp, const char *sep) {
    if( sp && *sp && **sp ) {
        char *p = *sp + strcspn(*sp, sep), *s = *sp;
        if( *p ) *p++ = '\0';
        *sp = p;
        return s;
    }
    return NULL;
}
#endif

#if 0
char** split(char *buffer, const char *separators) {
    static char **tokens = 0; // @fixme: slots
    tokens = realloc(tokens, strlen(buffer)/2 + 1);
    char **token = tokens;
    for( char *sep = buffer, *ptr = strsep(&sep, separators); ptr; ptr = strsep(&sep, separators) ) {
        *token++ = ptr;
    }
    *token++ = 0;
    return tokens;
}
#endif


const char *extract_utf32(const char *s, unsigned *out) {
    /**/ if( (s[0] & 0x80) == 0x00 ) return *out = (s[0]), s + 1;
    else if( (s[0] & 0xe0) == 0xc0 ) return *out = (s[0] & 31) <<  6 | (s[1] & 63), s + 2;
    else if( (s[0] & 0xf0) == 0xe0 ) return *out = (s[0] & 15) << 12 | (s[1] & 63) <<  6 | (s[2] & 63), s + 3;
    else if( (s[0] & 0xf8) == 0xf0 ) return *out = (s[0] &  7) << 18 | (s[1] & 63) << 12 | (s[2] & 63) << 6 | (s[3] & 63), s + 4;
    return *out = 0, s + 0;
}
unsigned codepoint(const char **s) {
    if( s && *s ) {
        unsigned glyph = 0;
        *s = extract_utf32(*s, &glyph);
        *s += !glyph;
        return glyph;
    }
    return 0;
}
const char* codepoint_to_utf8(unsigned c) { //< @r-lyeh
    static char s[4+1];
    memset(s, 0, 5);
    /**/ if (c <     0x80) s[0] = c, s[1] = 0;
    else if (c <    0x800) s[0] = 0xC0 | ((c >>  6) & 0x1F), s[1] = 0x80 | ( c        & 0x3F), s[2] = 0;
    else if (c <  0x10000) s[0] = 0xE0 | ((c >> 12) & 0x0F), s[1] = 0x80 | ((c >>  6) & 0x3F), s[2] = 0x80 | ( c        & 0x3F), s[3] = 0;
    else if (c < 0x110000) s[0] = 0xF0 | ((c >> 18) & 0x07), s[1] = 0x80 | ((c >> 12) & 0x3F), s[2] = 0x80 | ((c >>  6) & 0x3F), s[3] = 0x80 | (c & 0x3F), s[4] = 0;
    return s;
}

char *romanize(const char *s) {
    const int roman[] = {
        [0x00e0/*à*/]='a',[0x00c0/*À*/]='A',[0x00e1/*á*/]='a',[0x00c1/*Á*/]='A',[0x00e2/*â*/]='a',[0x00c2/*Â*/]='A',[0x00e3/*ã*/]='a',[0x00c3/*Ã*/]='A',[0x00e5/*å*/]='a',[0x00c5/*Å*/]='A',[0x0101/*ā*/]='a',[0x0100/*Ā*/]='A',[0x0103/*ă*/]='a',[0x0102/*Ă*/]='A',[0x0105/*ą*/]='a',[0x0104/*Ą*/]='A',
        [0x1e03/*ḃ*/]='b',[0x1e02/*Ḃ*/]='B',
        [0x00e7/*ç*/]='c',[0x00c7/*Ç*/]='C',[0x0107/*ć*/]='c',[0x0106/*Ć*/]='C',[0x0109/*ĉ*/]='c',[0x0108/*Ĉ*/]='C',[0x010b/*ċ*/]='c',[0x010a/*Ċ*/]='C',[0x010d/*č*/]='c',[0x010c/*Č*/]='C',
        [0x010f/*ď*/]='d',[0x010e/*Ď*/]='D',[0x0111/*đ*/]='d',[0x0110/*Đ*/]='D',[0x1e0b/*ḋ*/]='d',[0x1e0a/*Ḋ*/]='D',
        [0x00e8/*è*/]='e',[0x00c8/*È*/]='E',[0x00e9/*é*/]='e',[0x00c9/*É*/]='E',[0x00ea/*ê*/]='e',[0x00ca/*Ê*/]='E',[0x00eb/*ë*/]='e',[0x00cb/*Ë*/]='E',[0x0113/*ē*/]='e',[0x0112/*Ē*/]='E',[0x0114/*Ĕ*/]='E',[0x0115/*ĕ*/]='e',[0x0117/*ė*/]='e',[0x0116/*Ė*/]='E',[0x0119/*ę*/]='e',[0x0118/*Ę*/]='E',[0x011b/*ě*/]='e',[0x011a/*Ě*/]='E',
        [0x0191/*Ƒ*/]='F',[0x1e1f/*ḟ*/]='f',[0x1e1e/*Ḟ*/]='F',[0x0192/*ƒ*/]='f',
        [0x011d/*ĝ*/]='g',[0x011c/*Ĝ*/]='G',[0x011f/*ğ*/]='g',[0x011e/*Ğ*/]='G',[0x0121/*ġ*/]='g',[0x0120/*Ġ*/]='G',[0x0123/*ģ*/]='g',[0x0122/*Ģ*/]='G',
        [0x0125/*ĥ*/]='h',[0x0124/*Ĥ*/]='H',[0x0127/*ħ*/]='h',[0x0126/*Ħ*/]='H',
        [0x00ec/*ì*/]='i',[0x00cc/*Ì*/]='I',[0x00ed/*í*/]='i',[0x00cd/*Í*/]='I',[0x00ee/*î*/]='i',[0x00ce/*Î*/]='I',[0x00ef/*ï*/]='i',[0x00cf/*Ï*/]='I',[0x0129/*ĩ*/]='i',[0x0128/*Ĩ*/]='I',[0x012b/*ī*/]='i',[0x012a/*Ī*/]='I',[0x012f/*į*/]='i',[0x012e/*Į*/]='I',
        [0x0135/*ĵ*/]='j',[0x0134/*Ĵ*/]='J',
        [0x0137/*ķ*/]='k',[0x0136/*Ķ*/]='K',
        [0x013a/*ĺ*/]='l',[0x0139/*Ĺ*/]='L',[0x013c/*ļ*/]='l',[0x013b/*Ļ*/]='L',[0x013e/*ľ*/]='l',[0x013d/*Ľ*/]='L',[0x0142/*ł*/]='l',[0x0141/*Ł*/]='L',
        [0x1e41/*ṁ*/]='m',[0x1e40/*Ṁ*/]='M',
        [0x00f1/*ñ*/]='n',[0x00d1/*Ñ*/]='N',[0x0144/*ń*/]='n',[0x0143/*Ń*/]='N',[0x0146/*ņ*/]='n',[0x0145/*Ņ*/]='N',[0x0148/*ň*/]='n',[0x0147/*Ň*/]='N',
        [0x00f2/*ò*/]='o',[0x00d2/*Ò*/]='O',[0x00f3/*ó*/]='o',[0x00d3/*Ó*/]='O',[0x00f4/*ô*/]='o',[0x00d4/*Ô*/]='O',[0x00f5/*õ*/]='o',[0x00d5/*Õ*/]='O',[0x00f8/*ø*/]='o',[0x00d8/*Ø*/]='O',[0x014d/*ō*/]='o',[0x014c/*Ō*/]='O',[0x0151/*ő*/]='o',[0x0150/*Ő*/]='O',[0x01a1/*ơ*/]='o',[0x01a0/*Ơ*/]='O',
        [0x1e57/*ṗ*/]='p',[0x1e56/*Ṗ*/]='P',
        [0x0155/*ŕ*/]='r',[0x0154/*Ŕ*/]='R',[0x0157/*ŗ*/]='r',[0x0156/*Ŗ*/]='R',[0x0159/*ř*/]='r',[0x0158/*Ř*/]='R',
        [0x015b/*ś*/]='s',[0x015a/*Ś*/]='S',[0x015d/*ŝ*/]='s',[0x015c/*Ŝ*/]='S',[0x015f/*ş*/]='s',[0x015e/*Ş*/]='S',[0x0161/*š*/]='s',[0x0160/*Š*/]='S',[0x0219/*ș*/]='s',[0x0218/*Ș*/]='S',[0x1e61/*ṡ*/]='s',[0x1e60/*Ṡ*/]='S',
        [0x0163/*ţ*/]='t',[0x0162/*Ţ*/]='T',[0x0165/*ť*/]='t',[0x0164/*Ť*/]='T',[0x0167/*ŧ*/]='t',[0x0166/*Ŧ*/]='T',[0x1e6b/*ṫ*/]='t',[0x1e6a/*Ṫ*/]='T',[0x021b/*ț*/]='t',[0x021a/*Ț*/]='T',
        [0x00b5/*µ*/]='u',[0x00f9/*ù*/]='u',[0x00d9/*Ù*/]='U',[0x00fa/*ú*/]='u',[0x00da/*Ú*/]='U',[0x00fb/*û*/]='u',[0x00db/*Û*/]='U',[0x0169/*ũ*/]='u',[0x0168/*Ũ*/]='U',[0x016b/*ū*/]='u',[0x016a/*Ū*/]='U',[0x016d/*ŭ*/]='u',[0x016c/*Ŭ*/]='U',[0x016f/*ů*/]='u',[0x016e/*Ů*/]='U',[0x0171/*ű*/]='u',[0x0170/*Ű*/]='U',[0x0173/*ų*/]='u',[0x0172/*Ų*/]='U',[0x01b0/*ư*/]='u',[0x01af/*Ư*/]='U',
        [0x0175/*ŵ*/]='w',[0x0174/*Ŵ*/]='W',[0x1e81/*ẁ*/]='w',[0x1e80/*Ẁ*/]='W',[0x1e83/*ẃ*/]='w',[0x1e82/*Ẃ*/]='W',[0x1e85/*ẅ*/]='w',[0x1e84/*Ẅ*/]='W',
        [0x00fd/*ý*/]='y',[0x00dd/*Ý*/]='Y',[0x0177/*ŷ*/]='y',[0x0176/*Ŷ*/]='Y',[0x1ef3/*ỳ*/]='y',[0x1ef2/*Ỳ*/]='Y',[0x00ff/*ÿ*/]='y',[0x0178/*Ÿ*/]='Y',
        [0x017a/*ź*/]='z',[0x0179/*Ź*/]='Z',[0x017c/*ż*/]='z',[0x017b/*Ż*/]='Z',[0x017e/*ž*/]='z',[0x017d/*Ž*/]='Z',
        [0x00fe/*þ*/]='th',[0x00de/*Þ*/]='Th',
        [0x00f6/*ö*/]='oe',[0x00d6/*Ö*/]='Oe',[0x00f0/*ð*/]='dh',[0x00d0/*Ð*/]='Dh',
        [0x00e4/*ä*/]='ae',[0x00c4/*Ä*/]='Ae',[0x00e6/*æ*/]='ae',[0x00c6/*Æ*/]='Ae',
        [0x00fc/*ü*/]='ue',[0x00dc/*Ü*/]='Ue',
        [0x00df/*ß*/]='ss',
    };

    char *out = va("%*.s", strlen(s)*2, ""), *p = out;

    while( *s ) {
        const char *before = s;
        unsigned glyph = codepoint(&s);
        /**/ if( roman[glyph] > 255 ) *p++ = roman[glyph] & 255, *p++ = roman[glyph] >> 8;
        else if( roman[glyph] > 0 ) *p++ = roman[glyph];
        else memcpy(p, before, (int)(s-before)), p += (int)(s-before);
    }

    return *p++ = 0, out;
}

