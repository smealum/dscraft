#include "game/game_main.h"
#ifdef SURVIVAL
#define PAS 16

MTL_img* creeperTex;

void initMobs(void)
{
	mobList.first=NULL;
	mobList.count=0;
	creeperTex=Game_CreateTexture("creeper.pcx", "");
}

void addMob(map_struct* m, vect3D pos, u8 type)
{
	mobList_element* mb=malloc(sizeof(mobList_element));
	mb->mob.position=pos;
	mb->mob.position=(vect3D){pos.x+(m->offset.x+SUPERCLUSTERSIZE/2)*CLUSTERSIZE*rTilesize2,pos.y+(m->offset.y+SUPERCLUSTERSIZE/2)*CLUSTERSIZE*rTilesize2,pos.z};
	mb->mob.vector=(vect3D){0,0,0};
	mb->mob.type=type;
	mb->mob.id=mobList.count++;
	mb->next=mobList.first;
	mobList.first=mb;
}

void drawMob(map_struct* m, mob_struct* mb)
{
	glPushMatrix();
		glTranslatef32(mb->colPosition.x, mb->colPosition.y, mb->colPosition.z+2500);
		Game_ApplyMTL(creeperTex);
		glRotateZi(mb->angleZ);
		glCallList(creeper_bin);
	glPopMatrix(1);
}

void updateMobs(map_struct* m)
{
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0 | POLY_ID(47));
	
	#ifdef DEBUGMODE2
		PROF_START();
	#endif

	mobList_element* mb=mobList.first;
	while(mb)
	{
		//ai
			//follow
			/*mb->mob.angleZ=-getAngle(mb->mob.colPosition.x, mb->mob.colPosition.y, Player.position.x, Player.position.y)*64+8192;
			mb->mob.vector.x=sinLerp(mb->mob.angleZ)>>5;
			mb->mob.vector.y=-cosLerp(mb->mob.angleZ)>>5;*/
			//random
			mb->mob.vector.x+=(rand()%384)-192;
			mb->mob.vector.y+=(rand()%384)-192;
		vect3D oldVect=mb->mob.vector;
		
		//"physics"
		mb->mob.vector.z-=GRAVITY*gravityDiv;
		// NOGBA("mob : %d %d %d",mb->mob.position.x,mb->mob.position.y,mb->mob.position.z);
		mb->mob.colPosition=(vect3D){mb->mob.position.x-(m->offset.x+SUPERCLUSTERSIZE/2)*CLUSTERSIZE*rTilesize2,mb->mob.position.y-(m->offset.y+SUPERCLUSTERSIZE/2)*CLUSTERSIZE*rTilesize2,mb->mob.position.z};
		testPoint(m, mb->mob.colPosition, &mb->mob.vector);
		mb->mob.position.x+=mb->mob.vector.x;mb->mob.position.y+=mb->mob.vector.y;mb->mob.position.z+=mb->mob.vector.z;
		if(!mb->mob.vector.z && ((!mb->mob.vector.x && oldVect.x) || (!mb->mob.vector.y && oldVect.y)))mb->mob.vector.z+=850;
		if(mb->mob.vector.x>=PAS)mb->mob.vector.x-=PAS;
		else if(mb->mob.vector.x<=PAS)mb->mob.vector.x+=PAS;
		else mb->mob.vector.x=0;
		if(mb->mob.vector.y>=PAS)mb->mob.vector.y-=PAS;
		else if(mb->mob.vector.y<=PAS)mb->mob.vector.y+=PAS;
		else mb->mob.vector.y=0;
		// mb->mob.vector.x=0;mb->mob.vector.y=0;
		
		//rendering
		drawMob(m, &mb->mob);
		
		mb=mb->next;
	}
	#ifdef DEBUGMODE2
		int time;PROF_END(time);
		iprintf("\nmobs : %d   ",time);
	#endif
}

#endif
