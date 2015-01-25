
#ifndef __PLOT__
#define __PLOT__

#include <3ds.h>

//--------------------------------------------------------------------------------------------------

void Line(u8 * buf, u32 * zbuf, u32 pitch, u32 height,
			int x1, int y1, u32 w1,
			int x2, int y2, u32 w2,
			int r, int g, int b);

void LineEx(u8 * buf, u32 * zbuf, u32 pitch, u32 height, int line_width,
			int x1, int y1, u32 w1,
			int x2, int y2, u32 w2,
			int r, int g, int b);

void TriFill(u8 * buf, u32 * zbuf, uint32_t pitch, uint32_t height,
			int x1, int y1, u32 w1,
			int x2, int y2, u32 w2,
			int x3, int y3, u32 w3,
			int r, int g, int b);

//--------------------------------------------------------------------------------------------------

#endif //__PLOT__
