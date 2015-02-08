
//-------------------------------------------------------------------------------------------------------

#include <3ds.h>
#include <stdlib.h>

#include "S3D/engine.h"
#include "utils.h"
#include "game.h"
#include "pad.h"
#include "ball.h"
#include "room.h"
#include "ttf_console.h"

//-------------------------------------------------------------------------------------------------------

#include "bottom_screen_png_bin.h"

#include "numbers_png_bin.h"
#define NUMBER_SIZE_PX (32)

void _quad_blit_unsafe_24(u8 * buf, const u8 * src, int x, int y, int w, int h)
{
	u8 * linebuf = &(buf[(240*x+y)*3]);
	
	while(w--)
	{
		int i = h;
		u8 * p = linebuf; linebuf += 240*3;
		while(i--)
		{
			*p++ = *src++; *p++ = *src++; *p++ = *src++;
		}
	}
}

void _quad_blit_unsafe_32(u8 * buf, const u8 * src, int x, int y, int w, int h)
{
	u8 * linebuf = &(buf[(240*x+y)*3]);
	
	while(w--)
	{
		int i = h;
		u8 * p = linebuf; linebuf += 240*3;
		while(i--)
		{
			if(*src++) // alpha
			{
				*p++ = *src++; *p++ = *src++; *p++ = *src++;
			}
			else
			{
				p += 3; src += 3;
			}
		}
	}
}


void draw_number(u8 * buf, int number, int x, int y)
{
	_quad_blit_unsafe_32(buf, numbers_png_bin+(NUMBER_SIZE_PX*NUMBER_SIZE_PX*4)*number,
			x,y,NUMBER_SIZE_PX,NUMBER_SIZE_PX);
}

//-------------------------------------------------------------------------------------------------------

void __draw_ball(int screen, int r, int g, int b)
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

//-------------------------------------------------------------------------------------------------------

struct {
	int r,g,b;
	int vr,vg,vb;
} _clear_color;

void ClearColorInit(void)
{
	_clear_color.r = _clear_color.g = _clear_color.b = 64;
	_clear_color.vr = (fast_rand() & 3) + 1;
	_clear_color.vg = (fast_rand() & 3) + 1;
	_clear_color.vb = (fast_rand() & 3) + 1;
	if(fast_rand() & 1) _clear_color.vr = -_clear_color.vr;
	if(fast_rand() & 1) _clear_color.vg = -_clear_color.vg;
	if(fast_rand() & 1) _clear_color.vb = -_clear_color.vb;
}

void ClearColorHandle(void)
{
	_clear_color.r += _clear_color.vr;
	_clear_color.g += _clear_color.vg;
	_clear_color.b += _clear_color.vb;
	
	if(_clear_color.r > 128) { _clear_color.r = 128; _clear_color.vr = -_clear_color.vr; }
	else if(_clear_color.r < 0) { _clear_color.r = 0; _clear_color.vr = -_clear_color.vr; }
	
	if(_clear_color.g > 128) { _clear_color.g = 128; _clear_color.vg = -_clear_color.vg; }
	else if(_clear_color.g < 0) { _clear_color.g = 0; _clear_color.vg = -_clear_color.vg; }
	
	if(_clear_color.b > 128) { _clear_color.b = 128; _clear_color.vb = -_clear_color.vb; }
	else if(_clear_color.b < 0) { _clear_color.b = 0; _clear_color.vb = -_clear_color.vb; }
}

//-------------------------------------------------------------------------------------------------------

int rotation = 0;

void Game_DrawMenu(int screen)
{
	S3D_FramebuffersClearTopScreen(screen, _clear_color.r,_clear_color.g,_clear_color.b);

	//3D Stuff
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
	
	//2D Stuff
	{
		
	}
}

void Game_DrawRoom1(int screen)
{
	S3D_FramebuffersClearTopScreen(screen, _clear_color.r,_clear_color.g,_clear_color.b);

	//3D Stuff
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
		m44_CreateRotationY(&m,-x);
		//m44_create_rotation_axis(&m, angle1, float2fx(0.58), float2fx(0.58), float2fx(0.58));
		S3D_ModelviewMatrixMultiply(screen, &m);
		
		m44_CreateRotationZ(&m,x);
		S3D_ModelviewMatrixMultiply(screen, &m);
		
		// Draw
		
		Room_Draw(screen); // Internal flush
		
		Pad_P2Draw(screen); // IA
		Ball_Draw(screen);
		Pad_P1Draw(screen); // Player
		S3D_PolygonListFlush(screen, 1);
	}
	
	//2D stuff
	{
		u8 * buf = S3D_BufferGet(screen);
		
		draw_number(buf,4,10,240-32-10-1);
		
		draw_number(buf,5,400-32-10-1,240-32-10-1);
	}
}


//-------------------------------------------------------------------------------------------------------

void Game_DrawScreenTop(int screen)
{
	switch(Room_GetNumber())
	{
		case GAME_ROOM_MENU:
			Game_DrawMenu(screen);
			break;
			
		case GAME_ROOM_1:
			Game_DrawRoom1(screen);
			break;
			
		default:
			break;
	}
}

void Game_DrawScreenBottom(void)
{
	u8 * buf = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);

	switch(Room_GetNumber())
	{
		case GAME_ROOM_MENU:
		{
			_quad_blit_unsafe_24(buf,bottom_screen_png_bin,0,0,320,240);
			
			Con_Print(buf,0,220-1,"Pong 3DS by AntonioND");
			Con_Print(buf,0,200-1,"(Antonio Niño Díaz)");
			
			Con_Print(buf,0,40,"A: Start.");
			Con_Print(buf,0,20,"SELECT: Screenshot.");
			Con_Print(buf,0,0,"START:  Exit.");
			break;
		}
		
		case GAME_ROOM_1:
		{
			//Con_Print(buf,0,170,"3D Slider: %f   ",CONFIG_3D_SLIDERSTATE);
			Con_Print(buf,0,150,"FPS: %d %d ",Timing_GetFPS(0),Timing_GetFPS(1));
			Con_Print(buf,0,130,"CPU: %d%% %d%% ",(int)Timing_GetCPUUsage(0),(int)Timing_GetCPUUsage(1));
			break;
		}
		
		default:
			break;
	}
}

//-------------------------------------------------------------------------------------------------------

void Game_Init(void)
{
	ClearColorInit();
	
	Room_Init();
}

void Game_Handle(void)
{
	ClearColorHandle();
	
	int keys = hidKeysHeld();
	
	switch(Room_GetNumber())
	{
		case GAME_ROOM_MENU:
		{
			rotation += 0x100;
			if(keys & KEY_A) Room_SetNumber(GAME_ROOM_1);
			return;
			break;
		}
		
		case GAME_ROOM_1:
		{
			break;
		}
		
		default:
			return;
	}
	
	// Handle game
	
	Ball_Handle();
	Pad_HandleAll();
	Room_Handle();
}

void Game_End(void)
{
	// Nothing here...
}

//-------------------------------------------------------------------------------------------------------
