// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#ifndef ROOM_1_H__
#define ROOM_1_H__

#include "rooms.h"

void Room_1_Draw(int screen);

void Room_1_GetBounds(int *xmin, int *xmax, int *ymin, int *ymax,
                      int *zmin, int *zmax);

void Room_1_Init(void);
void Room_1_End(void);
void Room_1_Handle(void);

_3d_mode_e Room_1_3DMode(void);

#endif // ROOM_1_H__
