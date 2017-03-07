#include "game/game_main.h"

#include <maxmod9.h>

#include "soundbank.h"
#include "soundbank_bin.h"

vect3D tempAngle;
u8 oldwAngle;
u8 walkSfx;

void initControls(void)
{
	lastXY=(touchPosition){0,0,0,0};
	action=0;
}

void placeBlock(void)
{
	mmEffect(SFX_ADD);
	map_struct* m=&map;
	int i=testCursorI, j=testCursorJ, k=testCursorK;
	u8 ot=*getBlockP(m,i,j,k);
	if(ot>=DOORTYPE && ot<DOORTYPE+8)
	{
		if((ot-DOORTYPE)%2)k++;
		ot=(ot-DOORTYPE-((ot-DOORTYPE)%2))/2;
		removeBlock(m, i, j, k, false);
		removeBlock(m, i, j, k-1, false);
		*getBlockP(m,i,j,k)+=8;
		*getBlockP(m,i,j,k-1)+=8;
		u8 dir;
		switch(ot)
		{
			case 0:
				dir=4;
				break;
			case 1:
				dir=5;
				break;
			case 2:
				dir=3;
				break;
			default:
				dir=2;
				break;
		}
		vect3D clusterCoord=getCluster(m,i,j,k);
		quadList_struct* ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].quadList;
		vect3D clusterCoord2=getCluster(m,i,j,k-1);
		quadList_struct* ql2=&m->superCluster[clusterCoord2.x-m->offset.x][clusterCoord2.y-m->offset.y]->cluster[clusterCoord2.z-m->offset.z].quadList;
		u8 light;
		int bid;
		surface(m, i, j, k, &light);
		getLight(m, i, j, k, &light, dir);
		addQuad(ql, m, dir, light, bid, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);
		surface(m, i, j, k, &light);
		getLight(m, i, j, k, &light, dir+8);
		addQuad(ql, m, dir+8, light, bid, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);
		surface(m, i, j, k-1, &light);
		getLight(m, i, j, k-1, &light, dir);
		addQuad(ql2, m, dir, light, bid, m->superCluster[clusterCoord2.x-m->offset.x][clusterCoord2.y-m->offset.y]->data, i, j, k-1);
		surface(m, i, j, k-1, &light);
		getLight(m, i, j, k-1, &light, dir+8);
		addQuad(ql2, m, dir+8, light, bid, m->superCluster[clusterCoord2.x-m->offset.x][clusterCoord2.y-m->offset.y]->data, i, j, k-1);
		return;
	}else if(ot>=DOORTYPE+8 && ot<DOORTYPE+16)
	{
		if((ot-DOORTYPE)%2)k++;
		ot=(ot-DOORTYPE-((ot-DOORTYPE)%2)-8)/2;
		removeBlock(m, i, j, k, false);
		removeBlock(m, i, j, k-1, false);
		*getBlockP(m,i,j,k)-=8;
		*getBlockP(m,i,j,k-1)-=8;
		u8 dir;
		switch(ot)
		{
			case 2:
				dir=4;
				break;
			case 3:
				dir=5;
				break;
			case 1:
				dir=3;
				break;
			default:
				dir=2;
				break;
		}
		vect3D clusterCoord=getCluster(m,i,j,k);
		quadList_struct* ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].quadList;
		vect3D clusterCoord2=getCluster(m,i,j,k-1);
		quadList_struct* ql2=&m->superCluster[clusterCoord2.x-m->offset.x][clusterCoord2.y-m->offset.y]->cluster[clusterCoord2.z-m->offset.z].quadList;
		u8 light;
		int bid;
		surface(m, i, j, k, &light);
		getLight(m, i, j, k, &light, dir);
		addQuad(ql, m, dir, light, bid, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);
		surface(m, i, j, k, &light);
		getLight(m, i, j, k, &light, dir+8);
		addQuad(ql, m, dir+8, light, bid, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);
		surface(m, i, j, k-1, &light);
		getLight(m, i, j, k-1, &light, dir);
		addQuad(ql2, m, dir, light, bid, m->superCluster[clusterCoord2.x-m->offset.x][clusterCoord2.y-m->offset.y]->data, i, j, k-1);
		surface(m, i, j, k-1, &light);
		getLight(m, i, j, k-1, &light, dir+8);
		addQuad(ql2, m, dir+8, light, bid, m->superCluster[clusterCoord2.x-m->offset.x][clusterCoord2.y-m->offset.y]->data, i, j, k-1);
		return;
	}
	switch(cursorDir)
	{
		case 0:
			testCursorK++;
			break;
		case 1:
			testCursorK--;
			break;
		case 2:
			testCursorI++;
			break;
		case 3:
			testCursorI--;
			break;
		case 4:
			testCursorJ++;
			break;
		case 5:
			testCursorJ--;
			break;
	}
	testCursor=(testCursorI)+(testCursorJ)*(map).size.x+(testCursorK)*(map).size.y*(map).size.x;
	if(cursorBlock==11)
	{
		changeBlock(&map, testCursorI, testCursorJ, testCursorK, WATERTYPE);
		addWater(&map, testCursorI, testCursorJ, testCursorK, WATERTYPE);//TEST
	}else if(cursorBlock==LADDERTYPE)
	{
		if(cursorDir<2)return;
		if(isLadder(*getBlockP(m,i,j,k)) || isDoor(*getBlockP(m,i,j,k)) || *getBlockP(m,i,j,k)==13)return;
		i=testCursorI;j=testCursorJ;k=testCursorK;
		u8 *d=(getBlockP(m,i,j,k));
		if(*d)return;
		*d=LADDERTYPE+cursorDir-2;
		vect3D clusterCoord=getCluster(m,i,j,k);
		quadList_struct* ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].quadList;
		
		u32 bid=(i)+(j)*(m)->size.x+(k)*(m)->size.y*(m)->size.x;
		u8 light=0;
		surface(m, i, j, k, &light);
		u8 dir;
		switch(cursorDir)
		{
			case 2:
				dir=3;
				break;
			case 3:
				dir=2;
				break;
			case 4:
				dir=5;
				break;
			case 5:
				dir=4;
				break;
		}
		getLight(m, i, j, k, &light, 10+dir-2);
		addQuad(ql, m, 10+dir-2, light, bid, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);
		// changeBlock(&map, testCursorI, testCursorJ, testCursorK, LADDERTYPE);
	}else if(cursorBlock==DOORTYPE)
	{
		if(cursorDir)return;
		i=testCursorI;j=testCursorJ;k=testCursorK;
		u8 *d=(getBlockP(m,i,j,k));
		if(*d)return;
		i=testCursorI;j=testCursorJ;k=testCursorK+1;
		u8 *d2=(getBlockP(m,i,j,k));
		if(*d2)return;
		u8 dir;
		if(Player.angleZ<4096 || Player.angleZ>=32768-4096)
		{
			dir=5;
		}else if(Player.angleZ>=4096 && Player.angleZ<4096+8192)
		{
			dir=3;
		}else if(Player.angleZ>=4096+8192 && Player.angleZ<4096+8192+8192)
		{
			dir=4;
		}else{
			dir=2;
		}
		*d=DOORTYPE+1+(dir-2)*2;
		*d2=DOORTYPE+(dir-2)*2;
		vect3D clusterCoord=getCluster(m,i,j,k);
		quadList_struct* ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].quadList;
		vect3D clusterCoord2=getCluster(m,i,j,k-1);
		quadList_struct* ql2=&m->superCluster[clusterCoord2.x-m->offset.x][clusterCoord2.y-m->offset.y]->cluster[clusterCoord2.z-m->offset.z].quadList;
		u8 light;
		int bid;
		surface(m, i, j, k, &light);
		getLight(m, i, j, k, &light, dir);
			addQuad(ql, m, dir, light, bid, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);
		surface(m, i, j, k, &light);
		getLight(m, i, j, k, &light, dir+8);
			addQuad(ql, m, dir+8, light, bid, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);
		surface(m, i, j, k-1, &light);
		getLight(m, i, j, k-1, &light, dir);
			addQuad(ql2, m, dir, light, bid, m->superCluster[clusterCoord2.x-m->offset.x][clusterCoord2.y-m->offset.y]->data, i, j, k-1);
		surface(m, i, j, k-1, &light);
		getLight(m, i, j, k-1, &light, dir+8);
			addQuad(ql2, m, dir+8, light, bid, m->superCluster[clusterCoord2.x-m->offset.x][clusterCoord2.y-m->offset.y]->data, i, j, k-1);
			m->superCluster[clusterCoord2.x-m->offset.x][clusterCoord2.y-m->offset.y]->changed=1;
			m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->changed=1;
		// changeBlock(&map, testCursorI, testCursorJ, testCursorK, DOORTYPE+1);
		// changeBlock(&map, testCursorI, testCursorJ, testCursorK+1, DOORTYPE);
	}else{
		changeBlock(&map, testCursorI, testCursorJ, testCursorK, cursorBlock);
	}
}

void destroyBlock(void)
{
	mmEffect(SFX_REMOVE);
	map_struct* m=&map;
	int i=testCursorI, j=testCursorJ, k=testCursorK;
	u8 ot=*getBlockP(m,i,j,k);
	if(ot>=DOORTYPE && ot<DOORTYPE+16)
	{
		if((ot-DOORTYPE)%2)k++;
		ot=(ot-DOORTYPE-((ot-DOORTYPE)%2))/2;
		removeBlock(m, i, j, k, false);
		removeBlock(m, i, j, k-1, false);
		*getBlockP(m,i,j,k)=0;
		*getBlockP(m,i,j,k-1)=0;
		return;
	}
	changeBlock(&map, testCursorI, testCursorJ, testCursorK, 0);
}

u8 holdABXY;
u8 doubletap;

void controlScheme1(void)
{
	u16 keys = keysHeld();
	// if((keysHeld() & KEY_A)) {sunZ += 400;}
	// if((keysDown() & KEY_START)) {DS_ChangeState(&Game_State);}
	// if((keysHeld() & KEY_Y) && (keysHeld() & KEY_START)) {sunX += 400;}
	// if((keysHeld() & KEY_Y)) {sunX += 20;}
	// iprintf("\nsun %d %d    ",sunX,sunZ);
	// if(!noclip && (Player.inWater || !Player.vector.z) && (keysDown() & KEY_A)) {Player.vector.z += 850;}
	if(!noclip)
	{
		// if((keysDown() & KEY_A)) {Player.vector.z += 1400;}
		if((keys & KEY_LEFT) || (keys & KEY_Y)) {walkSfx++;Player.vector.y += cosLerp(Player.angleZ-8192)>>3;Player.vector.x += sinLerp(Player.angleZ-8192)>>3;if(!Player.vector.z)walkAngle+=2500;}
		if((keys & KEY_RIGHT) || (keys & KEY_A)) {walkSfx++;Player.vector.y += cosLerp(Player.angleZ+8192)>>3;Player.vector.x += sinLerp(Player.angleZ+8192)>>3;if(!Player.vector.z)walkAngle+=2500;}
	}else{
		if((keys & KEY_LEFT) || (keys & KEY_Y)) tempAngle.z -= 500;
		if((keys & KEY_RIGHT) || (keys & KEY_A)) tempAngle.z += 500;
	}
	
		// if((keysDown() & KEY_X)) noclip^=1;//DEBUG (speed)
		// if(noclip)if((keys & KEY_UP) || (!gameSettings.controls && (keys & KEY_X))) {Player.vector.y = mulf32(cosLerp(Player.angleZ),cosLerp(Player.angleX))>>1;Player.vector.x = mulf32(sinLerp(Player.angleZ),cosLerp(Player.angleX))>>1;Player.vector.z = -sinLerp(Player.angleX)>>1;}
		
		
	touchRead(&thisXY);
	
	if(doubletap)doubletap++;
	if(doubletap>15)doubletap=0;
	if((keysDown() & KEY_TOUCH) && !doubletap)doubletap=1;
	else if((keysDown() & KEY_TOUCH) && doubletap && !noclip && (Player.inWater || !Player.vector.z) && !invOpen && !overButtons){Player.vector.z += 850;doubletap=0;}
	
	if(updateInterface() && !overButtons && (keysHeld() & KEY_TOUCH))
	{
		if(!(keysDown() & KEY_TOUCH))
		{
			int16 dx = thisXY.px - lastXY.px;
			int16 dy = thisXY.py - lastXY.py;

			// filtering measurement errors
			if (dx<20 && dx>-20 && dy<20 && dy>-20)
			{
				if(dx>-2&&dx<2)
					dx=0;

				if(dy>-2&&dy<2)
					dy=0;

					tempAngle.x += degreesToAngle(dy);
					tempAngle.z += degreesToAngle(dx);
			}

		}
	}
	if(keysHeld() & KEY_TOUCH)lastXY = thisXY;
	
	/*if((keysUp() & KEY_X))noclip=!noclip;
	if((keysUp() & KEY_START))
	{
		writeStats(&frameTime,"frame","stats");
		writeStats(&streamRead,"streaming","stats");
		writeStats(&streamCalc,"streaming2","stats");
		char str[255];
		sprintf(str, "write (%d)",TESTVALUE3);
		writeStats(&columnWrite,str,"stats");
		writeStats(&freeRam,"ram","stats");
	}*/
	
	// iprintf("\nselected block : %d    ",cursorBlock);
	// iprintf("\noffset : %d %d %d    ",map.offset.x,map.offset.y,map.offset.z);
	
	testCursorI=testCursor%map.size.x;
	testCursorJ=((testCursor-testCursorI)%(map.size.x*map.size.y))/(map.size.x);
	testCursorK=(testCursor-testCursorI-testCursorJ*(map.size.x))/(map.size.x*map.size.y);
	#ifdef DEBUGMODE
	iprintf("\ncursor1 : %d %d %d %d   ",testCursorI,testCursorJ,testCursorK, testCursor);
	#endif
	if((keysUp() & KEY_R) || (keysUp() & KEY_L)){
		cubeAngleX=-1;
		if(action)
		{
			destroyBlock();
		}else{
			placeBlock();
		}
	}
}

void controlScheme2(void)
{
	u16 keys = keysHeld();

	if((keys & KEY_LEFT)) tempAngle.z -= 500;
	if((keys & KEY_RIGHT)) tempAngle.z += 500;
	if((keys & KEY_X)) tempAngle.x -= 500;
	if((keys & KEY_B)) tempAngle.x += 500;
	if((keysDown() & KEY_Y))
	{
		tempCursor++;
		tempCursor%=9;
		// NOGBA("cursor : %d => %d (%d)",tempCursor,cursorBlock,slots[testCursor].id);
		int i;
		for(i=0;i<MAXITEMS;i++)
		{
			if(items[i].used && items[i].slot==tempCursor)
			{
				cursorBlock=items[i].type;
				break;
			}
		}
	}
	if(!noclip && (Player.inWater || !Player.vector.z) && (keysDown() & KEY_A)) {Player.vector.z += 850;}
	if(keysUp() & KEY_R)
	{
		cubeAngleX=-1;
		destroyBlock();
	}else if(keysUp() & KEY_L){
		cubeAngleX=-1;
		placeBlock();
	}
	touchRead(&thisXY);
	updateInterface();
	if(keysHeld() & KEY_TOUCH)lastXY = thisXY;
}

void controlScheme3(void)
{
	u16 keys = keysHeld();

	if((keys & KEY_LEFT)) tempAngle.z -= 500;
	if((keys & KEY_RIGHT)) tempAngle.z += 500;
	if((keys & KEY_X)) tempAngle.x -= 500;
	if((keys & KEY_B)) tempAngle.x += 500;
	if(!noclip && (Player.inWater || !Player.vector.z) && (keysDown() & KEY_A)) {Player.vector.z += 850;}
	if(keysUp() & KEY_Y)
	{
		cubeAngleX=-1;
		if(action)
		{
			destroyBlock();
		}else{
			placeBlock();
		}
	}
	touchRead(&thisXY);
	updateInterface();
	if(keysHeld() & KEY_TOUCH)lastXY = thisXY;
}

void updateControls(void)
{
	if(testBuffer)
	{
		scanKeys();
		tempAngle=(vect3D){0,0,0};
		u16 keys = keysHeld();
		if(!noclip && !Player.inWater && !Player.onLadder)
		{
			// if((keysDown() & KEY_A)) {Player.vector.z += 1400;}
			if((keys & KEY_UP) || (!gameSettings.controls && (keys & KEY_X))) {walkSfx++;Player.vector.y = cosLerp(Player.angleZ)>>3;Player.vector.x = sinLerp(Player.angleZ)>>3;if(!Player.vector.z)walkAngle+=2500;}
			if((keys & KEY_DOWN) || (!gameSettings.controls && (keys & KEY_B))) {walkSfx++;Player.vector.y = -cosLerp(Player.angleZ)>>3;Player.vector.x = -sinLerp(Player.angleZ)>>3;if(!Player.vector.z)walkAngle-=2500;}
		}else if(Player.inWater){
			if((keys & KEY_UP) || (!gameSettings.controls && (keys & KEY_X))) {Player.vector.y = mulf32(cosLerp(Player.angleZ),cosLerp(Player.angleX))>>3;Player.vector.x = mulf32(sinLerp(Player.angleZ),cosLerp(Player.angleX))>>3;Player.vector.z += -sinLerp(Player.angleX)>>6;}
			if((keys & KEY_DOWN) || (!gameSettings.controls && (keys & KEY_B))) {Player.vector.y = -mulf32(cosLerp(Player.angleZ),cosLerp(Player.angleX))>>3;Player.vector.x = -mulf32(sinLerp(Player.angleZ),cosLerp(Player.angleX))>>3;Player.vector.z += sinLerp(Player.angleX)>>6;}
		}else if(Player.onLadder){
			if((keys & KEY_UP) || (!gameSettings.controls && (keys & KEY_X))) {Player.vector.y = mulf32(cosLerp(Player.angleZ),cosLerp(Player.angleX))>>2;Player.vector.x = mulf32(sinLerp(Player.angleZ),cosLerp(Player.angleX))>>2;Player.vector.z = 1024;}
			if((keys & KEY_DOWN) || (!gameSettings.controls && (keys & KEY_B))) {Player.vector.y = -mulf32(cosLerp(Player.angleZ),cosLerp(Player.angleX))>>2;Player.vector.x = -mulf32(sinLerp(Player.angleZ),cosLerp(Player.angleX))>>2;Player.vector.z = -1024;}
		}else{
			if((keys & KEY_UP) || (!gameSettings.controls && (keys & KEY_X))) {Player.vector.y = mulf32(cosLerp(Player.angleZ),cosLerp(Player.angleX))>>3;Player.vector.x = mulf32(sinLerp(Player.angleZ),cosLerp(Player.angleX))>>3;Player.vector.z = -sinLerp(Player.angleX)>>3;}
			if((keys & KEY_DOWN) || (!gameSettings.controls && (keys & KEY_B))) {Player.vector.y = -mulf32(cosLerp(Player.angleZ),cosLerp(Player.angleX))>>3;Player.vector.x = -mulf32(sinLerp(Player.angleZ),cosLerp(Player.angleX))>>3;Player.vector.z = sinLerp(Player.angleX)>>3;}
		}
		if(!(keysHeld() & KEY_A) && !(keysHeld() & KEY_B) && !(keysHeld() & KEY_X) && !(keysHeld() & KEY_Y))holdABXY=false;
		if(!holdABXY && (keysHeld() & KEY_A) && (keysHeld() & KEY_B) && (keysHeld() & KEY_X) && (keysHeld() & KEY_Y))
		{
			holdABXY=true;
			noclip^=1;
		}
		if((keysDown() & KEY_START)) {DS_ChangeState(&Menu_State);}
		if(oldwAngle!=walkSfx && ((walkSfx)>=10) && !Player.vector.z){mmEffect(SFX_STEP);walkSfx=0;}
		oldwAngle=walkSfx;
		switch(gameSettings.controls)
		{
			case 1:
				controlScheme2();
				break;
			case 2:
				controlScheme3();
				break;
			default:
				controlScheme1();
				break;
		}		
	}
	
	Player.angleX+=tempAngle.x/2;
	Player.angleZ+=tempAngle.z/2;
	if(Player.angleX>8192)Player.angleX=8192;
	else if(Player.angleX<-8192)Player.angleX=-8192;
	tempCursor%=9;
}
