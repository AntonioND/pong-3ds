
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

#include <3ds.h>
#include <stdlib.h>
#include <string.h>

#include "S3D/engine.h"
#include "utils.h"
#include "game.h"
#include "pad.h"
#include "ball.h"
#include "rooms.h"
#include "screens_2d.h"

//-------------------------------------------------------------------------------------------------------

static int game_paused = 0;

inline void Game_Pause(int pause)
{
	game_paused = pause;
}

inline int Game_IsPaused(void)
{
	return game_paused;
}

//-------------------------------------------------------------------------------------------------------

static _game_state_e game_state_machine;
static int game_state_machine_delay; // delay frames

void Game_StateMachineReset(void)
{
	Game_Pause(0);
	game_state_machine = GAME_STARTING;
	game_state_machine_delay = 120;
}

void Game_PlayerScoreStartDelay(void)
{
	if(game_state_machine != GAME_NORMAL_PLAY) return;
	
	game_state_machine = GAME_GOAL_DELAY;
	game_state_machine_delay = 30; // 30 frames
}

int Game_StateMachinePadMovementEnabled(void)
{
	switch(game_state_machine)
	{
		case GAME_INITIAL_DELAY:
		case GAME_NORMAL_PLAY:
			return 1;
		
		case GAME_STARTING:
		case GAME_ENDING:
		case GAME_GOAL_DELAY:
			return 0;
		
		default:
			return 0;
	}
}

int Game_StateMachineBallMovementEnabled(void)
{
	switch(game_state_machine)
	{
		case GAME_STARTING:
		case GAME_ENDING:
		case GAME_INITIAL_DELAY:
			return 0;
		
		case GAME_NORMAL_PLAY:
		case GAME_GOAL_DELAY:
			return 1;
		
		default:
			return 0;
	}
}

int Game_StateMachineBallAddScoreEnabled(void)
{
	switch(game_state_machine)
	{
		case GAME_INITIAL_DELAY:
		case GAME_STARTING:
		case GAME_ENDING:
		case GAME_GOAL_DELAY:
			return 0;
		
		case GAME_NORMAL_PLAY:
			return 1;

		default:
			return 0;
	}
}

void Game_UpdateStateMachine(void)
{
	int need_to_change = 0;
	
	if(game_state_machine_delay)
	{
		game_state_machine_delay--;
		if(game_state_machine_delay == 0)
		{
			need_to_change = 1;
		}
	}
	
	if(need_to_change == 0) return;
	
	switch(game_state_machine)
	{
		case GAME_STARTING:
			game_state_machine = GAME_INITIAL_DELAY;
			game_state_machine_delay = 30;
			Ball_Reset();
			Pad_ResetAll();
			return;
		case GAME_INITIAL_DELAY:
			game_state_machine = GAME_NORMAL_PLAY;
			return;
		case GAME_NORMAL_PLAY:
			return;
		case GAME_GOAL_DELAY:
			if(Game_PlayerScoreGet(0) >= GAME_WIN_SCORE)
			{
				game_state_machine = GAME_ENDING;
				game_state_machine_delay = 300;
			}
			else if(Game_PlayerScoreGet(1) >= GAME_WIN_SCORE)
			{
				game_state_machine = GAME_ENDING;
				game_state_machine_delay = 300;
			}
			else
			{
				game_state_machine = GAME_INITIAL_DELAY;
				game_state_machine_delay = 30;
				Ball_Reset();
				Pad_ResetAll();
			}
			return;
		case GAME_ENDING:
			Room_SetNumber(GAME_ROOM_MENU);
			return;
		
		default:
			return;
	}
}


inline _game_state_e Game_StateMachineGet(void)
{
	return game_state_machine;
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
	if(Game_IsPaused()) return;
	
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

/*
static void HSV2RGB(int h, int s, int v, int * r, int * g, int * b) // h,s,v: fixed point 0.0 to 1.0 | | | r,b,g: 0..255
{
	if(s == 0)
	{
		int c = fx2int(v * 255);
		*r = c;
		*g = c;
		*b = c;
	}
	else
	{
		int vH = fx2ufrac(h) * 6;
		int vHI = vH - fx2ufrac(vH);
		int sector = fx2int(vH);
		
		int v0 = fx2int( v*255 );
		int v1 = fx2int( fxmul(v,int2fx(1)-s) * 255 );
		int v2 = fx2int( fxmul(v,int2fx(1)-fxmul(s,vH-vHI)) * 255 );
		int v3 = fx2int( fxmul(v,int2fx(1)-fxmul(s,int2fx(1)-(vH-vHI))) * 255 );
		
		switch(sector)
		{
			case 0:  *r = v0; *g = v3; *b = v1; break;
			case 1:  *r = v2; *g = v0; *b = v1; break;
			case 2:  *r = v1; *g = v0; *b = v3; break;
			case 3:  *r = v1; *g = v2; *b = v0; break;
			case 4:  *r = v3; *g = v1; *b = v0; break;
			case 5:  *r = v0; *g = v1; *b = v2; break;
			
			default: *r = 0;  *g = 0;  *b = 0;  break;
		}
	}
}

struct {
	int r,g,b;
	int vr,vg,vb;
} _clear_color;

void ClearColorInit(void)
{
	_clear_color.r = _clear_color.g = _clear_color.b = 64;
	_clear_color.vr = (fast_rand() & ((int2fx(1)>>6)-1)) + (int2fx(1)>>8);
	_clear_color.vg = (fast_rand() & ((int2fx(1)>>6)-1)) + (int2fx(1)>>8);
	_clear_color.vb = (fast_rand() & ((int2fx(1)>>6)-1)) + (int2fx(1)>>8);
	if(fast_rand() & 1) _clear_color.vr = -_clear_color.vr;
	if(fast_rand() & 1) _clear_color.vg = -_clear_color.vg;
	if(fast_rand() & 1) _clear_color.vb = -_clear_color.vb;
}

void ClearColorHandle(void)
{
	if(Game_IsPaused()) return;
	
	_clear_color.r += _clear_color.vr;
	_clear_color.g += _clear_color.vg;
	_clear_color.b += _clear_color.vb;
	
	if(_clear_color.g > float2fx(1.0)) { _clear_color.g = float2fx(1.0); _clear_color.vg = -_clear_color.vg; }
	else if(_clear_color.g < float2fx(0.5)) { _clear_color.g = float2fx(0.5); _clear_color.vg = -_clear_color.vg; }
	
	if(_clear_color.b > float2fx(1.0)) { _clear_color.b = float2fx(1.0); _clear_color.vb = -_clear_color.vb; }
	else if(_clear_color.b < float2fx(0.5)) { _clear_color.b = float2fx(0.5); _clear_color.vb = -_clear_color.vb; }
}
*/
//-------------------------------------------------------------------------------------------------------

void Game_DrawScreenTop(int screen)
{
	S3D_FramebuffersClearTopScreen(screen, _clear_color.r,_clear_color.g,_clear_color.b);
	//int r,g,b;
	//HSV2RGB(_clear_color.r,_clear_color.g,_clear_color.b,&r,&g,&b);
	//S3D_FramebuffersClearTopScreen(screen, r,g,b);

	// 3D stuff
	Room_Draw(screen);
	
	S3D_PolygonListFlush(screen, 1);
	
	//2D stuff
	Draw2D_TopScreen(screen);
}

void Game_DrawScreenBottom(void)
{
	Draw2D_BottomScreen();
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
	
	Room_Handle();
}

void Game_End(void)
{
	// Nothing here...
}

//-------------------------------------------------------------------------------------------------------
