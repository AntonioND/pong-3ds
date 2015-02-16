
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

#ifndef __ROOM_MENU__
#define __ROOM_MENU__

//--------------------------------------------------------------------------------------------------

#include "rooms.h"

//--------------------------------------------------------------------------------------------------

void Room_Menu_Draw(int screen);
void Room_Menu_Draw_Bottom(void);

//--------------------------------------------------------------------------------------------------

void Room_Menu_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax);

//--------------------------------------------------------------------------------------------------

void Room_Menu_Init(void);

void Room_Menu_End(void);

void Room_Menu_Handle(void);

_3d_mode_e Room_Menu_3DMode(void);

//--------------------------------------------------------------------------------------------------

#endif //__ROOM_MENU__
