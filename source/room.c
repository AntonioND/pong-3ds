
//--------------------------------------------------------------------------------------------------

#include <string.h>

#include "S3D/engine.h"
#include "game.h"
#include "room.h"
#include "pad.h"
#include "ball.h"

//--------------------------------------------------------------------------------------------------

_game_room_e current_room;

//--------------------------------------------------------------------------------------------------

void Room_SetNumber(_game_room_e room)
{
	current_room = room;
	Room_Init();
}

int Room_GetNumber(void)
{
	return current_room;
}

//--------------------------------------------------------------------------------------------------

static inline int max(int a, int b)
{
	return a > b ? a : b;
}

static inline int min(int a, int b)
{
	return a < b ? a : b;
}

void Room_Draw(int screen)
{
	switch(current_room)
	{
		case GAME_ROOM_1:
		{
			//Surface ...
	
			S3D_PolygonBegin(screen, S3D_QUADS);
			
			int i,j;
			for(j = 0; j < 4; j++) for(i = 0; i < 4; i++)
			{
				S3D_PolygonColor(screen, 0,max(255-((i+j)*40),0),0);
				
				S3D_PolygonNormal(screen, float2fx(0.0),float2fx(1.0),float2fx(0.0));
				
				int xbase = float2fx(-6.75) + float2fx(3.5)*j - float2fx(0.1);
				int zbase = float2fx(-1.0) + float2fx(3.5)*i - float2fx(0.1);
				
				S3D_PolygonVertex(screen, xbase, float2fx(-1), zbase);
				S3D_PolygonVertex(screen, xbase, float2fx(-1), zbase + float2fx(3.2));
				S3D_PolygonVertex(screen, xbase + float2fx(3.2), float2fx(-1), zbase + float2fx(3.2));
				S3D_PolygonVertex(screen, xbase + float2fx(3.2), float2fx(-1), zbase);
			}

			S3D_PolygonListFlush(screen, 0);

			// Borders ...
			
			for(i = 0; i < 4; i++)
			{
				S3D_PolygonColor(screen, 0,0,max(255-(i<<6),0));
				
				int zbase = float2fx(-1.0) + float2fx(3.5)*i - float2fx(0.1);
				
				S3D_PolygonNormal(screen, float2fx(1.0),float2fx(0.0),float2fx(0.0));
				
				S3D_PolygonVertex(screen, float2fx(-7),float2fx(2.6),zbase);
				S3D_PolygonVertex(screen, float2fx(-7),float2fx(2.6),zbase + float2fx(3.2));
				S3D_PolygonVertex(screen, float2fx(-7),float2fx(-0.6),zbase + float2fx(3.2));
				S3D_PolygonVertex(screen, float2fx(-7),float2fx(-0.6),zbase);
				
				S3D_PolygonNormal(screen, float2fx(-1.0),float2fx(0.0),float2fx(0.0));
				
				S3D_PolygonVertex(screen, float2fx(7),float2fx(-0.6),zbase);
				S3D_PolygonVertex(screen, float2fx(7),float2fx(-0.6),zbase + float2fx(3.0));
				S3D_PolygonVertex(screen, float2fx(7),float2fx(2.6),zbase + float2fx(3.0));
				S3D_PolygonVertex(screen, float2fx(7),float2fx(2.6),zbase);
			}

			S3D_PolygonListFlush(screen, 1);
			break;
		}
		
		default:
		case GAME_ROOM_MENU:
			break;
	}
}

//--------------------------------------------------------------------------------------------------

void Room_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax)
{
	switch(current_room)
	{
		case GAME_ROOM_1:
			if(xmin) *xmin = float2fx(-7.0);
			if(xmax) *xmax = float2fx(+7.0);
			if(ymin) *ymin = float2fx(-1.0);
			if(ymax) *ymax = float2fx(+3.0);
			if(zmin) *zmin = float2fx(-1.25);
			if(zmax) *zmax = float2fx(+12.75);
			break;
			
		// Non-playable rooms
		default:
		case GAME_ROOM_MENU:
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
		case GAME_ROOM_1:
			Ball_Init();
			Ball_SetDimensions(float2fx(1.5),float2fx(1.5),float2fx(1.5));
			Ball_SetColor(128,128,128);
			
			Pad_InitAll();
			Pad_P1SetDimensions(float2fx(4.0),float2fx(2.0),float2fx(1.0));
			Pad_P1SetColor(255,0,0);
			Pad_P2SetDimensions(float2fx(4.0),float2fx(2.0),float2fx(1.0));
			Pad_P2SetColor(255,255,0);
			break;
			
		// Non-playable rooms
		default:
		case GAME_ROOM_MENU:
			break;
	}
}

//--------------------------------------------------------------------------------------------------

void Room_Handle(void)
{
	//Nothing for now
}

//--------------------------------------------------------------------------------------------------
