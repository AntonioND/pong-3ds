// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#ifndef S3D_POLYGON_H__
#define S3D_POLYGON_H__

#include <3ds.h>

// -----------------------------------------------------------------------------

// Must be called at the beginning of each frame!
void S3D_BuffersSetup(void);

u8 *S3D_BufferGet(int screen);

// -----------------------------------------------------------------------------

// in utils_asm.s
// color = 31 _BGR 0
void S3D_FramebufferFill(void *dst, u32 color, u32 bottom_screen);

void S3D_FramebuffersClearTopScreen(int screen, int r, int g, int b);

// -----------------------------------------------------------------------------

typedef enum
{
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

// Stores a color for the following polygons and sets drawing color to that
// color. Clears the current normal effect.
void S3D_PolygonColor(int screen, u32 r, u32 g, u32 b);
void S3D_PolygonColorAlpha(int screen, u32 r, u32 g, u32 b, u32 a);
void S3D_PolygonAlpha(int screen, u32 a);

// Sets the drawing color to current stored color affected by light.
void S3D_PolygonNormal(int screen, s32 x, s32 y, s32 z);

// -----------------------------------------------------------------------------

void S3D_SetCulling(int screen, int draw_front, int draw_back);

// -----------------------------------------------------------------------------

void S3D_LightAmbientColorSet(int screen, int r, int g, int b);

void S3D_LightDirectionalVectorSet(int screen, int index, s32 x, s32 y, s32 z);
void S3D_LightDirectionalColorSet(int screen, int index, int r, int g, int b);

#define S3D_LIGHT_N(i) (1 << (i))
void S3D_LightEnable(int screen, u32 enable_mask);

// -----------------------------------------------------------------------------

void S3D_PolygonListClear(int screen);

void S3D_PolygonListFlush(int screen, int perform_z_sort);

#endif // S3D_POLYGON_H__
