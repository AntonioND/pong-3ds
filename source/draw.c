
#include <3ds.h>
#include <string.h>

#include "matrix.h"
#include "engine.h"
#include "plot.h"

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

static u32 zb_left[240*400];
static u32 zb_right[240*400];

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

// in utils.s
void fill_framebuffer(void * dst, u32 color, u32 bottom_screen); // color = 31 _BGR 0

void clear_screen_buffers(void)
{
	u8 * fb_left = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	u8 * fb_right = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL);
	
	fill_framebuffer(fb_left, 0x00FFFFFF, GFX_TOP);
	fill_framebuffer(fb_right, 0x00FFFFFF, GFX_TOP);

	fill_framebuffer(zb_left, 0xFFFFFFFF, GFX_TOP);
	fill_framebuffer(zb_right, 0xFFFFFFFF, GFX_TOP);
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
static u32 * curr_buf_z;

static u32 currcolor_r, currcolor_g, currcolor_b;
static u32 saved_r = 255, saved_g = 255, saved_b = 255;

void set_current_buffer(int right)
{
	if(right)
	{
		curr_buf = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL);
		curr_buf_z = zb_right;
	}
	else
	{
		curr_buf = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
		curr_buf_z = zb_left;
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

static e_primitive_type currmode = P_DOTS;
static u32 vtx_count = 0;
#define MAX_VERTICES_IN_A_PRIMITIVE (4)
static v4 vtx_array[MAX_VERTICES_IN_A_PRIMITIVE];
static const u32 primitive_num_vertices[P_PRIMITIVE_NUMBER] = {1,2,3,4,3,4};
static u32 curr_max_vertices;

void polygon_begin(e_primitive_type type)
{
	currmode = type;
	vtx_count = 0;
	curr_max_vertices = primitive_num_vertices[currmode];
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

static void _draw_dot(void)
{
	u32 x = vtx_array[0][0];
	u32 y = vtx_array[0][1];
	u32 w = vtx_array[0][3];
	
	if((x >= 400) || (y >= 240)) return;
	
	u32 * curr_w = &(curr_buf_z[240*x+y]);
	if(*curr_w > w)
	{
		*curr_w = w;
		
		u8 * p = &(curr_buf[(240*x+y)*3]);
		*p++ = currcolor_b;
		*p++ = currcolor_g;
		*p = currcolor_r;
	}
}

static void _draw_line(void)
{
	Line(curr_buf,curr_buf_z, 400,240,
		vtx_array[0][0],vtx_array[0][1],vtx_array[0][3],
		vtx_array[1][0],vtx_array[1][1],vtx_array[1][3],
		currcolor_r,currcolor_g,currcolor_b);
}

static inline void _draw_triangle(void)
{
	TriFill(curr_buf,curr_buf_z,400,240, 
		vtx_array[0][0],vtx_array[0][1],vtx_array[0][3],
		vtx_array[1][0],vtx_array[1][1],vtx_array[1][3],
		vtx_array[2][0],vtx_array[2][1],vtx_array[2][3],
		currcolor_r,currcolor_g,currcolor_b);
}

static inline void _draw_quad(void)
{
	TriFill(curr_buf,curr_buf_z,400,240, 
		vtx_array[0][0],vtx_array[0][1],vtx_array[0][3],
		vtx_array[1][0],vtx_array[1][1],vtx_array[1][3],
		vtx_array[2][0],vtx_array[2][1],vtx_array[2][3],
		currcolor_r,currcolor_g,currcolor_b);
	TriFill(curr_buf,curr_buf_z,400,240, 
		vtx_array[0][0],vtx_array[0][1],vtx_array[0][3],
		vtx_array[2][0],vtx_array[2][1],vtx_array[2][3],
		vtx_array[3][0],vtx_array[3][1],vtx_array[3][3],
		currcolor_r,currcolor_g,currcolor_b);
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
			case P_DOTS: 
				_draw_dot();
				vtx_count = 0;
				break;
			case P_LINES:
				_draw_line();
				vtx_count = 0;
				break;
			case P_TRIANGLES:
				_draw_triangle();
				vtx_count = 0;
				break;
			case P_QUADS:
				_draw_quad();
				vtx_count = 0;
				break;
			case P_TRIANGLE_STRIP:
				_draw_triangle();
				vtx_array[0][0] = vtx_array[2][0];
				vtx_array[0][1] = vtx_array[2][1];
				vtx_array[0][3] = vtx_array[2][3];
				vtx_count = 2;
				break;
			case P_QUAD_STRIP:
				_draw_quad();
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

void polygon_normal(s32 x, s32 y, s32 z)
{
	v4 l = { x, y, z, 0};
	
	int fr = 0, fg = 0, fb = 0; // add factors
	
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
