// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#ifndef __ROOM__
#define __ROOM__

typedef enum {
    GAME_ROOM_MENU,
    GAME_ROOM_1,
    GAME_ROOM_2,
    GAME_ROOM_3
} _game_room_e;

typedef enum {
    GAME_MODE_2D,
    GAME_MODE_2D_BOUNCE,
    GAME_MODE_3D
} _3d_mode_e;

//--------------------------------------------------------------------------------------------------

void Room_SetNumber(_game_room_e room);
int Room_GetNumber(void);

void Room_Draw(int screen);

_3d_mode_e Room_3DMode(void);

void Room_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax);

void Room_Init(void);
void Room_End(void);

void Room_Handle(void);

#endif //__ROOM__
