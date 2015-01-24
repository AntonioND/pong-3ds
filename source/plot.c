
#include <3ds.h>

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

static inline void _plot(u8 * buf, u32 pitch, u32 height, u32 x, u32 y, int r, int g, int b)
{
	u8 * p = &(buf[(height*x+y)*3]);
	*p = b; p++;
	*p = g; p++;
	*p = r;
}

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

void _Line(u8 * buf, u32 pitch, u32 height, int x1, int y1, int x2, int y2, int r, int g, int b)
{
	int i,dx,dy,sdx,sdy,dxabs,dyabs,x,y,px,py;

	dx=x2-x1;      /* the horizontal distance of the line */
	dy=y2-y1;      /* the vertical distance of the line */
	dxabs=abs(dx);
	dyabs=abs(dy);
	sdx=sgn(dx);
	sdy=sgn(dy);
	x=dyabs>>1;
	y=dxabs>>1;
	px=x1;
	py=y1;

	_plot(buf,400,240,px,py,r,g,b);
	

	if (dxabs>=dyabs) /* the line is more horizontal than vertical */
	{
		for(i=0;i<dxabs;i++)
		{
			y+=dyabs;
			if (y>=dxabs)
			{
				y-=dxabs;
				py+=sdy;
			}
			px+=sdx;
			_plot(buf,400,240,px,py,r,g,b);
		}
	}
	else /* the line is more vertical than horizontal */
	{
		for(i=0;i<dyabs;i++)
		{
			x+=dxabs;
			if (x>=dyabs)
			{
				x-=dyabs;
				px+=sdx;
			}
			py+=sdy;
			_plot(buf,400,240,px,py,r,g,b);
		}
	}
}

//--------------------------------------------------------------------

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
void Line(u8 * buf, u32 pitch, u32 height, int x1, int y1, int x2, int y2, int r, int g, int b)
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

	if(accept) _Line(buf,pitch,height, fx1,fy1,fx2,fy2,r,g,b);
}

//--------------------------------------------------------------------

void LineEx(u8 * buf, u32 pitch, u32 height, int line_width, int x1, int y1, int x2, int y2, int r, int g, int b)
{
    line_width --;

    int i,j;
    int hw = line_width/2;

    for(i = -hw; i <= -hw+line_width; i++)
        for(j = -hw; j <= -hw+line_width; j++)
            Line(buf,pitch,height,x1+i,y1+j,x2+i,y2+j,r,g,b);
}

void Rect(u8 * buf, u32 pitch, u32 height, int x1, int y1, int x2, int y2, int r, int g, int b)
{
    Line(buf,pitch,height,x1,y1,x1,y2,r,g,b);
    Line(buf,pitch,height,x1,y2,x2,y2,r,g,b);
    Line(buf,pitch,height,x1,y1,x2,y1,r,g,b);
    Line(buf,pitch,height,x2,y1,x2,y2,r,g,b);
}

void RectFill(u8 * buf, u32 pitch, u32 height, int x1, int y1, int x2, int y2, int r, int g, int b)
{
    if(y2 < y1)
    {
        int temp = y2;
        y2 = y1;
        y1 = temp;
    }

    int j;
    for(j = y1; j <= y2; j++)
        Line(buf,pitch,height,x1,j,x2,j,r,g,b);
}

void RectEx(u8 * buf, uint32_t pitch, uint32_t height, int line_width, int x1, int y1, int x2, int y2, int r, int g, int b)
{
    if(y2 < y1)
    {
        int temp = y2;
        y2 = y1;
        y1 = temp;
    }

    if(x2 < x1)
    {
        int temp = x2;
        x2 = x1;
        x1 = temp;
    }

    line_width --;

    int i;
    for(i = 0; i <= line_width; i++)
        Rect(buf,pitch,height,x1+i,y1+i,x2-i,y2-i,r,g,b);
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
