#ifndef __API9__
#define __API9__

typedef void(*API_drawfunction)(void* e);

typedef struct
{
	s32 x, y;
}coord2Ds;

typedef struct
{
	u32 x, y;
}coord2Du;

typedef struct
{
	u8* data;
	u8 charsizeX, charsizeY;
	u8 difference;
}API_Font;

typedef struct API_Entity
{
	u16 id, type;
	u8 timep;
	u8 times;
	u8 timea;
	s16 prio;
	s16 alpha;
	u16 color;
	u8 t_timep;
	u8 t_times;
	u8 t_timea;
	void* data;
	u8 outline;
	s16 t_alpha;
	bool halpha;
	bool shadow;
	bool bastard;
	u8 kidToDate;
	coord2Ds Size;
	coord2Ds Ratio;
	coord2Ds a_Size;
	coord2Ds o_Size;
	coord2Ds b_Size;
	coord2Ds t_Size;
	GLvector scaleVs;
	u16 outline_color;
	coord2Ds Position;
	coord2Ds a_Position;
	coord2Ds o_Position;
	coord2Ds t_Position;
	API_drawfunction Draw;
	struct API_Entity *father;
	struct API_Entity *temp_son;
	struct API_Entity *up, *down, *left, *right;
}API_Entity;

typedef void(*API_function)(API_Entity* e);

typedef struct
{
	MTL_img *background;
}APIE_WindowData;

typedef struct
{
	// MTL_img *background;
	char* string;
	u16 width;
	u16 color;
}APIE_LabelData;

typedef struct
{
	bool over, clicked;
	MTL_img *background;
	API_Entity* label;
	API_function function;
	bool clickable;
}APIE_ButtonData;

typedef struct
{
	bool over, checked, clickable;
	MTL_img *background;
	API_Entity* label;
	API_function function;
}APIE_CheckBoxData;

typedef struct
{
	s16 position, oldpos;
	API_Entity* label;
	bool over, selected;
	MTL_img *background;
	API_function function;
}APIE_SliderData;

typedef struct API_ListElement
{
	struct API_ListElement *previous, *next;
	API_Entity *entity;
}API_ListElement;

typedef struct
{
	API_ListElement *first, *last;
	u16 count;
}API_EntList;

bool cull;
bool API_DrawingOutline;
API_Font API_font;
API_EntList API_List;
touchPosition API_Touch;

API_Entity* buttonBlock;

API_Entity *window1, *window2, *window3, *window4, *window5, *window6, *window7, *window8, *window9, *installWindow, *descriptionWindow, *replayWindow, *replayWindow2; //TEMP

API_function API_ToCall;
API_Entity* API_ToCallEntity;
API_Entity* Cursor;
API_Entity* DefaultCursor;

API_Entity* TESTLABEL;

void APIcall();
void API_Init();
void API_CleanUp();
void API_SetX(API_Entity* e, s16 x);
void API_ToggleShadow(API_Entity* e);
void API_UpdateScene(API_EntList *cl);
void API_SetAlpha(API_Entity* e, u8 alpha);
void API_SetSize(API_Entity* e, u16 x, u16 y);
void API_SetAlphaSons(API_Entity* f, u8 alpha);
void API_SetPosition(API_Entity* e, s16 x, s16 y);
void API_FadeSons(API_Entity* f, u8 alpha, u8 time);
void API_FadeEntity(API_Entity* e, u8 alpha, u8 time);
void API_MoveEntity(API_Entity* e, s16 x, s16 y, u8 time);
void API_ResizeEntity(API_Entity* e, u16 x, u16 y, u8 time);
void API_DeleteEntityByFather(API_EntList *cl, API_Entity* father);
API_Entity* API_CreateLabel(s16 x, s16 y, u16 color, char* text);
API_Entity* API_CreateLabelFather(s16 x, s16 y, u16 color, API_Entity* father, char* text, bool halpha);
API_Entity* API_CreateCheckBoxFather(s16 x, s16 y, API_function function, API_Entity* father, char* text, bool halpha);
API_Entity* API_CreateSliderFather(s16 x, s16 y, u8 size, API_function function, API_Entity* father, char* text, bool halpha);
API_Entity* API_CreateVSliderFather(s16 x, s16 y, u8 size, API_function function, API_Entity* father, char* text, bool halpha);
API_Entity* API_CreateWindow(s16 x, s16 y, u16 sx, u16 sy, u8 alpha, u16 color, u8 outline, u16 outline_color, char* filename);
API_Entity* API_CreateButtonFather(s16 x, s16 y, u16 color, API_function function, API_Entity* father, char* text, char* filename, bool halpha);

#endif