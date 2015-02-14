
//--------------------------------------------------------------------------------------------------

#include "../S3D/engine.h"
#include "../game.h"
#include "../rooms.h"
#include "../pad.h"
#include "../ball.h"

//--------------------------------------------------------------------------------------------------

static int rotation = 0;
static int x[4], y[4];
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
}

void Room_Menu_Draw(int screen)
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
	
	
	
	
	S3D_2D_QuadFill(S3D_BufferGet(screen),
		x[0],y[0],x[1],y[1],x[2],y[2],x[3],y[3],
		255,255,255);
		
S3D_2D_Plot(S3D_BufferGet(screen),x[0],y[0], 255,0,255,255);
S3D_2D_Plot(S3D_BufferGet(screen),x[1],y[1], 255,0,255,255);
S3D_2D_Plot(S3D_BufferGet(screen),x[2],y[2], 255,0,255,255);
S3D_2D_Plot(S3D_BufferGet(screen),x[3],y[3], 255,0,255,255);

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
	rotation = 0;
}

void Room_Menu_End(void)
{

}

void Room_Menu_Handle(void)
{
	int keys = hidKeysHeld();
	rotation += 0x100;
	//if(keys & KEY_A) Room_SetNumber(GAME_ROOM_1);
	////if(keys & KEY_B) Room_SetNumber(GAME_ROOM_2);
	//if(keys & KEY_X) Room_SetNumber(GAME_ROOM_3);
	
	int i = 0;
	if(keys & KEY_A) i = 1;
	if(keys & KEY_B) i = 2;
	if(keys & KEY_Y) i = 3;

	keys = hidKeysDown();
	if(keys & KEY_LEFT) { if(x[i] > 0) x[i]--; }
	if(keys & KEY_RIGHT) { if(x[i] < (400-1)) x[i]++; }
	if(keys & KEY_DOWN) { if(y[i] > 0) y[i]--; }
	if(keys & KEY_UP) { if(y[i] < (240-1)) y[i]++; }
}

int Room_Menu_3DMovementEnabled(void)
{
	return 0;
}

//--------------------------------------------------------------------------------------------------
