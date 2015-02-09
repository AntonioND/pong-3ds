
//-------------------------------------------------------------------------------------------------------

#include <stdlib.h>

#include "fxmath.h"
#include "math.h"
#include "draw.h"

//---------------------------------------------------------------------------------------

#define MAX_POLYGONS_IN_LIST (300)

typedef struct {
	s3d_primitive type;
	s32 avg_z;
	v4 v[4]; // max vertices per polygon = 4
	int r,g,b;
} _poly_list_t;

static _poly_list_t List[2][MAX_POLYGONS_IN_LIST];
static int polygon_number[2] = { 0, 0 };

//---------------------------------------------------------------------------------------

void S3D_PolygonListClear(int screen)
{
	polygon_number[screen] = 0;
}

//---------------------------------------------------------------------------------------

void _s3d_polygon_list_add_dot(int screen, v4 * a, int _r, int _g, int _b)
{
	_poly_list_t * e = &(List[screen][polygon_number[screen]]);
	polygon_number[screen]++;
	
	e->type = S3D_DOTS;
	int i;
	for(i = 0; i < 4; i++)
	{
		e->v[0][i] = ptr_V4(a,i);
	}
	
	e->avg_z = e->v[0][3];
	
	e->r = _r; e->g = _g; e->b = _b;
}

void _s3d_polygon_list_add_line(int screen, v4 * a, v4 * b, int _r, int _g, int _b)
{
	_poly_list_t * e = &(List[screen][polygon_number[screen]]);
	polygon_number[screen]++;
	
	e->type = S3D_LINES;
	int i;
	for(i = 0; i < 4; i++)
	{
		e->v[0][i] = ptr_V4(a,i);
		e->v[1][i] = ptr_V4(b,i);
	}
	
	e->avg_z = e->v[0][3] + e->v[1][3];
	e->avg_z /= 2;
	
	e->r = _r; e->g = _g; e->b = _b;
}

void _s3d_polygon_list_add_triangle(int screen, v4 * a, v4 * b, v4 * c, int _r, int _g, int _b)
{
	_poly_list_t * e = &(List[screen][polygon_number[screen]]);
	polygon_number[screen]++;
	
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
	
	e->r = _r; e->g = _g; e->b = _b;
}

void _s3d_polygon_list_add_quad(int screen, v4 * a, v4 * b, v4 * c, v4 * d, int _r, int _g, int _b)
{
	_poly_list_t * e = &(List[screen][polygon_number[screen]]);
	polygon_number[screen]++;
	
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
	
	e->r = _r; e->g = _g; e->b = _b;
}

//---------------------------------------------------------------------------------------

//In draw.c
void _s3d_draw_dot(int screen, int x, int y, int r, int g, int b);
void _s3d_draw_line(int screen, int x1, int y1, int x2, int y2, int r, int g, int b);
void _s3d_draw_triangle(int screen, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b);
void _s3d_draw_quad(int screen, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int r, int g, int b);

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
							e->r,e->g,e->b);
				break;
				
			case S3D_LINES:
				_s3d_draw_line(screen,
							e->v[0][0],e->v[0][1], e->v[1][0],e->v[1][1],
							e->r,e->g,e->b);
				break;
				
			case S3D_TRIANGLES:
				_s3d_draw_triangle(screen,
							e->v[0][0],e->v[0][1], e->v[1][0],e->v[1][1], e->v[2][0],e->v[2][1],
							e->r,e->g,e->b);
				break;
				
			case S3D_QUADS:
				_s3d_draw_quad(screen,
							e->v[0][0],e->v[0][1], e->v[1][0],e->v[1][1], e->v[2][0],e->v[2][1], e->v[3][0],e->v[3][1],
							e->r,e->g,e->b);
				break;
				
			default:
				break;
		}
	}
	
	polygon_number[screen] = 0;
}

//---------------------------------------------------------------------------------------
