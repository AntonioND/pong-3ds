
#ifndef __S3D_FXMATH__
#define __S3D_FXMATH__

//----------------------------------------------------------------------------

#include <3ds.h>

//----------------------------------------------------------------------------

//This fixed point part is based on "tonc_math.h"

#define SIN_LUT_SIZE 512 // Don't change this without changing the lut itself.

extern const s16 sin_lut[SIN_LUT_SIZE];	// .12f

#define FIX_SHIFT_EXTRA 0 // Positive (odd) values only!

#define FX_2PI (0x10000) // For fxsin() and fxcos()

// Look-up a sine value (2·PI = 0x10000)
// \param theta Angle in [0,FFFFh] range
// \return .12f sine value
static inline s32 fxsin(u32 theta) { return sin_lut[(theta>>7)&(SIN_LUT_SIZE-1)]<<FIX_SHIFT_EXTRA; }

// Look-up a cosine value (2·PI = 0x10000)
// \param theta Angle in [0,FFFFh] range
// \return .12f cosine value
static inline s32 fxcos(u32 theta) { return sin_lut[((theta>>7)+(SIN_LUT_SIZE/4))&(SIN_LUT_SIZE-1)]<<FIX_SHIFT_EXTRA; }

#define FIX_SHIFT       (12+FIX_SHIFT_EXTRA)  // Don't modify this, modify FIX_SHIFT_EXTRA
#define FIX_SCALE       ( 1<<FIX_SHIFT		)
#define FIX_SCALEF      ( (float)FIX_SCALE	)

// Convert an integer to fixed-point
static inline s32 int2fx(int d) { return d<<FIX_SHIFT; }

// Convert a float to fixed-point
static inline s32 float2fx(float f) { return (s32)(f*FIX_SCALEF); }

// Convert a FIXED point value to an unsigned integer (orly?).
static inline u32 fx2uint(s32 fx) { return fx>>FIX_SHIFT; }

// Convert a FIXED point value to an signed integer.
static inline int fx2int(s32 fx) { return fx/FIX_SCALE; }

// Convert a fixed point value to floating point.
static inline float fx2float(s32 fx) { return fx/FIX_SCALEF; }

// Multiply two fixed point values
static inline s32 fxmul(s32 fa, s32 fb) { return (fa*fb)>>FIX_SHIFT; }

// Divide two fixed point values.
static inline s32 fxdiv(s32 fa, s32 fb) { return ((fa)*FIX_SCALE)/(fb); }

// Multiply two fixed point values using 64bit math.
static inline s32 fxmul64(s32 fa, s32 fb) { return (((s64)fa)*fb)>>FIX_SHIFT; }

// Divide two fixed point values using 64bit math.
static inline s32 fxdiv64(s32 fa, s32 fb) { return ( ((s64)fa)<<FIX_SHIFT)/(fb); }

// Square root.
u32 fxsqrt(u32 fx);

// Square root using 64bit math.
u32 fxsqrt64(u32 fx);

//----------------------------------------------------------------------------

typedef s32 v4[4];

#define V4(v,r) (v[r])
#define ptr_V4(v,r) ((*v)[r])
#define V4SET(v,a0,a1,a2) { v[0]=(a0); v[1]=(a1); v[2]=(a2); }
#define ptr_V4SET(v,a0,a1,a2) { (*v)[0]=(a0); (*v)[1]=(a1); (*v)[2]=(a2); }

static inline s32 v4_DotProduct(v4 * v1, v4 * v2)
{
	return fxmul64(ptr_V4(v1,0),ptr_V4(v2,0)) + fxmul64(ptr_V4(v1,1),ptr_V4(v2,1)) + 
           fxmul64(ptr_V4(v1,2),ptr_V4(v2,2)) + fxmul64(ptr_V4(v1,3),ptr_V4(v2,3));
}

void v4_CrossProduct(v4 * v1, v4 * v2, v4 * res);

void v4_Normalize(v4 * v);

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

void m44_Copy(m44 * src, m44 * dest);

void m44_SetIdentity(m44 * m);
void m44_Add(m44 * m1, m44 * m2, m44 * dest);
void m44_Multiply(m44 * m1, m44 * m2, m44 * dest);

//----------------------------------------------------------------------------

void m44_v4_Multiply(m44 * m, v4 * v, v4 * dest);

//----------------------------------------------------------------------------

//void m44_CreateOrtho(m44 * m,u32 width, u32 height);
//void m44_CreatePerspective(m44 * m,u32 width, u32 height, s32 znear);
void m44_CreateFrustum(m44 * m, int left, int right, int bottom, int top, int znear, int zfar);

//----------------------------------------------------------------------------

void m44_CreateRotationX(m44 * m, s32 angle);
void m44_CreateRotationY(m44 * m, s32 angle);
void m44_CreateRotationZ(m44 * m, s32 angle);
void m44_CreateRotationAxis(m44 * m, s32 angle, s32 x, s32 y, s32 z); //axis must be normalized

void m44_CreateScale(m44 * m, s32 x, s32 y, s32 z);

void m44_CreateTranslation(m44 * m, s32 x, s32 y, s32 z);

//----------------------------------------------------------------------------

#endif //__S3D_FXMATH__