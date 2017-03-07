#include "game/game_main.h"

#define minus(a) (((a)<0)?((a)+1):((a)-1))
	
void Game_Init(void)
{
	int i;
	// defaultExceptionHandler();
	lcdMainOnTop();

	initFilesystem();

	//set mode 0, enable BG0 and set it to 3D
	videoSetMode(MODE_3_3D | DISPLAY_BG3_ACTIVE);
	#ifdef DEBUGMODE
		videoSetModeSub(MODE_0_2D | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_2D_BMP_256 | DISPLAY_SPR_EXT_PALETTE | DISPLAY_BG0_ACTIVE);
		// consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);
	#else
		videoSetModeSub(MODE_0_2D | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_2D_BMP_256 | DISPLAY_SPR_EXT_PALETTE);
	#endif
	// videoSetModeSub(MODE_0_2D | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_2D_BMP_256 | DISPLAY_SPR_EXT_PALETTE);
	vramSetBankA(VRAM_A_TEXTURE);
	vramSetBankB(VRAM_B_LCD);
	// vramSetBankC(VRAM_C_LCD);
	// vramSetBankC(VRAM_C_SUB_BG);
	vramSetBankC(VRAM_C_MAIN_BG_0x06000000);
	vramSetBankD(VRAM_D_SUB_SPRITE);
	vramSetBankE(VRAM_E_TEX_PALETTE);
	vramSetBankF(VRAM_F_LCD);
	vramSetBankG(VRAM_G_LCD);
	vramSetBankH(VRAM_H_SUB_BG);
	// vramSetBankI(VRAM_I_SUB_BG_0x06208000);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
	
	// consoleDemoInit();
	// consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);
	#ifdef DEBUGMODE
		consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);
	#endif
	#ifdef DEBUGMODE2
		videoSetModeSub(MODE_0_2D | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_2D_BMP_256 | DISPLAY_SPR_EXT_PALETTE | DISPLAY_BG0_ACTIVE);
		consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);
	#endif
	
	initInterface();
	
	REG_BG3CNT = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY(1);
        REG_BG3PA = 1 << 8;
        REG_BG3PB = 0;
        REG_BG3PC = 0;
        REG_BG3PD = 1 << 8;
		
        REG_BG3X = 0;
        REG_BG3Y = 0;
	REG_BG0CNT = BG_PRIORITY(0);

	// initialize gl
	glReInit();
	
	// enable antialiasing
	// glEnable(GL_ANTIALIAS);
	glDisable(GL_ANTIALIAS);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	
	setFog(0);

	Game_InitVramBanks(1);
	Game_InitTextures();

	// sImage pcx;
	// loadPCX((u8*)grass_pcx, &pcx);
	// image8to16(&pcx);
	// glGenTextures(1, &testTex);
	// glBindTexture(0, testTex);
	// glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_32 , TEXTURE_SIZE_32, 0, TEXGEN_TEXCOORD, (u8*)pcx.image.data8);
	// imageDestroy(&pcx);
	// loadPCX((u8*)grasstop_pcx, &pcx);
	// image8to16(&pcx);
	// glGenTextures(1, &testTexTop);
	// glBindTexture(0, testTexTop);
	// glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_32 , TEXTURE_SIZE_32, 0, TEXGEN_TEXCOORD, (u8*)pcx.image.data8);
	// imageDestroy(&pcx);
	
	// setup the rear plane
	// glClearColor(0,0,24,31); // BG must be opaque for AA to work
	glClearColor(0,0,0,0); // BG must be opaque for AA to work
	glClearPolyID(63); // BG must have a unique polygon ID for AA to work
	glClearDepth(0x7FFF);
	
/*	
	glMaterialf(GL_AMBIENT, RGB15(16,16,16));
	glMaterialf(GL_DIFFUSE, RGB15(31,31,31));
	glMaterialf(GL_SPECULAR, BIT(15) | RGB15(18,18,18));
	glMaterialf(GL_EMISSION, RGB15(0,0,0));
	
	//ds uses a table for shinyness..this generates a half-ass one
	glMaterialShinyness();
*/
	//this should work the same as the normal gl call
	glViewport(0,0,255,191);

	//any floating point gl call is being converted to fixed prior to being implemented
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// gluPerspective(70, 256.0 / 192.0, 0.1, 100);
	gluPerspective(70, 256.0 / 192.0, 0.1, 10000);
	
	gluLookAt(	0.0, 0.0, 0.0,		//camera possition
				0.0, 1.0, 0.0,		//look at
				0.0, 0.0, 1.0);		//up
	
	initControls();
	// initPlayer(&Player);
	// updatePlayer(&Player);
	
	initEnvironment(false);
	loadBlockTextures(true, true);
	// initMap(&map, (vect3D){32,32,16});
	// initMap(&map, (vect3D){64,64,16});
	// initMap(&map, (vect3D){128,128,16});
	// initMap(&map, (vect3D){256,256,16});
	// initMap(&map, (vect3D){16,16,16});
	
	// createTestMap(&map);
	
	// generateTestMap(&map);
	
	loadTestMap(&map);
	
	// generateDisplayLists(&map);
	// generateMapQuadList(&map);
	
	// initSuperCluster(&map);
		
		testCursorI=map.size.x/2;
		testCursorJ=map.size.y/2;
		testCursorK=map.size.z/2 + 2;

	NOGBA("RAM : %dko used, %dko free    \n",DS_UsedMem()/1024,DS_FreeMem()/1024);
	Game_GetVramStatus();
	
	// int i;
		vramSetBankB(VRAM_B_LCD);
		vramSetBankC(VRAM_C_LCD);
	for(i=0;i<256*192;i++){VRAM_B[i]=0;VRAM_C[i]=0;}
	loadInterface("interface.bin", 1);
	initPlayer(&Player);
	#ifdef SURVIVAL
		initMobs();
		addMob(&map, Player.position, 0);
		addMob(&map, Player.position, 0);
		addMob(&map, Player.position, 0);
		addMob(&map, Player.position, 0);
		addMob(&map, Player.position, 0);
		addMob(&map, Player.position, 0);
		addMob(&map, Player.position, 0);
		addMob(&map, Player.position, 0);
		addMob(&map, Player.position, 0);
		addMob(&map, Player.position, 0);
	#endif
	testBuffer=false;
}

void SetRegCapture(bool enable, uint8 srcBlend, uint8 destBlend, uint8 bank, uint8 offset, uint8 size, uint8 source, uint8 srcOffset)
{
	uint32 value=0;

	if(enable)value|=1 << 31; // 31 is enable
	value|=0 << 29; // 29-30 seems to have something to do with the blending   //3
	value|=(srcOffset & 0x3) << 26; // capture source offset is 26-27
	value|=(source & 0x3) << 24; // capture source is 24-25
	value|=(size & 0x3) << 20; // capture data write size is 20-21
	value|=(offset & 0x3) << 18; // write offset is 18-19
	value|=(bank & 0x3) << 16; // vram bank select is 16-17
	value|=(srcBlend & 0x1F) << 8; // graphics blend evb is 8..12
	value|=(destBlend & 0x1F) << 0; // ram blend EVA is bits 0..4
	REG_DISPCAPCNT=value;
}

void testScreenshot(void)
{
	// int i, j;
	// u16* image=malloc(2*256*192);
	// for(i=0;i<256*192;i++)
	// {
		// image[i]=VRAM_B[i];
	// }
		vramSetBankB(VRAM_B_LCD);
		vramSetBankC(VRAM_C_LCD);
	Debug_TakeScreenshotBMP(VRAM_B, VRAM_C);
	// chdir("fat:/");
	// FILE* f=fopen("prout2.png","wb");
	// fclose(f);
	// LodePNG_encode24_file("prout.png", image, 256, 192);
	// free(image);
}

void Game_Frame(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70, 256.0 / 192.0, 0.1, 10000);
				
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	if(testBuffer)
	{
		Game_FrameCount++;
		if((keysDown() & KEY_SELECT))testScreenshot();
		vramSetBankB(VRAM_B_MAIN_BG_0x06000000);
		vramSetBankC(VRAM_C_LCD);
		SetRegCapture(true, 0, 16, 2, 0, 3, 1, 0);
		drawSky();
		REG_BG0CNT = BG_PRIORITY(0);
		REG_BG3CNT = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY(1);
		testBuffer=false;
	}else{
		// if((keysDown() & KEY_B))Debug_TakeScreenshotBMP(VRAM_C);
		vramSetBankB(VRAM_B_LCD);
		vramSetBankC(VRAM_C_MAIN_BG_0x06000000);
		SetRegCapture(true, 0, 16, 1, 0, 3, 1, 0);
		glClearColor(0,0,0,0);
		REG_BG0CNT = BG_PRIORITY(1);
		REG_BG3CNT = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY(0);
		testBuffer=true;
	}
	
	// if(!testBuffer)updatePlayer(&Player);

	playerCamera(&Player, false);
	// glLight(0, RGB15(31,31,31), minus(mulf32(sinLerp(sunZ),cosLerp(sunX))/8), minus(mulf32(cosLerp(sunZ),cosLerp(sunX))/8), minus(-(sinLerp(sunX)/8)));

	glPolyFmt(POLY_ALPHA(31) /*| POLY_FORMAT_LIGHT0 | POLY_FORMAT_LIGHT1*/ | POLY_CULL_BACK);
	
	updateControls();
	
	#ifdef SURVIVAL
		updateMobs(&map);
	#endif
	drawTestMap(&map);
	
	glPopMatrix(1);
	
	if(testBuffer)
	{
		//HUD
		glPolyFmt(POLY_ALPHA(31) | POLY_CULL_FRONT);
		drawTestCube();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-128, 127, 95, -96, -1000, 1000);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef32(0, inttof32(-800), 0);
		glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
		glColor(RGB15(31,31,31));
		Game_ApplyMTL(crossHair);
		glBegin(GL_QUADS);
		GFX_TEX_COORD = TEXTURE_PACK(16*(0), 16*(16));
		GFX_VERTEX10 = NORMAL_PACK((255),(0),(-256));
		GFX_TEX_COORD = TEXTURE_PACK(16*(16), 16*(16));
		GFX_VERTEX10 = NORMAL_PACK((255),(0),(255));
		GFX_TEX_COORD = TEXTURE_PACK(16*(16), 16*0);
		GFX_VERTEX10 = NORMAL_PACK((-256),(0),(255));
		GFX_TEX_COORD = TEXTURE_PACK(16*(0), 16*0);
		GFX_VERTEX10 = NORMAL_PACK((-256),(0),(-256));
		glPopMatrix(1);
	}

	glFlush(0);

	int time;
	PROF2_END(time);
	addValue(&frameTime,time);
	swiWaitForVBlank();
	PROF2_START();
	
	if(keysDown() & KEY_X){DS_UsedMem();addValue(&freeRam,DS_FreeMem()/1024);}
	
	// vramSetBankE(VRAM_E_LCD);
		// int i;
		// for(i=0;i<glareLength;i++)((u16*)dawnTexture->pal)[i]=RGB15(glareR+((skyR-glareR)*i)/glareLength,glareG+((skyG-glareG)*i)/glareLength,glareB+((skyB-glareB)*i)/glareLength);
		// for(i=glareLength;i<32;i++)((u16*)dawnTexture->pal)[i]=RGB15(skyR,skyG,skyB);
	// vramSetBankE(VRAM_E_TEX_PALETTE);
	// if(testBuffer)iprintf("\x1b[0;0HMCDS_test \n\nframe :  %d   ",time);
	// else iprintf("\nframe :  %d   ",time);
	#ifdef DEBUGMODE
	if(testBuffer)iprintf("\x1b[0;0HMCDS_test %d fps   \n\n%dK;%dK %d, %d, %d, %d ",Game_FPS,latestFree/1024,latestUsed/1024,TESTVALUE, cacheNumber, cacheCursor, lightProcess.count);
	#endif
	#ifdef DEBUGMODE2
	if(testBuffer)iprintf("\x1b[0;0HMCDS_test %d fps   \n\n%dK;%dK %d, %d, %d, %d ",Game_FPS,latestFree/1024,latestUsed/1024,TESTVALUE, cacheNumber, cacheCursor, lightProcess.count);
	#endif
}

void Game_Kill(void)
{
	freeMap(&map);
	freeQuadCache();
	freeLightCache();
	freeEnvironment();
	NOGBA("RAM after free :\n%dko used, %dko free    \n",DS_UsedMem()/1024,DS_FreeMem()/1024);
}

void Game_VBlank(void)
{
	Game_VBLcount++;
	if(Game_VBLcount>=60){Game_FPS=Game_FrameCount;Game_FrameCount=0;Game_VBLcount=0;}
}
