#ifdef SURVIVAL
#ifndef MOBS_9
#define MOBS_9

typedef struct
{
	vect3D colPosition, position, vector;
	s16 angleZ;
	u8 type;
	u16 id;
}mob_struct;

typedef struct
{
	mob_struct mob;
	void* next;
}mobList_element;

typedef struct
{
	mobList_element* first;
	u16 count;
}mobList_struct;

mobList_struct mobList;

#endif
#endif
