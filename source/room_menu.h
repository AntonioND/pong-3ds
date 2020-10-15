// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#ifndef ROOM_MENU_H__
#define ROOM_MENU_H__

#include "rooms.h"

void Room_Menu_Draw(int screen);
void Room_Menu_Draw_Bottom(void);

void Room_Menu_GetBounds(int *xmin, int *xmax, int *ymin, int *ymax,
                         int *zmin, int *zmax);

void Room_Menu_Init(void);
void Room_Menu_End(void);
void Room_Menu_Handle(void);

_3d_mode_e Room_Menu_3DMode(void);

#endif // ROOM_MENU_H__
