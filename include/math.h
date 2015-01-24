//BASED ON "tonc_math.h"

//
//  Mathematical functions
//
//! \file tonc_math.h
//! \author J Vijn
//! \date 20060508 - 20060908
//
// === NOTES ===


#ifndef TONC_MATH
#define TONC_MATH

#include <3ds.h>

typedef s32 FIXED;					//!< Fixed point type

// --------------------------------------------------------------------
//   FIXED POINT 
// --------------------------------------------------------------------

#define SIN_LUT_SIZE 514	// 512 for main lut, 2 extra for lerp

extern s16 sin_lut[514];	// .12f

static inline s32 lu_sin(u32 theta);
static inline s32 lu_cos(u32 theta);

//------------------------------------------------------------

//! Look-up a sine value (2&#960; = 0x10000)
/*! \param theta Angle in [0,FFFFh] range
*	 \return .12f sine value
*/
static inline s32 lu_sin(u32 theta)
{	return sin_lut[(theta>>7)&0x1FF];	}

//! Look-up a cosine value (2&#960; = 0x10000)
/*! \param theta Angle in [0,FFFFh] range
*	 \return .12f cosine value
*/
static inline s32 lu_cos(u32 theta)
{	return sin_lut[((theta>>7)+128)&0x1FF];	}

//--------------------------------------------------------------------

#define FIX_SHIFT       8
#define FIX_SCALE       ( 1<<FIX_SHIFT		)
#define FIX_MASK        ( FIX_SCALE-1		)
#define FIX_SCALEF      ( (float)FIX_SCALE	)
#define FIX_SCALEF_INV	( 1.0/FIX_SCALEF	)

#define FIX_ONE			FIX_SCALE

static inline FIXED int2fx(int d);
static inline FIXED float2fx(float f);
static inline u32 fx2uint(FIXED fx);	
static inline u32 fx2ufrac(FIXED fx);
static inline int fx2int(FIXED fx);
static inline float fx2float(FIXED fx);
static inline FIXED fxmul(FIXED fa, FIXED fb);
static inline FIXED fxdiv(FIXED fa, FIXED fb);
static inline FIXED fxmul64(FIXED fa, FIXED fb);
static inline FIXED fxdiv64(FIXED fa, FIXED fb);

// --- Fixed point ----------------------------------------------------

//! Convert an integer to fixed-point
static inline FIXED int2fx(int d)
{	return d<<FIX_SHIFT;	}

//! Convert a float to fixed-point
static inline FIXED float2fx(float f)
{	return (FIXED)(f*FIX_SCALEF);	}

//! Convert a FIXED point value to an unsigned integer (orly?).
static inline u32 fx2uint(FIXED fx)	
{	return fx>>FIX_SHIFT;	}

//! Get the unsigned fractional part of a fixed point value (orly?).
static inline u32 fx2ufrac(FIXED fx)
{	return fx&FIX_MASK;	}

//! Convert a FIXED point value to an signed integer.
static inline int fx2int(FIXED fx)
{	return fx/FIX_SCALE;	}

//! Convert a fixed point value to floating point.
static inline float fx2float(FIXED fx)
{	return fx/FIX_SCALEF;	}

//! Multiply two fixed point values
static inline FIXED fxmul(FIXED fa, FIXED fb)
{	return (fa*fb)>>FIX_SHIFT;				}

//! Divide two fixed point values.
static inline FIXED fxdiv(FIXED fa, FIXED fb)
{	return ((fa)*FIX_SCALE)/(fb);			}

//! Multiply two fixed point values using 64bit math.
static inline FIXED fxmul64(FIXED fa, FIXED fb)
{	return (((s64)fa)*fb)>>FIX_SHIFT;		}

//! Divide two fixed point values using 64bit math.
static inline FIXED fxdiv64(FIXED fa, FIXED fb)
{	return ( ((s64)fa)<<FIX_SHIFT)/(fb);	}


#endif // TONC_MATH

