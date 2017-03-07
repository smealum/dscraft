#include "game/game_main.h"

#define IBARY 170
#define IBARD 20
#define IBARX (39+IBARD)

#define COMPX 8
#define COMPY 34

#define CLOCKX 8
#define CLOCKY 78

#define TOGGLEX 0
#define TOGGLEY 1
#define TOGGLESX 96
#define TOGGLESY 24

#define SAVEX 187
#define SAVEY 1
#define SAVESX (256-SAVEX)
#define SAVESY 24

#define INVX 97
#define INVY 1
#define INVSX 90
#define INVSY 24

#define INVENTORYX 55
#define INVENTORYY 110
#define INVENTORYOX 18
#define INVENTORYOY 18

#define INVENTORYBX 55
#define INVENTORYBY 168
#define INVENTORYBOX 18

#define TOGGLECOL (256-10)
#define SAVECOL (256-11)
#define INVCOL (256-12)

u8 itemBar[]={1,3,4,13,6,7,8,11,12}; //water test
// u8 itemBar[]={1,3,4,13,6,7,40,11,12}; //ladder test
// u8 itemBar[]={1,3,4,13,6,7,DOORTYPE,11,12}; //door test
// u8 inventoryItem[]={14,15,16,17,18,19,20,21,22,
					// 23,24,25,26,27,28,29,30,31,
					// 32,33,34,35,36,37,38,39,2};
u8 inventoryItem[]={14,15,16,17,18,19,20,21,22,
					23,24,25,26,27,28,29,30,31,
					32,33,34,35,36,40,DOORTYPE,39,2};
u8 inventoryItems=27;

u8 cursorSprite, buttonSprite;

//from libnds (heavily modified)
void imageTileDataDest(sImage* img, u32* destination)
{
	int ix, iy, tx, ty;
	int th, tw;
	int i = 0;

	th = img->height >> 3;
	tw = img->width >> 3;

	int x8, y8, x4, y4;	
	for(x4=0;x4<4;x4++)
	{
		for(y4=0;y4<4;y4++)
		{
			for(x8=0;x8<8;x8++)
			{
				for(y8=0;y8<8;y8++)
				{
					ty=y8+8*y4;tx=x8+8*x4;i=0;
						for(iy = 0; iy < 8; iy++)
							for(ix = 0; ix < 2; ix++)
								destination[(i++)+(x8+(y8+(x4+4*y4)*8)*8)*16] = img->image.data32[ix + tx * 2 + (iy + ty * 8) * tw * 2 ]; 
				}
			}
		}
	}
}

void loadImageDirect(char* filename)
{
	FILE* f=DS_OpenFile(filename, "", false, true);
	vramSetBankI(VRAM_I_LCD);
	fread(VRAM_I_EXT_SPR_PALETTE[0],2,256,f);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
	fread(SPRITE_GFX_SUB,256*256,1,f);
	fclose(f);
}

void initItems(void)
{
	int i;
	for(i=0;i<MAXITEMS;i++)
	{
		items[i].id=usedSprites;
		items[i].used=false;
		oamSub.oamMemory[items[i].id].attribute[0] = ATTR0_DISABLED;
		usedSprites++;
	}
}

void initSlots(void)
{
	int i;
	for(i=0;i<MAXSLOTS;i++)
	{
		slots[i].id=-1;
		slots[i].used=false;
	}
}

void setItemPosition(u8 id, u8 x, u8 y)
{
	items[id].position.x=x;
	items[id].position.y=y;
	oamSub.oamMemory[items[id].id].attribute[0] = ATTR0_BMP | ATTR0_SQUARE | (items[id].position.y);
	oamSub.oamMemory[items[id].id].attribute[1] = ATTR1_SIZE_16 | (items[id].position.x);
	// oamSub.oamMemory[items[id].id].attribute[2] = ATTR2_ALPHA(1) | ATTR2_PRIORITY(0) | (8*2*(24+items[id].type*2));
	// oamSub.oamMemory[items[id].id].attribute[2] = ATTR2_ALPHA(1) | ATTR2_PRIORITY(0) | (8*2*(24+((items[id].type-(items[id].type%8))/8)*2)+2*(items[id].type%8));
	// oamSub.oamMemory[items[id].id].attribute[2] = ATTR2_ALPHA(1) | ATTR2_PRIORITY(0) | (8*2*(32+((items[id].type-(items[id].type%8))/8)*2)+2*(items[id].type%8));
	oamSub.oamMemory[items[id].id].attribute[2] = ATTR2_ALPHA(1) | ATTR2_PRIORITY(0) | (8*2*(36+((items[id].type-(items[id].type%8))/8)*2)+2*(items[id].type%8));
}

void initItemBar(void)
{
	int i;
	for(i=0;i<9;i++)
	{
		items[i].type=itemBar[i];
		items[i].slot=i;
		items[i].used=true;
		
		slots[i].id=i;
		slots[i].position.x=IBARX+i*IBARD;
		slots[i].position.y=IBARY;
		slots[i].used=true;
		
		setItemPosition(i, slots[items[i].slot].position.x, slots[items[i].slot].position.y);
	}
}

void initInventory(void)
{
	int i, j;
	int id=9;
	for(j=0;j<3;j++)
	{
		for(i=0;i<9;i++)
		{
			slots[id].id=id;
			slots[id].position.x=INVENTORYX+i*INVENTORYOX;
			slots[id].position.y=INVENTORYY+j*INVENTORYOY;
			slots[id].used=false;
			id++;
		}
	}
	for(i=0;i<inventoryItems;i++)
	{
		items[9+i].type=inventoryItem[i];
		items[9+i].slot=9+i;
		items[9+i].used=true;
		
		setItemPosition(i, slots[items[i].slot].position.x, slots[items[i].slot].position.y);
	}
}

void loadInterface(char* filename, u8 prio)
{
	int i,x,y;
	
	// u8* buffer=DS_OpenFile(filename, "", true, true);
	
	// sImage pcx;
	// FILE* f=DS_OpenFile(filename, "", false, true);
	// directLoadPCX(f, &pcx);
	// fclose(f);
	// loadPCX((u8*)buffer, &pcx);
	// free(buffer);
    // imageTileDataDest(&pcx, (u32*)SPRITE_GFX_SUB);
	loadImageDirect(filename);
	
	// vramSetBankI(VRAM_I_LCD);
	// for(i=0;i<256;i++)VRAM_I_EXT_SPR_PALETTE[0][i]  = pcx.palette[i];
	// vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
	
	// int x4, y4;
	// for(x4=0;x4<4;x4++)
	// {
		// for(y4=0;y4<4;y4++)
		// {
			// for(x=0;x<8;x++)
			// {
				// for(y=0;y<8;y++)
				// {
					// for(i=0;i<32;i++)SPRITE_GFX_SUB[i+x*32+(y+(x4+4*y4)*8)*32*8] = pcx.image.data16[i+(x+x4*8)*32+(y+8*y4)*32*32];
				// }
			// }
		// }
	// }
	// imageDestroy(&pcx);
	int id=0;
	for(y = 0; y < 3; y++)
	{
		for(x = 0; x < 4; x++)
		{
			oamSub.oamMemory[id].attribute[0] = ATTR0_COLOR_256 | ATTR0_SQUARE | (64 * y);
			oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (64 * x);
			oamSub.oamMemory[id].attribute[2] = ATTR2_PRIORITY(prio) | ATTR2_PALETTE(0) | (8 * 2 * (x+4*y));
			id++;
		}
	}
}

void initInterface(void)
{
	// oamInit(&oamSub, SpriteMapping_Bmp_2D_256, false);
	oamInit(&oamSub, SpriteMapping_1D_256, true);
 
	int x, y, i;
 
	usedSprites=0;
	int id = 0;

	for(y = 0; y < 3; y++)
	{
		for(x = 0; x < 4; x++)
		{
			oamSub.oamMemory[id].attribute[0] = ATTR0_COLOR_256 | ATTR0_SQUARE | (64 * y);
			oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (64 * x);
			oamSub.oamMemory[id].attribute[2] = ATTR2_PRIORITY(1) | ATTR2_PALETTE(0) | (8 * 2 * (x+4*y));
			id++;
		}
	}
	{
		buttonSprite=id;
		oamSub.oamMemory[id].attribute[0] = ATTR0_COLOR_256 | ATTR0_WIDE | (0);
		oamSub.oamMemory[id].attribute[0] = ATTR0_DISABLED;
		oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (0);
		oamSub.oamMemory[id].attribute[2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(0) | (8 * 2 * (0+4*3));
		id++;
		oamSub.oamMemory[id].attribute[0] = ATTR0_COLOR_256 | ATTR0_WIDE | (0);
		oamSub.oamMemory[id].attribute[0] = ATTR0_DISABLED;
		oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (64);
		oamSub.oamMemory[id].attribute[2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(0) | (8 * 2 * (1+4*3));
		id++;
		
		
		oamSub.oamMemory[id].attribute[0] = ATTR0_COLOR_256 | ATTR0_WIDE | (0);
		oamSub.oamMemory[id].attribute[0] = ATTR0_DISABLED;
		oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (96);
		oamSub.oamMemory[id].attribute[2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(0) | (8 * 2 * (0+4*3)+8);
		id++;
		oamSub.oamMemory[id].attribute[0] = ATTR0_COLOR_256 | ATTR0_WIDE | (0);
		oamSub.oamMemory[id].attribute[0] = ATTR0_DISABLED;
		oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (96+64);
		oamSub.oamMemory[id].attribute[2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(0) | (8 * 2 * (1+4*3)+8);
		id++;
		
		
		oamSub.oamMemory[id].attribute[0] = ATTR0_COLOR_256 | ATTR0_WIDE | (0);
		oamSub.oamMemory[id].attribute[0] = ATTR0_DISABLED;
		oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (128);
		oamSub.oamMemory[id].attribute[2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(0) | (8 * 2 * (2+4*3));
		id++;
		oamSub.oamMemory[id].attribute[0] = ATTR0_COLOR_256 | ATTR0_WIDE | (0);
		oamSub.oamMemory[id].attribute[0] = ATTR0_DISABLED;
		oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (128+64);
		oamSub.oamMemory[id].attribute[2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(0) | (8 * 2 * (3+4*3));
		id++;
		
		oamSub.oamMemory[id].attribute[0] = ATTR0_COLOR_256 | ATTR0_WIDE | (80);
		oamSub.oamMemory[id].attribute[0] = ATTR0_DISABLED;
		oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (128-64);
		oamSub.oamMemory[id].attribute[2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(0) | (8 * 2 * (2+4*3)+8);
		id++;
		oamSub.oamMemory[id].attribute[0] = ATTR0_COLOR_256 | ATTR0_WIDE | (80);
		oamSub.oamMemory[id].attribute[0] = ATTR0_DISABLED;
		oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (128-64+64);
		oamSub.oamMemory[id].attribute[2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(0) | (8 * 2 * (3+4*3)+8);
		id++;
	}
	{
		oamSub.oamMemory[id].attribute[0] = ATTR0_COLOR_256 | ATTR0_SQUARE | ATTR0_ROTSCALE_DOUBLE | (COMPY);
		oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_16 | ATTR1_ROTDATA(0) | (COMPX);
		oamSub.oamMemory[id].attribute[2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(3) | (8 * 2 * (0+4*4));
		id++;
	}
	{
		oamSub.oamMemory[id].attribute[0] = ATTR0_COLOR_256 | ATTR0_SQUARE | ATTR0_ROTSCALE | (CLOCKY);
		oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_32 | ATTR1_ROTDATA(1) | (CLOCKX);
		oamSub.oamMemory[id].attribute[2] = ATTR2_PRIORITY(2) | ATTR2_PALETTE(1) | (8 * 2 * (0+4*4)+1);
		id++;
	}
	{
		oamSub.oamMemory[id].attribute[0] = ATTR0_COLOR_256 | ATTR0_SQUARE | (0);
		oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_32 | (0);
		oamSub.oamMemory[id].attribute[2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(2) | (8 * 2 * (0+4*4)+5);
		cursorSprite=id;
		id++;
	}
	usedSprites=id;
	initItems();
	initSlots();
	initItemBar();
	initInventory();
	
	loadInterface("loading.bin",0);

	sImage pcx2;
	u8* buffer2=DS_OpenFile("compass.pcx", "", true, true);
	loadPCX((u8*)buffer2, &pcx2);
	free(buffer2);
	vramSetBankI(VRAM_I_LCD);
	for(i=0;i<256;i++)VRAM_I_EXT_SPR_PALETTE[3][i]  = pcx2.palette[i];
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
    imageTileData(&pcx2);
	for(i=0;i<64*2;i++)SPRITE_GFX_SUB[i+256*128]=pcx2.image.data16[i];
	imageDestroy(&pcx2);
	buffer2=DS_OpenFile("daynight.pcx", "", true, true);
	loadPCX((u8*)buffer2, &pcx2);
	free(buffer2);
	vramSetBankI(VRAM_I_LCD);
	for(i=0;i<256;i++)VRAM_I_EXT_SPR_PALETTE[1][i]  = pcx2.palette[i];
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
    imageTileData(&pcx2);
	for(i=0;i<64*8;i++)SPRITE_GFX_SUB[i+256*128+64*2]=pcx2.image.data16[i];
	imageDestroy(&pcx2);
	buffer2=DS_OpenFile("cursor.pcx", "", true, true);
	loadPCX((u8*)buffer2, &pcx2);
	free(buffer2);
	vramSetBankI(VRAM_I_LCD);
	for(i=0;i<256;i++)VRAM_I_EXT_SPR_PALETTE[2][i]  = pcx2.palette[i];
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
    imageTileData(&pcx2);
	for(i=0;i<64*8;i++)SPRITE_GFX_SUB[i+256*128+64*10]=pcx2.image.data16[i];
	imageDestroy(&pcx2);

	swiWaitForVBlank();
 
	oamUpdate(&oamSub);

	invOpen=false;
}

s8 selectedItem=-1;
vect3D offset;

bool updateInterface(void)
{
	int i, j;

	oamRotateScale(&oamSub, 0, -Player.angleZ, intToFixed(1, 8), intToFixed(1, 8));
	oamRotateScale(&oamSub, 1, sunX+8192, intToFixed(1, 8), intToFixed(1, 8));
	oamUpdate(&oamSub);

	// vramSetBankI(VRAM_I_LCD);
		
	for(i=0;i<MAXITEMS;i++)
	{
		if(items[i].used)
		{
			if(slots[items[i].slot].used)
			{
				if(!((keysUp() & KEY_TOUCH) && selectedItem==i))setItemPosition(i, slots[items[i].slot].position.x, slots[items[i].slot].position.y);
			}else setItemPosition(i, 255, 192);
			if(items[i].slot<9)
			{
				// if(cursorBlock==items[i].type)SPRITE_PALETTE_SUB[255-i]=RGB15(31,31,31);
				// else SPRITE_PALETTE_SUB[255-i]=RGB15(15,15,15);	
				// if(cursorBlock==items[i].type)VRAM_I_EXT_SPR_PALETTE[0][255-items[i].slot]=RGB15(31,31,31);
				// else VRAM_I_EXT_SPR_PALETTE[0][255-items[i].slot]=RGB15(15,15,15);
				if(cursorBlock==items[i].type)
				{
					tempCursor=items[i].slot;
					oamSub.oamMemory[cursorSprite].attribute[0] = ATTR0_COLOR_256 | ATTR0_SQUARE | (items[i].position.y-2);
					oamSub.oamMemory[cursorSprite].attribute[1] = ATTR1_SIZE_32 | (items[i].position.x-2);
					oamSub.oamMemory[cursorSprite].attribute[2] = ATTR2_PRIORITY(0) | ATTR2_PALETTE(2) | (8 * 2 * (0+4*4)+5);
				}
			}
			if(keysDown() & KEY_TOUCH)
			{
				if(thisXY.px>=items[i].position.x && thisXY.py>=items[i].position.y && thisXY.px<items[i].position.x+16 && thisXY.py<items[i].position.y+16)
				{
					if(items[i].slot<9)cursorBlock=items[i].type;
					if(invOpen)
					{
						selectedItem=i;
						offset.x=items[i].position.x-thisXY.px;
						offset.y=items[i].position.y-thisXY.py;
					}
				}
			}
			if(invOpen)
			{
				if(keysHeld() & KEY_TOUCH)
				{
					if(selectedItem==i)
					{
						setItemPosition(i,thisXY.px+offset.x,thisXY.py+offset.y);
					}
				}
				if(keysUp() & KEY_TOUCH)
				{
					if(selectedItem==i)
					{
						for(j=0;j<MAXSLOTS;j++)
						{
							if(slots[j].used && items[i].position.x+8>slots[j].position.x && items[i].position.y+8>slots[j].position.y 
							&& items[i].position.x+8<=slots[j].position.x+16 && items[i].position.y+8<=slots[j].position.y+16)
							{
								if(slots[j].id>=0)items[slots[j].id].slot=items[i].slot;
								slots[items[i].slot].id=slots[j].id;
								items[i].slot=j;
								slots[j].id=i;
								break;
							}
						}
					}
				}
			}
		}
	}
	if(!invOpen && (keysDown() & KEY_TOUCH))overButtons=thisXY.py<TOGGLEY+TOGGLESY;
	else if(invOpen)overButtons=true;
	if(!(keysHeld() & KEY_TOUCH))selectedItem=-1;
	if((keysHeld() & KEY_TOUCH) && thisXY.px>=CLOCKX && thisXY.py>=CLOCKY && thisXY.px<CLOCKX+32 && thisXY.py<CLOCKY+16)
	{
		sunX+=600;
		cloudcnt+=200;
	}
	if(overButtons)
	{
		if((keysUp() & KEY_TOUCH) && lastXY.px>=TOGGLEX && lastXY.py>=TOGGLEY && lastXY.px<TOGGLEX+TOGGLESX && lastXY.py<TOGGLEY+TOGGLESY)
		{
			action^=1;
		}else if(!action){oamSub.oamMemory[buttonSprite+2*0].attribute[0] = ATTR0_DISABLED;oamSub.oamMemory[buttonSprite+2*0+1].attribute[0] = ATTR0_DISABLED;}//else VRAM_I_EXT_SPR_PALETTE[0][TOGGLECOL]=RGB15(31,31,31);
		if(action || ((keysHeld() & KEY_TOUCH) && thisXY.px>=TOGGLEX && thisXY.py>=TOGGLEY && thisXY.px<TOGGLEX+TOGGLESX && thisXY.py<TOGGLEY+TOGGLESY))
		{
			// VRAM_I_EXT_SPR_PALETTE[0][TOGGLECOL]=RGB15(15,15,15);
			oamSub.oamMemory[buttonSprite+2*0].attribute[0] = ATTR0_COLOR_256 | ATTR0_WIDE | (0);
			oamSub.oamMemory[buttonSprite+2*0+1].attribute[0] = ATTR0_COLOR_256 | ATTR0_WIDE | (0);
		}
		
		if((keysHeld() & KEY_TOUCH) && thisXY.px>=INVX && thisXY.py>=INVY && thisXY.px<INVX+INVSX && thisXY.py<INVY+INVSY)
		{
			// VRAM_I_EXT_SPR_PALETTE[0][INVCOL]=RGB15(15,15,15);
			oamSub.oamMemory[buttonSprite+2*1].attribute[0] = ATTR0_COLOR_256 | ATTR0_WIDE | (0);
			oamSub.oamMemory[buttonSprite+2*1+1].attribute[0] = ATTR0_COLOR_256 | ATTR0_WIDE | (0);
		}else if((keysUp() & KEY_TOUCH) && lastXY.px>=INVX && lastXY.py>=INVY && lastXY.px<INVX+INVSX && lastXY.py<INVY+INVSY)
		{
			if(invOpen)
			{
				loadInterface("interface.bin",1);
				for(i=0;i<9;i++)
				{
					slots[i].position.x=IBARX+i*IBARD;
					slots[i].position.y=IBARY;
				}
				for(i=9;i<9+9*3;i++)
				{
					slots[i].used=false;
				}
			}
			else
			{
				loadInterface("inventory.bin",1);
				for(i=0;i<9;i++)
				{
					slots[i].position.x=INVENTORYBX+i*INVENTORYBOX;
					slots[i].position.y=INVENTORYBY;
				}
				for(i=9;i<9+9*3;i++)
				{
					slots[i].used=true;
				}
			}
			invOpen^=1;
		}else{oamSub.oamMemory[buttonSprite+2*1].attribute[0] = ATTR0_DISABLED;oamSub.oamMemory[buttonSprite+2*1+1].attribute[0] = ATTR0_DISABLED;}//else VRAM_I_EXT_SPR_PALETTE[0][INVCOL]=RGB15(31,31,31);
		
		if((keysHeld() & KEY_TOUCH) && thisXY.px>=SAVEX && thisXY.py>=SAVEY && thisXY.px<SAVEX+SAVESX && thisXY.py<SAVEY+SAVESY)
		{
			// VRAM_I_EXT_SPR_PALETTE[0][SAVECOL]=RGB15(15,15,15);
			oamSub.oamMemory[buttonSprite+2*2].attribute[0] = ATTR0_COLOR_256 | ATTR0_WIDE | (0);
			oamSub.oamMemory[buttonSprite+2*2+1].attribute[0] = ATTR0_COLOR_256 | ATTR0_WIDE | (0);
		}else if((keysUp() & KEY_TOUCH) && lastXY.px>=SAVEX && lastXY.py>=SAVEY && lastXY.px<SAVEX+SAVESX && lastXY.py<SAVEY+SAVESY)
		{
			globalSaveMap(&map);
		}else{oamSub.oamMemory[buttonSprite+2*2].attribute[0] = ATTR0_DISABLED;oamSub.oamMemory[buttonSprite+2*2+1].attribute[0] = ATTR0_DISABLED;}
	}
	// vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
	return !invOpen;
}

void startSave(void)
{
	oamSub.oamMemory[buttonSprite+2*3].attribute[0] = ATTR0_COLOR_256 | ATTR0_WIDE | (80);
	oamSub.oamMemory[buttonSprite+2*3+1].attribute[0] = ATTR0_COLOR_256 | ATTR0_WIDE | (80);
	oamUpdate(&oamSub);
}

void endSave(void)
{
	oamSub.oamMemory[buttonSprite+2*3].attribute[0] = ATTR0_DISABLED;
	oamSub.oamMemory[buttonSprite+2*3+1].attribute[0] = ATTR0_DISABLED;
	oamUpdate(&oamSub);
}
