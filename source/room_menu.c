
//--------------------------------------------------------------------------------------------------

#include "S3D/engine.h"
#include "game.h"
#include "rooms.h"
#include "pad.h"
#include "ball.h"

//--------------------------------------------------------------------------------------------------

static int rotation = 0;

//--------------------------------------------------------------------------------------------------

static void __draw_ball(int screen, int r, int g, int b)
{
	S3D_PolygonColor(screen, r,g,b);
	
	S3D_PolygonBegin(screen, S3D_QUAD_STRIP);
	
	S3D_PolygonNormal(screen, float2fx(-1.0),float2fx(0.0),float2fx(0.0));
	
	S3D_PolygonVertex(screen, float2fx(-0.75),float2fx(-1),float2fx(-0.75));
	S3D_PolygonVertex(screen, float2fx(-0.75),float2fx(-1),float2fx(0.75));
	S3D_PolygonVertex(screen, float2fx(-0.75),float2fx(0.5),float2fx(0.75)); 
	S3D_PolygonVertex(screen, float2fx(-0.75),float2fx(0.5),float2fx(-0.75));

	S3D_PolygonNormal(screen, float2fx(0.0),float2fx(1.0),float2fx(0.0));

	S3D_PolygonVertex(screen, float2fx(0.75),float2fx(0.5),float2fx(0.75)); 			
	S3D_PolygonVertex(screen, float2fx(0.75),float2fx(0.5),float2fx(-0.75));
	
	S3D_PolygonBegin(screen, S3D_QUAD_STRIP);

	S3D_PolygonNormal(screen, float2fx(0.0),float2fx(0.0),float2fx(-1.0));
	
	S3D_PolygonVertex(screen, float2fx(-0.75),float2fx(-1),float2fx(-0.75));
	S3D_PolygonVertex(screen, float2fx(-0.75),float2fx(0.5),float2fx(-0.75));
	S3D_PolygonVertex(screen, float2fx(0.75),float2fx(0.5),float2fx(-0.75));
	S3D_PolygonVertex(screen, float2fx(0.75),float2fx(-1),float2fx(-0.75));

	S3D_PolygonNormal(screen, float2fx(1.0),float2fx(0.0),float2fx(0.0));

	S3D_PolygonVertex(screen, float2fx(0.75),float2fx(0.5),float2fx(0.75));
	S3D_PolygonVertex(screen, float2fx(0.75),float2fx(-1),float2fx(0.75));
	
	S3D_PolygonNormal(screen, float2fx(0.0),float2fx(0.0),float2fx(1.0));

	S3D_PolygonVertex(screen, float2fx(-0.75),float2fx(0.5),float2fx(0.75));
	S3D_PolygonVertex(screen, float2fx(-0.75),float2fx(-1),float2fx(0.75));
	
	S3D_PolygonListFlush(screen, 1);
}

void Room_Menu_Draw(int screen)
{
	{
		//---------------------------------------------------
		//                 Configuration
		//---------------------------------------------------
		
		S3D_SetCulling(screen, 1,0);
		
		m44 m;
		m44_CreateTranslation(&m,0,int2fx(0),int2fx(12));
		S3D_ModelviewMatrixSet(screen, &m);	
		m44_CreateRotationX(&m,-0x1800);
		S3D_ModelviewMatrixMultiply(screen, &m);
		m44_CreateScale(&m,int2fx(2),int2fx(2),int2fx(2));
		S3D_ModelviewMatrixMultiply(screen, &m);
		
		S3D_LightAmbientColorSet(screen, 64,64,64);
		
		S3D_LightEnable(screen, S3D_LIGHT_N(0));
		S3D_LightDirectionalColorSet(screen, 0, 192,192,192);
		S3D_LightDirectionalVectorSet(screen, 0, float2fx(-0.38),float2fx(-0.76),float2fx(0.53));
		
		//---------------------------------------------------
		//                 Draw cube
		//---------------------------------------------------
		
		m44_CreateRotationY(&m,rotation);
		S3D_ModelviewMatrixMultiply(screen, &m);
		
		__draw_ball(screen, 0,0,255);
	}
	
	{
		//---------------------------------------------------
		//                 Configuration
		//---------------------------------------------------
		
		S3D_SetCulling(screen, 1,0);
		
		circlePosition cp; // dx,dy (-0x9C ~ 0x9C)
		hidCircleRead(&cp);
	
		m44 m;
		m44_CreateTranslation(&m,0,int2fx(0), float2fx(2.5) + (cp.dy*float2fx(2.0))/0x9C);
		S3D_ModelviewMatrixSet(screen, &m);	
		
		S3D_LightAmbientColorSet(screen, 64,64,64);
		
		S3D_LightEnable(screen, S3D_LIGHT_N(0));
		S3D_LightDirectionalColorSet(screen, 0, 192,192,192);
		S3D_LightDirectionalVectorSet(screen, 0, float2fx(-0.38),float2fx(-0.76),float2fx(0.53));
		
		//---------------------------------------------------
		//                 Draw cube
		//---------------------------------------------------
		
		m44_CreateRotationZ(&m,-rotation);
		S3D_ModelviewMatrixMultiply(screen, &m);
		
		S3D_PolygonColor(screen, 255,0,0);
		
		S3D_PolygonBegin(screen, S3D_QUADS);
			S3D_PolygonVertex(screen, float2fx(+0.05),float2fx(+0.05),0);
			S3D_PolygonVertex(screen, float2fx(+0.05),float2fx(-0.05),0);
			S3D_PolygonVertex(screen, float2fx(-0.05),float2fx(-0.05),0);
			S3D_PolygonVertex(screen, float2fx(-0.05),float2fx(+0.05),0);
		S3D_PolygonListFlush(screen, 1);
	}
}

//--------------------------------------------------------------------------------------------------

void Room_Menu_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax)
{
	if(xmin) *xmin = 0;
	if(xmax) *xmax = 0;
	if(ymin) *ymin = 0;
	if(ymax) *ymax = 0;
	if(zmin) *zmin = 0;
	if(zmax) *zmax = 0;
}

//--------------------------------------------------------------------------------------------------

void Room_Menu_Init(void)
{
	Game_Pause(0);
	rotation = 0;
}

void Room_Menu_End(void)
{

}

void Room_Menu_Handle(void)
{
	int keys = hidKeysHeld();
	rotation += 0x100;
	if(keys & KEY_A) Room_SetNumber(GAME_ROOM_1);
	if(keys & KEY_B) Room_SetNumber(GAME_ROOM_2);
	if(keys & KEY_X) Room_SetNumber(GAME_ROOM_3);
}

_3d_mode_e Room_Menu_3DMode(void)
{
	return GAME_MODE_2D;
}

//--------------------------------------------------------------------------------------------------
