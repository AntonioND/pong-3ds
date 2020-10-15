// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#include <string.h>

#include "fxmath.h"

//--------------------------------------------------------------------------------------------------

u32 fxsqrt(u32 fx)
{
    register u32 root, remainder, place;

    root = 0;
    remainder = fx;
    place = 0x40000000;

    while (place > remainder)
        place = place >> 2;
    while (place)
    {
        if (remainder >= root + place)
        {
            remainder = remainder - root - place;
            root = root + (place << 1);
        }
        root = root >> 1;
        place = place >> 2;
    }

    return root<<(FIX_SHIFT/2);
}

u32 fxsqrt64(u32 fx)
{
    register u64 root, remainder, place;

    root = 0;
    remainder = ((u64)fx) << FIX_SHIFT;
    place = 0x4000000000000000ULL;

    while (place > remainder)
        place = place >> 2;
    while (place)
    {
        if (remainder >= root + place)
        {
            remainder = remainder - root - place;
            root = root + (place << 1);
        }
        root = root >> 1;
        place = place >> 2;
    }

    return root;
}

//--------------------------------------------------------------------------------------------------

void v4_CrossProduct(v4 * v1, v4 * v2, v4 * res)
{
    // |  i  j  k |
    // | 1x 1y 1z |
    // | 2x 2y 2z |

    ptr_V4SET(res,
        fxmul64(ptr_V4(v1,1),ptr_V4(v2,2)) - fxmul64(ptr_V4(v1,2),ptr_V4(v2,1)),
        -fxmul64(ptr_V4(v1,0),ptr_V4(v2,2)) + fxmul64(ptr_V4(v1,2),ptr_V4(v2,0)),
        fxmul64(ptr_V4(v1,0),ptr_V4(v2,1)) - fxmul64(ptr_V4(v1,1),ptr_V4(v2,0)) );
}

void v4_Normalize(v4 * v)
{
    int l = fxsqrt( fxmul(ptr_V4(v,0),ptr_V4(v,0)) + fxmul(ptr_V4(v,1),ptr_V4(v,1)) +
                    fxmul(ptr_V4(v,2),ptr_V4(v,2)) + fxmul(ptr_V4(v,3),ptr_V4(v,3)) );

    s32 inv_l = fxdiv(float2fx(1.0),l);

    ptr_V4(v,0) = fxmul(ptr_V4(v,0),inv_l);
    ptr_V4(v,1) = fxmul(ptr_V4(v,1),inv_l);
    ptr_V4(v,2) = fxmul(ptr_V4(v,2),inv_l);
    ptr_V4(v,3) = fxmul(ptr_V4(v,3),inv_l);
}

//--------------------------------------------------------------------------------------------------

void m44_Copy(m44 * src, m44 * dest)
{
    memcpy((void*)dest,(const void*)src,sizeof(m44));
}

void m44_SetIdentity(m44 * m)
{
    ptr_M44SET(m,    int2fx(1), int2fx(0), int2fx(0), int2fx(0),
                    int2fx(0), int2fx(1), int2fx(0), int2fx(0),
                    int2fx(0), int2fx(0), int2fx(1), int2fx(0),
                    int2fx(0), int2fx(0), int2fx(0), int2fx(1)  );
}

void m44_Add(m44 * m1, m44 * m2, m44 * dest)
{
    u32 i;
    for(i = 0; i < 16; i++)
        (*dest)[i] = (*m1)[i] + (*m2)[i];
}

void m44_Multiply(m44 * m1, m44 * m2, m44 * dest)
{
#if 1 // Just a bit of optimization
#define M44_MULTIPLY_ITERATION(row,col) \
{ \
    ptr_M44(dest,row,col) = \
        fxmul64( ptr_M44(m1,row,0) , ptr_M44(m2,0,col) ) + \
        fxmul64( ptr_M44(m1,row,1) , ptr_M44(m2,1,col) ) + \
        fxmul64( ptr_M44(m1,row,2) , ptr_M44(m2,2,col) ) + \
        fxmul64( ptr_M44(m1,row,3) , ptr_M44(m2,3,col) ); \
}
    M44_MULTIPLY_ITERATION(0,0);
    M44_MULTIPLY_ITERATION(0,1);
    M44_MULTIPLY_ITERATION(0,2);
    M44_MULTIPLY_ITERATION(0,3);

    M44_MULTIPLY_ITERATION(1,0);
    M44_MULTIPLY_ITERATION(1,1);
    M44_MULTIPLY_ITERATION(1,2);
    M44_MULTIPLY_ITERATION(1,3);

    M44_MULTIPLY_ITERATION(2,0);
    M44_MULTIPLY_ITERATION(2,1);
    M44_MULTIPLY_ITERATION(2,2);
    M44_MULTIPLY_ITERATION(2,3);

    M44_MULTIPLY_ITERATION(3,0);
    M44_MULTIPLY_ITERATION(3,1);
    M44_MULTIPLY_ITERATION(3,2);
    M44_MULTIPLY_ITERATION(3,3);
#else
    u32 row,col;
    for(col = 0; col < 4; col++) for(row = 0; row < 4; row++)
    {
        s32 sum = 0;
        u32 i;
        for(i = 0; i < 4; i++)
            sum += fxmul64( ptr_M44(m1,row,i) , ptr_M44(m2,i,col) );
        ptr_M44(dest,row,col) = sum;
    }
#endif
}

//--------------------------------------------------------------------------------------------------

void m44_v4_Multiply(m44 * m, v4 * v, v4 * dest)
{
#if 1 // Just a bit of optimization
#define M44_V4_MULTIPLY_ITERATION(row) \
{ \
    ptr_V4(dest,row) = \
        fxmul64( ptr_M44(m,row,0) , ptr_V4(v,0) ) + \
        fxmul64( ptr_M44(m,row,1) , ptr_V4(v,1) ) + \
        fxmul64( ptr_M44(m,row,2) , ptr_V4(v,2) ) + \
        fxmul64( ptr_M44(m,row,3) , ptr_V4(v,3) ); \
}
    M44_V4_MULTIPLY_ITERATION(0);
    M44_V4_MULTIPLY_ITERATION(1);
    M44_V4_MULTIPLY_ITERATION(2);
    M44_V4_MULTIPLY_ITERATION(3);
#else
    u32 row;
    for(row = 0; row < 4; row++)
    {
        s32 sum = 0;
        u32 i;
        for(i = 0; i < 4; i++)
            sum += fxmul64( ptr_M44(m,row,i) , ptr_V4(v,i) );
        ptr_V4(dest,row) = sum;
    }
#endif
}

//--------------------------------------------------------------------------------------------------
/*
void m44_CreateOrtho(m44 * m,u32 width, u32 height)
{
    m44 screen, ortho;
    M44SET(ortho,
     fxdiv(float2fx(2),width),                 int2fx(0),  int2fx(0), int2fx(0),
                    int2fx(0), fxdiv(float2fx(2),height),  int2fx(0), int2fx(0),
                    int2fx(0),                 int2fx(0),  int2fx(1), int2fx(0),
                    int2fx(0),                 int2fx(0),  int2fx(0), int2fx(1)  );
    M44SET(screen,
        int2fx(400)>>1,          0,         0,  int2fx(400)>>1,
               0, -int2fx(240)>>1,         0, int2fx(240)>>1,
               0,          0, int2fx(1),         0,
               0,          0,         0,  int2fx(1));

    m44_Multiply(&screen,&ortho,m);
}

void m44_CreatePerspective(m44 * m,u32 width, u32 height, s32 znear)
{
    m44 screen, perspective;

    M44SET(perspective,
     fxdiv(fxmul(float2fx(2),znear),width),                 int2fx(0),  int2fx(0), int2fx(0),
                    int2fx(0), fxdiv(fxmul(float2fx(2),znear),height),  int2fx(0), int2fx(0),
                    int2fx(0),                 int2fx(0),  int2fx(1), int2fx(0),
                    int2fx(0),                 int2fx(0),  int2fx(1), int2fx(0)  );
    M44SET(screen,
        int2fx(400)>>1,          0,        0, int2fx(400)>>1,
               0, -int2fx(240)>>1,         0, int2fx(240)>>1,
               0,          0, int2fx(1),         0,
               0,          0,         0,  int2fx(1));

    m44_Multiply(&screen,&perspective,m);
}
*/
void m44_CreateFrustum(m44 * m, int left, int right, int bottom, int top, int znear, int zfar)
{
    m44 screen, perspective;

    M44SET(perspective,
        -fxdiv64( 2*znear,right-left ), 0, fxdiv64(right+left,right-left), 0,
        0,  fxdiv64( 2*znear,top-bottom ),fxdiv64(top+bottom,top-bottom), 0,
        0, 0, fxdiv64(zfar+znear,zfar-znear), -fxdiv64(2*fxmul64(zfar,znear),zfar-znear),
        0, 0, -int2fx(1),0 );

    M44SET(screen,
        int2fx(400)>>1,          0,        0, int2fx(400)>>1,
               0, -int2fx(240)>>1,         0, int2fx(240)>>1,
               0,          0, int2fx(1),         0,
               0,          0,         0,  int2fx(1));

    m44_Multiply(&screen,&perspective,m);

}
//--------------------------------------------------------------------------------------------------

void m44_CreateRotationX(m44 * m, s32 angle)
{
    s32 sin = fxsin(angle);
    s32 cos = fxcos(angle);

    ptr_M44SET(m,
        int2fx(1), int2fx(0), int2fx(0), int2fx(0),
        int2fx(0),       cos,      -sin, int2fx(0),
        int2fx(0),       sin,       cos, int2fx(0),
        int2fx(0), int2fx(0), int2fx(0), int2fx(1)  );
}

void m44_CreateRotationY(m44 * m, s32 angle)
{
    s32 sin = fxsin(angle);
    s32 cos = fxcos(angle);

    ptr_M44SET(m,
              cos, int2fx(0),       -sin, int2fx(0),
        int2fx(0), int2fx(1),  int2fx(0), int2fx(0),
              sin, int2fx(0),        cos, int2fx(0),
        int2fx(0), int2fx(0),  int2fx(0), int2fx(1)  );
}

void m44_CreateRotationZ(m44 * m, s32 angle)
{
    s32 sin = fxsin(angle);
    s32 cos = fxcos(angle);

    ptr_M44SET(m,
              cos,      -sin, int2fx(0), int2fx(0),
              sin,       cos, int2fx(0), int2fx(0),
        int2fx(0), int2fx(0), int2fx(1), int2fx(0),
        int2fx(0), int2fx(0), int2fx(0), int2fx(1)  );
}

void m44_CreateRotationAxis(m44 * m, s32 angle, s32 x, s32 y, s32 z)
{
    s32 sin = fxsin(angle);
    s32 cos = fxcos(angle);
    s32 one_minus_cos = int2fx(1) - cos;
    s32 x_sin = fxmul(x,sin);
    s32 y_sin = fxmul(y,sin);
    s32 z_sin = fxmul(z,sin);
    s32 x_y_one_minus_cos = fxmul(fxmul(x,y),one_minus_cos);
    s32 x_z_one_minus_cos = fxmul(fxmul(x,z),one_minus_cos);
    s32 y_z_one_minus_cos = fxmul(fxmul(y,z),one_minus_cos);

    ptr_M44SET(m,
        cos+fxmul(fxmul(x,x),one_minus_cos),
        x_y_one_minus_cos - z_sin,
        x_z_one_minus_cos + y_sin,
        int2fx(0),

        x_y_one_minus_cos + z_sin,
        cos+fxmul(fxmul(y,y),one_minus_cos),
        y_z_one_minus_cos - x_sin,
        int2fx(0),

        x_z_one_minus_cos - y_sin,
        y_z_one_minus_cos + x_sin,
        cos+fxmul(fxmul(z,z),one_minus_cos),
        int2fx(0),

        int2fx(0),
        int2fx(0),
        int2fx(0),
        int2fx(1)  );
}

void m44_CreateScale(m44 * m, s32 x, s32 y, s32 z)
{
    ptr_M44SET(m,
                x, int2fx(0), int2fx(0), int2fx(0),
        int2fx(0),         y, int2fx(0), int2fx(0),
        int2fx(0), int2fx(0),         z, int2fx(0),
        int2fx(0), int2fx(0), int2fx(0), int2fx(1)  );
}

void m44_CreateTranslation(m44 * m, s32 x, s32 y, s32 z)
{
    ptr_M44SET(m,
        int2fx(1), int2fx(0), int2fx(0),         x,
        int2fx(0), int2fx(1), int2fx(0),         y,
        int2fx(0), int2fx(0), int2fx(1),         z,
        int2fx(0), int2fx(0), int2fx(0), int2fx(1)  );
}

//--------------------------------------------------------------------------------------------------

// A 512 long LUT of 16bit values in 4.12 format. sin(x*pi/256)

// From Tonc's library for GBA,
// Exported by Cearn's excellut v1.0
// (comments, kudos, flames to daytshen@hotmail.com)

const s16 sin_lut[SIN_LUT_SIZE] = { // .12f
    0x0000,0x0032,0x0064,0x0096,0x00C8,0x00FB,0x012D,0x015F,
    0x0191,0x01C3,0x01F5,0x0227,0x0259,0x028A,0x02BC,0x02ED,
    0x031F,0x0350,0x0381,0x03B2,0x03E3,0x0413,0x0444,0x0474,
    0x04A5,0x04D5,0x0504,0x0534,0x0563,0x0593,0x05C2,0x05F0,
    0x061F,0x064D,0x067B,0x06A9,0x06D7,0x0704,0x0731,0x075E,
    0x078A,0x07B7,0x07E2,0x080E,0x0839,0x0864,0x088F,0x08B9,
    0x08E3,0x090D,0x0936,0x095F,0x0987,0x09B0,0x09D7,0x09FF,
    0x0A26,0x0A4D,0x0A73,0x0A99,0x0ABE,0x0AE3,0x0B08,0x0B2C,

    0x0B50,0x0B73,0x0B96,0x0BB8,0x0BDA,0x0BFC,0x0C1D,0x0C3E,
    0x0C5E,0x0C7D,0x0C9D,0x0CBB,0x0CD9,0x0CF7,0x0D14,0x0D31,
    0x0D4D,0x0D69,0x0D84,0x0D9F,0x0DB9,0x0DD2,0x0DEB,0x0E04,
    0x0E1C,0x0E33,0x0E4A,0x0E60,0x0E76,0x0E8B,0x0EA0,0x0EB4,
    0x0EC8,0x0EDB,0x0EED,0x0EFF,0x0F10,0x0F21,0x0F31,0x0F40,
    0x0F4F,0x0F5D,0x0F6B,0x0F78,0x0F85,0x0F91,0x0F9C,0x0FA7,
    0x0FB1,0x0FBA,0x0FC3,0x0FCB,0x0FD3,0x0FDA,0x0FE1,0x0FE7,
    0x0FEC,0x0FF0,0x0FF4,0x0FF8,0x0FFB,0x0FFD,0x0FFE,0x0FFF,

    0x1000,0x0FFF,0x0FFE,0x0FFD,0x0FFB,0x0FF8,0x0FF4,0x0FF0,
    0x0FEC,0x0FE7,0x0FE1,0x0FDA,0x0FD3,0x0FCB,0x0FC3,0x0FBA,
    0x0FB1,0x0FA7,0x0F9C,0x0F91,0x0F85,0x0F78,0x0F6B,0x0F5D,
    0x0F4F,0x0F40,0x0F31,0x0F21,0x0F10,0x0EFF,0x0EED,0x0EDB,
    0x0EC8,0x0EB4,0x0EA0,0x0E8B,0x0E76,0x0E60,0x0E4A,0x0E33,
    0x0E1C,0x0E04,0x0DEB,0x0DD2,0x0DB9,0x0D9F,0x0D84,0x0D69,
    0x0D4D,0x0D31,0x0D14,0x0CF7,0x0CD9,0x0CBB,0x0C9D,0x0C7D,
    0x0C5E,0x0C3E,0x0C1D,0x0BFC,0x0BDA,0x0BB8,0x0B96,0x0B73,

    0x0B50,0x0B2C,0x0B08,0x0AE3,0x0ABE,0x0A99,0x0A73,0x0A4D,
    0x0A26,0x09FF,0x09D7,0x09B0,0x0987,0x095F,0x0936,0x090D,
    0x08E3,0x08B9,0x088F,0x0864,0x0839,0x080E,0x07E2,0x07B7,
    0x078A,0x075E,0x0731,0x0704,0x06D7,0x06A9,0x067B,0x064D,
    0x061F,0x05F0,0x05C2,0x0593,0x0563,0x0534,0x0504,0x04D5,
    0x04A5,0x0474,0x0444,0x0413,0x03E3,0x03B2,0x0381,0x0350,
    0x031F,0x02ED,0x02BC,0x028A,0x0259,0x0227,0x01F5,0x01C3,
    0x0191,0x015F,0x012D,0x00FB,0x00C8,0x0096,0x0064,0x0032,

    0x0000,0xFFCE,0xFF9C,0xFF6A,0xFF38,0xFF05,0xFED3,0xFEA1,
    0xFE6F,0xFE3D,0xFE0B,0xFDD9,0xFDA7,0xFD76,0xFD44,0xFD13,
    0xFCE1,0xFCB0,0xFC7F,0xFC4E,0xFC1D,0xFBED,0xFBBC,0xFB8C,
    0xFB5B,0xFB2B,0xFAFC,0xFACC,0xFA9D,0xFA6D,0xFA3E,0xFA10,
    0xF9E1,0xF9B3,0xF985,0xF957,0xF929,0xF8FC,0xF8CF,0xF8A2,
    0xF876,0xF849,0xF81E,0xF7F2,0xF7C7,0xF79C,0xF771,0xF747,
    0xF71D,0xF6F3,0xF6CA,0xF6A1,0xF679,0xF650,0xF629,0xF601,
    0xF5DA,0xF5B3,0xF58D,0xF567,0xF542,0xF51D,0xF4F8,0xF4D4,

    0xF4B0,0xF48D,0xF46A,0xF448,0xF426,0xF404,0xF3E3,0xF3C2,
    0xF3A2,0xF383,0xF363,0xF345,0xF327,0xF309,0xF2EC,0xF2CF,
    0xF2B3,0xF297,0xF27C,0xF261,0xF247,0xF22E,0xF215,0xF1FC,
    0xF1E4,0xF1CD,0xF1B6,0xF1A0,0xF18A,0xF175,0xF160,0xF14C,
    0xF138,0xF125,0xF113,0xF101,0xF0F0,0xF0DF,0xF0CF,0xF0C0,
    0xF0B1,0xF0A3,0xF095,0xF088,0xF07B,0xF06F,0xF064,0xF059,
    0xF04F,0xF046,0xF03D,0xF035,0xF02D,0xF026,0xF01F,0xF019,
    0xF014,0xF010,0xF00C,0xF008,0xF005,0xF003,0xF002,0xF001,

    0xF000,0xF001,0xF002,0xF003,0xF005,0xF008,0xF00C,0xF010,
    0xF014,0xF019,0xF01F,0xF026,0xF02D,0xF035,0xF03D,0xF046,
    0xF04F,0xF059,0xF064,0xF06F,0xF07B,0xF088,0xF095,0xF0A3,
    0xF0B1,0xF0C0,0xF0CF,0xF0DF,0xF0F0,0xF101,0xF113,0xF125,
    0xF138,0xF14C,0xF160,0xF175,0xF18A,0xF1A0,0xF1B6,0xF1CD,
    0xF1E4,0xF1FC,0xF215,0xF22E,0xF247,0xF261,0xF27C,0xF297,
    0xF2B3,0xF2CF,0xF2EC,0xF309,0xF327,0xF345,0xF363,0xF383,
    0xF3A2,0xF3C2,0xF3E3,0xF404,0xF426,0xF448,0xF46A,0xF48D,

    0xF4B0,0xF4D4,0xF4F8,0xF51D,0xF542,0xF567,0xF58D,0xF5B3,
    0xF5DA,0xF601,0xF629,0xF650,0xF679,0xF6A1,0xF6CA,0xF6F3,
    0xF71D,0xF747,0xF771,0xF79C,0xF7C7,0xF7F2,0xF81E,0xF849,
    0xF876,0xF8A2,0xF8CF,0xF8FC,0xF929,0xF957,0xF985,0xF9B3,
    0xF9E1,0xFA10,0xFA3E,0xFA6D,0xFA9D,0xFACC,0xFAFC,0xFB2B,
    0xFB5B,0xFB8C,0xFBBC,0xFBED,0xFC1D,0xFC4E,0xFC7F,0xFCB0,
    0xFCE1,0xFD13,0xFD44,0xFD76,0xFDA7,0xFDD9,0xFE0B,0xFE3D,
    0xFE6F,0xFEA1,0xFED3,0xFF05,0xFF38,0xFF6A,0xFF9C,0xFFCE
};

//--------------------------------------------------------------------------------------------------
