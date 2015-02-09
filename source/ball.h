
#ifndef __BALL__
#define __BALL__

//--------------------------------------------------------------------------------------------------

#include "game.h"

//--------------------------------------------------------------------------------------------------

void Ball_SetDimensions(int x, int y, int z);

void Ball_SetColor(int r, int g, int b);

void Ball_Draw(int screen);

void Ball_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax);

void Ball_GetPosition(int * x, int * y, int * z);
void Ball_GetSpeed(int * x, int * y, int * z);
void Ball_GetDimensions(int * x, int * y, int * z);

void Ball_Enable3DMovement(int enable);

void Ball_Init(void);
void Ball_Reset(void);
void Ball_Handle(void);

//--------------------------------------------------------------------------------------------------

#endif //__BALL__
