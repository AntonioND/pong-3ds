// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#ifndef __BALL__
#define __BALL__

#include "game.h"

void Ball_SetDimensions(int x, int y, int z);

void Ball_SetColor(int r, int g, int b, int a);

void Ball_Draw(int screen); // Non-internal flush
void Ball_DrawShadows(int screen); // Internal flush

void Ball_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax);

void Ball_GetPosition(int * x, int * y, int * z);
void Ball_GetSpeed(int * x, int * y, int * z);
void Ball_GetDimensions(int * x, int * y, int * z);

void Ball_Enable3DMovement(int enable);

void Ball_Init(void);
void Ball_Reset(void);
void Ball_Handle(void);

void Ball_Bounce(int speed, int acc);

#endif //__BALL__
