
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

#ifndef __S3D_ENGINE__
#define __S3D_ENGINE__

//----------------------------------------------------------------------------
//-                      Software 3D engine. Main file.                      -
//----------------------------------------------------------------------------

#include "fxmath.h"
#include "draw.h"
#include "polygon.h"

//----------------------------------------------------------------------------

void S3D_ProjectionMatrixSet(int screen, m44 * m);

//----------------------------------------------------------------------------

void S3D_ModelviewMatrixSet(int screen, m44 * m);
void S3D_ModelviewMatrixMultiply(int screen, m44 * m);
void S3D_ModelviewMatrixPush(int screen);
void S3D_ModelviewMatrixPop(int screen);

//----------------------------------------------------------------------------

#endif //__S3D_ENGINE__

