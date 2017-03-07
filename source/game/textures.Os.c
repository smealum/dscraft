#include "game/game_main.h"

void Texture_GetGlWL(u16 width, u16 height, u8* w, u8* l)
{
	switch(width)
	{
		case 8:
			*w=TEXTURE_SIZE_8;
		break;
		case 16:
			*w=TEXTURE_SIZE_16;
		break;
		case 32:
			*w=TEXTURE_SIZE_32;
		break;
		case 64:
			*w=TEXTURE_SIZE_64;
		break;
		case 128:
			*w=TEXTURE_SIZE_128;
		break;
		case 256:
			*w=TEXTURE_SIZE_256;
		break;
		case 512:
			*w=TEXTURE_SIZE_512;
		break;
	}
	
	switch(height)
	{
		case 8:
			*l=TEXTURE_SIZE_8;
		break;
		case 16:
			*l=TEXTURE_SIZE_16;
		break;
		case 32:
			*l=TEXTURE_SIZE_32;
		break;
		case 64:
			*l=TEXTURE_SIZE_64;
		break;
		case 128:
			*l=TEXTURE_SIZE_128;
		break;
		case 256:
			*l=TEXTURE_SIZE_256;
		break;
		case 512:
			*l=TEXTURE_SIZE_512;
		break;
	}
}

void Game_InitVramBanks(u8 banks)
{
	int i;
	BANKS=banks;
	for(i=0;i<BANKS;i++)
	{
		Bank[i].s_total=128*1024;
		Bank[i].s_free=128*1024;
		Bank[i].s_used=0;
		Bank[i].num_t=0;
		Bank[i].addr=(void*)(0x6800000+0x0020000*i);
		//iprintf("%d : address %p\n",i,Bank[i].addr);
	}
	PalBank.s_total=64*1024;
	PalBank.s_free=64*1024;
	PalBank.s_used=0;
	PalBank.num_t=0;
	PalBank.addr=(void*)(0x6880000);
}

void Game_InitTextures()
{
	int i;
	for(i=0;i<MAX_TEX;i++)
	{
		Texture[i].used=false;
		Texture[i].ID=i;
	}
}

MTL_img* Game_CreateTexture(char* filename, char* directory)
{
	int i, j;
	FILE* file=DS_OpenFile(filename, directory, false, true);
	if(file==NULL)return NULL;
	else fclose(file);
	for(i=0;i<MAX_TEX;i++)
	{
		if(!Texture[i].used)
		{
			for(j=0;j<MAX_TEX;j++){if(Texture[j].used){if(!strcmp(Texture[j].name,filename)){NOGBA("STOP !!!");return &Texture[j];}}}
			Game_LoadTexturePCX(filename, directory, &Texture[i]);
			return &Texture[i];
		}
	}
	return NULL;
}

MTL_img* Game_CreateTextureAlpha(char* filename, char* directory, u8 alpha)
{
	int i, j;
	FILE* file=DS_OpenFile (filename, directory, false, true);
	if(file==NULL)return NULL;
	else fclose(file);
	for(i=0;i<MAX_TEX;i++)
	{
		if(!Texture[i].used)
		{
			for(j=0;j<MAX_TEX;j++){if(Texture[j].used){if(!strcmp(Texture[j].name,filename)){NOGBA("STOP !!!");return &Texture[j];}}}
			Game_LoadAlphaTexture(filename, directory, &Texture[i], alpha);
			return &Texture[i];
		}
	}
	return NULL;
}

MTL_img* Game_CreateAlphaMask(char* filename, char* directory)
{
	int i, j;
	FILE* file=DS_OpenFile (filename, directory, false, true);
	if(file==NULL)return NULL;
	else fclose(file);
	for(i=0;i<MAX_TEX;i++)
	{
		if(!Texture[i].used)
		{
			for(j=0;j<MAX_TEX;j++){if(Texture[j].used){if(!strcmp(Texture[j].name,filename)){NOGBA("STOP !!!");return &Texture[j];}}}
			Game_LoadAlphaMask(filename, directory, &Texture[i]);
			return &Texture[i];
		}
	}
	return NULL;
}

MTL_img* Game_CreateTextureAlphaMask(char* filename, char* directory, u8* alpha)
{
	int i, j;
	FILE* file=DS_OpenFile (filename, directory, false, true);
	if(file==NULL)return NULL;
	else fclose(file);
	for(i=0;i<MAX_TEX;i++)
	{
		if(!Texture[i].used)
		{
			for(j=0;j<MAX_TEX;j++){if(Texture[j].used){if(!strcmp(Texture[j].name,filename)){NOGBA("STOP !!!");return &Texture[j];}}}
			Game_LoadAlphaMaskTexture(filename, directory, &Texture[i], alpha);
			return &Texture[i];
		}
	}
	return NULL;
}

MTL_img* Game_CreateMaskedTexture(char* filename, char* directory)
{
	int i, j;
	FILE* file=DS_OpenFile (filename, directory, false, true);
	if(file==NULL)return NULL;
	else fclose(file);
	for(i=0;i<MAX_TEX;i++)
	{
		if(!Texture[i].used)
		{
			for(j=0;j<MAX_TEX;j++){if(Texture[j].used){if(!strcmp(Texture[j].name,filename)){NOGBA("STOP !!!");return &Texture[j];}}}
			Game_LoadMaskedTexture(filename, directory, &Texture[i]);
			return &Texture[i];
		}
	}
	return NULL;
}

MTL_img* Game_CreateTextureBuffer(u8* buffer, u16* buffer2, u16 x, u16 y)
{
	int i;
	for(i=0;i<MAX_TEX;i++)
	{
		if(!Texture[i].used)
		{
			Game_LoadTextureBuffer(buffer, buffer2, x, y, &Texture[i]);
			return &Texture[i];
		}
	}
	return NULL;
}

MTL_img* Game_CreateTextureBuffer16(u16* buffer, u16 x, u16 y, bool cpy)
{
	int i;
	for(i=0;i<MAX_TEX;i++)
	{
		if(!Texture[i].used)
		{
			Game_LoadTextureBuffer16(buffer, x, y, &Texture[i], true, cpy);
			return &Texture[i];
		}
	}
	return NULL;
}

void Game_GetVramStatus()
{
	int i;
	for(i=0;i<BANKS;i++)
	{
		//iprintf("%d : %d/%dko (%dko used)\n",i,Bank[i].s_free/1024,Bank[i].s_total/1024,Bank[i].s_used/1024);
		NOGBA("%d : %d/%dko (%dko used)\n",i,Bank[i].s_free/1024,Bank[i].s_total/1024,Bank[i].s_used/1024);
	}
}

void Texture_AddToBank(MTL_img *mtl, u8* data, int b)
{
	NOGBA("h : %d, %d, %p",Bank[b].s_used,mtl->size,mtl->addr);
	Bank[b].s_used+=mtl->size;
	Bank[b].s_free=Bank[b].s_total-Bank[b].s_used;
	Bank[b].num_t++;
	
	u32 vramTemp = vramSetPrimaryBanks(VRAM_A_LCD,VRAM_B_LCD,VRAM_C_LCD,VRAM_D_LCD);
		memcpy(mtl->addr, data, mtl->size);
	vramRestorePrimaryBanks(vramTemp);
}

void Texture_ReserveInBank(MTL_img *mtl, u8* data, int b)
{
	NOGBA("h : %d, %d, %p",Bank[b].s_used,mtl->size,mtl->addr);
	Bank[b].s_used+=mtl->size;
	Bank[b].s_free=Bank[b].s_total-Bank[b].s_used;
	Bank[b].num_t++;
}

//function from libnds
void Texture_SetParameter(MTL_img *mtl, uint8 sizeX, uint8 sizeY, const uint32* addr, GL_TEXTURE_TYPE_ENUM mode, uint32 param)
{
	mtl->param = param | (sizeX << 20) | (sizeY << 23) | (((uint32)addr >> 3) & 0xFFFF) | (mode << 26);
}

void* Texture_GetTexAddress(MTL_img *mtl)
{
	int i;
	for(i=0;i<BANKS;i++)
	{
		if(Bank[i].s_free>=mtl->size)
		{
			mtl->bank=i;
			mtl->addr=(void*)(Bank[i].addr+0x1*Bank[i].s_used);
			return mtl->addr;
		}
	}
	return 0x0;
}

void Palette_AddToBank(MTL_img *mtl, u16* data, size_t size)
{
	mtl->pal=(void*)(PalBank.addr+0x1*PalBank.s_used);
	PalBank.s_used+=size;
	PalBank.s_free=PalBank.s_total-PalBank.s_used;
	PalBank.num_t++;
	NOGBA("palettes : %d (%p)\n",PalBank.num_t,mtl->pal);
	
	vramSetBankE(VRAM_E_LCD);
		memcpy(mtl->pal, data, size);
	vramSetBankE(VRAM_E_TEX_PALETTE);
}

		
		/*//DEBUG
		int i;
		for(i=0;i<size/2;i++)((u16*)mtl->pal)[i]=RGB15(31,31,31);
		//END DEBUG*/

void Palette_Bind(MTL_img *mtl)
{
		GFX_PAL_FORMAT = ((uint32)mtl->pal)>>(4);
}

void Palette_Bind4(MTL_img *mtl)
{
		GFX_PAL_FORMAT = ((uint32)mtl->pal)>>(4-1);
}

void Texture_Bind(MTL_img *mtl)
{
	if(mtl)GFX_TEX_FORMAT = mtl->param;
	else Texture_Unbind();
}

void Texture_Unbind()
{
		GFX_TEX_FORMAT = 0; 
}

/*void Game_LoadAlphaMask(char* filename, char* directory, MTL_img *mtl)
{
	int param;
	bool transp=false;
	uint8 texX=0, texy=0;
	int i, j;
	u8 *buffer, *b;
	
	struct gl_texture_t *pcxt=(struct gl_texture_t *)ReadPCXFile(filename,directory);
	mtl->width=pcxt->width;
	mtl->height=pcxt->height;
	mtl->size=mtl->width*mtl->height;
	// NOGBA("format : %d %d %d %p",pcxt->format,mtl->width,mtl->height,pcxt->texels);
	
		pcxt->palette[0]=0;
		pcxt->palette[1]=RGB15(31,31,31);
		
		for(i=0;i<mtl->width;i++)
		{
			for(j=0;j<mtl->height;j++)
			{
				u8 alpha=31-(pcxt->texels[i+j*mtl->width]*2);
				// u8 alpha=31;
				u8 color=1;
				pcxt->texels[i+j*mtl->width]=(color&7)|(((alpha)<<3));
				// if(!color && transp)b[i+j*mtl->width]=(color&7)|(((0)<<3));
			}
		}
		
		Palette_AddToBank(mtl, pcxt->palette, 8*2);
		Texture_GetTexAddress(mtl);
		Texture_GetGlWL(mtl->width, mtl->height, &texX, &texy);
		param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | (1<<29);
		Texture_SetParameter(mtl, texX, texy, mtl->addr, GL_RGB8_A5, param);
		mtl->palbind = ((uint32)mtl->pal)>>(4);

	Texture_AddToBank(mtl, pcxt->texels, mtl->bank);
	
	freePCX(pcxt);
	// sImage pcx;
	
	// buffer=DS_OpenFile(filename, directory, true, true);

	// loadPCX((u8*)buffer, &pcx);
	
	// free(buffer);

	mtl->used=true;
	mtl->width=pcx.width;
	mtl->height=pcx.height;
	mtl->size=mtl->width*mtl->height;
	
	b=malloc(mtl->size);
	if(pcx.palette[0]==RGB15(31,0,31))transp=true;
	pcx.palette[0]=0;
	pcx.palette[1]=RGB15(31,31,31);
	
	for(i=0;i<mtl->width;i++)
	{
		for(j=0;j<mtl->width;j++)
		{
			// u8 alpha=31-(pcx.image.data8[i+j*mtl->width]*2);
			u8 alpha=31;
			u8 color=1;
			b[i+j*mtl->width]=(color&7)|(((alpha)<<3));
			if(!color && transp)b[i+j*mtl->width]=(color&7)|(((0)<<3));
		}
	}
	
	Palette_AddToBank(mtl, pcx.palette, 8*2);
	param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | (1<<29);
	
	Texture_GetTexAddress(mtl);
	
	Texture_GetGlWL(mtl->width, mtl->height, &texX, &texy);
	
	Texture_SetParameter(mtl, texX, texy, mtl->addr, GL_RGB8_A5, param);

	Texture_AddToBank(mtl, b, mtl->bank);
	
	mtl->palbind = ((uint32)mtl->pal)>>(4);
	// imageDestroy(&pcx);
	// free(b);
}*/

void Game_LoadAlphaMask(char* filename, char* directory, MTL_img *mtl)
{
	int param;
	bool transp=false;
	uint8 texX=0, texy=0;
	int i, j;
	u8 *buffer, *b;
	sImage pcx;
	
	buffer=DS_OpenFile(filename, directory, true, true);

	loadPCX((u8*)buffer, &pcx);
	
	free(buffer);

	mtl->used=true;
	mtl->width=pcx.width;
	mtl->height=pcx.height;
	mtl->size=mtl->width*mtl->height;
	
	b=malloc(mtl->size);
	if(pcx.palette[0]==RGB15(31,0,31))transp=true;
	// NOGBA("lalala %d %d %d",mtl->width,mtl->height,mtl->size);
	
	for(i=0;i<mtl->width;i++)
	{
		for(j=0;j<mtl->height;j++)
		{
			u8 alpha=31-(pcx.image.data8[i+j*mtl->width]*2);
			// u8 alpha=31;
			if(!pcx.palette[pcx.image.data8[i+j*mtl->width]])alpha=0;
			u8 color=1;
			b[i+j*mtl->width]=(color&7)|(((alpha)<<3));
			if(!color && transp)b[i+j*mtl->width]=(color&7)|(((0)<<3));
		}
	}
	pcx.palette[0]=0;
	pcx.palette[1]=RGB15(31,31,31);
	
	Palette_AddToBank(mtl, pcx.palette, 8*2);
	param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | (1<<29);
	
	Texture_GetTexAddress(mtl);
	
	Texture_GetGlWL(mtl->width, mtl->height, &texX, &texy);
	
	Texture_SetParameter(mtl, texX, texy, mtl->addr, GL_RGB8_A5, param);

	Texture_AddToBank(mtl, b, mtl->bank);
	
	mtl->palbind = ((uint32)mtl->pal)>>(4);
	imageDestroy(&pcx);
	free(b);
}

void Game_LoadAlphaMaskTexture(char* filename, char* directory, MTL_img *mtl, u8* alpha)
{
	int param;
	bool transp=false;
	uint8 texX=0, texy=0;
	int i, j;
	u8 *buffer, *b;
	sImage pcx;
	
	// chdir("textures");
	buffer=DS_OpenFile(filename, directory, true, true);
	// chdir("..");

	loadPCX((u8*)buffer, &pcx);
	
	free(buffer);

	mtl->used=true;
	mtl->width=pcx.width;
	mtl->height=pcx.height;
	mtl->size=mtl->width*mtl->height;
	
	b=malloc(mtl->size);
	if(pcx.palette[0]==RGB15(31,0,31))transp=true;
	
	for(i=0;i<mtl->width;i++)
	{
		for(j=0;j<mtl->height;j++)
		{
			// u8 alpha=alpha;
			u8 color=pcx.image.data8[i+j*mtl->width]%8;
			b[i+j*mtl->width]=(color&7)|(((alpha[i+j*mtl->width])<<3));
			if(!color && transp)b[i+j*mtl->width]=(color&7)|(((0)<<3));
		}
	}
	
	Palette_AddToBank(mtl, pcx.palette, 8*2);
	param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | (1<<29);
	
	Texture_GetTexAddress(mtl);
	
	Texture_GetGlWL(mtl->width, mtl->height, &texX, &texy);
	
	Texture_SetParameter(mtl, texX, texy, mtl->addr, GL_RGB8_A5, param);

	Texture_AddToBank(mtl, b, mtl->bank);
	
	mtl->palbind = ((uint32)mtl->pal)>>(4);
	imageDestroy(&pcx);
	free(b);
}

void Game_LoadAlphaTexture(char* filename, char* directory, MTL_img *mtl, u8 alpha)
{
	int param;
	bool transp=false;
	uint8 texX=0, texy=0;
	int i, j;
	u8 *buffer, *b;
	sImage pcx;
	
	// chdir("textures");
	buffer=DS_OpenFile(filename, directory, true, true);
	// chdir("..");

	loadPCX((u8*)buffer, &pcx);
	
	free(buffer);

	mtl->used=true;
	mtl->width=pcx.width;
	mtl->height=pcx.height;
	mtl->size=mtl->width*mtl->height;
	
	b=malloc(mtl->size);
	if(pcx.palette[0]==RGB15(31,0,31))transp=true;
	
	for(i=0;i<mtl->width;i++)
	{
		for(j=0;j<mtl->height;j++)
		{
			// u8 alpha=alpha;
			u8 color=pcx.image.data8[i+j*mtl->width]%8;
			b[i+j*mtl->width]=(color&7)|(((alpha)<<3));
			if(!color && transp)b[i+j*mtl->width]=(color&7)|(((0)<<3));
		}
	}
	
	Palette_AddToBank(mtl, pcx.palette, 8*2);
	param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | (1<<29);
	
	Texture_GetTexAddress(mtl);
	
	Texture_GetGlWL(mtl->width, mtl->height, &texX, &texy);
	
	Texture_SetParameter(mtl, texX, texy, mtl->addr, GL_RGB8_A5, param);

	Texture_AddToBank(mtl, b, mtl->bank);
	
	mtl->palbind = ((uint32)mtl->pal)>>(4);
	imageDestroy(&pcx);
	free(b);
}

void Game_LoadMaskedTexture(char* filename, char* directory, MTL_img *mtl)
{
	int param;
	bool transp=false;
	uint8 texX=0, texy=0;
	int i, j;
	u8 *buffer, *buffer2, *b;
	sImage pcx, pcx2;
	char str[255];
	strcpy(str,filename);
	sprintf(&str[strlen(str)-4],"mask.pcx");
	
	// chdir("textures");
	buffer=DS_OpenFile(filename, directory, true, true);
	buffer2=DS_OpenFile(str, directory, true, true);
	// chdir("..");

	loadPCX((u8*)buffer, &pcx);
	loadPCX((u8*)buffer2, &pcx2);
	
	free(buffer);
	free(buffer2);

	mtl->used=true;
	mtl->width=pcx.width;
	mtl->height=pcx.height;
	mtl->size=mtl->width*mtl->height;
	
	b=malloc(mtl->size);
	if(pcx.palette[0]==RGB15(31,0,31))transp=true;
	
	for(i=0;i<256;i++)pcx2.palette[i]=((pcx2.palette[i]&31)+((pcx2.palette[i]>>5)&31)+((pcx2.palette[i]>>10)&31))/3;
	for(i=0;i<mtl->width;i++)
	{
		for(j=0;j<mtl->height;j++)
		{
			u8 alpha=(pcx2.palette[pcx2.image.data8[i+j*mtl->width]])%32;
			u8 color=pcx.image.data8[i+j*mtl->width]%8;
			b[i+j*mtl->width]=(color&7)|(((alpha)<<3));
			if(!color && transp)b[i+j*mtl->width]=(color&7)|(((0)<<3));
		}	
	}
		
	Palette_AddToBank(mtl, pcx.palette, 8*2);
	param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | (1<<29);
	
	Texture_GetTexAddress(mtl);
	
	Texture_GetGlWL(mtl->width, mtl->height, &texX, &texy);
	
	Texture_SetParameter(mtl, texX, texy, mtl->addr, GL_RGB8_A5, param);

	Texture_AddToBank(mtl, b, mtl->bank);
	
	imageDestroy(&pcx);
	imageDestroy(&pcx2);
	free(b);
}

void Game_GenerateDegrad(MTL_img *mtl)
{
	int param;
	uint8 texX=8, texy=32;
	int i, j;
	u8 buffer[texX*texy];
	u16 palette[8];
	
	for(i=0;i<8;i++)palette[i]=RGB15(31,31,31);
	for(i=0;i<texX;i++)
	{
		for(j=0;j<texy;j++)
		{
			u8 alpha=(31-j);
			u8 color=0;
			buffer[i+j*texX]=(color&7)|(((alpha)<<3));
		}	
	}
	
	mtl->used=true;

	mtl->width=texX;
	mtl->height=texy;
	mtl->size=mtl->width*mtl->height;
		
	Palette_AddToBank(mtl, palette, 8*2);
	param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | (1<<29);
	
	Texture_GetTexAddress(mtl);
	
	Texture_GetGlWL(mtl->width, mtl->height, &texX, &texy);
	
	Texture_SetParameter(mtl, texX, texy, mtl->addr, GL_RGB8_A5, param);

	Texture_AddToBank(mtl, buffer, mtl->bank);
}

void Game_LoadTextureBuffer(u8* buffer, u16* buffer2, u16 x, u16 y, MTL_img *mtl)
{
	int param;
	uint8 texX=0, texy=0;
	
	mtl->used=true;

	mtl->width=x;
	mtl->height=y;	
	mtl->size=mtl->width*mtl->height;
		
	Palette_AddToBank(mtl, buffer2, 256*2);
	if(buffer2[0]==RGB15(31,0,31))param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | (1<<29);
	else param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T;	
	
	Texture_GetTexAddress(mtl);
	Texture_GetGlWL(mtl->width, mtl->height, &texX, &texy);
	
	Texture_SetParameter(mtl, texX, texy, mtl->addr, GL_RGB256, param);

	Texture_AddToBank(mtl, buffer, mtl->bank);
}

void Game_LoadTextureBuffer16(u16* buffer, u16 x, u16 y, MTL_img *mtl, bool genaddr, bool cpy)
{
	int param;
	uint8 texX=0, texy=0;
	
	mtl->used=true;

	mtl->width=x;
	mtl->height=y;	
	mtl->size=mtl->width*mtl->height*2;
	
	param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T;	
	
	if(genaddr)Texture_GetTexAddress(mtl);
	Texture_GetGlWL(mtl->width, mtl->height, &texX, &texy);
	
	Texture_SetParameter(mtl, texX, texy, mtl->addr, GL_RGBA, param);

	if(cpy)Texture_AddToBank(mtl, (u8*)buffer, mtl->bank);
	else Texture_ReserveInBank(mtl, (u8*)buffer, mtl->bank);
}

struct gl_texture_t * ReadPCXFile (const char *filename, char* directory);

void Game_LoadTexturePCX(char* filename, char* directory, MTL_img *mtl)
{
	int param;
	// sImage pcx; 
	// u8 *buffer;
	uint8 texX=0, texy=0;
	
	mtl->used=true;
	mtl->name=DS_mAlloc(strlen(filename)+1,&Game_State);
	strcpy(mtl->name,filename);
	
	DS_Debug("loading %s.\n",filename);
	
	// chdir("textures");
	struct gl_texture_t *pcxt=(struct gl_texture_t *)ReadPCXFile(filename,directory);
	// buffer=DS_OpenFile(filename, directory, true, true);
	// chdir("..");

	// loadPCX((u8*)buffer, &pcx);

	// mtl->width=pcx.width;
	// mtl->height=pcx.height;
	mtl->width=pcxt->width;
	mtl->height=pcxt->height;
	
	
	if(pcxt->palette[0]==RGB15(31,0,31))param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | (1<<29);
	else param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T;	
	
	NOGBA("format : %d",pcxt->format);
	switch(pcxt->format)
	{
		case 4:
			Palette_AddToBank(mtl, pcxt->palette, 16*2);
			mtl->size=(mtl->width*mtl->height)/2;
			Texture_GetTexAddress(mtl);
			Texture_GetGlWL(mtl->width, mtl->height, &texX, &texy);
			Texture_SetParameter(mtl, texX, texy, mtl->addr, GL_RGB16, param);
			mtl->palbind = ((uint32)mtl->pal)>>(4-1);
			break;
		case 8:
			Palette_AddToBank(mtl, pcxt->palette, 256*2);
			mtl->size=mtl->width*mtl->height;
			Texture_GetTexAddress(mtl);
			Texture_GetGlWL(mtl->width, mtl->height, &texX, &texy);
			Texture_SetParameter(mtl, texX, texy, mtl->addr, GL_RGB256, param);
			mtl->palbind = ((uint32)mtl->pal)>>(4);
			break;
	}

	// Texture_AddToBank(mtl, pcx.image.data8, mtl->bank);
	Texture_AddToBank(mtl, pcxt->texels, mtl->bank);
	
	freePCX(pcxt);
	
	// imageDestroy(&pcx);
	// free(buffer);
	DS_Debug("%s loaded.\n",filename);
}

void* Game_LoadPalettePCX(char* filename, char* directory)
{
	int param;
	// sImage pcx; 
	// u8 *buffer;
	uint8 texX=0, texy=0;
	MTL_img mtl;
	
	DS_Debug("loading %s.\n",filename);
	
	// chdir("textures");
	struct gl_texture_t *pcxt=(struct gl_texture_t *)ReadPCXFile(filename,directory);
	// buffer=DS_OpenFile(filename, directory, true, true);
	// chdir("..");

	// loadPCX((u8*)buffer, &pcx);

	// mtl->width=pcx.width;
	// mtl->height=pcx.height;
	mtl.width=pcxt->width;
	mtl.height=pcxt->height;
	
	NOGBA("format : %d",pcxt->format);
	switch(pcxt->format)
	{
		case 4:
			Palette_AddToBank(&mtl, pcxt->palette, 16*2);
			break;
		case 8:
			Palette_AddToBank(&mtl, pcxt->palette, 256*2);
			break;
	}
	
	freePCX(pcxt);
	
	DS_Debug("%s's palette loaded.\n",filename);
	return mtl.pal;
}

/*void Game_GenerateRipple(MTL_img *mtl)
{
	int param;
	uint8 texX=64, texy=64;
	u16 start=300, end=1000;
	int i, j;
	u8 buffer[texX*texy];
	u16 palette[8];
	
	for(i=0;i<8;i++)palette[i]=RGB15(31-(i*31)/8,31-(i*31)/8,31-(i*31)/8);
	for(i=0;i<texX;i++)
	{
		for(j=0;j<texy;j++)
		{
			int dist=(texX/2-i)*(texX/2-i)+(texy/2-j)*(texy/2-j);
			u8 alpha=(31-abs(31*(((end+start)/2)-dist)/((end-start)/2)));
			if(dist>end || dist<start)alpha=0;
			u8 color=0;
			color=min(abs(7*(((end+start)/2)-dist)/((end-start)/2)),7);
			buffer[i+j*texX]=(color&7)|(((alpha)<<3));
		}	
	}
	
	mtl->used=true;

	mtl->width=texX;
	mtl->height=texy;	
	mtl->size=mtl->width*mtl->height;
		
	Palette_AddToBank(mtl, palette, 8*2);
	param=TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T | (1<<29);
	
	Texture_GetTexAddress(mtl);
	Texture_GetGlWL(mtl->width, mtl->height, &texX, &texy);
	
	Texture_SetParameter(mtl, texX, texy, mtl->addr, GL_RGB8_A5, param);

	Texture_AddToBank(mtl, buffer, mtl->bank);
}*/

void Game_ApplyMTL(MTL_img *mtl)
{
	Palette_Bind(mtl);
	Texture_Bind(mtl);
}
