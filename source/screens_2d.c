
//-------------------------------------------------------------------------------------------------------

#include <3ds.h>

#include "S3D/engine.h"
#include "ttf_console.h"
#include "game.h"
#include "rooms.h"
#include "utils.h"

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

void Draw2D_TopScreen(int screen)
{
	u8 * buf = S3D_BufferGet(screen);
	
	draw_number(buf,Game_PlayerScoreGet(0),10,240-32-10-1);
	
	draw_number(buf,Game_PlayerScoreGet(1),400-32-10-1,240-32-10-1);
}

void Draw2D_BottomScreen(void)
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
		case GAME_ROOM_2:
		case GAME_ROOM_3:
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
