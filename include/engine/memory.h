#ifndef __MEMORY9__
#define __MEMORY9__

// #define MAX_MALLOC 16384
// #define MAX_MALLOC 1
// #define MAX_MALLOC 4096
#define MAX_MALLOC 512

typedef struct{
	void* addr;
	size_t size;
	DS_state *state;
	u16 id;
	bool used;
}DS_malloc;

void *DS_malloc_list[MAX_MALLOC];
void *GetStackPointer();

void* DS_mAlloc(size_t size, DS_state* state);
void DS_free(u16 id, DS_state* state);
void DS_freeState(DS_state* state);
void DS_InitMalloc();

#endif
