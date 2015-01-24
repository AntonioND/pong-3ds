
#include <3ds.h>
#include <string.h>

#include "matrix.h"
#include "engine.h"
#include "plot.h"

void clear_screen_buffers(void)
{
	u8 * fb_left = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	u8 * fb_right = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL);
	memset(fb_left, 0xFF, 240*400*3);
	memset(fb_right, 0xFF, 240*400*3);
}

void flush_screen_buffers(void)
{
	// Flush and swap framebuffers
	gfxFlushBuffers();
	gfxSwapBuffers();
}

//------------------------------------------------

static u32 currcolor_r, currcolor_g, currcolor_b;
static u8 * curr_buf;

void set_current_buffer(u8 * buf)
{
	curr_buf = buf;
}

void set_current_color(u32 r, u32 g, u32 b)
{
	currcolor_r = r;
	currcolor_g = g;
	currcolor_b = b;
}

static e_primitive_type currmode = P_DOTS;
static u32 vtx_count = 0;
#define MAX_VERTICES_IN_A_PRIMITIVE (4)
static v4 vtx_array[MAX_VERTICES_IN_A_PRIMITIVE];
static const u32 primitive_num_vertices[P_PRIMITIVE_NUMBER] = {1,2,3,4};
static u32 curr_max_vertices;

void set_primitive_type(e_primitive_type type)
{
	currmode = type;
	vtx_count = 0;
	curr_max_vertices = primitive_num_vertices[currmode];
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

static void _draw_dot(void)
{
	//__screen_plot_check(vtx_array[0][0],vtx_array[0][1],currcolor);
}

static void _draw_line(void)
{
	LineEx(curr_buf,400,240,2, vtx_array[0][0],vtx_array[0][1], vtx_array[1][0],vtx_array[1][1], 
		currcolor_r,currcolor_g,currcolor_b);
}

static void _draw_triangle(void)
{
	LineEx(curr_buf,400,240,2, vtx_array[0][0],vtx_array[0][1], vtx_array[1][0],vtx_array[1][1], 
		currcolor_r,currcolor_g,currcolor_b);
	LineEx(curr_buf,400,240,2, vtx_array[1][0],vtx_array[1][1], vtx_array[2][0],vtx_array[2][1], 
		currcolor_r,currcolor_g,currcolor_b);
	LineEx(curr_buf,400,240,2, vtx_array[2][0],vtx_array[2][1], vtx_array[0][0],vtx_array[0][1], 
		currcolor_r,currcolor_g,currcolor_b);

	//TO DO:
	//http://www-users.mat.uni.torun.pl/~wrona/3d_tutor/tri_fillers.html
}

static void _draw_quad(void)
{
	LineEx(curr_buf,400,240,2, vtx_array[0][0],vtx_array[0][1], vtx_array[1][0],vtx_array[1][1], 
		currcolor_r,currcolor_g,currcolor_b);
	LineEx(curr_buf,400,240,2, vtx_array[1][0],vtx_array[1][1], vtx_array[2][0],vtx_array[2][1], 
		currcolor_r,currcolor_g,currcolor_b);
	LineEx(curr_buf,400,240,2, vtx_array[2][0],vtx_array[2][1], vtx_array[3][0],vtx_array[3][1], 
		currcolor_r,currcolor_g,currcolor_b);
	LineEx(curr_buf,400,240,2, vtx_array[3][0],vtx_array[3][1], vtx_array[0][0],vtx_array[0][1], 
		currcolor_r,currcolor_g,currcolor_b);
}

//---------------------------------------------

extern m44 GLOBAL;
extern u32 global_updated;
void __global_matrix_update(void);

void vertex(s32 x, s32 y, s32 z)
{
	v4 v = { x, y, z, int2fx(1) };
	v4 result;
	
	if(global_updated == 0) { __global_matrix_update(); }
	
	m44_v4_multiply(&GLOBAL,&v,&result);
	
	vtx_array[vtx_count][0] = fx2int(fxdiv(result[0],result[3]));
	vtx_array[vtx_count][1] = fx2int(fxdiv(result[1],result[3]));
	vtx_count++;
	
	if(vtx_count == curr_max_vertices)
	{
		switch(currmode)
		{
			case P_DOTS: _draw_dot(); break;
			case P_LINES: _draw_line(); break;
			case P_TRIANGLES: _draw_triangle(); break;
			case P_QUADS: _draw_quad(); break;
			default: break;
		}
		vtx_count = 0;
	}
}

//------------------------------------------------

