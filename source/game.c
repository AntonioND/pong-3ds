
//-------------------------------------------------------------------------------------------------------

#include <3ds.h>
#include <stdlib.h>
#include <string.h>

#include "S3D/engine.h"
#include "utils.h"
#include "game.h"
#include "pad.h"
#include "ball.h"
#include "rooms.h"
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

typedef struct {
	int score;
} _player_info_s;

_player_info_s PLAYER[2];

void Game_PlayerResetAll(void)
{
	memset(&PLAYER,0,sizeof(PLAYER));
}

void Game_PlayerScoreIncrease(int player)
{
	PLAYER[player].score++;
}

int Game_PlayerScoreGet(int player)
{
	return PLAYER[player].score;
}

static int player_score_delay;

void Game_PlayerScoreStartDelay(void)
{
	player_score_delay = 30; // 30 frames
}

int Game_PlayerScoreDelayEnabled(void)
{
	return (player_score_delay > 0);
}

void Game_PlayerScoreDelayHandle(void)
{
	if(player_score_delay)
	{
		player_score_delay--;
		if(player_score_delay == 0)
		{
			Ball_Reset();
			Pad_ResetAll();
		}
	}
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

void Game_DrawScreenTop(int screen)
{
	S3D_FramebuffersClearTopScreen(screen, _clear_color.r,_clear_color.g,_clear_color.b);

	// 3D stuff
	Room_Draw(screen);
	
	//2D stuff
	{
		u8 * buf = S3D_BufferGet(screen);
		
		draw_number(buf,Game_PlayerScoreGet(0),10,240-32-10-1);
		
		draw_number(buf,Game_PlayerScoreGet(1),400-32-10-1,240-32-10-1);
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
	
	Room_SetNumber(GAME_ROOM_MENU);
}

void Game_Handle(void)
{
	ClearColorHandle();
	
	switch(Room_GetNumber())
	{
		case GAME_ROOM_MENU:
			Room_Handle();
			break;
		
		case GAME_ROOM_1:
			Game_PlayerScoreDelayHandle();
			Room_Handle();
			Ball_Handle();
			Pad_HandleAll();
			break;
		
		default:
			break;
	}
}

void Game_End(void)
{
	// Nothing here...
}

//-------------------------------------------------------------------------------------------------------
