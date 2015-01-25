
#ifndef __DRAW__
#define __DRAW__

#include <3ds.h>
#include <matrix.h>

void clear_screen_buffers(void);
void flush_screen_buffers(void);

//------------------------------------------------

void set_current_buffer(int right);
void set_current_color(u32 r, u32 g, u32 b);

typedef enum {
	P_DOTS,
	P_LINES,
	P_TRIANGLES,
	P_QUADS,
	P_TRIANGLE_STRIP,
	P_QUAD_STRIP,
	P_PRIMITIVE_NUMBER
} e_primitive_type;

void polygon_begin(e_primitive_type type);
void polygon_vertex(s32 x, s32 y, s32 z);

//------------------------------------------------

void polygon_normal(s32 x, s32 y, s32 z);

void light_set_dir(int index, s32 x, s32 y, s32 z);
void light_set_color(int index, int r, int g, int b);
#define LIGHT_N(i) (1<<(i))
void lights_enable(u32 enable_mask);

//------------------------------------------------

#endif //__DRAW__

