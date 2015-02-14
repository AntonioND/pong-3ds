
#ifndef __S3D_DRAW__
#define __S3D_DRAW__

//----------------------------------------------------------------------------------

#include <3ds.h>

//--------------------------------------------------------------------------------------------------

// (0,240-1) ---------- (400-1,240-1)
//    |                       |
//    |     TOP  SCREENS      |
//    |        (x,y)          |
//    |                       |
//  (0,0) -------------- (400-1,0)

void S3D_2D_Plot(u8 * buf, int x, int y, int r, int g, int b, int a);
void S3D_2D_PlotEx(u8 * buf, int thickness, int x, int y, int r, int g, int b, int a);

void S3D_2D_Line(u8 * buf, int x1, int y1, int x2, int y2, int r, int g, int b, int a);
void S3D_2D_LineEx(u8 * buf, int thickness, int x1, int y1, int x2, int y2, int r, int g, int b, int a);

void S3D_2D_TriangleFill(u8 * buf, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b);
void S3D_2D_TriangleFillAlpha(u8 * buf, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b, int a);

void S3D_2D_QuadAllignedFill(u8 * buf, int x1, int y1, int x2, int y2, int r, int g, int b);

// Must be convex!!
void S3D_2D_QuadFill(u8 * buf, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int r, int g, int b);
void S3D_2D_QuadFillAlpha(u8 * buf, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int r, int g, int b, int a);

//--------------------------------------------------------------------------------------------------

#endif //__S3D_DRAW__

