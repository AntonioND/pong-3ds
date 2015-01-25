
#include <stdlib.h>

#include "matrix.h"
#include "math.h"
#include "draw.h"
#include "plot.h"

//---------------------------------------------------------------------------------------

#define MAX_POLYGONS_IN_LIST (200)

typedef struct {
	e_primitive_type type;
	s32 avg_z;
	v4 v[4]; // max vertices per polygon = 4
	int r,g,b;
} _poly_list_t;

static _poly_list_t List[MAX_POLYGONS_IN_LIST];
static int polygon_number;

//---------------------------------------------------------------------------------------

void polygon_list_clear(void)
{
	polygon_number = 0;
}

//---------------------------------------------------------------------------------------

void polygon_list_add_line(v4 * a, v4 * b, int _r, int _g, int _b)
{
	_poly_list_t * e = &(List[polygon_number++]);
	
	e->type = P_LINES;
	int i;
	for(i = 0; i < 4; i++)
	{
		e->v[0][i] = ptr_V4(a,i);
		e->v[1][i] = ptr_V4(b,i);
	}
	
	e->avg_z = e->v[0][3] + e->v[1][3];
	e->avg_z /= 2;
	
	e->r = _r;
	e->g = _g;
	e->b = _b;
}

void polygon_list_add_triangle(v4 * a, v4 * b, v4 * c, int _r, int _g, int _b)
{
	_poly_list_t * e = &(List[polygon_number++]);
	
	e->type = P_TRIANGLES;
	int i;
	for(i = 0; i < 4; i++)
	{
		e->v[0][i] = ptr_V4(a,i);
		e->v[1][i] = ptr_V4(b,i);
		e->v[2][i] = ptr_V4(c,i);
	}
	
	e->avg_z = e->v[0][3] + e->v[1][3] + e->v[2][3];
	e->avg_z /= 3;
	
	e->r = _r;
	e->g = _g;
	e->b = _b;
}

void polygon_list_add_quad(v4 * a, v4 * b, v4 * c, v4 * d, int _r, int _g, int _b)
{
	_poly_list_t * e = &(List[polygon_number++]);
	
	e->type = P_QUADS;
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
	
	e->r = _r;
	e->g = _g;
	e->b = _b;
}

//---------------------------------------------------------------------------------------

//In draw.c
void _draw_line(int x1, int y1, int x2, int y2, int r, int g, int b);
void _draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b);
void _draw_quad(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int r, int g, int b);

//---------------------------------------------------------------------------------------

static int _compare_fun(const void * p1, const void * p2)
{
	s32 z1 = ((const _poly_list_t*)p1)->avg_z;
	s32 z2 = ((const _poly_list_t*)p2)->avg_z;
	return z1 - z2;
}

void polygon_list_flush(int perform_z_sort)
{
	if(perform_z_sort)
		qsort((void*)List, polygon_number, sizeof(_poly_list_t), _compare_fun);
	
	int i;
	for(i = 0; i < polygon_number; i++)
	{
		_poly_list_t * e = &(List[i]);
		switch(e->type)
		{
			case P_LINES:
				_draw_line(e->v[0][0],e->v[0][1], e->v[1][0],e->v[1][1],
							e->r,e->g,e->b);
				break;
				
			case P_TRIANGLES:
				_draw_triangle(e->v[0][0],e->v[0][1], e->v[1][0],e->v[1][1], e->v[2][0],e->v[2][1],
							e->r,e->g,e->b);
				break;
				
			case P_QUADS:
				_draw_quad(e->v[0][0],e->v[0][1], e->v[1][0],e->v[1][1], e->v[2][0],e->v[2][1], e->v[3][0],e->v[3][1],
							e->r,e->g,e->b);
				break;
				
			default:
				break;
		}
	}
	
	polygon_number = 0;
}

//---------------------------------------------------------------------------------------
