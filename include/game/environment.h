#ifndef ENVIRONMENT_9
#define ENVIRONMENT_9

#define STARS 32
// #define SUNSIZE 64
#define SUNSIZE 96
#define STARSIZE 5
#define DAWNSIZE 256
#define CLOUDSIZE 511

typedef struct
{
	vect3D angle;
	u8 size, shiny;
}star_struct;

star_struct stars[STARS];

MTL_img *cloudTexture, *sunTexture, *moonTexture, *dawnTexture, *crossHair;

int sunX, sunZ;
int dayTime, nightTime;

u8 skyR, skyG, skyB;
u8 glareR, glareG, glareB;
u8 glareLength;

bool envMenu;
s16 cloudcnt;

void drawSky(void);
void drawSun(void);
void drawMoon(void);
void drawDawn(void);
void initStars(void);
void drawStars(void);
void drawCloud(void);
void freeEnvironment(void);
void initEnvironment(bool menu);

#endif