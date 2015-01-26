
#ifndef __DRAW__
#define __DRAW__

//----------------------------------------------------------------------------------

#include <3ds.h>

//----------------------------------------------------------------------------------

// in utils_asm.s
void S3D_FramebufferFill(void * dst, u32 color, u32 bottom_screen); // color = 31 _BGR 0

void S3D_FramebuffersClearTopScreen(int r, int g, int b);

void S3D_BufferSetScreen(int right);

//--------------------------------------------------------------------------------------------------

void S3D_2D_Line(u8 * buf, int x1, int y1, int x2, int y2, int r, int g, int b);

void S3D_2D_LineEx(u8 * buf, int thickness, int x1, int y1, int x2, int y2, int r, int g, int b);

void S3D_2D_TriangleFill(u8 * buf, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b);

//--------------------------------------------------------------------------------------------------

typedef enum {
	S3D_LINES,
	S3D_TRIANGLES,
	S3D_QUADS,
	S3D_LINE_STRIP,
	S3D_TRIANGLE_STRIP,
	S3D_QUAD_STRIP,
	
	S3D_PRIMITIVE_NUMBER
} s3d_primitive;

void S3D_PolygonBegin(s3d_primitive type);

void S3D_PolygonVertex(s32 x, s32 y, s32 z);

// Stores a color for the following polygons and sets drawing color to that color. Anulates the current normal.
void S3D_PolygonColor(u32 r, u32 g, u32 b); 

// Sets the drawing color to current stored color affected by light.
void S3D_PolygonNormal(s32 x, s32 y, s32 z);

//--------------------------------------------------------------------------------------------------

void S3D_SetCulling(int draw_front, int draw_back);

//--------------------------------------------------------------------------------------------------

void S3D_LightAmbientColorSet(int r, int g, int b);

void S3D_LightDirectionalVectorSet(int index, s32 x, s32 y, s32 z);
void S3D_LightDirectionalColorSet(int index, int r, int g, int b);

#define S3D_LIGHT_N(i) (1<<(i))
void S3D_LightEnable(u32 enable_mask);

//--------------------------------------------------------------------------------------------------

#endif //__DRAW__

