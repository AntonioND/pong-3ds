
#ifndef __ROOM__
#define __ROOM__

//--------------------------------------------------------------------------------------------------

#include "game.h"

//--------------------------------------------------------------------------------------------------

void Room_SetNumber(_game_room_e room);
int Room_GetNumber(void);

void Room_Draw(int screen);

void Room_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax);

void Room_Init(void);
void Room_Handle(void);

//--------------------------------------------------------------------------------------------------

#endif //__ROOM__