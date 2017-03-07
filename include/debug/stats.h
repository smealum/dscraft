#ifndef _STATS_H
#define _STATS_H

typedef struct
{
	u32 total;
	u32 max, min;
	u32 count;
}stats_struct;

stats_struct streamRead;
stats_struct streamCalc;
stats_struct columnWrite;
stats_struct frameTime;
stats_struct freeRam;

void initStats(stats_struct* s);
void addValue(stats_struct* s, u32 value);
void writeStats(stats_struct* s, char* name, char* filename);

#endif 
