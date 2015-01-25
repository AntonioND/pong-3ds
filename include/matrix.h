
#ifndef __MATRIX__
#define __MATRIX__

#include "math.h"

//----------------------------------------------------------------------------

typedef s32 v4[4];

#define V4(v,r) (v[r])
#define ptr_V4(v,r) ((*v)[r])
#define V4SET(v,a0,a1,a2) { v[0]=(a0); v[1]=(a1); v[2]=(a2); }
#define ptr_V4SET(v,a0,a1,a2) { (*v)[0]=(a0); (*v)[1]=(a1); (*v)[2]=(a2); }

static inline s32 v4_dot_product(v4 * v1, v4 * v2)
{
	return fxmul(ptr_V4(v1,0),ptr_V4(v2,0)) + fxmul(ptr_V4(v1,1),ptr_V4(v2,1)) + 
           fxmul(ptr_V4(v1,2),ptr_V4(v2,2)) + fxmul(ptr_V4(v1,3),ptr_V4(v2,3));
}

void v4_cross_product(v4 * v1, v4 * v2, v4 * res);

//----------------------------------------------------------------------------

typedef s32 m44[16];

#define M44(m,r,c) (m[((r)<<2)+(c)])
#define ptr_M44(m,r,c) ((*m)[((r)<<2)+(c)])
#define M44SET(m, a00,a01,a02,a03, a10,a11,a12,a13, a20,a21,a22,a23, a30,a31,a32,a33) \
				{ 	m[0]=(a00); m[1]=(a01); m[2]=(a02); m[3]=(a03); \
					m[4]=(a10); m[5]=(a11); m[6]=(a12); m[7]=(a13); \
					m[8]=(a20); m[9]=(a21); m[10]=(a22); m[11]=(a23); \
					m[12]=(a30); m[13]=(a31); m[14]=(a32); m[15]=(a33);   }
#define ptr_M44SET(m, a00,a01,a02,a03, a10,a11,a12,a13, a20,a21,a22,a23, a30,a31,a32,a33) \
				{ 	(*m)[0]=(a00); (*m)[1]=(a01); (*m)[2]=(a02); (*m)[3]=(a03); \
					(*m)[4]=(a10); (*m)[5]=(a11); (*m)[6]=(a12); (*m)[7]=(a13); \
					(*m)[8]=(a20); (*m)[9]=(a21); (*m)[10]=(a22); (*m)[11]=(a23); \
					(*m)[12]=(a30); (*m)[13]=(a31); (*m)[14]=(a32); (*m)[15]=(a33);   }

void m44_copy(m44 * src, m44 * dest);

void m44_identity(m44 * m);
void m44_add(m44 * m1, m44 * m2, m44 * dest);
void m44_multiply(m44 * m1, m44 * m2, m44 * dest);

//----------------------------------------------------------------------------

void m44_v4_multiply(m44 * m, v4 * v, v4 * dest);

//----------------------------------------------------------------------------

void m44_create_ortho(m44 * m,u32 width, u32 height);
void m44_create_perspective(m44 * m,u32 width, u32 height, s32 znear);
void m44_create_frustum(m44 * m, int left, int right, int bottom, int top, int znear, int zfar);

//----------------------------------------------------------------------------

void m44_create_rotation_x(m44 * m, s32 angle);
void m44_create_rotation_y(m44 * m, s32 angle);
void m44_create_rotation_z(m44 * m, s32 angle);
void m44_create_rotation_axis(m44 * m, s32 angle, s32 x, s32 y, s32 z); //axis must be normalized

void m44_create_scale(m44 * m, s32 x, s32 y, s32 z);

void m44_create_translation(m44 * m, s32 x, s32 y, s32 z);

#endif //__MATRIX__