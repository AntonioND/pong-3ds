
//--------------------------------------------------------------------------------------------------

#include <3ds.h>
#include <string.h>

#include "matrix.h"
#include "engine.h"
#include "draw.h"

//---------------------------------------------

//In engine.c
extern m44 S3D_GLOBAL_MATRIX[2];
extern m44 PROJECTION[2], MODELVIEW[2];
inline void _s3d_global_matrix_update(int screen);

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
	int dx1 = x2-x1; int dy1 = y2-y1;
	if((dx1 & dy1) == 0) return 1; // if 2 pixels are the same, draw the polygon always
	int dx2 = x3-x1; int dy2 = y3-y1;
	if((dx2 & dy2) == 0) return 1;

	int cross = dx1*dy2 - dx2*dy1; // Z axis goes away (to the inside of the screen)
	if((cross > 0) && !_s3d_draw_back[screen]) return 0;
	if((cross < 0) && !_s3d_draw_front[screen]) return 0;
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
	if(x > 0) return 1;
	if(x < 0) return -1;
	return 0;
}

static inline void swapints(int * a1, int * a2)
{
	int t = *a1; *a1 = *a2; *a2 = t;
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

	if(y >= int2fx(240)) code |= TOP;      //above the clip window
	else if(y < int2fx(0)) code |= BOTTOM; //below the clip window

	if(x >= int2fx(400)) code |= RIGHT;    //to the right of clip window
	else if(x < int2fx(0)) code |= LEFT;   //to the left of clip window

	return code;
}

//Cohen–Sutherland clipping algorithm clips a line from
//P1 = (x1, y1) to P2 = (x2, y2) against a rectangle with
//diagonal from (xmin, ymin) to (xmax, ymax).
void S3D_2D_Line(u8 * buf, int x1, int y1, int x2, int y2, int r, int g, int b)
{
	//Outcodes for P1, P1, and whatever point lies outside the clip rectangle
	int outcode1, outcode2, outcodeOut;

	s32 fx1 = int2fx(x1);
	s32 fy1 = int2fx(y1);
	s32 fx2 = int2fx(x2);
	s32 fy2 = int2fx(y2);

	//compute outcodes
	outcode1 = _s3d_compute_out_code(fx1, fy1);
	outcode2 = _s3d_compute_out_code(fx2, fy2);

	while(1)
	{
		if((outcode1|outcode2) == 0) //logical or is 0. Trivially accept and get out of loop
		{
			_s3d_line_unsafe(buf, fx2int(fx1),fx2int(fy1), fx2int(fx2),fx2int(fy2), r,g,b);
			return;
		}
		else if(outcode1 & outcode2) //logical and is not 0. Trivially reject and exit
        {
			return;
		}
		else
		{
			//failed both tests, so calculate the line segment to clip
			//from an outside point to an intersection with clip edge
			s32 x, y;
			//At least one endpoint is outside the clip rectangle; pick it.
			outcodeOut = outcode1 ? outcode1 : outcode2;
			//Now find the intersection point;
			//use formulas y = y0 + slope * (x - x0), x = x0 + (1/slope)* (y - y0)
			if(outcodeOut & TOP) //point is above the clip rectangle
			{
				x = fx1 + fxdiv64( fxmul64(fx2-fx1,int2fx(240-1)-fy1) , fy2-fy1 );
				y = int2fx(240-1);
			}
			else if(outcodeOut & BOTTOM) //point is below the clip rectangle
			{
				x = fx1 + fxdiv64( fxmul64(fx2-fx1,int2fx(0)-fy1) , fy2-fy1 );
				y = int2fx(0);
			}
			else if(outcodeOut & RIGHT) //point is to the right of clip rectangle
			{
				y = fy1 + fxdiv64( fxmul64(fy2-fy1,int2fx(400-1)-fx1) , fx2-fx1 );
				x = int2fx(400-1);
			}
			else //if(outcodeOut & LEFT) //point is to the left of clip rectangle
			{
				y = fy1 + fxdiv64( fxmul64(fy2-fy1,int2fx(0)-fx1) , fx2-fx1 );
				x = int2fx(0);
			}
			//Now we move outside point to intersection point to clip
			//and get ready for next pass.
			if(outcodeOut == outcode1)
			{
				fx1 = x; fy1 = y;
				outcode1 = _s3d_compute_out_code(fx1, fy1);
			}
			else
			{
				fx2 = x; fy2 = y;
				outcode2 = _s3d_compute_out_code(fx2, fy2);
			}
		}
	}
}

//----------------------

void S3D_2D_Plot(u8 * buf, int x, int y, int r, int g, int b)
{
	_s3d_plot_safe(buf,x,y,r,g,b);
}

void S3D_2D_PlotEx(u8 * buf, int thickness, int x, int y, int r, int g, int b)
{
    thickness --;

    int i,j;
    int hw = thickness/2;

    for(i = -hw; i <= -hw+thickness; i++)
        for(j = -hw; j <= -hw+thickness; j++)
            _s3d_plot_safe(buf,x+i,y+j,r,g,b);
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
	
	// Sort: x1 <= x2 <= x3
	if(x1 > x2) { swapints_pairs(&x1,&x2, &y1,&y2); }
	if(x2 > x3) { swapints_pairs(&x2,&x3, &y2,&y3); if(x1 > x2) { swapints_pairs(&x1,&x2, &y1,&y2); } }

	u8 * linebuf = &(buf[240*x1*3]);
	
	if(x1 != x2) // x1 != x2
	{
		int dy1 = fxdiv64(y2-y1,int2fx(x2-x1));
		
		//if(x1 != x3) // x1 != x2, x1 != x3
		{
			int dy2 = fxdiv64(y3-y1,int2fx(x3-x1));
			
			if(x2 != x3)  // x1 != x2, x1 != x3, x2 != x3 -> Most common case
			{
				int dy3 = fxdiv64(y3-y2,int2fx(x3-x2));
				
				int sx = x1; int sy = y1; int ey = y1;
				
				if(dy1 > dy2)
				{
					for( ;sx<x2; sx++,sy+=dy2,ey+=dy1,linebuf+=240*3) if( (u32)sx < 400 )
						_s3d_vertical_line_downwards(linebuf, sx,fx2int(sy),fx2int(ey), r,g,b);
					ey = y2;
					for( ;sx<x3; sx++,sy+=dy2,ey+=dy3,linebuf+=240*3) if( (u32)sx < 400 )
						_s3d_vertical_line_downwards(linebuf, sx,fx2int(sy),fx2int(ey), r,g,b);
					_s3d_plot_safe(buf, x3,fx2int(y3), r,g,b);
				}
				else //if(dy1 <= dy2)
				{
					for( ;sx<x2; sx++,sy+=dy2,ey+=dy1,linebuf+=240*3) if( (u32)sx < 400 )
						_s3d_vertical_line_downwards(linebuf, sx,fx2int(ey),fx2int(sy), r,g,b);
					ey = y2;
					for( ;sx<x3; sx++,sy+=dy2,ey+=dy3,linebuf+=240*3) if( (u32)sx < 400 )
						_s3d_vertical_line_downwards(linebuf, sx,fx2int(ey),fx2int(sy), r,g,b);
					_s3d_plot_safe(buf, x3,fx2int(y3), r,g,b);
				}
				
				return;
			}
			else  // x1 != x2, x1 != x3, x2 == x3
			{
				int sx = x1; int sy = y1; int ey = y1;
				
				if(dy1 > dy2)
				{
					for( ;sx<x2; sx++,sy+=dy2,ey+=dy1,linebuf+=240*3) if( (u32)sx < 400 )
						_s3d_vertical_line_downwards(linebuf, sx,fx2int(sy),fx2int(ey), r,g,b);
					if( sx >= 0 ) _s3d_vertical_line_downwards(linebuf, sx,fx2int(y3),fx2int(y2), r,g,b);
				}
				else //if(dy1 <= dy2)
				{
					for( ;sx<x2; sx++,sy+=dy2,ey+=dy1,linebuf+=240*3) if( (u32)sx < 400 )
						_s3d_vertical_line_downwards(linebuf, sx,fx2int(ey),fx2int(sy), r,g,b);
					if( sx >= 0 ) _s3d_vertical_line_downwards(linebuf, sx,fx2int(y2),fx2int(y3), r,g,b);
				}
				
				return;
			}
		}
		//else // x1 != x2, x1 == x3
		//{
		//	//Impossible because x1 <= x2 <= x3
		//	printf("C");
		//	return;
		//}
	}
	else // x1 == x2
	{
		if(x1 != x3) // x1 == x2, x1 != x3
		{
			int dy2 = fxdiv64(y3-y1,int2fx(x3-x1));
			
			//if(x2 != x3)  // x1 == x2, x1 != x3, x2 != x3
			{
				int dy3 = fxdiv64(y3-y2,int2fx(x3-x2));
				
				int sx = x1;
				int sy = y1; int ey = y2;
				
				if(y1 > y2)
				{
					for( ;sx<x3; sx++,sy+=dy2,ey+=dy3,linebuf+=240*3) if( (u32)sx < 400 )
						_s3d_vertical_line_downwards(linebuf, sx,fx2int(ey),fx2int(sy), r,g,b);
					_s3d_plot_safe(buf, x3,fx2int(y3), r,g,b);
				}
				else //if(dy1 <= dy2)
				{
					for( ;sx<x3; sx++,sy+=dy2,ey+=dy3,linebuf+=240*3) if( (u32)sx < 400 )
						_s3d_vertical_line_downwards(linebuf, sx,fx2int(sy),fx2int(ey), r,g,b);
					_s3d_plot_safe(buf, x3,fx2int(y3), r,g,b);
				}
				
				return;
			}
			//else // x1 == x2, x1 != x3, x2 == x3
			//{
			//	//Impossible
			//	printf("B");
			//	return;
			//}
		}
		else // x1 == x2, x1 == x3
		{
			//if(x2 != x3)  // x1 == x2, x1 == x3, x2 != x3
			//{
			//	//Impossible
			//	printf("A");
			//	return;
			//}
			//else  // x1 == x2, x1 == x3, x2 == x3
			{
				//Just a line
				if( ((u32)x1) < 400 )
				{
					int ymin = min(min(y1,y2),y3);
					int ymax = max(max(y1,y2),y3);
					_s3d_vertical_line(linebuf, x1,fx2int(ymin),fx2int(ymax), r,g,b);
				}
				return;
			}
		}
	}
	
#if 0
	int sx = x1; int sy = y1; int ey = y1;
	int dy1,dy2,dy3;
	if(x1 != x2) dy1=fxdiv64(y2-y1,int2fx(x2-x1)); else dy1=0;
	if(x1 != x3) dy2=fxdiv64(y3-y1,int2fx(x3-x1)); else dy2=0;
	if(x2 != x3) dy3=fxdiv64(y3-y2,int2fx(x3-x2)); else dy3=0;
	
	//Safe invalid way of doing it... If x coordinates are the same, it fails
	for( ;sx<=x2; sx++,sy+=dy2,ey+=dy1,linebuf+=240*3) if( sx >= 0 )
		_s3d_vertical_line(linebuf, sx,fx2int(sy),fx2int(ey), r,g,b);
	ey = y2;
	for( ;sx<=x3; sx++,sy+=dy2,ey+=dy3,linebuf+=240*3) if( sx >= 0 )
		_s3d_vertical_line(linebuf, sx,fx2int(sy),fx2int(ey), r,g,b);
	
	_s3d_plot_safe(buf, x3,fx2int(y3), r,g,b);
#endif
}

//----------------------

void S3D_2D_QuadAllignedFill(u8 * buf, int x1, int y1, int x2, int y2, int r, int g, int b)
{
	if(y1 > y2) swapints(&y1,&y2);
	if(x1 > x2) swapints(&x1,&x2);
	
	//y1 < y2, x1 < x2
	
	if(y2 < 0) return;
	else if(y2 >= 240) y2 = 240-1;
	
	if(x2 < 0) return;
	else if(x2 >= 400) x2 = 400-1;
	
	if(y1 >= 240) return;
	else if(y1 < 0) y1 = 0;
	
	if(x1 >= 400) return;
	else if(x1 < 0) x1 = 0;
	
	u8 * linebuf = &(buf[240*x1*3]);

	for(;x1<=x2;x1++,linebuf+=240*3)
	{
		int y = y1;
		u8 * p = linebuf + y*3;
		for( ;y<=y2; y++) { *p++ = b; *p++ = g; *p++ = r; }
	}
}

//---------------------------------------------------------------------------------------

static u32 currcolor_r[2], currcolor_g[2], currcolor_b[2];
static u32 saved_r[2] = { 255, 255 }, saved_g[2] = { 255, 255 }, saved_b[2] = { 255, 255 };

#define MAX_LIGHT_SOURCES (2) // no more than 32
static v4 light_dir[2][MAX_LIGHT_SOURCES];
static v4 light_color[2][MAX_LIGHT_SOURCES];
static u32 light_enabled[2] = { 0, 0 };
static int amb_r[2] = { 0, 0 }, amb_g[2] = { 0, 0 }, amb_b[2] = { 0, 0 };

//---------------------------------------------

void S3D_PolygonColor(int screen, u32 r, u32 g, u32 b)
{
	currcolor_r[screen] = saved_r[screen] = r;
	currcolor_g[screen] = saved_g[screen] = g;
	currcolor_b[screen] = saved_b[screen] = b;
}

//---------------------------------------------

void S3D_PolygonNormal(int screen, s32 x, s32 y, s32 z)
{
	v4 temp_l = { x, y, z, 0};
	v4 l;
	_s3d_global_matrix_update(screen);
	
	m44_v4_Multiply(&MODELVIEW[screen],&temp_l,&l);
	
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

void _s3d_draw_dot(int screen, int x, int y, int r, int g, int b)
{
	S3D_2D_PlotEx(curr_buf[screen],LINE_THICKNESS,x,y,r,g,b);
}

void _s3d_draw_line(int screen, int x1, int y1, int x2, int y2, int r, int g, int b)
{
	S3D_2D_LineEx(curr_buf[screen],LINE_THICKNESS,x1,y1,x2,y2,r,g,b);
}

void _s3d_draw_triangle(int screen, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b)
{
	if(!_s3d_check_cull_face(screen,x1,y1,x2,y2,x3,y3)) return;
	
	S3D_2D_TriangleFill(curr_buf[screen],x1,y1,x2,y2,x3,y3,r,g,b);
}

void _s3d_draw_quad(int screen, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int r, int g, int b)
{
	if(!_s3d_check_cull_face(screen,x1,y1,x2,y2,x3,y3)) return;

	if( (x1==x2) || (x2==x3) || (x3==x4) || (x4==x1) )
	{
		// Nothing
		S3D_2D_TriangleFill(curr_buf[screen],x1,y1,x2,y2,x3,y3,r,g,b);
		S3D_2D_TriangleFill(curr_buf[screen],x1,y1,x3,y3,x4,y4,r,g,b);
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
		
		S3D_2D_TriangleFill(curr_buf[screen],x1,y1,x2,y2,x4,y4,r,g,b);
		S3D_2D_TriangleFill(curr_buf[screen],x2,y2,x3,y3,x4,y4,r,g,b);
	}
}

//---------------------------------------------

//In polygon.c
void _s3d_polygon_list_add_dot(int screen, v4 * a, int _r, int _g, int _b);
void _s3d_polygon_list_add_line(int screen, v4 * a, v4 * b, int _r, int _g, int _b);
void _s3d_polygon_list_add_triangle(int screen, v4 * a, v4 * b, v4 * c, int _r, int _g, int _b);
void _s3d_polygon_list_add_quad(int screen, v4 * a, v4 * b, v4 * c, v4 * d, int _r, int _g, int _b);

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
						currcolor_r[screen],currcolor_g[screen],currcolor_b[screen]);
				
				vtx_count[screen] = 0;
				break;
				
			case S3D_LINES:
				_s3d_polygon_list_add_line(screen,
						&(vtx_array[screen][0]),&(vtx_array[screen][1]),
						currcolor_r[screen],currcolor_g[screen],currcolor_b[screen]);
				
				vtx_count[screen] = 0;
				break;
				
			case S3D_TRIANGLES:
				_s3d_polygon_list_add_triangle(screen,
						&(vtx_array[screen][0]),&(vtx_array[screen][1]),&(vtx_array[screen][2]),
						currcolor_r[screen],currcolor_g[screen],currcolor_b[screen]);
				
				vtx_count[screen] = 0;
				break;
				
			case S3D_QUADS:
				_s3d_polygon_list_add_quad(screen,
						&(vtx_array[screen][0]),&(vtx_array[screen][1]),&(vtx_array[screen][2]),&(vtx_array[screen][3]),
						currcolor_r[screen],currcolor_g[screen],currcolor_b[screen]);
				
				vtx_count[screen] = 0;
				break;
			
			case S3D_LINE_STRIP:
				_s3d_polygon_list_add_line(screen,
						&(vtx_array[screen][0]),&(vtx_array[screen][1]),
						currcolor_r[screen],currcolor_g[screen],currcolor_b[screen]);
				
				vtx_array[screen][0][0] = vtx_array[screen][1][0];
				vtx_array[screen][0][1] = vtx_array[screen][1][1];
				vtx_array[screen][0][3] = vtx_array[screen][1][3];
				
				vtx_count[screen] = 1;
				break;
				
			case S3D_TRIANGLE_STRIP:
				_s3d_polygon_list_add_triangle(screen,
						&(vtx_array[screen][0]),&(vtx_array[screen][1]),&(vtx_array[screen][2]),
						currcolor_r[screen],currcolor_g[screen],currcolor_b[screen]);
				
				vtx_array[screen][0][0] = vtx_array[screen][2][0];
				vtx_array[screen][0][1] = vtx_array[screen][2][1];
				vtx_array[screen][0][3] = vtx_array[screen][2][3];
				
				vtx_count[screen] = 2;
				break;
				
			case S3D_QUAD_STRIP:
				_s3d_polygon_list_add_quad(screen,
						&(vtx_array[screen][0]),&(vtx_array[screen][1]),&(vtx_array[screen][2]),&(vtx_array[screen][3]),
						currcolor_r[screen],currcolor_g[screen],currcolor_b[screen]);
				
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

//---------------------------------------------------------------------------------------
