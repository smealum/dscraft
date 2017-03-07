#ifndef INTERFACE_9
#define INTERFACE_9

#define MAXITEMS 64
#define MAXSLOTS 64

u8 usedSprites;

typedef struct
{
	vect3D position;
	u8 id, type;
	s8 slot;
	bool used;
}item_struct;

typedef struct
{
	vect3D position;
	s8 id;
	bool used;
}slot_struct;

item_struct items[MAXITEMS];
slot_struct slots[MAXSLOTS];

bool invOpen, overButtons;

void initItems(void);
void initItemBar(void);
void initInterface(void);
bool updateInterface(void);

#endif