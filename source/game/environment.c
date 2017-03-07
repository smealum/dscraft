#include "game/game_main.h"

// #define LOGOSIZE 128
#define LOGOSIZE 96
#define sunmacro(a,b) (((a)<(b))?(0):(a))

u32* starsList;

void initStars(void)
{
	int i;
	for(i=0;i<STARS;i++)
	{
		stars[i].angle.x=-(rand()%16384);
		stars[i].angle.z=rand()%32768;
		stars[i].size=STARSIZE/2+rand()%(STARSIZE/2);
	}
	
	u32* list=glBeginListDL();
	glBeginDL(GL_QUADS);
		for(i=0;i<STARS;i++)
		{
			glVertexPackedDL(NORMAL_PACK(((mulf32(cosLerp(stars[i].angle.z+8192),cosLerp(stars[i].angle.x))>>3)+(-stars[i].size)),((mulf32(sinLerp(stars[i].angle.z+8192),cosLerp(stars[i].angle.x))>>3)+(-stars[i].size)),((-sinLerp(stars[i].angle.x)>>3))));
			glVertexPackedDL(NORMAL_PACK(((mulf32(cosLerp(stars[i].angle.z+8192),cosLerp(stars[i].angle.x))>>3)+(-stars[i].size)),((mulf32(sinLerp(stars[i].angle.z+8192),cosLerp(stars[i].angle.x))>>3)+(stars[i].size)),((-sinLerp(stars[i].angle.x)>>3))));
			glVertexPackedDL(NORMAL_PACK(((mulf32(cosLerp(stars[i].angle.z+8192),cosLerp(stars[i].angle.x))>>3)+(stars[i].size)),((mulf32(sinLerp(stars[i].angle.z+8192),cosLerp(stars[i].angle.x))>>3)+(stars[i].size)),((-sinLerp(stars[i].angle.x)>>3))));
			glVertexPackedDL(NORMAL_PACK(((mulf32(cosLerp(stars[i].angle.z+8192),cosLerp(stars[i].angle.x))>>3)+(stars[i].size)),((mulf32(sinLerp(stars[i].angle.z+8192),cosLerp(stars[i].angle.x))>>3)+(-stars[i].size)),((-sinLerp(stars[i].angle.x)>>3))));
		}
	glEndListDL();

	starsList=malloc(((*list)+1)*4);
	memcpy(starsList, list, ((*list)+1)*4);
}

void freeEnvironment(void)
{
	if(starsList)free(starsList);
}

void initEnvironment(bool menu)
{
	envMenu=menu;
	sunX=0;sunZ=4096/2;
	glareLength=15;
	cloudcnt=0;
	glareR=25;glareG=16;glareB=6;
	initStars();
	cloudTexture=Game_CreateTexture("clouds.pcx", "textures");
	// dawnTexture=Game_CreateTexture("dawndusk.pcx", "textures");
	// sunTexture=Game_CreateTexture("sun.pcx", "textures");
	// moonTexture=Game_CreateTexture("moon.pcx", "textures");
	
	unsigned char* buffer;
	unsigned char* image;
	u8* alpha;
	int i, j;
	size_t buffersize, imagesize;
	LodePNG_Decoder decoder;
	
	char path[255];
	getcwd(path,255);
	chdir("misc");
	LodePNG_loadFile(&buffer, &buffersize, "sun.png");
	LodePNG_Decoder_init(&decoder);
	LodePNG_Decoder_decode(&decoder, &image, &imagesize, buffer, buffersize);
		
	alpha=malloc(decoder.infoPng.height*decoder.infoPng.width);
	for(i=0;i<decoder.infoPng.width;i++)for(j=0;j<decoder.infoPng.height;j++)
	{
		// if(image[(i+j*decoder.infoPng.width)*4]/8<2 && image[(i+j*decoder.infoPng.width)*4+1]/8<2 && image[(i+j*decoder.infoPng.width)*4+2]/8<2)alpha[i+j*decoder.infoPng.width]=0;
		// else alpha[i+j*decoder.infoPng.width]=max(31-(sunmacro(f32toint(sqrtf32(inttof32((i-32)*(i-32)+(j-32)*(j-32)))),13)*31/20),0);
		alpha[i+j*decoder.infoPng.width]=min(image[(i+j*decoder.infoPng.width)*4]/8,31);
		// NOGBA("alpha : %d",alpha[i+j*decoder.infoPng.width]);
	}
	chdir(path);
	free(image);
	free(buffer);
	LodePNG_Decoder_cleanup(&decoder);
	sunTexture=Game_CreateTextureAlphaMask("sun.pcx", "textures", alpha);
	
	chdir("misc");
	LodePNG_loadFile(&buffer, &buffersize, "moon.png");
	LodePNG_Decoder_init(&decoder);
	LodePNG_Decoder_decode(&decoder, &image, &imagesize, buffer, buffersize);
		
	for(i=0;i<decoder.infoPng.width;i++)for(j=0;j<decoder.infoPng.height;j++)
	{
		// if(image[(i+j*decoder.infoPng.width)*4]/8<2 && image[(i+j*decoder.infoPng.width)*4+1]/8<2 && image[(i+j*decoder.infoPng.width)*4+2]/8<2)alpha[i+j*decoder.infoPng.width]=0;
		// else alpha[i+j*decoder.infoPng.width]=max(31-(sunmacro(f32toint(sqrtf32(inttof32((i-32)*(i-32)+(j-32)*(j-32)))),13)*31/20),0);
		alpha[i+j*decoder.infoPng.width]=min(image[(i+j*decoder.infoPng.width)*4]/8,31);
		// NOGBA("alpha : %d",alpha[i+j*decoder.infoPng.width]);
	}
	chdir(path);
	free(image);
	free(buffer);
	LodePNG_Decoder_cleanup(&decoder);
	moonTexture=Game_CreateTextureAlphaMask("moon.pcx", "textures", alpha);
	
	free(alpha);
	
}

void drawStars(void)
{
	// int i;
	u8 a=((nightTime*31)/4096);
	if(!a)return;
	glPushMatrix();
	// glPolyFmt(POLY_ALPHA(a) | POLY_CULL_NONE);
	glPolyFmt(POLY_ALPHA(a) | POLY_CULL_BACK | POLY_ID(1));
	glScalef32(inttof32(SCALEFACTOR*15),inttof32(SCALEFACTOR*15),inttof32(SCALEFACTOR*15));
	Game_ApplyMTL(NULL);
	glColor3b(255,255,255);
	glCallList(starsList);
	/*glBegin(GL_QUADS);
	for(i=0;i<STARS;i++)
	{
		// glPushMatrix();
			// glRotateZi(stars[i].angle.z+8192);
			// glRotatef32i(stars[i].angle.x,cosLerp(0),sinLerp(0),0);
			// glTranslatef32(0,inttof32(15),0);
			// glBegin(GL_QUADS);
				// GFX_VERTEX10 = NORMAL_PACK((stars[i].size),(0),(-stars[i].size));
				// GFX_VERTEX10 = NORMAL_PACK((stars[i].size),(0),(stars[i].size));
				// GFX_VERTEX10 = NORMAL_PACK((-stars[i].size),(0),(stars[i].size));
				// GFX_VERTEX10 = NORMAL_PACK((-stars[i].size),(0),(-stars[i].size));
				GFX_VERTEX10 = NORMAL_PACK(((mulf32(cosLerp(stars[i].angle.z+8192),cosLerp(stars[i].angle.x))>>3)+(stars[i].size)),((mulf32(sinLerp(stars[i].angle.z+8192),cosLerp(stars[i].angle.x))>>3)+(-stars[i].size)),((-sinLerp(stars[i].angle.x)>>3)));
				GFX_VERTEX10 = NORMAL_PACK(((mulf32(cosLerp(stars[i].angle.z+8192),cosLerp(stars[i].angle.x))>>3)+(stars[i].size)),((mulf32(sinLerp(stars[i].angle.z+8192),cosLerp(stars[i].angle.x))>>3)+(stars[i].size)),((-sinLerp(stars[i].angle.x)>>3)));
				GFX_VERTEX10 = NORMAL_PACK(((mulf32(cosLerp(stars[i].angle.z+8192),cosLerp(stars[i].angle.x))>>3)+(-stars[i].size)),((mulf32(sinLerp(stars[i].angle.z+8192),cosLerp(stars[i].angle.x))>>3)+(stars[i].size)),((-sinLerp(stars[i].angle.x)>>3)));
				GFX_VERTEX10 = NORMAL_PACK(((mulf32(cosLerp(stars[i].angle.z+8192),cosLerp(stars[i].angle.x))>>3)+(-stars[i].size)),((mulf32(sinLerp(stars[i].angle.z+8192),cosLerp(stars[i].angle.x))>>3)+(-stars[i].size)),((-sinLerp(stars[i].angle.x)>>3)));
		// glPopMatrix(1);
	}*/
	glPopMatrix(1);
}

void drawCloud()
{
	// glBindTexture(0, 0);
	glPolyFmt(POLY_ALPHA(3+((31-3)*dayTime)/4096) | POLY_CULL_BACK | POLY_ID(11));
	glPushMatrix();
	glScalef32(inttof32(SCALEFACTOR),inttof32(SCALEFACTOR),inttof32(SCALEFACTOR));
		Game_ApplyMTL(cloudTexture);
		glColor3b(255,255,255);
		glBegin(GL_QUADS);	
			//top
			GFX_TEX_COORD = TEXTURE_PACK(16*(0)+cloudcnt+((Player.position.y+map.offset.y*bsize*SCALEFACTOR)>>8), 16*0+((Player.position.x+map.offset.x*bsize*SCALEFACTOR)>>8));
			GFX_VERTEX10 = NORMAL_PACK((-CLOUDSIZE),(-CLOUDSIZE),(0+tilesize2*CLUSTERSIZE*16));
			GFX_TEX_COORD = TEXTURE_PACK(16*(64)+cloudcnt+((Player.position.y+map.offset.y*bsize*SCALEFACTOR)>>8), 16*0+((Player.position.x+map.offset.x*bsize*SCALEFACTOR)>>8));
			GFX_VERTEX10 = NORMAL_PACK((-CLOUDSIZE),(CLOUDSIZE),(0+tilesize2*CLUSTERSIZE*16));
			GFX_TEX_COORD = TEXTURE_PACK(16*(64)+cloudcnt+((Player.position.y+map.offset.y*bsize*SCALEFACTOR)>>8), 16*(64)+((Player.position.x+map.offset.x*bsize*SCALEFACTOR)>>8));
			GFX_VERTEX10 = NORMAL_PACK((CLOUDSIZE),(CLOUDSIZE),(0+tilesize2*CLUSTERSIZE*16));
			GFX_TEX_COORD = TEXTURE_PACK(16*(0)+cloudcnt+((Player.position.y+map.offset.y*bsize*SCALEFACTOR)>>8), 16*(64)+((Player.position.x+map.offset.x*bsize*SCALEFACTOR)>>8));
			GFX_VERTEX10 = NORMAL_PACK((CLOUDSIZE),(-CLOUDSIZE),(0+tilesize2*CLUSTERSIZE*16));

		glEnd();
	glPopMatrix(1);
	cloudcnt++;
	if(cloudcnt>16*(256))cloudcnt-=16*(256);
}

void drawLogo()
{
	// glBindTexture(0, 0);
	glPolyFmt(POLY_ALPHA(20) | POLY_CULL_BACK | POLY_ID(41));
	glPushMatrix();
	glScalef32(inttof32(SCALEFACTOR),inttof32(SCALEFACTOR),inttof32(SCALEFACTOR));
		// Game_ApplyMTL(cloudTexture);
		glColor3b(255,255,255);
		glBegin(GL_QUADS);
			//top
			GFX_TEX_COORD = TEXTURE_PACK(16*(0), 16*(64));
			GFX_VERTEX10 = NORMAL_PACK((-LOGOSIZE/2),(-LOGOSIZE),(0+tilesize2*CLUSTERSIZE*16));
			GFX_TEX_COORD = TEXTURE_PACK(16*(256), 16*(64));
			GFX_VERTEX10 = NORMAL_PACK((-LOGOSIZE/2),(LOGOSIZE),(0+tilesize2*CLUSTERSIZE*16));
			GFX_TEX_COORD = TEXTURE_PACK(16*(256), 16*(0));
			GFX_VERTEX10 = NORMAL_PACK((LOGOSIZE/2),(LOGOSIZE),(0+tilesize2*CLUSTERSIZE*16));
			GFX_TEX_COORD = TEXTURE_PACK(16*(0), 16*(0));
			GFX_VERTEX10 = NORMAL_PACK((LOGOSIZE/2),(-LOGOSIZE),(0+tilesize2*CLUSTERSIZE*16));

		glEnd();
	glPopMatrix(1);
	cloudcnt++;
}

void drawSun()
{
	sunX+=3;
	dayTime=((sunX<16384)?(8192-abs(sunX-8192)):0);
	dayTime=(dayTime<4096)?(dayTime):4096;
	// glPolyFmt(POLY_ALPHA(31) /*| POLY_FORMAT_LIGHT0 | POLY_FORMAT_LIGHT1*/ | POLY_CULL_NONE);
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_ID(31));
	glPushMatrix();
	glScalef32(inttof32(SCALEFACTOR),inttof32(SCALEFACTOR),inttof32(SCALEFACTOR));
		// glScalef32(inttof32(8),inttof32(8),inttof32(1));
		// glTranslatef32(-(mulf32(sinLerp(sunZ),cosLerp(sunX))), -(mulf32(cosLerp(sunZ),cosLerp(sunX))), -(-(sinLerp(sunX))));
		// glRotateXi(-sunX);
		glRotateZi(sunZ+8192);
		// glRotatef32i(-sunX,cosLerp(-sunZ+8192),sinLerp(-sunZ+8192),0);
		// glRotatef32i(-sunX,cosLerp(8192),sinLerp(8192),0);
		glRotatef32i(sunX,cosLerp(0),sinLerp(0),0);
		// glTranslatef32(inttof32(1),0,0);
		glTranslatef32(0,inttof32(7),0);
		// Game_ApplyMTL(NULL);
		Game_ApplyMTL(sunTexture);
		glColor3b(255,255,255);
		glBegin(GL_QUADS);	
			//top
			GFX_TEX_COORD = TEXTURE_PACK(16*(0), 16*(64));
			GFX_VERTEX10 = NORMAL_PACK((SUNSIZE),(0),(-SUNSIZE));
			GFX_TEX_COORD = TEXTURE_PACK(16*(64), 16*(64));
			GFX_VERTEX10 = NORMAL_PACK((SUNSIZE),(0),(SUNSIZE));
			GFX_TEX_COORD = TEXTURE_PACK(16*(64), 16*0);
			GFX_VERTEX10 = NORMAL_PACK((-SUNSIZE),(0),(SUNSIZE));
			GFX_TEX_COORD = TEXTURE_PACK(16*(0), 16*0);
			GFX_VERTEX10 = NORMAL_PACK((-SUNSIZE),(0),(-SUNSIZE));

		glEnd();
	glPopMatrix(1);
	sunZ%=32768;
	sunX%=32768;
	if(!envMenu)
	{
		if(dayTime)
		{
			vect3D lightDir=(vect3D){(mulf32(sinLerp(sunZ),cosLerp(sunX))),(mulf32(cosLerp(sunZ),cosLerp(sunX))),(-(sinLerp(sunX)))};
			lightSun[1]=min(max(f32toint(31*lightDir.z),0)+SUNAMBIENT,31);
			lightSun[0]=min(max(f32toint(31*(-lightDir.z)),0)+SUNAMBIENT,31);
			lightSun[3]=min(max(f32toint(31*lightDir.y),0)+SUNAMBIENT,31);
			lightSun[2]=min(max(f32toint(31*(-lightDir.y)),0)+SUNAMBIENT,31);
			lightSun[5]=min(max(f32toint(31*lightDir.x),0)+SUNAMBIENT,31);
			lightSun[4]=min(max(f32toint(31*(-lightDir.x)),0)+SUNAMBIENT,31);
		}else{
			vect3D lightDir=(vect3D){(mulf32(sinLerp(-sunZ+16384),cosLerp(-sunX))),(mulf32(cosLerp(-sunZ+16384),cosLerp(-sunX))),-((sinLerp(-sunX)))};
			lightSun[1]=min(max(f32toint(MOONLIGHT*lightDir.z),0),31);
			lightSun[0]=min(max(f32toint(MOONLIGHT*(-lightDir.z)),0),31);
			lightSun[3]=min(max(f32toint(MOONLIGHT*lightDir.y),0),31);
			lightSun[2]=min(max(f32toint(MOONLIGHT*(-lightDir.y)),0),31);
			lightSun[5]=min(max(f32toint(MOONLIGHT*lightDir.x),0),31);
			lightSun[4]=min(max(f32toint(MOONLIGHT*(-lightDir.x)),0),31);
		}
		// PROF_START();
		updateLightMap();
		// PROF_END(TESTVALUE);
	}
}

void drawDawn()
{
	if(!dayTime || dayTime>1024+512)return;
	if(dayTime<1024+512)glareLength=(31*(((1024+512)/2)-abs(((1024+512)/2)-dayTime)))/((1024+512)/2);
	else glareLength=0;
	// glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_FRONT);
	glPushMatrix();
	glScalef32(inttof32(SCALEFACTOR),inttof32(SCALEFACTOR),inttof32(SCALEFACTOR));
	u16 dawnHeight=(DAWNSIZE+96)*glareLength/31;
	// NOGBA("height : %d",dawnHeight);
		glRotateZi(sunZ+8192);
		if(sunX<1024+512)glRotatef32i(min(sunX,256),cosLerp(0),sinLerp(0),0);
		// else if(sunX>16384-1024)glRotatef32i(max(sunX,16384-256),cosLerp(0),sinLerp(0),0);
		else if(sunX>16384-(1024+512))glRotateZi(8192*2);
		glTranslatef32(0,inttof32(15),0);
		glScalef32(inttof32(6),inttof32(6),inttof32(6));
		Game_ApplyMTL(NULL);
		// glColor3b(255,255,255);
		glBegin(GL_QUAD_STRIP);	
			glColor(RGB15(glareR,glareG,glareB));
			GFX_VERTEX10 = NORMAL_PACK((-DAWNSIZE*2),(-DAWNSIZE),(-DAWNSIZE/3));
			glColor(RGB15(skyR,skyG,skyB));
			GFX_VERTEX10 = NORMAL_PACK((-DAWNSIZE*2),(-DAWNSIZE),(-(DAWNSIZE)/3)+8);
			// glColor(RGB15(glareR,glareG,glareB));
			// GFX_VERTEX10 = NORMAL_PACK((-DAWNSIZE-DAWNSIZE/2),(-DAWNSIZE/2),(-DAWNSIZE/2));
			// glColor(RGB15(skyR,skyG,skyB));
			// GFX_VERTEX10 = NORMAL_PACK((-DAWNSIZE-DAWNSIZE/2),(-DAWNSIZE/2),((DAWNSIZE)/4));
			
			glColor(RGB15(glareR,glareG,glareB));
			GFX_VERTEX10 = NORMAL_PACK((-DAWNSIZE),(0),(-DAWNSIZE/3));
			glColor(RGB15(skyR,skyG,skyB));
			GFX_VERTEX10 = NORMAL_PACK((-DAWNSIZE),(0),((dawnHeight)/3));
			glColor(RGB15(glareR,glareG,glareB));
			GFX_VERTEX10 = NORMAL_PACK((0),(0),(-DAWNSIZE/4));
			glColor(RGB15(skyR,skyG,skyB));
			GFX_VERTEX10 = NORMAL_PACK((0),(0),((dawnHeight)/2));
			glColor(RGB15(glareR,glareG,glareB));
			GFX_VERTEX10 = NORMAL_PACK((DAWNSIZE),(0),(-DAWNSIZE/3));
			glColor(RGB15(skyR,skyG,skyB));
			GFX_VERTEX10 = NORMAL_PACK((DAWNSIZE),(0),((dawnHeight)/3));
			
			// glColor(RGB15(glareR,glareG,glareB));
			// GFX_VERTEX10 = NORMAL_PACK((DAWNSIZE+DAWNSIZE/2),(-DAWNSIZE/2),(-DAWNSIZE/2));
			// glColor(RGB15(skyR,skyG,skyB));
			// GFX_VERTEX10 = NORMAL_PACK((DAWNSIZE+DAWNSIZE/2),(-DAWNSIZE/2),((DAWNSIZE)/4));
			glColor(RGB15(glareR,glareG,glareB));
			GFX_VERTEX10 = NORMAL_PACK((DAWNSIZE*2-1),(-DAWNSIZE),(-DAWNSIZE/3));
			glColor(RGB15(skyR,skyG,skyB));
			GFX_VERTEX10 = NORMAL_PACK((DAWNSIZE*2-1),(-DAWNSIZE),(-DAWNSIZE/3+8));
			
			
			// GFX_TEX_COORD = TEXTURE_PACK(16*(128), 16*(128));
			// GFX_VERTEX10 = NORMAL_PACK((DAWNSIZE),(0),(-DAWNSIZE/2));
			// GFX_TEX_COORD = TEXTURE_PACK(16*(128), 16*0);
			// GFX_VERTEX10 = NORMAL_PACK((DAWNSIZE),(0),((DAWNSIZE)/2));
			// GFX_TEX_COORD = TEXTURE_PACK(16*(0), 16*0);
			// GFX_VERTEX10 = NORMAL_PACK((-DAWNSIZE),(0),((DAWNSIZE)/2));
			// GFX_TEX_COORD = TEXTURE_PACK(16*(0), 16*(128));
			// GFX_VERTEX10 = NORMAL_PACK((-DAWNSIZE),(0),(-DAWNSIZE/2));

		glEnd();
	glPopMatrix(1);
}

void drawMoon()
{
	nightTime=((sunX<16384)?0:(8192-abs((sunX-16384)-8192)));
	nightTime=(nightTime<4096)?(nightTime):4096;
	u8 a=((nightTime*31)/4096);
	if(!a)return;
	glPolyFmt(POLY_ALPHA(a) | POLY_CULL_BACK | POLY_ID(21));
	glPushMatrix();
	glScalef32(inttof32(SCALEFACTOR),inttof32(SCALEFACTOR),inttof32(SCALEFACTOR));
		glRotateZi((-sunZ+8192+16384));
		glRotatef32i(-sunX,cosLerp(0),sinLerp(0),0);
		glTranslatef32(0,inttof32(7),0);
		Game_ApplyMTL(moonTexture);
		glColor3b(255,255,255);
		glBegin(GL_QUADS);	
			//top
			GFX_TEX_COORD = TEXTURE_PACK(16*(0), 16*(64));
			GFX_VERTEX10 = NORMAL_PACK((SUNSIZE),(0),(-SUNSIZE));
			GFX_TEX_COORD = TEXTURE_PACK(16*(64), 16*(64));
			GFX_VERTEX10 = NORMAL_PACK((SUNSIZE),(0),(SUNSIZE));
			GFX_TEX_COORD = TEXTURE_PACK(16*(64), 16*0);
			GFX_VERTEX10 = NORMAL_PACK((-SUNSIZE),(0),(SUNSIZE));
			GFX_TEX_COORD = TEXTURE_PACK(16*(0), 16*0);
			GFX_VERTEX10 = NORMAL_PACK((-SUNSIZE),(0),(-SUNSIZE));

		glEnd();
	glPopMatrix(1);
}

void drawSky(void)
{
		skyR=(8)/8+((107-8)*(dayTime))/(32768);
		skyG=(11)/8+((183-11)*(dayTime))/(32768);
		skyB=(30)/8+((228-30)*(dayTime))/(32768);
		glClearColor(skyR,skyG,skyB,31);
}
