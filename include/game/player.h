#ifndef PLAYER_9
#define PLAYER_9

#define GRAVITY 70
#define BBSIZE 1000

typedef struct
{
	vect3D position, vector, clusterCoord;
	s16 angleZ, angleX;
	u8 inWater, onLadder, inCave;
}player_struct;

typedef struct
{
	u8 controls;
	char texturePack[511];
}settings_struct;

settings_struct gameSettings;

void loadSettings(void);
void initPlayer(player_struct* p);
void updatePlayer(player_struct* p);
void playerCamera(player_struct* p, bool environment);
vect3D getPointBlockPos(map_struct* m, int32 i, int32 j, int32 k);

player_struct Player;

s16 walkAngle;
s8 gravityDiv;

bool noclip;

u32 testCursor, testCursorI, testCursorJ, testCursorK;
u8 cursorBlock;

#endif