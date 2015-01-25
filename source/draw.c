
#include <3ds.h>
#include <string.h>

#include "matrix.h"
#include "engine.h"
#include "plot.h"
#include "utils.h"

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

void clear_buffers(int r, int g, int b)
{
	u8 * fb_left = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	u8 * fb_right = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL);
	
	u32 val = (b<<16)|(g<<8)|(r);
	fill_framebuffer(fb_left, val, GFX_TOP);
	fill_framebuffer(fb_right, val, GFX_TOP);
}

void flush_screen_buffers(void)
{
	// Flush and swap framebuffers
	gfxFlushBuffers();
	gfxSwapBuffers();
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

static u8 * curr_buf;

static u32 currcolor_r, currcolor_g, currcolor_b;
static u32 saved_r = 255, saved_g = 255, saved_b = 255;

void set_current_buffer(int right)
{
	if(right)
	{
		curr_buf = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL);
	}
	else
	{
		curr_buf = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	}
}

void set_current_color(u32 r, u32 g, u32 b)
{
	currcolor_r = saved_r = r;
	currcolor_g = saved_g = g;
	currcolor_b = saved_b = b;
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

static e_primitive_type currmode = P_TRIANGLES;
static u32 vtx_count = 0;
#define MAX_VERTICES_IN_A_PRIMITIVE (4)
static v4 vtx_array[MAX_VERTICES_IN_A_PRIMITIVE];
static const u32 primitive_num_vertices[P_PRIMITIVE_NUMBER] = {2,3,4,2,3,4};
static u32 curr_max_vertices;

void polygon_begin(e_primitive_type type)
{
	currmode = type;
	vtx_count = 0;
	curr_max_vertices = primitive_num_vertices[currmode];
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

//In polygon.c
void polygon_list_add_line(v4 * a, v4 * b, int _r, int _g, int _b);
void polygon_list_add_triangle(v4 * a, v4 * b, v4 * c, int _r, int _g, int _b);
void polygon_list_add_quad(v4 * a, v4 * b, v4 * c, v4 * d, int _r, int _g, int _b);

void _draw_line(int x1, int y1, int x2, int y2, int r, int g, int b)
{
	LineEx(curr_buf,3,x1,y1,x2,y2,r,g,b);
}

void _draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b)
{
	TriFill(curr_buf,x1,y1,x2,y2,x3,y3,r,g,b);
}

void _draw_quad(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int r, int g, int b)
{
	TriFill(curr_buf,x1,y1,x2,y2,x3,y3,r,g,b);
	TriFill(curr_buf,x1,y1,x3,y3,x4,y4,r,g,b);
}

//---------------------------------------------

extern m44 GLOBAL;
extern u32 global_updated;
void __global_matrix_update(void);

void polygon_vertex(s32 x, s32 y, s32 z)
{
	v4 v = { x, y, z, int2fx(1) };
	v4 result;
	
	if(global_updated == 0) { __global_matrix_update(); }
	
	m44_v4_multiply(&GLOBAL,&v,&result);
	
	vtx_array[vtx_count][0] = fx2int(fxdiv(result[0],result[3]));
	vtx_array[vtx_count][1] = fx2int(fxdiv(result[1],result[3]));
	vtx_array[vtx_count][3] = result[3];
	vtx_count++;
	
	if(vtx_count == curr_max_vertices)
	{
		switch(currmode)
		{
			case P_LINES:
				polygon_list_add_line(&(vtx_array[0]),&(vtx_array[1]),
						currcolor_r,currcolor_g,currcolor_b);
				
				vtx_count = 0;
				break;
				
			case P_TRIANGLES:
				polygon_list_add_triangle(&(vtx_array[0]),&(vtx_array[1]),&(vtx_array[2]),
						currcolor_r,currcolor_g,currcolor_b);
				
				vtx_count = 0;
				break;
				
			case P_QUADS:
				polygon_list_add_quad(&(vtx_array[0]),&(vtx_array[1]),&(vtx_array[2]),&(vtx_array[3]),
						currcolor_r,currcolor_g,currcolor_b);
				
				vtx_count = 0;
				break;
			
			case P_LINE_STRIP:
				polygon_list_add_line(&(vtx_array[0]),&(vtx_array[1]),
						currcolor_r,currcolor_g,currcolor_b);
				
				vtx_array[0][0] = vtx_array[1][0];
				vtx_array[0][1] = vtx_array[1][1];
				vtx_array[0][3] = vtx_array[1][3];
				
				vtx_count = 1;
				break;
				
			case P_TRIANGLE_STRIP:
				polygon_list_add_triangle(&(vtx_array[0]),&(vtx_array[1]),&(vtx_array[2]),
						currcolor_r,currcolor_g,currcolor_b);
				
				vtx_array[0][0] = vtx_array[2][0];
				vtx_array[0][1] = vtx_array[2][1];
				vtx_array[0][3] = vtx_array[2][3];
				
				vtx_count = 2;
				break;
				
			case P_QUAD_STRIP:
				polygon_list_add_quad(&(vtx_array[0]),&(vtx_array[1]),&(vtx_array[2]),&(vtx_array[3]),
						currcolor_r,currcolor_g,currcolor_b);
				
				vtx_array[0][0] = vtx_array[3][0];
				vtx_array[0][1] = vtx_array[3][1];
				vtx_array[0][3] = vtx_array[3][3];
				
				vtx_array[1][0] = vtx_array[2][0];
				vtx_array[1][1] = vtx_array[2][1];
				vtx_array[1][3] = vtx_array[2][3];
				
				vtx_count = 2;
				break;
				
			default:
				vtx_count = 0;
				break;
		}
	}
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

#define MAX_LIGHT_SOURCES (2) // no more than 32
static v4 light_dir[MAX_LIGHT_SOURCES];
static v4 light_color[MAX_LIGHT_SOURCES];
static u32 light_enabled = 0;
static int amb_r = 0, amb_g = 0, amb_b = 0;

void polygon_normal(s32 x, s32 y, s32 z)
{
	v4 l = { x, y, z, 0};
	
	int fr = amb_r, fg = amb_g, fb = amb_b; // add factors
	
	int i;
	for(i = 0; i < MAX_LIGHT_SOURCES; i++) if(light_enabled & LIGHT_N(i))
	{
		s32 factor = -v4_dot_product(&l,&(light_dir[i])); // change sign because light goes AGAINST a normal
		if(factor > 0)
		{
			fr += fxmul(factor,light_color[i][0]);
			fg += fxmul(factor,light_color[i][1]);
			fb += fxmul(factor,light_color[i][2]);
		}
	}
	
	if(fr > 255) fr = 255;
	if(fg > 255) fg = 255;
	if(fb > 255) fb = 255;
	
	int r = (saved_r * fr) / 256;
	int g = (saved_g * fg) / 256;
	int b = (saved_b * fb) / 256;
	
	currcolor_r = r; currcolor_g = g; currcolor_b = b;
}


void light_set_ambient_color(int r, int g, int b)
{
	amb_r = r;
	amb_g = g;
	amb_b = b;
}

void light_set_color(int index, int r, int g, int b)
{
	light_color[index][0] = r;
	light_color[index][1] = g;
	light_color[index][2] = b;
}

void light_set_dir(int index, s32 x, s32 y, s32 z)
{
	light_dir[index][0] = x;
	light_dir[index][1] = y;
	light_dir[index][2] = z;
}

void lights_enable(u32 enable_mask)
{
	light_enabled = enable_mask;
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
