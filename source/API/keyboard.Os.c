#include "common/general.h"
#include "game/textures.h"
#include "API/font.h"
#include "API/API.h"
#include "API/keyboard.h"

void initKeyboard(u8 t)
{
	int i, j, k, h;
	char string[2];
	
	if(t==0 || t>1){keyboardWindow=API_CreateWindow(150, 50, 170, 80, 31, RGB15(31,31,31), 1, RGB15(0,0,0), "");keyboardWindow->prio=800;}	
	
	if(t>0)
	{
		i=0;h=5;
		for(k=0;k<4;k++)
		{
			for(j=0;j<keyboardRows[k];j++)
			{
				sprintf(string,"%c",keyboardButtons[i]);
				keyboardButton[i]=API_CreateButtonFather(j*16+5, h, RGB15(31,31,31), &keyboardButtonPressed, keyboardWindow, string, "arsenal_buttons.pcx", false);
				i++;
			}
			h+=16;
		}
		keyboardButton[i]=API_CreateButtonFather(32+5, h, RGB15(31,31,31), &keyboardButtonPressed, keyboardWindow, "       ", "arsenal_buttons.pcx", false);
		i++;
		keyboardButton[i]=API_CreateButtonFather(j*16+5, h-16, RGB15(31,31,31), &keyboardButtonPressed, keyboardWindow, "<-", "arsenal_buttons.pcx", false);
		i++;
		keyboardButton[i]=API_CreateButtonFather((j+1)*16+14, h-30, RGB15(31,31,31), &keyboardButtonPressed, keyboardWindow, "", "enter.pcx", false);
		API_SetSize(keyboardButton[i],22,30);
		API_ComputeDirections(&API_List, true);
		API_SetAlphaSons(keyboardWindow,0);
	}
	keyboardLock=false;
}

void keyboardButtonPressed(API_Entity *e)
{
	int i;
	Cursor=e;
	for(i=0;i<KEYBOARDBUTTONS;i++)
	{
		if(((APIE_ButtonData*)keyboardButton[i]->data)->clicked)
		{
			NOGBA("%d",i);
			if(i<KEYBOARDBUTTONS-2)
			{
				if(keyboardCursor<keyboardStrlen-1)
				{
					(keyboardString)[keyboardCursor]=keyboardButtons[i];
					keyboardCursor++;
					(keyboardString)[keyboardCursor]='\0';
				}
			}else if(i==KEYBOARDBUTTONS-2){
				if(keyboardCursor>0)
				{
					keyboardCursor--;
					(keyboardString)[keyboardCursor]='\0';
				}
			}else if(i==KEYBOARDBUTTONS-1){
				if(keyboardCursor>0)
				{
					keyboardReturn(e);
				}
			}
			break;
		}
	}
	for(i=0;i<KEYBOARDBUTTONS;i++)((APIE_ButtonData*)keyboardButton[i]->data)->clicked=false;
	NOGBA("%s",(keyboardString));
}

void setupKeyboard(char* string, u8 stringlen, API_drawfunction r)
{
	keyboardString=string;
	keyboardStrlen=stringlen;
	keyboardCursor=0;
	keyboardReturn=r;
}

void lockKeyboard()
{
	keyboardLock=true;
}

void showKeyboard()
{
	API_SetPosition(keyboardWindow,-85,50);
	API_SetAlpha(keyboardWindow,15);
	API_SetAlphaSons(keyboardWindow,31);
	Cursor=keyboardButton[38];
}

void hideKeyboard()
{
	API_SetPosition(keyboardWindow,150,50);
	API_SetAlpha(keyboardWindow,15);
	API_SetAlphaSons(keyboardWindow,0);
	keyboardLock=false;
	Cursor=NULL;
}

void appearKeyboard(u8 t)
{
	API_MoveEntity(keyboardWindow,-85,50,t);
	API_FadeEntity(keyboardWindow,15,t);
	API_FadeSons(keyboardWindow,31,t);
	Cursor=keyboardButton[38];
}

void disappearKeyboard(u8 t)
{
	API_MoveEntity(keyboardWindow,150,50,t);
	API_FadeEntity(keyboardWindow,0,t);
	API_FadeSons(keyboardWindow,0,t);
	keyboardLock=false;
	Cursor=NULL;
}
