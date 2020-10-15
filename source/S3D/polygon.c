// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#include <3ds.h>
#include <stdlib.h>
#include <string.h>

#include "fxmath.h"
#include "math.h"
#include "draw.h"
#include "polygon.h"

//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------

//In engine.c
extern m44 S3D_GLOBAL_MATRIX[2];
extern m44 PROJECTION[2], MODELVIEW[2];
void _s3d_global_matrix_update(int screen);

//--------------------------------------------------------------------------------------------------

static u8 * curr_buf[2];

void S3D_BuffersSetup(void)
{
    curr_buf[0] = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    curr_buf[1] = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL);
}

u8 * S3D_BufferGet(int screen)
{
    return curr_buf[screen];
}

//---------------------------------------------------------------------------------------

void S3D_FramebuffersClearTopScreen(int screen, int r, int g, int b)
{
    u32 val = (b<<16)|(g<<8)|(r);

    S3D_FramebufferFill(curr_buf[screen], val, GFX_TOP);
}

//---------------------------------------------------------------------------------------

static int _s3d_draw_front[2] = { 1, 1 };
static int _s3d_draw_back[2] = { 0, 0 };

void S3D_SetCulling(int screen, int draw_front, int draw_back)
{
    _s3d_draw_front[screen] = draw_front;
    _s3d_draw_back[screen] = draw_back;
}

static inline int _s3d_check_cull_face(int screen, int x1, int y1, int x2, int y2, int x3, int y3)
{
    // if 3 pixels are the same line (horizontal or vertical), always draw the polygon
    int dx1 = x2-x1; int dy1 = y2-y1;
    if( (dx1 == 0) && (dy1 == 0) ) return 1;
    int dx2 = x3-x1; int dy2 = y3-y1;
    if( (dx2 == 0) && (dy2 == 0) ) return 1;

    // Check orientation
    int cross = dx1*dy2 - dx2*dy1; // Z axis goes away (to the inside of the screen)
    if((cross > 0) && !(_s3d_draw_back[screen])) return 0;
    if((cross < 0) && !(_s3d_draw_front[screen])) return 0;
    return 1;
}

//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------

static u32 currcolor_r[2], currcolor_g[2], currcolor_b[2];
static u32 saved_r[2] = { 255, 255 }, saved_g[2] = { 255, 255 }, saved_b[2] = { 255, 255 };
static u32 currcolor_alpha[2] = { 255, 255 };

#define MAX_LIGHT_SOURCES (2) // no more than 32
static v4 light_dir[2][MAX_LIGHT_SOURCES];
static v4 light_color[2][MAX_LIGHT_SOURCES];
static u32 light_enabled[2] = { 0, 0 };
static int amb_r[2] = { 0, 0 }, amb_g[2] = { 0, 0 }, amb_b[2] = { 0, 0 };

//---------------------------------------------

void S3D_PolygonColor(int screen, u32 r, u32 g, u32 b)
{
    S3D_PolygonColorAlpha(screen,r,g,b,255);
}

void S3D_PolygonAlpha(int screen, u32 a)
{
    currcolor_alpha[screen] = a;
}

void S3D_PolygonColorAlpha(int screen, u32 r, u32 g, u32 b, u32 a)
{
    currcolor_r[screen] = saved_r[screen] = r;
    currcolor_g[screen] = saved_g[screen] = g;
    currcolor_b[screen] = saved_b[screen] = b;
    currcolor_alpha[screen] = a;
}

//---------------------------------------------

void S3D_PolygonNormal(int screen, s32 x, s32 y, s32 z)
{
    v4 temp_l = { x, y, z, 0};
    v4 l;

    m44_v4_Multiply(&MODELVIEW[screen],&temp_l,&l);
    v4_Normalize(&l);

    int fr = amb_r[screen], fg = amb_g[screen], fb = amb_b[screen]; // add factors

    int i;
    for(i = 0; i < MAX_LIGHT_SOURCES; i++) if(light_enabled[screen] & S3D_LIGHT_N(i))
    {
        s32 factor = -v4_DotProduct(&l,&(light_dir[screen][i])); // change sign because light goes AGAINST a normal
        if(factor > 0)
        {
            fr += fxmul64(factor,light_color[screen][i][0]);
            fg += fxmul64(factor,light_color[screen][i][1]);
            fb += fxmul64(factor,light_color[screen][i][2]);
        }
    }

    if(fr > 255) fr = 255;
    if(fg > 255) fg = 255;
    if(fb > 255) fb = 255;

    int r = (saved_r[screen] * fr) / 256;
    int g = (saved_g[screen] * fg) / 256;
    int b = (saved_b[screen] * fb) / 256;

    currcolor_r[screen] = r; currcolor_g[screen] = g; currcolor_b[screen] = b;
}

//---------------------------------------------

void S3D_LightAmbientColorSet(int screen, int r, int g, int b)
{
    amb_r[screen] = r; amb_g[screen] = g; amb_b[screen] = b;
}

void S3D_LightDirectionalColorSet(int screen, int index, int r, int g, int b)
{
    light_color[screen][index][0] = r;
    light_color[screen][index][1] = g;
    light_color[screen][index][2] = b;
}

void S3D_LightDirectionalVectorSet(int screen, int index, s32 x, s32 y, s32 z)
{
    light_dir[screen][index][0] = x;
    light_dir[screen][index][1] = y;
    light_dir[screen][index][2] = z;
}

void S3D_LightEnable(int screen, u32 enable_mask)
{
    light_enabled[screen] = enable_mask;
}

//---------------------------------------------------------------------------------------

static const u32 primitive_num_vertices[S3D_PRIMITIVE_NUMBER] = {1,2,3,4,2,3,4};

static s3d_primitive currmode[2] = { S3D_TRIANGLES, S3D_TRIANGLES };
static u32 vtx_count[2] = { 0, 0 };
#define MAX_VERTICES_IN_A_PRIMITIVE (4)
static v4 vtx_array[2][MAX_VERTICES_IN_A_PRIMITIVE];
static u32 curr_max_vertices[2];

void S3D_PolygonBegin(int screen, s3d_primitive type)
{
    currmode[screen] = type;
    vtx_count[screen] = 0;
    curr_max_vertices[screen] = primitive_num_vertices[type];
}

//---------------------------------------------

#define LINE_THICKNESS (3)

static inline void _s3d_draw_dot(int screen, int x, int y, int r, int g, int b, int a)
{
    S3D_2D_PlotEx(curr_buf[screen],LINE_THICKNESS,x,y,r,g,b,a);
}

static inline void _s3d_draw_line(int screen, int x1, int y1, int x2, int y2, int r, int g, int b, int a)
{
    S3D_2D_LineEx(curr_buf[screen],LINE_THICKNESS,x1,y1,x2,y2,r,g,b,a);
}

static inline void _s3d_draw_triangle(int screen, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b, int a)
{
    if(a == 255)
        S3D_2D_TriangleFill(curr_buf[screen],x1,y1,x2,y2,x3,y3,r,g,b);
    else
        S3D_2D_TriangleFillAlpha(curr_buf[screen],x1,y1,x2,y2,x3,y3,r,g,b,a);
}

static inline void _s3d_draw_quad(int screen, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4,
                                  int r, int g, int b, int a)
{
    if(a != 255)
        S3D_2D_QuadFillAlpha(curr_buf[screen],x1,y1,x2,y2,x3,y3,x4,y4,r,g,b,a);
    else
        S3D_2D_QuadFill(curr_buf[screen],x1,y1,x2,y2,x3,y3,x4,y4,r,g,b);
}

//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------

#define MAX_POLYGONS_IN_LIST (300)

typedef struct {
    s3d_primitive type;
    s32 avg_z;
    v4 v[4]; // max vertices per polygon = 4
    int r,g,b,a;
} _poly_list_t;

static _poly_list_t List[2][MAX_POLYGONS_IN_LIST];
static int polygon_number[2] = { 0, 0 };

//---------------------------------------------------------------------------------------

void S3D_PolygonListClear(int screen)
{
    polygon_number[screen] = 0;
}

//---------------------------------------------------------------------------------------

static inline void _s3d_polygon_list_add_dot(int screen, v4 * a, int _r, int _g, int _b, int _a)
{
    _poly_list_t * e = &(List[screen][polygon_number[screen]]);

    e->type = S3D_DOTS;
    int i;
    for(i = 0; i < 4; i++)
    {
        e->v[0][i] = ptr_V4(a,i);
    }

    e->avg_z = e->v[0][3];

    //if(ptr_V4(a,2) > float2fx(0.0)) return;

    e->r = _r; e->g = _g; e->b = _b; e->a = _a;

    polygon_number[screen]++;
}

static inline void _s3d_polygon_list_add_line(int screen, v4 * a, v4 * b, int _r, int _g, int _b, int _a)
{
    _poly_list_t * e = &(List[screen][polygon_number[screen]]);

    e->type = S3D_LINES;
    int i;
    for(i = 0; i < 4; i++)
    {
        e->v[0][i] = ptr_V4(a,i);
        e->v[1][i] = ptr_V4(b,i);
    }

    e->avg_z = e->v[0][3] + e->v[1][3];
    e->avg_z /= 2;

    //if(ptr_V4(a,2) > float2fx(0.0)) return;
    //if(ptr_V4(b,2) > float2fx(0.0)) return;

    e->r = _r; e->g = _g; e->b = _b; e->a = _a;

    polygon_number[screen]++;
}

static inline void _s3d_polygon_list_add_triangle(int screen, v4 * a, v4 * b, v4 * c, int _r, int _g, int _b, int _a)
{
    if(!_s3d_check_cull_face(screen, ptr_V4(a,0),ptr_V4(a,1),ptr_V4(b,0),ptr_V4(b,1),ptr_V4(c,0),ptr_V4(c,1)))
        return;

    _poly_list_t * e = &(List[screen][polygon_number[screen]]);

    e->type = S3D_TRIANGLES;
    int i;
    for(i = 0; i < 4; i++)
    {
        e->v[0][i] = ptr_V4(a,i);
        e->v[1][i] = ptr_V4(b,i);
        e->v[2][i] = ptr_V4(c,i);
    }

    e->avg_z = e->v[0][3] + e->v[1][3] + e->v[2][3];
    e->avg_z /= 3;

    //if(ptr_V4(a,2) > float2fx(0.0)) return;
    //if(ptr_V4(b,2) > float2fx(0.0)) return;
    //if(ptr_V4(c,2) > float2fx(0.0)) return;

    e->r = _r; e->g = _g; e->b = _b; e->a = _a;

    polygon_number[screen]++;
}

static inline void _s3d_polygon_list_add_quad(int screen, v4 * a, v4 * b, v4 * c, v4 * d, int _r, int _g, int _b, int _a)
{
    if(!_s3d_check_cull_face(screen, ptr_V4(a,0),ptr_V4(a,1),ptr_V4(b,0),ptr_V4(b,1),ptr_V4(c,0),ptr_V4(c,1)))
        return;

    _poly_list_t * e = &(List[screen][polygon_number[screen]]);

    e->type = S3D_QUADS;
    int i;
    for(i = 0; i < 4; i++)
    {
        e->v[0][i] = ptr_V4(a,i);
        e->v[1][i] = ptr_V4(b,i);
        e->v[2][i] = ptr_V4(c,i);
        e->v[3][i] = ptr_V4(d,i);
    }

    e->avg_z = e->v[0][3] + e->v[1][3] + e->v[2][3] + e->v[3][3];
    e->avg_z /= 4;

    //if(ptr_V4(a,2) > float2fx(0.0)) return;
    //if(ptr_V4(b,2) > float2fx(0.0)) return;
    //if(ptr_V4(c,2) > float2fx(0.0)) return;
    //if(ptr_V4(d,2) > float2fx(0.0)) return;

    e->r = _r; e->g = _g; e->b = _b; e->a = _a;

    polygon_number[screen]++;
}

//---------------------------------------------------------------------------------------

static int _s3d_sort_poly_list_compare_function(const void * p1, const void * p2)
{
    s32 z1 = ((const _poly_list_t*)p1)->avg_z;
    s32 z2 = ((const _poly_list_t*)p2)->avg_z;
    return z1 - z2;
}

void S3D_PolygonListFlush(int screen, int perform_z_sort)
{
    if(perform_z_sort)
        qsort((void*)&(List[screen]), polygon_number[screen], sizeof(_poly_list_t), _s3d_sort_poly_list_compare_function);

    int i;
    for(i = 0; i < polygon_number[screen]; i++)
    {
        _poly_list_t * e = &(List[screen][i]);
        switch(e->type)
        {
            case S3D_DOTS:
                _s3d_draw_dot(screen,
                            e->v[0][0],e->v[0][1],
                            e->r,e->g,e->b,e->a);
                break;

            case S3D_LINES:
                _s3d_draw_line(screen,
                            e->v[0][0],e->v[0][1], e->v[1][0],e->v[1][1],
                            e->r,e->g,e->b,e->a);
                break;

            case S3D_TRIANGLES:
                _s3d_draw_triangle(screen,
                            e->v[0][0],e->v[0][1], e->v[1][0],e->v[1][1], e->v[2][0],e->v[2][1],
                            e->r,e->g,e->b,e->a);
                break;

            case S3D_QUADS:
                _s3d_draw_quad(screen,
                            e->v[0][0],e->v[0][1], e->v[1][0],e->v[1][1], e->v[2][0],e->v[2][1], e->v[3][0],e->v[3][1],
                            e->r,e->g,e->b,e->a);
                break;

            default:
                break;
        }
    }

    polygon_number[screen] = 0;
}

//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------

void S3D_PolygonVertex(int screen, s32 x, s32 y, s32 z)
{
    v4 v = { x, y, z, int2fx(1) };
    v4 result;

    _s3d_global_matrix_update(screen);

    m44_v4_Multiply(&S3D_GLOBAL_MATRIX[screen],&v,&result);

    vtx_array[screen][vtx_count[screen]][0] = fx2int(fxdiv64(result[0],result[3]));
    vtx_array[screen][vtx_count[screen]][1] = fx2int(fxdiv64(result[1],result[3]));
    vtx_array[screen][vtx_count[screen]][3] = result[3];
    vtx_count[screen]++;

    if(vtx_count[screen] == curr_max_vertices[screen])
    {
        switch(currmode[screen])
        {
            case S3D_DOTS:
                _s3d_polygon_list_add_dot(screen,
                        &(vtx_array[screen][0]),
                        currcolor_r[screen],currcolor_g[screen],currcolor_b[screen],currcolor_alpha[screen]);

                vtx_count[screen] = 0;
                break;

            case S3D_LINES:
                _s3d_polygon_list_add_line(screen,
                        &(vtx_array[screen][0]),&(vtx_array[screen][1]),
                        currcolor_r[screen],currcolor_g[screen],currcolor_b[screen],currcolor_alpha[screen]);

                vtx_count[screen] = 0;
                break;

            case S3D_TRIANGLES:
                _s3d_polygon_list_add_triangle(screen,
                        &(vtx_array[screen][0]),&(vtx_array[screen][1]),&(vtx_array[screen][2]),
                        currcolor_r[screen],currcolor_g[screen],currcolor_b[screen],currcolor_alpha[screen]);

                vtx_count[screen] = 0;
                break;

            case S3D_QUADS:
                _s3d_polygon_list_add_quad(screen,
                        &(vtx_array[screen][0]),&(vtx_array[screen][1]),&(vtx_array[screen][2]),&(vtx_array[screen][3]),
                        currcolor_r[screen],currcolor_g[screen],currcolor_b[screen],currcolor_alpha[screen]);

                vtx_count[screen] = 0;
                break;

            case S3D_LINE_STRIP:
                _s3d_polygon_list_add_line(screen,
                        &(vtx_array[screen][0]),&(vtx_array[screen][1]),
                        currcolor_r[screen],currcolor_g[screen],currcolor_b[screen],currcolor_alpha[screen]);

                vtx_array[screen][0][0] = vtx_array[screen][1][0];
                vtx_array[screen][0][1] = vtx_array[screen][1][1];
                vtx_array[screen][0][3] = vtx_array[screen][1][3];

                vtx_count[screen] = 1;
                break;

            case S3D_TRIANGLE_STRIP:
                _s3d_polygon_list_add_triangle(screen,
                        &(vtx_array[screen][0]),&(vtx_array[screen][1]),&(vtx_array[screen][2]),
                        currcolor_r[screen],currcolor_g[screen],currcolor_b[screen],currcolor_alpha[screen]);

                vtx_array[screen][0][0] = vtx_array[screen][2][0];
                vtx_array[screen][0][1] = vtx_array[screen][2][1];
                vtx_array[screen][0][3] = vtx_array[screen][2][3];

                vtx_count[screen] = 2;
                break;

            case S3D_QUAD_STRIP:
                _s3d_polygon_list_add_quad(screen,
                        &(vtx_array[screen][0]),&(vtx_array[screen][1]),&(vtx_array[screen][2]),&(vtx_array[screen][3]),
                        currcolor_r[screen],currcolor_g[screen],currcolor_b[screen],currcolor_alpha[screen]);

                vtx_array[screen][0][0] = vtx_array[screen][3][0];
                vtx_array[screen][0][1] = vtx_array[screen][3][1];
                vtx_array[screen][0][3] = vtx_array[screen][3][3];

                vtx_array[screen][1][0] = vtx_array[screen][2][0];
                vtx_array[screen][1][1] = vtx_array[screen][2][1];
                vtx_array[screen][1][3] = vtx_array[screen][2][3];

                vtx_count[screen] = 2;
                break;

            default:
                vtx_count[screen] = 0;
                break;
        }
    }
}
