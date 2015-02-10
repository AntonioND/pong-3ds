
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

typedef enum {
	GAME_INITIAL_DELAY,
	GAME_NORMAL_PLAY,
	GAME_GOAL_DELAY
} _game_state_e;

static _game_state_e game_state_machine;
static int game_state_machine_delay; // delay frames

void Game_StateMachineReset(void)
{
	game_state_machine = GAME_INITIAL_DELAY;
	game_state_machine_delay = 30;
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
			return 0;
		case GAME_NORMAL_PLAY:
			return 1;
		case GAME_GOAL_DELAY:
			return 0;
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
		case GAME_INITIAL_DELAY:
			game_state_machine = GAME_NORMAL_PLAY;
			return;
		case GAME_NORMAL_PLAY:
			return;
		case GAME_GOAL_DELAY:
			game_state_machine = GAME_INITIAL_DELAY;
			game_state_machine_delay = 30;
			Ball_Reset();
			Pad_ResetAll();
			return;
		default:
			return;
	}
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
