void db_set(const char *key, int value) {
    char fname[1024+1];
    snprintf(fname, 1024, "%s.db", key);
    for(FILE *fp = fopen(fname, "wb"); fp; fclose(fp), fp=0) {
        fprintf(fp, "%d", value);
    }
}
int db_get(const char *key) {
    char fname[1024+1];
    snprintf(fname, 1024, "%s.db", key);
    int value = 0;
    for(FILE *fp = fopen(fname, "rb"); fp; fclose(fp), fp=0) {
        fscanf(fp, "%d", &value);
    }
//  fprintf(stderr, "%s=%d\n", key, value);
    return value;
}

#if 0

// #include "src/3rd.h"

// note: you must pre-define db-key, db-val structs beforehand
#if 1
typedef int db_key;
typedef int db_val;
#define db_key(...) __VA_ARGS__
#define db_val(...) __VA_ARGS__
#else
typedef struct db_key {
	int dummy;
} db_key;

typedef struct db_val {
	int dummy;
} db_val;

#define db_key(...) ((db_key) { __VA_ARGS__ })
#define db_val(...) ((db_val) { __VA_ARGS__ })
#endif

// header

void*   db_open(const char *dbfile);
bool     db_put(void *db, db_key key, db_val value); // overwrite
db_val*  db_get(void *db, db_key key);
void    db_close(void **db);

// impl

static KISSDB *global;
static
void db_close_(void) {
	db_close(&global);
}
void* db_open(const char *dbfile) {
	KISSDB *db = calloc(1, sizeof(KISSDB));
	if( KISSDB_open(db, dbfile, KISSDB_OPEN_MODE_RWCREAT, 4096/4, sizeof(db_key), sizeof(db_val) ) ) {
		return free(db), NULL;
	}
	if( !global ) {
		global = db;
		atexit( db_close_ );
	}
	return db;
}
void db_close(void **db) {
	if( db && *db ) KISSDB_close(*(KISSDB**)db), free(*db), *db = 0;
}
db_val *db_get(void *db, db_key key) {
	static db_val values[256], *ptr = values;
	ptr = &values[ ( (ptr + 1 - values) / sizeof(db_val) ) % 256 ];
	return KISSDB_get((KISSDB*)db, &key, ptr) == 0 ? ptr : NULL; // <0: error, 1: not found
}
bool db_put(void *db, db_key key, db_val value) { // overwrite
	return KISSDB_put((KISSDB*)db, &key, &value) == 0; // <0: error
}

#define each_db(db,kbuf,vbuf) \
	/*for*/( db_key dbk, *kbuf = &dbk; kbuf ; kbuf = 0 ) \
	for( db_val dbv, *vbuf = &dbv; vbuf ; vbuf = 0 ) \
	for( KISSDB_Iterator dbi, *dbiptr = (KISSDB_Iterator_init((KISSDB*)db,&dbi), &dbi); dbiptr; dbiptr = 0) \
	for( ; KISSDB_Iterator_next(&dbi,kbuf,vbuf) > 0 ; ) // <0:error, 0:eof

#ifdef DB_TEST
int main() {
	void *db = db_open("my.db");
	if( db ) {
		db_val *found = db_get(db, db_key(0) );
		if( found ) printf("found key=%d\n", *(int*)found );

		db_put(db, db_key(0), db_val((found ? *found : 0) + 1));

		srand(time(0));
		db_put(db, db_key(rand()), db_val(rand()));

		for each_db(db,key,val) {
			printf("[%d]=%d\n", *key, *val);
		}
	}
}
#define main main2
#endif

#endif
