/*
copy /y Spectral.db.gz s.gz
gzip -d -f s.gz

cl hash.c
hash.exe < s > s2
sort s2 > s3
find /C "/" s3
hash.exe s3 && echo ok || echo error

del s
del s2
del s3
*/

#include "../../3rd.h"
#include "../../sys_string.h"
int main(int argc, char **argv) {
	char buffer[256], line1[256], line2[256];

	if( argc == 2 ) {
		int collisions = 0;
		for( FILE* in = fopen(argv[1], "rb"); in ; fclose(in), in = 0 ) {
			unsigned hash1; fscanf(in, "%x %[^\r\n]", &hash1, line1); fgets(buffer, 256, in);

			while(!feof(in)) {
			unsigned hash2; fscanf(in, "%x %[^\r\n]", &hash2, line2); fgets(buffer, 256, in);
			if( hash1 == hash2 )
				if( strcmp(line1, line2) ) 
					++collisions, printf("Error! CRC collider: `%s` vs `%s`\n", line1, line2);
			hash1 = hash2;
			strcpy(line1, line2);
			} 
		}
		printf("%d collisions\n", collisions);
		exit(-collisions);
	}

	while( fgets(buffer, 256, stdin) ) {
		if( buffer[0] != '/' ) continue;
		if(strchr(buffer,'|'))  strchr(buffer,'|')[0] = '\0';
		if(strchr(buffer,'\r')) strchr(buffer,'\r')[0] = '\0';
		if(strchr(buffer,'\n')) strchr(buffer,'\n')[0] = '\0';
//		printf("%p %s\n", (void*)(uintptr_t)crc32(0, buffer, strlen(buffer)), buffer);
		printf("%p %s\n", (void*)(uintptr_t)fnv1a(buffer, strlen(buffer)), buffer);
	}
}
