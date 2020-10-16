// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#include <stdlib.h>
#include <string.h>

#include <3ds.h>

#include "draw.h"
#include "fxmath.h"
#include "math.h"
#include "polygon.h"
#include "private.h"

// -----------------------------------------------------------------------------

// In engine.c
extern s3d_state S3D[2];
void _s3d_global_matrix_update(int screen);

// -----------------------------------------------------------------------------

void S3D_BuffersSetup(void)
{
    S3D[0].curr_buf = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    S3D[1].curr_buf = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL);
}

u8 *S3D_BufferGet(int screen)
{
    s3d_state *st = &S3D[screen];

    return st->curr_buf;
}

// -----------------------------------------------------------------------------

void S3D_FramebuffersClearTopScreen(int screen, int r, int g, int b)
{
    s3d_state *st = &S3D[screen];

    u32 val = (b << 16) | (g << 8) | (r);

    S3D_FramebufferFill(st->curr_buf, val, GFX_TOP);
}

// -----------------------------------------------------------------------------

void S3D_SetCulling(int screen, int draw_front, int draw_back)
{
    s3d_state *st = &S3D[screen];

    st->draw_front = draw_front;
    st->draw_back = draw_back;
}

static inline int _s3d_check_cull_face(int screen, int x1, int y1,
                                       int x2, int y2, int x3, int y3)
{
    s3d_state *st = &S3D[screen];

    // If 3 pixels are the same line (horizontal or vertical), always draw the
    // polygon.
    int dx1 = x2 - x1;
    int dy1 = y2 - y1;
    if ((dx1 == 0) && (dy1 == 0))
        return 1;

    int dx2 = x3 - x1;
    int dy2 = y3 - y1;
    if ((dx2 == 0) && (dy2 == 0))
        return 1;

    // Check orientation

    // Z axis goes away from the player (to the inside of the screen)
    int cross = dx1 * dy2 - dx2 * dy1;
    if ((cross > 0) && !(st->draw_back))
        return 0;
    if ((cross < 0) && !(st->draw_front))
        return 0;

    return 1;
}

// -----------------------------------------------------------------------------

void S3D_PolygonColor(int screen, u32 r, u32 g, u32 b)
{
    S3D_PolygonColorAlpha(screen, r, g, b, 255);
}

void S3D_PolygonAlpha(int screen, u32 a)
{
    s3d_state *st = &S3D[screen];

    st->currcolor_alpha = a;
}

void S3D_PolygonColorAlpha(int screen, u32 r, u32 g, u32 b, u32 a)
{
    s3d_state *st = &S3D[screen];

    st->currcolor_r = st->saved_r = r;
    st->currcolor_g = st->saved_g = g;
    st->currcolor_b = st->saved_b = b;
    st->currcolor_alpha = a;
}

// ---------------------------------------------

void S3D_PolygonNormal(int screen, s32 x, s32 y, s32 z)
{
    s3d_state *st = &S3D[screen];

    v4 temp_l = { x, y, z, 0 };
    v4 l;

    m44_v4_Multiply(&(st->MODELVIEW), &temp_l, &l);
    v4_Normalize(&l);

    // Add factors
    int fr = st->amb_r, fg = st->amb_g, fb = st->amb_b;

    int i;
    for (i = 0; i < MAX_LIGHT_SOURCES; i++)
    {
        if (st->light_enabled & S3D_LIGHT_N(i))
        {
            // Change sign because light goes AGAINST a normal
            s32 factor = -v4_DotProduct(&l, &(st->light_dir[i]));
            if (factor > 0)
            {
                fr += fxmul64(factor, st->light_color[i][0]);
                fg += fxmul64(factor, st->light_color[i][1]);
                fb += fxmul64(factor, st->light_color[i][2]);
            }
        }
    }

    if (fr > 255)
        fr = 255;
    if (fg > 255)
        fg = 255;
    if (fb > 255)
        fb = 255;

    int r = (st->saved_r * fr) / 256;
    int g = (st->saved_g * fg) / 256;
    int b = (st->saved_b * fb) / 256;

    st->currcolor_r = r;
    st->currcolor_g = g;
    st->currcolor_b = b;
}

// ---------------------------------------------

void S3D_LightAmbientColorSet(int screen, int r, int g, int b)
{
    s3d_state *st = &S3D[screen];

    st->amb_r = r;
    st->amb_g = g;
    st->amb_b = b;
}

void S3D_LightDirectionalColorSet(int screen, int index, int r, int g, int b)
{
    s3d_state *st = &S3D[screen];

    st->light_color[index][0] = r;
    st->light_color[index][1] = g;
    st->light_color[index][2] = b;
}

void S3D_LightDirectionalVectorSet(int screen, int index, s32 x, s32 y, s32 z)
{
    s3d_state *st = &S3D[screen];

    st->light_dir[index][0] = x;
    st->light_dir[index][1] = y;
    st->light_dir[index][2] = z;
}

void S3D_LightEnable(int screen, u32 enable_mask)
{
    s3d_state *st = &S3D[screen];

    st->light_enabled = enable_mask;
}

// -----------------------------------------------------------------------------

void S3D_PolygonBegin(int screen, s3d_primitive type)
{
    s3d_state *st = &S3D[screen];

    const u32 primitive_num_vertices[S3D_PRIMITIVE_NUMBER] = {
        1, 2, 3, 4, 2, 3, 4
    };

    st->currmode = type;
    st->vtx_count = 0;
    st->curr_max_vertices = primitive_num_vertices[type];
}

// ---------------------------------------------

#define LINE_THICKNESS (3)

static inline void _s3d_draw_dot(int screen, int x, int y,
                                 int r, int g, int b, int a)
{
    s3d_state *st = &S3D[screen];

    S3D_2D_PlotEx(st->curr_buf, LINE_THICKNESS, x, y, r, g, b, a);
}

static inline void _s3d_draw_line(int screen, int x1, int y1, int x2, int y2,
                                  int r, int g, int b, int a)
{
    s3d_state *st = &S3D[screen];

    S3D_2D_LineEx(st->curr_buf, LINE_THICKNESS, x1, y1, x2, y2, r, g, b, a);
}

static inline void _s3d_draw_triangle(int screen, int x1, int y1,
                                      int x2, int y2, int x3, int y3,
                                      int r, int g, int b, int a)
{
    s3d_state *st = &S3D[screen];

    if (a == 255)
    {
        S3D_2D_TriangleFill(st->curr_buf, x1, y1, x2, y2, x3, y3, r, g, b);
    }
    else
    {
        S3D_2D_TriangleFillAlpha(st->curr_buf, x1, y1, x2, y2, x3, y3,
                                 r, g, b, a);
    }
}

static inline void _s3d_draw_quad(int screen, int x1, int y1, int x2, int y2,
                                  int x3, int y3, int x4, int y4,
                                  int r, int g, int b, int a)
{
    s3d_state *st = &S3D[screen];

    if (a != 255)
    {
        S3D_2D_QuadFillAlpha(st->curr_buf, x1, y1, x2, y2, x3, y3, x4, y4,
                             r, g, b, a);
    }
    else
    {
        S3D_2D_QuadFill(st->curr_buf, x1, y1, x2, y2, x3, y3, x4, y4, r, g, b);
    }
}

// -----------------------------------------------------------------------------

void S3D_PolygonListClear(int screen)
{
    s3d_state *st = &S3D[screen];

    st->polygon_number = 0;
}

// -----------------------------------------------------------------------------

static inline void _s3d_polygon_list_add_dot(int screen, v4 *a,
                                             int _r, int _g, int _b, int _a)
{
    s3d_state *st = &S3D[screen];

    _poly_list_t *e = &(st->List[st->polygon_number]);

    e->type = S3D_DOTS;

    int i;
    for (i = 0; i < 4; i++)
    {
        e->v[0][i] = ptr_V4(a, i);
    }

    e->avg_z = e->v[0][3];

    // if (ptr_V4(a,2) > float2fx(0.0))
    //     return;

    e->r = _r;
    e->g = _g;
    e->b = _b;
    e->a = _a;

    st->polygon_number++;
}

static inline void _s3d_polygon_list_add_line(int screen, v4 *a, v4 *b,
                                              int _r, int _g, int _b, int _a)
{
    s3d_state *st = &S3D[screen];

    _poly_list_t *e = &(st->List[st->polygon_number]);

    e->type = S3D_LINES;

    int i;
    for (i = 0; i < 4; i++)
    {
        e->v[0][i] = ptr_V4(a, i);
        e->v[1][i] = ptr_V4(b, i);
    }

    e->avg_z = e->v[0][3] + e->v[1][3];
    e->avg_z /= 2;

    // if (ptr_V4(a,2) > float2fx(0.0))
    //     return;
    // if (ptr_V4(b,2) > float2fx(0.0))
    //     return;

    e->r = _r;
    e->g = _g;
    e->b = _b;
    e->a = _a;

    st->polygon_number++;
}

static inline void _s3d_polygon_list_add_triangle(int screen,
                                                  v4 *a, v4 *b, v4 *c,
                                                  int _r, int _g, int _b, int _a)
{
    s3d_state *st = &S3D[screen];

    if (!_s3d_check_cull_face(screen,
                              ptr_V4(a, 0), ptr_V4(a, 1),
                              ptr_V4(b, 0), ptr_V4(b, 1),
                              ptr_V4(c, 0), ptr_V4(c, 1)))
    {
        return;
    }

    _poly_list_t *e = &(st->List[st->polygon_number]);

    e->type = S3D_TRIANGLES;

    int i;
    for (i = 0; i < 4; i++)
    {
        e->v[0][i] = ptr_V4(a, i);
        e->v[1][i] = ptr_V4(b, i);
        e->v[2][i] = ptr_V4(c, i);
    }

    e->avg_z = e->v[0][3] + e->v[1][3] + e->v[2][3];
    e->avg_z /= 3;

    // if (ptr_V4(a,2) > float2fx(0.0))
    //     return;
    // if (ptr_V4(b,2) > float2fx(0.0))
    //     return;
    // if (ptr_V4(c,2) > float2fx(0.0))
    //     return;

    e->r = _r;
    e->g = _g;
    e->b = _b;
    e->a = _a;

    st->polygon_number++;
}

static inline void _s3d_polygon_list_add_quad(int screen,
                                              v4 *a, v4 *b, v4 *c, v4 *d,
                                              int _r, int _g, int _b, int _a)
{
    s3d_state *st = &S3D[screen];

    if (!_s3d_check_cull_face(screen,
                              ptr_V4(a, 0), ptr_V4(a, 1),
                              ptr_V4(b, 0), ptr_V4(b, 1),
                              ptr_V4(c, 0), ptr_V4(c, 1)))
    {
        return;
    }

    _poly_list_t *e = &(st->List[st->polygon_number]);

    e->type = S3D_QUADS;

    int i;
    for (i = 0; i < 4; i++)
    {
        e->v[0][i] = ptr_V4(a, i);
        e->v[1][i] = ptr_V4(b, i);
        e->v[2][i] = ptr_V4(c, i);
        e->v[3][i] = ptr_V4(d, i);
    }

    e->avg_z = e->v[0][3] + e->v[1][3] + e->v[2][3] + e->v[3][3];
    e->avg_z /= 4;

    // if (ptr_V4(a,2) > float2fx(0.0))
    //     return;
    // if (ptr_V4(b,2) > float2fx(0.0))
    //     return;
    // if (ptr_V4(c,2) > float2fx(0.0))
    //     return;
    // if (ptr_V4(d,2) > float2fx(0.0))
    //     return;

    e->r = _r;
    e->g = _g;
    e->b = _b;
    e->a = _a;

    st->polygon_number++;
}

// -----------------------------------------------------------------------------

static int _s3d_sort_poly_list_compare_function(const void *p1, const void *p2)
{
    s32 z1 = ((const _poly_list_t *)p1)->avg_z;
    s32 z2 = ((const _poly_list_t *)p2)->avg_z;
    return z1 - z2;
}

void S3D_PolygonListFlush(int screen, int perform_z_sort)
{
    s3d_state *st = &S3D[screen];

    if (perform_z_sort)
    {
        qsort((void *)&(st->List), st->polygon_number,
              sizeof(_poly_list_t), _s3d_sort_poly_list_compare_function);
    }

    for (int i = 0; i < st->polygon_number; i++)
    {
        _poly_list_t *e = &(st->List[i]);

        switch (e->type)
        {
            case S3D_DOTS:
                _s3d_draw_dot(screen,
                              e->v[0][0], e->v[0][1],
                              e->r, e->g, e->b, e->a);
                break;

            case S3D_LINES:
                _s3d_draw_line(screen,
                               e->v[0][0], e->v[0][1],
                               e->v[1][0], e->v[1][1],
                               e->r, e->g, e->b, e->a);
                break;

            case S3D_TRIANGLES:
                _s3d_draw_triangle(screen,
                                   e->v[0][0], e->v[0][1],
                                   e->v[1][0], e->v[1][1],
                                   e->v[2][0], e->v[2][1],
                                   e->r, e->g, e->b, e->a);
                break;

            case S3D_QUADS:
                _s3d_draw_quad(screen,
                               e->v[0][0], e->v[0][1],
                               e->v[1][0], e->v[1][1],
                               e->v[2][0], e->v[2][1],
                               e->v[3][0], e->v[3][1],
                               e->r, e->g, e->b, e->a);
                break;

            default:
                break;
        }
    }

    st->polygon_number = 0;
}

// -----------------------------------------------------------------------------

void S3D_PolygonVertex(int screen, s32 x, s32 y, s32 z)
{
    s3d_state *st = &S3D[screen];

    v4 v = { x, y, z, int2fx(1) };
    v4 result;

    _s3d_global_matrix_update(screen);

    m44_v4_Multiply(&(st->GLOBAL_MATRIX), &v, &result);

    st->vtx_array[st->vtx_count][0] = fx2int(fxdiv64(result[0], result[3]));
    st->vtx_array[st->vtx_count][1] = fx2int(fxdiv64(result[1], result[3]));
    st->vtx_array[st->vtx_count][3] = result[3];
    st->vtx_count++;

    if (st->vtx_count == st->curr_max_vertices)
    {
        switch (st->currmode)
        {
            case S3D_DOTS:
                _s3d_polygon_list_add_dot(screen,
                                          &(st->vtx_array[0]),
                                          st->currcolor_r,
                                          st->currcolor_g,
                                          st->currcolor_b,
                                          st->currcolor_alpha);

                st->vtx_count = 0;
                break;

            case S3D_LINES:
                _s3d_polygon_list_add_line(screen,
                                           &(st->vtx_array[0]),
                                           &(st->vtx_array[1]),
                                           st->currcolor_r,
                                           st->currcolor_g,
                                           st->currcolor_b,
                                           st->currcolor_alpha);

                st->vtx_count = 0;
                break;

            case S3D_TRIANGLES:
                _s3d_polygon_list_add_triangle(screen,
                                               &(st->vtx_array[0]),
                                               &(st->vtx_array[1]),
                                               &(st->vtx_array[2]),
                                               st->currcolor_r,
                                               st->currcolor_g,
                                               st->currcolor_b,
                                               st->currcolor_alpha);

                st->vtx_count = 0;
                break;

            case S3D_QUADS:
                _s3d_polygon_list_add_quad(screen,
                                           &(st->vtx_array[0]),
                                           &(st->vtx_array[1]),
                                           &(st->vtx_array[2]),
                                           &(st->vtx_array[3]),
                                           st->currcolor_r,
                                           st->currcolor_g,
                                           st->currcolor_b,
                                           st->currcolor_alpha);

                st->vtx_count = 0;
                break;

            case S3D_LINE_STRIP:
                _s3d_polygon_list_add_line(screen,
                                           &(st->vtx_array[0]),
                                           &(st->vtx_array[1]),
                                           st->currcolor_r,
                                           st->currcolor_g,
                                           st->currcolor_b,
                                           st->currcolor_alpha);

                st->vtx_array[0][0] = st->vtx_array[1][0];
                st->vtx_array[0][1] = st->vtx_array[1][1];
                st->vtx_array[0][3] = st->vtx_array[1][3];

                st->vtx_count = 1;
                break;

            case S3D_TRIANGLE_STRIP:
                _s3d_polygon_list_add_triangle(screen,
                                               &(st->vtx_array[0]),
                                               &(st->vtx_array[1]),
                                               &(st->vtx_array[2]),
                                               st->currcolor_r,
                                               st->currcolor_g,
                                               st->currcolor_b,
                                               st->currcolor_alpha);

                st->vtx_array[0][0] = st->vtx_array[2][0];
                st->vtx_array[0][1] = st->vtx_array[2][1];
                st->vtx_array[0][3] = st->vtx_array[2][3];

                st->vtx_count = 2;
                break;

            case S3D_QUAD_STRIP:
                _s3d_polygon_list_add_quad(screen,
                                           &(st->vtx_array[0]),
                                           &(st->vtx_array[1]),
                                           &(st->vtx_array[2]),
                                           &(st->vtx_array[3]),
                                           st->currcolor_r,
                                           st->currcolor_g,
                                           st->currcolor_b,
                                           st->currcolor_alpha);

                st->vtx_array[0][0] = st->vtx_array[3][0];
                st->vtx_array[0][1] = st->vtx_array[3][1];
                st->vtx_array[0][3] = st->vtx_array[3][3];

                st->vtx_array[1][0] = st->vtx_array[2][0];
                st->vtx_array[1][1] = st->vtx_array[2][1];
                st->vtx_array[1][3] = st->vtx_array[2][3];

                st->vtx_count = 2;
                break;

            default:
                st->vtx_count = 0;
                break;
        }
    }
}
