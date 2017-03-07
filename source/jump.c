#include "game/game_main.h"

struct _reent r;

FILE_STRUCT* sOpen(const char *path)
{
	FILE_STRUCT* f=malloc(sizeof(FILE_STRUCT));
	_FAT_open_r(&r, f, path, O_RDONLY, 0);
	return f;
}
int sClose(FILE_STRUCT* f)
{
	return _FAT_close_r (&r, (int)f);
}

ssize_t sRead(FILE_STRUCT* f, char *ptr, size_t len)
{
// iprintf("about to read : %d",r._errno);
	return _FAT_read_r(&r, (int)f, ptr, len);
// iprintf("read errno : %d",r._errno);
}

off_t sSeek(FILE_STRUCT* f, u32 pos, int dir)
{
	return _FAT_seek_r (&r, (int)f, (off_t)pos, dir);
}
