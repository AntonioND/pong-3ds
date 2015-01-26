
//--------------------------------------------------------------------------------------------------

#include <3ds.h>
#include <string.h>

#include "matrix.h"
#include "engine.h"
#include "draw.h"

//--------------------------------------------------------------------------------------------------

#define CONFIG_3D_SLIDERSTATE (*(float*)0x1FF81080) //this should be in ctrulib...

void S3D_FramebuffersClearTopScreen(int r, int g, int b)
{
	u32 val = (b<<16)|(g<<8)|(r);
	
	u8 * fb_left = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	S3D_FramebufferFill(fb_left, val, GFX_TOP);
	
	if(CONFIG_3D_SLIDERSTATE == 0.0f) return;
	u8 * fb_right = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL);
	S3D_FramebufferFill(fb_right, val, GFX_TOP);
}

static u8 * curr_buf;

void S3D_BufferSetScreen(int right)
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

//---------------------------------------------------------------------------------------

static int _s3d_draw_front = 1;
static int _s3d_draw_back = 0;

void S3D_SetCulling(int draw_front, int draw_back)
{
	_s3d_draw_front = draw_front;
	_s3d_draw_back = draw_back;
}

static inline int _s3d_check_face(int x1, int y1, int x2, int y2, int x3, int y3)
{
	int dx1 = x2-x1; int dy1 = y2-y1;
	int dx2 = x3-x1; int dy2 = y3-y1;
	int cross = dx1*dy2 - dx2*dy1; // Z axis goes away (to the inside of the screen)
	if((cross > 0) && !_s3d_draw_back) return 0;
	if((cross < 0) && !_s3d_draw_front) return 0;
	return 1;
}

//---------------------------------------------------------------------------------------

static inline int abs(int x)
{
	return x < 0 ? -x : x;
}

static inline int min(int a, int b)
{
	return a < b ? a : b;
}

static inline int max(int a, int b)
{
	return a > b ? a : b;
}

static inline int sgn(int x)
{
	if(x < 0) return -1;
	if(x > 0) return 1;
	return 0;
}

static inline void swapints_pairs(int * a1, int * a2, int * b1, int * b2)
{
	int t = *a1; *a1 = *a2; *a2 = t;
	t = *b1; *b1 = *b2; *b2 = t;
}

static inline void roll_values(int * a1, int * a2, int * a3, int * a4, 
								int * b1, int * b2, int * b3, int * b4)
{
	int t = *a1; *a1 = *a2; *a2 = *a3; *a3 = *a4; *a4 = t;
	    t = *b1; *b1 = *b2; *b2 = *b3; *b3 = *b4; *b4 = t;
}

//--------------------------------------------------------------------------------------------------

static inline void _s3d_plot_unsafe(u8 * buf, u32 x, u32 y, int r, int g, int b)
{
	u8 * p = &(buf[(240*x+y)*3]);
	*p++ = b; *p++ = g; *p = r;
}

static inline void _s3d_plot_safe(u8 * buf, u32 x, u32 y, int r, int g, int b)
{
	if((x >= 400) || (y >= 240)) return;
	u8 * p = &(buf[(240*x+y)*3]);
	*p++ = b; *p++ = g; *p = r;
}

static inline void _s3d_vertical_line(u8 * linebuf, u32 x, int y1, int y2, int r, int g, int b) //x unsafe
{
	if(y1 < y2)
	{
		if(y2 < 0) return;
		else if(y2 >= 240) y2 = 240-1;
		
		if(y1 >= 240) return;
		else if(y1 < 0) y1 = 0;
		
		linebuf += y1*3;
		for( ;y1<=y2; y1++) { *linebuf++ = b; *linebuf++ = g; *linebuf++ = r; }
	}
	else // y2 < y1
	{
		if(y1 < 0) return;
		else if(y1 >= 240) y1 = 240-1;
		
		if(y2 >= 240) return;
		else if(y2 < 0) y2 = 0;
		
		linebuf += y2*3;
		for( ;y2<=y1; y2++) { *linebuf++ = b; *linebuf++ = g; *linebuf++ = r; }
	}
}

static inline void _s3d_vertical_line_downwards(u8 * linebuf, u32 x, int y1, int y2, int r, int g, int b) //x unsafe, y1 < y2
{
//if(y2 < y1) { printf("error"); return; }

	if(y2 < 0) return;
	else if(y2 >= 240) y2 = 240-1;
	
	if(y1 >= 240) return;
	else if(y1 < 0) y1 = 0;
	
	linebuf += y1*3;
	for( ;y1<=y2; y1++) { *linebuf++ = b; *linebuf++ = g; *linebuf++ = r; }
}

void _s3d_line_unsafe(u8 * buf, int x1, int y1, int x2, int y2, int r, int g, int b)
{
	int i,dx,dy,sdx,sdy,dxabs,dyabs,x,y,px,py;

	dx = x2-x1; // horizontal distance
	dy = y2-y1; // vertical distance
	dxabs = abs(dx);
	dyabs = abs(dy);
	sdx = sgn(dx);
	sdy = sgn(dy);
	x = dyabs>>1;
	y = dxabs>>1;
	px = x1;
	py = y1;

	_s3d_plot_unsafe(buf,px,py,r,g,b);
	
	if(dxabs >= dyabs) // the line is more horizontal than vertical
	{
		for(i = 0; i < dxabs; i++)
		{
			y += dyabs;
			if(y >= dxabs)
			{
				y -= dxabs;
				py += sdy;
			}
			px += sdx;
			_s3d_plot_unsafe(buf,px,py,r,g,b);
		}
	}
	else // the line is more vertical than horizontal
	{
		for(i = 0; i < dyabs; i++)
		{
			x += dxabs;
			if(x >= dyabs)
			{
				x -= dyabs;
				px += sdx;
			}
			py += sdy;
			_s3d_plot_unsafe(buf,px,py,r,g,b);
		}
	}
}

//----------------------

//http://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland

#define RIGHT  (8)
#define TOP    (4)
#define LEFT   (2)
#define BOTTOM (1)

//Compute the bit code for a point (x, y) using the clip rectangle
//bounded diagonally by (xmin, ymin), and (xmax, ymax)
static inline int _s3d_compute_out_code(int32_t x, int32_t y)
{
	int code = 0;

	if(y >= 240) code |= TOP;             //above the clip window
	else if(y < 0) code |= BOTTOM;     //below the clip window

	if(x >= 400) code |= RIGHT;           //to the right of clip window
	else if(x < 0) code |= LEFT;       //to the left of clip window

	return code;
}

//Cohen–Sutherland clipping algorithm clips a line from
//P1 = (x1, y1) to P2 = (x2, y2) against a rectangle with
//diagonal from (xmin, ymin) to (xmax, ymax).
void S3D_2D_Line(u8 * buf, int x1, int y1, int x2, int y2, int r, int g, int b)
{
	//Outcodes for P1, P1, and whatever point lies outside the clip rectangle
	int outcode1, outcode2, outcodeOut;
	int accept = 0;

	float fx1 = x1;
	float fy1 = y1;
	float fx2 = x2;
	float fy2 = y2;

	//compute outcodes
	outcode1 = _s3d_compute_out_code(fx1, fy1);
	outcode2 = _s3d_compute_out_code(fx2, fy2);

	while(1)
	{
		if((outcode1|outcode2) == 0) //logical or is 0. Trivially accept and get out of loop
		{
			accept = 1; break;
		}
		else if(outcode1 & outcode2) //logical and is not 0. Trivially reject and get out of loop
        {
			break;
		}
		else
		{
			//failed both tests, so calculate the line segment to clip
			//from an outside point to an intersection with clip edge
			float x, y;
			//At least one endpoint is outside the clip rectangle; pick it.
			outcodeOut = outcode1 ? outcode1 : outcode2;
			//Now find the intersection point;
			//use formulas y = y0 + slope * (x - x0), x = x0 + (1/slope)* (y - y0)
			if(outcodeOut & TOP) //point is above the clip rectangle
			{
				x = fx1 + ( ( (fx2 - fx1) * ( ((float)(240-1)) - fy1 ) ) / (fy2 - fy1) );
				y = ((float)(240-1));
			}
			else if(outcodeOut & BOTTOM) //point is below the clip rectangle
			{
			    x = fx1 + ( ( (fx2 - fx1) * ( 0.0f - fy1 ) ) / (fy2 - fy1) );
				y = 0.0f;
			}
			else if(outcodeOut & RIGHT) //point is to the right of clip rectangle
			{
			    y = fy1 + ( ( (fy2 - fy1) * ( ((float)(400-1)) - fx1 ) ) / (fx2 - fx1) );
				x = ((float)(400-1));
			}
			else //if(outcodeOut & LEFT) //point is to the left of clip rectangle
			{
				y = fy1 + ( ( (fy2 - fy1) * ( 0.0f - fx1 ) ) / (fx2 - fx1) );
				x = 0.0f;
			}
			//Now we move outside point to intersection point to clip
			//and get ready for next pass.
			if(outcodeOut == outcode1)
			{
				fx1 = x;
				fy1 = y;
				outcode1 = _s3d_compute_out_code(fx1, fy1);
			}
			else
			{
				fx2 = x;
				fy2 = y;
				outcode2 = _s3d_compute_out_code(fx2, fy2);
			}
		}
	}

	if(accept) _s3d_line_unsafe(buf, fx1,fy1, fx2,fy2, r,g,b);
}

//----------------------

void S3D_2D_LineEx(u8 * buf, int thickness, int x1, int y1, int x2, int y2, int r, int g, int b)
{
    thickness --;

    int i,j;
    int hw = thickness/2;

    for(i = -hw; i <= -hw+thickness; i++)
        for(j = -hw; j <= -hw+thickness; j++)
            S3D_2D_Line(buf,x1+i,y1+j,x2+i,y2+j,r,g,b);
}

//----------------------

void S3D_2D_TriangleFill(u8 * buf, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b)
{
	y1 = int2fx(y1); y2 = int2fx(y2); y3 = int2fx(y3);
	
	// Sort: x1 < x2 < x3
	if(x1 > x2) { swapints_pairs(&x1,&x2, &y1,&y2); }
	if(x2 > x3) { swapints_pairs(&x2,&x3, &y2,&y3); if(x1 > x2) { swapints_pairs(&x1,&x2, &y1,&y2); } }
	
	int dy1, dy2, dy3;
	if(x1 != x2) dy1=fxdiv(y2-y1,int2fx(x2-x1)); else dy1=0;
	if(x1 != x3) dy2=fxdiv(y3-y1,int2fx(x3-x1)); else dy2=0;
	if(x2 != x3) dy3=fxdiv(y3-y2,int2fx(x3-x2)); else dy3=0;
	
	int sx = x1;
	int sy = y1; int ey = y1;
	
	if(x2 >= 400) x2 = 400-1;
	if(x3 >= 400) x3 = 400-1;

	u8 * linebuf = &(buf[240*sx*3]);

#if 1
	if(x1 != x2) // x1 != x2
	{
		if(x1 != x3) // x1 != x2, x1 != x3
		{
			if(x2 != x3)  // x1 != x2, x1 != x3, x2 != x3
			{
				//Optimize for the most common case
				
				if(dy1 > dy2)
				{
					for( ;sx<x2; sx++,sy+=dy2,ey+=dy1,linebuf+=240*3) if( sx >= 0 )
						_s3d_vertical_line_downwards(linebuf, sx,fx2int(sy),fx2int(ey), r,g,b);
					ey = y2;
					for( ;sx<x3; sx++,sy+=dy2,ey+=dy3,linebuf+=240*3) if( sx >= 0 )
						_s3d_vertical_line_downwards(linebuf, sx,fx2int(sy),fx2int(ey), r,g,b);
					
					_s3d_plot_safe(buf, x3,fx2int(y3), r,g,b);
				}
				else
				{
					for( ;sx<x2; sx++,sy+=dy2,ey+=dy1,linebuf+=240*3) if( sx >= 0 )
						_s3d_vertical_line_downwards(linebuf, sx,fx2int(ey),fx2int(sy), r,g,b);
					ey = y2;
					for( ;sx<x3; sx++,sy+=dy2,ey+=dy3,linebuf+=240*3) if( sx >= 0 )
						_s3d_vertical_line_downwards(linebuf, sx,fx2int(ey),fx2int(sy), r,g,b);
					
					_s3d_plot_safe(buf, x3,fx2int(y3), r,g,b);
				}
				
				return;
			}
		}
	}

	for( ;sx<x2; sx++,sy+=dy2,ey+=dy1,linebuf+=240*3) if( sx >= 0 )
		_s3d_vertical_line(linebuf, sx,fx2int(sy),fx2int(ey), r,g,b);
	ey = y2;
	for( ;sx<x3; sx++,sy+=dy2,ey+=dy3,linebuf+=240*3) if( sx >= 0 )
		_s3d_vertical_line(linebuf, sx,fx2int(sy),fx2int(ey), r,g,b);
	
	_s3d_plot_safe(buf, x3,fx2int(y3), r,g,b);
	
#else
	//Safe invalid way of doing it... If x coordinates are the same, it fails
	for( ;sx<=x2; sx++,sy+=dy2,ey+=dy1,linebuf+=240*3) if( sx >= 0 )
		_s3d_vertical_line(linebuf, sx,fx2int(sy),fx2int(ey), r,g,b);
	ey = y2;
	for( ;sx<=x3; sx++,sy+=dy2,ey+=dy3,linebuf+=240*3) if( sx >= 0 )
		_s3d_vertical_line(linebuf, sx,fx2int(sy),fx2int(ey), r,g,b);
#endif
}

//---------------------------------------------------------------------------------------

static u32 currcolor_r, currcolor_g, currcolor_b;
static u32 saved_r = 255, saved_g = 255, saved_b = 255;

#define MAX_LIGHT_SOURCES (2) // no more than 32
static v4 light_dir[MAX_LIGHT_SOURCES];
static v4 light_color[MAX_LIGHT_SOURCES];
static u32 light_enabled = 0;
static int amb_r = 0, amb_g = 0, amb_b = 0;

//---------------------------------------------

void S3D_PolygonColor(u32 r, u32 g, u32 b)
{
	currcolor_r = saved_r = r;
	currcolor_g = saved_g = g;
	currcolor_b = saved_b = b;
}

//---------------------------------------------

void S3D_PolygonNormal(s32 x, s32 y, s32 z)
{
	v4 l = { x, y, z, 0};
	
	int fr = amb_r, fg = amb_g, fb = amb_b; // add factors
	
	int i;
	for(i = 0; i < MAX_LIGHT_SOURCES; i++) if(light_enabled & S3D_LIGHT_N(i))
	{
		s32 factor = -v4_DotProduct(&l,&(light_dir[i])); // change sign because light goes AGAINST a normal
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

//---------------------------------------------

void S3D_LightAmbientColorSet(int r, int g, int b)
{
	amb_r = r; amb_g = g; amb_b = b;
}

void S3D_LightDirectionalColorSet(int index, int r, int g, int b)
{
	light_color[index][0] = r; light_color[index][1] = g; light_color[index][2] = b;
}

void S3D_LightDirectionalVectorSet(int index, s32 x, s32 y, s32 z)
{
	light_dir[index][0] = x; light_dir[index][1] = y; light_dir[index][2] = z;
}

void S3D_LightEnable(u32 enable_mask)
{
	light_enabled = enable_mask;
}

//---------------------------------------------------------------------------------------

static s3d_primitive currmode = S3D_TRIANGLES;
static u32 vtx_count = 0;
#define MAX_VERTICES_IN_A_PRIMITIVE (4)
static v4 vtx_array[MAX_VERTICES_IN_A_PRIMITIVE];
static const u32 primitive_num_vertices[S3D_PRIMITIVE_NUMBER] = {2,3,4,2,3,4};
static u32 curr_max_vertices;

void S3D_PolygonBegin(s3d_primitive type)
{
	currmode = type;
	vtx_count = 0;
	curr_max_vertices = primitive_num_vertices[currmode];
}

//---------------------------------------------

#define LINE_THICKNESS (3)

void _s3d_draw_line(int x1, int y1, int x2, int y2, int r, int g, int b)
{
	S3D_2D_LineEx(curr_buf,LINE_THICKNESS,x1,y1,x2,y2,r,g,b);
}

void _s3d_draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b)
{
	if(!_s3d_check_face(x1,y1,x2,y2,x3,y3)) return;
	
	S3D_2D_TriangleFill(curr_buf,x1,y1,x2,y2,x3,y3,r,g,b);
}

void _s3d_draw_quad(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int r, int g, int b)
{
	if(!_s3d_check_face(x1,y1,x2,y2,x3,y3)) return;

	if( (x1==x2) || (x2==x3) || (x3==x4) || (x4==x1) )
	{
		// Nothing
		S3D_2D_TriangleFill(curr_buf,x1,y1,x2,y2,x3,y3,r,g,b);
		S3D_2D_TriangleFill(curr_buf,x1,y1,x3,y3,x4,y4,r,g,b);
	}
	else
	{
		// Divide the quad in two triangles reducing the delta X between the edges
		// because the triangles are drawn by vertical lines
	
		// Sort: x1 < x2,x4 < x3
		if(x1 > x2) 
		{
			roll_values(&x1,&x2,&x3,&x4,&y1,&y2,&y3,&y4);
			if(x1 > x2) 
			{
				roll_values(&x1,&x2,&x3,&x4,&y1,&y2,&y3,&y4);
				if(x1 > x2) roll_values(&x1,&x2,&x3,&x4,&y1,&y2,&y3,&y4);
			}
		}
		
		S3D_2D_TriangleFill(curr_buf,x1,y1,x2,y2,x4,y4,r,g,b);
		S3D_2D_TriangleFill(curr_buf,x2,y2,x3,y3,x4,y4,r,g,b);
	}
}

//---------------------------------------------

//In engine.c
extern m44 S3D_GLOBAL_MATRIX;
inline void _s3d_global_matrix_update(void);

//In polygon.c
void _s3d_polygon_list_add_line(v4 * a, v4 * b, int _r, int _g, int _b);
void _s3d_polygon_list_add_triangle(v4 * a, v4 * b, v4 * c, int _r, int _g, int _b);
void _s3d_polygon_list_add_quad(v4 * a, v4 * b, v4 * c, v4 * d, int _r, int _g, int _b);

void S3D_PolygonVertex(s32 x, s32 y, s32 z)
{
	v4 v = { x, y, z, int2fx(1) };
	v4 result;
	
	_s3d_global_matrix_update();
	
	m44_v4_Multiply(&S3D_GLOBAL_MATRIX,&v,&result);
	
	vtx_array[vtx_count][0] = fx2int(fxdiv(result[0],result[3]));
	vtx_array[vtx_count][1] = fx2int(fxdiv(result[1],result[3]));
	vtx_array[vtx_count][3] = result[3];
	vtx_count++;
	
	if(vtx_count == curr_max_vertices)
	{
		switch(currmode)
		{
			case S3D_LINES:
				_s3d_polygon_list_add_line(&(vtx_array[0]),&(vtx_array[1]),
						currcolor_r,currcolor_g,currcolor_b);
				
				vtx_count = 0;
				break;
				
			case S3D_TRIANGLES:
				_s3d_polygon_list_add_triangle(&(vtx_array[0]),&(vtx_array[1]),&(vtx_array[2]),
						currcolor_r,currcolor_g,currcolor_b);
				
				vtx_count = 0;
				break;
				
			case S3D_QUADS:
				_s3d_polygon_list_add_quad(&(vtx_array[0]),&(vtx_array[1]),&(vtx_array[2]),&(vtx_array[3]),
						currcolor_r,currcolor_g,currcolor_b);
				
				vtx_count = 0;
				break;
			
			case S3D_LINE_STRIP:
				_s3d_polygon_list_add_line(&(vtx_array[0]),&(vtx_array[1]),
						currcolor_r,currcolor_g,currcolor_b);
				
				vtx_array[0][0] = vtx_array[1][0];
				vtx_array[0][1] = vtx_array[1][1];
				vtx_array[0][3] = vtx_array[1][3];
				
				vtx_count = 1;
				break;
				
			case S3D_TRIANGLE_STRIP:
				_s3d_polygon_list_add_triangle(&(vtx_array[0]),&(vtx_array[1]),&(vtx_array[2]),
						currcolor_r,currcolor_g,currcolor_b);
				
				vtx_array[0][0] = vtx_array[2][0];
				vtx_array[0][1] = vtx_array[2][1];
				vtx_array[0][3] = vtx_array[2][3];
				
				vtx_count = 2;
				break;
				
			case S3D_QUAD_STRIP:
				_s3d_polygon_list_add_quad(&(vtx_array[0]),&(vtx_array[1]),&(vtx_array[2]),&(vtx_array[3]),
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
