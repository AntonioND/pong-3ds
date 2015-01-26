
//-------------------------------------------------------------------------------------------------------

#include <3ds.h>
#include <stdlib.h>

#include "engine.h"
#include "utils.h"
#include "game.h"

//-------------------------------------------------------------------------------------------------------

typedef struct {
	s32 x,vx,ax;
	
	u32 x_col_old, z_col_old;
	u32 x_col, z_col;
} _pad_t;

typedef struct {
	s32 x,vx;
	s32 z,vz;
} _ball_t;

_pad_t pad, pad_ia;

_ball_t ball;

//-------------------------------------------------------------------------------------------------------

void draw_scenario(int r, int g, int b)
{
	// Surface...
	
	S3D_PolygonColor(r,g,b);
	
	S3D_PolygonBegin(S3D_QUADS);
	
	S3D_PolygonNormal(float2fx(0.0),float2fx(1.0),float2fx(0.0));
	
	S3D_PolygonVertex(float2fx(-7),float2fx(-1),float2fx(-0.5));
	S3D_PolygonVertex(float2fx(-7),float2fx(-1),float2fx(10.5));
	S3D_PolygonVertex(float2fx(7),float2fx(-1),float2fx(10.5));
	S3D_PolygonVertex(float2fx(7),float2fx(-1),float2fx(-0.5));
	
	S3D_PolygonListFlush(0);
	
	// Lines...
	
	S3D_PolygonColor(r/2,g/2,b/2);
	
	S3D_PolygonNormal(float2fx(0.0),float2fx(1.0),float2fx(0.0));
	
	S3D_PolygonBegin(S3D_LINES);
	
	S3D_PolygonVertex(float2fx(-7),float2fx(-1),float2fx(-0.5));
	S3D_PolygonVertex(float2fx(7),float2fx(-1),float2fx(-0.5));
	
	S3D_PolygonVertex(float2fx(7),float2fx(-1),float2fx(10.5));
	S3D_PolygonVertex(float2fx(-7),float2fx(-1),float2fx(10.5)); 
	
	S3D_PolygonVertex(float2fx(-7),float2fx(-1),float2fx(5.0)); 	
	S3D_PolygonVertex(float2fx(7),float2fx(-1),float2fx(5.0));
	
	S3D_PolygonListFlush(0);
	
	// Borders ...
	
	S3D_PolygonColor(r,g,b);
	
	S3D_PolygonBegin(S3D_QUADS);
	
	S3D_PolygonNormal(float2fx(1.0),float2fx(0.0),float2fx(0.0));
	
	S3D_PolygonVertex(float2fx(-7),float2fx(1),float2fx(-0.5));
	S3D_PolygonVertex(float2fx(-7),float2fx(1),float2fx(10.5));
	S3D_PolygonVertex(float2fx(-7),float2fx(-1),float2fx(10.5));
	S3D_PolygonVertex(float2fx(-7),float2fx(-1),float2fx(-0.5));
	
	S3D_PolygonNormal(float2fx(-1.0),float2fx(0.0),float2fx(0.0));
	
	S3D_PolygonVertex(float2fx(7),float2fx(-1),float2fx(-0.5));
	S3D_PolygonVertex(float2fx(7),float2fx(-1),float2fx(10.5));
	S3D_PolygonVertex(float2fx(7),float2fx(1),float2fx(10.5));
	S3D_PolygonVertex(float2fx(7),float2fx(1),float2fx(-0.5));
	
	S3D_PolygonListFlush(1);
}

void draw_ball(int r, int g, int b)
{
	S3D_PolygonColor(r,g,b);
	
	S3D_PolygonBegin(S3D_QUAD_STRIP);
	
	S3D_PolygonNormal(float2fx(-1.0),float2fx(0.0),float2fx(0.0));
	
	S3D_PolygonVertex(float2fx(-0.75),float2fx(-1),float2fx(-0.75));
	S3D_PolygonVertex(float2fx(-0.75),float2fx(-1),float2fx(0.75));
	S3D_PolygonVertex(float2fx(-0.75),float2fx(0.5),float2fx(0.75)); 
	S3D_PolygonVertex(float2fx(-0.75),float2fx(0.5),float2fx(-0.75));

	S3D_PolygonNormal(float2fx(0.0),float2fx(1.0),float2fx(0.0));

	S3D_PolygonVertex(float2fx(0.75),float2fx(0.5),float2fx(0.75)); 			
	S3D_PolygonVertex(float2fx(0.75),float2fx(0.5),float2fx(-0.75));
	
	S3D_PolygonBegin(S3D_QUAD_STRIP);

	S3D_PolygonNormal(float2fx(0.0),float2fx(0.0),float2fx(-1.0));
	
	S3D_PolygonVertex(float2fx(-0.75),float2fx(-1),float2fx(-0.75));
	S3D_PolygonVertex(float2fx(-0.75),float2fx(0.5),float2fx(-0.75));
	S3D_PolygonVertex(float2fx(0.75),float2fx(0.5),float2fx(-0.75));
	S3D_PolygonVertex(float2fx(0.75),float2fx(-1),float2fx(-0.75));

	S3D_PolygonNormal(float2fx(1.0),float2fx(0.0),float2fx(0.0));

	S3D_PolygonVertex(float2fx(0.75),float2fx(0.5),float2fx(0.75));
	S3D_PolygonVertex(float2fx(0.75),float2fx(-1),float2fx(0.75));
	
	S3D_PolygonNormal(float2fx(0.0),float2fx(0.0),float2fx(1.0));

	S3D_PolygonVertex(float2fx(-0.75),float2fx(0.5),float2fx(0.75));
	S3D_PolygonVertex(float2fx(-0.75),float2fx(-1),float2fx(0.75));
}

void draw_pad(int r, int g, int b)
{
	S3D_PolygonColor(r,g,b);
	
	S3D_PolygonBegin(S3D_QUAD_STRIP);
	
	S3D_PolygonNormal(float2fx(-1.0),float2fx(0.0),float2fx(0.0));
	
	S3D_PolygonVertex(float2fx(-2),float2fx(-1),float2fx(-0.5));
	S3D_PolygonVertex(float2fx(-2),float2fx(-1),float2fx(0.5));
	S3D_PolygonVertex(float2fx(-2),float2fx(1),float2fx(0.5)); 
	S3D_PolygonVertex(float2fx(-2),float2fx(1),float2fx(-0.5));
	
	S3D_PolygonNormal(float2fx(0.0),float2fx(1.0),float2fx(0.0));
	
	S3D_PolygonVertex(float2fx(2),float2fx(1),float2fx(0.5)); 			
	S3D_PolygonVertex(float2fx(2),float2fx(1),float2fx(-0.5));
	
	S3D_PolygonBegin(S3D_QUAD_STRIP);
	
	S3D_PolygonNormal(float2fx(0.0),float2fx(0.0),float2fx(-1.0));
	
	S3D_PolygonVertex(float2fx(-2),float2fx(-1),float2fx(-0.5));
	S3D_PolygonVertex(float2fx(-2),float2fx(1),float2fx(-0.5));
	S3D_PolygonVertex(float2fx(2),float2fx(1),float2fx(-0.5));
	S3D_PolygonVertex(float2fx(2),float2fx(-1),float2fx(-0.5));
	
	S3D_PolygonNormal(float2fx(1.0),float2fx(0.0),float2fx(0.0));

	S3D_PolygonVertex(float2fx(2),float2fx(1),float2fx(0.5));
	S3D_PolygonVertex(float2fx(2),float2fx(-1),float2fx(0.5));
	
	S3D_PolygonNormal(float2fx(0.0),float2fx(0.0),float2fx(1.0));

	S3D_PolygonVertex(float2fx(-2),float2fx(1),float2fx(0.5));
	S3D_PolygonVertex(float2fx(-2),float2fx(-1),float2fx(0.5));
}


void Game_DrawScene(void)
{
	//3D Stuff
	{
		//---------------------------------------------------
		//                 Configuration
		//---------------------------------------------------
		
		S3D_SetCulling(1,0);
		
		m44 m;
		m44_CreateTranslation(&m,0,int2fx(-2),int2fx(12));
		S3D_ModelviewMatrixSet(&m);
		m44_CreateRotationX(&m,-0x1000);
		S3D_ModelviewMatrixMultiply(&m);
		
		S3D_LightAmbientColorSet(64,64,64);
		
		S3D_LightEnable(S3D_LIGHT_N(0));
		S3D_LightDirectionalColorSet(0, 192,192,192);
		S3D_LightDirectionalVectorSet(0, float2fx(-0.38),float2fx(-0.76),float2fx(0.53));
		
		//---------------------------------------------------
		//                 Draw scenario
		//---------------------------------------------------
		
		// Camera rotation effect...
		m44_CreateRotationY(&m,pad.x);
		//m44_create_rotation_axis(&m, angle1, float2fx(0.58), float2fx(0.58), float2fx(0.58));
		S3D_ModelviewMatrixMultiply(&m);
		
		m44_CreateRotationZ(&m,-pad.x);
		S3D_ModelviewMatrixMultiply(&m);
		
		draw_scenario(0,0,255);
		
		//---------------------------------------------------
		//                     IA
		//---------------------------------------------------
		
		S3D_ModelviewMatrixPush();
		
		m44_CreateTranslation(&m,pad_ia.x,0,int2fx(10));
		S3D_ModelviewMatrixMultiply(&m);
		
		draw_pad(255,0,0);
		
		S3D_ModelviewMatrixPop();
		
		//---------------------------------------------------
		//                     Ball
		//---------------------------------------------------
		
		S3D_ModelviewMatrixPush();
		
		m44_CreateTranslation(&m,ball.x,0,ball.z);
		S3D_ModelviewMatrixMultiply(&m);
		
		draw_ball(128,128,128);
		
		S3D_ModelviewMatrixPop();
		
		//---------------------------------------------------
		//                     Player
		//---------------------------------------------------
		
		S3D_ModelviewMatrixPush();
		
		m44_CreateTranslation(&m,pad.x,0,0);
		S3D_ModelviewMatrixMultiply(&m);
		
		/*
		int keys = hidKeysHeld();
		if(keys & KEY_A) { int i; for(i = 0; i < 5; i++) draw_pad(0,255,0); };
		if(keys & KEY_B) { int i; for(i = 0; i < 5; i++) draw_pad(0,255,0); };
		if(keys & KEY_L) { int i; for(i = 0; i < 5; i++) draw_pad(0,255,0); };
		if(keys & KEY_R) { int i; for(i = 0; i < 5; i++) draw_pad(0,255,0); };
		*/
		
		draw_pad(0,255,0);
		
		S3D_ModelviewMatrixPop();
	}
	
	//2D stuff
	{
		
	}
}




//-------------------------------------------------------------------------------------------------------

void Game_Init(void)
{
	pad.x = 0; pad.vx = 0; pad.ax = 0;
	pad.x_col_old = 0; pad.z_col_old = 0; pad.x_col = 0; pad.z_col = 0;
	pad_ia.x = 0; pad_ia.vx = 0; pad_ia.ax = 0;
	pad_ia.x_col_old = 0; pad_ia.z_col_old = 0; pad_ia.x_col = 0; pad_ia.z_col = 0;
	
	ball.x = int2fx(0);
	ball.z = int2fx(5);	
	ball.vx = (int2fx(1)>>4) + (fast_rand() & ((int2fx(1)>>3)-1));
	ball.vx = fast_rand() & 1 ? ball.vx : -ball.vx;
	ball.vz = (int2fx(1)>>3) + (fast_rand() & ((int2fx(1)>>3)-1));
	ball.vz = fast_rand() & 1 ? ball.vz : -ball.vz;
}

static inline u32 _segments_overlap(s32 amin, s32 amax, s32 bmin, s32 bmax)
{
	if(amin < bmin) { if(amax < bmin) return 0; } //completely out (left)
	else if(amin > bmax) return 0; // completely out (right)
	return 1; //partially or totally overlapping
}

void Game_HandleAndDraw(int keys)
{
	//---------------------------------------------------
	//                 Handle game
	//---------------------------------------------------
	
	//Player
	
	if(keys & KEY_RIGHT)
	{
		if(pad.vx < 0) pad.vx = 0;
		else if(pad.vx < 128) pad.vx += 32;
	}
	else if(keys & KEY_LEFT)
	{
		if(pad.vx > 0) pad.vx = 0;
		else if(pad.vx > -128) pad.vx -= 32;
	}
	else
	{
		if(pad.vx > 0) pad.vx -= 16; 
		else if(pad.vx < 0) pad.vx += 16;
	}
	pad.x += pad.vx;
	if(pad.x < int2fx(-5)) { pad.x = int2fx(-5); pad.vx = 0; }
	else if(pad.x > int2fx(5)) { pad.x = int2fx(5); pad.vx = 0; }
	
	//IA
	
	if(ball.z > int2fx(3) && ball.vz > 0)
	{
		if((pad_ia.x - ball.x) < int2fx(-1))
		{
			if(pad_ia.vx < 0) pad_ia.vx = 0;
			else if(pad_ia.vx < 128) pad_ia.vx += 32;
		}
		else if((pad_ia.x - ball.x) > int2fx(1)) 
		{
			if(pad_ia.vx > 0) pad_ia.vx = 0;
			else if(pad_ia.vx > -128) pad_ia.vx -= 32;
		}
		else
		{
			if(pad_ia.vx > 0) pad_ia.vx -= 16; 
			else if(pad_ia.vx < 0) pad_ia.vx += 16;
		}
	}
	else
	{
		if(pad_ia.vx > 0) pad_ia.vx -= 16; 
		else if(pad_ia.vx < 0) pad_ia.vx += 16;
	}
	pad_ia.x += pad_ia.vx;
	if(pad_ia.x < int2fx(-5)) { pad_ia.x = int2fx(-5); pad_ia.vx = 0; }
	else if(pad_ia.x > int2fx(5)) { pad_ia.x = int2fx(5); pad_ia.vx = 0; }
	
	//Ball
	
	pad.x_col_old = pad.x_col; pad.z_col_old = pad.z_col;
	pad_ia.x_col_old = pad_ia.x_col; pad_ia.z_col_old = pad_ia.z_col;
	
	ball.x += ball.vx;
	if(abs(ball.x) > float2fx(6.25)) ball.vx = -ball.vx;
	ball.z += ball.vz;
	
	pad.z_col = 0;
	pad_ia.z_col = 0;
	pad.x_col = 0;
	pad_ia.x_col = 0;
	
	if(_segments_overlap(ball.z-float2fx(0.75),ball.z+float2fx(0.75),float2fx(-0.5),float2fx(0.5)))
	{
		pad.z_col = 1;
	}
	else if(_segments_overlap(ball.z-float2fx(0.75),ball.z+float2fx(0.75),float2fx(10-0.5),float2fx(10+0.5)))
	{
		pad_ia.z_col = 1;
	}
	
	if(_segments_overlap(ball.x-float2fx(0.75),ball.x+float2fx(0.75),
				pad.x-float2fx(2),pad.x+float2fx(2)))
	{
		pad.x_col = 1;
	}
	
	if(_segments_overlap(ball.x-float2fx(0.75),ball.x+float2fx(0.75),
				pad_ia.x-float2fx(2),pad_ia.x+float2fx(2)))
	{
		pad_ia.x_col = 1;
	}
	
	if(pad.x_col && pad.z_col)
	{
		if(pad.x_col ^ pad.x_col_old) ball.vx = -ball.vx;
		if(pad.z_col ^ pad.z_col_old) ball.vz = -ball.vz;
	}
	else if(pad_ia.x_col && pad_ia.z_col)
	{
		if(pad_ia.x_col ^ pad_ia.x_col_old) ball.vx = -ball.vx;
		if(pad_ia.z_col ^ pad_ia.z_col_old) ball.vz = -ball.vz;
	}
	
	if(ball.z > float2fx(10.375)) ball.vz = -ball.vz;
	else if(ball.z < float2fx(-0.375)) ball.vz = -ball.vz;
	
	//if(ball.z > float2fx(10.375)) score_player ++;
	//else if(ball.z < float2fx(-0.375)) score_ia ++;
	
	//---------------------------------------------------
	//
	//---------------------------------------------------
}

void Game_End(void)
{
	
}

//-------------------------------------------------------------------------------------------------------
