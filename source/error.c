#include "common/general.h"

char *errors[] =
{
	"Random error has occured. Please reboot.",
	"Could not allocate memory.",
	"Could not find free memory slot.",
	"Could not free : pointer is NULL.",
	"Could not free : memory slot not found.",
	"Could not load file."
};

void DS_Error(u8 code)
{
	/*videoSetMode(0);
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);
	vramSetBankC(VRAM_C_SUB_BG);
	SUB_BG0_CR = BG_MAP_BASE(31);
	BG_PALETTE_SUB[255] = RGB15(31,31,31);
	BG_PALETTE_SUB[0] = RGB15(31,0,0);
	consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(31), (u16*)CHAR_BASE_BLOCK_SUB(0), 16);
	NOGBA("Error Report\nCode %d :\n%s",(int)code,errors[code]);
	while(1)swiWaitForVBlank();*/
}
