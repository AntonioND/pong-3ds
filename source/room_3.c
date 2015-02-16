
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

#include "S3D/engine.h"
#include "game.h"
#include "rooms.h"
#include "pad.h"
#include "ball.h"

//--------------------------------------------------------------------------------------------------

static inline int max(int a, int b)
{
	return a > b ? a : b;
}

static inline int min(int a, int b)
{
	return a < b ? a : b;
}

//--------------------------------------------------------------------------------------------------

static void _room_DrawRoom3(int screen)
{
	int i,j;
	
	S3D_PolygonBegin(screen, S3D_QUADS);
	
	//------
	
	S3D_PolygonColor(screen, 255,0,0);
	
	S3D_PolygonNormal(screen, float2fx(0.0),float2fx(+1.0),float2fx(0.0));
	
	for(j = 0; j < 2; j++) for(i = 0; i < 2; i++)
	{
		int alpha = 255 >> ((i^j)&1);
		S3D_PolygonAlpha(screen, alpha);
		
		int xbase = float2fx(-7.0) + float2fx(7.0)*j;
		int zbase = float2fx(-1.25) + float2fx(7.0)*i;
		
		S3D_PolygonVertex(screen, xbase, float2fx(-6.0), zbase);
		S3D_PolygonVertex(screen, xbase, float2fx(-6.0), zbase + float2fx(7.0));
		S3D_PolygonVertex(screen, xbase + float2fx(7.0), float2fx(-6.0), zbase + float2fx(7.0));
		S3D_PolygonVertex(screen, xbase + float2fx(7.0), float2fx(-6.0), zbase);
	}
	
	//------
	
	S3D_PolygonColor(screen, 0,0,255);
	
	S3D_PolygonNormal(screen, float2fx(0.0),float2fx(-1.0),float2fx(0.0));
	
	for(j = 0; j < 2; j++) for(i = 0; i < 2; i++)
	{
		int alpha = 255 >> (((1-i)^j)&1);
		S3D_PolygonAlpha(screen, alpha);
		
		int xbase = float2fx(-7.0) + float2fx(7.0)*j;
		int zbase = float2fx(-1.25) + float2fx(7.0)*i;
		
		S3D_PolygonVertex(screen, xbase + float2fx(7.0), float2fx(+6.0), zbase);
		S3D_PolygonVertex(screen, xbase + float2fx(7.0), float2fx(+6.0), zbase + float2fx(7.0));
		S3D_PolygonVertex(screen, xbase, float2fx(+6.0), zbase + float2fx(7.0));
		S3D_PolygonVertex(screen, xbase, float2fx(+6.0), zbase);
	}
	
	//------
	
	S3D_PolygonColor(screen, 0,255,0);
	
	S3D_PolygonNormal(screen, float2fx(+1.0),float2fx(0.0),float2fx(0.0));
	
	for(j = 0; j < 2; j++) for(i = 0; i < 2; i++)
	{
		int alpha = 255 >> (((1-i)^j)&1);
		S3D_PolygonAlpha(screen, alpha);
		
		int ybase = float2fx(-6.0) + float2fx(6.0)*j;
		int zbase = float2fx(-1.25) + float2fx(7.0)*i;
		
		S3D_PolygonVertex(screen, float2fx(-7.0), ybase + float2fx(6.0), zbase);
		S3D_PolygonVertex(screen, float2fx(-7.0), ybase + float2fx(6.0), zbase + float2fx(7.0));
		S3D_PolygonVertex(screen, float2fx(-7.0), ybase, zbase + float2fx(7.0));
		S3D_PolygonVertex(screen, float2fx(-7.0), ybase, zbase);	
	}
	
	//------
	
	S3D_PolygonColor(screen, 0,255,0);
	
	S3D_PolygonNormal(screen, float2fx(-1.0),float2fx(0.0),float2fx(0.0));
	
	for(j = 0; j < 2; j++) for(i = 0; i < 2; i++)
	{
		int alpha = 255 >> ((i^j)&1);
		S3D_PolygonAlpha(screen, alpha);
		
		int ybase = float2fx(-6.0) + float2fx(6.0)*j;
		int zbase = float2fx(-1.25) + float2fx(7.0)*i;
		
		S3D_PolygonVertex(screen, float2fx(+7.0), ybase, zbase);
		S3D_PolygonVertex(screen, float2fx(+7.0), ybase, zbase + float2fx(7.0));
		S3D_PolygonVertex(screen, float2fx(+7.0), ybase + float2fx(6.0), zbase + float2fx(7.0));
		S3D_PolygonVertex(screen, float2fx(+7.0), ybase + float2fx(6.0), zbase);	
	}
	
	//------
	
	S3D_PolygonListFlush(screen, 0);
}

void Room_3_Draw(int screen)
{
	// Configure
	
	S3D_SetCulling(screen, 1,0);
	
	m44 m;
	m44_CreateTranslation(&m,0,int2fx(0),int2fx(10));
	S3D_ModelviewMatrixSet(screen, &m);	
	S3D_ModelviewMatrixMultiply(screen, &m);
	
	S3D_LightAmbientColorSet(screen, 64,64,64);
	
	S3D_LightEnable(screen, S3D_LIGHT_N(0));
	S3D_LightDirectionalColorSet(screen, 0, 192,192,192);
	S3D_LightDirectionalVectorSet(screen, 0, float2fx(-0.38),float2fx(-0.76),float2fx(0.53));
	
	// Move camera
	
	int x,y;
	Pad_P1GetPosition(&x,&y,NULL);
	
	// Camera rotation effect...
	m44_CreateRotationY(&m,(x>>3));
	S3D_ModelviewMatrixMultiply(screen, &m);
	
	m44_CreateRotationX(&m,(y>>3));
	S3D_ModelviewMatrixMultiply(screen, &m);
	
	// Draw
	
	_room_DrawRoom3(screen); // Internal flush
	
	Ball_DrawShadows(screen); // Internal flush
	Pad_P1DrawShadows(screen); // Internal flush
	Pad_P2DrawShadows(screen); // Internal flush
	
	Pad_P2Draw(screen); // IA
	Ball_Draw(screen);
	S3D_PolygonListFlush(screen, 1);
	
	S3D_SetCulling(screen, 1,1);
	Pad_P1Draw(screen); // Player	
	S3D_SetCulling(screen, 1,0);
	S3D_PolygonListFlush(screen, 1);
}

//--------------------------------------------------------------------------------------------------

void Room_3_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax)
{
	if(xmin) *xmin = float2fx(-7.0);
	if(xmax) *xmax = float2fx(+7.0);
	if(ymin) *ymin = float2fx(-6.0);
	if(ymax) *ymax = float2fx(+6.0);
	if(zmin) *zmin = float2fx(-1.25);
	if(zmax) *zmax = float2fx(+12.75);
}

//--------------------------------------------------------------------------------------------------

void Room_3_Init(void)
{
	Ball_Init();
	Ball_SetDimensions(float2fx(1.5),float2fx(1.5),float2fx(1.5));
	Ball_SetColor(128,128,128,255);
	Ball_Reset();
	
	Pad_InitAll();
	Pad_P1SetDimensions(float2fx(3.5),float2fx(3.5),float2fx(0.75));
	Pad_P1SetColor(255,0,0,128);
	Pad_P2SetDimensions(float2fx(3.5),float2fx(3.5),float2fx(0.75));
	Pad_P2SetColor(255,255,0,255);
	Pad_ResetAll();
	
	Game_PlayerResetAll();
	
	Game_StateMachineReset();
}

void Room_3_End(void)
{

}

void Room_3_Handle(void)
{
	if(!Game_IsPaused())
	{
		Game_UpdateStateMachine();
		Ball_Handle();
		Pad_HandleAll();
	}
	
	int keys = hidKeysDown();
	if(keys & KEY_START) Room_SetNumber(GAME_ROOM_MENU);
	if(keys & KEY_X) Game_Pause(!Game_IsPaused());
}

_3d_mode_e Room_3_3DMode(void)
{
	return GAME_MODE_3D;
}

//--------------------------------------------------------------------------------------------------
