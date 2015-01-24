
#ifndef __PLOT__
#define __PLOT__

#include <3ds.h>

//--------------------------------------------------------------------------------------------------

void Line(u8 * buf, u32 pitch, u32 height, int x1, int y1, int x2, int y2, int r, int g, int b);

void LineEx(u8 * buf, u32 pitch, u32 height, int line_width, int x1, int y1, int x2, int y2, int r, int g, int b);

void Rect(u8 * buf, u32 pitch, u32 height, int x1, int y1, int x2, int y2, int r, int g, int b);

void RectFill(u8 * buf, u32 pitch, u32 height, int x1, int y1, int x2, int y2, int r, int g, int b);

void RectEx(u8 * buf, uint32_t pitch, uint32_t height, int line_width, int x1, int y1, int x2, int y2, int r, int g, int b);

//--------------------------------------------------------------------------------------------------

#endif //__PLOT__
