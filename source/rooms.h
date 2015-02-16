
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

#ifndef __ROOM__
#define __ROOM__

//--------------------------------------------------------------------------------------------------

typedef enum {
	GAME_ROOM_MENU,
	GAME_ROOM_1,
	GAME_ROOM_2,
	GAME_ROOM_3
} _game_room_e;

typedef enum {
	GAME_MODE_2D,
	GAME_MODE_2D_BOUNCE,
	GAME_MODE_3D
} _3d_mode_e;

//--------------------------------------------------------------------------------------------------

void Room_SetNumber(_game_room_e room);
int Room_GetNumber(void);

void Room_Draw(int screen);

_3d_mode_e Room_3DMode(void);

void Room_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax);

void Room_Init(void);
void Room_End(void);

void Room_Handle(void);

//--------------------------------------------------------------------------------------------------

#endif //__ROOM__
