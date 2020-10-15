// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#ifndef PAD_H__
#define PAD_H__

#include "game.h"

void Pad_P1SetDimensions(int x, int y, int z); // Player (z min)
void Pad_P2SetDimensions(int x, int y, int z); // AI (z max)

void Pad_P1SetColor(int r, int g, int b, int a);
void Pad_P2SetColor(int r, int g, int b, int a);

void Pad_P1Draw(int screen);
void Pad_P2Draw(int screen);
void Pad_P1DrawShadows(int screen);
void Pad_P2DrawShadows(int screen);

void Pad_P1GetBounds(int *xmin, int *xmax, int *ymin, int *ymax,
                     int *zmin, int *zmax); // Player
void Pad_P2GetBounds(int *xmin, int *xmax, int *ymin, int *ymax,
                     int *zmin, int *zmax); // AI

void Pad_P1GetPosition(int *x, int *y, int *z);
void Pad_P2GetPosition(int *x, int *y, int *z);

void Pad_InitAll(void);
void Pad_ResetAll(void);
void Pad_HandleAll(void);

void Pad_P1Bounce(int speed, int acc);
void Pad_P2Bounce(int speed, int acc);

#endif // PAD_H__
