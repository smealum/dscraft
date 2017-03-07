#include "menu/menu_main.h"
#include <dirent.h>
#include "API/API.h"
#include "game/map.h"
#include "game/player.h"
#include "game/environment.h"

#define minus(a) (((a)<0)?((a)+1):((a)-1))

typedef struct
{
	API_Entity* button;
	char* filename;
	bool isDir, nitro;
}listFile_struct;

typedef struct
{
	listFile_struct* list;
	u16 count;
}filelist_struct;

filelist_struct worldList, packList;

API_Entity* bottomScreen;
API_Entity *singleWindow, *optionWindow, *textureWindow;
API_Entity* topScreen;
API_Entity* schemes[3];
API_Entity* schemeWindows[3*2];
u8 selectedScheme;

MTL_img* logo;

u32* sceneList1;
u32* sceneList2;
u32* subSceneList1;
u32* subSceneList2;

s16 testAngleZ;
s16 testAngleX;
s16 subtestAngleZ;
s16 subtestAngleX;

vect3D logoPos=(vect3D){0,0,0};

void D3D2_glFrustumf32(int32 left, int32 right, int32 bottom, int32 top, int32 near, int32 far) {

	MATRIX_MULT4x4 = divf32(2*near, right - left);
	MATRIX_MULT4x4 = 0;
	MATRIX_MULT4x4 = 0;
	MATRIX_MULT4x4 = 0;
 
	MATRIX_MULT4x4 = 0;
	MATRIX_MULT4x4 = divf32(2*near, top - bottom);
	MATRIX_MULT4x4 = 0;
	MATRIX_MULT4x4 = 0;
 
	MATRIX_MULT4x4 = divf32(right + left, right - left);
	MATRIX_MULT4x4 = divf32(top + bottom, top - bottom);
	MATRIX_MULT4x4 = -divf32(far + near, far - near);
	MATRIX_MULT4x4 = floattof32(-1.0F);
 
	MATRIX_MULT4x4 = 0;
	MATRIX_MULT4x4 = 0;
	MATRIX_MULT4x4 = -divf32(2 * mulf32(far, near), far - near);
	MATRIX_MULT4x4 = 0;
}

void D3D2_gluPerspectivef32(int fovy, int32 aspect, int32 zNear, int32 zFar) {
	int32 xmin, xmax, ymin, ymax;
	
	ymax = mulf32(zNear, tanLerp(fovy>>1));
	
	ymin = -ymax;
	xmin = mulf32(ymin, aspect);
	xmax = mulf32(ymax, aspect);
	
	D3D2_glFrustumf32(xmin, xmax, ymin, ymax, zNear, zFar);
}


void D3D2_gluPerspectivef32_2(int fovy, int32 aspect, int32 zNear, int32 zFar) {
	int32 xmin, xmax, ymin, ymax, ymax2;
	
	ymax = mulf32(zNear, tanLerp(fovy>>1));
	
	ymin = -ymax;

	ymax2 = ymax+(ymax-ymin);

	xmin = mulf32(ymin, aspect);
	xmax = mulf32(ymax, aspect);
	
	D3D2_glFrustumf32(xmin, xmax, ymax, ymax2, zNear, zFar);
}

void RenderScene()
{
		if(!D3D_Screen)sunX+=degreesToAngle(180);
		else sunX-=degreesToAngle(180);
		sunZ%=32768;
		sunX%=32768;
		if(sunX<0)sunX+=degreesToAngle(360);
		dayTime=((sunX<16384)?(8192-abs(sunX-8192)):0);
		dayTime=(dayTime<4096)?(dayTime):4096;
		nightTime=((sunX<16384)?0:(8192-abs((sunX-16384)-8192)));
		nightTime=(nightTime<4096)?(nightTime):4096;
		
		drawSky();
		glPushMatrix();
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(70, 256.0 / 192.0, 0.1, 10000);
		// if(D3D_Screen)D3D2_gluPerspectivef32(floattof32(70), floattof32(256.0 / 192.0), floattof32(0.1), floattof32(10000));
		// else D3D2_gluPerspectivef32_2(floattof32(70), floattof32(256.0 / 192.0), floattof32(0.1), floattof32(10000));
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		if(!D3D_Screen)
		{
			glRotateXi(testAngleX);
			glRotateZi(testAngleZ);
		}else{
			glRotateXi(subtestAngleX);
			glRotateZi(subtestAngleZ);
		}

		glMaterialf(GL_AMBIENT, RGB15(14,14,14));
		glMaterialf(GL_DIFFUSE, RGB15(31,31,31));
		glMaterialf(GL_SPECULAR, BIT(15) | RGB15(21,21,21));
		glMaterialf(GL_EMISSION, RGB15(0,0,0));

		//ds uses a table for shinyness..this generates a half-ass one
		glMaterialShinyness();
		glLight(0, RGB15(31,31,31), minus(mulf32(sinLerp(sunZ),cosLerp(sunX))/8), minus(mulf32(cosLerp(sunZ),cosLerp(sunX))/8), minus(-(sinLerp(sunX)/8)));

		glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE | POLY_ID(63));
		
		drawSun();
		drawMoon();
		drawDawn();
		drawStars();
		drawCloud();
		
		if(!D3D_Screen)
		{
			glPushMatrix();
				Game_ApplyMTL(logo);
				glTranslatef32(logoPos.x,logoPos.y,0);
				glScalef32(-inttof32(1),-inttof32(1),inttof32(1));
				drawLogo();
			glPopMatrix(1);
		}
		
		glPushMatrix();
			glPolyFmt(POLY_ALPHA(31) | POLY_FORMAT_LIGHT0 | POLY_CULL_BACK);
			glScalef32(inttof32(SCALEFACTOR),inttof32(SCALEFACTOR),inttof32(SCALEFACTOR));
			glTranslatef32(-(SUPERCLUSTERSIZE*CLUSTERSIZE*(tilesize2<<6))/2, -(SUPERCLUSTERSIZE*CLUSTERSIZE*(tilesize2<<6))/2,-(64*(tilesize2<<6))/2);
			// Game_FastBind(blockSuperTexture);
			Game_ApplyMTL(blockSuperTexture);
			if(!D3D_Screen)
			{
				glCallList(sceneList1);
				// glCallList(sceneList2);
			}else{
				glCallList(subSceneList1);
				// glCallList(subSceneList2);
			}
		glPopMatrix(1);
		
		sunX+=20;
		
		glPopMatrix(1);
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		
		if(D3D_Screen)glOrtho(-128, 127, 191, 0, -1000, 1000);
		else glOrtho(-128, 127, 0, -191, -1000, 1000);
		
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		
		scanKeys();
		
		// NOGBA("lala????");
		API_UpdateScene(&API_List);
		
		glPopMatrix(1);
			
		glFlush(0);
}

u8* loadFile(char* filename)
{
	FILE* file;
	
	NOGBA("opening...");
	file = fopen(filename, "rb+");
	
	u8* buffer;
	long lsize;
	fseek (file, 0 , SEEK_END);
	lsize = ftell (file);
	lastSize=lsize;
	rewind (file);
	buffer = (u8*) malloc (lsize);
		
	fread (buffer, 1, lsize, file);
	fclose (file);
	return buffer;
}

void Menu_Singleplayer(API_Entity* e)
{
	// DS_ChangeState(&Game_State);
	// DS_ChangeState(&Menu_State);
	API_SetAlpha(bottomScreen,0);
	API_SetAlphaSons(bottomScreen,0);
	API_SetAlpha(singleWindow,31);
	API_SetAlphaSons(singleWindow,31);
	setupButtons(&worldList, 0);
}

void Menu_Texture(API_Entity* e)
{
	API_SetAlpha(bottomScreen,0);
	API_SetAlphaSons(bottomScreen,0);
	API_SetAlpha(textureWindow,31);
	API_SetAlphaSons(textureWindow,31);
	setupButtons(&packList, 0);
}

void Menu_Option(API_Entity* e)
{
	API_SetAlpha(bottomScreen,0);
	API_SetAlphaSons(bottomScreen,0);
	API_SetAlpha(optionWindow,31);
	API_SetAlphaSons(optionWindow,31);
	Menu_SelectScheme(schemes[selectedScheme]);
}

void Menu_Back(API_Entity* e)
{
	API_SetAlpha(bottomScreen,31);
	API_SetAlphaSons(bottomScreen,31);
	API_SetAlpha(singleWindow,0);
	API_SetAlpha(optionWindow,0);
	API_SetAlpha(textureWindow,0);
	API_SetAlphaSons(singleWindow,0);
	API_SetAlphaSons(optionWindow,0);
	API_SetAlphaSons(textureWindow,0);
	API_SetAlpha(schemeWindows[0],0);
	API_SetAlpha(schemeWindows[1],0);
	API_SetAlpha(schemeWindows[2],0);
	API_SetAlpha(schemeWindows[3],0);
	API_SetAlpha(schemeWindows[4],0);
	API_SetAlpha(schemeWindows[5],0);
}

void Menu_BackOptions(API_Entity* e)
{
	Menu_Back(e);
	
	if(gameSettings.controls!=selectedScheme || strcmp(gameSettings.texturePack,packPath))
	{
		sprintf(gameSettings.texturePack,packPath);
		gameSettings.controls=selectedScheme;
		saveSettings();
	}
}

void Menu_ChangePack(API_Entity* e)
{
	int i;
	for(i=0;i<packList.count;i++)
	{
		if(e==packList.list[i].button)
		{
			#ifdef FATONLY
				sprintf(packPath,"packs/%s",packList.list[i].filename);
			#else
				if(packList.list[i].nitro)sprintf(packPath,"nitro:/dscraft/packs/%s",packList.list[i].filename);
				else{sprintf(packPath,"%s/%s/packs/%s",basePath,ROOT,packList.list[i].filename);}
			#endif
		}
	}
	loadBlockTextures(false, false);
}

void Menu_StartGame(API_Entity* e)
{
	int i;
	for(i=0;i<worldList.count;i++)
	{
		if(e==worldList.list[i].button)
		{
			#ifdef FATONLY
				sprintf(mapPath,"worlds/%s",worldList.list[i].filename);
			#else
				if(worldList.list[i].nitro)sprintf(mapPath,"nitro:/dscraft/worlds/%s",worldList.list[i].filename);
				else sprintf(mapPath,"%s/%s/worlds/%s",basePath,ROOT,worldList.list[i].filename);
			#endif
		}
	}
	DS_ChangeState(&Game_State);
}

void listFiles(char* path, filelist_struct* l, API_Entity* father, API_function function, u8 mode) //0 files, 1 both, 2 dir
{
	char str[255];
	l->list=NULL;
	l->count=0;
	DIR *dir;
	struct dirent *ent;
	struct stat st;
	
	#ifndef FATONLY
		sprintf(str,"nitro:/%s/%s",ROOT,path);
		dir=opendir(str);
		if(dir)
		{
			while((ent=readdir(dir)))
			{
				stat(ent->d_name,&st);
				if(((S_ISDIR(st.st_mode) && mode) || !mode || (!S_ISDIR(st.st_mode) && mode==1))
				&& !(strlen(ent->d_name)==1 && ent->d_name[0]=='.')
				&& !(strlen(ent->d_name)==2 && ent->d_name[0]=='.' && ent->d_name[1]=='.'))l->count++;
			}
			closedir(dir);
		}
	#endif
	#ifdef FATONLY
		sprintf(str,"%s",path);
	#else
		sprintf(str,"%s/%s/%s",basePath,ROOT,path);
	#endif
	dir=opendir(str);
	if(dir)
	{
		while((ent=readdir(dir)))
		{
			stat(ent->d_name,&st);
			if(((S_ISDIR(st.st_mode) && mode) || !mode || (!S_ISDIR(st.st_mode) && mode==1))
			&& !(strlen(ent->d_name)==1 && ent->d_name[0]=='.')
			&& !(strlen(ent->d_name)==2 && ent->d_name[0]=='.' && ent->d_name[1]=='.'))l->count++;
		}
		closedir(dir);
	}
	
	l->list=malloc(sizeof(listFile_struct)*l->count);
	if(l->list)
	{
		l->count=0;
		
		#ifndef FATONLY
			sprintf(str,"nitro:/%s/%s",ROOT,path);
			dir=opendir(str);
			while((ent=readdir(dir)))
			{
				stat(ent->d_name,&st);
				if(((S_ISDIR(st.st_mode) && mode) || !mode || (!S_ISDIR(st.st_mode) && mode==1))
				&& !(strlen(ent->d_name)==1 && ent->d_name[0]=='.')
				&& !(strlen(ent->d_name)==2 && ent->d_name[0]=='.' && ent->d_name[1]=='.'))
				{
					l->list[l->count].filename=malloc(strlen(ent->d_name)+1);
					strcpy(l->list[l->count].filename,ent->d_name);
					l->list[l->count].isDir=S_ISDIR(st.st_mode);
					l->list[l->count].nitro=true;
					l->list[l->count].button=API_CreateButtonFather(128-strlen(l->list[l->count].filename)*4, 32+20*l->count, RGB15(31,31,31), function, father, l->list[l->count].filename, "button.pcx", false);
					l->count++;
				}
			}
			closedir(dir);
		#endif
		
		#ifdef FATONLY
			sprintf(str,"%s",path);
		#else
			sprintf(str,"%s/%s/%s",basePath,ROOT,path);
		#endif
		dir=opendir(str);
		while((ent=readdir(dir)))
		{
			stat(ent->d_name,&st);
			if(((S_ISDIR(st.st_mode) && mode) || !mode || (!S_ISDIR(st.st_mode) && mode==1))
			&& !(strlen(ent->d_name)==1 && ent->d_name[0]=='.')
			&& !(strlen(ent->d_name)==2 && ent->d_name[0]=='.' && ent->d_name[1]=='.'))
			{
				l->list[l->count].filename=malloc(strlen(ent->d_name)+1);
				strcpy(l->list[l->count].filename,ent->d_name);
				l->list[l->count].isDir=S_ISDIR(st.st_mode);
				l->list[l->count].nitro=false;
				l->list[l->count].button=API_CreateButtonFather(128-strlen(l->list[l->count].filename)*4, 32+20*l->count, RGB15(31,31,31), function, father, l->list[l->count].filename, "button.pcx", false);
				l->count++;
			}
		}
		closedir(dir);
	}
}

void setupButtons(filelist_struct* l, u16 index)
{
	int i;
	for(i=0;i<l->count;i++)
	{
		if(i>=index && i<index+4)
		{
			API_SetPosition(l->list[i].button, 128-strlen(l->list[i].filename)*4, 32+20*(i-index));
			API_SetAlpha(l->list[i].button,31);
		}else{
			API_SetAlpha(l->list[i].button,0);
		}
	}
	// API_ComputeDirections(&API_List, 0);
}

void Menu_SingleSlider(API_Entity* e)
{
	APIE_SliderData* data=(APIE_SliderData*)e->data;
	// NOGBA("%d",data->position);
	setupButtons(&worldList, data->position*(max(worldList.count-4,0))/f32toint(e->Size.y));
}

void Menu_TextureSlider(API_Entity* e)
{
	APIE_SliderData* data=(APIE_SliderData*)e->data;
	// NOGBA("%d",data->position);
	setupButtons(&packList, data->position*(max(packList.count-4,0))/f32toint(e->Size.y));
}

void Menu_SelectScheme(API_Entity* e)
{
	// APIE_CheckBoxData* data=(APIE_CheckBoxData*)e->data;
	int i;
	
	for(i=0;i<3;i++)
	{
		APIE_CheckBoxData* d=(APIE_CheckBoxData*)schemes[i]->data;
		if(e==schemes[i])
		{
			selectedScheme=i;
			API_SetAlpha(schemeWindows[i*2],31);
			API_SetAlpha(schemeWindows[i*2+1],31);
			d->checked=true;
		}else{
			d->checked=false;
			API_SetAlpha(schemeWindows[i*2],0);
			API_SetAlpha(schemeWindows[i*2+1],0);
		}
	}
}

void Menu_Init(void)
{
	int i;
	
	#ifdef FATONLY
		// chdir("fat:/");
		// chdir(ROOT);//!ROOT!
	#else
		chdir("nitro:/");
	#endif
	chdir(ROOT);
	sceneList1=(u32*)loadFile("testscene1.bin");
	// sceneList2=(u32*)loadFile("testscene2.bin");
	subSceneList1=(u32*)loadFile("subtestscene1.bin");
	// subSceneList2=(u32*)loadFile("subtestscene2.bin");
		
	lcdMainOnTop();
	videoSetMode(MODE_3_3D | DISPLAY_BG0_ACTIVE);
	videoSetModeSub(MODE_5_2D | DISPLAY_BG2_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_2D_BMP_256);
    vramSetPrimaryBanks(VRAM_A_TEXTURE,VRAM_B_TEXTURE,VRAM_C_SUB_BG,VRAM_D_SUB_SPRITE);	
	vramSetBankE(VRAM_E_TEX_PALETTE);
	vramSetBankF(VRAM_F_LCD);
	vramSetBankG(VRAM_G_LCD);
	vramSetBankH(VRAM_H_LCD);
	vramSetBankI(VRAM_I_LCD);
	
	REG_BG2CNT_SUB = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY(1);
        REG_BG2PA_SUB = 1 << 8;
        REG_BG2PB_SUB = 0;
        REG_BG2PC_SUB = 0;
        REG_BG2PD_SUB = 1 << 8;
		
        REG_BG2X_SUB = 0;
        REG_BG2Y_SUB = 0;
	
	REG_BG0CNT = BG_PRIORITY(0);
	
	glFlush(0);
	
	glReInit();
	
	Game_InitVramBanks(2);
	Game_InitTextures();
	
	srand(time(NULL));
	
	// enable antialiasing
	glEnable(GL_ANTIALIAS);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	//glEnable(GL_OUTLINE);
	
	// setup the rear plane
	glClearColor(0,0,0,31); // BG must be opaque for AA to work
	glClearPolyID(63); // BG must have a unique polygon ID for AA to work
	glClearDepth(0x7FFF);

	//this should work the same as the normal gl call
	glViewport(0,0,255,191);
	
	//any floating point gl call is being converted to fixed prior to being implemented
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70, 256.0 / 192.0, 0.1, 40);
	
	/*gluLookAt(	0.0, 0.0, 3.5,		//camera possition 
				0.0, 0.0, 0.0,		//look at
				0.0, 1.0, 0.0);		//up*/
	gluLookAt(	0.0, 0.0, 0.0,		//camera possition
				0.0, 0.0, -1.0,		//look at
				0.0, 1.0, 0.0);		//up
	
	glLight(0, RGB15(31,31,31) , 0, floattov10(-100.0f), 0);
	
	//not a real gl function and will likely change
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE | POLY_FORMAT_LIGHT0 ) ;
	Game_InitD3D();
	API_Init();
	NOGBA("api done");

	// initErrorWindow();
	// NOGBA("error done");
	// initMessageBox();
	// NOGBA("message done");
	
	loadSettings();
	sprintf(packPath,gameSettings.texturePack);
	selectedScheme=gameSettings.controls;
	
	// API_Entity* tw=API_CreateWindow(0, 0, 100, 100, 31, RGB15(31,31,31), 1, RGB15(0,0,0), "");
	bottomScreen=API_CreateWindow(-128, 0, 256, 192, 31, RGB15(31,31,31), 0, RGB15(0,0,0), "transp.pcx");
	singleWindow=API_CreateWindow(-128, 0, 256, 192, 31, RGB15(31,31,31), 0, RGB15(0,0,0), "transp.pcx");
	textureWindow=API_CreateWindow(-128, 0, 256, 192, 31, RGB15(31,31,31), 0, RGB15(0,0,0), "transp.pcx");
	optionWindow=API_CreateWindow(-128, 0, 256, 192, 31, RGB15(31,31,31), 0, RGB15(0,0,0), "transp.pcx");
	
	schemeWindows[0]=API_CreateWindow(-128+32, 16-192, 128, 256, 31, RGB15(31,31,31), 0, RGB15(0,0,0), "control1_1.pcx");
	schemeWindows[1]=API_CreateWindow(-128+32+128, 16-192, 64, 256, 31, RGB15(31,31,31), 0, RGB15(0,0,0), "control1_2.pcx");
	
	schemeWindows[2]=API_CreateWindow(-128+32, 16-192, 128, 256, 31, RGB15(31,31,31), 0, RGB15(0,0,0), "control2_1.pcx");
	schemeWindows[3]=API_CreateWindow(-128+32+128, 16-192, 64, 256, 31, RGB15(31,31,31), 0, RGB15(0,0,0), "control2_2.pcx");
	
	schemeWindows[4]=API_CreateWindow(-128+32, 16-192, 128, 256, 31, RGB15(31,31,31), 0, RGB15(0,0,0), "control3_1.pcx");
	schemeWindows[5]=API_CreateWindow(-128+32+128, 16-192, 64, 256, 31, RGB15(31,31,31), 0, RGB15(0,0,0), "control3_2.pcx");
	
	//main menu
	API_CreateButtonFather(128-strlen("Singleplayer")*4, 32+0, RGB15(31,31,31), &Menu_Singleplayer, bottomScreen, "Singleplayer", "button.pcx", false);
	API_CreateButtonFather(128-strlen("Texture packs")*4, 32+20, RGB15(31,31,31), &Menu_Texture, bottomScreen, "Texture packs", "button.pcx", false);
	API_CreateButtonFather(128-strlen("Options")*4, 32+40, RGB15(31,31,31), &Menu_Option, bottomScreen, "Options", "button.pcx", false);
	
	//singleplayer
	API_CreateLabelFather(128-strlen("Select world")*4, 16, RGB15(31,31,31), singleWindow, "Select world", true);
	API_CreateButtonFather(256-64+16, 192-32+16, RGB15(31,31,31), &Menu_Back, singleWindow, "Back", "button.pcx", false);
	API_CreateVSliderFather(256-32, 32, 80, &Menu_SingleSlider, singleWindow, "", true);
	listFiles("worlds", &worldList, singleWindow, &Menu_StartGame, 1);
	
	//texture
	API_CreateLabelFather(128-strlen("Select texture pack")*4, 16, RGB15(31,31,31), textureWindow, "Select texture pack", true);
	API_CreateButtonFather(256-64+16, 192-32+16, RGB15(31,31,31), &Menu_BackOptions, textureWindow, "Back", "button.pcx", false);
	API_CreateVSliderFather(256-32, 32, 80, &Menu_TextureSlider, textureWindow, "", true);
	listFiles("packs", &packList, textureWindow, &Menu_ChangePack, 1);
	
	//options
	API_CreateLabelFather(128-strlen("Options")*4, 16, RGB15(31,31,31), optionWindow, "Options", true);
	API_CreateButtonFather(256-64+16, 192-32+16, RGB15(31,31,31), &Menu_BackOptions, optionWindow, "Back", "button.pcx", false);
	schemes[0]=API_CreateCheckBoxFather(64, 64, Menu_SelectScheme, optionWindow, "Control scheme 1", true);
	schemes[1]=API_CreateCheckBoxFather(64, 64+20, Menu_SelectScheme, optionWindow, "Control scheme 2", true);
	schemes[2]=API_CreateCheckBoxFather(64, 64+40, Menu_SelectScheme, optionWindow, "Control scheme 3", true);
	
	selectedScheme%=3;
	Menu_SelectScheme(schemes[selectedScheme]);
	
	topScreen=API_CreateWindow(-128, -192, 256, 192, 31, RGB15(31,31,31), 0, RGB15(0,0,0), "transp.pcx");
	NOGBA("window done");
	
	API_SetAlpha(schemeWindows[0],0);
	API_SetAlpha(schemeWindows[1],0);
	API_SetAlpha(schemeWindows[2],0);
	API_SetAlpha(schemeWindows[3],0);
	API_SetAlpha(schemeWindows[4],0);
	API_SetAlpha(schemeWindows[5],0);
	
	API_SetAlpha(singleWindow,0);
	API_SetAlpha(textureWindow,0);
	API_SetAlpha(optionWindow,0);
	API_SetAlphaSons(singleWindow,0);
	API_SetAlphaSons(textureWindow,0);
	API_SetAlphaSons(optionWindow,0);
	
	initEnvironment(true);
	// logo=Game_CreateTexture("logo.pcx", "textures");
	logo=Game_CreateAlphaMask("logo.pcx", "textures");
	loadBlockTextures(false, true);
	
	testAngleX=-9990;
	testAngleZ=4800;
	subtestAngleX=-9200;
	subtestAngleZ=-2200;
	logoPos.x=346832;
	logoPos.y=81800;
	
	API_ComputeDirections(&API_List, 0);
	
	NOGBA("MEMORY : %dko used, %dko free    \n",DS_UsedMem()/1024,DS_FreeMem()/1024);
	Game_GetVramStatus();
}

int button;

//29;106 : 105;139
void Menu_Frame(void)
{
	int i;
	Game_UpdateD3D();
	RenderScene();
	
	// if(keysHeld() & KEY_R)
	// {
		// if(keysHeld() & KEY_DOWN)subtestAngleX+=200;
		// if(keysHeld() & KEY_UP)subtestAngleX-=200;
		// if(keysHeld() & KEY_RIGHT)subtestAngleZ+=200;
		// if(keysHeld() & KEY_LEFT)subtestAngleZ-=200;
	// }else{
		// if(keysHeld() & KEY_DOWN)testAngleX+=200;
		// if(keysHeld() & KEY_UP)testAngleX-=200;
		// if(keysHeld() & KEY_RIGHT)testAngleZ+=200;
		// if(keysHeld() & KEY_LEFT)testAngleZ-=200;
	// }
	
	// if(keysHeld() & KEY_B)logoPos.x+=8*floattof32(0.1f);
	// if(keysHeld() & KEY_X)logoPos.x-=8*floattof32(0.1f);
	// if(keysHeld() & KEY_A)logoPos.y+=8*floattof32(0.1f);
	// if(keysHeld() & KEY_Y)logoPos.y-=8*floattof32(0.1f);
	
	// NOGBA("%d %d ; %d %d ; %d %d",subtestAngleX,subtestAngleZ,testAngleX,testAngleZ,logoPos.x,logoPos.y);
	
	lcdSwap();
	swiWaitForVBlank();
	// NOGBA("prout");
	APIcall();
}

void Menu_Kill(void)
{
	API_CleanUp();
	free(sceneList1);
	free(subSceneList1);
	freeEnvironment();
	DS_freeState(&Menu_State);
}


void Menu_VBlank(void)
{
}
