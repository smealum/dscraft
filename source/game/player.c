#include "game/game_main.h"

#define sign(a) (((a)==0)?(0):(((a)<0)?(-1):(1)))
#define positive(a) (((a)>0)?(1):(0))

bool IntersectedPolygon(vect3D* vPoly, vect3D* vLine, int verticeCount, int32* originDistance, int32* realDist);
vect3D ClosestPointOnLine(vect3D vA, vect3D vD, int length, int32* dist, vect3D vPoint);
int32 sqMagnitude(vect3D vNormal);
int32 sqDistance(vect3D vA, vect3D vB);
void testPoint(map_struct* m, vect3D point, vect3D* vector);
u8 getPointBlock(map_struct* m, int32 i, int32 j, int32 k);

void initPlayer(player_struct* p)
{
	// p->position=(vect3D){0,0,inttov16(0)};
	// p->position=(vect3D){0,0,(getHighest(&map, SUPERCLUSTERSIZE*CLUSTERSIZE/2, SUPERCLUSTERSIZE*CLUSTERSIZE/2)+3-map.size.z/2)*rTilesize2};
	p->position=(vect3D){(map.header->spawnX-(map.offset.x+SUPERCLUSTERSIZE/2)*CLUSTERSIZE)*rTilesize2,(map.header->spawnY-(map.offset.y+SUPERCLUSTERSIZE/2)*CLUSTERSIZE)*rTilesize2,(getHighest(&map, map.header->spawnX, map.header->spawnY)+3-map.size.z/2)*rTilesize2};
	if(map.header->magicVersionNumber==VERSIONMAGIC)p->position.z=map.header->spawnZ;
	NOGBA("player pos : %d %d",p->position.x,p->position.y,p->position.z);
	p->vector=(vect3D){0,0,0};
	p->angleZ=0;
	p->angleX=0;
	p->inWater=false;
	p->onLadder=false;
	cursorBlock=1;
	gravityDiv=1;
	noclip=false;
}

void loadSettings(void)
{
	short b;
	#ifdef FATONLY
		dictionary* setdic=iniparser_load("settings.ini");//ROOT ?
	#else
		char path[255];sprintf(path,"%s/%s/settings.ini",basePath,ROOT);
		dictionary* setdic=iniparser_load(path);//ROOT ?
	#endif
	sscanf(dictionary_get(setdic,"settings:controls","0"),"%hd",&b);
	gameSettings.controls=b;
	#ifdef FATONLY
		sscanf(dictionary_get(setdic,"settings:texturepack","packs/eldpack"),"%s",gameSettings.texturePack);
	#else
		sscanf(dictionary_get(setdic,"settings:texturepack","nitro:/dscraft/packs/eldpack"),"%s",gameSettings.texturePack);
	#endif
	iniparser_freedict(setdic);
}

void saveSettings(void)
{
	if(!saveAvailable)return;
	#ifdef FATONLY
		FILE* f=fopen("settings.ini","w");
	#else
		char path[255];sprintf(path,"%s/%s/settings.ini",basePath,ROOT);
		FILE* f=fopen(path,"w");
	#endif
	fprintf(f,"[settings]\ncontrols=%d\ntexturepack=%s\n\n\n",gameSettings.controls,gameSettings.texturePack);
	fclose(f);
}

inline vect3D v10vector(map_struct* m, int32 a)
{
	vect3D v;
	v.x=(((a&1023))<<6)*SCALEFACTOR-(SUPERCLUSTERSIZE*CLUSTERSIZE*rTilesize2)/2;
	v.y=(((a>>10)&1023)<<6)*SCALEFACTOR-(SUPERCLUSTERSIZE*CLUSTERSIZE*rTilesize2)/2;
	v.z=(((a>>20)&1023)<<6)*SCALEFACTOR-(map.size.z*rTilesize2)/2;
	return v;
}

s8 clusterCheck(map_struct* m, vect3D* line, int i, int j, int k)
{
	int32 originDistance=0, realDist=0, foDist=inttof32(100);
	s8 dir=-1;
	vect3D poly[4];
	Game_ApplyMTL(NULL);
	u8 t=0;
	if(!tangible(*getBlockP(m,i,j,k+1)))
	{
		t++;
		poly[0]=v10vector(m, NORMAL_PACK((tilesize+tilesize2*i)-m->offset.x*bsizeR,(-tilesize+tilesize2*j)-m->offset.y*bsizeR,(tilesize+tilesize2*k)));
		poly[1]=v10vector(m, NORMAL_PACK((tilesize+tilesize2*i)-m->offset.x*bsizeR,(tilesize+tilesize2*j)-m->offset.y*bsizeR,(tilesize+tilesize2*k)));
		poly[2]=v10vector(m, NORMAL_PACK((-tilesize+tilesize2*i)-m->offset.x*bsizeR,(tilesize+tilesize2*j)-m->offset.y*bsizeR,(tilesize+tilesize2*k)));
		poly[3]=v10vector(m, NORMAL_PACK((-tilesize+tilesize2*i)-m->offset.x*bsizeR,(-tilesize+tilesize2*j)-m->offset.y*bsizeR,(tilesize+tilesize2*k)));
		if(IntersectedPolygon(poly, line, 4, &originDistance, &realDist))
		{
			if(foDist>abs(realDist))
			{
				foDist=abs(realDist);
				dir=0;
			}
		}
	}
	if(!tangible(*getBlockP(m,i,j,k-1)))
	{	
		t++;
		poly[0]=v10vector(m, NORMAL_PACK((-tilesize+tilesize2*i)-m->offset.x*bsizeR,(-tilesize+tilesize2*j)-m->offset.y*bsizeR,(-tilesize+tilesize2*k)));
		poly[1]=v10vector(m, NORMAL_PACK((-tilesize+tilesize2*i)-m->offset.x*bsizeR,(tilesize+tilesize2*j)-m->offset.y*bsizeR,(-tilesize+tilesize2*k)));
		poly[2]=v10vector(m, NORMAL_PACK((tilesize+tilesize2*i)-m->offset.x*bsizeR,(tilesize+tilesize2*j)-m->offset.y*bsizeR,(-tilesize+tilesize2*k)));
		poly[3]=v10vector(m, NORMAL_PACK((tilesize+tilesize2*i)-m->offset.x*bsizeR,(-tilesize+tilesize2*j)-m->offset.y*bsizeR,(-tilesize+tilesize2*k)));
		if(IntersectedPolygon(poly, line, 4, &originDistance, &realDist))
		{
			if(foDist>abs(realDist))
			{
				foDist=abs(realDist);
				dir=1;
			}
		}
	}
	
	if(!tangible(*getBlockP(m,i+1,j,k)))
	{
		t++;
		poly[0]=v10vector(m, NORMAL_PACK((tilesize+tilesize2*i)-m->offset.x*bsizeR,(tilesize+tilesize2*j)-m->offset.y*bsizeR,(tilesize+tilesize2*k)));
		poly[1]=v10vector(m, NORMAL_PACK((tilesize+tilesize2*i)-m->offset.x*bsizeR,(-tilesize+tilesize2*j)-m->offset.y*bsizeR,(tilesize+tilesize2*k)));
		poly[2]=v10vector(m, NORMAL_PACK((tilesize+tilesize2*i)-m->offset.x*bsizeR,(-tilesize+tilesize2*j)-m->offset.y*bsizeR,(-tilesize+tilesize2*k)));
		poly[3]=v10vector(m, NORMAL_PACK((tilesize+tilesize2*i)-m->offset.x*bsizeR,(tilesize+tilesize2*j)-m->offset.y*bsizeR,(-tilesize+tilesize2*k)));
		if(IntersectedPolygon(poly, line, 4, &originDistance, &realDist))
		{
			if(foDist>abs(realDist))
			{
				foDist=abs(realDist);
				dir=2;
			}
		}
	}
	if(!tangible(*getBlockP(m,i-1,j,k)))
	{
		t++;
		poly[0]=v10vector(m, NORMAL_PACK((-tilesize+tilesize2*i)-m->offset.x*bsizeR,(-tilesize+tilesize2*j)-m->offset.y*bsizeR,(tilesize+tilesize2*k)));
		poly[1]=v10vector(m, NORMAL_PACK((-tilesize+tilesize2*i)-m->offset.x*bsizeR,(tilesize+tilesize2*j)-m->offset.y*bsizeR,(tilesize+tilesize2*k)));
		poly[2]=v10vector(m, NORMAL_PACK((-tilesize+tilesize2*i)-m->offset.x*bsizeR,(tilesize+tilesize2*j)-m->offset.y*bsizeR,(-tilesize+tilesize2*k)));
		poly[3]=v10vector(m, NORMAL_PACK((-tilesize+tilesize2*i)-m->offset.x*bsizeR,(-tilesize+tilesize2*j)-m->offset.y*bsizeR,(-tilesize+tilesize2*k)));
		if(IntersectedPolygon(poly, line, 4, &originDistance, &realDist))
		{
			if(foDist>abs(realDist))
			{
				foDist=abs(realDist);
				dir=3;
			}
		}
	}
	
	if(!tangible(*getBlockP(m,i,j+1,k)))
	{
		t++;
		poly[0]=v10vector(m, NORMAL_PACK((-tilesize+tilesize2*i)-m->offset.x*bsizeR,(tilesize+tilesize2*j)-m->offset.y*bsizeR,(tilesize+tilesize2*k)));
		poly[1]=v10vector(m, NORMAL_PACK((tilesize+tilesize2*i)-m->offset.x*bsizeR,(tilesize+tilesize2*j)-m->offset.y*bsizeR,(tilesize+tilesize2*k)));
		poly[2]=v10vector(m, NORMAL_PACK((tilesize+tilesize2*i)-m->offset.x*bsizeR,(tilesize+tilesize2*j)-m->offset.y*bsizeR,(-tilesize+tilesize2*k)));
		poly[3]=v10vector(m, NORMAL_PACK((-tilesize+tilesize2*i)-m->offset.x*bsizeR,(tilesize+tilesize2*j)-m->offset.y*bsizeR,(-tilesize+tilesize2*k)));
		if(IntersectedPolygon(poly, line, 4, &originDistance, &realDist))
		{
			if(foDist>abs(realDist))
			{
				foDist=abs(realDist);
				dir=4;
			}
		}
	}
	if(!tangible(*getBlockP(m,i,j-1,k)))
	{
		t++;
		poly[0]=v10vector(m, NORMAL_PACK((tilesize+tilesize2*i)-m->offset.x*bsizeR,(-tilesize+tilesize2*j)-m->offset.y*bsizeR,(tilesize+tilesize2*k)));
		poly[1]=v10vector(m, NORMAL_PACK((-tilesize+tilesize2*i)-m->offset.x*bsizeR,(-tilesize+tilesize2*j)-m->offset.y*bsizeR,(tilesize+tilesize2*k)));
		poly[2]=v10vector(m, NORMAL_PACK((-tilesize+tilesize2*i)-m->offset.x*bsizeR,(-tilesize+tilesize2*j)-m->offset.y*bsizeR,(-tilesize+tilesize2*k)));
		poly[3]=v10vector(m, NORMAL_PACK((tilesize+tilesize2*i)-m->offset.x*bsizeR,(-tilesize+tilesize2*j)-m->offset.y*bsizeR,(-tilesize+tilesize2*k)));
		if(IntersectedPolygon(poly, line, 4, &originDistance, &realDist))
		{
			if(foDist>abs(realDist))
			{
				foDist=abs(realDist);
				dir=5;
			}
		}
	}
	// NOGBA("\ntlf : %d  ",t);
	return dir;
}

void updatePlayer(player_struct* p)
{
	vect3D lineOfSight=(vect3D){mulf32(sinLerp(Player.angleZ),cosLerp(Player.angleX)),mulf32(cosLerp(Player.angleZ),cosLerp(Player.angleX)),-sinLerp(Player.angleX)};
	vect3D testLine[2];//, testPoly[4];
	int i, j, k;
	// i=testCursorI-(map.size.x)/2;
	// j=testCursorJ-(map.size.y)/2;
	// k=testCursorK-(map.size.z)/2;
	// testPoly[0].x=(rTilesize2*i-rTilesize);testPoly[0].y=(rTilesize2*j-rTilesize);testPoly[0].z=(rTilesize2*k+rTilesize);
	// testPoly[1].x=(rTilesize2*i+rTilesize);testPoly[1].y=(rTilesize2*j-rTilesize);testPoly[1].z=(rTilesize2*k+rTilesize);
	// testPoly[2].x=(rTilesize2*i+rTilesize);testPoly[2].y=(rTilesize2*j+rTilesize);testPoly[2].z=(rTilesize2*k+rTilesize);
	// testPoly[3].x=(rTilesize2*i-rTilesize);testPoly[3].y=(rTilesize2*j+rTilesize);testPoly[3].z=(rTilesize2*k+rTilesize);
	
	testLine[0]=p->position;
	testLine[1]=p->position;
	testLine[1].x+=lineOfSight.x*32;testLine[1].y+=lineOfSight.y*32;testLine[1].z+=lineOfSight.z*32;
	
		// glBegin(GL_QUAD);
			// glColor3b(255,0,0);
			// glVertex3v16(testPoly[0].x,testPoly[0].y,testPoly[0].z);
			// glVertex3v16(testPoly[1].x,testPoly[1].y,testPoly[1].z);
			// glVertex3v16(testPoly[2].x,testPoly[2].y,testPoly[2].z);
			// glVertex3v16(testPoly[3].x,testPoly[3].y,testPoly[3].z);
		// glEnd();
		// glBegin(GL_TRIANGLE);
			// glColor3b(0,255,0);
			// glVertex3v16(testLine[0].x,testLine[0].y,testLine[0].z);
			// glVertex3v16(testLine[1].x,testLine[1].y,testLine[1].z);
			// glVertex3v16(testLine[1].x,testLine[1].y,testLine[1].z);
		// glEnd();
	
	// bool test=IntersectedPolygon(testPoly, testLine, 4);
	map_struct* m=&map;
	quad_struct* q=NULL;
	/*for(i=0;i<10 && i<closedList.size && !q;i++)
	{
		listElement_struct* le=&closedList.elements[i];
		q=clusterCheck(m, &m->cluster[qgetCluster(m,le->i,le->j,le->k)], testLine);
		if(q)
		{
			testCursor=q->blockID;
			cursorDir=q->direction;
		}
	}*/
	int pi, pj, pk;
	pi=Player.position.x/(rTilesize2);//+(m->size.x)/2;
	pj=Player.position.y/(rTilesize2);//+(m->size.y)/2;
	pk=Player.position.z/(rTilesize2);//+(m->size.z)/2;
	vect3D r;
	int32 sqrTilesize=2*mulf32(rTilesize,rTilesize);
	int32 mindist=inttof32(7);
	int32 dist;
	bool got=false;
	PROF_START(); //PROBLEEEEEEMES (superclustersize etc.)
	for(i=pi-3;i<=pi+3;i++)
	{
		for(j=pj-3;j<=pj+3;j++)
		{
			for(k=pk-3;k<=pk+3;k++)
			{
				if(tangible(*getBlockP(m, i+SUPERCLUSTERSIZE*CLUSTERSIZE/2+m->offset.x*CLUSTERSIZE, j+SUPERCLUSTERSIZE*CLUSTERSIZE/2+m->offset.y*CLUSTERSIZE, k+m->size.z/2+m->offset.z*CLUSTERSIZE)))
				{
					vect3D center=(vect3D){(i)*rTilesize2,(j)*rTilesize2,(k)*rTilesize2};
					// vect3D center=(vect3D){(((i)-m->offset.x*CLUSTERSIZE)*(rTilesize2))-(m->size.x*rTilesize2)/2-(tilesize<<6)*SCALEFACTOR,
					// (((j)-m->offset.y*CLUSTERSIZE)*(rTilesize2))-(m->size.y*rTilesize2)/2-(tilesize<<6)*SCALEFACTOR,
					// (((k)-m->offset.z*CLUSTERSIZE)*(rTilesize2))-(m->size.z*rTilesize2)/2-(tilesize<<6)*SCALEFACTOR};
					r=ClosestPointOnLine(p->position, lineOfSight, 10, &dist, center);
					if(sqDistance(r, center)<=sqrTilesize && (dist<mindist || !got))
					{
						// NOGBA("col : %d %d %d",i+m->size.x/2,j+m->size.y/2,k+m->size.z/2);
						mindist=dist;
						testCursorI=i+SUPERCLUSTERSIZE*CLUSTERSIZE/2+m->offset.x*CLUSTERSIZE;
						testCursorJ=j+SUPERCLUSTERSIZE*CLUSTERSIZE/2+m->offset.y*CLUSTERSIZE;
						testCursorK=k+m->size.z/2+m->offset.z*CLUSTERSIZE;
						testCursor=testCursorI+testCursorJ*m->size.x+testCursorK*m->size.x*m->size.y;
						// drawTestBlock(m, testCursorI, testCursorJ, testCursorK);
						got=true;
					}
				}
			}
		}
	}
	#ifdef DEBUGMODE
	iprintf("\nplayer : %d %d %d  ",pi,pj,pk);
	iprintf("\ncursor2 : %d %d %d %d  ",testCursorI,testCursorJ,testCursorK,testCursor);
	#endif
	if(got)
	{
		// vect3D testCluster=getCluster(m, testCursorI, testCursorJ, testCursorK);
		// q=clusterCheck(m, &m->cluster[qgetCluster(m,testCluster.x,testCluster.y,testCluster.z)], testLine, testCursorI, testCursorJ, testCursorK);
		cursorDir=clusterCheck(m, testLine, testCursorI, testCursorJ, testCursorK);
		#ifdef DEBUGMODE
		iprintf("\ncursor : %d   ",cursorDir);
		#endif
		if(cursorDir<0)cursorDir=0;
	}
		int time;
		PROF_END(time);
	// printf("\nTEST : %d, %d   ", q!=NULL, time);
	
	if(cull)if(!m->transitioning[2] && !m->transitioning[3] && !m->transitioning[1] && !m->transitioning[0])
	{
		bool managed=false;
		// if(((m->offset.y && m->offset.y < m->clusterSize.y-SUPERCLUSTERSIZE-1) && abs(p->position.x)>abs(p->position.y)) || (!m->offset.y || m->offset.y >= m->clusterSize.y-SUPERCLUSTERSIZE-1))
		if((!m->offset.y || m->offset.y>=m->clusterSize.y-SUPERCLUSTERSIZE-1) || (abs(p->position.x)>abs(p->position.y)))
		{
			if(p->position.x>bsize*SCALEFACTOR && m->offset.x < m->clusterSize.x-SUPERCLUSTERSIZE) //streaming
			{
				p->position.x-=bsize*SCALEFACTOR;
				translateSuperCluster(&map, 1);
				managed=true;
			}else if(p->position.x<-bsize*SCALEFACTOR && m->offset.x > 0) //streaming
			{
				p->position.x+=bsize*SCALEFACTOR;
				translateSuperCluster(&map, 0);
				managed=true;
			}
		}
		// if(!managed && (((m->offset.x && m->offset.x < m->clusterSize.x-SUPERCLUSTERSIZE-1) && abs(p->position.x)<=abs(p->position.y)) || (!m->offset.y || m->offset.x >= m->clusterSize.x-SUPERCLUSTERSIZE-1)))
		if(!managed && ((!m->offset.x || m->offset.x>=m->clusterSize.x-SUPERCLUSTERSIZE-1) || (abs(p->position.x)<=abs(p->position.y))))
		{
			if(p->position.y>bsize*SCALEFACTOR && m->offset.y < m->clusterSize.y-SUPERCLUSTERSIZE) //streaming
			{
				p->position.y-=bsize*SCALEFACTOR;
				translateSuperCluster(&map, 3);
			}else if(p->position.y<-bsize*SCALEFACTOR && m->offset.y > 0) //streaming
			{
				p->position.y+=bsize*SCALEFACTOR;
				translateSuperCluster(&map, 2);
			}
		}
	}
	
	// if(!noclip)
	{
		// iprintf("\ngravity div : %d   ",gravityDiv);
		if(!noclip)p->vector.z-=GRAVITY*gravityDiv;
		PROF_START();
		u8 t=getPointBlock(m, p->position.x, p->position.y, p->position.z);
		vect3D po=getPointBlockPos(m, p->position.x, p->position.y, p->position.z);
		p->clusterCoord=getCluster(m, po.x, po.y, po.z);
		p->clusterCoord.x-=m->offset.x;
		p->clusterCoord.y-=m->offset.y;
		p->inCave=0;
		surface2(m, po.x, po.y, po.z, &p->inCave);
		u8 t_2=getPointBlock(m, p->position.x, p->position.y, p->position.z-4500);
		if(t>=WATERTYPE || t_2>=WATERTYPE)
		{
			p->vector.x*=8;
			p->vector.y*=8;
			p->vector.z*=8;
			p->vector.x/=10;
			p->vector.y/=10;
			p->vector.z/=10;
			if(t>=WATERTYPE)p->inWater=2;
			else p->inWater=1;
			p->onLadder=false;
			p->inCave=1<<7;
			gravityDiv=0;
			if(fogMode && p->inWater==2)setFog(0);
		}else if(isLadder(t) || isLadder(t_2))
		{
			p->onLadder=true;
			p->inWater=false;
		}else{
			p->inWater=false;
			p->onLadder=false;
			gravityDiv=1;
		}
		if(p->inWater!=2 && !fogMode && p->inCave)setFog(1);
		// vect3D testPos=p->position;
		// testPos.z-=4000;
		// testPoint(&map, testPos, &p->vector); // works but simple
		vect3D testPos=(vect3D){p->position.x,p->position.y,p->position.z-6000};
		testPlane(&map, testPos, &p->vector);
		testPos=(vect3D){p->position.x,p->position.y,p->position.z-2000};
		testPlane(&map, testPos, &p->vector);
		testPos=(vect3D){p->position.x,p->position.y,p->position.z+2000};
		testPlane(&map, testPos, &p->vector);
		// int time;
		PROF_END(time);
		#ifdef DEBUGMODE
		iprintf("\ncollisions : %d    ",time);
		#endif
		p->position.x+=p->vector.x;p->position.y+=p->vector.y;p->position.z+=p->vector.z;
		p->vector.x=0;
		p->vector.y=0;
		if(noclip || p->onLadder)p->vector.z=0;
	}//else {p->position.x+=p->vector.x;p->position.y+=p->vector.y;p->position.z+=p->vector.z;p->vector=(vect3D){0,0,0};}
}

void playerCamera(player_struct* p, bool environment)
{
	glRotateXi(p->angleX);
	glRotateZi(p->angleZ);
	if(p->angleZ<0)p->angleZ+=32768;
	glTranslatef32(0,0,-p->position.z+(cosLerp(walkAngle)>>4));
	if(!testBuffer && environment)
	{
		drawStars();
		drawSun();
		drawMoon();
		drawDawn();
		drawCloud();
	}
	glTranslatef32(-p->position.x,-p->position.y,0);
}

vect3D getPointBlockPos(map_struct* m, int32 i, int32 j, int32 k)
{
	return (vect3D){(i+(tilesize<<6)*SCALEFACTOR+(SUPERCLUSTERSIZE*CLUSTERSIZE*rTilesize2)/2)/(rTilesize2)+m->offset.x*CLUSTERSIZE,
	(j+(tilesize<<6)*SCALEFACTOR+(SUPERCLUSTERSIZE*CLUSTERSIZE*rTilesize2)/2)/(rTilesize2)+m->offset.y*CLUSTERSIZE,
	(k+(tilesize<<6)*SCALEFACTOR+(m->size.z*rTilesize2)/2)/(rTilesize2)+m->offset.z*CLUSTERSIZE};
}

u8 getPointBlock(map_struct* m, int32 i, int32 j, int32 k)
{
	i=(i+(tilesize<<6)*SCALEFACTOR+(SUPERCLUSTERSIZE*CLUSTERSIZE*rTilesize2)/2)/(rTilesize2)+m->offset.x*CLUSTERSIZE;
	j=(j+(tilesize<<6)*SCALEFACTOR+(SUPERCLUSTERSIZE*CLUSTERSIZE*rTilesize2)/2)/(rTilesize2)+m->offset.y*CLUSTERSIZE;
	k=(k+(tilesize<<6)*SCALEFACTOR+(m->size.z*rTilesize2)/2)/(rTilesize2)+m->offset.z*CLUSTERSIZE;
	// NOGBA("pos : %d",k);
	// iprintf("\nblock %d %d %d (%d)  ",i,j,k,(*getBlockP(m,i,j,k)));
	if(i<0 || j<0 || k<0 || i>=m->size.x || j>=m->size.y || k>=m->size.z)return 1;
	return (*getBlockP(m,i,j,k));
	// i-=rTilesize;
	// j-=rTilesize;
	// k-=rTilesize;
	// return *getBlockP(m,(i-i%rTilesize2+(m->size.x*rTilesize2)/2)/rTilesize2,(j-j%rTilesize2+(m->size.y*rTilesize2)/2)/rTilesize2,(k-k%rTilesize2+(m->size.z*rTilesize2)/2)/rTilesize2);
}

void testPoint(map_struct* m, vect3D point, vect3D* vector)
{	
	if(!vector->x && !vector->y && !vector->z)return;
	if(vector->x>rTilesize2)vector->x=rTilesize2-1;
	else if(vector->x<-rTilesize2)vector->x=-rTilesize2+1;
	if(vector->y>rTilesize2)vector->y=rTilesize2-1;
	else if(vector->y<-rTilesize2)vector->y=-rTilesize2+1;
	if(vector->z>rTilesize2)vector->z=rTilesize2-1;
	else if(vector->z<-rTilesize2)vector->z=-rTilesize2+1;
	
	if(solid(getPointBlock(m, point.x, point.y, point.z+vector->z)))vector->z=(vector->z<0)?(-(((point.z-rTilesize+map.size.z*rTilesize2))%rTilesize2)+1):((rTilesize2-(((point.z-rTilesize+map.size.z*rTilesize2))%rTilesize2))-1);
	if(solid(getPointBlock(m, point.x, point.y+vector->y, point.z+vector->z)))vector->y=(vector->y<0)?(-(((point.y-rTilesize+map.size.y*rTilesize2))%rTilesize2)+1):((rTilesize2-(((point.y-rTilesize+map.size.y*rTilesize2))%rTilesize2))-1);
	if(solid(getPointBlock(m, point.x+vector->x, point.y+vector->y, point.z+vector->z)))vector->x=(vector->x<0)?(-(((point.x-rTilesize+map.size.x*rTilesize2))%rTilesize2)+1):((rTilesize2-(((point.x-rTilesize+map.size.x*rTilesize2))%rTilesize2))-1);
}

void testPlane(map_struct* m, vect3D point, vect3D* vector)
{	
	if(!vector->x && !vector->y && !vector->z)return;
	if(vector->x>rTilesize2)vector->x=rTilesize2-1;
	else if(vector->x<-rTilesize2)vector->x=-rTilesize2+1;
	if(vector->y>rTilesize2)vector->y=rTilesize2-1;
	else if(vector->y<-rTilesize2)vector->y=-rTilesize2+1;
	if(vector->z>rTilesize2)vector->z=rTilesize2-1;
	else if(vector->z<-rTilesize2)vector->z=-rTilesize2+1;
	
	vect3D point2=(vect3D){point.x+BBSIZE,point.y+BBSIZE,point.z};
	if(solid(getPointBlock(m, point2.x, point2.y, point2.z+vector->z)))vector->z=(vector->z<0)?(-(((point2.z-rTilesize+map.size.z*rTilesize2))%rTilesize2)+1):((rTilesize2-(((point2.z-rTilesize+map.size.z*rTilesize2))%rTilesize2))-1);
	point2=(vect3D){point.x-BBSIZE,point.y+BBSIZE,point.z};
	if(solid(getPointBlock(m, point2.x, point2.y, point2.z+vector->z)))vector->z=(vector->z<0)?(-(((point2.z-rTilesize+map.size.z*rTilesize2))%rTilesize2)+1):((rTilesize2-(((point2.z-rTilesize+map.size.z*rTilesize2))%rTilesize2))-1);
	point2=(vect3D){point.x-BBSIZE,point.y-BBSIZE,point.z};
	if(solid(getPointBlock(m, point2.x, point2.y, point2.z+vector->z)))vector->z=(vector->z<0)?(-(((point2.z-rTilesize+map.size.z*rTilesize2))%rTilesize2)+1):((rTilesize2-(((point2.z-rTilesize+map.size.z*rTilesize2))%rTilesize2))-1);
	point2=(vect3D){point.x+BBSIZE,point.y-BBSIZE,point.z};
	if(solid(getPointBlock(m, point2.x, point2.y, point2.z+vector->z)))vector->z=(vector->z<0)?(-(((point2.z-rTilesize+map.size.z*rTilesize2))%rTilesize2)+1):((rTilesize2-(((point2.z-rTilesize+map.size.z*rTilesize2))%rTilesize2))-1);
	
	
	point2=(vect3D){point.x+BBSIZE,point.y+BBSIZE,point.z};
	if(solid(getPointBlock(m, point2.x, point2.y+vector->y, point2.z+vector->z)))vector->y=(vector->y<0)?(-(((point2.y-rTilesize+map.size.y*rTilesize2))%rTilesize2)+1):((rTilesize2-(((point2.y-rTilesize+map.size.y*rTilesize2))%rTilesize2))-1);
	point2=(vect3D){point.x-BBSIZE,point.y+BBSIZE,point.z};
	if(solid(getPointBlock(m, point2.x, point2.y+vector->y, point2.z+vector->z)))vector->y=(vector->y<0)?(-(((point2.y-rTilesize+map.size.y*rTilesize2))%rTilesize2)+1):((rTilesize2-(((point2.y-rTilesize+map.size.y*rTilesize2))%rTilesize2))-1);
	point2=(vect3D){point.x-BBSIZE,point.y-BBSIZE,point.z};
	if(solid(getPointBlock(m, point2.x, point2.y+vector->y, point2.z+vector->z)))vector->y=(vector->y<0)?(-(((point2.y-rTilesize+map.size.y*rTilesize2))%rTilesize2)+1):((rTilesize2-(((point2.y-rTilesize+map.size.y*rTilesize2))%rTilesize2))-1);
	point2=(vect3D){point.x+BBSIZE,point.y-BBSIZE,point.z};
	if(solid(getPointBlock(m, point2.x, point2.y+vector->y, point2.z+vector->z)))vector->y=(vector->y<0)?(-(((point2.y-rTilesize+map.size.y*rTilesize2))%rTilesize2)+1):((rTilesize2-(((point2.y-rTilesize+map.size.y*rTilesize2))%rTilesize2))-1);
	
	point2=(vect3D){point.x+BBSIZE,point.y+BBSIZE,point.z};
	if(solid(getPointBlock(m, point2.x+vector->x, point2.y+vector->y, point2.z+vector->z)))vector->x=(vector->x<0)?(-(((point2.x-rTilesize+map.size.x*rTilesize2))%rTilesize2)+1):((rTilesize2-(((point2.x-rTilesize+map.size.x*rTilesize2))%rTilesize2))-1);
	point2=(vect3D){point.x-BBSIZE,point.y+BBSIZE,point.z};
	if(solid(getPointBlock(m, point2.x+vector->x, point2.y+vector->y, point2.z+vector->z)))vector->x=(vector->x<0)?(-(((point2.x-rTilesize+map.size.x*rTilesize2))%rTilesize2)+1):((rTilesize2-(((point2.x-rTilesize+map.size.x*rTilesize2))%rTilesize2))-1);
	point2=(vect3D){point.x-BBSIZE,point.y-BBSIZE,point.z};
	if(solid(getPointBlock(m, point2.x+vector->x, point2.y+vector->y, point2.z+vector->z)))vector->x=(vector->x<0)?(-(((point2.x-rTilesize+map.size.x*rTilesize2))%rTilesize2)+1):((rTilesize2-(((point2.x-rTilesize+map.size.x*rTilesize2))%rTilesize2))-1);
	point2=(vect3D){point.x+BBSIZE,point.y-BBSIZE,point.z};
	if(solid(getPointBlock(m, point2.x+vector->x, point2.y+vector->y, point2.z+vector->z)))vector->x=(vector->x<0)?(-(((point2.x-rTilesize+map.size.x*rTilesize2))%rTilesize2)+1):((rTilesize2-(((point2.x-rTilesize+map.size.x*rTilesize2))%rTilesize2))-1);
}

vect3D Cross(vect3D vVector1, vect3D vVector2)
{
	vect3D vNormal;
	
	vNormal.x = (mulf32(vVector1.y, vVector2.z) - mulf32(vVector1.z, vVector2.y));
														
	// The Y value for the vector is:  (V1.z * V2.x) - (V1.x * V2.z)
	vNormal.y = (mulf32(vVector1.z, vVector2.x) - mulf32(vVector1.x, vVector2.z));
														
	// The Z value for the vector is:  (V1.x * V2.y) - (V1.y * V2.x)
	vNormal.z = (mulf32(vVector1.x, vVector2.y) - mulf32(vVector1.y, vVector2.x));

	return vNormal;										// Return the cross product (Direction the polygon is facing - Normal)
}

static inline vect3D Vector(vect3D vPoint1, vect3D vPoint2)
{
	vect3D vVector;

	vVector.x = vPoint1.x - vPoint2.x;
	vVector.y = vPoint1.y - vPoint2.y;
	vVector.z = vPoint1.z - vPoint2.z;

	return vVector;
}

int32 Magnitude(vect3D vNormal)
{
	return sqrtf32( mulf32(vNormal.x, vNormal.x) + mulf32(vNormal.y, vNormal.y) + mulf32(vNormal.z, vNormal.z) );
}

int32 sqMagnitude(vect3D vNormal)
{
	return ( mulf32(vNormal.x, vNormal.x) + mulf32(vNormal.y, vNormal.y) + mulf32(vNormal.z, vNormal.z) );
}

int32 sqDistance(vect3D vA, vect3D vB)
{
	return sqMagnitude(Vector(vA,vB));
}

vect3D Normalize(vect3D vNormal)
{
	int32 magnitude = divf32(inttof32(1),Magnitude(vNormal));
	
	vNormal.x = mulf32(vNormal.x,magnitude);
	vNormal.y = mulf32(vNormal.y,magnitude);
	vNormal.z = mulf32(vNormal.z,magnitude);

	return vNormal;
}

vect3D Normal(vect3D* vTriangle)					
{														// Get 2 vectors from the polygon (2 sides), Remember the order!
	vect3D vVector1 = Vector(vTriangle[2], vTriangle[0]);
	vect3D vVector2 = Vector(vTriangle[1], vTriangle[0]);

	vect3D vNormal = Cross(vVector1, vVector2);	

	vNormal = Normalize(vNormal);

	return vNormal;
}

int32 PlaneDistance(vect3D Normal, vect3D Point)
{	
	int32 distance = 0;
	distance = - (mulf32(Normal.x, Point.x) + mulf32(Normal.y, Point.y) + mulf32(Normal.z, Point.z));
	return distance;
}

bool IntersectedPlane(vect3D* vPoly, vect3D* vLine, vect3D* vNormal, int32* originDistance)
{
	int32 distance1=0, distance2=0;
	
	*vNormal = Normal(vPoly);
	
	*originDistance = PlaneDistance(*vNormal, vPoly[0]);
	// NOGBA("origindist : %d",*originDistance);

	distance1 = (mulf32(vNormal->x, vLine[0].x)  +					// Ax +
		         mulf32(vNormal->y, vLine[0].y)  +					// Bx +
				 mulf32(vNormal->z, vLine[0].z)) + *originDistance;	// Cz + D
	
	// Get the distance from point2 from the plane using Ax + By + Cz + D = (The distance from the plane)
	
	distance2 = (mulf32(vNormal->x, vLine[1].x)  +					// Ax +
		         mulf32(vNormal->y, vLine[1].y)  +					// Bx +
				 mulf32(vNormal->z, vLine[1].z)) + *originDistance;	// Cz + D

	if(mulf32(distance1, distance2) >= 0)			// Check to see if both point's distances are both negative or both positive
	   return false;						// Return false if each point has the same sign.  -1 and 1 would mean each point is on either side of the plane.  -1 -2 or 3 4 wouldn't...
					
	return true;							// The line intersected the plane, Return TRUE
}


/////////////////////////////////// DOT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This computers the dot product of 2 vectors
/////
/////////////////////////////////// DOT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

int32 Dot(vect3D vVector1, vect3D vVector2) 
{
	return ( mulf32(vVector1.x, vVector2.x) + mulf32(vVector1.y, vVector2.y) + mulf32(vVector1.z, vVector2.z) );
}

int32 AngleBetweenVectors(vect3D Vector1, vect3D Vector2)
{
	int32 dotProduct = Dot(Vector1, Vector2);				

	int32 vectorsMagnitude = mulf32(Magnitude(Vector1), Magnitude(Vector2));

	// Get the arc cosine of the (dotProduct / vectorsMagnitude) which is the angle in RADIANS.
	// (IE.   PI/2 radians = 90 degrees      PI radians = 180 degrees    2*PI radians = 360 degrees)
	// To convert radians to degress use this equation:   radians * (PI / 180)
	// TO convert degrees to radians use this equation:   degrees * (180 / PI)
	int32 angle = /*angleToDegrees*/(acosLerp(divf32(dotProduct, vectorsMagnitude)));

	/*// Here we make sure that the angle is not a -1.#IND0000000 number, which means indefinate.
	// acos() thinks it's funny when it returns -1.#IND0000000.  If we don't do this check,
	// our collision results will sometimes say we are colliding when we aren't.  I found this
	// out the hard way after MANY hours and already wrong written tutorials :)  Usually
	// this value is found when the dot product and the maginitude are the same value.
	// We want to return 0 when this happens.
	if(_isnan(angle))
		return 0;*/
	
	// Return the angle in radians
	return( angle );
}
											
vect3D IntersectionPoint(vect3D vNormal, vect3D* vLine, int32 distance, int32 *realDist)
{
	vect3D vPoint, vLineDir;		// Variables to hold the point and the line's direction
	int32 Numerator, Denominator, dist;

	// Here comes the confusing part.  We need to find the 3D point that is actually
	// on the plane.  Here are some steps to do that:
	
	// 1)  First we need to get the vector of our line, Then normalize it so it's a length of 1
	vLineDir = Vector(vLine[1], vLine[0]);		// Get the Vector of the line
	vLineDir = Normalize(vLineDir);				// Normalize the lines vector


	// 2) Use the plane equation (distance = Ax + By + Cz + D) to find the distance from one of our points to the plane.
	//    Here I just chose a arbitrary point as the point to find that distance.  You notice we negate that
	//    distance.  We negate the distance because we want to eventually go BACKWARDS from our point to the plane.
	//    By doing this is will basically bring us back to the plane to find our intersection point.
	Numerator = - (mulf32(vNormal.x, vLine[0].x) +		// Use the plane equation with the normal and the line
				   mulf32(vNormal.y, vLine[0].y) +
				   mulf32(vNormal.z, vLine[0].z) + distance);

	// 3) If we take the dot product between our line vector and the normal of the polygon,
	//    this will give us the cosine of the angle between the 2 (since they are both normalized - length 1).
	//    We will then divide our Numerator by this value to find the offset towards the plane from our arbitrary point.
	Denominator = Dot(vNormal, vLineDir);		// Get the dot product of the line's vector and the normal of the plane
				  
	// Since we are using division, we need to make sure we don't get a divide by zero error
	// If we do get a 0, that means that there are INFINATE points because the the line is
	// on the plane (the normal is perpendicular to the line - (Normal.Vector = 0)).  
	// In this case, we should just return any point on the line.

	if( Denominator == 0)						// Check so we don't divide by zero
		return vLine[0];						// Return an arbitrary point on the line

	// We divide the (distance from the point to the plane) by (the dot product)
	// to get the distance (dist) that we need to move from our arbitrary point.  We need
	// to then times this distance (dist) by our line's vector (direction).  When you times
	// a scalar (single number) by a vector you move along that vector.  That is what we are
	// doing.  We are moving from our arbitrary point we chose from the line BACK to the plane
	// along the lines vector.  It seems logical to just get the numerator, which is the distance
	// from the point to the line, and then just move back that much along the line's vector.
	// Well, the distance from the plane means the SHORTEST distance.  What about in the case that
	// the line is almost parallel with the polygon, but doesn't actually intersect it until half
	// way down the line's length.  The distance from the plane is short, but the distance from
	// the actual intersection point is pretty long.  If we divide the distance by the dot product
	// of our line vector and the normal of the plane, we get the correct length.  Cool huh?

	dist = divf32(Numerator,Denominator);				// Divide to get the multiplying (percentage) factor
	
	// Now, like we said above, we times the dist by the vector, then add our arbitrary point.
	// This essentially moves the point along the vector to a certain distance.  This now gives
	// us the intersection point.  Yay!

	vPoint.x = (int32)(vLine[0].x + mulf32(vLineDir.x, dist));
	vPoint.y = (int32)(vLine[0].y + mulf32(vLineDir.y, dist));
	vPoint.z = (int32)(vLine[0].z + mulf32(vLineDir.z, dist));
	*realDist=dist;

	return vPoint;								// Return the intersection point
}


/////////////////////////////////// INSIDE POLYGON \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This checks to see if a point is inside the ranges of a polygon
/////
/////////////////////////////////// INSIDE POLYGON \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

bool InsidePolygon(vect3D vIntersection, vect3D* Poly, long verticeCount)
{
	// const double MATCH_FACTOR = 0.9999;		// Used to cover up the error in floating point
	int32 Angle=0;						// Initialize the angle
	vect3D vA, vB;						// Create temp vectors
	
	int i;
	for (i = 0; i < verticeCount; i++)		// Go in a circle to each vertex and get the angle between
	{	
		vA = Vector(Poly[i], vIntersection);	// Subtract the intersection point from the current vertex
												// Subtract the point from the next vertex
		vB = Vector(Poly[(i + 1) % verticeCount], vIntersection);
												
		Angle += AngleBetweenVectors(vA, vB);	// Find the angle between the 2 vectors and add them all up as we go along
	}
	
	if(Angle >= (32760))	// If the angle is greater than 2 PI, (360 degrees)
		return TRUE;							// The point is inside of the polygon
		
	return FALSE;								// If you get here, it obviously wasn't inside the polygon, so Return FALSE
}


/////////////////////////////////// INTERSECTED POLYGON \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This checks if a line is intersecting a polygon
/////
/////////////////////////////////// INTERSECTED POLYGON \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

bool IntersectedPolygon(vect3D* vPoly, vect3D* vLine, int verticeCount, int32* originDistance, int32 *realDist)
{
	vect3D vNormal;

	// First we check to see if our line intersected the plane.  If this isn't true
	// there is no need to go on, so return false immediately.
	// We pass in address of vNormal and originDistance so we only calculate it once

									 // Reference   // Reference
	if(!IntersectedPlane(vPoly, vLine,   &vNormal,   originDistance))
		return false;
	// return true;

	// Now that we have our normal and distance passed back from IntersectedPlane(), 
	// we can use it to calculate the intersection point.  The intersection point
	// is the point that actually is ON the plane.  It is between the line.  We need
	// this point test next, if we are inside the polygon.  To get the I-Point, we
	// give our function the normal of the plan, the points of the line, and the originDistance.

	vect3D vIntersection = IntersectionPoint(vNormal, vLine, *originDistance, realDist);

	// Now that we have the intersection point, we need to test if it's inside the polygon.
	// To do this, we pass in :
	// (our intersection point, the polygon, and the number of vertices our polygon has)

	if(InsidePolygon(vIntersection, vPoly, verticeCount))return true;


	// If we get here, we must have NOT collided

	return false;								// There was no collision, so return false
}

vect3D ClosestPointOnLine(vect3D vA, vect3D vD, int length, int32* dist, vect3D vPoint)
{
	// This function takes a line segment, vA to vB, then a point, vPoint.
	// We want to find the closet point on the line segment to our vPoint
	// out in space.  Either it is going to be one of the end points of the line,
	// or it is going to be somewhere between vA and vB.  This is a important
	// function when dealing with collision detection.

	// Here is how it works, it's a bit confusing at first, so you will need
	// to contemplate it a bit.  First, we want to grab a vector from "vA" to the point.
	// Then we want to get a normalized vector from "vA" to "vB".  We don't need the
	// full length of the line segment vector, we just want a direction.  That is why 
	// we normalize it.  Remember, this is important because we are going to be using 
	// the dot product coming up next.  So, now we have 2 vectors that form a pseudo corner
	// of a triangle on the plane of the line segment and the point.
	//
	// Next, we want to find the distance or "magnitude" of the line segment.  This is
	// done with a simple distance formula.  Then we use the dot "vVector2" with "vVector1".
	// By using the dot product, we can essentially project vVector1 onto the
	// line segments normalized vector, "vVector2".  If the result of the dot product is
	// 0, that means the vectors were perpendicular and had a 90 degree angle between them.
	// The 0 part is the distance the new projected vector is from the starting of vVector2.
	// If the result is a negative number, that means the angle between the 2 vectors
	// is greater than 90 degrees, which means that the closest point must be "vA" because
	// it's projected vector is on the outside of the line.  So, if the result is a positive
	// number, the projected vector is on the right side of "vA", but could be past the right
	// side of vB.  To test this, we make sure that the result of the dot product is NOT
	// greater than the distance "d".  If it is, then the closest point on the plane is
	// obviously vB.  
	//
	// So, we can find the closest point easily if it's one of the end points of the line
	// segment, but how do we find the point between the 2 end points?  This is simple.
	// Since we have the distance "t" from point "vA" (given to us from the dot product 
	// of the 2 vectors), we just use our vector that is going the direction of the
	// line segment, "vVector2", and multiply it by the distance scalar "t".  This will
	// create a vector going in the direction of the line segment, with a distance
	// (or magnitude) of the projected vector, "vVector1", is from from "vA".  We then add
	// this vector to "vA", which gives us the point on the line that is closest to our
	// point out in space, vPoint!  
	//
	// This is probably pretty hard to visualize with just comments, unless you have a good 
	// grasp of linear algebra.  
	
	// Create the vector from end point vA to our point vPoint.
	vect3D vVector1 = vPoint;
	vVector1.x-=vA.x;
	vVector1.y-=vA.y;
	vVector1.z-=vA.z;

	// Create a normalized direction vector from end point vA to end point vB
    vect3D vVector2 = vD; //vD direction du segment, normé

	// Use the distance formula to find the distance of the line segment (or magnitude)
    int32 d = inttof32(length);

	// Using the dot product, we project the vVector1 onto the vector vVector2.
	// This essentially gives us the distance from our projected vector from vA.
    int32 t = Dot(vVector2, vVector1);
	*dist=t;

	// If our projected distance from vA, "t", is less than or equal to 0, it must
	// be closest to the end point vA.  We want to return this end point.
    if (t <= 0) 
		return vA;

	// If our projected distance from vA, "t", is greater than or equal to the magnitude
	// or distance of the line segment, it must be closest to the end point vB.  So, return vB.
    if (t >= d) 
		return (vect3D){vA.x+vD.x*d,vA.y+vD.y*d,vA.z+vD.z*d};
 
	// Here we create a vector that is of length t and in the direction of vVector2
    vect3D vVector3 = vVector2;
	vVector3.x=mulf32(t,vVector3.x);
	vVector3.y=mulf32(t,vVector3.y);
	vVector3.z=mulf32(t,vVector3.z);

	// To find the closest point on the line segment, we just add vVector3 to the original
	// end point vA.  
    vect3D vClosestPoint = vA;
	vClosestPoint.x+=vVector3.x;
	vClosestPoint.y+=vVector3.y;
	vClosestPoint.z+=vVector3.z;

	// Return the closest point on the line segment
	return vClosestPoint;
}
