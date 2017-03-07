#include "game/game_main.h"

//#define RESERVED_SIZE_DISPLAY_LISTS   (1024*1024) /*4 MB*/
// #define RESERVED_SIZE_DISPLAY_LISTS   (16*1024) /*64 KB*/
#define RESERVED_SIZE_DISPLAY_LISTS   (1*1024) /*4 KB*/
// #define RESERVED_SIZE_DISPLAY_LISTS   (64*1024) //FOR CAPTURE ONLY (don't need that much for stars)
#define S_TEXTURE_COORDINATE_OFFSETED

unsigned int textureSizeWidthDL  = DEFAULT_TEXTURE_SIZE_DL;
unsigned int textureSizeHeightDL = DEFAULT_TEXTURE_SIZE_DL;

u32 dl_displayLists[RESERVED_SIZE_DISPLAY_LISTS];
u32 dl_displayLists_filled = 0;

u32 dl_curdisplayList_filled_start;

u8 dl_commands_buffer[4];
u32 dl_commands_buffer_filled = 0;

u32 dl_attributes_buffer[8];
u32 dl_attributes_buffer_filled = 0;



void setCurrentTextureSizeDL(enum GL_TEXTURE_SIZE_ENUM sizeWidth, enum GL_TEXTURE_SIZE_ENUM sizeHeight) {
    textureSizeWidthDL  = sizeWidth;
    textureSizeHeightDL = sizeHeight;
}

void dl_packCommandsForDisplayList() {
    u32 i;

	// NOGBA("packing : %d %d",dl_commands_buffer_filled,dl_attributes_buffer_filled);
    while (dl_commands_buffer_filled < 4) {
		// NOGBA("NOP");
        dl_commands_buffer[dl_commands_buffer_filled] = FIFO_NOP;
        dl_commands_buffer_filled++;
    }
    dl_displayLists[dl_displayLists_filled] = FIFO_COMMAND_PACK(dl_commands_buffer[0], dl_commands_buffer[1], dl_commands_buffer[2], dl_commands_buffer[3]);
    dl_displayLists_filled++;
    for (i=0; i<dl_attributes_buffer_filled; i++) {
        dl_displayLists[dl_displayLists_filled] = dl_attributes_buffer[i];
        dl_displayLists_filled++;
    }

    dl_commands_buffer_filled = 0;
    dl_attributes_buffer_filled = 0;

    if (dl_displayLists_filled > RESERVED_SIZE_DISPLAY_LISTS){NOGBA("overflow in model loading!\n");iprintf("overflow in model loading!\n");}
}


u32 glVertexPackedDL(u32 packed) {
    if (dl_attributes_buffer_filled > 2)
        dl_packCommandsForDisplayList();
    dl_commands_buffer[dl_commands_buffer_filled] = FIFO_VERTEX10;
    dl_commands_buffer_filled++;
    dl_attributes_buffer[dl_attributes_buffer_filled] = (u32) (packed);
    dl_attributes_buffer_filled++;
	u32 temp=dl_displayLists_filled+dl_commands_buffer_filled;
    if (dl_commands_buffer_filled == 4 || dl_attributes_buffer_filled == 4) dl_packCommandsForDisplayList();
	return temp;
}

void glTexCoordPACKED(u32 uv) {
    dl_commands_buffer[dl_commands_buffer_filled] = FIFO_TEX_COORD;
    dl_commands_buffer_filled++;
    dl_attributes_buffer[dl_attributes_buffer_filled] = (u32) (uv);
    dl_attributes_buffer_filled++;
    if (dl_commands_buffer_filled == 4 || dl_attributes_buffer_filled == 4)
        dl_packCommandsForDisplayList();
}

u32 glNormalDL(uint32 normal) {
    dl_commands_buffer[dl_commands_buffer_filled] = FIFO_NORMAL;
    dl_commands_buffer_filled++;
    dl_attributes_buffer[dl_attributes_buffer_filled] = (u32) normal;
    dl_attributes_buffer_filled++;
	u32 temp=dl_displayLists_filled+dl_commands_buffer_filled;
    if (dl_commands_buffer_filled == 4 || dl_attributes_buffer_filled == 4)dl_packCommandsForDisplayList();
	return temp;
}

void glColorDL(rgb color) {
    dl_commands_buffer[dl_commands_buffer_filled] = FIFO_COLOR;
    dl_commands_buffer_filled++;
    dl_attributes_buffer[dl_attributes_buffer_filled] = (u32) color;
    dl_attributes_buffer_filled++;
    if (dl_commands_buffer_filled == 4 || dl_attributes_buffer_filled == 4)
        dl_packCommandsForDisplayList();
}

void glBindPaletteDL(u32 addr) {
    dl_commands_buffer[dl_commands_buffer_filled] = FIFO_PAL_FORMAT;
    dl_commands_buffer_filled++;
    dl_attributes_buffer[dl_attributes_buffer_filled] = (u32) addr;
    dl_attributes_buffer_filled++;
    if (dl_commands_buffer_filled == 4 || dl_attributes_buffer_filled == 4)
        dl_packCommandsForDisplayList();
}

void glBindTextureDL(u32 addr) {
    dl_commands_buffer[dl_commands_buffer_filled] = FIFO_TEX_FORMAT;
    dl_commands_buffer_filled++;
    dl_attributes_buffer[dl_attributes_buffer_filled] = (u32) addr;
    dl_attributes_buffer_filled++;
    if (dl_commands_buffer_filled == 4 || dl_attributes_buffer_filled == 4)
        dl_packCommandsForDisplayList();
}

void applyMTLDL(MTL_img* mtl)
{
	if(mtl)
	{
		glBindTextureDL(mtl->param);
		glBindPaletteDL(((uint32)mtl->pal)>>(4));
	}else glBindTextureDL(0);
}

u32 glBeginDL(u32 type) {
	dl_packCommandsForDisplayList();
	u32 r=(dl_displayLists_filled - dl_curdisplayList_filled_start) - 1;
    dl_commands_buffer[dl_commands_buffer_filled] = FIFO_BEGIN;
    dl_commands_buffer_filled++;
    dl_attributes_buffer[dl_attributes_buffer_filled] = type;
    dl_attributes_buffer_filled++;
    if (dl_commands_buffer_filled == 4 || dl_attributes_buffer_filled == 4) dl_packCommandsForDisplayList();
	return r;
}

void glEndDL() {
    dl_commands_buffer[dl_commands_buffer_filled] = FIFO_END;
    dl_commands_buffer_filled++;
    if (dl_commands_buffer_filled == 4)
        dl_packCommandsForDisplayList();
}

u32* glBeginListDL() {
    dl_displayLists_filled = 0;
	dl_attributes_buffer_filled = 0;
	dl_commands_buffer_filled = 0;
    dl_curdisplayList_filled_start = dl_displayLists_filled;
    dl_displayLists_filled++; // current position will need to contain how much commands there are in the end, so we skip that one for now
    return (dl_displayLists + dl_curdisplayList_filled_start);
}

void glEndListDL() {
    dl_packCommandsForDisplayList();
    // start of the display list now needs to be updated as to how much commands it holds
    dl_displayLists[dl_curdisplayList_filled_start] = (dl_displayLists_filled - dl_curdisplayList_filled_start) - 1;
    // printf("  [ display lists: %u bytes ] ", dl_displayLists_filled);
}
