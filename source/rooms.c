
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

#include <string.h>

#include "rooms.h"

#include "room_menu.h"
#include "room_1.h"
#include "room_2.h"
#include "room_3.h"

//--------------------------------------------------------------------------------------------------

static _game_room_e current_room;

void Room_SetNumber(_game_room_e room)
{
	Room_End();
	current_room = room;
	Room_Init();
}

int Room_GetNumber(void)
{
	return current_room;
}

//--------------------------------------------------------------------------------------------------

void Room_Draw(int screen)
{
	switch(current_room)
	{
		// Playable rooms
		
		case GAME_ROOM_1: Room_1_Draw(screen); break;
		case GAME_ROOM_2: Room_2_Draw(screen); break;
		case GAME_ROOM_3: Room_3_Draw(screen); break;
		
		// Non-playable rooms
		
		case GAME_ROOM_MENU: Room_Menu_Draw(screen); break;
		
		default: break;
	}
}

//--------------------------------------------------------------------------------------------------

_3d_mode_e Room_3DMode(void)
{
	switch(current_room)
	{
		// Playable rooms
		
		case GAME_ROOM_1: return Room_1_3DMode();
		case GAME_ROOM_2: return Room_2_3DMode();
		case GAME_ROOM_3: return Room_3_3DMode();
		
		// Non-playable rooms
		
		case GAME_ROOM_MENU: return Room_Menu_3DMode();
		
		default: return GAME_MODE_2D;
	}
}

//--------------------------------------------------------------------------------------------------

void Room_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax)
{
	switch(current_room)
	{
		// Playable rooms
		
		case GAME_ROOM_1: Room_1_GetBounds(xmin,xmax,ymin,ymax,zmin,zmax); break;
		case GAME_ROOM_2: Room_2_GetBounds(xmin,xmax,ymin,ymax,zmin,zmax); break;
		case GAME_ROOM_3: Room_3_GetBounds(xmin,xmax,ymin,ymax,zmin,zmax); break;
		
		// Non-playable rooms
		
		case GAME_ROOM_MENU: Room_Menu_GetBounds(xmin,xmax,ymin,ymax,zmin,zmax); break;
		
		default:
			if(xmin) *xmin = 0;
			if(xmax) *xmax = 0;
			if(ymin) *ymin = 0;
			if(ymax) *ymax = 0;
			if(zmin) *zmin = 0;
			if(zmax) *zmax = 0;
			break;
	}
}

//--------------------------------------------------------------------------------------------------

void Room_Init(void)
{
	switch(current_room)
	{
		// Playable rooms
		
		case GAME_ROOM_1: Room_1_Init(); break;
		case GAME_ROOM_2: Room_2_Init(); break;
		case GAME_ROOM_3: Room_3_Init(); break;
		
		// Non-playable rooms
		
		case GAME_ROOM_MENU: Room_Menu_Init(); break;
		
		default: break;
	}
}


void Room_End(void)
{
	switch(current_room)
	{
		// Playable rooms
		
		case GAME_ROOM_1: Room_1_End(); break;
		case GAME_ROOM_2: Room_2_End(); break;
		case GAME_ROOM_3: Room_3_End(); break;
		
		// Non-playable rooms
		
		case GAME_ROOM_MENU: Room_Menu_End(); break;
		
		default: break;
	}
}

//--------------------------------------------------------------------------------------------------

void Room_Handle(void)
{
	switch(current_room)
	{
		// Playable rooms
		
		case GAME_ROOM_1: Room_1_Handle(); break;
		case GAME_ROOM_2: Room_2_Handle(); break;
		case GAME_ROOM_3: Room_3_Handle(); break;
		
		// Non-playable rooms
		
		case GAME_ROOM_MENU: Room_Menu_Handle(); break;
		
		default: break;
	}
}

//--------------------------------------------------------------------------------------------------
