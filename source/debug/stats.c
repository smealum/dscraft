#include "common/general.h"

void initStats(stats_struct* s)
{
	s->total=0;
	s->min=1<<30;
	s->max=0;
	s->count=0;
}

void addValue(stats_struct* s, u32 value)
{
	s->total+=value;
	if(value>s->max)s->max=value;
	if(value<s->min)s->min=value;
	s->count++;
}

void writeStats(stats_struct* s, char* name, char* filename)
{
	/*FILE* f=fopen(filename,"a");
	if(!f)return;
	char str[255];
	sprintf(str,"\n%s : %d-%d \n %d (%d) \n\n",name,s->min,s->max,s->total/s->count,s->count);
	fwrite(str,1,strlen(str),f);
	fclose(f);*/
}
