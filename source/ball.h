
//-------------------------------------------------------------------------------------------------------

/*
    Pong 3DS. Just a pong for the Nintendo 3DS.
    Copyright (C) 2015 Antonio Niño Díaz

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//-------------------------------------------------------------------------------------------------------

#ifndef __BALL__
#define __BALL__

//--------------------------------------------------------------------------------------------------

#include "game.h"

//--------------------------------------------------------------------------------------------------

void Ball_SetDimensions(int x, int y, int z);

void Ball_SetColor(int r, int g, int b, int a);

void Ball_Draw(int screen); // Non-internal flush
void Ball_DrawShadows(int screen); // Internal flush

void Ball_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax);

void Ball_GetPosition(int * x, int * y, int * z);
void Ball_GetSpeed(int * x, int * y, int * z);
void Ball_GetDimensions(int * x, int * y, int * z);

void Ball_Enable3DMovement(int enable);

void Ball_Init(void);
void Ball_Reset(void);
void Ball_Handle(void);

void Ball_Bounce(int speed, int acc);

//--------------------------------------------------------------------------------------------------

#endif //__BALL__
