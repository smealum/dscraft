#include "common/general.h"
#include <errno.h>

void DS_InitMalloc()
{
	int i;
	
	for(i=0;i<MAX_MALLOC;i++)
	{
		DS_malloc_list[i]=NULL;
	}
}

void* DS_mAlloc(size_t size, DS_state* state)
{
	int i;
	for(i=0;i<MAX_MALLOC;i++)
	{
		if(DS_malloc_list[i]==NULL)
		{
			DS_malloc_list[i]=malloc(size);
			if(DS_malloc_list[i]==NULL)NOGBA("MALLOC ERROR ! DUCK FOR COVER : %d, %s (%d,%d)",errno,strerror(errno),DS_UsedMem()/1024,DS_FreeMem()/1024);
			state->mc_id++;
			return DS_malloc_list[i];
		}
	}
	NOGBA("malloc error !");
	return NULL;
}

void DS_freeState(DS_state* state)
{
	if(state->mc_id>0)
	{
		int i;
		for(i=MAX_MALLOC-1;i>=0;i--)
		{
			if(DS_malloc_list[i]!=NULL)
			{
				free(DS_malloc_list[i]);
				DS_malloc_list[i]=NULL;
				//NOGBA("%d, %p\n",i,GetStackPointer());
			}
		}
		DS_Debug("%d,",state->mc_id);
		state->mc_id=0;
	}
}
