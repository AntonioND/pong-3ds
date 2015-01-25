
#ifndef __PLOT__
#define __PLOT__

#include <3ds.h>

//--------------------------------------------------------------------------------------------------

void Line(u8 * buf, int x1, int y1, int x2, int y2, int r, int g, int b);

void LineEx(u8 * buf, int line_width, int x1, int y1, int x2, int y2, int r, int g, int b);

void TriFill(u8 * buf, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b);

//--------------------------------------------------------------------------------------------------

#endif //__PLOT__
