// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#ifndef S3D_PRIVATE_H__
#define S3D_PRIVATE_H__

#include "draw.h"
#include "fxmath.h"
#include "polygon.h"

#define MAX_POLYGONS_IN_LIST (300)

typedef struct
{
    s3d_primitive type;
    s32 avg_z;
    v4 v[4]; // max vertices per polygon = 4
    int r, g, b, a;
} _poly_list_t;

typedef struct
{
    m44 GLOBAL_MATRIX;
    m44 PROJECTION;
    m44 MODELVIEW;
    // This is 0 if the matrix needs to be updated
    u32 global_updated;

#define MATRIX_STACK_SIZE (8)
    m44 matrix_stack[MATRIX_STACK_SIZE];
    s32 matrix_stack_ptr;

    // Pointer to the HW framebuffer
    u8 *curr_buf;

    // Front and back culling
    int draw_front;
    int draw_back;

    // Vertices handling state
    s3d_primitive currmode;
    u32 vtx_count;
#define MAX_VERTICES_IN_A_PRIMITIVE (4)
    v4 vtx_array[MAX_VERTICES_IN_A_PRIMITIVE];
    u32 curr_max_vertices;

    // Polygon handling state
    _poly_list_t List[MAX_POLYGONS_IN_LIST];
    int polygon_number;

    // Color state
    u32 currcolor_r, currcolor_g, currcolor_b, currcolor_alpha;
    u32 saved_r, saved_g, saved_b;

    // Lighting state
#define MAX_LIGHT_SOURCES (2) // No more than 32
    v4 light_dir[MAX_LIGHT_SOURCES];
    v4 light_color[MAX_LIGHT_SOURCES];
    u32 light_enabled;
    int amb_r, amb_g, amb_b;
} s3d_state;

#endif // S3D_PRIVATE_H__
