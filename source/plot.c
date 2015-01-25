
#include <3ds.h>

#include "math.h"

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

static inline int abs(int x)
{
	return x < 0 ? -x : x;
}

static inline int sgn(int x)
{
	if(x < 0) return -1;
	if(x > 0) return 1;
	return 0;
}

static inline void swapints(int * a, int * b)
{
	int t = *a;
	*a = *b;
	*b = t;
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

static inline void _plot(u8 * buf, u32 * zbuf, u32 pitch, u32 height, u32 x, u32 y, u32 w, int r, int g, int b)
{
	if((x >= pitch) || (y >= height)) return;

	u8 * p = &(buf[(height*x+y)*3]);
	*p++ = b; *p++ = g; *p = r;
	
}

static inline void __horline(u8 * buf, u32 * zbuf, u32 pitch, int height,
							u32 x, int y1, int y2, u32 w1, u32 w2,
							int r, int g, int b)
{
	//make the X coordinate unsigned to avoid the < 0 comparison
	if(x >= pitch) return;
	
	if(y1 < 0)
	{
		if(y2 < 0) return;
		y1 = 0;
	}
	else if(y1 >= height)
	{
		if(y2 >= height) return;
		y1 = height-1;
	}
	else
	{
		if(y2 < 0) y2 = 0;
		else if(y2 >= height) y2 = height-1;
	}
	
	if(y1 < y2)
	{
		u8 * p = &(buf[(height*x+y1)*3]);
		for( ;y1<y2; y1++) { *p++ = b; *p++ = g; *p++ = r; }
	}
	else
	{
		u8 * p = &(buf[(height*x+y2)*3]);
		for( ;y2<y1; y2++) { *p++ = b; *p++ = g; *p++ = r; }
	}
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void Line(u8 * buf, u32 * zbuf, u32 pitch, u32 height,
			int x1, int y1, u32 w1,
			int x2, int y2, u32 w2,
			int r, int g, int b)
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

	_plot(buf,zbuf,400,240,px,py,0,r,g,b);
	
	if(dxabs>=dyabs) /* the line is more horizontal than vertical */
	{
		for(i=0;i<dxabs;i++)
		{
			y+=dyabs;
			if(y>=dxabs)
			{
				y-=dxabs;
				py+=sdy;
			}
			px+=sdx;
			_plot(buf,zbuf,400,240,px,py,0,r,g,b);
		}
	}
	else /* the line is more vertical than horizontal */
	{
		for(i=0;i<dyabs;i++)
		{
			x+=dxabs;
			if(x>=dyabs)
			{
				x-=dyabs;
				px+=sdx;
			}
			py+=sdy;
			_plot(buf,zbuf,400,240,px,py,0,r,g,b);
		}
	}
}

//--------------------------------------------------------------------
/*
//http://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland

#define RIGHT  (8)
#define TOP    (4)
#define LEFT   (2)
#define BOTTOM (1)

//Compute the bit code for a point (x, y) using the clip rectangle
//bounded diagonally by (xmin, ymin), and (xmax, ymax)
static inline int __ComputeOutCode(int32_t pitch, int32_t height, int32_t x, int32_t y)
{
	int code = 0;

	if(y >= height) code |= TOP;             //above the clip window
	else if(y < 0) code |= BOTTOM;     //below the clip window

	if(x >= pitch) code |= RIGHT;           //to the right of clip window
	else if(x < 0) code |= LEFT;       //to the left of clip window

	return code;
}

//Cohen–Sutherland clipping algorithm clips a line from
//P1 = (x1, y1) to P2 = (x2, y2) against a rectangle with
//diagonal from (xmin, ymin) to (xmax, ymax).
void Line(u8 * buf, u32 * zbuf, u32 pitch, u32 height,
			int x1, int y1, u32 w1,
			int x2, int y2, u32 w2,
			int r, int g, int b)
{
	//Outcodes for P1, P1, and whatever point lies outside the clip rectangle
	int outcode1, outcode2, outcodeOut;
	int accept = 0;

	float fx1 = x1;
	float fy1 = y1;
	float fx2 = x2;
	float fy2 = y2;

	//compute outcodes
	outcode1 = __ComputeOutCode(pitch, height, fx1, fy1);
	outcode2 = __ComputeOutCode(pitch, height, fx2, fy2);

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
				x = fx1 + ( ( (fx2 - fx1) * ( ((float)(height-1)) - fy1 ) ) / (fy2 - fy1) );
				y = ((float)(height-1));
			}
			else if(outcodeOut & BOTTOM) //point is below the clip rectangle
			{
			    x = fx1 + ( ( (fx2 - fx1) * ( 0.0f - fy1 ) ) / (fy2 - fy1) );
				y = 0.0f;
			}
			else if(outcodeOut & RIGHT) //point is to the right of clip rectangle
			{
			    y = fy1 + ( ( (fy2 - fy1) * ( ((float)(pitch-1)) - fx1 ) ) / (fx2 - fx1) );
				x = ((float)(pitch-1));
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
				outcode1 = __ComputeOutCode(pitch, height, fx1, fy1);
			}
			else
			{
				fx2 = x;
				fy2 = y;
				outcode2 = __ComputeOutCode(pitch, height, fx2, fy2);
			}
		}
	}

	if(accept) _Line(buf,zbuf,pitch,height, fx1,fy1,0, fx2,fy2,0, r,g,b);
}
*/
//--------------------------------------------------------------------

void LineEx(u8 * buf, u32 * zbuf, u32 pitch, u32 height, int line_width,
			int x1, int y1, u32 w1,
			int x2, int y2, u32 w2,
			int r, int g, int b)
{
    line_width --;

    int i,j;
    int hw = line_width/2;

    for(i = -hw; i <= -hw+line_width; i++)
        for(j = -hw; j <= -hw+line_width; j++)
            Line(buf,zbuf,pitch,height,x1+i,y1+j,w1,x2+i,y2+j,w2,r,g,b);
}

//--------------------------------------------------------------------

void TriFill(u8 * buf, u32 * zbuf, uint32_t pitch, uint32_t height,
			int x1, int y1, u32 w1,
			int x2, int y2, u32 w2,
			int x3, int y3, u32 w3,
			int r, int g, int b)
{
	y1 = int2fx(y1); y2 = int2fx(y2); y3 = int2fx(y3);
	
	// x1 < x2 < x3
	if(x1 > x2) { swapints(&x1,&x2); swapints(&y1,&y2); }
	if(x2 > x3) { swapints(&x2,&x3); swapints(&y2,&y3); if(x1 > x2) { swapints(&x1,&x2); swapints(&y1,&y2); } }
	
	int dy1, dy2, dy3;
	if(x2 != x1) dy1=fxdiv(y2-y1,int2fx(x2-x1)); else dy1=0;
	if(x3 != x1) dy2=fxdiv(y3-y1,int2fx(x3-x1)); else dy2=0;
	if(x3 != x2) dy3=fxdiv(y3-y2,int2fx(x3-x2)); else dy3=0;
	
	int sx = x1;
	int sy = y1; int ey = y1;
	
	for( ;sx<=x2; sx++,sy+=dy2,ey+=dy1)
		__horline(buf,zbuf,pitch,height, sx,fx2int(sy),fx2int(ey),0,0, r,g,b);
	ey = y2;
	for( ;sx<=x3; sx++,sy+=dy2,ey+=dy3)
		__horline(buf,zbuf,pitch,height, sx,fx2int(sy),fx2int(ey),0,0, r,g,b);
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
