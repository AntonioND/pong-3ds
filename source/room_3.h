// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#ifndef ROOM_3_H__
#define ROOM_3_H__

#include "rooms.h"

void Room_3_Draw(int screen);

void Room_3_GetBounds(int *xmin, int *xmax, int *ymin, int *ymax,
                      int *zmin, int *zmax);

void Room_3_Init(void);
void Room_3_End(void);
void Room_3_Handle(void);

_3d_mode_e Room_3_3DMode(void);

#endif // ROOM_3_H__
