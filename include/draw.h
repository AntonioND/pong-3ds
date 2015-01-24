
#ifndef __DRAW__
#define __DRAW__

#include <3ds.h>
#include <matrix.h>

void clear_screen_buffers(void);
void flush_screen_buffers(void);

//------------------------------------------------

void set_current_buffer(u8 * buf);
void set_current_color(u32 r, u32 g, u32 b);

typedef enum {
	P_DOTS,
	P_LINES,
	P_TRIANGLES,
	P_QUADS,
	P_PRIMITIVE_NUMBER
} e_primitive_type;

void set_primitive_type(e_primitive_type type);
void vertex(s32 x, s32 y, s32 z);

//------------------------------------------------

#endif //__DRAW__

