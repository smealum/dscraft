#include "common/general.h"
#include "game/game_main.h"

#include <maxmod9.h>

#include "soundbank.h"
#include "soundbank_bin.h"

int main(int argc, char **argv)
{
	DS_InitHardware();
	DS_InitFS(argc, argv);
	
	consoleDemoInit();//DEBUG
	
	#ifdef FATONLY
		// sprintf(packPath,"fat:/dscraft/packs/eldpack");
		sprintf(packPath,"packs/eldpack");
	#else
		sprintf(packPath,"nitro:/dscraft/packs/eldpack");
	#endif
	
	NOGBA("init ! %d",fsMode);
	
	time_t unixTime = time(NULL);
	struct tm* timeStruct = gmtime((const time_t *)&unixTime);
	srand(timeStruct->tm_hour*timeStruct->tm_min*timeStruct->tm_sec*timeStruct->tm_mon*timeStruct->tm_mday*timeStruct->tm_year);
	
	mmInitDefaultMem((mm_addr)soundbank_bin);
		
	mmLoadEffect(SFX_STEP);
	mmLoadEffect(SFX_ADD);
	mmLoadEffect(SFX_REMOVE);
		
	DS_CreateState(&Game_State, (function)&Game_Init, (function)&Game_Frame, (function)&Game_Kill, (function)&Game_VBlank);
	DS_CreateState(&Menu_State, (function)&Menu_Init, (function)&Menu_Frame, (function)&Menu_Kill, (function)&Menu_VBlank);
	
	// DS_ChangeState(&Game_State);
	DS_ChangeState(&Menu_State);
	DS_ApplyState();
	
	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);
    vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
    vramSetBankC(VRAM_C_SUB_BG_0x06200000 );

	int bg3 = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0,0);
	int bg4 = bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0,0);
	
	u8 *buffer;
	buffer=DS_OpenFile("spalsh1.pcx", "", true, true);
	sImage pcx; 
	int t=loadPCX((u8*)buffer, &pcx);


	dmaCopy(pcx.image.data8, bgGetGfxPtr(bg3), 256*192);
	dmaCopy(pcx.palette, BG_PALETTE, 256*2);
	imageDestroy(&pcx);
	buffer=DS_OpenFile("spalsh2.pcx", "", true, true);
	sImage pcx2; 
	loadPCX((u8*)buffer, &pcx2);

	NOGBA("D");

	dmaCopy(pcx2.image.data8, bgGetGfxPtr(bg4), 256*192);
	dmaCopy(pcx2.palette, BG_PALETTE_SUB, 256*2);
	imageDestroy(&pcx2);

	int te=0;
    while(te<60*2 && !(keysUp())){te++;swiWaitForVBlank();scanKeys();}
	
	while(1)
	{
		CurrentState->Init();
		while(CurrentState->used)CurrentState->Frame();
		CurrentState->Kill();
		DS_ApplyState();
	}
	return 0;
}
