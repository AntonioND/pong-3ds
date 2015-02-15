
//--------------------------------------------------------------------------------------------------

#include <string.h>

#include "S3D/engine.h"
#include "game.h"
#include "rooms.h"
#include "pad.h"
#include "ball.h"
#include "utils.h"

//--------------------------------------------------------------------------------------------------

typedef struct {
	int h, vh, ah;
} _bumper_s;

static _bumper_s Bumper[4][2];

static int bumper_enabled = 0;

//--------------------------------------------------------------------------------------------------

static void Bumper_Init(void)
{
	memset(Bumper,0,sizeof(Bumper));
	bumper_enabled = 0;
}

static void Bumper_Handle(void)
{
	if(bumper_enabled) return;
	
	int ballxmin, ballxmax, ballymin, ballymax, ballzmin, ballzmax;
	Ball_GetBounds(&ballxmin,&ballxmax,&ballymin,&ballymax,&ballzmin,&ballzmax);

	int i,j;
	for(i = 0; i < 2; i ++) for(j = 0; j < 4; j++)
	{
		if(ballymin < float2fx(-0.99))
		{
			if((fast_rand() & 31) == 0)
			{
				int xmin = float2fx(-7.0) + float2fx(3.5)*j + float2fx(0.1);
				int zmin = float2fx(-1.25) + float2fx(3.5) + float2fx(3.5)*i + float2fx(0.1);
				int xmax = xmin + float2fx(3.3);
				int zmax = zmin + float2fx(3.3);
				
				if(_segments_overlap(xmin,xmax, ballxmin,ballxmax) && _segments_overlap(zmin,zmax, ballzmin,ballzmax))
				{
					Bumper[j][i].vh = float2fx(0.3);
					Bumper[j][i].ah = -float2fx(0.015);
					Ball_Bounce(float2fx(0.3),-float2fx(0.015));
					bumper_enabled = 1;
				}
			}
		}
	}
}

static void Bumper_Gravity(void)
{
	if(bumper_enabled == 0) return;
	
	int i,j;
	for(i = 0; i < 2; i ++) for(j = 0; j < 4; j++)
	{
		if(Bumper[j][i].ah)
		{
			Bumper[j][i].h += Bumper[j][i].vh;
			Bumper[j][i].vh += Bumper[j][i].ah;
			
			if(Bumper[j][i].h < 0)
			{
				Bumper[j][i].h = 0;
				Bumper[j][i].vh = 0;
				Bumper[j][i].ah = 0;
				bumper_enabled = 0;
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------

static void _room_DrawRoom2_Borders(int screen)
{
	int i;

	S3D_PolygonBegin(screen, S3D_QUADS);
	
	for(i = 0; i < 7; i++)
	{
		S3D_PolygonColorAlpha(screen, 0,0,255,(i&1)?128:255);
		
		int zbase = float2fx(-1.25) + float2fx(2.0)*i;
		
		int ymax = (i&1) ? float2fx(2.5) : float2fx(3.0);
		
		S3D_PolygonNormal(screen, float2fx(1.0),float2fx(0.0),float2fx(0.0));
		
		S3D_PolygonVertex(screen, float2fx(-7),ymax,zbase);
		S3D_PolygonVertex(screen, float2fx(-7),ymax,zbase + float2fx(2.0));
		S3D_PolygonVertex(screen, float2fx(-7),float2fx(-1.0),zbase + float2fx(2.0));
		S3D_PolygonVertex(screen, float2fx(-7),float2fx(-1.0),zbase);
		
		S3D_PolygonNormal(screen, float2fx(-1.0),float2fx(0.0),float2fx(0.0));
		
		S3D_PolygonVertex(screen, float2fx(7),float2fx(-1.0),zbase);
		S3D_PolygonVertex(screen, float2fx(7),float2fx(-1.0),zbase + float2fx(2.0));
		S3D_PolygonVertex(screen, float2fx(7),ymax,zbase + float2fx(2.0));
		S3D_PolygonVertex(screen, float2fx(7),ymax,zbase);
	}
}

static void _room_DrawRoom2_Floor(int screen)
{
	int j;

	S3D_PolygonBegin(screen, S3D_QUADS);
	
	// SECOND: Surface, starting from the furthest polygons
	
	for(j = 0; j < 4; j++)
	{
		int xbase = float2fx(-7) + float2fx(3.5) * j;
		
		if(j&1) S3D_PolygonColor(screen, 64,64,128);
		else S3D_PolygonColor(screen, 96,96,192);
		
		S3D_PolygonNormal(screen, float2fx(0.0),float2fx(1.0),float2fx(0.0));
		
		S3D_PolygonVertex(screen, xbase, float2fx(-1), float2fx(-1.25));
		S3D_PolygonVertex(screen, xbase, float2fx(-1), float2fx(-1.25) + float2fx(3.4));
		S3D_PolygonVertex(screen, xbase + float2fx(3.5), float2fx(-1), float2fx(-1.25) + float2fx(3.4));
		S3D_PolygonVertex(screen, xbase + float2fx(3.5), float2fx(-1), float2fx(-1.25));
		
		if(j&1) S3D_PolygonColor(screen, 96,96,192);
		else S3D_PolygonColor(screen, 64,64,128);
		
		S3D_PolygonNormal(screen, float2fx(0.0),float2fx(1.0),float2fx(0.0));
		
		S3D_PolygonVertex(screen, xbase, float2fx(-1), float2fx(12.75) - float2fx(3.4));
		S3D_PolygonVertex(screen, xbase, float2fx(-1), float2fx(12.75));
		S3D_PolygonVertex(screen, xbase + float2fx(3.5), float2fx(-1), float2fx(12.75));
		S3D_PolygonVertex(screen, xbase + float2fx(3.5), float2fx(-1), float2fx(12.75) - float2fx(3.4));
	}
}


static void _room_DrawRoom2_Bumpers(int screen)
{
	int i,j;

	S3D_PolygonBegin(screen, S3D_QUADS);
	
	for(j = 0; j < 4; j++) for(i = 1; i >= 0; i--)
	{
		S3D_PolygonColorAlpha(screen, ((j^i)&1) ? 255 : 192,0,0, 128);
		
		S3D_PolygonNormal(screen, float2fx(0.0),float2fx(1.0),float2fx(0.0));
		
		int xbase = float2fx(-7.0) + float2fx(3.5)*j + float2fx(0.1);
		int zbase = float2fx(-1.25) + float2fx(3.5) + float2fx(3.5)*i + float2fx(0.1);
		
		int ybase = Bumper[j][i].h + float2fx(-1);
		
		S3D_PolygonVertex(screen, xbase, ybase, zbase);
		S3D_PolygonVertex(screen, xbase, ybase, zbase + float2fx(3.3));
		S3D_PolygonVertex(screen, xbase + float2fx(3.3), ybase, zbase + float2fx(3.3));
		S3D_PolygonVertex(screen, xbase + float2fx(3.3), ybase, zbase);
	}
}

void Room_2_Draw(int screen)
{
	// Configure
	
	S3D_SetCulling(screen, 1,0);
	
	m44 m;
	m44_CreateTranslation(&m,0,int2fx(-2),int2fx(12));
	S3D_ModelviewMatrixSet(screen, &m);	
	m44_CreateRotationX(&m,-0x1800);
	S3D_ModelviewMatrixMultiply(screen, &m);
	
	S3D_LightAmbientColorSet(screen, 64,64,64);
	
	S3D_LightEnable(screen, S3D_LIGHT_N(0));
	S3D_LightDirectionalColorSet(screen, 0, 192,192,192);
	S3D_LightDirectionalVectorSet(screen, 0, float2fx(-0.38),float2fx(-0.76),float2fx(0.53));
	
	// Move camera
	
	int x;
	Pad_P1GetPosition(&x,NULL,NULL);
	
	// Camera rotation effect...
	m44_CreateRotationY(&m,-(x>>4));
	S3D_ModelviewMatrixMultiply(screen, &m);
	
	m44_CreateRotationZ(&m,(x>>4));
	S3D_ModelviewMatrixMultiply(screen, &m);
	
	// Draw
	
	_room_DrawRoom2_Borders(screen);
	_room_DrawRoom2_Floor(screen);
	S3D_PolygonListFlush(screen, 1);
	
	Pad_P2DrawShadows(screen); // Internal flush
	Pad_P2Draw(screen); // IA
	S3D_PolygonListFlush(screen, 1);
	
	Ball_DrawShadows(screen); // Internal flush
	
	_room_DrawRoom2_Bumpers(screen);
	S3D_PolygonListFlush(screen, 1);
	
	Pad_P1DrawShadows(screen); // Internal flush
	
	Pad_P1Draw(screen); // Player
	Ball_Draw(screen);
	S3D_PolygonListFlush(screen, 1);
}

//--------------------------------------------------------------------------------------------------

void Room_2_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax)
{
	if(xmin) *xmin = float2fx(-7.0);
	if(xmax) *xmax = float2fx(+7.0);
	if(ymin) *ymin = float2fx(-1.0);
	if(ymax) *ymax = float2fx(+10.0);
	if(zmin) *zmin = float2fx(-1.25);
	if(zmax) *zmax = float2fx(+12.75);
}

//--------------------------------------------------------------------------------------------------

void Room_2_Init(void)
{
	Ball_Init();
	Ball_SetDimensions(float2fx(1.0),float2fx(1.0),float2fx(1.0));
	Ball_SetColor(128,128,128,255);
	Ball_Reset();
	
	Pad_InitAll();
	Pad_P1SetDimensions(float2fx(3.0),float2fx(1.5),float2fx(0.75));
	Pad_P1SetColor(255,0,0,255);
	Pad_P2SetDimensions(float2fx(3.0),float2fx(1.5),float2fx(0.75));
	Pad_P2SetColor(255,255,0,255);
	Pad_ResetAll();
	
	Game_PlayerResetAll();
	
	Game_StateMachineReset();
	
	Bumper_Init();
}

void Room_2_End(void)
{

}

void Room_2_Handle(void)
{
	if(!Game_IsPaused())
	{
		Game_UpdateStateMachine();
		Ball_Handle();
		Pad_HandleAll();
		
		Bumper_Gravity();
		
		if(Game_StateMachineGet() == GAME_NORMAL_PLAY)
			Bumper_Handle();
		else if( (Game_StateMachineGet() == GAME_INITIAL_DELAY) || (Game_StateMachineGet() == GAME_STARTING) )
			Bumper_Init();
	}
	
	int keys = hidKeysDown();
	if(keys & KEY_START) Room_SetNumber(GAME_ROOM_MENU);
	if(keys & KEY_X) Game_Pause(!Game_IsPaused());
}

_3d_mode_e Room_2_3DMode(void)
{
	return GAME_MODE_2D_BOUNCE;
}

//--------------------------------------------------------------------------------------------------
