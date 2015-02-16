
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

#ifndef __ROOM_3__
#define __ROOM_3__

//--------------------------------------------------------------------------------------------------

#include "rooms.h"

//--------------------------------------------------------------------------------------------------

void Room_3_Draw(int screen);

//--------------------------------------------------------------------------------------------------

void Room_3_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax);

//--------------------------------------------------------------------------------------------------

void Room_3_Init(void);

void Room_3_End(void);

void Room_3_Handle(void);

_3d_mode_e Room_3_3DMode(void);

//--------------------------------------------------------------------------------------------------

#endif //__ROOM_3__
