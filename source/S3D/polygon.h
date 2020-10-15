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

#ifndef __S3D_POLYGON__
#define __S3D_POLYGON__

#include <3ds.h>

//----------------------------------------------------------------------------------

void S3D_BuffersSetup(void); // Must be called at the beginning of each frame!

u8 * S3D_BufferGet(int screen);

//----------------------------------------------------------------------------------

// in utils_asm.s
void S3D_FramebufferFill(void * dst, u32 color, u32 bottom_screen); // color = 31 _BGR 0

void S3D_FramebuffersClearTopScreen(int screen, int r, int g, int b);

//--------------------------------------------------------------------------------------------------

typedef enum {
    S3D_DOTS,
    S3D_LINES,
    S3D_TRIANGLES,
    S3D_QUADS,
    S3D_LINE_STRIP,
    S3D_TRIANGLE_STRIP,
    S3D_QUAD_STRIP,

    S3D_PRIMITIVE_NUMBER
} s3d_primitive;

void S3D_PolygonBegin(int screen, s3d_primitive type);

void S3D_PolygonVertex(int screen, s32 x, s32 y, s32 z);

// Stores a color for the following polygons and sets drawing color to that color. Anulates the current normal.
void S3D_PolygonColor(int screen, u32 r, u32 g, u32 b);
void S3D_PolygonColorAlpha(int screen, u32 r, u32 g, u32 b, u32 a);
void S3D_PolygonAlpha(int screen, u32 a);

// Sets the drawing color to current stored color affected by light.
void S3D_PolygonNormal(int screen, s32 x, s32 y, s32 z);

//--------------------------------------------------------------------------------------------------

void S3D_SetCulling(int screen, int draw_front, int draw_back);

//--------------------------------------------------------------------------------------------------

void S3D_LightAmbientColorSet(int screen, int r, int g, int b);

void S3D_LightDirectionalVectorSet(int screen, int index, s32 x, s32 y, s32 z);
void S3D_LightDirectionalColorSet(int screen, int index, int r, int g, int b);

#define S3D_LIGHT_N(i) (1<<(i))
void S3D_LightEnable(int screen, u32 enable_mask);

//--------------------------------------------------------------------------------------------------

void S3D_PolygonListClear(int screen);

void S3D_PolygonListFlush(int screen, int perform_z_sort);

#endif //__S3D_POLYGON__
