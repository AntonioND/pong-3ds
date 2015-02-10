
#ifndef __GAME__
#define __GAME__

//--------------------------------------------------------------------------------------------------

#include <3ds.h>

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

void Game_StateMachineReset(void);

void Game_UpdateStateMachine(void);

void Game_PlayerScoreStartDelay(void);

int Game_StateMachinePadMovementEnabled(void);
int Game_StateMachineBallMovementEnabled(void);
int Game_StateMachineBallAddScoreEnabled(void);

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

