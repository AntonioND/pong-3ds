
//--------------------------------------------------------------------------------------------------

#include <string.h>

#include "rooms.h"

#include "room_menu.h"
#include "room_1.h"
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
		
		case GAME_ROOM_1:
			Room_1_Draw(screen);
			break;
		case GAME_ROOM_3:
			Room_3_Draw(screen);
			break;
		
		// Non-playable rooms
		
		case GAME_ROOM_MENU:
			Room_Menu_Draw(screen);
			break;
		
		default:
			break;
	}
}

//--------------------------------------------------------------------------------------------------

int Room_3DMovementEnabled(void)
{
	switch(current_room)
	{
		// Playable rooms
		
		case GAME_ROOM_1:
			return Room_1_3DMovementEnabled();
		case GAME_ROOM_3:
			return Room_3_3DMovementEnabled();
			
		// Non-playable rooms
			
		case GAME_ROOM_MENU:
			return Room_Menu_3DMovementEnabled();
			
		default:
			return 0;
	}
}

//--------------------------------------------------------------------------------------------------

void Room_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax)
{
	switch(current_room)
	{
		// Playable rooms
		
		case GAME_ROOM_1:
			Room_1_GetBounds(xmin,xmax,ymin,ymax,zmin,zmax);
			break;
		case GAME_ROOM_3:
			Room_3_GetBounds(xmin,xmax,ymin,ymax,zmin,zmax);
			break;
			
		// Non-playable rooms
			
		case GAME_ROOM_MENU:
			Room_Menu_GetBounds(xmin,xmax,ymin,ymax,zmin,zmax);
			break;
			
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
		
		case GAME_ROOM_1:
			Room_1_Init();
			break;
		case GAME_ROOM_3:
			Room_3_Init();
			break;
		
		// Non-playable rooms
		
		case GAME_ROOM_MENU:
			Room_Menu_Init();
			break;
			
		default:
			break;
	}
}


void Room_End(void)
{
	switch(current_room)
	{
		// Playable rooms
		
		case GAME_ROOM_1:
			Room_1_End();
			break;
		case GAME_ROOM_3:
			Room_3_End();
			break;
		
		// Non-playable rooms
		
		case GAME_ROOM_MENU:
			Room_Menu_End();
			break;
			
		default:
			break;
	}
}

//--------------------------------------------------------------------------------------------------

void Room_Handle(void)
{
	
	switch(current_room)
	{
		// Playable rooms
		
		case GAME_ROOM_1:
			Room_1_Handle();
			break;
		case GAME_ROOM_3:
			Room_3_Handle();
			break;
		
		// Non-playable rooms
		
		case GAME_ROOM_MENU:
			Room_Menu_Handle();
			break;
		
		default:
			break;
	}
}

//--------------------------------------------------------------------------------------------------
