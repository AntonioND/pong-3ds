
#ifndef __S3D_DRAW__
#define __S3D_DRAW__

//----------------------------------------------------------------------------------

#include <3ds.h>

//----------------------------------------------------------------------------------

void S3D_BuffersSetup(void); // must be called at the beginning of each frame

u8 * S3D_BufferGet(int screen);

//----------------------------------------------------------------------------------

// in utils_asm.s
void S3D_FramebufferFill(void * dst, u32 color, u32 bottom_screen); // color = 31 _BGR 0

void S3D_FramebuffersClearTopScreen(int screen, int r, int g, int b);

//--------------------------------------------------------------------------------------------------

// (0,240-1) ---------- (400-1,240-1)
//    |                       |
//    |     TOP      (x,y)    |
//    |        SCREENS        |
//    |                       |
//  (0,0) -------------- (400-1,0)

void S3D_2D_Plot(u8 * buf, int x, int y, int r, int g, int b, int a);
void S3D_2D_PlotEx(u8 * buf, int thickness, int x, int y, int r, int g, int b, int a);

void S3D_2D_Line(u8 * buf, int x1, int y1, int x2, int y2, int r, int g, int b, int a);
void S3D_2D_LineEx(u8 * buf, int thickness, int x1, int y1, int x2, int y2, int r, int g, int b, int a);

void S3D_2D_TriangleFill(u8 * buf, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b);
void S3D_2D_TriangleFillAlpha(u8 * buf, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b, int a);

void S3D_2D_QuadAllignedFill(u8 * buf, int x1, int y1, int x2, int y2, int r, int g, int b);

//--------------------------------------------------------------------------------------------------

typedef enum {
	S3D_DOTS,
	S3D_LINES,
	S3D_TRIANGLES,
	S3D_QUADS,
	S3D_LINE_STRIP,
	S3D_TRIANGLE_STRIP,
	S3D_QUAD_STRIP,
	
	S3D_PRIMITIVE_NUMBER
} s3d_primitive;

void S3D_PolygonBegin(int screen, s3d_primitive type);

void S3D_PolygonVertex(int screen, s32 x, s32 y, s32 z);

// Stores a color for the following polygons and sets drawing color to that color. Anulates the current normal.
void S3D_PolygonColor(int screen, u32 r, u32 g, u32 b);
void S3D_PolygonColorAlpha(int screen, u32 r, u32 g, u32 b, u32 a);

// Sets the drawing color to current stored color affected by light.
void S3D_PolygonNormal(int screen, s32 x, s32 y, s32 z);

//--------------------------------------------------------------------------------------------------

void S3D_SetCulling(int screen, int draw_front, int draw_back);

//--------------------------------------------------------------------------------------------------

void S3D_LightAmbientColorSet(int screen, int r, int g, int b);

void S3D_LightDirectionalVectorSet(int screen, int index, s32 x, s32 y, s32 z);
void S3D_LightDirectionalColorSet(int screen, int index, int r, int g, int b);

#define S3D_LIGHT_N(i) (1<<(i))
void S3D_LightEnable(int screen, u32 enable_mask);

//--------------------------------------------------------------------------------------------------

#endif //__S3D_DRAW__

