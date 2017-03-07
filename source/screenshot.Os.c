#include "common/general.h"
#include "common/bmp.h"

void wait();
int scrnum;


void screenshot(u8* buffer) {

	u8 vram_cr_temp=VRAM_A_CR;
	VRAM_A_CR=VRAM_A_LCD;

	u8* vram_temp=(u8*)malloc(128*1024);
	dmaCopy(VRAM_A, vram_temp, 128*1024);

	REG_DISPCAPCNT=DCAP_BANK(0)|DCAP_ENABLE|DCAP_SIZE(3);
	while(REG_DISPCAPCNT & DCAP_ENABLE);

	dmaCopy(VRAM_A, buffer, 256*192*2);
	dmaCopy(vram_temp, VRAM_A, 128*1024);
	
	VRAM_A_CR=vram_cr_temp;
	
	free(vram_temp);

}

void write16(u16* address, u16 value) {

	u8* first=(u8*)address;
	u8* second=first+1;

	*first=value&0xff;
	*second=value>>8;
}

void write32(u32* address, u32 value) {

	u8* first=(u8*)address;
	u8* second=first+1;
	u8* third=first+2;
	u8* fourth=first+3;

	*first=value&0xff;
	*second=(value>>8)&0xff;
	*third=(value>>16)&0xff;
	*fourth=(value>>24)&0xff;
}

void screenshotbmp(const char* filename, u16* vram)
{

	FILE* file=fopen(filename, "wb");
	// vramSetBankD(VRAM_D_LCD);

	// REG_DISPCAPCNT=DCAP_BANK(3)|DCAP_ENABLE|DCAP_SIZE(3);
	// while(REG_DISPCAPCNT & DCAP_ENABLE);

	u8* temp=(u8*)malloc(256*192*3+sizeof(INFOHEADER)+sizeof(HEADER));

	HEADER* header=(HEADER*)temp;
	INFOHEADER* infoheader=(INFOHEADER*)(temp+sizeof(HEADER));

	write16(&header->type, 0x4D42);
	write32(&header->size, 256*192*3+sizeof(INFOHEADER)+sizeof(HEADER));
	write32(&header->offset, sizeof(INFOHEADER)+sizeof(HEADER));
	write16(&header->reserved1, 0);
	write16(&header->reserved2, 0);

	write16(&infoheader->bits, 24);
	write32(&infoheader->size, sizeof(INFOHEADER));
	write32(&infoheader->compression, 0);
	write32(&infoheader->width, 256);
	write32(&infoheader->height, 192);
	write16(&infoheader->planes, 1);
	write32(&infoheader->imagesize, 256*192*3);
	write32(&infoheader->xresolution, 0);
	write32(&infoheader->yresolution, 0);
	write32(&infoheader->importantcolours, 0);
	write32(&infoheader->ncolours, 0);
	int y,x;
	for(y=0;y<192;y++)
	{
		for(x=0;x<256;x++)
		{
			// u16 color=VRAM_D[256*192-y*256+x];
			u16 color=vram[256*192-y*256+x];

			u8 b=(color&31)<<3;
			u8 g=((color>>5)&31)<<3;
			u8 r=((color>>10)&31)<<3;

			temp[((y*256)+x)*3+sizeof(INFOHEADER)+sizeof(HEADER)]=r;
			temp[((y*256)+x)*3+1+sizeof(INFOHEADER)+sizeof(HEADER)]=g;
			temp[((y*256)+x)*3+2+sizeof(INFOHEADER)+sizeof(HEADER)]=b;
		}
	}

	DC_FlushAll();
	fwrite(temp, 1, 256*192*3+sizeof(INFOHEADER)+sizeof(HEADER), file);
	fclose(file);
	free(temp);
}

void screenshotbmp2(const char* filename, u16* vram1, u16* vram2)//, u16 bgcolor)
{

	FILE* file=fopen(filename, "wb");
	
	// u8* temp=(u8*)malloc(256*192*3+sizeof(INFOHEADER)+sizeof(HEADER));
	u8* temp=(u8*)malloc(sizeof(INFOHEADER)+sizeof(HEADER));

	HEADER* header=(HEADER*)temp;
	INFOHEADER* infoheader=(INFOHEADER*)(temp+sizeof(HEADER));

	write16(&header->type, 0x4D42);
	write32(&header->size, 256*192*3+sizeof(INFOHEADER)+sizeof(HEADER));
	write32(&header->offset, sizeof(INFOHEADER)+sizeof(HEADER));
	write16(&header->reserved1, 0);
	write16(&header->reserved2, 0);

	write16(&infoheader->bits, 24);
	write32(&infoheader->size, sizeof(INFOHEADER));
	write32(&infoheader->compression, 0);
	write32(&infoheader->width, 256);
	write32(&infoheader->height, 192);
	write16(&infoheader->planes, 1);
	write32(&infoheader->imagesize, 256*192*3);
	write32(&infoheader->xresolution, 0);
	write32(&infoheader->yresolution, 0);
	write32(&infoheader->importantcolours, 0);
	write32(&infoheader->ncolours, 0);
	fwrite(temp, 1, sizeof(INFOHEADER)+sizeof(HEADER), file);
	int y,x;
	for(y=0;y<192;y++)
	{
		for(x=0;x<256;x++)
		{
			// u16 color=VRAM_D[256*192-y*256+x];
			u16 color=vram2[256*192-y*256+x];
			// if(color==bgcolor)color=vram1[256*192-y*256+x];
			if(!((color>>15)&1)){color=vram1[256*192-y*256+x];}

			u8 b=(color&31)<<3;
			u8 g=((color>>5)&31)<<3;
			u8 r=((color>>10)&31)<<3;

			// temp[((y*256)+x)*3+sizeof(INFOHEADER)+sizeof(HEADER)]=r;
			// temp[((y*256)+x)*3+1+sizeof(INFOHEADER)+sizeof(HEADER)]=g;
			// temp[((y*256)+x)*3+2+sizeof(INFOHEADER)+sizeof(HEADER)]=b;
			fwrite(&r,1,sizeof(u8),file);
			fwrite(&g,1,sizeof(u8),file);
			fwrite(&b,1,sizeof(u8),file);
		}
	}

	DC_FlushAll();
	// fwrite(temp, 1, 256*192*3+sizeof(INFOHEADER)+sizeof(HEADER), file);
	fclose(file);
	free(temp);
}

char bmpname[40];
char dirname[40];

void Debug_TakeScreenshotBMP(u16* vram1, u16* vram2)//, u16 bgcolor)
{
	scrnum++;
	#ifdef FATONLY
		if(scrnum<10)sprintf(bmpname, "screens/SCR_0000%d.bmp",scrnum);
		else if(scrnum<100)sprintf(bmpname, "screens/SCR_000%d.bmp",scrnum);
		else if(scrnum<1000)sprintf(bmpname, "screens/SCR_00%d.bmp",scrnum);
		else if(scrnum<10000)sprintf(bmpname, "screens/SCR_0%d.bmp",scrnum);
		else sprintf(bmpname, "screens/SCR_%d.bmp",scrnum);
	#else
		if(scrnum<10)sprintf(bmpname, "%s/%s/screens/SCR_0000%d.bmp",basePath,ROOT,scrnum);
		else if(scrnum<100)sprintf(bmpname, "%s/%s/screens/SCR_000%d.bmp",basePath,ROOT,scrnum);
		else if(scrnum<1000)sprintf(bmpname, "%s/%s/screens/SCR_00%d.bmp",basePath,ROOT,scrnum);
		else if(scrnum<10000)sprintf(bmpname, "%s/%s/screens/SCR_0%d.bmp",basePath,ROOT,scrnum);
		else sprintf(bmpname, "%s/%s/screens/SCR_%d.bmp",basePath,ROOT,scrnum);
	#endif
	// FILE* f=fopen(bmpname,"rb");
	while(!access(bmpname,R_OK))
	{
		// fclose(f);
		scrnum++;
		#ifdef FATONLY
			if(scrnum<10)sprintf(bmpname, "screens/SCR_0000%d.bmp",scrnum);
			else if(scrnum<100)sprintf(bmpname, "screens/SCR_000%d.bmp",scrnum);
			else if(scrnum<1000)sprintf(bmpname, "screens/SCR_00%d.bmp",scrnum);
			else if(scrnum<10000)sprintf(bmpname, "screens/SCR_0%d.bmp",scrnum);
			else sprintf(bmpname, "screens/SCR_%d.bmp",scrnum);
		#else
		if(scrnum<10)sprintf(bmpname, "%s/%s/screens/SCR_0000%d.bmp",basePath,ROOT,scrnum);
		else if(scrnum<100)sprintf(bmpname, "%s/%s/screens/SCR_000%d.bmp",basePath,ROOT,scrnum);
		else if(scrnum<1000)sprintf(bmpname, "%s/%s/screens/SCR_00%d.bmp",basePath,ROOT,scrnum);
		else if(scrnum<10000)sprintf(bmpname, "%s/%s/screens/SCR_0%d.bmp",basePath,ROOT,scrnum);
		else sprintf(bmpname, "%s/%s/screens/SCR_%d.bmp",basePath,ROOT,scrnum);
		#endif
		// FILE* f=fopen(bmpname,"rb");
	}
	screenshotbmp2(bmpname, vram1, vram2);//, bgcolor);
}

void D3D_TakeScreenshotBMP(u16* buffer)
{
	scrnum++;
	if(scrnum<10)sprintf(bmpname, "fat:/SCR_0000%d.bmp",scrnum);
	else if(scrnum<100)sprintf(bmpname, "fat:/SCR_000%d.bmp",scrnum);
	else if(scrnum<1000)sprintf(bmpname, "fat:/SCR_00%d.bmp",scrnum);
	else if(scrnum<10000)sprintf(bmpname, "fat:/SCR_0%d.bmp",scrnum);
	else sprintf(bmpname, "fat:/SCR_%d.bmp",scrnum);
	//sprintf(dirname, "prout%d",(int)(scrnum/99));
	//mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	//chdir(dirname);
	D3D_SaveScreen(bmpname, buffer);
	//chdir("..");
}


void D3D_SaveScreen(const char* filename, u16* buffer)
{
	u8* temp=(u8*)malloc(256*192*3+sizeof(INFOHEADER)+sizeof(HEADER));

	HEADER* header=(HEADER*)temp;
	INFOHEADER* infoheader=(INFOHEADER*)(temp+sizeof(HEADER));

	write16(&header->type, 0x4D42);
	write32(&header->size, 256*384*3+sizeof(INFOHEADER)+sizeof(HEADER));
	write32(&header->offset, sizeof(INFOHEADER)+sizeof(HEADER));
	write16(&header->reserved1, 0);
	write16(&header->reserved2, 0);

	write16(&infoheader->bits, 24);
	write32(&infoheader->size, sizeof(INFOHEADER));
	write32(&infoheader->compression, 0);
	write32(&infoheader->width, 256);
	write32(&infoheader->height, 384);
	write16(&infoheader->planes, 1);
	write32(&infoheader->imagesize, 256*384*3);
	write32(&infoheader->xresolution, 0);
	write32(&infoheader->yresolution, 0);
	write32(&infoheader->importantcolours, 0);
	write32(&infoheader->ncolours, 0);
	int y,x;
	for(y=0;y<384;y++)
	{
		for(x=0;x<256;x++)
		{
			u16 color=buffer[256*384-y*256+x];

			u8 b=(color&31)<<3;
			u8 g=((color>>5)&31)<<3;
			u8 r=((color>>10)&31)<<3;

			temp[((y*256)+x)*3+sizeof(INFOHEADER)+sizeof(HEADER)]=r;
			temp[((y*256)+x)*3+1+sizeof(INFOHEADER)+sizeof(HEADER)]=g;
			temp[((y*256)+x)*3+2+sizeof(INFOHEADER)+sizeof(HEADER)]=b;
		}
	}

	//DC_FlushAll();
	FILE* file=fopen(filename, "wb");
	fwrite(temp, 1, 256*384*3+sizeof(INFOHEADER)+sizeof(HEADER), file);
	fclose(file);
	free(temp);
}
