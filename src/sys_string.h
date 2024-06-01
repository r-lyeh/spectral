#include <stdio.h>
#include <stdarg.h>
char* va(const char *fmt, ...) {
    static char buf[16][1024];
    static int l = 0; l = (l+1) % 16;
    va_list vl;
    va_start(vl,fmt);
    int rc = vsnprintf(buf[l], 1024-1, fmt, vl);
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
bool strendi(const char *src, const char *sub) { // returns true if both strings match at end. case insensitive
    int srclen = strlen(src);
    int sublen = strlen(sub);
    if( sublen > srclen ) return 0;
    return !strcmpi(src + srclen - sublen, sub);
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
void *memset32(void *dst, unsigned ch, unsigned bytes) {
    unsigned ch4 = (ch >> 24) & 0xff;
    unsigned ch3 = (ch >> 16) & 0xff;
    unsigned ch2 = (ch >>  8) & 0xff;
    unsigned ch1 = (ch >>  0) & 0xff;
    char *ptr = (char*)dst;
    while( bytes ) {
        if( bytes-- ) *ptr++ = ch4;
        if( bytes-- ) *ptr++ = ch3;
        if( bytes-- ) *ptr++ = ch2;
        if( bytes-- ) *ptr++ = ch1;
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
