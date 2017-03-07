#include "game/game_main.h"

#define waterm(a) (((a)>=WATERTYPE)?(a):(300))
#define CLUSTER_FIRST	0x00000002

bool testvar=false;
// int cullMagic;
u16 cullMagic;
u8 fsFormat;


/*inline u8 getBlock(map_struct* m, int i, int j, int k)
{
	if(i<0 || j<0 || k<0 || i>=m->size.x || j>=m->size.y || k>=m->size.z)return 0;
	return qgetBlock(m,i,j,k);
}*/

void initFilesystem(void)
{
	switch(fsMode)
	{
		case 1:
			openMap=&openMapNOCASH;
			readClusterColumn=&readClusterColumnNOCASH;
			writeClusterColumn=&writeClusterColumnNOCASH;
			fsFormat=0;
			break;
		default:
			openMap=&openMap2048;
			readClusterColumn=&readClusterColumn2048;
			writeClusterColumn=&writeClusterColumn2048;
			fsFormat=1;
			break;
	}
}

void initLightMap(void)
{
	int i;
	for(i=0;i<256*14;i++)lightMap[i]=RGB15(31,31,31);
	#ifdef FOGLIGHT
		for(i=0;i<256*14;i++)lightMap2[i]=RGB15(21,21,21);
	#endif
}

void addWaterFace(map_struct* m, int i, int j, int k, u8 f)
{
	vect3D clusterCoord=getCluster(m,i,j,k);
	quadList_struct* ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].specialList;
	u8* t=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->highest[(i%CLUSTERSIZE)+(j%CLUSTERSIZE)*CLUSTERSIZE];
	if(k>(*t))(*t)=k;
	u8 light=0;
	surface(m, i, j, k, &light);
	getLight(m, i, j, k, &light, f);
	addQuad(ql, m, f, light, 0, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);
	// m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->changed=true;
}

void removeWaterFace(map_struct* m, int i, int j, int k, u8 face)
{
	vect3D clusterCoord=getCluster(m,i,j,k);
	quadList_struct* ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].specialList;
	u8 type=*getBlockP(m,i,j,k);
	quad_struct* oq=ql->first;
	quad_struct* q;
	if(oq)q=oq->next;
	else q=NULL;
	int i1=i,j1=j,k1=k;
	i%=CLUSTERSIZE;
	j%=CLUSTERSIZE;
	k%=CLUSTERSIZE;
	u8 mID=i+j*CLUSTERSIZE+k*CLUSTERSIZE*CLUSTERSIZE;
	while(q)
	{
		// if(q->i==i && q->j==j && q->k==k)
		if(q->mID==mID && q->direction==face)
		{
			oq->next=q->next;
			// if(!oq->next)ql->last=oq;
			// free(q);
			releaseQuad(&q);
			ql->count--;
			q=oq->next;
			return;
		}else{
			oq=q;
			q=q->next;
		}
	}
	q=ql->first;
	// if(q && q->i==i && q->j==j && q->k==k)
	if(q && q->mID==mID && q->direction==face)
	{
		ql->first=q->next;
		// if(!ql->first)ql->last=NULL;
		ql->count--;
		// free(q);
		releaseQuad(&q);
	}
	// m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->changed=true;
}

void initWater(void)
{
	waterCount=0;
	waterCursor=0;
	waterCount2=0;
	waterCursor2=0;
}

void addWater2(map_struct* m, water_struct w)
{
	waterSpread[waterCount++]=w;
	if(waterCount>=WATERNUMBER)waterCount-=WATERNUMBER;
}

bool addWater(map_struct* m, u16 i, u16 j, u16 k, u8 t)
{
	if(t-WATERTYPE>=WATERSPREAD)return false;
	water_struct* w;
	if(i-m->offset.x*CLUSTERSIZE >= 120
	|| i-m->offset.x*CLUSTERSIZE < 8
	|| j-m->offset.y*CLUSTERSIZE >= 120
	|| j-m->offset.y*CLUSTERSIZE < 8)w=&waterToSpread[waterCount2++];
	else w=&waterSpread[waterCount++];
	w->pos=(i&8191)|((j&8191)<<13)|((k&63)<<26);
	// w->dir=t;
	if(waterCount>=WATERNUMBER)waterCount-=WATERNUMBER;
	if(waterCount2>=WATERNUMBER2)waterCount2-=WATERNUMBER2;
	return true;
}

// void addWaterCalc(map_struct* m, u16 i, u16 j, u16 k, u8 t)
// {
	// /*const u8 t=(((transparent2(m,i,j,k,i,j,k-1))&1))
	// |((((transparent2(m,i,j,k,i,j,k+1))&1)<<1))
	// |((((transparent2(m,i,j,k,i,j-1,k))&1)<<2))
	// |((((transparent2(m,i,j,k,i,j+1,k))&1)<<3))
	// |((((transparent2(m,i,j,k,i-1,j,k))&1)<<4))
	// |((((transparent2(m,i,j,k,i+1,j,k))&1)<<5));*/
	// const u8 t=0;
	// NOGBA("t ! %d",t);
	// addWater(m, i, j, k, t);	
// }

void processWater(map_struct* m)
{
	if((waterCount-waterCursor)<=0)return;
	water_struct* w=&waterSpread[waterCursor++];
	const u16 i=w->pos&8191, j=(w->pos>>13)&8191, k=(w->pos>>26)&63;
	const u8 type=*getBlockP(m,i,j,k);
	// NOGBA("pos ! %d %d %d",i,j,k);
	if(/*w->dir&1 && */!solid(*getBlockP(m,i,j,k-1)) && type-WATERTYPE<WATERSPREAD)
	{
		/*addWaterFace(m, i, j, k, 1);*/
		u8 d=*getBlockP(m,i,j-1,k);
		if(d==0)addWaterFace(m, i, j, k, 5);
		else if(d>=WATERTYPE)removeWaterFace(m, i, j-1, k, 4);
		d=*getBlockP(m,i,j+1,k);
		if(d==0)addWaterFace(m, i, j, k, 4);
		else if(d>=WATERTYPE)removeWaterFace(m, i, j+1, k, 5);
		d=*getBlockP(m,i-1,j,k);
		if(d==0)addWaterFace(m, i, j, k, 3);
		else if(d>=WATERTYPE)removeWaterFace(m, i-1, j, k, 2);
		d=*getBlockP(m,i+1,j,k);
		if(d==0)addWaterFace(m, i, j, k, 2);
		else if(d>=WATERTYPE)removeWaterFace(m, i+1, j, k, 3);
		d=*getBlockP(m,i,j,k-1);
		if(d>=WATERTYPE)removeWaterFace(m, i, j, k-1, 0);
		d=*getBlockP(m,i,j,k+1);
		if(d>=WATERTYPE)removeWaterFace(m, i, j, k+1, 1);
		*getBlockP(m,i,j,k-1)=type;
		addWater(m, i, j, k-1, type);
	}else if(type-WATERTYPE==WATERSPREAD-1){
		u8 d=*getBlockP(m,i,j-1,k);
		if(!d)addWaterFace(m, i, j, k, 5);
		else if(d>=WATERTYPE)removeWaterFace(m, i, j-1, k, 4);
		d=*getBlockP(m,i,j+1,k);
		if(!d)addWaterFace(m, i, j, k, 4);
		else if(d>=WATERTYPE)removeWaterFace(m, i, j+1, k, 5);
		d=*getBlockP(m,i-1,j,k);
		if(!d)addWaterFace(m, i, j, k, 3);
		else if(d>=WATERTYPE)removeWaterFace(m, i-1, j, k, 2);
		d=*getBlockP(m,i+1,j,k);
		if(!d)addWaterFace(m, i, j, k, 2);
		else if(d>=WATERTYPE)removeWaterFace(m, i+1, j, k, 3);
		d=*getBlockP(m,i,j,k+1);
		if(d>=WATERTYPE)removeWaterFace(m, i, j, k+1, 1);
	}else if(type+1-WATERTYPE<WATERSPREAD){
		u8 d=*getBlockP(m,i,j-1,k);
		if(!d)
		{
			if(*getBlockP(m,i,j-1,k-1)>=WATERTYPE)removeWaterFace(m, i, j-1, k-1, 0);
			if(!*getBlockP(m,i,j-1,k+1))addWaterFace(m, i, j-1, k, 0);
			
			// if(!*getBlockP(m,i,j-1-1,k)>=WATERTYPE)removeWaterFace(m, i, j-1-1, k, 4);
			// if(!*getBlockP(m,i-1,j-1,k)>=WATERTYPE)removeWaterFace(m, i-1, j-1, k, 2);
			// if(!*getBlockP(m,i+1,j-1,k)>=WATERTYPE)removeWaterFace(m, i+1, j-1, k, 3);
			
			*getBlockP(m,i,j-1,k)=type+1;
			addWater(m, i, j-1, k, type+1);
		}else if(d>=WATERTYPE)removeWaterFace(m, i, j-1, k, 4);
		d=*getBlockP(m,i,j+1,k);
		if(!d)
		{
			if(*getBlockP(m,i,j+1,k-1)>=WATERTYPE)removeWaterFace(m, i, j+1, k-1, 0);
			if(!*getBlockP(m,i,j+1,k+1))addWaterFace(m, i, j+1, k, 0);
			
			// if(!*getBlockP(m,i,j+1+1,k)>=WATERTYPE)removeWaterFace(m, i, j+1+1, k, 5);
			// if(!*getBlockP(m,i-1,j+1,k)>=WATERTYPE)removeWaterFace(m, i-1, j+1, k, 2);
			// if(!*getBlockP(m,i+1,j+1,k)>=WATERTYPE)removeWaterFace(m, i+1, j+1, k, 3);
			
			*getBlockP(m,i,j+1,k)=type+1;
			addWater(m, i, j+1, k, type+1);
		}else if(d>=WATERTYPE)removeWaterFace(m, i, j+1, k, 5);
		d=*getBlockP(m,i-1,j,k);
		if(!d)
		{
			if(*getBlockP(m,i-1,j,k-1)>=WATERTYPE)removeWaterFace(m, i-1, j, k-1, 0);
			if(!*getBlockP(m,i-1,j,k+1))addWaterFace(m, i-1, j, k, 0);
			
			// if(!*getBlockP(m,i-1,j-1,k)>=WATERTYPE)removeWaterFace(m, i-1, j-1, k, 4);
			// if(!*getBlockP(m,i-1,j+1,k)>=WATERTYPE)removeWaterFace(m, i-1, j+1, k, 5);
			// if(!*getBlockP(m,i-1-1,j,k)>=WATERTYPE)removeWaterFace(m, i-1-1, j, k, 2);
			
			*getBlockP(m,i-1,j,k)=type+1;
			addWater(m, i-1, j, k, type+1);
		}else if(d>=WATERTYPE)removeWaterFace(m, i-1, j, k, 2);
		d=*getBlockP(m,i+1,j,k);
		if(!d)
		{
			if(*getBlockP(m,i+1,j,k-1)>=WATERTYPE)removeWaterFace(m, i+1, j, k-1, 0);
			if(!*getBlockP(m,i+1,j,k+1))addWaterFace(m, i+1, j, k, 0);
			
			// if(!*getBlockP(m,i+1,j-1,k)>=WATERTYPE)removeWaterFace(m, i+1, j-1, k, 4);
			// if(!*getBlockP(m,i+1,j+1,k)>=WATERTYPE)removeWaterFace(m, i+1, j+1, k, 5);
			// if(!*getBlockP(m,i+1+1,j,k)>=WATERTYPE)removeWaterFace(m, i+1+1, j, k, 3);
			
			*getBlockP(m,i+1,j,k)=type+1;
			addWater(m, i+1, j, k, type+1);
		}else if(d>=WATERTYPE)removeWaterFace(m, i+1, j, k, 3);
		d=*getBlockP(m,i,j,k+1);
		if(d>=WATERTYPE)removeWaterFace(m, i, j, k+1, 1);
	}
	if(waterCursor>=WATERNUMBER)waterCursor-=WATERNUMBER;
}

void freeQuadCache(void)
{
	int i;
	for(i=0;i<cacheRecord;i++)
	{
		free(((quad_struct**)VRAM_F)[i]);
	}
}

void freeLightCache(void)
{
	int i;
	for(i=0;i<lightCacheRecord;i++)
	{
		free(((lightsource_struct**)VRAM_G)[i]);
	}
}

void initQuadCache(void)
{
	int i;
	for(i=0;i<CACHESIZE;i++)
	{
		cache[i]=NULL;
	}
	vramSetBankF(VRAM_F_LCD);
	cacheRecord=0;
	cacheNumber=0;
	cacheCursor=CACHESIZE-1;
}

void initLightCache(void)
{
	int i;
	for(i=0;i<LIGHTCACHESIZE;i++)
	{
		lightCache[i]=NULL;
	}
	vramSetBankG(VRAM_G_LCD);
	lightCacheRecord=0;
	lightCacheNumber=0;
	lightCacheCursor=LIGHTCACHESIZE-1;
}

void cacheAllocateBlock(void) //ONLY IF EMPTY
{
	quad_struct* p=malloc(sizeof(quad_struct)*CACHEBLOCK);
	((quad_struct**)VRAM_F)[cacheRecord++]=p;
	u8 i;
	for(i=0;i<CACHEBLOCK;i++)
	{
		cache[CACHESIZE-1-i]=&p[i];
	}
	cacheCursor=CACHESIZE-1-i;
	cacheNumber+=CACHEBLOCK;
}

void lightCacheAllocateBlock(void) //ONLY IF EMPTY
{
	lightsource_struct* p=malloc(sizeof(lightsource_struct)*LIGHTCACHEBLOCK);
	((lightsource_struct**)VRAM_G)[lightCacheRecord++]=p;
	u16 i;
	for(i=0;i<LIGHTCACHEBLOCK;i++)
	{
		lightCache[LIGHTCACHESIZE-1-i]=&p[i];
	}
	lightCacheCursor=LIGHTCACHESIZE-1-i;
	lightCacheNumber+=LIGHTCACHEBLOCK;
}

lightsource_struct* getLightSource(void)
{
	// NOGBA("get ! %d",cacheNumber);
	if(!lightCacheNumber)lightCacheAllocateBlock();
	lightCacheCursor++;
	lightCacheNumber--;
	lightsource_struct* p=lightCache[lightCacheCursor];
	lightCache[lightCacheCursor]=NULL;
	p->next=NULL;
	return p;
}

void releaseLight(lightsource_struct** q)
{
	lightCache[lightCacheCursor]=*q;
	*q=NULL;
	lightCacheCursor--;
	lightCacheNumber++;
	return;
}

void initmIDTables(void)
{
	u8 i, j, k;
	for(i=0;i<CLUSTERSIZE;i++)
	{
		for(j=0;j<CLUSTERSIZE;j++)
		{
			for(k=0;k<CLUSTERSIZE;k++)
			{
				u8 mID=i+j*CLUSTERSIZE+k*CLUSTERSIZE*CLUSTERSIZE;
				imIDtable[mID]=i;
				jmIDtable[mID]=j;
				kmIDtable[mID]=k;
				// imIDtable2[mID]=i<<3;
				// jmIDtable2[mID]=j<<7;
				// kmIDtable2[mID]=k<<11;
				// ijkmIDtable2[mID]=(i<<3)+(j<<7)+(k<<11);
				ijkmIDtable2[mID]=(i)+(j<<4)+(k<<8);
			}	
		}	
	}
}

void initLightTable(void) //ADD DIRECTION
{
	int i, j, k, d;
	vect3D normal;
	for(d=0;d<14;d++)
	{
		switch(d)
		{
			case 0:
				normal=(vect3D){0,0,inttof32(-1)};
				break;
			case 1:
				normal=(vect3D){0,0,inttof32(1)};
				break;
			case 5:
				normal=(vect3D){0,inttof32(1),0};
				break;
			case 4:
				normal=(vect3D){0,inttof32(-1),0};
				break;
			case 3:
				normal=(vect3D){inttof32(1),0,0};
				break;
			case 2:	
				normal=(vect3D){inttof32(-1),0,0};
				break;
			case 12:
				normal=(vect3D){0,inttof32(1),0};
				break;
			case 13:
				normal=(vect3D){0,inttof32(-1),0};
				break;
			case 10:
				normal=(vect3D){inttof32(1),0,0};
				break;
			case 11:	
				normal=(vect3D){inttof32(-1),0,0};
				break;
		}
		for(i=-8;i<=7;i++)
		{
			for(j=-8;j<=7;j++)
			{
				for(k=-8;k<=7;k++)
				{
					int32 length=sqrtf32(inttof32(i*i+j*j+k*k));
					vect3D vec=(vect3D){-divf32(inttof32(i),(length)),-divf32(inttof32(j),(length)),-divf32(inttof32(k),(length))};
					// NOGBA("%f, %f, %f : %f",f32tofloat(vec.x),f32tofloat(vec.y),f32tofloat(vec.z),f32tofloat(mulf32(vec.x,vec.x)+mulf32(vec.y,vec.y)+mulf32(vec.z,vec.z)));
					int32 ps=max(mulf32(normal.x,vec.x)+mulf32(normal.y,vec.y)+mulf32(normal.z,vec.z),0);
					// NOGBA("%f",f32tofloat(ps));
					// lightTable[(i+6)*6+(j+6)*6*13+(k+6)*6*13*13+d]=f32toint(ps*(max(31-((i)*(i)+(j)*(j)+(k)*(k)),0)));
					// lightTable[((i+8)+(j+8)*16+(k+8)*16*16)*8+d]=f32toint(ps*(max(31-((i)*(i)+(j)*(j)+(k)*(k)),0)));
					// lightTable[((i+8)+(j+8)*16+(k+8)*16*16)*14+d]=f32toint(ps*(max(31-((i)*(i)+(j)*(j)+(k)*(k)),0)));
					lightTable[((i+8)+(j+8)*16+(k+8)*16*16)+(d<<12)]=f32toint(ps*(max(31-((i)*(i)+(j)*(j)+(k)*(k)),0)));
					#ifdef FOGLIGHT
						if(!lightTable[((i+8)+(j+8)*16+(k+8)*16*16)+(d<<12)])lightTable[((i+8)+(j+8)*16+(k+8)*16*16)+(d<<12)]=1;
					#endif
					// lightTable[(i+6)*6+(j+6)*6*13+(k+6)*6*13*13+d]=((max(31-((i)*(i)+(j)*(j)+(k)*(k)),0)));
					// lightTable[(i+6)*6+(j+6)*6*13+(k+6)*6*13*13+d]=min(f32toint(ps*(31)),31);
					// NOGBA("%d,%d,%d : %d (%d)",i,j,k,lightTable[(i+8)*8+(j+8)*8*16+(k+8)*8*16*16+d],(max(31-((i)*(i)+(j)*(j)+(k)*(k)),0)));
				}	
			}	
		}
	}
	for(i=0;i<256;i++)
	{
		for(j=0;j<32;j++)
		{
			lightComputeTable[i+(j<<8)]=min((i&127)+j,127)|(i&(1<<7));
		}
	}
	lightProcess.first=NULL;
	lightProcess.count=0;
}

vect3D waterAnimV;

void initUVmap(void)
{
	int i;
	for(i=0;i<256;i++)
	{
		u8 u=(i%16)*16;
		u8 v=((i-(i%16))/16)*16;
		// NOGBA("UV %d : %d %d",i,u,v);
		uvMap[i*4+0]=TEXTURE_PACK(16*(0+u)+1, 16*(0+v)+1);
		uvMap[i*4+1]=TEXTURE_PACK(16*(16+u)-1, 16*(0+v)+1);
		uvMap[i*4+2]=TEXTURE_PACK(16*(16+u)-1, 16*(16+v)-1);
		uvMap[i*4+3]=TEXTURE_PACK(16*(0+u)+1, 16*(16+v)-1);
	}
	waterAnim.x=16<<5;
	waterAnim.y=12<<5;
	waterAnimV.x=30;
	waterAnimV.y=-50;
}

s16 waterFall;

void updateUVwater(void)
{
	uvMapWater[0]=TEXTURE_PACK(16*(0+(waterAnim.x>>9))+1, 16*(0+(waterAnim.y>>9))+1);
	uvMapWater[1]=TEXTURE_PACK(16*(16+(waterAnim.x>>9))-1, 16*(0+(waterAnim.y>>9))+1);
	uvMapWater[2]=TEXTURE_PACK(16*(16+(waterAnim.x>>9))-1, 16*(16+(waterAnim.y>>9))-1);
	uvMapWater[3]=TEXTURE_PACK(16*(0+(waterAnim.x>>9))+1, 16*(16+(waterAnim.y>>9))-1);
	uvMapWater[0+4]=uvMapWater[0];
	uvMapWater[1+4]=uvMapWater[1];
	uvMapWater[2+4]=uvMapWater[2];
	uvMapWater[3+4]=uvMapWater[3];
	int i;
	for(i=2;i<6;i++)
	{
		uvMapWater[0+4*(i)]=TEXTURE_PACK(16*(0)+1, 16*(0+(waterFall>>2))+1);
		uvMapWater[1+4*(i)]=TEXTURE_PACK(16*(16)-1, 16*(0+(waterFall>>2))+1);
		uvMapWater[2+4*(i)]=TEXTURE_PACK(16*(16)-1, 16*(16+(waterFall>>2))-1);
		uvMapWater[3+4*(i)]=TEXTURE_PACK(16*(0)+1, 16*(16+(waterFall>>2))-1);
	}
	// waterAnim++;
	waterAnimV.x+=-waterAnim.x/128;
	waterAnimV.y+=-waterAnim.y/128;
	// NOGBA("%d %d %d %d",waterAnimV.x,waterAnimV.y,waterAnim.x,waterAnim.y);
	waterAnim.x+=waterAnimV.x;
	waterAnim.y+=waterAnimV.y;
	waterFall--;
}

void initXYmap(void)
{
	int i, j, k, d;
	for(d=0;d<14;d++)
	{
		for(i=0;i<CLUSTERSIZE;i++)
		{
			for(j=0;j<CLUSTERSIZE;j++)
			{
				for(k=0;k<CLUSTERSIZE;k++)
				{
					switch(d)
					{
						case 0:
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+0] = NORMAL_PACK((tilesize+tilesize2*i),(-tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+1] = NORMAL_PACK((tilesize+tilesize2*i),(tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+2] = NORMAL_PACK((-tilesize+tilesize2*i),(tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+3] = NORMAL_PACK((-tilesize+tilesize2*i),(-tilesize+tilesize2*j),(tilesize+tilesize2*k));
							break;
						case 1:
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+0] = NORMAL_PACK((-tilesize+tilesize2*i),(-tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+1] = NORMAL_PACK((-tilesize+tilesize2*i),(tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+2] = NORMAL_PACK((tilesize+tilesize2*i),(tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+3] = NORMAL_PACK((tilesize+tilesize2*i),(-tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							break;
						case 2:
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+0] = NORMAL_PACK((tilesize+tilesize2*i),(tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+1] = NORMAL_PACK((tilesize+tilesize2*i),(-tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+2] = NORMAL_PACK((tilesize+tilesize2*i),(-tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+3] = NORMAL_PACK((tilesize+tilesize2*i),(tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							break;
						case 3:
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+0] = NORMAL_PACK((-tilesize+tilesize2*i),(-tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+1] = NORMAL_PACK((-tilesize+tilesize2*i),(tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+2] = NORMAL_PACK((-tilesize+tilesize2*i),(tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+3] = NORMAL_PACK((-tilesize+tilesize2*i),(-tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							break;
						case 4:
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+0] = NORMAL_PACK((-tilesize+tilesize2*i),(tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+1] = NORMAL_PACK((tilesize+tilesize2*i),(tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+2] = NORMAL_PACK((tilesize+tilesize2*i),(tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+3] = NORMAL_PACK((-tilesize+tilesize2*i),(tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							break;
						case 5:
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+0] = NORMAL_PACK((tilesize+tilesize2*i),(-tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+1] = NORMAL_PACK((-tilesize+tilesize2*i),(-tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+2] = NORMAL_PACK((-tilesize+tilesize2*i),(-tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+3] = NORMAL_PACK((tilesize+tilesize2*i),(-tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							break;	
						case 6:
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+0] = NORMAL_PACK((tilesize+tilesize2*i),(-tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+1] = NORMAL_PACK((-tilesize+tilesize2*i),(tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+2] = NORMAL_PACK((-tilesize+tilesize2*i),(tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+3] = NORMAL_PACK((tilesize+tilesize2*i),(-tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							break;	
						case 7:
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+2] = NORMAL_PACK((tilesize+tilesize2*i),(-tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+3] = NORMAL_PACK((-tilesize+tilesize2*i),(tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+0] = NORMAL_PACK((-tilesize+tilesize2*i),(tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+1] = NORMAL_PACK((tilesize+tilesize2*i),(-tilesize+tilesize2*j),(tilesize+tilesize2*k));
							break;
						case 8:
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+0] = NORMAL_PACK((-tilesize+tilesize2*i),(-tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+1] = NORMAL_PACK((tilesize+tilesize2*i),(tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+2] = NORMAL_PACK((tilesize+tilesize2*i),(tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+3] = NORMAL_PACK((-tilesize+tilesize2*i),(-tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							break;	
						case 9:
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+2] = NORMAL_PACK((-tilesize+tilesize2*i),(-tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+3] = NORMAL_PACK((tilesize+tilesize2*i),(tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+0] = NORMAL_PACK((tilesize+tilesize2*i),(tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+1] = NORMAL_PACK((-tilesize+tilesize2*i),(-tilesize+tilesize2*j),(tilesize+tilesize2*k));
							break;	
						case 10:
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+1] = NORMAL_PACK((tilesize+tilesize2*i),(tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+0] = NORMAL_PACK((tilesize+tilesize2*i),(-tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+3] = NORMAL_PACK((tilesize+tilesize2*i),(-tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+2] = NORMAL_PACK((tilesize+tilesize2*i),(tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							break;
						case 11:
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+1] = NORMAL_PACK((-tilesize+tilesize2*i),(-tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+0] = NORMAL_PACK((-tilesize+tilesize2*i),(tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+3] = NORMAL_PACK((-tilesize+tilesize2*i),(tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+2] = NORMAL_PACK((-tilesize+tilesize2*i),(-tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							break;
						case 12:
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+1] = NORMAL_PACK((-tilesize+tilesize2*i),(tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+0] = NORMAL_PACK((tilesize+tilesize2*i),(tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+3] = NORMAL_PACK((tilesize+tilesize2*i),(tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+2] = NORMAL_PACK((-tilesize+tilesize2*i),(tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							break;
						case 13:
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+1] = NORMAL_PACK((tilesize+tilesize2*i),(-tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+0] = NORMAL_PACK((-tilesize+tilesize2*i),(-tilesize+tilesize2*j),(tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+3] = NORMAL_PACK((-tilesize+tilesize2*i),(-tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							xyMap[i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+d*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+2] = NORMAL_PACK((tilesize+tilesize2*i),(-tilesize+tilesize2*j),(-tilesize+tilesize2*k));
							break;	
					}
				}		
			}		
		}
	}
}

void initDegradTable(void)
{
	int k, l, n;
	s16 a,b,c,d;
	for(k=0;k<8;k++)
	{
		for(l=0;l<8;l++)
		{
			if((k || l) && (k<7 || l<7) && !(!k && l==7) && !(!l && k==7))
			{
				n=k*5+8*5*l;
				a=divf32(inttof32(1),(sqrtf32(inttof32(k*k+l*l))));
				b=divf32(inttof32(1),(sqrtf32(inttof32((8-k)*(8-k)+l*l))));
				c=divf32(inttof32(1),(sqrtf32(inttof32((8-k)*(8-k)+(8-l)*(8-l)))));
				d=divf32(inttof32(1),(sqrtf32(inttof32(k*k+(8-l)*(8-l)))));
				degradTable[n+4]=(divf32(inttof32(1),(a+b+c+d)))>>6;
				//NOGBA("%d, %d, %d; %d, %d",divf32(inttof32(1),(a+b+c+d)),divf32(inttof32(1),(a+b+c+d))>>4,degradTable[n+4]<<4, a, a>>4);
				degradTable[n+0]=((a)>>4)-1;
				degradTable[n+1]=((b)>>4)-1;
				degradTable[n+2]=((c)>>4)-1;
				degradTable[n+3]=((d)>>4)-1;
				if(k<3 && l<3)NOGBA("%d, %d, %d, %d : %d (%d, %d, %d, %d)",degradTable[n+0],degradTable[n+1],degradTable[n+2],degradTable[n+3],degradTable[n+4], a, b, c, d);
				// NOGBA("degradTable : %d",degradTable[n+4]);
			}
		}
	}
	n=0*5+8*5*0;
	degradTable[n+0]=255;
	degradTable[n+1]=0;
	degradTable[n+2]=0;
	degradTable[n+3]=0;
	degradTable[n+4]=1<<6;
	n=7*5+8*5*0;
	degradTable[n+0]=0;
	degradTable[n+1]=255;
	degradTable[n+2]=0;
	degradTable[n+3]=0;
	degradTable[n+4]=1<<6;
	n=7*5+8*5*7;
	degradTable[n+0]=0;
	degradTable[n+1]=0;
	degradTable[n+2]=255;
	degradTable[n+3]=0;
	degradTable[n+4]=1<<6;
	n=0*5+8*5*7;
	degradTable[n+0]=0;
	degradTable[n+1]=0;
	degradTable[n+2]=0;
	degradTable[n+3]=255;
	degradTable[n+4]=1<<6;
}

void addLightProcess(u16 i, u16 j, u16 k)
{
	toProcess_struct* q=malloc(sizeof(toProcess_struct));
	q->next=lightProcess.first;
	q->i=i;
	q->j=j;
	q->k=k;
	lightProcess.first=q;
	lightProcess.count++;
}

void addTileTexture(u8 id, u16* buffer)
{
	u8 i, j;
	u8 u=(id%16)*16;
	u8 v=((id-(id%16))/16)*16;
	NOGBA("TILE ! %d : %d,%d",id,u,v);
	u32 vramTemp = vramSetPrimaryBanks(VRAM_A_LCD,VRAM_B_LCD,VRAM_C_LCD,VRAM_D_LCD);
		for(i=0;i<16;i++)
		{
			for(j=0;j<16;j++)
			{
				((u16*)blockSuperTexture->addr)[(u+i)+(v+j)*256]=buffer[i+j*16];
			}		
		}
	vramRestorePrimaryBanks(vramTemp);
}

MTL_img* processTile(u16* buffer, u8 x, u8 y, u8 id)
{
	int i, j;//, k;
	int i16, j16;
	// u16 palette[256];
	// u16 colors=0;
	// u8 texture[16*16];
	u16 texture[16*16];
	for(i16=0;i16<16;i16++)
	{
		for(j16=0;j16<16;j16++)
		{
			i=x*16+i16;
			j=y*16+j16;
			/*bool ex=false;
			for(k=0;k<colors;k++)if(buffer[i+j*256]==palette[k]){ex=true;break;}
			if(ex)texture[i16+j16*16]=k;
			else *///{texture[i16+j16*16]=colors;palette[colors]=buffer[i+j*256];colors++;}
			texture[i16+j16*16]=buffer[i+j*256];
		}	
	}
	// return Game_CreateTextureBuffer16(texture, 16, 16);
	addTileTexture(id, texture);
	return NULL;
}

MTL_img* processTileFilter(u8* buffer, u8 x, u8 y, u8 r, u8 g, u8 b, u8 id)
{
	int i, j;//, k;
	int i16, j16;
	// u16 palette[256];
	// u16 colors=0;
	// u8 texture[16*16];
	u16 texture[16*16];
	for(i16=0;i16<16;i16++)
	{
		for(j16=0;j16<16;j16++)
		{
			i=x*16+i16;
			j=y*16+j16;
			texture[i16+j16*16]=RGB15((buffer[i*4+j*256*4]*r)>>11,
									(buffer[i*4+j*256*4+1]*g)>>11,
									(buffer[i*4+j*256*4+2]*b)>>11)|((buffer[i*4+j*256*4+3]!=0)<<15);
									// (buffer[i*4+j*256*4+2]*b)>>11)|(1<<15);
		}	
	}
	// return Game_CreateTextureBuffer16(texture, 16, 16);
	addTileTexture(id, texture);
	return NULL;
}

MTL_img* processTileFilterMask(u8* buffer, u16* buff, u8 x, u8 y, u8 x2, u8 y2, u8 r, u8 g, u8 b, u8 id)
{
	int i, j;
	int i2, j2;
	int i16, j16;
	// u16 palette[256];
	// u16 colors=0;
	// u8 texture[16*16];
	u16 texture[16*16];
	for(i16=0;i16<16;i16++)
	{
		for(j16=0;j16<16;j16++)
		{
			i=x*16+i16;
			j=y*16+j16;
			i2=x2*16+i16;
			j2=y2*16+j16;
			if(!buffer[i2*4+j2*256*4+3])texture[i16+j16*16]=buff[i+j*256];
			else texture[i16+j16*16]=RGB15((buffer[i2*4+j2*256*4]*r)>>11,
									(buffer[i2*4+j2*256*4+1]*g)>>11,
									(buffer[i2*4+j2*256*4+2]*b)>>11)|(1<<15);
		}	
	}
	// return Game_CreateTextureBuffer16(texture, 16, 16);
	addTileTexture(id, texture);
	return NULL;
}

u16 getPixel(int i, int j, int x, int y, u16* buff, u8 lum)
{
	int r=buff[(x+i*16)+(y+j*16)*256]&31;
	int g=(buff[(x+i*16)+(y+j*16)*256]>>5)&31;
	int b=(buff[(x+i*16)+(y+j*16)*256]>>10)&31;
	bool a=(buff[(x+i*16)+(y+j*16)*256]>>15)&1;
	return RGB15(r*lum/32,g*lum/32,b*lum/32)|(a<<15);
}

void getIcon(u16* buff, int id, int i1, int j1)
{
	int x, y;
	// int offset=256*192/2;
	// int offset=256*256/2;
	int offset=256*288/2;
	for(x=0;x<16;x++)
	{
		for(y=0;y<16;y++)
		{
			u16* dest=&SPRITE_GFX_SUB[offset+x+(id%8)*16+(y+((id-(id%8))/8)*16)*128];
			*dest = getPixel(i1,j1,x,y,buff,32);
		}
	}
}

void makeIcon(u16* buff, int id, int i1, int j1, int i2, int j2)
{
	int x, y;
	// int offset=256*192/2+8*16*128;
	// int offset=256*256/2;
	int offset=256*288/2;
	for(x=0;x<16;x++)
	{
		for(y=0;y<16;y++)
		{
			// if(y<5){if(abs(8-x)<y)SPRITE_GFX_SUB[x+y*256] = RGB15(31,0,0)|(1<<15);}
			int dx=x, dy=y;
			int dx2=x, dy2=y;
			while(dy>0){dx-=2;dy--;}
			while(dx2>0){dx2-=2;dy2++;}
			dx+=8;
			dy2-=4;
			dy2--;dy2*=2;
			// dx*=16;dy2*=16;
			// dx/=7;dy2/=7;
			// u16* dest=&SPRITE_GFX_SUB[offset+x+(y+id*16)*128];
			u16* dest=&SPRITE_GFX_SUB[offset+x+(id%8)*16+(y+((id-(id%8))/8)*16)*128];
			if(y<4){if(abs(8-x)<y*2){*dest = getPixel(i1,j1,dx,dy2,buff,32);}else *dest = RGB15(0,0,0);}
			else if(y<4*2 && abs(8-x)<16-y*2){*dest = getPixel(i1,j1,dx,dy2,buff,32);}
			else if(x>8 && abs(8-x)<2*(15-y) && y<15)*dest = getPixel(i2,j2,(x-9)*2,(y-5-(16-x)/2)*2,buff,22);
			else if(x>0 && abs(8-x)<2*(15-y) && y<15)*dest = getPixel(i2,j2,(x-1)*2,(y-5-(x-1)/2)*2,buff,32);
			else *dest = RGB15(0,0,0);
			// SPRITE_GFX_SUB[x+y*256] = buff[3*16+dx+dy2*256]|(1<<15);
		}
	}
}

void loadBlockTextures(bool spr, bool tex)
{
	int i, j;
	u8 k, l;
	if(tex)
	{
		waterTexture=Game_CreateTextureAlpha("12.pcx", "textures", 27);
		cursorTexture=Game_CreateTexture("256.pcx", "textures");
		crossHair=Game_CreateTexture("crosshair.pcx", "textures");
	}
	
	char path[255];
	getcwd(path,255);
	// chdir("packs/eldpack");
	chdir(packPath);
	
	if(tex)blockSuperTexture=Game_CreateTextureBuffer16(NULL, 256, 64, false);

	unsigned char* buffer;
	unsigned char* image;
	size_t buffersize, imagesize;
	LodePNG_Decoder decoder;
	
	char path2[255];
	getcwd(path2,255);
	chdir("misc");
	LodePNG_loadFile(&buffer, &buffersize, "grasscolor.png");
	LodePNG_Decoder_init(&decoder);
	LodePNG_Decoder_decode(&decoder, &image, &imagesize, buffer, buffersize);
	
	u8 biomeX=120, biomeY=120;
	
	u8 grassR=image[biomeX*4+biomeY*4*decoder.infoPng.width];
	u8 grassG=image[biomeX*4+biomeY*4*decoder.infoPng.width+1];
	u8 grassB=image[biomeX*4+biomeY*4*decoder.infoPng.width+2];
	
	free(image);
	free(buffer);
	LodePNG_Decoder_cleanup(&decoder);
	
	LodePNG_loadFile(&buffer, &buffersize, "foliagecolor.png");
	LodePNG_Decoder_init(&decoder);
	LodePNG_Decoder_decode(&decoder, &image, &imagesize, buffer, buffersize);
	
	u8 foliageR=image[biomeX*4+biomeY*4*decoder.infoPng.width];
	u8 foliageG=image[biomeX*4+biomeY*4*decoder.infoPng.width+1];
	u8 foliageB=image[biomeX*4+biomeY*4*decoder.infoPng.width+2];
	
	free(image);
	free(buffer);
	LodePNG_Decoder_cleanup(&decoder);
	
	chdir(path2);

	LodePNG_loadFile(&buffer, &buffersize, "terrain.png");
	LodePNG_Decoder_init(&decoder);
	LodePNG_Decoder_decode(&decoder, &image, &imagesize, buffer, buffersize);
	if(decoder.error) NOGBA("error %u: %s\n", decoder.error, LodePNG_error_text(decoder.error));
	else
	{
		u16* buff=malloc(decoder.infoPng.width*decoder.infoPng.height*sizeof(u16));
		for(i=0;i<decoder.infoPng.width;i++)
		{
			for(j=0;j<decoder.infoPng.height;j++)
			{
				buff[i+j*decoder.infoPng.width]=RGB15(image[i*4+j*4*decoder.infoPng.width]>>3,
													  image[i*4+j*4*decoder.infoPng.width+1]>>3,
													  image[i*4+j*4*decoder.infoPng.width+2]>>3)|((image[i*4+j*4*decoder.infoPng.width+3]!=0)<<15);
			}
		}
		
		for(i=0;i<BLOCKTEXTURES;i++)
		{
			switch(i)
			{
				case 1:
					processTileFilterMask(image, buff, blockTextures[i].i, blockTextures[i].j, 6, 2, grassR,grassG,grassB,i);
					break;
				case 2:
					processTileFilter(image,blockTextures[i].i,blockTextures[i].j,grassR,grassG,grassB,i);
					break;
				case 10:
					processTileFilter(image,blockTextures[i].i,blockTextures[i].j,foliageR,foliageG,foliageB,i);
					break;
				default:
					processTile(buff,blockTextures[i].i,blockTextures[i].j,i);
					break;
			}
		}
		NOGBA("supertex done");
		if(spr)
		{
			for(i=0;i<BLOCKS;i++)
			{
				switch(i)
				{
					case 13:
						getIcon(buff, i, blockTextures[i].i, blockTextures[i].j);
						break;
					case LADDERTYPE:
						getIcon(buff, i, blockTextures[i].i, blockTextures[i].j);
						break;
					default:
						makeIcon(buff, i, blockTextures[blocks[i].top].i, blockTextures[blocks[i].top].j, blockTextures[blocks[i].side].i, blockTextures[blocks[i].side].j);
						break;
				}
			}
		}
		free(buff);
		NOGBA("supertex done2");
	}

	/*cleanup decoder*/
	free(image);
	free(buffer);
	LodePNG_Decoder_cleanup(&decoder);
	
	getcwd(path2,255);
	chdir("gui");
	LodePNG_loadFile(&buffer, &buffersize, "items.png");
	LodePNG_Decoder_init(&decoder);
	LodePNG_Decoder_decode(&decoder, &image, &imagesize, buffer, buffersize);
	
		u16* buff=malloc(decoder.infoPng.width*decoder.infoPng.height*sizeof(u16));
		for(i=0;i<decoder.infoPng.width;i++)
		{
			for(j=0;j<decoder.infoPng.height;j++)
			{
				buff[i+j*decoder.infoPng.width]=RGB15(image[i*4+j*4*decoder.infoPng.width]>>3,
													  image[i*4+j*4*decoder.infoPng.width+1]>>3,
													  image[i*4+j*4*decoder.infoPng.width+2]>>3)|((image[i*4+j*4*decoder.infoPng.width+3]!=0)<<15);
			}
		}
		getIcon(buff, 11, 11, 4);
		processTile(buff,11,4,11);
		getIcon(buff,DOORTYPE,11,2);
		processTile(buff,11,2,DOORTYPE);
		free(buff);

	free(image);
	free(buffer);
	LodePNG_Decoder_cleanup(&decoder);
	NOGBA("supertex done3");
	chdir(path);
}

void cleanList(list_struct* l)
{
	l->size=0;
}

static inline void addListElement(list_struct* l, u16 i, u16 j, u16 k, u8 direction)
{
	l->elements[l->size].i=i;l->elements[l->size].j=j;l->elements[l->size].k=k;l->elements[l->size].direction=direction;
	l->size++;
}

int olcursor;

void cullClusters2(map_struct* m, list_struct* ol, list_struct* cl, int sI, int sJ, int sK)
{
	int i, bid, count;
	int cx=SUPERCLUSTERSIZE, cxy=SUPERCLUSTERSIZE*SUPERCLUSTERSIZE;
	cleanList(cl);
	if(!testBuffer)
	{
		cullMagic++;
		cleanList(ol);
		addListElement(cl, sI, sJ, sK, 0);
		bid=qgetCluster(m,sI,sJ,sK);m->clusterDraw[bid]=cullMagic;
		if(sI<SUPERCLUSTERSIZE-1){addListElement(ol, sI+1, sJ, sK, dir_x);m->clusterDraw[bid+1]=cullMagic;}
		if(sI>0){addListElement(ol, sI-1, sJ, sK, dir_x);m->clusterDraw[bid-1]=cullMagic;}
		if(sJ<SUPERCLUSTERSIZE-1){addListElement(ol, sI, sJ+1, sK, dir_y);m->clusterDraw[bid+cx]=cullMagic;}
		if(sJ>0){addListElement(ol, sI, sJ-1, sK, dir_y);m->clusterDraw[bid-cx]=cullMagic;}
		if(sK<m->clusterSize.z-1){addListElement(ol, sI, sJ, sK+1, dir_z);m->clusterDraw[bid+cxy]=cullMagic;}
		if(sK>0){addListElement(ol, sI, sJ, sK-1, dir_z);m->clusterDraw[bid-cxy]=cullMagic;}
		olcursor=0;
	}
	
	count=0;
		i=olcursor;
		listElement_struct *le=&ol->elements[i];
		listElement_struct *le2=&ol->elements[i+1];
		if(i<ol->size)BoxTest_Asynch((le->i)*bsize-(tilesize<<6),(le->j)*bsize-(tilesize<<6),(le->k)*bsize-(tilesize<<6),bsize,bsize,bsize);
		int r, n=0;
		// for(i=olcursor;i<ol->size && cl->size<300 && count<1700 && i-olcursor<325;i++) // test values
		// for(i=olcursor;i+1<ol->size && n<300 && count<1700;i++) //good
		for(i=olcursor;i+1<ol->size && n<400 && count<1700;i++) //test
		{
			r=BoxTestResult();
			if(i+1<ol->size)
			{
				le2=&ol->elements[i+1];
				BoxTest_Asynch((le2->i)*bsize-(tilesize<<6),(le2->j)*bsize-(tilesize<<6),(le2->k)*bsize-(tilesize<<6),bsize,bsize,bsize);
			}
			if(r || i<6) //test
			{
				bid=qgetCluster(m,le->i,le->j,le->k);
				if((m->superCluster[le->i][le->j]->cluster[le->k].quadList.count || m->superCluster[le->i][le->j]->cluster[le->k].specialList.count) && r)addListElement(cl, le->i, le->j, le->k, 0);
				n++;
				m->clusterDraw[bid]=cullMagic;
				// count+=m->cluster[bid].quadList.count;
				count+=m->superCluster[le->i][le->j]->cluster[le->k].quadList.count+m->superCluster[le->i][le->j]->cluster[le->k].specialList.count;
				bool d1=m->superCluster[le->i][le->j]->cluster[le->k].wall&1, d2=(m->superCluster[le->i][le->j]->cluster[le->k].wall>>1)&1, d3=(m->superCluster[le->i][le->j]->cluster[le->k].wall>>2)&1;
				// NOGBA("ICI : %d %d %d (%d%d%d)(%d)",le->i,le->j,le->k,d1,d2,d3,m->cluster[bid].wall);
				
				u8 newdir=dir_x|le->direction;		
				if(!(d3 && le->direction&dir_x))
				{
					if(le->i<SUPERCLUSTERSIZE-1 && m->clusterDraw[bid+1]!=cullMagic){addListElement(ol, le->i+1, le->j, le->k, newdir);m->clusterDraw[bid+1]=cullMagic;}
					if(le->i>0 && m->clusterDraw[bid-1]!=cullMagic){addListElement(ol, le->i-1, le->j, le->k, newdir);m->clusterDraw[bid-1]=cullMagic;}
				}
				if(!(d2 && le->direction&dir_y))
				{
					newdir=dir_y|le->direction;
					if(le->j<SUPERCLUSTERSIZE-1 && m->clusterDraw[bid+cx]!=cullMagic){addListElement(ol, le->i, le->j+1, le->k, newdir);m->clusterDraw[bid+cx]=cullMagic;}
					if(le->j>0 && m->clusterDraw[bid-cx]!=cullMagic){addListElement(ol, le->i, le->j-1, le->k, newdir);m->clusterDraw[bid-cx]=cullMagic;}
				}
				if(!(d1 && le->direction&dir_z))
				{
					newdir=dir_z|le->direction;
					if(le->k<m->clusterSize.z-1 && m->clusterDraw[bid+cxy]!=cullMagic){addListElement(ol, le->i, le->j, le->k+1, newdir);m->clusterDraw[bid+cxy]=cullMagic;}
					if(le->k>0 && m->clusterDraw[bid-cxy]!=cullMagic){addListElement(ol, le->i, le->j, le->k-1, newdir);m->clusterDraw[bid-cxy]=cullMagic;}
				}
			}
			le=le2;
		}
	olcursor=i;
	#ifdef DEBUGMODE
	iprintf("\nculled %d clust (%d %d %d)",cl->size,i,ol->size,count);
	#endif
}

void cullClusters(map_struct* m, list_struct* ol, list_struct* cl, int sI, int sJ, int sK)
{
	int i, bid, bid2, count;
	int cx=SUPERCLUSTERSIZE, cxy=SUPERCLUSTERSIZE*SUPERCLUSTERSIZE;
	cleanList(cl);
	if(!testBuffer)
	{
		cullMagic++;
		cleanList(ol);
		addListElement(cl, sI, sJ, sK, 0);
		bid=qgetCluster(m,sI,sJ,sK);m->clusterDraw[bid]=cullMagic;
		if(sI<SUPERCLUSTERSIZE-1){addListElement(ol, sI+1, sJ, sK, dir_x);m->clusterDraw[bid+1]=cullMagic;}
		if(sI>0){addListElement(ol, sI-1, sJ, sK, dir_x);m->clusterDraw[bid-1]=cullMagic;}
		if(sJ<SUPERCLUSTERSIZE-1){addListElement(ol, sI, sJ+1, sK, dir_y);m->clusterDraw[bid+cx]=cullMagic;}
		if(sJ>0){addListElement(ol, sI, sJ-1, sK, dir_y);m->clusterDraw[bid-cx]=cullMagic;}
		if(sK<m->clusterSize.z-1){addListElement(ol, sI, sJ, sK+1, dir_z);m->clusterDraw[bid+cxy]=cullMagic;}
		if(sK>0){addListElement(ol, sI, sJ, sK-1, dir_z);m->clusterDraw[bid-cxy]=cullMagic;}
		olcursor=0;
	}
	
	count=0;
		i=olcursor;
		listElement_struct *le=&ol->elements[i];
		listElement_struct *le2=&ol->elements[i+1];
		if(i<ol->size)BoxTest_Asynch((le->i)*bsize-(tilesize<<6),(le->j)*bsize-(tilesize<<6),(le->k)*bsize-(tilesize<<6),bsize,bsize,bsize);
		int r, n=0;
		u8 o=0;
		// for(i=olcursor;i<ol->size && cl->size<300 && count<1700 && i-olcursor<325;i++) // test values
		// for(i=olcursor;i+1<ol->size && n<300 && count<1700;i++) //good
		for(i=olcursor;i+1<ol->size && n<400 && count<1700;i++) //test
		{
			bid=qgetCluster(m,le->i,le->j,le->k);
			if(m->clusterDrawn[bid]<cullMagic-1)r=BoxTestResult();
			else {r=3;}
			if(i+1<ol->size)
			{
				le2=&ol->elements[i+1];
				bid2=qgetCluster(m,le2->i,le2->j,le2->k);
				if(m->clusterDrawn[bid2]<cullMagic-1)BoxTest_Asynch((le2->i)*bsize-(tilesize<<6),(le2->j)*bsize-(tilesize<<6),(le2->k)*bsize-(tilesize<<6),bsize,bsize,bsize);
			}
			if(r || i<6) //test
			{
				// bid=qgetCluster(m,le->i,le->j,le->k);
				if((m->superCluster[le->i][le->j]->cluster[le->k].quadList.count || m->superCluster[le->i][le->j]->cluster[le->k].specialList.count) && r)
				{
					addListElement(cl, le->i, le->j, le->k, 0);
				}
				if(r<3){m->clusterDrawn[bid]=cullMagic+o%3;o++;}
				n++;
				m->clusterDraw[bid]=cullMagic;
				// count+=m->cluster[bid].quadList.count;
				count+=m->superCluster[le->i][le->j]->cluster[le->k].quadList.count+m->superCluster[le->i][le->j]->cluster[le->k].specialList.count;
				bool d1=m->superCluster[le->i][le->j]->cluster[le->k].wall&1, d2=(m->superCluster[le->i][le->j]->cluster[le->k].wall>>1)&1, d3=(m->superCluster[le->i][le->j]->cluster[le->k].wall>>2)&1;
				// NOGBA("ICI : %d %d %d (%d%d%d)(%d)",le->i,le->j,le->k,d1,d2,d3,m->cluster[bid].wall);
				
				u8 newdir=dir_x|le->direction;		
				if(!(d3 && le->direction&dir_x))
				{
					if(le->i<SUPERCLUSTERSIZE-1 && m->clusterDraw[bid+1]!=cullMagic){addListElement(ol, le->i+1, le->j, le->k, newdir);m->clusterDraw[bid+1]=cullMagic;}
					if(le->i>0 && m->clusterDraw[bid-1]!=cullMagic){addListElement(ol, le->i-1, le->j, le->k, newdir);m->clusterDraw[bid-1]=cullMagic;}
				}
				if(!(d2 && le->direction&dir_y))
				{
					newdir=dir_y|le->direction;
					if(le->j<SUPERCLUSTERSIZE-1 && m->clusterDraw[bid+cx]!=cullMagic){addListElement(ol, le->i, le->j+1, le->k, newdir);m->clusterDraw[bid+cx]=cullMagic;}
					if(le->j>0 && m->clusterDraw[bid-cx]!=cullMagic){addListElement(ol, le->i, le->j-1, le->k, newdir);m->clusterDraw[bid-cx]=cullMagic;}
				}
				if(!(d1 && le->direction&dir_z))
				{
					newdir=dir_z|le->direction;
					if(le->k<m->clusterSize.z-1 && m->clusterDraw[bid+cxy]!=cullMagic){addListElement(ol, le->i, le->j, le->k+1, newdir);m->clusterDraw[bid+cxy]=cullMagic;}
					if(le->k>0 && m->clusterDraw[bid-cxy]!=cullMagic){addListElement(ol, le->i, le->j, le->k-1, newdir);m->clusterDraw[bid-cxy]=cullMagic;}
				}
			}
			le=le2;
		}
	olcursor=i;
	#ifdef DEBUGMODE
	iprintf("\nculled %d clust (%d %d %d)",cl->size,i,ol->size,count);
	#endif
}

void addQuad(quadList_struct* ql, map_struct* m, u8 direction, u8 light, u32 bid, u8* data, u16 i, u16 j, u16 k)
{
	// quad_struct* q=malloc(sizeof(quad_struct));
	quad_struct* q=getQuad();
	// int t=DS_FreeMem();
	// quad_struct* q=malloc(2);
	// NOGBA("2 : %d",t-DS_FreeMem());
	ql->count++;
	// q->a=a;q->b=b;q->c=c;q->d=d;
	
	// TESTVALUE2++;
	
	// q->blockID=bid;
	
	// q->i=q->blockID%map.size.x;
	// q->j=((q->blockID-q->i)%(map.size.x*map.size.y))/(map.size.x);
	// q->k=(q->blockID-q->i-q->j*(map.size.x))/(map.size.x*map.size.y);
	i%=CLUSTERSIZE;
	j%=CLUSTERSIZE;
	// q->i=i;
	// q->j=j;
	// q->k=k;
	
	q->light=light;
	q->direction=direction;q->next=NULL;
	// q->type=*getBlockP(m,q->i,q->j,q->k);
	q->type=data[i+j*CLUSTERSIZE+k*CLUSTERSIZE*CLUSTERSIZE];
	if(q->type>=WATERTYPE)
	{
		// q->type=0;
		q->type=direction;
	}else{
		switch(q->type)
		{
			case 1:
				if(!q->direction)q->type=2;
				else if(q->direction==1 || (k<m->size.z && data[i+j*CLUSTERSIZE+(k+1)*CLUSTERSIZE*CLUSTERSIZE]))q->type=0;
				else q->type=1;
				break;
			default :
				// q->type=min(q->type,BLOCKTEXTURES-1);
				if(!q->direction)q->type=blocks[q->type].top;
				else if(q->direction==1)q->type=blocks[q->type].bottom;
				else q->type=blocks[q->type].side;
				break;
		}
	}
	k%=CLUSTERSIZE;
	// q->k%=CLUSTERSIZE;
	q->mID=i+j*CLUSTERSIZE+k*CLUSTERSIZE*CLUSTERSIZE;
	// q->lID=i*4+j*4*CLUSTERSIZE+k*4*CLUSTERSIZE*CLUSTERSIZE+q->direction*CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*4+0;
	if(ql->first)q->next=ql->first;
	ql->first=q;
	// if(ql->last==NULL){ql->first=ql->last=q;}
	// else {ql->last=ql->last->next=q;}
}

void addLight(lightsourceList_struct* ql, map_struct* m, s8 i, s8 j, s8 k)
{
	// lightsource_struct* q=malloc(sizeof(lightsource_struct));
	lightsource_struct* q=getLightSource();
	ql->count=(((ql->count&127)+1)&127)|(ql->count&128);
	q->i=i;
	q->j=j;
	q->k=k;
	// NOGBA("first ! %p",ql->first);
	q->next=ql->first;
	// q->next=NULL;
	ql->first=q;
	// NOGBA("next ! %p",q->next);
	// if(ql->last==NULL){ql->first=ql->last=q;}
	// else {ql->last=ql->last->next=q;}
}

void adjustBlockLight(map_struct* m, int i, int j, int k)
{
	// quadList_struct* ql=&m->quadList;
	
	// u32 bid=(i)+(j)*(m)->size.x+(k)*(m)->size.y*(m)->size.x;
	
	// int bid1=bid+1, bid2=bid-1, bid3=bid+m->size.x, bid4=bid-m->size.x, bid5=bid+m->size.x*m->size.y, bid6=bid-m->size.x*m->size.y;
	// u16 clusterID=getClusterID(m,i,j,k);
	// quadList_struct* ql=&m->cluster[clusterID].quadList;
	vect3D clusterCoord=getCluster(m,i,j,k);
	quadList_struct* ql;
	u8 type=*getBlockP(m,i,j,k);
	if(type>=WATERTYPE)ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].specialList;
	else ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].quadList;
	quad_struct* q=ql->first;
	// NOGBA("f, l : %p ; %p",ql->first,ql->last);
	bool ls=(k==getHighest(m, i, j)
			|| k>getHighest(m, i+1, j)
			|| k>getHighest(m, i-1, j)
			|| k>getHighest(m, i, j+1)
			|| k>getHighest(m, i, j-1));
	NOGBA("ls : %d (%d %d %d %d %d)",ls,getHighest(m, i, j),getHighest(m, i+1, j),getHighest(m, i-1, j),getHighest(m, i, j+1),getHighest(m, i, j-1));
	int i1=i,j1=j,k1=k;
	i%=CLUSTERSIZE;
	j%=CLUSTERSIZE;
	k%=CLUSTERSIZE;
	u8 mID=i+j*CLUSTERSIZE+k*CLUSTERSIZE*CLUSTERSIZE;
	while(q)
	{
		if(q->mID==mID)
		{
			q->light=(q->light&127)|((ls&1)<<7);
		}
		q=q->next;
	}
}

void removeBlock(map_struct* m, int i, int j, int k, bool fix)
{
	// quadList_struct* ql=&m->quadList;
	
	// u32 bid=(i)+(j)*(m)->size.x+(k)*(m)->size.y*(m)->size.x;
	
	// int bid1=bid+1, bid2=bid-1, bid3=bid+m->size.x, bid4=bid-m->size.x, bid5=bid+m->size.x*m->size.y, bid6=bid-m->size.x*m->size.y;
	// u16 clusterID=getClusterID(m,i,j,k);
	// quadList_struct* ql=&m->cluster[clusterID].quadList;
	vect3D clusterCoord=getCluster(m,i,j,k);
	quadList_struct* ql;
	u8 type=*getBlockP(m,i,j,k);
	if(type>=WATERTYPE)ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].specialList;
	else ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].quadList;
	quad_struct* oq=ql->first;
	quad_struct* q;
	if(oq)q=oq->next;
	else q=NULL;
	// NOGBA("f, l : %p ; %p",ql->first,ql->last);
	int i1=i,j1=j,k1=k;
	i%=CLUSTERSIZE;
	j%=CLUSTERSIZE;
	k%=CLUSTERSIZE;
	u8 mID=i+j*CLUSTERSIZE+k*CLUSTERSIZE*CLUSTERSIZE;
	while(q)
	{
		// if(q->i==i && q->j==j && q->k==k)
		if(q->mID==mID)
		{
			oq->next=q->next;
			// if(!oq->next)ql->last=oq;
			// free(q);
			releaseQuad(&q);
			ql->count--;
			q=oq->next;
		}else{
			oq=q;
			q=q->next;
		}
	}
	q=ql->first;
	// if(q && q->i==i && q->j==j && q->k==k)
	if(q && q->mID==mID)
	{
		ql->first=q->next;
		// if(!ql->first)ql->last=NULL;
		ql->count--;
		// free(q);
		releaseQuad(&q);
	}
	// NOGBA("f, l : %p ; %p",ql->first,ql->last);
	if(fix)fixGap(m, i1, j1, k1);
}

void fixGap(map_struct* m, int i, int j, int k)
{
	if(*getBlockP(m,i,j,k))return;
	u32 bid=(i)+(j)*(m)->size.x+(k)*(m)->size.y*(m)->size.x;
	if(block(*getBlockP(m,i,j,k-1)))
	{
		u8 light=0;
		surface(m, i, j, k-1, &light);
		getLight(m, i, j, k-1, &light, 0);
		vect3D clusterCoord=getCluster(m,i,j,k-1);
		quadList_struct* ql;
		if(*getBlockP(m,i,j,k-1)>=WATERTYPE)ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].specialList;
		else ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].quadList;
		addQuad(ql, m, 0, light, bid-(m)->size.y*(m)->size.x, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k-1);
	}
	if(block(*getBlockP(m,i,j,k+1)))
	{
		u8 light=0;
		surface(m, i, j, k+1, &light);
		getLight(m, i, j, k+1, &light, 1);
		vect3D clusterCoord=getCluster(m,i,j,k+1);
		quadList_struct* ql;
		if(*getBlockP(m,i,j,k+1)>=WATERTYPE)ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].specialList;
		else ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].quadList;
		addQuad(ql, m, 1, light, bid+(m)->size.y*(m)->size.x, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k+1);
	}
	
	if(block(*getBlockP(m,i-1,j,k)))
	{
		u8 light=0;
		surface(m, i-1, j, k, &light);
		getLight(m, i-1, j, k, &light, 2);
		vect3D clusterCoord=getCluster(m,i-1,j,k);
		quadList_struct* ql;
		if(*getBlockP(m,i-1,j,k)>=WATERTYPE)ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].specialList;
		else ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].quadList;
		addQuad(ql, m, 2, light, bid-1, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i-1, j, k);
	}
	if(block(*getBlockP(m,i+1,j,k)))
	{
		u8 light=0;
		surface(m, i+1, j, k, &light);
		getLight(m, i+1, j, k, &light, 3);
		vect3D clusterCoord=getCluster(m,i+1,j,k);
		quadList_struct* ql;
		if(*getBlockP(m,i+1,j,k)>=WATERTYPE)ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].specialList;
		else ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].quadList;
		addQuad(ql, m, 3, light, bid+1, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i+1, j, k);
	}
	
	if(block(*getBlockP(m,i,j-1,k)))
	{
		u8 light=0;
		surface(m, i, j-1, k, &light);
		getLight(m, i, j-1, k, &light, 4);
		vect3D clusterCoord=getCluster(m,i,j-1,k);
		quadList_struct* ql;
		if(*getBlockP(m,i,j-1,k)>=WATERTYPE)ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].specialList;
		else ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].quadList;
		addQuad(ql, m, 4, light, bid-(m)->size.x, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j-1, k);
	}
	if(block(*getBlockP(m,i,j+1,k)))
	{
		u8 light=0;
		surface(m, i, j+1, k, &light);
		getLight(m, i, j+1, k, &light, 5);
		vect3D clusterCoord=getCluster(m,i,j+1,k);
		quadList_struct* ql;
		if(*getBlockP(m,i,j+1,k)>=WATERTYPE)ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].specialList;
		else ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].quadList;
		addQuad(ql, m, 5, light, bid+(m)->size.x, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j+1, k);
	}
}

void processLight(map_struct* m, int i, int j, int k, const vect3D clusterCoord)
{
	int x, y, z;
	// const u32 ijk3=i*8+j*8*16+k*8*16*16;
	const u32 ijk3=i+j*16+k*16*16;
	u16 x1=max(clusterCoord.x-1,m->offset.x),x2=min(clusterCoord.x+2,m->offset.x+SUPERCLUSTERSIZE-1);
	u16 y1=max(clusterCoord.y-1,m->offset.y),y2=min(clusterCoord.y+2,m->offset.y+SUPERCLUSTERSIZE-1);
	u16 z1=max(clusterCoord.z-1,0),z2=min(clusterCoord.z+2,m->clusterSize.z);
	NOGBA("hoho : %d %d %d %d vs %d %d",x1,x2,y1,y2,clusterCoord.x,clusterCoord.y);
	for(x=x1;x<x2;x++)
	{
		for(y=y1;y<y2;y++)
		{
			for(z=z1;z<z2;z++)
			{
				// u32 xyz2=(x+(y<<4)+(z<<8))<<(5); //CLUSTERSIZE EN HARD ! *CLUSTERSIZE*8
				u32 xyz2=(x+(y<<4)+(z<<8))<<(2); //CLUSTERSIZE EN HARD ! *CLUSTERSIZE
				{
					quadList_struct* ql=&m->superCluster[x-m->offset.x][y-m->offset.y]->cluster[z-m->offset.z].quadList;
					quad_struct* q=ql->first;
					while(q)
					{
						// u16 i2=i-(x*CLUSTERSIZE+imIDtable[q->mID])+6,j2=j-(y*CLUSTERSIZE+jmIDtable[q->mID])+6,k2=k-(z*CLUSTERSIZE+kmIDtable[q->mID])+6;
						// if(i2>0 && j2>0 && k2>0 && i2<13 && j2<13 && k2<13)
						// {
							// q->light+=max(31-((i-i2)*(i-i2)+(j-j2)*(j-j2)+(k-k2)*(k-k2)),0);
							// q->light=min((q->light&127)+lightTable[i2*6+j2*6*13+k2*6*13*13+q->direction],127)|(q->light&(1<<7));
							// q->light=lightComputeTable[q->light+(lightTable[ijk3-xyz2-(ijkmIDtable2[q->mID])+8*8+8*8*16+8*8*16*16+q->direction]<<8)];
							q->light=lightComputeTable[q->light+(lightTable[ijk3-xyz2-(ijkmIDtable2[q->mID])+8+8*16+8*16*16+(q->direction<<12)]<<8)];
						// }
						q=q->next;
					}
				}
				{
					quadList_struct* ql=&m->superCluster[x-m->offset.x][y-m->offset.y]->cluster[z-m->offset.z].specialList;
					quad_struct* q=ql->first;
					while(q)
					{
						// int i2=i-(x*CLUSTERSIZE+imIDtable[q->mID])+6,j2=j-(y*CLUSTERSIZE+jmIDtable[q->mID])+6,k2=k-(z*CLUSTERSIZE+kmIDtable[q->mID])+6;
						// if(i2>0 && j2>0 && k2>0 && i2<13 && j2<13 && k2<13)
						// {
							// q->light+=max(31-((i-i2)*(i-i2)+(j-j2)*(j-j2)+(k-k2)*(k-k2)),0);
							// q->light=min((q->light&127)+lightTable[i2*6+j2*6*13+k2*6*13*13+q->direction],127)|(q->light&(1<<7));
							// q->light=lightTable[i2*6+j2*6*13+k2*6*13*13+q->direction];
							// q->light=lightComputeTable[q->light+(lightTable[ijk3-xyz2-(ijkmIDtable2[q->mID])+8*8+8*8*16+8*8*16*16+q->direction]<<8)];
							q->light=lightComputeTable[q->light+(lightTable[ijk3-xyz2-(ijkmIDtable2[q->mID])+8+8*16+8*16*16+(q->direction<<12)]<<8)];
						// }
						q=q->next;
					}
				}
				addLight(&m->superCluster[x-m->offset.x][y-m->offset.y]->cluster[z-m->offset.z].lightList, m, i-CLUSTERSIZE*x, j-CLUSTERSIZE*y, k-CLUSTERSIZE*z);
			}
		}
	}
}

bool compPos(vect3D p1, vect3D p2)
{
	return p1.x==p2.x && p1.z==p2.z && p1.y==p2.y;
}

void changeBlock(map_struct* m, int i, int j, int k, u8 type)
{
	// u32 bid=(i)+(j)*(m)->size.x+(k)*(m)->size.y*(m)->size.x;
	// u8 ot=m->data[bid];
	// m->data[bid]=type;
	u8 ot=(*getBlockP(m, i, j, k));
	if(ot==5 || (ot>=WATERTYPE && !type))return;
		if(solid(type))
		{
			vect3D block=(vect3D){i,j,k};
			if(compPos(getPointBlockPos(m, Player.position.x+BBSIZE, Player.position.y-BBSIZE, Player.position.z-6000),block)
			|| compPos(getPointBlockPos(m, Player.position.x+BBSIZE, Player.position.y+BBSIZE, Player.position.z-6000),block)
			|| compPos(getPointBlockPos(m, Player.position.x-BBSIZE, Player.position.y+BBSIZE, Player.position.z-6000),block)
			|| compPos(getPointBlockPos(m, Player.position.x-BBSIZE, Player.position.y-BBSIZE, Player.position.z-6000),block)
			|| compPos(getPointBlockPos(m, Player.position.x+BBSIZE, Player.position.y-BBSIZE, Player.position.z-2000),block)
			|| compPos(getPointBlockPos(m, Player.position.x+BBSIZE, Player.position.y+BBSIZE, Player.position.z-2000),block)
			|| compPos(getPointBlockPos(m, Player.position.x-BBSIZE, Player.position.y+BBSIZE, Player.position.z-2000),block)
			|| compPos(getPointBlockPos(m, Player.position.x-BBSIZE, Player.position.y-BBSIZE, Player.position.z-2000),block)
			|| compPos(getPointBlockPos(m, Player.position.x+BBSIZE, Player.position.y-BBSIZE, Player.position.z+2000),block)
			|| compPos(getPointBlockPos(m, Player.position.x+BBSIZE, Player.position.y+BBSIZE, Player.position.z+2000),block)
			|| compPos(getPointBlockPos(m, Player.position.x-BBSIZE, Player.position.y+BBSIZE, Player.position.z+2000),block)
			|| compPos(getPointBlockPos(m, Player.position.x-BBSIZE, Player.position.y-BBSIZE, Player.position.z+2000),block))return;
		}	
	
	// u16 clusterID=getClusterID(m,i,j,k);
	// quadList_struct* ql=&m->cluster[clusterID].quadList;
	NOGBA("HEHE : %d %d", ot, type);
	vect3D clusterCoord=getCluster(m,i,j,k);
	u8* t=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->highest[(i%CLUSTERSIZE)+(j%CLUSTERSIZE)*CLUSTERSIZE];
	u8 oldt=*t;
	if(!type)
	{
		(*getBlockP(m, i, j, k))=type;
		if(k==(*t))
		{
			while(seeThrough(*getBlockP(m, i, j, *t)))(*t)--;
		}
		removeBlock(m, i, j, k, true);
		if(ot==13)
		{
			int x, y, z;
			PROF_START();
			u16 x1=max(clusterCoord.x-1,m->offset.x),x2=min(clusterCoord.x+2,m->offset.x+SUPERCLUSTERSIZE-1);
			u16 y1=max(clusterCoord.y-1,m->offset.y),y2=min(clusterCoord.y+2,m->offset.y+SUPERCLUSTERSIZE-1);
			u16 z1=max(clusterCoord.z-1,0),z2=min(clusterCoord.z+2,m->clusterSize.z);
			NOGBA("hoho : %d %d %d %d vs %d %d",x1,x2,y1,y2,clusterCoord.x,clusterCoord.y);
			for(x=x1;x<x2;x++)
			{
				for(y=y1;y<y2;y++)
				{
					for(z=z1;z<z2;z++)
					{
						{
							lightsourceList_struct* ql=&m->superCluster[x-m->offset.x][y-m->offset.y]->cluster[z-m->offset.z].lightList;
							lightsource_struct* oq=ql->first;
							// void** pq=&ql->first;
							s8 i2=(i-CLUSTERSIZE*x), j2=(j-CLUSTERSIZE*y), k2=(k-CLUSTERSIZE*z);
							// while(q)
							// {
								// if(q->i==i2 && q->j==j2 && q->k==k2)
								// {
									// *pq=q->next;
									// releaseLight(&q);
									// ql->count--;
									// q=*pq;
								// }else{
									// pq=&q->next;
									// q=q->next;
								// }
							// }
							lightsource_struct* q;
							if(oq)q=oq->next;
							else q=NULL;
							while(q)
							{
								if(q->i==i2 && q->j==j2 && q->k==k2)
								{
									oq->next=q->next;
									releaseLight(&q);
									ql->count--;
									q=oq->next;
								}else{
									oq=q;
									q=q->next;
								}
							}
							q=ql->first;
							if(q && q->i==i2 && q->j==j2 && q->k==k2)
							{
								ql->first=q->next;
								// ql->count--;
								ql->count=(((ql->count&127)-1)&127)|(ql->count&128);
								releaseLight(&q);
							}
						}
						{
							quadList_struct* ql=&m->superCluster[x-m->offset.x][y-m->offset.y]->cluster[z-m->offset.z].quadList;
							quad_struct* q=ql->first;
							while(q)
							{
								int i2=i-(x*CLUSTERSIZE+imIDtable[q->mID])+8,j2=j-(y*CLUSTERSIZE+jmIDtable[q->mID])+8,k2=k-(z*CLUSTERSIZE+kmIDtable[q->mID])+8;
								// if(i2>0 && j2>0 && k2>0 && i2<16 && j2<16 && k2<16)
								{
									// q->light+=max(31-((i-i2)*(i-i2)+(j-j2)*(j-j2)+(k-k2)*(k-k2)),0);
									// q->light=max((q->light&127)-lightTable[i2*8+j2*8*16+k2*8*16*16+q->direction],0)|(q->light&(1<<7));
									q->light=max((q->light&127)-lightTable[i2+j2*16+k2*16*16+(q->direction<<12)],0)|(q->light&(1<<7));
								}
								q=q->next;
							}
						}
						{
							quadList_struct* ql=&m->superCluster[x-m->offset.x][y-m->offset.y]->cluster[z-m->offset.z].specialList;
							quad_struct* q=ql->first;
							while(q)
							{
								int i2=i-(x*CLUSTERSIZE+imIDtable[q->mID])+8,j2=j-(y*CLUSTERSIZE+jmIDtable[q->mID])+8,k2=k-(z*CLUSTERSIZE+kmIDtable[q->mID])+8;
								// if(i2>0 && j2>0 && k2>0 && i2<13 && j2<13 && k2<13)
								{
									// q->light+=max(31-((i-i2)*(i-i2)+(j-j2)*(j-j2)+(k-k2)*(k-k2)),0);
									// q->light=max((q->light&127)-lightTable[i2*8+j2*8*16+k2*8*16*16+q->direction],0)|(q->light&(1<<7));
									q->light=max((q->light&127)-lightTable[i2+j2*16+k2*16*16+(q->direction<<12)],0)|(q->light&(1<<7));
								}
								q=q->next;
							}
						}
					}
				}
			}
			PROF_END(TESTVALUE);
		}else{
			u8 minim=min(min(waterm(*getBlockP(m, i-1, j, k)),waterm((*getBlockP(m, i+1, j, k)))),min(waterm((*getBlockP(m, i, j-1, k))),waterm((*getBlockP(m, i, j+1, k)))));
			if((*getBlockP(m, i-1, j, k))==minim)addWater(m, i-1, j, k, (*getBlockP(m, i-1, j, k)));
			else if((*getBlockP(m, i+1, j, k))==minim)addWater(m, i+1, j, k, (*getBlockP(m, i+1, j, k)));
			else if((*getBlockP(m, i, j-1, k))==minim)addWater(m, i, j-1, k, (*getBlockP(m, i, j-1, k)));
			else if((*getBlockP(m, i, j+1, k))==minim)addWater(m, i, j+1, k, (*getBlockP(m, i, j+1, k)));
			else if((*getBlockP(m, i, j, k+1))>=WATERTYPE)addWater(m, i, j, k+1, (*getBlockP(m, i, j, k+1)));
		}
		if(oldt!=*t)
		{
			NOGBA("lalala : %d %d",oldt,*t);
			int l;
			for(l=*t;l<=oldt;l++)
			{
				adjustBlockLight(m, i, j, l);
				adjustBlockLight(m, i+1, j, l);
				adjustBlockLight(m, i-1, j, l);
				adjustBlockLight(m, i, j+1, l);
				adjustBlockLight(m, i, j-1, l);
			}
		}
	}
	else if(!solid(ot))
	{
		if(k>(*t) && !seeThrough(type))(*t)=k;
		if(type==13)
		{
			vect3D clusterCoord=getCluster(m,i,j,k);
			quadList_struct* ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].quadList;
			(*getBlockP(m, i, j, k))=type;
			addQuad(ql, m, 6, 31, 0, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);
			addQuad(ql, m, 7, 31, 0, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);
			addQuad(ql, m, 8, 31, 0, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);
			addQuad(ql, m, 9, 31, 0, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);
			
			int cn=0;
			PROF_START();
			processLight(m,i,j,k,clusterCoord);
			PROF_END(TESTVALUE);
		}else{
			removeBlock(m, i, j, k, false);
			(*getBlockP(m, i, j, k))=type;
			addBlock(m, i, j, k);
			if(i<m->size.x-1 && block(*getBlockP(m,i+1,j,k)))
			{
				removeBlock(m, i+1, j, k, false);
				addBlock(m, i+1, j, k);
			}
			if(i>0 && block(*getBlockP(m,i-1,j,k)))
			{
				removeBlock(m, i-1, j, k, false);
				addBlock(m, i-1, j, k);
			}
			if(j<m->size.y-1 && block(*getBlockP(m,i,j+1,k)))
			{
				removeBlock(m, i, j+1, k, false);
				addBlock(m, i, j+1, k);
			}
			if(j>0 && block(*getBlockP(m,i,j-1,k)))
			{
				removeBlock(m, i, j-1, k, false);
				addBlock(m, i, j-1, k);
			}
			if(k<m->size.z-1 && block(*getBlockP(m,i,j,k+1)))
			{
				removeBlock(m, i, j, k+1, false);
				addBlock(m, i, j, k+1);
			}
			if(k>0 && block(*getBlockP(m,i,j,k-1)))
			{
				removeBlock(m, i, j, k-1, false);
				addBlock(m, i, j, k-1);
			}
		}
		if(oldt!=*t)
		{
			int l;
			for(l=oldt;l<=*t;l++)
			{
				adjustBlockLight(m, i, j, l);
				adjustBlockLight(m, i+1, j, l);
				adjustBlockLight(m, i-1, j, l);
				adjustBlockLight(m, i, j+1, l);
				adjustBlockLight(m, i, j-1, l);
			}
		}
	}
	renderClusterList(m, clusterCoord.x, clusterCoord.y, clusterCoord.z);
	m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->changed=1;
	#ifdef DEBUGMODE
	iprintf("\ncount : %d   ",m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z].quadList.count);
	#endif
	// while(!(keysDown() & KEY_A))scanKeys();
}

void addBlock(map_struct* m, int i, int j, int k)
{
	vect3D clusterCoord=getCluster(m,i,j,k);
	quadList_struct* ql;
	const u8 d=(*getBlockP(m,i,j,k));
	if(!d)return;
	else if(d>=WATERTYPE)ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].specialList;
	else ql=&m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->cluster[clusterCoord.z-m->offset.z].quadList;
	
	u32 bid=(i)+(j)*(m)->size.x+(k)*(m)->size.y*(m)->size.x;
	u8 light=0;
	surface(m, i, j, k, &light);
	if(transparent2(m,i,j,k,i,j,k-1))
	{
		getLight(m, i, j, k, &light, 1);
		addQuad(ql, m, 1, light, bid, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);
	}
	if(transparent2(m,i,j,k,i,j,k+1))
	{
		getLight(m, i, j, k, &light, 0);
		addQuad(ql, m, 0, light, bid, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);
	}
	
	if(transparent2(m,i,j,k,i-1,j,k))
	{
		getLight(m, i, j, k, &light, 3);
		addQuad(ql, m, 3, light, bid, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);
	}
	if(transparent2(m,i,j,k,i+1,j,k))
	{
		getLight(m, i, j, k, &light, 2);
		addQuad(ql, m, 2, light, bid, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);
	}
	
	if(transparent2(m,i,j,k,i,j-1,k))
	{
		getLight(m, i, j, k, &light, 5);
		addQuad(ql, m, 5, light, bid, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);		
	}
	if(transparent2(m,i,j,k,i,j+1,k))
	{
		getLight(m, i, j, k, &light, 4);
		addQuad(ql, m, 4, light, bid, m->superCluster[clusterCoord.x-m->offset.x][clusterCoord.y-m->offset.y]->data, i, j, k);
	}
}

void precalcCollumn(map_struct* m, int x, int y, u8* t)
{
	int i, j, k;
	int x2=x-m->offset.x, y2=y-m->offset.y;
	x*=CLUSTERSIZE;
	y*=CLUSTERSIZE;
	for(i=x;i<x+CLUSTERSIZE;i++)
	{	
		for(j=y;j<y+CLUSTERSIZE;j++)
		{	
			for(k=0;k<16*CLUSTERSIZE;k++)
			{
				if(!*getBlockPE(m,i,j,k))t[(i-x)+(j-y)*CLUSTERSIZE+k*CLUSTERSIZE*CLUSTERSIZE]=0;
				else if(isDoor(*getBlockPE(m,i,j,k)) || isLadder(*getBlockPE(m,i,j,k)))t[(i-x)+(j-y)*CLUSTERSIZE+k*CLUSTERSIZE*CLUSTERSIZE]=1;
				// else {t[(i-x)+(j-y)*CLUSTERSIZE+k*CLUSTERSIZE*CLUSTERSIZE]=(((!*getBlockPE(m,i,j,k-1))&1))
																// |((((!*getBlockPE(m,i,j,k+1))&1)<<1))
																// |((((!*getBlockPE(m,i,j-1,k))&1)<<2))
																// |((((!*getBlockPE(m,i,j+1,k))&1)<<3))
																// |((((!*getBlockPE(m,i-1,j,k))&1)<<4))
																// |((((!*getBlockPE(m,i+1,j,k))&1)<<5));
				else {t[(i-x)+(j-y)*CLUSTERSIZE+k*CLUSTERSIZE*CLUSTERSIZE]=(((transparent3(m,i,j,k,i,j,k-1))&1))
																|((((transparent3(m,i,j,k,i,j,k+1))&1)<<1))
																|((((transparent3(m,i,j,k,i,j-1,k))&1)<<2))
																|((((transparent3(m,i,j,k,i,j+1,k))&1)<<3))
																|((((transparent3(m,i,j,k,i-1,j,k))&1)<<4))
																|((((transparent3(m,i,j,k,i+1,j,k))&1)<<5));
					// NOGBA("L : %d %d %d : %d (%d %d %d %d %d %d)",(i-x),(j-y),k,t[(i-x)+(j-y)*CLUSTERSIZE+k*CLUSTERSIZE*CLUSTERSIZE],
					// !*getBlockP(m,i,j,k-1),!*getBlockP(m,i,j,k+1),!*getBlockP(m,i-1,j,k),!*getBlockP(m,i+1,j,k),!*getBlockP(m,i,j-1,k),!*getBlockP(m,i,j+1,k));
					}
				// bool ls=(k==getHighest(m, i, j)
						// || k>getHighest(m, i+1, j)
						// || k>getHighest(m, i-1, j)
						// || k>getHighest(m, i, j+1)
						// || k>getHighest(m, i, j-1));
				char ls=(k==getHighest(m, i, j));
				if(!ls && i<m->size.x-1)ls=k>getHighest(m, i+1, j);
				if(!ls && j<m->size.y-1)ls=k>getHighest(m, i, j+1);
				if(!ls && i>0)ls=k>getHighest(m, i-1, j);
				if(!ls && j>0)ls=k>getHighest(m, i, j-1);
				t[(i-x)+(j-y)*CLUSTERSIZE+k*CLUSTERSIZE*CLUSTERSIZE]|=((ls&1)<<6);
			}
		}
	}
	for(i=0;i<16;i++)
	{
		bool d1=m->superCluster[x2][y2]->cluster[i].wall&1, d2=(m->superCluster[x2][y2]->cluster[i].wall>>1)&1, d3=(m->superCluster[x2][y2]->cluster[i].wall>>2)&1;
		// NOGBA("%d,%d,%d : %d %d %d",x2,y2,i,d1,d2,d3); 
		t[CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*i]|=(d1<<7);
		t[CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*i+1]|=(d2<<7);
		t[CLUSTERSIZE*CLUSTERSIZE*CLUSTERSIZE*i+2]|=(d3<<7);
	}
}

void setFog(u8 mode)
{
	fogMode=mode;
	int i;
	if(mode)
	{
		glEnable(GL_FOG);
		glFogShift(2);
		glFogColor(0,0,0,31);
		// for(i=0;i<16;i++)glFogDensity(i,96);
		// for(i=16;i<32;i++)glFogDensity(i,96+(i-16)*2);
		// for(i=0;i<16;i++)glFogDensity(i,64);
		// for(i=16;i<32;i++)glFogDensity(i,64+(i-16)*4);
		for(i=0;i<32;i++)glFogDensity(i,(i*36)/10);
		// glFogDensity(31,127);
		glFogOffset(0x6100);
	}else{
		//TEST TEST TEST
		glEnable(GL_FOG);
		glFogShift(2);
		// glFogColor(0,0,0,0);
		glFogColor(8,19,21,31);//water
		// for(i=0;i<32;i++)glFogDensity(i,i*4);
		for(i=0;i<32;i++)glFogDensity(i,127);
		for(i=0;i<31;i++)glFogDensity(i,(i+1)*4);//water
		glFogOffset(0x6500);//water
		//TEST TEST TEST
	}
}

void generateQuads(map_struct* m, clusterColumn_struct* cC, u8* t, u16 x, u16 y)
{
	int i, j, k;
	
	// NOGBA("GENERATING %d %d",x,y);
	
	quadList_struct* ql;
	cluster_struct* c=cC->cluster;
	u32 bid;
	c--;
	x*=CLUSTERSIZE;
	y*=CLUSTERSIZE;
	u8* d=cC->data;
	// u8* h=cC->highest;
	for(i=0;i<CLUSTERSIZE*CLUSTERSIZE;i++)cC->highest[i]=0;
	for(k=0;k<CLUSTERSIZE*16;k++)
	{
		if(!(k%CLUSTERSIZE))
		{
			c++;
			{
				quad_struct *oq=NULL, *q=c->quadList.first;
				while(q)
				{
					oq=q;
					q=q->next;
					// free(oq);
					releaseQuad(&oq);
				}//optimisable
				// c->quadList.first=NULL;c->quadList.last=NULL;c->quadList.count=0;
				c->quadList.first=NULL;c->quadList.count=0;
			}
			{
				quad_struct *oq=NULL, *q=c->specialList.first;
				while(q)
				{
					oq=q;
					q=q->next;
					releaseQuad(&oq);
					// free(oq);
				}//optimisable
				// c->specialList.first=NULL;c->specialList.last=NULL;c->specialList.count=0;
				c->specialList.first=NULL;c->specialList.count=0;
			}
			{
				lightsource_struct *oq=NULL, *q=c->lightList.first;
				while(q)
				{
					oq=q;
					q=q->next;
					// free(oq);
					releaseLight(&oq);
				}//optimisable
				// c->lightList.first=NULL;c->lightList.last=NULL;c->lightList.count=0;
				c->lightList.first=NULL;c->lightList.count=0;
				#ifdef FOGLIGHT
					c->lightList.count=1<<7;
				#endif
			}
			c->wall=(((*t)>>7)&1)|((((*(t+1))>>7)&1)<<1)|((((*(t+2))>>7)&1)<<2);
		}
		u8* h=cC->highest;
		for(j=0;j<CLUSTERSIZE;j++)
		{
			// bid=(x)+(y+j)*(m)->size.x+(k)*(m)->size.y*(m)->size.x;
			for(i=0;i<CLUSTERSIZE;i++)
			{
				// u8* d=&t[i+j*CLUSTERSIZE+k*CLUSTERSIZE*CLUSTERSIZE];
				// u8* d=t;
				if((*t)&63)
				{
					// bid=(x+i)+(y+j)*(m)->size.x+(k)*(m)->size.y*(m)->size.x;
					// cC->highest[i+j*CLUSTERSIZE]=max(cC->highest[i+j*CLUSTERSIZE],k);
					if(!seeThrough(*d))*h=k;
					if(*d>=WATERTYPE)
					{
						ql=&c->specialList;
						if(((*t)&63)!=2 || *d>WATERTYPE)addWater(m, x+i, y+j, k, *d);//à optimisay //pas sûr que la nouvelle condition corrige tout
					}else ql=&c->quadList;
					if(*d>=LADDERTYPE && *d<LADDERTYPE+4+16)
					{
						if(*d<LADDERTYPE+4)
						{
							u8 dir;
							switch(*d+2-LADDERTYPE)
							{
								case 2:
									dir=8+3;
									break;
								case 3:
									dir=8+2;
									break;
								case 4:
									dir=8+5;
									break;
								case 5:
									dir=8+4;
									break;
							}
							const u8 light=((((*t)>>6)&1)<<7);
							addQuad(ql, m, dir, light, bid, cC->data, x+i, y+j, k);
						}else if(*d<DOORTYPE+8){
							const u8 dir=(*d-DOORTYPE)/2+2;
							const u8 light=((((*t)>>6)&1)<<7);
							addQuad(ql, m, dir, light, bid, cC->data, x+i, y+j, k);
							addQuad(ql, m, dir+8, light, bid, cC->data, x+i, y+j, k);
						}else{
							u8 dir=(*d-DOORTYPE)/2+2;
							switch(dir)
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
							const u8 light=((((*t)>>6)&1)<<7);
							addQuad(ql, m, dir, light, bid, cC->data, x+i, y+j, k);
							addQuad(ql, m, dir+8, light, bid, cC->data, x+i, y+j, k);
						}
					}else if(*d==13)
					{
						addLightProcess(x+i, y+j, k);
						const u8 light=31;
						addQuad(ql,m, 6, light, bid, cC->data, x+i, y+j, k);
						addQuad(ql,m, 7, light, bid, cC->data, x+i, y+j, k);
						addQuad(ql,m, 8, light, bid, cC->data, x+i, y+j, k);
						addQuad(ql,m, 9, light, bid, cC->data, x+i, y+j, k);
					}else{
						// NOGBA("%d",*t);
						const u8 light=((((*t)>>6)&1)<<7);
						if((*t)&1){addQuad(ql,m, 1, light, bid, cC->data, x+i, y+j, k);}
						if(((*t)>>1)&1){addQuad(ql,m, 0, light, bid, cC->data, x+i, y+j, k);}
						if(((*t)>>2)&1){addQuad(ql,m, 5, light, bid, cC->data, x+i, y+j, k);}
						if(((*t)>>3)&1){addQuad(ql,m, 4, light, bid, cC->data, x+i, y+j, k);}
						if(((*t)>>4)&1){addQuad(ql,m, 3, light, bid, cC->data, x+i, y+j, k);}
						if(((*t)>>5)&1){addQuad(ql,m, 2, light, bid, cC->data, x+i, y+j, k);}
						#ifdef FOGLIGHT
							c->lightList.count&=light;
						#endif
					}
				}
				// NOGBA("K : %d %d %d : %d",(i),(j),k,*d);
				t++;
				d++;
				h++;
				// bid++;
			}
		}
	}
}

void translateSuperCluster(map_struct* m, u8 dir)
{
	int i, j;
	
	clusterColumn_struct* temp[32];
	
	switch(dir)
	{
		case 0:
			for(j=0;j<32;j++)
			{
				if(m->superCluster[30][j]->changed){globalSaveMap(m);break;}
			}
			for(j=0;j<32;j++)
			{
				temp[j]=m->transitionCluster[j+32];
				m->transitionCluster[j+32]=m->superCluster[31][j];
			}
			for(i=32-1;i>0;i--)
			{
				for(j=0;j<32;j++)
				{
					m->superCluster[i][j]=m->superCluster[i-1][j];
				}
			}
			for(j=0;j<32;j++)
			{
				m->superCluster[0][j]=m->transitionCluster[j];
				m->transitionCluster[j]=temp[j];
			}
			m->offset.x--;
			// if(m->offset.x > 0)m->transitioning[0]=34;
			m->transitioning[0]=34;
			break;
		case 1:
			for(j=0;j<32;j++)
			{
				if(m->superCluster[1][j]->changed){globalSaveMap(m);break;}
			}
			for(j=0;j<32;j++)
			{
				temp[j]=m->transitionCluster[j];
				m->transitionCluster[j]=m->superCluster[0][j];
			}
			for(i=0;i<32-1;i++)
			{
				for(j=0;j<32;j++)
				{
					m->superCluster[i][j]=m->superCluster[i+1][j];
				}
			}
			for(j=0;j<32;j++)
			{
				m->superCluster[31][j]=m->transitionCluster[j+32];
				m->transitionCluster[j+32]=temp[j];
				// fseek(m->fileHandle,((sizeof(cluster_struct)*16+(4*4*4)*16))*(32+m->offset.x)+((sizeof(cluster_struct)*16+(4*4*4)*16)*(32*2))*(j),SEEK_SET);
				// readClusterColumn(m, 32, j, m->transitionCluster[j+32], m->transitionStuff, m->fileHandle);
			}
			m->offset.x++;
			// if(m->offset.x < m->clusterSize.x-SUPERCLUSTERSIZE/*-SUPERCLUSTERSIZE/2*/)m->transitioning[1]=34;
			m->transitioning[1]=34;
			break;
		case 2:
			for(j=0;j<32;j++)
			{
				if(m->superCluster[j][30]->changed){globalSaveMap(m);break;}
			}
			for(j=0;j<32;j++)
			{
				temp[j]=m->transitionCluster[j+32*3];
				m->transitionCluster[j+32*3]=m->superCluster[j][31];
			}
			for(i=32-1;i>0;i--)
			{
				for(j=0;j<32;j++)
				{
					m->superCluster[j][i]=m->superCluster[j][i-1];
				}
			}
			for(j=0;j<32;j++)
			{
				m->superCluster[j][0]=m->transitionCluster[j+32*2];
				m->transitionCluster[j+32*2]=temp[j];
			}
			m->offset.y--;
			// if(m->offset.y > 0)m->transitioning[2]=34;
			m->transitioning[2]=34;
			break;
		case 3:
			for(j=0;j<32;j++)
			{
				if(m->superCluster[j][1]->changed){globalSaveMap(m);break;}
			}
			for(j=0;j<32;j++)
			{
				temp[j]=m->transitionCluster[j+32*2];
				m->transitionCluster[j+32*2]=m->superCluster[j][0];
			}
			for(i=0;i<32-1;i++)
			{
				for(j=0;j<32;j++)
				{
					m->superCluster[j][i]=m->superCluster[j][i+1];
				}
			}
			for(j=0;j<32;j++)
			{
				m->superCluster[j][31]=m->transitionCluster[j+32*3];
				m->transitionCluster[j+32*3]=temp[j];
			}
			m->offset.y++;
			// if(m->offset.y < m->clusterSize.y-SUPERCLUSTERSIZE/*-SUPERCLUSTERSIZE/2*/)m->transitioning[3]=34;
			m->transitioning[3]=34;
			break;
	}
	// m->offset.y--;
}

void initMap(map_struct* m, vect3D clusterSize)
{
	int i;//, j, k;

	cull=true;
	
	initWater();
	initUVmap();
	initXYmap();
	initLightMap();
	initmIDTables();
	initLightTable();
	initDegradTable();
	initLightCache();
	initQuadCache();
	
	//DEBUG
	initStats(&frameTime);
	initStats(&streamRead);
	initStats(&streamCalc);
	initStats(&columnWrite);
	
	cullMagic=1;
	m->clusterSize=clusterSize;
	m->size.x=clusterSize.x*CLUSTERSIZE;m->size.y=clusterSize.y*CLUSTERSIZE;m->size.z=clusterSize.z*CLUSTERSIZE;
	// m->data=malloc(m->size.x*m->size.y*m->size.z);
	iprintf("RAM after data :\n%dko used, %dko free    \n",DS_UsedMem()/1024,DS_FreeMem()/1024);
	NOGBA("RAM after data : %dko used, %dko free    \n",DS_UsedMem()/1024,DS_FreeMem()/1024);
	// m->clusterz=malloc(m->clusterSize.x*m->clusterSize.y*m->clusterSize.z*sizeof(cluster_struct));
	// for(i=0;i<m->clusterSize.x*m->clusterSize.y*m->clusterSize.z;i++){m->clusterz[i].quadList.first=NULL;m->clusterz[i].quadList.last=NULL;m->clusterz[i].quadList.count=0;/*m->cluster[i].draw=0;*/m->clusterDraw[i]=0;}
	initSuperCluster(m);
	for(i=0;i<SUPERCLUSTERSIZE*SUPERCLUSTERSIZE*m->clusterSize.z;i++){m->clusterDraw[i]=0;}
	for(i=0;i<SUPERCLUSTERSIZE*SUPERCLUSTERSIZE*m->clusterSize.z;i++){m->clusterDrawn[i]=0;}
	m->transitioning[0]=0;
	m->transitioning[1]=0;
	m->transitioning[2]=0;
	m->transitioning[3]=0;
	iprintf("RAM after superclusters :\n%dko used, %dko free    \n",DS_UsedMem()/1024,DS_FreeMem()/1024);
	NOGBA("RAM after superclusters : %dko used, %dko free    \n",DS_UsedMem()/1024,DS_FreeMem()/1024);
	// m->quadList.first=NULL;
	// m->quadList.last=NULL;
	// openList.first=NULL;
	// openList.last=&(openList.first);
	// closedList.first=NULL;
	// closedList.last=&(closedList.first);
	openList.size=0;
	closedList.size=0;
	
	testCursor=0;
	
	// for(i=0;i<m->size.x*m->size.y*m->size.z;i++)m->data[i]=0;
	
	// for(i=0;i<SUPERCLUSTERSIZE*CLUSTERSIZE;i++)for(j=0;j<SUPERCLUSTERSIZE*CLUSTERSIZE;j++)for(k=0;k<m->size.z;k++)(*getBlockP(m,i,j,k))=0;
	
	// NOGBA("size : %d %d %d (%p)",m->size.x,m->size.y,m->size.z,m->data);
}

void initClusterColumn(clusterColumn_struct* cC)
{
	int k;
	for(k=0;k<16;k++)
	{
		cC->cluster[k].quadList.first=NULL;
		// cC->cluster[k].quadList.last=NULL;
		cC->cluster[k].quadList.count=0;
		cC->cluster[k].specialList.first=NULL;
		// cC->cluster[k].specialList.last=NULL;
		cC->cluster[k].specialList.count=0;
		cC->cluster[k].lightList.first=NULL;
		// cC->cluster[k].lightList.last=NULL;
		cC->cluster[k].lightList.count=0;
	}
}

void initSuperCluster(map_struct* m)
{
	int i, j;
	m->offset=(vect3D){0,0,0};
	for(i=0;i<SUPERCLUSTERSIZE;i++)
	{
		for(j=0;j<SUPERCLUSTERSIZE;j++)
		{
			m->superCluster[i][j]=malloc(sizeof(clusterColumn_struct));
			initClusterColumn(m->superCluster[i][j]);
		}
	}
}

void freeMap(map_struct* m)
{
	int i, j;
	if(openMap==openMapNOCASH)fclose(m->fileHandle);
	else{
		if(m->fileMap)free(m->fileMap);
		if(m->headerMap)free(m->headerMap);
		if(m->fileHandle)free(m->fileHandle);
	}
	for(i=0;i<SUPERCLUSTERSIZE;i++)
	{
		free(m->transitionCluster[i]);
		free(m->transitionCluster[i+32]);
		free(m->transitionCluster[i+32*2]);
		free(m->transitionCluster[i+32*3]);
		for(j=0;j<SUPERCLUSTERSIZE;j++)
		{
			free(m->superCluster[i][j]);
		}
	}
	if(m->header)free(m->header);
}

void createTestMap(map_struct* m)
{
	int i, j, k;
	// for(i=0;i<m->size.x*m->size.y*m->size.z;i++)m->data[i]=0;
	for(i=0;i<m->size.x;i++)for(j=0;j<m->size.y;j++)for(k=0;k<m->size.z;k++)(*getBlockP(m,i,j,k))=0;
	// for(i=0;i<m->size.x;i++)
	// {
		// for(j=0;j<m->size.y;j++)
		// {
			// for(k=0;k<m->size.z;k++)
			// {
				// if(k<3)m->data[i+j*m->size.x+k*m->size.y*m->size.x]=5; 
				// else if(k<32)m->data[i+j*m->size.x+k*m->size.y*m->size.x]=3; 
				// else if(k<35)m->data[i+j*m->size.x+k*m->size.y*m->size.x]=1; 
				// else m->data[i+j*m->size.x+k*m->size.y*m->size.x]=0;
			// }
		// }
	// }
	
	iprintf("\ngenerating map...");
	
	int x8, y8, i8, j8, i2, j2;
	x8=m->size.x/8;
	y8=m->size.y/8;
	u8 height[x8+1][y8+1];
	
	for(i8=0;i8<=x8;i8++)
	{
		for(j8=0;j8<=y8;j8++)
		{
			height[i8][j8]=(rand()%12)+26;
		}
	}
	int n;
	u8 tree;
	for(i8=0;i8<x8;i8++)
	{
		for(j8=0;j8<y8;j8++)
		{
			n=0;
			for(i2=0;i2<8;i2++)
			{
				for(j2=0;j2<8;j2++)
				{
					i=i2+i8*8;
					j=j2+j8*8;
					n=(i2+j2*8)*5;
					u8 v=((((degradTable[n]*height[i8][j8]))+((degradTable[n+1]*height[i8+1][j8]))+((degradTable[n+2]*height[i8+1][j8+1]))+((degradTable[n+3]*height[i8][j8+1])))*degradTable[n+4])>>14;
					tree=!(rand()%500);
					if(tree)tree=3+rand()%3;
					// NOGBA("v : %d",v);
					for(k=0;k<m->size.z;k++)
					{
						if(k<3)(*getBlockP(m,i,j,k))=5;
						else if(k<v-3)(*getBlockP(m,i,j,k))=3;
						else if(k<v)(*getBlockP(m,i,j,k))=1;
						else if(tree && k-v<tree)(*getBlockP(m,i,j,k))=8;
						else if(tree && k-v<tree+2){(*getBlockP(m,i,j,k))=10;
													(*getBlockP(m,i+1,j,k))=10;
													(*getBlockP(m,i-1,j,k))=10;
													(*getBlockP(m,i,j+1,k))=10;
													(*getBlockP(m,i,j-1,k))=10;
													(*getBlockP(m,i,j,k+1))=10;}
						else if((*getBlockP(m,i,j,k))!=10)(*getBlockP(m,i,j,k))=0;
					}
					// n+=5;
				}
			}
		}
	}
	
	iprintf("done !\nprocessing data...");
}

void generateTestMap(map_struct* m) //NOW INVALID, SORRY. (requires blank file generation for writeClusterColumn to work)
{
	/*int i, j, k;
	
	iprintf("\ngenerating basic terrain...");
	
	int x8, y8, z8, i8, j8, i2, j2;
	x8=1024/8;
	y8=1024/8;
	// u8 height[x8+1][y8+1];
	u8 height[129][129];
	
	for(i8=0;i8<=x8;i8++)
	{
		iprintf("%d,",i8);
		for(j8=0;j8<=y8;j8++)
		{
			height[i8][j8]=(rand()%12)+26;
		}
	}
	int n,l=0,o,p;
	u8 tree;
	FILE*f=fopen("fat:/testmap.map","wb+");
	for(o=0;o<8;o++)
	{
		for(p=0;p<8;p++)
		{
			iprintf("done\ngenerating supercluster %d...",l);
			for(i=0;i<m->size.x;i++)for(j=0;j<m->size.y;j++)for(k=0;k<m->size.z;k++)(*getBlockP(m,i,j,k))=0;
			for(i8=o*16;i8<16+o*16;i8++)
			{
				for(j8=0+p*16;j8<16+p*16;j8++)
				{
					n=0;
					for(i2=0;i2<8;i2++)
					{
						for(j2=0;j2<8;j2++)
						{
							i=i2+i8*8-o*128;
							j=j2+j8*8-p*128;
							n=(i2+j2*8)*5;
							u8 v=((((degradTable[n]*height[i8][j8]))+((degradTable[n+1]*height[i8+1][j8]))+((degradTable[n+2]*height[i8+1][j8+1]))+((degradTable[n+3]*height[i8][j8+1])))*degradTable[n+4])>>14;
							tree=!(rand()%500);
							if(tree)tree=3+rand()%3;
							// NOGBA("v : %d",v);
							for(k=0;k<m->size.z;k++)
							{
								if(k<3)(*getBlockP(m,i,j,k))=5;
								else if(k<v-3)(*getBlockP(m,i,j,k))=3;
								else if(k<v)(*getBlockP(m,i,j,k))=1;
								else if(tree && k-v<tree)(*getBlockP(m,i,j,k))=8;
								else if(tree && k-v<tree+2){(*getBlockP(m,i,j,k))=10;
															(*getBlockP(m,i+1,j,k))=10;
															(*getBlockP(m,i-1,j,k))=10;
															(*getBlockP(m,i,j+1,k))=10;
															(*getBlockP(m,i,j-1,k))=10;
															(*getBlockP(m,i,j,k+1))=10;}
								else if((*getBlockP(m,i,j,k))!=10)(*getBlockP(m,i,j,k))=0;
							}
							// n+=5;
						}
					}
				}
			}
			iprintf("done\nwriting supercluster %d...",l);
			for(j8=0;j8<32;j8++)
			{
				// fseek(f,((sizeof(cluster_struct)*16+(4*4*4)*16))*(o*32)+((sizeof(cluster_struct)*16+(4*4*4)*16)*(32*2))*(j8+(p*32)),SEEK_SET);
				fseek(f,(((4*4*4)*16+(4*4*4)*16))*(o*32)+(((4*4*4)*16+(4*4*4)*16)*(32*8))*(j8+(p*32)),SEEK_SET);
				for(i8=0;i8<32;i8++)
				{
					for(k=0;k<16;k++)renderClusterList(m, i8, j8, k);
					writeClusterColumn(m, i8, j8, m->superCluster[i8][j8], m->transitionStuff, f);
				}			
			}
			l++;
		}
	}
	fclose(f);
	
	iprintf("done !\nprocessing data...");*/
}

FILE_POSITION _FAT_getPosition(u32* pos);
uint32_t _FAT_setPosition(FILE_POSITION np, uint32_t pos);

static inline sec_t clusterToSector(PARTITION* partition, uint32_t cluster){
	return (cluster >= CLUSTER_FIRST) ? 
		((cluster - CLUSTER_FIRST) * (sec_t)partition->sectorsPerCluster) + partition->dataStart : 
		partition->rootDirStart;
}
// FILE_STRUCT* fZ;

void openMap2048(char* filename, map_struct* m)
{
	// chdir("fat:/");
	m->fileHandle=(void*)sOpen(filename);
	FILE_STRUCT* fZ=(FILE_STRUCT*)m->fileHandle;
	TESTVALUE3=fZ->partition->sectorsPerCluster;
	switch(fZ->partition->sectorsPerCluster)
	{
		case 1:
			openMap=&openMap512;
			readClusterColumn=&readClusterColumn512;
			writeClusterColumn=&writeClusterColumn512;
			openMap(filename,m);
			fsFormat=3;
			return;
			break;
		case 2:
			openMap=&openMap1024;
			readClusterColumn=&readClusterColumn1024;
			writeClusterColumn=&writeClusterColumn1024;
			fsFormat=2;
			openMap(filename,m);
			return;
			break;
	}
	m->header=malloc(2048);
	u32 poZ;
	sSeek(fZ,0,SEEK_SET);
	m->headerMap=malloc(sizeof(u32));
	FILE_POSITION fpZ=_FAT_getPosition(&poZ);
	*(m->headerMap)=_FAT_fat_clusterToSector(fZ->partition,fpZ.cluster)+fpZ.sector;
	sRead(fZ, (char*)m->header, 2048);
	m->clusterSize=(vect3D){m->header->sizeX,m->header->sizeY,16};
	
	if(m->header->magicVersionNumber!=VERSIONMAGIC)
	{
		m->header->spawnX=64;
		m->header->spawnY=64;
		m->header->spawnZ=0;
	}
	
	m->fileMap=malloc(sizeof(u32)*m->clusterSize.x*m->clusterSize.y);
	int i, j;
	sSeek(fZ,2048,SEEK_SET);//test
	for(j=0;j<m->clusterSize.y;j++)
	{
		for(i=0;i<m->clusterSize.x;i++)
		{
			// u32 po=2048+(((4*4*4)*16+(4*4*4)*16))*(i)+(((4*4*4)*16+(4*4*4)*16)*(m->clusterSize.x))*(j);
			u32 po;
			// sSeek(fZ,po,SEEK_SET);
			FILE_POSITION fp=_FAT_getPosition(&po);
			// m->fileMap[i+j*m->clusterSize.x]=fZ->partition->dataStart+fp.cluster*fZ->partition->sectorsPerCluster+fp.sector;
			m->fileMap[i+j*m->clusterSize.x]=_FAT_fat_clusterToSector(fZ->partition,fp.cluster)+fp.sector;
			sSeek(fZ,(4*4*4)*16+(4*4*4)*16,SEEK_CUR);//test
		}
	}
}

void openMap1024(char* filename, map_struct* m)
{
	// chdir("fat:/");
	m->fileHandle=(void*)sOpen(filename);
	FILE_STRUCT* fZ=(FILE_STRUCT*)m->fileHandle;
	m->header=malloc(2048);
	sRead(fZ, (char*)m->header, 2048);
	u32 poZ;
	sSeek(fZ,0,SEEK_SET);
	m->headerMap=malloc(sizeof(u32)*2);
	FILE_POSITION fpZ=_FAT_getPosition(&poZ);
	m->headerMap[0]=_FAT_fat_clusterToSector(fZ->partition,fpZ.cluster)+fpZ.sector;
	sSeek(fZ,1024,SEEK_SET);
	fpZ=_FAT_getPosition(&poZ);
	m->headerMap[1]=_FAT_fat_clusterToSector(fZ->partition,fpZ.cluster)+fpZ.sector;
	if(m->header->magicVersionNumber!=VERSIONMAGIC)
	{
		m->header->spawnX=64;
		m->header->spawnY=64;
	}
	m->clusterSize=(vect3D){m->header->sizeX,m->header->sizeY,16};
	m->fileMap=malloc(sizeof(u32)*m->clusterSize.x*m->clusterSize.y*2);
	int i, j;
	/*for(i=0;i<m->clusterSize.x*2;i++)
	{
		for(j=0;j<m->clusterSize.y;j++)
		{
			u32 po=2048+(((4*4*4)*16))*((i)+((m->clusterSize.x))*(j));
			sSeek(fZ,po,SEEK_SET);
			FILE_POSITION fp=_FAT_getPosition(&po);
			m->fileMap[i+j*m->clusterSize.x*2]=fZ->partition->dataStart+fp.cluster*fZ->partition->sectorsPerCluster+fp.sector;
		}
	}*/
	sSeek(fZ,2048,SEEK_SET);//test
	for(j=0;j<m->clusterSize.y;j++)
	{
		for(i=0;i<m->clusterSize.x*2;i++)
		{
			u32 po;
			FILE_POSITION fp=_FAT_getPosition(&po);
			m->fileMap[i+j*m->clusterSize.x*2]=_FAT_fat_clusterToSector(fZ->partition,fp.cluster)+fp.sector;
			sSeek(fZ,(4*4*4)*16,SEEK_CUR);//test
		}
	}
}

void openMap512(char* filename, map_struct* m)
{
	// chdir("fat:/");
	m->fileHandle=(void*)sOpen(filename);
	FILE_STRUCT* fZ=(FILE_STRUCT*)m->fileHandle;
	m->header=malloc(2048);
	sRead(fZ, (char*)m->header, 2048);
	u32 poZ;
	sSeek(fZ,0,SEEK_SET);
	m->headerMap=malloc(sizeof(u32)*4);
	FILE_POSITION fpZ=_FAT_getPosition(&poZ);
	m->headerMap[0]=_FAT_fat_clusterToSector(fZ->partition,fpZ.cluster)+fpZ.sector;
	sSeek(fZ,512,SEEK_CUR);
	fpZ=_FAT_getPosition(&poZ);
	m->headerMap[1]=_FAT_fat_clusterToSector(fZ->partition,fpZ.cluster)+fpZ.sector;
	sSeek(fZ,512,SEEK_CUR);
	fpZ=_FAT_getPosition(&poZ);
	m->headerMap[2]=_FAT_fat_clusterToSector(fZ->partition,fpZ.cluster)+fpZ.sector;
	sSeek(fZ,512,SEEK_CUR);
	fpZ=_FAT_getPosition(&poZ);
	m->headerMap[3]=_FAT_fat_clusterToSector(fZ->partition,fpZ.cluster)+fpZ.sector;
	if(m->header->magicVersionNumber!=VERSIONMAGIC)
	{
		m->header->spawnX=64;
		m->header->spawnY=64;
	}
	m->clusterSize=(vect3D){m->header->sizeX,m->header->sizeY,16};
	m->fileMap=malloc(sizeof(u32)*m->clusterSize.x*m->clusterSize.y*4);
	int i, j;
	/*for(i=0;i<m->clusterSize.x*4;i++)
	{
		for(j=0;j<m->clusterSize.y;j++)
		{
			u32 po=2048+(((4*4*4)*16)/2)*((i)+((m->clusterSize.x))*(j));
			sSeek(fZ,po,SEEK_SET);
			FILE_POSITION fp=_FAT_getPosition(&po);
			m->fileMap[i+j*m->clusterSize.x*4]=fZ->partition->dataStart+fp.cluster*fZ->partition->sectorsPerCluster+fp.sector;
		}
	}*/
	sSeek(fZ,2048,SEEK_SET);//test
	for(j=0;j<m->clusterSize.y;j++)
	{
		for(i=0;i<m->clusterSize.x*4;i++)
		{
			u32 po;
			FILE_POSITION fp=_FAT_getPosition(&po);
			m->fileMap[i+j*m->clusterSize.x*4]=_FAT_fat_clusterToSector(fZ->partition,fp.cluster)+fp.sector;
			sSeek(fZ,(4*4*4)*16/2,SEEK_CUR);//test
		}
	}
}

void openMapNOCASH(char* filename, map_struct* m)
{
	m->fileHandle=fopen(filename,"rb");
	m->header=malloc(2048);
	fread(m->header, 2048, 1, (FILE*)m->fileHandle);
	if(m->header->magicVersionNumber!=VERSIONMAGIC)
	{
		m->header->spawnX=64;
		m->header->spawnY=64;
	}
	m->clusterSize=(vect3D){m->header->sizeX,m->header->sizeY,16};
	NOGBA("file handle : %p (%s)",m->fileHandle,filename);
}

void loadTestMap(map_struct* m)
{
	openMap(mapPath, m);
	initMap(m,(vect3D){m->header->sizeX,m->header->sizeY,16});
	TESTVALUE2=0;
	int i8, j8;
	// m->header->spawnX=300;
	// m->header->spawnY=256;
	NOGBA("spawn ! %d %d (%d)",m->header->spawnX,m->header->spawnY,m->header->sizeX);
	if(m->header->spawnX<(SUPERCLUSTERSIZE/2)*CLUSTERSIZE)
	{
		m->offset.x=0;
	}else if(m->header->spawnX>(m->header->sizeX-(SUPERCLUSTERSIZE/2))*CLUSTERSIZE)
	{
		m->offset.x=((m->header->sizeX-SUPERCLUSTERSIZE)*CLUSTERSIZE)/CLUSTERSIZE;
	}else{
		m->offset.x=m->header->spawnX-(SUPERCLUSTERSIZE/2)*CLUSTERSIZE;
		m->offset.x=(m->offset.x-(m->offset.x%(CLUSTERSIZE)))/CLUSTERSIZE;
	}
	if(m->header->spawnY<(SUPERCLUSTERSIZE/2)*CLUSTERSIZE)
	{
		m->offset.y=0;
	}else if(m->header->spawnY>(m->header->sizeY-(SUPERCLUSTERSIZE/2))*CLUSTERSIZE)
	{
		m->offset.y=((m->header->sizeY-SUPERCLUSTERSIZE)*CLUSTERSIZE)/CLUSTERSIZE;
	}else{
		m->offset.y=m->header->spawnY-(SUPERCLUSTERSIZE/2)*CLUSTERSIZE;
		m->offset.y=(m->offset.y-(m->offset.y%(CLUSTERSIZE)))/CLUSTERSIZE;
	}
	NOGBA("offset ! %d %d",m->offset.x,m->offset.y);
	for(j8=0;j8<32;j8++)
	{
		for(i8=0;i8<32;i8++)
		{
			readClusterColumn(m, i8+m->offset.x, j8+m->offset.y, m->superCluster[i8][j8], m->transitionStuff, m->fileHandle);
		}
	}
	//init transition X
	i8=-1;
	for(j8=0;j8<32;j8++)
	{
		m->transitionCluster[j8]=malloc(sizeof(clusterColumn_struct));
		initClusterColumn(m->transitionCluster[j8]);
		if(m->offset.x)readClusterColumn(m, i8+m->offset.x, j8+m->offset.y, m->transitionCluster[j8], m->transitionStuff, m->fileHandle);
	}
	i8=32;
	for(j8=0;j8<32;j8++)
	{
		// fseek(m->fileHandle,((sizeof(cluster_struct)*16+(4*4*4)*16))*(i8)+((sizeof(cluster_struct)*16+(4*4*4)*16)*(32*2))*(j8),SEEK_SET);
		m->transitionCluster[j8+32]=malloc(sizeof(clusterColumn_struct));
		initClusterColumn(m->transitionCluster[j8+32]);
		if(m->offset.x<m->header->sizeX-(SUPERCLUSTERSIZE)-1)readClusterColumn(m, i8+m->offset.x, j8+m->offset.y, m->transitionCluster[j8+32], m->transitionStuff, m->fileHandle);
	}
	
	j8=-1;
	for(i8=0;i8<32;i8++)
	{
		m->transitionCluster[i8+32*2]=malloc(sizeof(clusterColumn_struct));
		initClusterColumn(m->transitionCluster[i8+32*2]);
		if(m->offset.y)readClusterColumn(m, i8+m->offset.x, j8+m->offset.y, m->transitionCluster[i8+32*2], m->transitionStuff, m->fileHandle);
	}
	j8=32;
	for(i8=0;i8<32;i8++)
	{
		m->transitionCluster[i8+32*3]=malloc(sizeof(clusterColumn_struct));
		initClusterColumn(m->transitionCluster[i8+32*3]);
		if(m->offset.y<m->header->sizeY-(SUPERCLUSTERSIZE)-1)readClusterColumn(m, i8+m->offset.x, j8+m->offset.y, m->transitionCluster[i8+32*3], m->transitionStuff, m->fileHandle);
	}
}

void readClusterColumn2048(map_struct* m, u16 i, u16 j, clusterColumn_struct* c, u8* t, void* f)
{
	PROF_START();
	readSectors(m->fileMap[i+m->clusterSize.x*j], 2, c->data);
	readSectors(m->fileMap[i+m->clusterSize.x*j]+2, 2, t);
	int time; PROF_END(time);
	addValue(&streamRead,time);
	PROF_START();
	c->changed=0;
	generateQuads(m, c, t, i, j); // ADD SUPPORT FOR WALL GENERATION (done ?)
	PROF_END(time);
	addValue(&streamCalc,time);
	#ifdef DEBUGMODE
	iprintf("\nprecalc : %d  ",time);
	#endif
}

void readClusterColumn1024(map_struct* m, u16 i, u16 j, clusterColumn_struct* c, u8* t, void* f)
{
	PROF_START();
	readSectors(m->fileMap[i*2+m->clusterSize.x*2*j], 2, c->data);
	readSectors(m->fileMap[i*2+1+m->clusterSize.x*2*j], 2, t);
	int time; PROF_END(time);
	addValue(&streamRead,time);
	PROF_START();
	c->changed=0;
	generateQuads(m, c, t, i, j); // ADD SUPPORT FOR WALL GENERATION (done ?)
	PROF_END(time);
	addValue(&streamCalc,time);
}

void readClusterColumn512(map_struct* m, u16 i, u16 j, clusterColumn_struct* c, u8* t, void* f)
{
	PROF_START();
	readSectors(m->fileMap[i*4+m->clusterSize.x*4*j], 1, c->data);
	readSectors(m->fileMap[i*4+1+m->clusterSize.x*4*j], 1, &c->data[512]);
	readSectors(m->fileMap[i*4+2+m->clusterSize.x*4*j], 1, t);
	readSectors(m->fileMap[i*4+3+m->clusterSize.x*4*j], 1, &t[512]);
	int time; PROF_END(time);
	addValue(&streamRead,time);
	PROF_START();
	c->changed=0;
	generateQuads(m, c, t, i, j); // ADD SUPPORT FOR WALL GENERATION (done ?)
	PROF_END(time);
	addValue(&streamCalc,time);
}

void readClusterColumnNOCASH(map_struct* m, u16 i, u16 j, clusterColumn_struct* c, u8* t, void* f)
{
	FILE* fp=(FILE*)f;
	fseek(fp,2048+((4*4*4)*16*2)*(i+j*m->clusterSize.x),SEEK_SET);
	fread(c,(4*4*4)*16,1,f);
	fread(t,(4*4*4)*16,1,f);
	// PROF_START();
	c->changed=0;
	generateQuads(m, c, t, i, j); // ADD SUPPORT FOR WALL GENERATION (done ?)
	// int time; PROF_END(time);
	// NOGBA("precalc : %d  ",time);
}

void writeClusterColumn2048(map_struct* m, u16 i, u16 j, clusterColumn_struct* c, u8* t, void* f)
{
	int time;
	PROF_START();
	precalcCollumn(m, i, j, t);
	writeSectors(m->fileMap[i+m->clusterSize.x*j], 2, c->data);
	writeSectors(m->fileMap[i+m->clusterSize.x*j]+2, 2, t);
	PROF_END(time);
	addValue(&columnWrite,time);
	if(!c->changed)c->changed=2;
}

void writeClusterColumn1024(map_struct* m, u16 i, u16 j, clusterColumn_struct* c, u8* t, void* f)
{
	int time;
	PROF_START();
	precalcCollumn(m, i, j, t);
	writeSectors(m->fileMap[i*2+m->clusterSize.x*2*j], 2, c->data);
	writeSectors(m->fileMap[i*2+1+m->clusterSize.x*2*j], 2, t);
	PROF_END(time);
	addValue(&columnWrite,time);
	if(!c->changed)c->changed=2;
}

void writeClusterColumn512(map_struct* m, u16 i, u16 j, clusterColumn_struct* c, u8* t, void* f)
{
	int time;
	PROF_START();
	precalcCollumn(m, i, j, t);
	writeSectors(m->fileMap[i*4+m->clusterSize.x*4*j], 1, c->data);
	writeSectors(m->fileMap[i*4+1+m->clusterSize.x*4*j], 1, &c->data[512]);
	writeSectors(m->fileMap[i*4+2+m->clusterSize.x*4*j], 1, t);
	writeSectors(m->fileMap[i*4+3+m->clusterSize.x*4*j], 1, &t[512]);
	PROF_END(time);
	addValue(&columnWrite,time);
	if(!c->changed)c->changed=2;
}

void writeClusterColumnNOCASH(map_struct* m, u16 i, u16 j, clusterColumn_struct* c, u8* t, void* f)
{
	//DO NOTHING
	precalcCollumn(m, i, j, t);
	if(!c->changed)c->changed=2;
}

void generateMapQuadList(map_struct* m)
{
	int i, j;
	int tot=0;
	iprintf("\ncluster column : %d  ",sizeof(cluster_struct)*16);
	for(i=0;i<32;i++)
	{
		for(j=0;j<32;j++)
		{
			precalcCollumn(m, i, j, m->transitionStuff);
			PROF_START();
			generateQuads(m, m->superCluster[i][j], m->transitionStuff, i, j);
			int time;
			PROF_END(time);
			tot+=time;
		}
	}
	iprintf("\naverage (column) : %d cycles",(tot)/(32*32));
	NOGBA("after blocks RAM : %dko used, %dko free    \n",DS_UsedMem()/1024,DS_FreeMem()/1024);
}

void updateLightMap(void)
{
	u8	d=0, i=0;
	u32* lm=lightMap;
	for(i=0;i<32-LIGHTAMBIENT;i++)
	{
		u8 l=LIGHTAMBIENT+(i&127);
		*lm=RGB15(l,l,l);
		lm++;
	}
	for(i=i;i<128;i++)*(lm++)=RGB15(31,31,31);
	do{
		u8 l=min(LIGHTAMBIENT2+(lightSun[d]+(i&127)),31);
		*lm=RGB15(l,l,l);
		lm++;
		i++;
		if(l==31)break;
	}while(i);
	for(i=i;i;i++)*(lm++)=RGB15(31,31,31);
	#ifdef FOGLIGHT
		// lightMap[0]=RGB15(LIGHTAMBIENT2,LIGHTAMBIENT2,LIGHTAMBIENT2);
		// lightMap[0]=RGB15(31,31,31);
	#endif
	for(d=1;d<6;d++)
	{
		memcpy(lm,lightMap,128*4);
		lm+=128;
		i=128;
		do{
			u8 l=min(LIGHTAMBIENT2+(lightSun[d]+(i&127)),31);
			*lm=RGB15(l,l,l);
			lm++;
			i++;
			if(l==31)break;
		}while(i);
		for(i=i;i;i++)*(lm++)=RGB15(31,31,31);
	}
	memcpy(&lightMap[256*10],&lightMap[256*3],256*4);
	memcpy(&lightMap[256*11],&lightMap[256*2],256*4);
	memcpy(&lightMap[256*12],&lightMap[256*5],256*4);
	memcpy(&lightMap[256*13],&lightMap[256*4],256*4);
	#ifdef FOGLIGHT
		// memcpy(lightMap2,lightMap,256*14*4);
		// for(d=0;d<6;d++)lightMap2[256*d]=RGB15(31,31,31);
		// lm=lightMap2;
		// for(i=0;i<32-LIGHTAMBIENT;i++)
		// {
			// u8 l=LIGHTAMBIENT+(i&127);
			// *lm=RGB15(l,l,l);
			// lm++;
		// }
		// for(i=i;i<128;i++)*(lm++)=RGB15(31,31,31);
		// for(d=1;d<6;d++)memcpy(&lightMap2[256*d],lightMap2,128*4);
	#endif
}

u8 lasttype;

void drawTestQuadOpt(map_struct* m, quad_struct* q)
{
	#ifdef DEBUGMODE
	testquads++;
	#endif
	
	u32* uv=&uvMapCur[q->type<<2];
	// u32* vert=&xyMap[q->lID];
	const u16 d=(q->direction<<8);
	u32* vert=&xyMap[(q->mID<<2)+d];
	// GFX_COLOR = lightMap[d+q->light];
	GFX_COLOR = lightMapCur[d+q->light];
	
	GFX_TEX_COORD = *uv;
	GFX_VERTEX10 = *vert;
	GFX_TEX_COORD = *(++uv);
	GFX_VERTEX10 = *(++vert);
	GFX_TEX_COORD = *(++uv);
	GFX_VERTEX10 = *(++vert);
	GFX_TEX_COORD = *(++uv);
	GFX_VERTEX10 = *(++vert);
}

void drawQuadList(map_struct* m, quadList_struct* ql, u16 x, u16 y, u16 z)
{
	if(!ql->count)return;
	quad_struct* q=ql->first;
	int i=0;
	glPushMatrix();
	glTranslatef32(x*bsize,y*bsize,z*bsize);
	glBegin(GL_QUADS);
		while(q)
		{
			i++;
			drawTestQuadOpt(m, q);
			q=q->next;
		}
	glPopMatrix(1);
}

void drawCursor(map_struct* m, int i, int j, int k)
{
	glPushMatrix();
	glTranslatef32(((tilesize2*i)<<6)-m->offset.x*bsize,((tilesize2*j)<<6)-m->offset.y*bsize,((tilesize2*k)<<6)-m->offset.z*bsize);
	// quad_struct q=(quad_struct){0, 0, 0, cursorDir, 255, 31, cursorDir, NULL};
	quad_struct q=(quad_struct){0, cursorDir, 0, 31, NULL};
	glScalef32(inttof32(11)/10,inttof32(11)/10,inttof32(11)/10);
		glBegin(GL_QUADS);
		drawTestQuadOpt(m, &q);
	glPopMatrix(1);
}

void drawCluster(cluster_struct* c)
{
	// if(c->list)glCallList(c->list);
	testquads+=c->quadList.count;
}

void renderClusterList(map_struct* m, int x, int y, int z)
{
	cluster_struct* c=&m->superCluster[x-m->offset.x][y-m->offset.y]->cluster[z-m->offset.z];
	int i, j, k;
	bool d1, d2, d3;
	c->wall=7;
	for(i=0;i<CLUSTERSIZE;i++)
	{
		for(j=0;j<CLUSTERSIZE;j++)
		{
			d1=false;d2=false;d3=false;
			for(k=0;k<CLUSTERSIZE;k++)
			{
				// d1=d1||*getBlockP(m,x*CLUSTERSIZE+i,y*CLUSTERSIZE+j,z*CLUSTERSIZE+k);
				// d2=d2||*getBlockP(m,x*CLUSTERSIZE+i,y*CLUSTERSIZE+k,z*CLUSTERSIZE+j);
				// d3=d3||*getBlockP(m,x*CLUSTERSIZE+k,y*CLUSTERSIZE+j,z*CLUSTERSIZE+i);
				d1=d1||*getBlockPE(m,x*CLUSTERSIZE+i,y*CLUSTERSIZE+j,z*CLUSTERSIZE+k); //no reason, just testing
				d2=d2||*getBlockPE(m,x*CLUSTERSIZE+i,y*CLUSTERSIZE+k,z*CLUSTERSIZE+j);
				d3=d3||*getBlockPE(m,x*CLUSTERSIZE+k,y*CLUSTERSIZE+j,z*CLUSTERSIZE+i);
			}
			c->wall&=(d1)|(d2<<1)|(d3<<2);
		}
	}
	NOGBA("%d %d %d vs %d %d %d : %d",x,y,z,x-m->offset.x,y-m->offset.y,z-m->offset.z,c->wall);
}

void drawTestCluster(cluster_struct* c, int i, int j, int k)
{
	glBindTexture(0, 0);
	bool d1=c->wall&1, d2=(c->wall>>1)&1, d3=(c->wall>>2)&1;
	if(!d1 && !d2 && !d3)return;
	// NOGBA("ICI : %d %d %d (%d%d%d)(%d)",i,j,k,d1,d2,d3,c->wall);
	glColor3b(200*d1,200*d2,200*d3);
	glBegin(GL_QUADS);	
		//top
		GFX_VERTEX10 = NORMAL_PACK((0+tilesize2*CLUSTERSIZE*i-tilesize),(0+tilesize2*CLUSTERSIZE*j-tilesize),(0+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((0+tilesize2*CLUSTERSIZE*i-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*j-tilesize),(0+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*i-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*j-tilesize),(0+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*i-tilesize),(0+tilesize2*CLUSTERSIZE*j-tilesize),(0+tilesize2*CLUSTERSIZE*k-tilesize));
		//bottom
		GFX_VERTEX10 = NORMAL_PACK((0+tilesize2*CLUSTERSIZE*i-tilesize),(0+tilesize2*CLUSTERSIZE*j-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((0+tilesize2*CLUSTERSIZE*i-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*j-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*i-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*j-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*i-tilesize),(0+tilesize2*CLUSTERSIZE*j-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*k-tilesize));
		
		//side
		GFX_VERTEX10 = NORMAL_PACK((0+tilesize2*CLUSTERSIZE*i-tilesize),(0+tilesize2*CLUSTERSIZE*j-tilesize),(0+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((0+tilesize2*CLUSTERSIZE*i-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*j-tilesize),(0+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((0+tilesize2*CLUSTERSIZE*i-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*j-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((0+tilesize2*CLUSTERSIZE*i-tilesize),(0+tilesize2*CLUSTERSIZE*j-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*k-tilesize));
		//side
		GFX_VERTEX10 = NORMAL_PACK((tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*i-tilesize),(0+tilesize2*CLUSTERSIZE*j-tilesize),(0+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*i-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*j-tilesize),(0+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*i-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*j-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*i-tilesize),(0+tilesize2*CLUSTERSIZE*j-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*k-tilesize));
		
		//side
		GFX_VERTEX10 = NORMAL_PACK((0+tilesize2*CLUSTERSIZE*i-tilesize),(0+tilesize2*CLUSTERSIZE*j-tilesize),(0+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*i-tilesize),(0+tilesize2*CLUSTERSIZE*j-tilesize),(0+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*i-tilesize),(0+tilesize2*CLUSTERSIZE*j-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((0+tilesize2*CLUSTERSIZE*i-tilesize),(0+tilesize2*CLUSTERSIZE*j-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*k-tilesize));
		//side
		GFX_VERTEX10 = NORMAL_PACK((0+tilesize2*CLUSTERSIZE*i-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*j-tilesize),(0+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*i-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*j-tilesize),(0+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*i-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*j-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*k-tilesize));
		GFX_VERTEX10 = NORMAL_PACK((0+tilesize2*CLUSTERSIZE*i-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*j-tilesize),(tilesize2*CLUSTERSIZE+tilesize2*CLUSTERSIZE*k-tilesize));
		
	glEnd();
	// BoxTest(i*bsize-(tilesize<<6),j*bsize-(tilesize<<6),k*bsize-(tilesize<<6),bsize,bsize,bsize);
}

void generateDisplayLists(map_struct* m)
{
	int i, j, k;
	for(i=0;i<SUPERCLUSTERSIZE;i++)for(j=0;j<SUPERCLUSTERSIZE;j++)for(k=0;k<m->clusterSize.z;k++)renderClusterList(m, i, j, k);
	NOGBA("after lists RAM : %dko used, %dko free    \n",DS_UsedMem()/1024,DS_FreeMem()/1024);
}

void writeMapHeader(map_struct* m)
{
	m->header->magicVersionNumber=VERSIONMAGIC;
	m->header->spawnX=Player.position.x/(rTilesize2)+(SUPERCLUSTERSIZE/2+m->offset.x)*CLUSTERSIZE;
	m->header->spawnY=Player.position.y/(rTilesize2)+(SUPERCLUSTERSIZE/2+m->offset.y)*CLUSTERSIZE;
	m->header->spawnZ=Player.position.z;
	NOGBA("player pos : %d %d",m->header->spawnX,m->header->spawnY);
	switch(fsFormat)
	{
		case 1:
			writeSectors(m->headerMap[0], 4, (u8*)m->header);
			break;
		case 2:
			writeSectors(m->headerMap[0], 2, &(((u8*)m->header)[0]));
			writeSectors(m->headerMap[1], 2, &(((u8*)m->header)[1024]));
			break;
		case 3:
			writeSectors(m->headerMap[0], 1, &(((u8*)m->header)[0]));
			writeSectors(m->headerMap[1], 1, &(((u8*)m->header)[512]));
			writeSectors(m->headerMap[2], 1, &(((u8*)m->header)[1024]));
			writeSectors(m->headerMap[3], 1, &(((u8*)m->header)[1024+512]));
			break;
		case 0:
			NOGBA("writing header !");
			break;
	}
}

void globalSaveMap(map_struct* m)
{
	startSave();
	writeMapHeader(m);
	int i, j;
	for(i=0;i<SUPERCLUSTERSIZE;i++)
	{
		for(j=0;j<SUPERCLUSTERSIZE;j++)
		{
			if(m->superCluster[i][j]->changed==1)
			{
				writeClusterColumn(m, i+m->offset.x, j+m->offset.y, m->superCluster[i][j], m->transitionStuff, m->fileHandle);
				if(i<SUPERCLUSTERSIZE-1)writeClusterColumn(m, i+m->offset.x+1, j+m->offset.y, m->superCluster[i+1][j], m->transitionStuff, m->fileHandle);
				if(i>0)writeClusterColumn(m, i+m->offset.x-1, j+m->offset.y, m->superCluster[i-1][j], m->transitionStuff, m->fileHandle);
				if(j<SUPERCLUSTERSIZE-1)writeClusterColumn(m, i+m->offset.x, j+m->offset.y+1, m->superCluster[i][j+1], m->transitionStuff, m->fileHandle);
				if(j>0)writeClusterColumn(m, i+m->offset.x, j+m->offset.y-1, m->superCluster[i][j-1], m->transitionStuff, m->fileHandle);
			}
		}
	}
	for(i=0;i<SUPERCLUSTERSIZE;i++)
	{
		for(j=0;j<SUPERCLUSTERSIZE;j++)
		{
			m->superCluster[i][j]->changed=0;
		}
	}
	endSave();
}

int maT1, maT2, miT1, miT2;
int rt1, rt2;

void captureQuadOpt(map_struct* m, quad_struct* q, u16 x, u16 y, u16 z)
{
	u32* uv=&uvMap[q->type<<2];
	// u32* vert=&xyMap[q->lID];
	// const u16 d=(q->direction<<8);
	// u32* vert=&xyMap[(q->mID<<2)+d];
	// glColorDL(lightMap[d+q->light]);	
	
	int i, j, k;
	i=imIDtable[q->mID];
	j=jmIDtable[q->mID];
	k=kmIDtable[q->mID];
	int32 x2=(Player.position.x/SCALEFACTOR)>>6;
	int32 y2=(Player.position.y/SCALEFACTOR)>>6;
	int32 z2=(Player.position.z/SCALEFACTOR)>>6;
	
	switch(q->direction)
	{
		case 0:
			glNormalDL(NORMAL_PACK(0,0,inttov10(1)-1));
			glTexCoordPACKED(*uv);
			glVertexPackedDL(NORMAL_PACK((tilesize+tilesize2*i)+x*bsizeR-x2,(-tilesize+tilesize2*j)+y*bsizeR-y2,(tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((tilesize+tilesize2*i)+x*bsizeR-x2,(tilesize+tilesize2*j)+y*bsizeR-y2,(tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((-tilesize+tilesize2*i)+x*bsizeR-x2,(tilesize+tilesize2*j)+y*bsizeR-y2,(tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((-tilesize+tilesize2*i)+x*bsizeR-x2,(-tilesize+tilesize2*j)+y*bsizeR-y2,(tilesize+tilesize2*k)+z*bsizeR-z2));
			break;
		case 1:
			glNormalDL(NORMAL_PACK(0,0,inttov10(-1)));
			glTexCoordPACKED(*uv);
			glVertexPackedDL(NORMAL_PACK((-tilesize+tilesize2*i)+x*bsizeR-x2,(-tilesize+tilesize2*j)+y*bsizeR-y2,(-tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((-tilesize+tilesize2*i)+x*bsizeR-x2,(tilesize+tilesize2*j)+y*bsizeR-y2,(-tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((tilesize+tilesize2*i)+x*bsizeR-x2,(tilesize+tilesize2*j)+y*bsizeR-y2,(-tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((tilesize+tilesize2*i)+x*bsizeR-x2,(-tilesize+tilesize2*j)+y*bsizeR-y2,(-tilesize+tilesize2*k)+z*bsizeR-z2));
			break;
		case 2:
			glNormalDL(NORMAL_PACK(0,inttov10(1)-1,0));
			glTexCoordPACKED(*uv);
			glVertexPackedDL(NORMAL_PACK((tilesize+tilesize2*i)+x*bsizeR-x2,(tilesize+tilesize2*j)+y*bsizeR-y2,(tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((tilesize+tilesize2*i)+x*bsizeR-x2,(-tilesize+tilesize2*j)+y*bsizeR-y2,(tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((tilesize+tilesize2*i)+x*bsizeR-x2,(-tilesize+tilesize2*j)+y*bsizeR-y2,(-tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((tilesize+tilesize2*i)+x*bsizeR-x2,(tilesize+tilesize2*j)+y*bsizeR-y2,(-tilesize+tilesize2*k)+z*bsizeR-z2));
			break;
		case 3:
			glNormalDL(NORMAL_PACK(0,inttov10(-1),0));
			glTexCoordPACKED(*uv);
			glVertexPackedDL(NORMAL_PACK((-tilesize+tilesize2*i)+x*bsizeR-x2,(-tilesize+tilesize2*j)+y*bsizeR-y2,(tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((-tilesize+tilesize2*i)+x*bsizeR-x2,(tilesize+tilesize2*j)+y*bsizeR-y2,(tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((-tilesize+tilesize2*i)+x*bsizeR-x2,(tilesize+tilesize2*j)+y*bsizeR-y2,(-tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((-tilesize+tilesize2*i)+x*bsizeR-x2,(-tilesize+tilesize2*j)+y*bsizeR-y2,(-tilesize+tilesize2*k)+z*bsizeR-z2));
			break;
		case 4:
			glNormalDL(NORMAL_PACK(inttov10(1)-1,0,0));
			glTexCoordPACKED(*uv);
			glVertexPackedDL(NORMAL_PACK((-tilesize+tilesize2*i)+x*bsizeR-x2,(tilesize+tilesize2*j)+y*bsizeR-y2,(tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((tilesize+tilesize2*i)+x*bsizeR-x2,(tilesize+tilesize2*j)+y*bsizeR-y2,(tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((tilesize+tilesize2*i)+x*bsizeR-x2,(tilesize+tilesize2*j)+y*bsizeR-y2,(-tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((-tilesize+tilesize2*i)+x*bsizeR-x2,(tilesize+tilesize2*j)+y*bsizeR-y2,(-tilesize+tilesize2*k)+z*bsizeR-z2));
			break;
		case 5:
			glNormalDL(NORMAL_PACK(inttov10(-1),0,0));
			glTexCoordPACKED(*uv);
			glVertexPackedDL(NORMAL_PACK((tilesize+tilesize2*i)+x*bsizeR-x2,(-tilesize+tilesize2*j)+y*bsizeR-y2,(tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((-tilesize+tilesize2*i)+x*bsizeR-x2,(-tilesize+tilesize2*j)+y*bsizeR-y2,(tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((-tilesize+tilesize2*i)+x*bsizeR-x2,(-tilesize+tilesize2*j)+y*bsizeR-y2,(-tilesize+tilesize2*k)+z*bsizeR-z2));
			glTexCoordPACKED(*(++uv));
			glVertexPackedDL(NORMAL_PACK((tilesize+tilesize2*i)+x*bsizeR-x2,(-tilesize+tilesize2*j)+y*bsizeR-y2,(-tilesize+tilesize2*k)+z*bsizeR-z2));
			break;
	}
}

void captureQuadList(map_struct* m, quadList_struct* ql, u16 x, u16 y, u16 z)
{
	if(!ql->count)return;
	quad_struct* q=ql->first;
	int i=0;
	glPushMatrix();
	// glTranslatef32(x*bsize,y*bsize,z*bsize);
	glBeginDL(GL_QUADS);
		while(q)
		{
			i++;
			captureQuadOpt(m, q, x, y, z);
			q=q->next;
		}
	glPopMatrix(1);
}

void captureScene(map_struct* m, char* filename)
{
	int i;
	u32* list=glBeginListDL();
		listElement_struct* le=closedList.elements;
		for(i=0;i<closedList.size;i++)
		{
			captureQuadList(m, &m->superCluster[le->i][le->j]->cluster[le->k].quadList, le->i, le->j, le->k);
			le++;
		}
	glEndListDL();

	// u32* sceneList=malloc(((*list)+1)*4);
	// memcpy(sceneList, list, ((*list)+1)*4);
	FILE* f=fopen(filename,"wb");
	fwrite(list,4,((*list)+1),f);
	fclose(f);
}

void drawTestCube(void)
{
	glPushMatrix();
	glTranslatef32(0,768,0);
	glTranslatef32(480+(cosLerp(walkAngle>>1)>>7),0,-512+(cosLerp(walkAngle>>2)>>8));
	glTranslatef32(0,0,-1000);
	glRotateZi(degreesToAngle(33));
	// glRotateXi(degreesToAngle(45));
	glRotateXi(cubeAngleX);
	glTranslatef32(0,0,1000);
	if(cubeAngleX && cubeAngleX>-3000)cubeAngleX-=600;
	else cubeAngleX=0;
	glScalef32(inttof32(5)/2,inttof32(5)/2,inttof32(5)/2);
	// Game_ApplyMTL(NULL);
	Game_ApplyMTL(blockSuperTexture);
	glBegin(GL_QUADS);
	
		if(cursorBlock==13 || cursorBlock==LADDERTYPE || cursorBlock==DOORTYPE)
		{
			u8 type=blocks[cursorBlock].bottom;
			u32* uv=&uvMapCur[type<<2];
			glColor(RGB15(31,31,31));
			
			//side
			GFX_TEX_COORD = *uv;
			GFX_VERTEX10 = NORMAL_PACK((0),(tilesize2*2-tilesize2),(tilesize2*2-tilesize));
			GFX_TEX_COORD = *(++uv);
			GFX_VERTEX10 = NORMAL_PACK((0),(0-tilesize2),(tilesize2*2-tilesize));
			GFX_TEX_COORD = *(++uv);
			GFX_VERTEX10 = NORMAL_PACK((0),(0-tilesize2),(0-tilesize));
			GFX_TEX_COORD = *(++uv);
			GFX_VERTEX10 = NORMAL_PACK((0),(tilesize2*2-tilesize2),(0-tilesize));
		}else if(cursorBlock==11)
		{
			u8 type=blocks[cursorBlock].top;
			u32* uv=&uvMapCur[type<<2];
			glColor(RGB15(31,31,31));
			
			//side
			GFX_TEX_COORD = *uv;
			GFX_VERTEX10 = NORMAL_PACK((0),(tilesize2*2-tilesize2),(tilesize2*2-tilesize*2));
			GFX_TEX_COORD = *(++uv);
			GFX_VERTEX10 = NORMAL_PACK((0),(0-tilesize2),(tilesize2*2-tilesize*2));
			GFX_TEX_COORD = *(++uv);
			GFX_VERTEX10 = NORMAL_PACK((0),(0-tilesize2),(0-tilesize*2));
			GFX_TEX_COORD = *(++uv);
			GFX_VERTEX10 = NORMAL_PACK((0),(tilesize2*2-tilesize2),(0-tilesize*2));
		}else{
			u8 type=blocks[cursorBlock].top;
			u32* uv=&uvMapCur[type<<2];
			glColor3b(200,200,200);
			
			GFX_TEX_COORD = *uv;
			GFX_VERTEX10 = NORMAL_PACK((0-tilesize),(0-tilesize),(tilesize2-tilesize));
			GFX_TEX_COORD = *(++uv);
			GFX_VERTEX10 = NORMAL_PACK((0-tilesize),(tilesize2-tilesize),(tilesize2-tilesize));
			GFX_TEX_COORD = *(++uv);
			GFX_VERTEX10 = NORMAL_PACK((tilesize2-tilesize),(tilesize2-tilesize),(tilesize2-tilesize));
			GFX_TEX_COORD = *(++uv);
			GFX_VERTEX10 = NORMAL_PACK((tilesize2-tilesize),(0-tilesize),(tilesize2-tilesize));
			
			type=blocks[cursorBlock].side;
			uv=&uvMapCur[type<<2];
			glColor3b(100,100,100);
			
			//side
			GFX_TEX_COORD = *uv;
			GFX_VERTEX10 = NORMAL_PACK((0-tilesize),(tilesize2-tilesize),(tilesize2-tilesize));
			GFX_TEX_COORD = *(++uv);
			GFX_VERTEX10 = NORMAL_PACK((0-tilesize),(0-tilesize),(tilesize2-tilesize));
			GFX_TEX_COORD = *(++uv);
			GFX_VERTEX10 = NORMAL_PACK((0-tilesize),(0-tilesize),(0-tilesize));
			GFX_TEX_COORD = *(++uv);
			GFX_VERTEX10 = NORMAL_PACK((0-tilesize),(tilesize2-tilesize),(0-tilesize));
			
			uv=&uvMapCur[type<<2];
			glColor3b(50,50,50);
			
			GFX_TEX_COORD = *uv;
			GFX_VERTEX10 = NORMAL_PACK((0-tilesize),(0-tilesize),(tilesize2-tilesize));
			GFX_TEX_COORD = *(++uv);
			GFX_VERTEX10 = NORMAL_PACK((tilesize2-tilesize),(0-tilesize),(tilesize2-tilesize));
			GFX_TEX_COORD = *(++uv);
			GFX_VERTEX10 = NORMAL_PACK((tilesize2-tilesize),(0-tilesize),(0-tilesize));
			GFX_TEX_COORD = *(++uv);
			GFX_VERTEX10 = NORMAL_PACK((0-tilesize),(0-tilesize),(0-tilesize));
		}
	glEnd();
	glPopMatrix(1);
}

void drawTestMap(map_struct* m)
{
	int i, j, k, time;
	
	// Game_ApplyMTL(NULL);
	// glCallList(creeper_bin);
	
	glPushMatrix();
	glScalef32(inttof32(SCALEFACTOR),inttof32(SCALEFACTOR),inttof32(SCALEFACTOR));
	glTranslatef32(-(SUPERCLUSTERSIZE*CLUSTERSIZE*(tilesize2<<6))/2, -(SUPERCLUSTERSIZE*CLUSTERSIZE*(tilesize2<<6))/2,-(m->size.z*(tilesize2<<6))/2);
	{
		testquads=0;
		PROF_START();
		glPolyFmt(POLY_ALPHA(31) /*| POLY_FORMAT_LIGHT0 | POLY_FORMAT_LIGHT1*/ | POLY_CULL_BACK);
		// if(keysHeld() & KEY_SELECT)
		if(0)
		{
			for(i=0;i<closedList.size;i++)
			{
				listElement_struct* le=&closedList.elements[i];
				// glPolyFmt(POLY_ALPHA(31) | POLY_FORMAT_LIGHT0/* | POLY_FORMAT_LIGHT1*/ | POLY_CULL_BACK);
				// drawCluster(&m->cluster[qgetCluster(m,le->i,le->j,le->k)]);
				// drawQuadList(m, &m->superCluster[le->i][le->j]->cluster[le->k].quadList, le->i, le->j, le->k);
				glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
				Game_ApplyMTL(NULL);
				/*if(!cull)*/drawTestCluster(&m->superCluster[le->i][le->j]->cluster[le->k], le->i, le->j, le->k);
			}
			/*for(i=0;i<32;i++)
			{
				for(j=0;j<16;j++)
				{
					drawQuadList(m, &m->transitionCluster[i+32*3]->cluster[j].quadList, i, 15, j);
				}
			}*/
		}else{
		
			if(Player.inWater==2)glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FOG); //TEST TEST TEST
			else glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
			// Game_ApplyMTL(NULL);
			Game_FastBind(blockSuperTexture);
			uvMapCur=uvMap;
			listElement_struct* le=closedList.elements;
			lightMapCur=lightMap;
			// NOGBA("cave ? %d",Player.inCave);
			// NOGBA("p : %d %d %d",Player.clusterCoord.x,Player.clusterCoord.y,Player.clusterCoord.z);
			if(!Player.inCave && fogMode)
			{
				for(i=0;i<closedList.size;i++)
				{
					#ifdef FOGLIGHT
						if(!Player.inCave && !m->superCluster[le->i][le->j]->cluster[le->k].lightList.count && le->i >= Player.clusterCoord.x-1 && le->j >= Player.clusterCoord.y-1 && le->k >= Player.clusterCoord.z-1
						&& le->i <= Player.clusterCoord.x+1 && le->j <= Player.clusterCoord.y+1 && le->k <= Player.clusterCoord.z+1)
						{
							glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FOG);
							lightMapCur=lightMap2;
						}else{
							glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
							lightMapCur=lightMap;
						}
					#endif
					drawQuadList(m, &m->superCluster[le->i][le->j]->cluster[le->k].quadList, le->i, le->j, le->k);
					le++;
				}
			}else{
				for(i=0;i<closedList.size;i++)
				{
					drawQuadList(m, &m->superCluster[le->i][le->j]->cluster[le->k].quadList, le->i, le->j, le->k);
					le++;
				}
			}
			le=closedList.elements;
			Game_FastBind(waterTexture);
			uvMapCur=uvMapWater;
			glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
			for(i=0;i<closedList.size;i++)
			{
				drawQuadList(m, &m->superCluster[le->i][le->j]->cluster[le->k].specialList, le->i, le->j, le->k);
				le++;
			}
			updateUVwater();
			uvMapCur=uvMap;
			Game_FastBind(cursorTexture);
			drawCursor(m, testCursorI, testCursorJ, testCursorK);
		}
		
		PROF_END(time);
		#ifdef DEBUGMODE
		iprintf("\ndrawing : %d   ",time);
		#endif
	}
	
	// PROF_START();
	// if(!testBuffer)
	{
		if(m->transitioning[0])
		{
			m->transitioning[0]--;
			int j=m->transitioning[0], i;
			clusterColumn_struct* temp;
			switch(j)
			{
				case 33:
					if(m->offset.y < m->clusterSize.y-SUPERCLUSTERSIZE/*-SUPERCLUSTERSIZE/2*/)
					{
						temp=m->transitionCluster[31+32*3];
						for(i=32-1;i>0;i--)
						{
							m->transitionCluster[i+32*3]=m->transitionCluster[i-1+32*3];
						}
						m->transitionCluster[0+32*3]=temp;
						readClusterColumn(m, m->offset.x, 32+m->offset.y, m->transitionCluster[0+32*3], m->transitionStuff, m->fileHandle);
					}
					break;
				case 32:
					if(m->offset.y > 0)
					{
						temp=m->transitionCluster[31+32*2];
						for(i=32-1;i>0;i--)
						{
							m->transitionCluster[i+32*2]=m->transitionCluster[i-1+32*2];
						}
						m->transitionCluster[0+32*2]=temp;
						readClusterColumn(m, m->offset.x, m->offset.y-1, m->transitionCluster[0+32*2], m->transitionStuff, m->fileHandle);
					}
					break;
				default:
					if(!(m->offset.x > 0))m->transitioning[0]=0;
					else readClusterColumn(m, m->offset.x-1, j+m->offset.y, m->transitionCluster[j], m->transitionStuff, m->fileHandle);
					break;
			}
		}else if(m->transitioning[1])
		{
			m->transitioning[1]--;
			int j=m->transitioning[1], i;
			clusterColumn_struct* temp;
			switch(j)
			{
				case 33:
					if(m->offset.y < m->clusterSize.y-SUPERCLUSTERSIZE/*-SUPERCLUSTERSIZE/2*/)
					{
						temp=m->transitionCluster[0+32*3];
						for(i=0;i<32-1;i++)
						{
							m->transitionCluster[i+32*3]=m->transitionCluster[i+1+32*3];
						}
						m->transitionCluster[31+32*3]=temp;
						readClusterColumn(m, 31+m->offset.x, 32+m->offset.y, m->transitionCluster[31+32*3], m->transitionStuff, m->fileHandle);
					}
					break;
				case 32:
					if(m->offset.y > 0)
					{
						temp=m->transitionCluster[0+32*2];
						for(i=0;i<32-1;i++)
						{
							m->transitionCluster[i+32*2]=m->transitionCluster[i+1+32*2];
						}
						m->transitionCluster[31+32*2]=temp;
						readClusterColumn(m, 31+m->offset.x, m->offset.y-1, m->transitionCluster[31+32*2], m->transitionStuff, m->fileHandle);
					}
					break;
				default:
					if(!(m->offset.x < m->clusterSize.x-SUPERCLUSTERSIZE/*-SUPERCLUSTERSIZE/2*/))m->transitioning[1]=0;
					else readClusterColumn(m, 32+m->offset.x, j+m->offset.y, m->transitionCluster[j+32], m->transitionStuff, m->fileHandle);
					break;
			}
		}else if(m->transitioning[2])
		{
			m->transitioning[2]--;
			int j=m->transitioning[2], i;
			clusterColumn_struct* temp;
			switch(j)
			{
				case 33:
					if(m->offset.x < m->clusterSize.x-SUPERCLUSTERSIZE/*-SUPERCLUSTERSIZE/2*/)
					{
						temp=m->transitionCluster[31+32];
						for(i=32-1;i>0;i--)
						{
							m->transitionCluster[i+32*1]=m->transitionCluster[i-1+32*1];
						}
						m->transitionCluster[0+32*1]=temp;
						readClusterColumn(m, 32+m->offset.x, m->offset.y, m->transitionCluster[0+32*1], m->transitionStuff, m->fileHandle);
					}
					break;
				case 32:
					if(m->offset.x > 0)
					{
						temp=m->transitionCluster[31+0];
						for(i=32-1;i>0;i--)
						{
							m->transitionCluster[i+32*0]=m->transitionCluster[i-1+32*0];
						}
						m->transitionCluster[0+32*0]=temp;
						readClusterColumn(m, m->offset.x-1, m->offset.y, m->transitionCluster[0+32*0], m->transitionStuff, m->fileHandle);
					}
					break;
				default:
					if(!(m->offset.y > 0))m->transitioning[2]=0;
					else readClusterColumn(m, j+m->offset.x, m->offset.y-1, m->transitionCluster[j+32*2], m->transitionStuff, m->fileHandle);
					break;
			}	
		}else if(m->transitioning[3])
		{
			m->transitioning[3]--;
			int j=m->transitioning[3], i;
			clusterColumn_struct* temp;
			switch(j)
			{
				case 33:
					if(m->offset.x < m->clusterSize.x-SUPERCLUSTERSIZE/*-SUPERCLUSTERSIZE/2*/)
					{
						temp=m->transitionCluster[0+32*1];
						for(i=0;i<32-1;i++)
						{
							m->transitionCluster[i+32*1]=m->transitionCluster[i+1+32*1];
						}
						m->transitionCluster[31+32*1]=temp;
						readClusterColumn(m, 32+m->offset.x, m->offset.y+31, m->transitionCluster[31+32*1], m->transitionStuff, m->fileHandle);
					}
					break;
				case 32:
					if(m->offset.x > 0)
					{
						temp=m->transitionCluster[0+32*0];
						for(i=0;i<32-1;i++)
						{
							m->transitionCluster[i+32*0]=m->transitionCluster[i+1+32*0];
						}
						m->transitionCluster[31+32*0]=temp;
						readClusterColumn(m, m->offset.x-1, m->offset.y+31, m->transitionCluster[31+32*0], m->transitionStuff, m->fileHandle);
					}
					break;
				default:
					if(!(m->offset.y < m->clusterSize.y-SUPERCLUSTERSIZE/*-SUPERCLUSTERSIZE/2*/))m->transitioning[3]=0;
					else readClusterColumn(m, j+m->offset.x, 32+m->offset.y, m->transitionCluster[j+32*3], m->transitionStuff, m->fileHandle);
					break;
			}
		}
	}
	// int time;
	// PROF_END(time);
	// if(time>20000)addValue(&streamRead,time);
	#ifdef DEBUGMODE
	iprintf("\nstreaming : %d (%d)         ",time,m->transitioning[1]);
	#endif
	// if(m->transitioning[1])while(!(keysDown()&KEY_A))scanKeys();
	
		// if(testBuffer && (keysDown() & KEY_START))cull=!cull;
		// if(testBuffer && (keysDown() & KEY_SELECT))cull=!cull;
		// if(keysDown() & KEY_X)testvar=!testvar;
	glPopMatrix(1);
		glPopMatrix(1);
		glPushMatrix();
		if(!testBuffer)updatePlayer(&Player);
		i=(Player.position.x+(tilesize<<6)*SCALEFACTOR+(SUPERCLUSTERSIZE*(bsize*SCALEFACTOR))/2)/(bsize*SCALEFACTOR);
		j=(Player.position.y+(tilesize<<6)*SCALEFACTOR+(SUPERCLUSTERSIZE*(bsize*SCALEFACTOR))/2)/(bsize*SCALEFACTOR);
		k=(Player.position.z+(tilesize<<6)*SCALEFACTOR+(m->clusterSize.z*(bsize*SCALEFACTOR))/2)/(bsize*SCALEFACTOR);
		#ifdef DEBUGMODE
		iprintf("\ncluster : %d, %d, %d", i+m->offset.x, j+m->offset.y, k);
		#endif
		playerCamera(&Player, true);
		glPushMatrix();
		glScalef32(inttof32(SCALEFACTOR),inttof32(SCALEFACTOR),inttof32(SCALEFACTOR));
		glTranslatef32(-(SUPERCLUSTERSIZE*CLUSTERSIZE*(tilesize2<<6))/2, -(SUPERCLUSTERSIZE*CLUSTERSIZE*(tilesize2<<6))/2,-(m->size.z*(tilesize2<<6))/2);
	PROF_START();
	if(cull)cullClusters(m, &openList, &closedList, i, j, k);
	// else cullClusters2(m, &openList, &closedList, i, j, k);
	PROF_END(time);
		glPopMatrix(1);
	if(testBuffer)
	{
		rt1++;
		rt1%=2;
		if(rt1)maT1=time;
		else miT1=time;
		#ifdef DEBUGMODE
		iprintf("\ntesting : %d,%d (%d)         ",maT1,miT1,cull);
		#endif
		{
			// PROF_START();
			toProcess_struct* q=lightProcess.first;
			// toProcess_struct** pq=&lightProcess.first;
			void** pq=&lightProcess.first;
			while(q)
			{
				if(q->i-m->offset.x*CLUSTERSIZE>CLUSTERSIZE && q->i-m->offset.x*CLUSTERSIZE<(SUPERCLUSTERSIZE-2)*CLUSTERSIZE
				&& q->j-m->offset.y*CLUSTERSIZE>CLUSTERSIZE && q->j-m->offset.y*CLUSTERSIZE<(SUPERCLUSTERSIZE-2)*CLUSTERSIZE)
				{
					vect3D clusterCoord=getCluster(m,q->i,q->j,q->k);
					processLight(m,q->i,q->j,q->k,clusterCoord);
					*pq=q->next;
					free(q);
					break;
				}
				// pq=(toProcess_struct**)&q->next;
				pq=&q->next;
				q=q->next;
			}
			// PROF_END(TESTVALUE);
		}
	}else{
		rt2++;
		rt2%=2;
		if(rt2)maT2=time;
		else miT2=time;
		#ifdef DEBUGMODE
		iprintf("\ntesting : %d,%d (%d)         ",maT2,miT2,cull);
		#endif
		{
			// int i, j, m;
			int i3, ma;
			PROF_START();
				processWater(m);
				if(waterCursor2>waterCount2)ma=WATERNUMBER2;
				else ma=waterCount2;
				// j=0;
				for(i3=waterCursor2;i3<ma/* && j<256*/;i3++)
				{
					water_struct* w=&waterToSpread[i3];
					const u16 i=w->pos&8191, j=(w->pos>>13)&8191;
						if(i-m->offset.x*CLUSTERSIZE<WATERMAX
						&& j-m->offset.y*CLUSTERSIZE<WATERMAX
						&& i-m->offset.x*CLUSTERSIZE>WATERMIN
						&& j-m->offset.y*CLUSTERSIZE>WATERMIN)
						{
							addWater2(m,*w);
							*w=waterToSpread[waterCursor2++];
						}else if(i-m->offset.x*CLUSTERSIZE<-CLUSTERSIZE
							|| j-m->offset.y*CLUSTERSIZE<-CLUSTERSIZE
							|| i-m->offset.x*CLUSTERSIZE>(SUPERCLUSTERSIZE+1)*CLUSTERSIZE
							|| j-m->offset.y*CLUSTERSIZE>(SUPERCLUSTERSIZE+1)*CLUSTERSIZE){
							*w=waterToSpread[waterCursor2++];
						}
					if(i3==ma-1 && ma==WATERNUMBER2){i3=0;ma=waterCount2;}
				}
			PROF_END(TESTVALUE);
		}
	}
	#ifdef DEBUGMODE
	iprintf("\nquads : %d (%d)  ",testquads,testBuffer);
	iprintf("\nFS : %d %d %d   ",fsFormat,m->headerMap[0],m->fileMap[0]);
	#endif
	// if((keysDown() & KEY_START) && !testBuffer)captureScene(m, "fat:/testscene1.bin");
	// if((keysDown() & KEY_START) && testBuffer)captureScene(m, "fat:/testscene2.bin");
}
