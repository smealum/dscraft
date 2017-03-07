#ifndef JUMP_9
#define JUMP_9

FILE_STRUCT* sOpen(const char *path);
int sClose(FILE_STRUCT* f);
ssize_t sRead(FILE_STRUCT* f, char *ptr, size_t len);
off_t sSeek(FILE_STRUCT* f, u32 pos, int dir);

#endif