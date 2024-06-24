// fuzzy multi hash map,
// - rlyeh, public domain

// @todo: maybe use xf8 as a bloom filter. needed? map impl works rn, but see:
// There are a few approaches used for building fuzzy hash algorithms:
// - Context Triggered Piecewise Hashing (CTPH), which constructs a hash by splitting the input into multiple pieces, calculating traditional hashes for each piece, and then combining those traditional hashes into a single string.[8]
// - Locality Sensitive Hashing places similar input items into the same "buckets", which can be used for data clustering and nearest neighbor searches

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

int strmatch(const char *s, const char *wildcard) {
    // returns true if wildcard matches
    if( *wildcard == '\0' ) return !*s;
    if( *wildcard ==  '*' ) return strmatch(s, wildcard+1) || (*s && strmatch(s+1, wildcard));
    if( *wildcard ==  '?' ) return *s && (*s != '.') && strmatch(s+1, wildcard+1);
    return (*s == *wildcard) && strmatch(s+1, wildcard+1);
}
int strmatchi(const char *s, const char *wildcard) {
    // returns true if wildcard matches (case insensitive)
    if( *wildcard == '\0' ) return !*s;
    if( *wildcard ==  '*' ) return strmatchi(s, wildcard+1) || (*s && strmatchi(s+1, wildcard));
    if( *wildcard ==  '?' ) return *s && (*s != '.') && strmatchi(s+1, wildcard+1);
    return (tolower(*s) == tolower(*wildcard)) && strmatchi(s+1, wildcard+1);
}






#ifndef KEY
#define KEY char*
#endif
#ifndef VAL
#define VAL void*
#endif

// convert a key into a bucket [0..N]
#define NN 28
#define KEYHASH(k) (isalpha(0[k]) ? (unsigned)toupper(0[k]) % 26 : 26u + (0[k] == '#'))
// utils
#ifndef KEYCMP
#define KEYCMP   strmatchi // !strcmp
#endif
#ifndef KEYNEW
#define KEYNEW   strdup
#endif
#ifndef KEYDEL
#define KEYDEL   free
#endif
#ifndef VALNEW
#define VALNEW   strdup
#endif
#ifndef VALDEL
#define VALDEL   free
#endif


struct bucket {
	KEY *keys;
	VAL *vals;
	unsigned count;
	unsigned cap;
};
VAL* bucket_insert(struct bucket *b, const KEY k, const VAL v) {
	if( (b->count+1) >= b->cap ) {
		b->cap = 1 + (b->cap * 1.5);
		b->keys = realloc(b->keys, sizeof(KEY) * b->cap);
		b->vals = realloc(b->vals, sizeof(VAL) * b->cap);
	}
	b->keys[b->count] = (KEY)KEYNEW(k);
	b->vals[b->count] = (VAL)VALNEW(v);
	return b->vals + b->count++;
}
unsigned bucket_find_index(struct bucket *b, const KEY k) {
	for( unsigned i = 0; i < b->count; ++i ) {
		if( KEYCMP(b->keys[i], k) ) {
			return i+1;
		}
	}
	return 0;
}
VAL* bucket_find(struct bucket *b, const KEY k) {
	unsigned index = bucket_find_index(b, k);
	return index ? b->vals + index - 1 : 0;
}

#define each_bucket(b,k,v) \
	/*for*/( unsigned I = 0; I < (b)->count; ++I ) \
		for(KEY *k = (b)->keys + I; k ; k = 0) \
			for(VAL *v = (b)->vals + I; v ; v = 0)

struct map {
	struct bucket b[NN];
};

#define map_empty(m)      (!map_count(m))
#define map_insert(m,k,v) bucket_insert((m)->b + KEYHASH(k), k, v)
//#define map_find(m,k)     bucket_find((m)->b + KEYHASH(k), k)
#define each_map(m,k,v) \
	/*for*/(int N = 0; N < NN; ++N) \
		for( struct bucket *b = (m)->b + N; b ; b = 0) \
			 for each_bucket(b, k, v)

VAL* map_find(struct map *m, const KEY k) {
	int N = 0, M = NN - 1;
	if( k[0] != '*' ) N = M = KEYHASH(k);
	for( ; N <= M; ++N )
	for( struct bucket *b = (m)->b + N; b ; b = 0) {
		for( unsigned i = 0; i < b->count; ++i ) {
			if( KEYCMP(b->keys[i], k) ) {
				return b->vals + i;
			}
		}
	}
	return NULL;
}

VAL** map_multifind(struct map *m, const KEY k, int *count) { // result must be free() after use
	VAL** ret = 0;
	int cap = 0;

	*count = 0;

	int N = 0, M = NN - 1;
	if( k[0] != '*' ) N = M = KEYHASH(k);
	for( ; N <= M; ++N )
	for( struct bucket *b = (m)->b + N; b ; b = 0) {
		for( unsigned i = 0; i < b->count; ++i ) {
			if( KEYCMP(b->keys[i], k) ) {
				if( (*count) == cap ) {
					ret = realloc(ret, sizeof(VAL*) * (cap = (*count * 1.5) + 1) );
				}
				ret[ (*count = *count + 1) - 1 ] = b->vals + i;
			}
		}
	}

	return ret;
}



int map_count(struct map *m) { unsigned c = 0; for(int N = 0; N < NN; ++N) c += (m)->b[N].count; return c; }

void map_free(struct map *m) {
	for each_map(m,k,v) {
		KEYDEL(*k);
		VALDEL(*v);
	}
	for(int N = 0; N < NN; ++N) {
		(m)->b[N].keys && realloc( (m)->b[N].keys, 0 );
		(m)->b[N].vals && realloc( (m)->b[N].vals, 0 );
	}
	memset(m, 0, sizeof(struct map));
}

#ifdef MAP_TEST
#define main main2
int (main)() {
	struct map m = {0};

	// insert
	assert( map_insert(&m, "abc", "def") );
	assert( map_insert(&m, "ghi", "jkl") );
	assert( map_insert(&m, "mno", "pqr") );

	// find
	assert( !map_find(&m, "non-existing") );

	assert( map_find(&m, "abc") );
	assert( !strcmp("def", *map_find(&m, "abc")) );

	assert( map_find(&m, "ghi") );
	assert( !strcmp("jkl", *map_find(&m, "ghi")) );

	// iterate
	for each_map(&m, k, v)
		printf("[%s] = %s\n", *k, (char*)*v);

	// extra cases because of our special KEYCMP function (strmatchi)
	assert( map_find(&m, "MnO") );
	assert( !strcmp("pqr", *map_find(&m, "MnO")) );

	assert( map_find(&m, "m*O") );
	assert( !strcmp("pqr", *map_find(&m, "m*O")) );

	// delete
	map_free(&m);

	assert(~puts("Ok"));
}
#endif
