#include "common/general.h"

void DS_InitHardware(void)
{
	// irqEnable(IRQ_VBLANK);
	// irqSet(IRQ_VBLANK, 0);
	defaultExceptionHandler();
	glInit();
}

void glReInit(void) //Code taken from libnds
{
	// Clear the FIFO
	GFX_STATUS |= (1<<29);

	// Clear overflows from list memory
	glResetMatrixStack();

	// prime the vertex/polygon buffers
	glFlush(0);

	// reset the control bits
	GFX_CONTROL = 0;

	// reset the rear-plane(a.k.a. clear color) to black, ID=0, and opaque
	glClearColor(0,0,0,31);
	glClearPolyID(0);

	// reset stored texture locations
	glResetTextures();

	// reset the depth to it's max
	glClearDepth(GL_MAX_DEPTH);

	GFX_TEX_FORMAT = 0;
	GFX_POLY_FORMAT = 0;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
}

void DS_CreateState(DS_state* state, function Init, function Frame, function Kill, function VBlank)
{
	state->Init=(function)Init;
	state->Frame=(function)Frame;
	state->Kill=(function)Kill;
	state->VBlank=(function)VBlank;
	
	state->id=state_id;
	state->mc_id=0;
	state_id++;
}

void DS_SetState(DS_state* state)
{
	CurrentState=state;
	CurrentState->used=1;
	//irqSet(IRQ_VBLANK, CurrentState->VBlank);
}

void DS_ApplyState()
{
	CurrentState=NextState;
	CurrentState->used=1;
	CurrentState->mc_id=0;
	DS_InitMalloc();
	irqSet(IRQ_VBLANK, CurrentState->VBlank);
}
