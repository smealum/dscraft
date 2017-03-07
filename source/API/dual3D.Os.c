#include "common/general.h"
#include "API/dual3D.h"

pSpriteRotation D3D_spriteRotations = (pSpriteRotation)D3D_sprites;

void D3D_InitSprites(void)
{
	int i;
	for(i = 0; i < 128; i++)
	{
	   D3D_sprites[i].attribute[0] = ATTR0_DISABLED;
	   D3D_sprites[i].attribute[1] = 0;
	   D3D_sprites[i].attribute[2] = 0;
    }
}

void D3D_UpdateOAM(void)
{
	DC_FlushRange(D3D_sprites, 128 * sizeof(SpriteEntry));
	memcpy(OAM_SUB, D3D_sprites, 128 * sizeof(SpriteEntry));
}

void Game_InitD3D()
{
	int x,y,i;
	D3D_InitSprites();
	
	D3D_spriteRotations[0].hdx=256;
	D3D_spriteRotations[0].hdy=0;
	D3D_spriteRotations[0].vdx=0;
	D3D_spriteRotations[0].vdy=256;
	
	i=0;
	
	for (y = 0; y < 3; y++)
	{
		for (x = 0; x < 4; x++) {
			D3D_sprites[i].attribute[0] = ATTR0_BMP | ATTR0_SQUARE | (64 * y);
			D3D_sprites[i].attribute[1] = ATTR1_SIZE_64 | (64 * x);
			D3D_sprites[i].attribute[2] = ATTR2_ALPHA(1) | (8 * 32 * y) | (8 * x);
			i++;
		}
	}
	
	D3D_UpdateOAM();
	D3D_Screen=true;
}

//not my code
void D3D_SetRegCapture(bool enable, uint8 srcBlend, uint8 destBlend, uint8 bank, uint8 offset, uint8 size, uint8 source, uint8 srcOffset)
{
	uint32 value=0;

	if(enable)value|=1 << 31; // 31 is enable
	value|=3 << 29; // 29-30 seems to have something to do with the blending
	value|=(srcOffset & 0x3) << 26; // capture source offset is 26-27
	value|=(source & 0x3) << 24; // capture source is 24-25
	value|=(size & 0x3) << 20; // capture data write size is 20-21
	value|=(offset & 0x3) << 18; // write offset is 18-19
	value|=(bank & 0x3) << 16; // vram bank select is 16-17
	value|=(srcBlend & 0xF) << 8; // graphics blend evb is 8..12
	value|=(destBlend & 0x1F) << 0; // ram blend EVA is bits 0..4
	REG_DISPCAPCNT=value;
}

void Game_UpdateD3D()
{
	if (D3D_Screen) {
		vramSetBankC(VRAM_C_SUB_BG);
		vramSetBankD(VRAM_D_LCD);
		// D3D_SetRegCapture(true, 0, 15, 3, 0, 3, 0, 0);
		D3D_SetRegCapture(true, 0, 16, 3, 0, 3, 0, 0);
		D3D_Screen=false;
	}else{
		vramSetBankC(VRAM_C_LCD);
		vramSetBankD(VRAM_D_SUB_SPRITE);
		// D3D_SetRegCapture(true, 0, 15, 2, 0, 3, 0, 0);
		D3D_SetRegCapture(true, 0, 16, 2, 0, 3, 0, 0);
		D3D_Screen=true;
	}
}

