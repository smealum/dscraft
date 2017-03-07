#ifndef __STATE9__
#define __STATE9__

typedef void(*function)();

typedef struct{
	function Init,Frame,Kill,VBlank;
	u16 mc_id;
	u8 id;
	bool used;
	int notusedtest;
}DS_state;

u8 state_id;

DS_state *CurrentState, *NextState;

DS_state Game_State, Bilan_State, Menu_State;

void glReInit(void);
void DS_ApplyState(void);
void DS_InitHardware(void);
void DS_SetState(DS_state* state);

static inline void DS_ChangeState(DS_state* state)
{
	CurrentState->used=0;
	NextState=state;
}

void DS_CreateState(DS_state* state, function Init, function Frame, function Kill, function VBlank);

#endif
