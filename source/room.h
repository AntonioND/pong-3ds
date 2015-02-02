
#ifndef __ROOM__
#define __ROOM__

//--------------------------------------------------------------------------------------------------

void Room_SetCurrent(int room);
int Room_GetCurrent(void);

void Room_Draw(int screen);

void Room_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax);

void Room_Init(void);
void Room_Handle(void);

//--------------------------------------------------------------------------------------------------

#endif //__ROOM__
