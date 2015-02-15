
//-------------------------------------------------------------------------------------------------------

#include <3ds.h>

#include "S3D/engine.h"
#include "ttf_console.h"
#include "game.h"
#include "rooms.h"
#include "utils.h"

//-------------------------------------------------------------------------------------------------------

#include "numbers_png_bin.h"
#define NUMBERS_WIDTH (28)
#define NUMBERS_HEIGHT (30)

static void _quad_blit_unsafe_24(u8 * buf, const u8 * src, int x, int y, int w, int h)
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

static void _quad_blit_unsafe_32(u8 * buf, const u8 * src, int x, int y, int w, int h)
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

static void _quad_blit_unsafe_real_alpha_32(u8 * buf, const u8 * src, int x, int y, int w, int h)
{
	u8 * linebuf = &(buf[(240*x+y)*3]);
	
	while(w--)
	{
		int i = h;
		u8 * p = linebuf; linebuf += 240*3;
		while(i--)
		{
			int a = *src++;
			if(a == 255)
			{
				*p++ = *src++; *p++ = *src++; *p++ = *src++;
			}
			else if(a == 0)
			{
				src += 3;
				p += 3;
			}
			else
			{
				int one_minus_alpha = 255 - a;
				
				int b = *src++;
				int g = *src++;
				int r = *src++;
				
				p[0] = ( (p[0] * one_minus_alpha) + b * a ) / 256;
				p[1] = ( (p[1] * one_minus_alpha) + g * a ) / 256;
				p[2] = ( (p[2] * one_minus_alpha) + r * a ) / 256;
				p += 3;
			}
		}
	}
}

static void draw_number(u8 * buf, int number, int x, int y)
{
	_quad_blit_unsafe_32(buf, numbers_png_bin+(NUMBERS_WIDTH*NUMBERS_HEIGHT*4)*number,
			x,y,NUMBERS_WIDTH,NUMBERS_HEIGHT);
}

//-------------------------------------------------------------------------------------------------------

#include "game_paused_png_bin.h"
#define GAME_PAUSED_WIDTH (175)
#define GAME_PAUSED_HEIGHT (16)

#include "player_n_wins_png_bin.h"
#define PLAYER_N_WINS_WIDTH (198)
#define PLAYER_N_WINS_HEIGHT (68)
#define PLAYER_N_WINS_PLAYER_OFFSET (183)

#include "get_ready_png_bin.h"
#define GET_READY_WIDTH (214)
#define GET_READY_HEIGHT (91)

#include "pong_3ds_png_bin.h"
#define PONG_3DS_WIDTH (354)
#define PONG_3DS_HEIGHT (48)

#include "by_antoniond_png_bin.h"
#define BY_ANTONIOND_WIDTH (136)
#define BY_ANTONIOND_HEIGHT (24)

void Draw2D_TopScreen(int screen)
{
	u8 * buf = S3D_BufferGet(screen);
	
	switch(Room_GetNumber())
	{
		case GAME_ROOM_MENU:
		{
			// Pong 3DS
			
			{
				int xbase = (400-PONG_3DS_WIDTH)/2;
				int ybase = (240-PONG_3DS_HEIGHT) - xbase;
				_quad_blit_unsafe_32(buf,pong_3ds_png_bin,xbase,ybase,
				                     PONG_3DS_WIDTH,PONG_3DS_HEIGHT);
			}
			
			// By AntonioND
			
			{
				int xbase = 400-BY_ANTONIOND_WIDTH-10;
				int ybase = 10;
				_quad_blit_unsafe_real_alpha_32(buf,by_antoniond_png_bin,xbase,ybase,
				                     BY_ANTONIOND_WIDTH,BY_ANTONIOND_HEIGHT);
			}
			
			break;
		}
		
		case GAME_ROOM_1:
		case GAME_ROOM_2:
		case GAME_ROOM_3:
		{
			// Game starts
			
			if(Game_StateMachineGet() == GAME_STARTING)
			{
				int xbase = (400-GET_READY_WIDTH)/2;
				int ybase = (240-GET_READY_HEIGHT)/2;
				_quad_blit_unsafe_32(buf,get_ready_png_bin,xbase,ybase,
				                     GET_READY_WIDTH,GET_READY_HEIGHT);
			}
			
			// Game result
			
			if(Game_StateMachineGet() == GAME_ENDING)
			{
				int xbase = (400-PLAYER_N_WINS_WIDTH)/2;
				int ybase = (240-PLAYER_N_WINS_HEIGHT)/2;
				_quad_blit_unsafe_32(buf,player_n_wins_png_bin,xbase,ybase,
				                     PLAYER_N_WINS_WIDTH,PLAYER_N_WINS_HEIGHT);
				
				if(Game_PlayerScoreGet(0) > 9)
					draw_number(buf,1,xbase+PLAYER_N_WINS_PLAYER_OFFSET,ybase+PLAYER_N_WINS_HEIGHT-NUMBERS_HEIGHT);
				else if(Game_PlayerScoreGet(1) > 9)
					draw_number(buf,2,xbase+PLAYER_N_WINS_PLAYER_OFFSET,ybase+PLAYER_N_WINS_HEIGHT-NUMBERS_HEIGHT);
			}
			
			// Draw score
			
			{
				if(Game_PlayerScoreGet(0) > 9)
				{
					draw_number(buf,Game_PlayerScoreGet(0)/10,10,240-NUMBERS_HEIGHT-10-1);
					draw_number(buf,Game_PlayerScoreGet(0)%10,10+NUMBERS_WIDTH,240-NUMBERS_HEIGHT-10-1);
				}
				else
				{
					draw_number(buf,Game_PlayerScoreGet(0),10,240-NUMBERS_WIDTH-10-1);
				}
				
				if(Game_PlayerScoreGet(1) > 9)
				{
					draw_number(buf,Game_PlayerScoreGet(1)/10,400-NUMBERS_WIDTH-10-1-NUMBERS_WIDTH,240-NUMBERS_HEIGHT-10-1);
					draw_number(buf,Game_PlayerScoreGet(1)%10,400-NUMBERS_WIDTH-10-1,240-NUMBERS_HEIGHT-10-1);
				}
				else
				{
					draw_number(buf,Game_PlayerScoreGet(1),400-NUMBERS_WIDTH-10-1,240-NUMBERS_HEIGHT-10-1);
				}
			}
			
			// Draw pause indicator
			
			if(Game_IsPaused())
			{
				_quad_blit_unsafe_32(buf,game_paused_png_bin,(400-GAME_PAUSED_WIDTH)/2,0,
				                     GAME_PAUSED_WIDTH,GAME_PAUSED_HEIGHT);
			}
			
			break;
		}
		
		default:
			break;
	}
}

//-------------------------------------------------------------------------------------------------------

#include "bottom_screen_png_bin.h"

void Draw2D_BottomScreen(void)
{
	u8 * buf = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);

	S3D_FramebufferFill(buf,0x00000000,GFX_BOTTOM);
	
	switch(Room_GetNumber())
	{
		case GAME_ROOM_MENU:
		{
			_quad_blit_unsafe_24(buf,bottom_screen_png_bin,0,0,320,240);
			
			Con_Print(buf,0,220-1,"(Antonio Niño Díaz)");
			
			Con_Print(buf,0,40,"A,B,X: Start.");
			Con_Print(buf,0,20,"Y: Screenshot.");
			Con_Print(buf,0,0,"SELECT:  Exit.");
			break;
		}
		
		case GAME_ROOM_2:
			Con_Print(buf,0,60,"A: Jump.");
			//Fall through
		case GAME_ROOM_1:
		case GAME_ROOM_3:
		{
			//Con_Print(buf,0,170,"3D Slider: %f   ",CONFIG_3D_SLIDERSTATE);
			Con_Print(buf,0,150,"FPS: %d %d ",Timing_GetFPS(0),Timing_GetFPS(1));
			Con_Print(buf,0,130,"CPU: %d%% %d%% ",(int)Timing_GetCPUUsage(0),(int)Timing_GetCPUUsage(1));
			
			Con_Print(buf,0,40,"START: Return.");
			Con_Print(buf,0,20,"Y: Screenshot.");
			Con_Print(buf,0,0,"SELECT:  Exit.");
			break;
		}
		
		default:
			break;
	}
}

//-------------------------------------------------------------------------------------------------------
