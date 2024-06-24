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
