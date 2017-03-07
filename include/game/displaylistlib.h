#ifndef _DISPLAYLISTLIB_H_
#define _DISPLAYLISTLIB_H_

#define DEFAULT_TEXTURE_SIZE_DL    TEXTURE_SIZE_128

u32 glNormalDL(uint32 normal);

u32 glVertexPackedDL(u32 packed);

void glTexCoordPACKED(u32 uv);

void glColorDL(rgb color);

u32 glBeginDL(u32 type);
void glEndDL();

u32* glBeginListDL();
void glEndListDL();

#endif
