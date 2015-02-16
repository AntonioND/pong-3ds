
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

#ifndef __PAD__
#define __PAD__

//--------------------------------------------------------------------------------------------------

#include "game.h"

//--------------------------------------------------------------------------------------------------

void Pad_P1SetDimensions(int x, int y, int z); // Player (z min)
void Pad_P2SetDimensions(int x, int y, int z); // AI (z max)

void Pad_P1SetColor(int r, int g, int b, int a);
void Pad_P2SetColor(int r, int g, int b, int a);

void Pad_P1Draw(int screen);
void Pad_P2Draw(int screen);
void Pad_P1DrawShadows(int screen);
void Pad_P2DrawShadows(int screen);

void Pad_P1GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax); // Player
void Pad_P2GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax); // AI

void Pad_P1GetPosition(int * x, int * y, int * z);
void Pad_P2GetPosition(int * x, int * y, int * z);

void Pad_InitAll(void);
void Pad_ResetAll(void);
void Pad_HandleAll(void);

inline void Pad_P1Bounce(int speed, int acc);
inline void Pad_P2Bounce(int speed, int acc);

//--------------------------------------------------------------------------------------------------

#endif //__PAD__
