// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#ifndef GAME_H__
#define GAME_H__

#include <3ds.h>

#define SFX_BOUNCE_REF (1)
#define SFX_JUMP_REF (2)
#define SFX_SELECT_REF (3)

#define GAME_WIN_SCORE (10)

void Game_Pause(int pause);
int Game_IsPaused(void);

typedef enum
{
    GAME_STARTING,
    GAME_INITIAL_DELAY,
    GAME_NORMAL_PLAY,
    GAME_GOAL_DELAY,
    GAME_ENDING
} _game_state_e;

void Game_StateMachineReset(void);
void Game_UpdateStateMachine(void);

void Game_PlayerScoreStartDelay(void);

int Game_StateMachinePadMovementEnabled(void);
int Game_StateMachineBallMovementEnabled(void);
int Game_StateMachineBallAddScoreEnabled(void);

_game_state_e Game_StateMachineGet(void);

#define COLLISION_X_MIN (1)
#define COLLISION_X_MAX (2)
#define COLLISION_Y_MIN (4)
#define COLLISION_Y_MAX (8)
#define COLLISION_Z_MIN (16)
#define COLLISION_Z_MAX (32)

void Game_PlayerResetAll(void);
void Game_PlayerScoreIncrease(int player); // Player 0 or 1
int Game_PlayerScoreGet(int player);       // Player 0 or 1

void Game_Init(void);

void Game_Handle(void);

void Game_DrawScreenTop(int screen);
void Game_DrawScreenBottom(void);

void Game_End(void);

// -----------------------------------------------------------------------------

// Returns how much the segments overlap
static inline s32 _segments_overlap(s32 amin, s32 amax, s32 bmin, s32 bmax)
{
    if (amin < bmin)
    {
        if (amax < bmin)
            return 0;
        else if (amax < bmax)
            return amax - bmin;
        else
            return bmax - bmin;
    }
    else
    {
        if (bmax < amin)
            return 0;
        else if (bmax < amax)
            return bmax - amin;
        else
            return amax - amin;
    }
}

#endif // GAME_H__
