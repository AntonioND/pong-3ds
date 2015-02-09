
//--------------------------------------------------------------------------------------------------

#include "../S3D/engine.h"
#include "../game.h"
#include "../rooms.h"
#include "../pad.h"
#include "../ball.h"

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

static void _room_DrawRoom1(int screen)
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
}

void Room_1_Draw(int screen)
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
	//m44_create_rotation_axis(&m, angle1, float2fx(0.58), float2fx(0.58), float2fx(0.58));
	S3D_ModelviewMatrixMultiply(screen, &m);
	
	m44_CreateRotationZ(&m,(x>>4));
	S3D_ModelviewMatrixMultiply(screen, &m);
	
	// Draw
	
	_room_DrawRoom1(screen); // Internal flush
	
	Pad_P2Draw(screen); // IA
	Ball_Draw(screen);
	Pad_P1Draw(screen); // Player
	S3D_PolygonListFlush(screen, 1);
}

//--------------------------------------------------------------------------------------------------

void Room_1_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax)
{
	if(xmin) *xmin = float2fx(-7.0);
	if(xmax) *xmax = float2fx(+7.0);
	if(ymin) *ymin = float2fx(-1.0);
	if(ymax) *ymax = float2fx(+3.0);
	if(zmin) *zmin = float2fx(-1.25);
	if(zmax) *zmax = float2fx(+12.75);
}

//--------------------------------------------------------------------------------------------------

void Room_1_Init(void)
{
	Ball_Init();
	Ball_SetDimensions(float2fx(1.5),float2fx(1.5),float2fx(1.5));
	Ball_SetColor(128,128,128);
	Ball_Reset();
	
	Pad_InitAll();
	Pad_P1SetDimensions(float2fx(3.5),float2fx(2.0),float2fx(1.0));
	Pad_P1SetColor(255,0,0);
	Pad_P2SetDimensions(float2fx(3.5),float2fx(2.0),float2fx(1.0));
	Pad_P2SetColor(255,255,0);
	Pad_ResetAll();
	
	Game_PlayerResetAll();
}

void Room_1_End(void)
{

}

void Room_1_Handle(void)
{
}

//--------------------------------------------------------------------------------------------------
