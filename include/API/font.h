#ifndef __FONT9__
#define __FONT9__

#define CHARSIZE 16

typedef struct
{
	MTL_img tex;
	u8 charsize;
}font;

font hudFont, APIfont;
font* currentFont;

void setFont(font* f);
void loadFont(font* f, u8 charsize);
void drawChar(char c, u16 color, int32 x, int32 y);
void drawString(char* s, u16 color, int32 size, int32 x, int32 y);

#endif