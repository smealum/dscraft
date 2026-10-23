#ifndef __SCREEN9__
#define __SCREEN9__

/*#define FRAMESC 1

FILE* screenfi;

u16 screenbuff[256*384];
int frm;

void Debug_TakeScreenshotBMP(void);
void D3D_TakeScreenshotBMP(u16* buffer);*/

/* The screenshot implementation is compiled in screenshot.Os.c and is used
 * by the debug screenshot path. Keep its public declaration visible. */
void D3D_SaveScreen(const char* filename, u16* buffer);

#endif
