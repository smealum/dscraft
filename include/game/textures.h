#ifndef __TEXTURES9__
#define __TEXTURES9__

#define MAX_TEX 128
#define BANKS vramBanks

u8 vramBanks;

typedef struct
{
	size_t s_total, s_used, s_free;
	void* addr;
	int num_t;
}VRAM_bank;

typedef struct
{
	u16 width, height;
	int ID;
	int bank;
	size_t size;
	u32 param;
	char* name;
	void *addr, *pal;
	u32 palbind;
	bool used;
}MTL_img;

/* OpenGL texture info */
struct gl_texture_t
{
  u16 width;
  u16 height;

  int format;
  int internalFormat;
  u32 id;

  u8 *texels;
  u16 *palette;
};

#pragma pack(1)
/* PCX header */
struct pcx_header_t
{
  u8 manufacturer;
  u8 version;
  u8 encoding;
  u8 bitsPerPixel;

  u16 xmin, ymin;
  u16 xmax, ymax;
  u16 horzRes, vertRes;

  u8 palette[48];
  u8 reserved;
  u8 numColorPlanes;

  u16 bytesPerScanLine;
  u16 paletteType;
  u16 horzSize, vertSize;

  u8 padding[54];
};
#pragma pack(4)

VRAM_bank Bank[4];
VRAM_bank PalBank;

MTL_img Texture[MAX_TEX];


static inline void Game_FastBind(MTL_img *mtl)
{
	GFX_PAL_FORMAT = mtl->palbind;
	GFX_TEX_FORMAT = mtl->param;
}

void Game_InitTextures();
void* Game_LoadPalettePCX(char* filename, char* directory);
MTL_img* Game_CreateTexture(char* filename, char* directory);
MTL_img* Game_CreateTextureBuffer16(u16* buffer, u16 x, u16 y, bool cpy);
void Game_LoadTexturePCX(char* filename, char* directory, MTL_img *mtl);
MTL_img* Game_CreateTextureBuffer(u8* buffer, u16* buffer2, u16 x, u16 y);
MTL_img* Game_CreateTextureAlpha(char* filename, char* directory, u8 alpha);
void Game_LoadTextureBuffer16(u16* buffer, u16 x, u16 y, MTL_img *mtl, bool genaddr, bool cpy);
void Game_LoadTextureBuffer(u8* buffer, u16* buffer2, u16 x, u16 y, MTL_img *mtl);
void Game_GenerateRipple(MTL_img *mtl);
void Game_GenerateDegrad(MTL_img *mtl);
void Game_ApplyMTL(MTL_img *mtl);
void Game_InitVramBanks(u8 banks);
void Game_GetVramStatus();
void Texture_Unbind();
void Texture_AddToBank(MTL_img *mtl, u8* data, int b);
void Texture_SetParameter(MTL_img *mtl, uint8 sizeX, uint8 sizeY, const uint32* addr, GL_TEXTURE_TYPE_ENUM mode, uint32 param);
void* Texture_GetTexAddress(MTL_img *mtl);
void Palette_AddToBank(MTL_img *mtl, u16* data, size_t size);
void Palette_Bind(MTL_img *mtl);
void Palette_Bind4(MTL_img *mtl);
void Texture_Bind(MTL_img *mtl);
void Texture_GetGlWL(u16 width, u16 height, u8* w, u8* l);

#endif