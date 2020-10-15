
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

#ifndef __GAME__
#define __GAME__

//--------------------------------------------------------------------------------------------------

#include <3ds.h>

//--------------------------------------------------------------------------------------------------

#define SFX_BOUNCE_REF (1)
#define SFX_JUMP_REF   (2)
#define SFX_SELECT_REF (3)

//--------------------------------------------------------------------------------------------------

#define GAME_WIN_SCORE (10)

//--------------------------------------------------------------------------------------------------

void Game_Pause(int pause);
int Game_IsPaused(void);

//--------------------------------------------------------------------------------------------------

typedef enum {
	GAME_STARTING,
	GAME_INITIAL_DELAY,
	GAME_NORMAL_PLAY,
	GAME_GOAL_DELAY,
	GAME_ENDING
} _game_state_e;

//--------------------------------------------------------------------------------------------------

void Game_StateMachineReset(void);

void Game_UpdateStateMachine(void);

void Game_PlayerScoreStartDelay(void);

int Game_StateMachinePadMovementEnabled(void);
int Game_StateMachineBallMovementEnabled(void);
int Game_StateMachineBallAddScoreEnabled(void);

_game_state_e Game_StateMachineGet(void);

//--------------------------------------------------------------------------------------------------

#define COLLISION_X_MIN (1)
#define COLLISION_X_MAX (2)
#define COLLISION_Y_MIN (4)
#define COLLISION_Y_MAX (8)
#define COLLISION_Z_MIN (16)
#define COLLISION_Z_MAX (32)

//--------------------------------------------------------------------------------------------------

void Game_PlayerResetAll(void);
void Game_PlayerScoreIncrease(int player); // 0 or 1
int Game_PlayerScoreGet(int player); // 0 or 1

//--------------------------------------------------------------------------------------------------

void Game_Init(void);

void Game_Handle(void);

void Game_DrawScreenTop(int screen);
void Game_DrawScreenBottom(void);

void Game_End(void);

//--------------------------------------------------------------------------------------------------

// returns how much the segments overlap
static inline s32 _segments_overlap(s32 amin, s32 amax, s32 bmin, s32 bmax)
{
	if(amin < bmin)
	{
		if(amax < bmin) return 0;
		else if(amax < bmax) return amax - bmin;
		else return bmax - bmin;
	}
	else
	{
		if(bmax < amin) return 0;
		else if(bmax < amax) return bmax - amin;
		else return amax - amin;
	}
}

//--------------------------------------------------------------------------------------------------

#endif //__GAME__

