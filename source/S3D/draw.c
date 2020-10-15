// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#include <3ds.h>

#include "draw.h"
#include "fxmath.h"

static inline int abs(int x)
{
    return x < 0 ? -x : x;
}

static inline int min(int a, int b)
{
    return a < b ? a : b;
}

static inline int max(int a, int b)
{
    return a > b ? a : b;
}

static inline int sgn(int x)
{
    if (x > 0)
        return 1;
    if (x < 0)
        return -1;
    return 0;
}

static inline void swapints(int *a1, int *a2)
{
    int t = *a1;
    *a1 = *a2;
    *a2 = t;
}

static inline void swapints_pairs(int *a1, int *a2, int *b1, int *b2)
{
    int t = *a1;
    *a1 = *a2;
    *a2 = t;
    t = *b1;
    *b1 = *b2;
    *b2 = t;
}

static inline void roll_values(int *a1, int *a2, int *a3, int *a4,
                               int *b1, int *b2, int *b3, int *b4)
{
    int t = *a1;
    *a1 = *a2;
    *a2 = *a3;
    *a3 = *a4;
    *a4 = t;
    t = *b1;
    *b1 = *b2;
    *b2 = *b3;
    *b3 = *b4;
    *b4 = t;
}

// -----------------------------------------------------------------------------

static inline void _s3d_plot_unsafe(u8 *buf, u32 x, u32 y, int r, int g, int b)
{
    u8 *p = &(buf[(240 * x + y) * 3]);
    *p++ = b;
    *p++ = g;
    *p = r;
}

static inline void _s3d_plot_unsafe_alpha(u8 *buf, u32 x, u32 y,
                                          int r, int g, int b, int a)
{
    u8 *p = &(buf[(240 * x + y) * 3]);
    if (a == 255)
    {
        *p++ = b;
        *p++ = g;
        *p = r;
        return;
    }
    int one_minus_alpha = 255 - a;
    p[0] = ((p[0] * one_minus_alpha) + b * a) / 256;
    p[1] = ((p[1] * one_minus_alpha) + g * a) / 256;
    p[2] = ((p[2] * one_minus_alpha) + r * a) / 256;
}

static inline void _s3d_plot_safe(u8 *buf, u32 x, u32 y, int r, int g, int b)
{
    if ((x >= 400) || (y >= 240))
        return;
    u8 *p = &(buf[(240 * x + y) * 3]);
    *p++ = b;
    *p++ = g;
    *p = r;
}

static inline void _s3d_plot_safe_alpha(u8 *buf, u32 x, u32 y,
                                        int r, int g, int b, int a)
{
    if ((x >= 400) || (y >= 240))
        return;

    u8 *p = &(buf[(240 * x + y) * 3]);
    int one_minus_alpha = 255 - a;
    p[0] = ((p[0] * one_minus_alpha) + b * a) / 256;
    p[1] = ((p[1] * one_minus_alpha) + g * a) / 256;
    p[2] = ((p[2] * one_minus_alpha) + r * a) / 256;
}

// No checks are done in the bounds of X, so it is unsafe
static inline void _s3d_vertical_line(u8 *linebuf, u32 x, int y1, int y2,
                                      int r, int g, int b)
{
    if (y1 < y2)
    {
        if (y2 < 0)
            return;
        else if (y2 >= 240)
            y2 = 240 - 1;

        if (y1 >= 240)
            return;
        else if (y1 < 0)
            y1 = 0;

        linebuf += y1 * 3;
        for (; y1 <= y2; y1++)
        {
            *linebuf++ = b;
            *linebuf++ = g;
            *linebuf++ = r;
        }
    }
    else // y2 < y1
    {
        if (y1 < 0)
            return;
        else if (y1 >= 240)
            y1 = 240 - 1;

        if (y2 >= 240)
            return;
        else if (y2 < 0)
            y2 = 0;

        linebuf += y2 * 3;
        for (; y2 <= y1; y2++)
        {
            *linebuf++ = b;
            *linebuf++ = g;
            *linebuf++ = r;
        }
    }
}

// No checks are done in the bounds of X, so it is unsafe
static inline void _s3d_vertical_line_alpha(u8 *linebuf, u32 x, int y1, int y2,
                                            int r, int g, int b, int a)
{
    if (y1 < y2)
    {
        if (y2 < 0)
            return;
        else if (y2 >= 240)
            y2 = 240 - 1;

        if (y1 >= 240)
            return;
        else if (y1 < 0)
            y1 = 0;

        int one_minus_alpha = 255 - a;

        linebuf += y1 * 3;
        for (; y1 <= y2; y1++)
        {
            linebuf[0] = ((linebuf[0] * one_minus_alpha) + b * a) / 256;
            linebuf[1] = ((linebuf[1] * one_minus_alpha) + g * a) / 256;
            linebuf[2] = ((linebuf[2] * one_minus_alpha) + r * a) / 256;
            linebuf += 3;
        }
    }
    else // y2 < y1
    {
        if (y1 < 0)
            return;
        else if (y1 >= 240)
            y1 = 240 - 1;

        if (y2 >= 240)
            return;
        else if (y2 < 0)
            y2 = 0;

        int one_minus_alpha = 255 - a;

        linebuf += y2 * 3;
        for (; y2 <= y1; y2++)
        {
            linebuf[0] = ((linebuf[0] * one_minus_alpha) + b * a) / 256;
            linebuf[1] = ((linebuf[1] * one_minus_alpha) + g * a) / 256;
            linebuf[2] = ((linebuf[2] * one_minus_alpha) + r * a) / 256;
            linebuf += 3;
        }
    }
}

// No checks are done in the bounds of X, so it is unsafe
// y1 must be lower than y2
static inline void _s3d_vertical_line_downwards(u8 *linebuf,
                                                u32 x, int y1, int y2,
                                                int r, int g, int b)
{
    if (y2 < 0)
        return;
    else if (y2 >= 240)
        y2 = 240 - 1;

    if (y1 >= 240)
        return;
    else if (y1 < 0)
        y1 = 0;

    linebuf += y1 * 3;
    for (; y1 <= y2; y1++)
    {
        *linebuf++ = b;
        *linebuf++ = g;
        *linebuf++ = r;
    }
}

// No checks are done in the bounds of X, so it is unsafe
// y1 must be lower than y2
static inline void _s3d_vertical_line_downwards_alpha(u8 *linebuf,
                                                      u32 x, int y1, int y2,
                                                      int r, int g, int b, int a)
{
    if (y2 < 0)
        return;
    else if (y2 >= 240)
        y2 = 240 - 1;

    if (y1 >= 240)
        return;
    else if (y1 < 0)
        y1 = 0;

    int one_minus_alpha = 255 - a;

    linebuf += y1 * 3;
    for (; y1 <= y2; y1++)
    {
        linebuf[0] = ((linebuf[0] * one_minus_alpha) + b * a) / 256;
        linebuf[1] = ((linebuf[1] * one_minus_alpha) + g * a) / 256;
        linebuf[2] = ((linebuf[2] * one_minus_alpha) + r * a) / 256;
        linebuf += 3;
    }
}

void _s3d_line_unsafe_alpha(u8 *buf, int x1, int y1, int x2, int y2,
                            int r, int g, int b, int a)
{
    int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;

    dx = x2 - x1; // Horizontal distance
    dy = y2 - y1; // Vertical distance
    dxabs = abs(dx);
    dyabs = abs(dy);
    sdx = sgn(dx);
    sdy = sgn(dy);
    x = dyabs >> 1;
    y = dxabs >> 1;
    px = x1;
    py = y1;

    _s3d_plot_unsafe_alpha(buf, px, py, r, g, b, a);

    if (dxabs >= dyabs) // The line is more horizontal than vertical
    {
        for (i = 0; i < dxabs; i++)
        {
            y += dyabs;
            if (y >= dxabs)
            {
                y -= dxabs;
                py += sdy;
            }
            px += sdx;
            _s3d_plot_unsafe_alpha(buf, px, py, r, g, b, a);
        }
    }
    else // The line is more vertical than horizontal
    {
        for (i = 0; i < dyabs; i++)
        {
            x += dxabs;
            if (x >= dyabs)
            {
                x -= dyabs;
                px += sdx;
            }
            py += sdy;
            _s3d_plot_unsafe_alpha(buf, px, py, r, g, b, a);
        }
    }
}

// -----------------------------------------------------------------------------

// http://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland

#define RIGHT (8)
#define TOP (4)
#define LEFT (2)
#define BOTTOM (1)

// Compute the bit code for a point (x, y) using the clip rectangle bounded
// diagonally by (xmin, ymin), and (xmax, ymax)
static inline int _s3d_compute_out_code(int32_t x, int32_t y)
{
    int code = 0;

    if (y >= int2fx(240))
        code |= TOP; // Above the clip window
    else if (y < int2fx(0))
        code |= BOTTOM; // Below the clip window

    if (x >= int2fx(400))
        code |= RIGHT; // To the right of clip window
    else if (x < int2fx(0))
        code |= LEFT; // To the left of clip window

    return code;
}

// Cohen-Sutherland clipping algorithm clips a line from
// P1 = (x1, y1) to P2 = (x2, y2) against a rectangle with
// diagonal from (xmin, ymin) to (xmax, ymax).
void S3D_2D_Line(u8 *buf, int x1, int y1, int x2, int y2,
                 int r, int g, int b, int a)
{
    // Outcodes for P1, P1, and whatever point lies outside the clip rectangle
    int outcode1, outcode2, outcodeOut;

    s32 fx1 = int2fx(x1);
    s32 fy1 = int2fx(y1);
    s32 fx2 = int2fx(x2);
    s32 fy2 = int2fx(y2);

    // Compute outcodes
    outcode1 = _s3d_compute_out_code(fx1, fy1);
    outcode2 = _s3d_compute_out_code(fx2, fy2);

    while (1)
    {
        if ((outcode1 | outcode2) == 0)
        {
            // Logical or is 0. Trivially accept and get out of loop
            _s3d_line_unsafe_alpha(buf, fx2int(fx1), fx2int(fy1),
                                   fx2int(fx2), fx2int(fy2), r, g, b, a);
            return;
        }
        else if (outcode1 & outcode2)
        {
            // Logical and is not 0. Trivially reject and exit
            return;
        }
        else
        {
            // Failed both tests, so calculate the line segment to clip from an
            // outside point to an intersection with clip edge
            s32 x, y;

            // At least one endpoint is outside the clip rectangle; pick it.
            outcodeOut = outcode1 ? outcode1 : outcode2;

            // Now find the intersection point;
            // use formulas:
            //     y = y0 + slope * (x - x0)
            //     x = x0 + (1/slope)* (y - y0)

            if (outcodeOut & TOP)
            {
                // Point is above the clip rectangle
                x = fx1 + fxdiv64(fxmul64(fx2 - fx1, int2fx(240 - 1) - fy1),
                                  fy2 - fy1);
                y = int2fx(240 - 1);
            }
            else if (outcodeOut & BOTTOM)
            {
                // Point is below the clip rectangle
                x = fx1 + fxdiv64(fxmul64(fx2 - fx1, int2fx(0) - fy1),
                                  fy2 - fy1);
                y = int2fx(0);
            }
            else if (outcodeOut & RIGHT)
            {
                // Point is to the right of clip rectangle
                y = fy1 + fxdiv64(fxmul64(fy2 - fy1, int2fx(400 - 1) - fx1),
                                  fx2 - fx1);
                x = int2fx(400 - 1);
            }
            else // if(outcodeOut & LEFT)
            {
                // Point is to the left of clip rectangle
                y = fy1 + fxdiv64(fxmul64(fy2 - fy1, int2fx(0) - fx1),
                                  fx2 - fx1);
                x = int2fx(0);
            }

            // Now we move outside point to intersection point to clip and get
            // ready for next pass.
            if (outcodeOut == outcode1)
            {
                fx1 = x;
                fy1 = y;
                outcode1 = _s3d_compute_out_code(fx1, fy1);
            }
            else
            {
                fx2 = x;
                fy2 = y;
                outcode2 = _s3d_compute_out_code(fx2, fy2);
            }
        }
    }
}

// -----------------------------------------------------------------------------

void S3D_2D_Plot(u8 *buf, int x, int y, int r, int g, int b, int a)
{
    if (a == 255)
        _s3d_plot_safe(buf, x, y, r, g, b);
    else
        _s3d_plot_safe_alpha(buf, x, y, r, g, b, a);
}

void S3D_2D_PlotEx(u8 *buf, int thickness, int x, int y,
                   int r, int g, int b, int a)
{
    thickness--;

    int i, j;
    int hw = thickness / 2;

    for (i = -hw; i <= -hw + thickness; i++)
    {
        for (j = -hw; j <= -hw + thickness; j++)
        {
            S3D_2D_Plot(buf, x + i, y + j, r, g, b, a);
        }
    }
}

// -----------------------------------------------------------------------------

void S3D_2D_LineEx(u8 *buf, int thickness, int x1, int y1, int x2, int y2,
                   int r, int g, int b, int a)
{
    thickness--;

    int i, j;
    int hw = thickness / 2;

    for (i = -hw; i <= -hw + thickness; i++)
    {
        for (j = -hw; j <= -hw + thickness; j++)
        {
            S3D_2D_Line(buf, x1 + i, y1 + j, x2 + i, y2 + j, r, g, b, a);
        }
    }
}

// -----------------------------------------------------------------------------

void S3D_2D_TriangleFill(u8 *buf, int x1, int y1, int x2, int y2,
                         int x3, int y3, int r, int g, int b)
{
    y1 = int2fx(y1);
    y2 = int2fx(y2);
    y3 = int2fx(y3);

    // Sort: x1 <= x2 <= x3
    if (x1 > x2)
    {
        swapints_pairs(&x1, &x2, &y1, &y2);
    }
    if (x2 > x3)
    {
        swapints_pairs(&x2, &x3, &y2, &y3);
        if (x1 > x2)
        {
            swapints_pairs(&x1, &x2, &y1, &y2);
        }
    }

    u8 *linebuf = &(buf[240 * x1 * 3]);

    if (x1 != x2) // x1 != x2
    {
        int dy1 = fxdiv64(y2 - y1, int2fx(x2 - x1));

        //if (x1 != x3) // x1 != x2, x1 != x3
        {
            int dy2 = fxdiv64(y3 - y1, int2fx(x3 - x1));

            if (x2 != x3) // x1 != x2, x1 != x3, x2 != x3 -> Most common case
            {
                int dy3 = fxdiv64(y3 - y2, int2fx(x3 - x2));

                int sx = x1;
                int sy = y1;
                int ey = y1;

                if (dy1 > dy2)
                {
                    for (; sx < x2; sx++, sy += dy2, ey += dy1, linebuf += 240 * 3)
                    {
                        if ((u32)sx < 400)
                        {
                            _s3d_vertical_line_downwards(linebuf, sx,
                                                         fx2int(sy), fx2int(ey),
                                                         r, g, b);
                        }
                    }

                    ey = y2;
                    for (; sx < x3; sx++, sy += dy2, ey += dy3, linebuf += 240 * 3)
                    {
                        if ((u32)sx < 400)
                        {
                            _s3d_vertical_line_downwards(linebuf, sx,
                                                         fx2int(sy), fx2int(ey),
                                                         r, g, b);
                        }
                    }

                    _s3d_plot_safe(buf, x3, fx2int(y3), r, g, b);
                }
                else // if (dy1 <= dy2)
                {
                    for (; sx < x2; sx++, sy += dy2, ey += dy1, linebuf += 240 * 3)
                    {
                        if ((u32)sx < 400)
                        {
                            _s3d_vertical_line_downwards(linebuf, sx,
                                                         fx2int(ey), fx2int(sy),
                                                         r, g, b);
                        }
                    }

                    ey = y2;
                    for (; sx < x3; sx++, sy += dy2, ey += dy3, linebuf += 240 * 3)
                    {
                        if ((u32)sx < 400)
                        {
                            _s3d_vertical_line_downwards(linebuf, sx,
                                                         fx2int(ey), fx2int(sy),
                                                         r, g, b);
                        }
                    }

                    _s3d_plot_safe(buf, x3, fx2int(y3), r, g, b);
                }

                return;
            }
            else // x1 != x2, x1 != x3, x2 == x3
            {
                int sx = x1;
                int sy = y1;
                int ey = y1;

                if (dy1 > dy2)
                {
                    for (; sx < x2; sx++, sy += dy2, ey += dy1, linebuf += 240 * 3)
                    {
                        if ((u32)sx < 400)
                        {
                            _s3d_vertical_line_downwards(linebuf, sx,
                                                         fx2int(sy), fx2int(ey),
                                                         r, g, b);
                        }
                    }

                    if (sx >= 0)
                    {
                        _s3d_vertical_line_downwards(linebuf, sx,
                                                     fx2int(y3), fx2int(y2),
                                                     r, g, b);
                    }
                }
                else //if(dy1 <= dy2)
                {
                    for (; sx < x2; sx++, sy += dy2, ey += dy1, linebuf += 240 * 3)
                    {
                        if ((u32)sx < 400)
                        {
                            _s3d_vertical_line_downwards(linebuf, sx,
                                                         fx2int(ey), fx2int(sy),
                                                         r, g, b);
                        }
                    }

                    if (sx >= 0)
                    {
                        _s3d_vertical_line_downwards(linebuf, sx,
                                                     fx2int(y2), fx2int(y3),
                                                     r, g, b);
                    }
                }

                return;
            }
        }
        //else // x1 != x2, x1 == x3
        //{
        //    // Impossible because x1 <= x2 <= x3
        //    printf("C");
        //    return;
        //}
    }
    else // x1 == x2
    {
        if (x1 != x3) // x1 == x2, x1 != x3
        {
            int dy2 = fxdiv64(y3 - y1, int2fx(x3 - x1));

            // if(x2 != x3)  // x1 == x2, x1 != x3, x2 != x3
            {
                int dy3 = fxdiv64(y3 - y2, int2fx(x3 - x2));

                int sx = x1;
                int sy = y1;
                int ey = y2;

                if (y1 > y2)
                {
                    for (; sx < x3; sx++, sy += dy2, ey += dy3, linebuf += 240 * 3)
                    {
                        if ((u32)sx < 400)
                        {
                            _s3d_vertical_line_downwards(linebuf, sx,
                                                         fx2int(ey), fx2int(sy),
                                                         r, g, b);
                        }
                    }

                    _s3d_plot_safe(buf, x3, fx2int(y3), r, g, b);
                }
                else // if (dy1 <= dy2)
                {
                    for (; sx < x3; sx++, sy += dy2, ey += dy3, linebuf += 240 * 3)
                    {
                        if ((u32)sx < 400)
                        {
                            _s3d_vertical_line_downwards(linebuf, sx,
                                                         fx2int(sy), fx2int(ey),
                                                         r, g, b);
                        }
                    }

                    _s3d_plot_safe(buf, x3, fx2int(y3), r, g, b);
                }

                return;
            }
            //else // x1 == x2, x1 != x3, x2 == x3
            //{
            //    // Impossible
            //    printf("B");
            //    return;
            //}
        }
        else // x1 == x2, x1 == x3
        {
            //if(x2 != x3)  // x1 == x2, x1 == x3, x2 != x3
            //{
            //    //Impossible
            //    printf("A");
            //    return;
            //}
            //else  // x1 == x2, x1 == x3, x2 == x3
            {
                // Just a line
                if (((u32)x1) < 400)
                {
                    int ymin = min(min(y1, y2), y3);
                    int ymax = max(max(y1, y2), y3);
                    _s3d_vertical_line(linebuf, x1, fx2int(ymin), fx2int(ymax),
                                       r, g, b);
                }
                return;
            }
        }
    }
}

// ----------------------

void S3D_2D_TriangleFillAlpha(u8 *buf, int x1, int y1, int x2, int y2,
                              int x3, int y3, int r, int g, int b, int a)
{
    y1 = int2fx(y1);
    y2 = int2fx(y2);
    y3 = int2fx(y3);

    // Sort: x1 <= x2 <= x3
    if (x1 > x2)
    {
        swapints_pairs(&x1, &x2, &y1, &y2);
    }
    if (x2 > x3)
    {
        swapints_pairs(&x2, &x3, &y2, &y3);
        if (x1 > x2)
        {
            swapints_pairs(&x1, &x2, &y1, &y2);
        }
    }

    u8 *linebuf = &(buf[240 * x1 * 3]);

    if (x1 != x2) // x1 != x2
    {
        int dy1 = fxdiv64(y2 - y1, int2fx(x2 - x1));

        // if (x1 != x3) // x1 != x2, x1 != x3
        {
            int dy2 = fxdiv64(y3 - y1, int2fx(x3 - x1));

            if (x2 != x3) // x1 != x2, x1 != x3, x2 != x3 -> Most common case
            {
                int dy3 = fxdiv64(y3 - y2, int2fx(x3 - x2));

                int sx = x1;
                int sy = y1;
                int ey = y1;

                if (dy1 > dy2)
                {
                    for (; sx < x2; sx++, sy += dy2, ey += dy1, linebuf += 240 * 3)
                    {
                        if ((u32)sx < 400)
                        {
                            _s3d_vertical_line_downwards_alpha(linebuf, sx,
                                                               fx2int(sy),
                                                               fx2int(ey),
                                                               r, g, b, a);
                        }
                    }

                    ey = y2;

                    for (; sx < x3; sx++, sy += dy2, ey += dy3, linebuf += 240 * 3)
                    {
                        if ((u32)sx < 400)
                        {
                            _s3d_vertical_line_downwards_alpha(linebuf, sx,
                                                               fx2int(sy),
                                                               fx2int(ey),
                                                               r, g, b, a);
                        }
                    }

                    _s3d_plot_safe_alpha(buf, x3, fx2int(y3), r, g, b, a);
                }
                else //if(dy1 <= dy2)
                {
                    for (; sx < x2; sx++, sy += dy2, ey += dy1, linebuf += 240 * 3)
                    {
                        if ((u32)sx < 400)
                        {
                            _s3d_vertical_line_downwards_alpha(linebuf, sx,
                                                               fx2int(ey),
                                                               fx2int(sy),
                                                               r, g, b, a);
                        }
                    }

                    ey = y2;

                    for (; sx < x3; sx++, sy += dy2, ey += dy3, linebuf += 240 * 3)
                    {
                        if ((u32)sx < 400)
                        {
                            _s3d_vertical_line_downwards_alpha(linebuf, sx,
                                                               fx2int(ey),
                                                               fx2int(sy),
                                                               r, g, b, a);
                        }
                    }
                    _s3d_plot_safe_alpha(buf, x3, fx2int(y3), r, g, b, a);
                }

                return;
            }
            else // x1 != x2, x1 != x3, x2 == x3
            {
                int sx = x1;
                int sy = y1;
                int ey = y1;

                if (dy1 > dy2)
                {
                    for (; sx < x2; sx++, sy += dy2, ey += dy1, linebuf += 240 * 3)
                    {
                        if ((u32)sx < 400)
                        {
                            _s3d_vertical_line_downwards_alpha(linebuf, sx,
                                                               fx2int(sy),
                                                               fx2int(ey),
                                                               r, g, b, a);
                        }
                    }

                    if (sx >= 0)
                    {
                        _s3d_vertical_line_downwards_alpha(linebuf, sx,
                                                           fx2int(y3),
                                                           fx2int(y2),
                                                           r, g, b, a);
                    }
                }
                else //if(dy1 <= dy2)
                {
                    for (; sx < x2; sx++, sy += dy2, ey += dy1, linebuf += 240 * 3)
                    {
                        if ((u32)sx < 400)
                        {
                            _s3d_vertical_line_downwards_alpha(linebuf, sx,
                                                               fx2int(ey),
                                                               fx2int(sy),
                                                               r, g, b, a);
                        }
                    }

                    if (sx >= 0)
                    {
                        _s3d_vertical_line_downwards_alpha(linebuf, sx,
                                                           fx2int(y2),
                                                           fx2int(y3),
                                                           r, g, b, a);
                    }
                }

                return;
            }
        }
        //else // x1 != x2, x1 == x3
        //{
        //    // Impossible because x1 <= x2 <= x3
        //    printf("C");
        //    return;
        //}
    }
    else // x1 == x2
    {
        if (x1 != x3) // x1 == x2, x1 != x3
        {
            int dy2 = fxdiv64(y3 - y1, int2fx(x3 - x1));

            // if (x2 != x3)  // x1 == x2, x1 != x3, x2 != x3
            {
                int dy3 = fxdiv64(y3 - y2, int2fx(x3 - x2));

                int sx = x1;
                int sy = y1;
                int ey = y2;

                if (y1 > y2)
                {
                    for (; sx < x3; sx++, sy += dy2, ey += dy3, linebuf += 240 * 3)
                    {
                        if ((u32)sx < 400)
                        {
                            _s3d_vertical_line_downwards_alpha(linebuf, sx,
                                                               fx2int(ey),
                                                               fx2int(sy),
                                                               r, g, b, a);
                        }
                    }
                    _s3d_plot_safe_alpha(buf, x3, fx2int(y3), r, g, b, a);
                }
                else //if(dy1 <= dy2)
                {
                    for (; sx < x3; sx++, sy += dy2, ey += dy3, linebuf += 240 * 3)
                    {
                        if ((u32)sx < 400)
                        {
                            _s3d_vertical_line_downwards_alpha(linebuf, sx,
                                                               fx2int(sy),
                                                               fx2int(ey),
                                                               r, g, b, a);
                        }
                    }
                    _s3d_plot_safe_alpha(buf, x3, fx2int(y3), r, g, b, a);
                }

                return;
            }
            //else // x1 == x2, x1 != x3, x2 == x3
            //{
            //    // Impossible
            //    printf("B");
            //    return;
            //}
        }
        else // x1 == x2, x1 == x3
        {
            //if (x2 != x3)  // x1 == x2, x1 == x3, x2 != x3
            //{
            //    // Impossible
            //    printf("A");
            //    return;
            //}
            //else // x1 == x2, x1 == x3, x2 == x3
            {
                // Just a line
                if (((u32)x1) < 400)
                {
                    int ymin = min(min(y1, y2), y3);
                    int ymax = max(max(y1, y2), y3);
                    _s3d_vertical_line_alpha(linebuf, x1,
                                             fx2int(ymin), fx2int(ymax),
                                             r, g, b, a);
                }
                return;
            }
        }
    }
}

// -----------------------------------------------------------------------------

void S3D_2D_QuadAllignedFill(u8 *buf, int x1, int y1, int x2, int y2,
                             int r, int g, int b)
{
    if (y1 > y2)
        swapints(&y1, &y2);
    if (x1 > x2)
        swapints(&x1, &x2);

    //y1 < y2, x1 < x2

    if (y2 < 0)
        return;
    else if (y2 >= 240)
        y2 = 240 - 1;

    if (x2 < 0)
        return;
    else if (x2 >= 400)
        x2 = 400 - 1;

    if (y1 >= 240)
        return;
    else if (y1 < 0)
        y1 = 0;

    if (x1 >= 400)
        return;
    else if (x1 < 0)
        x1 = 0;

    u8 *linebuf = &(buf[240 * x1 * 3]);

    for (; x1 <= x2; x1++, linebuf += 240 * 3)
    {
        int y = y1;
        u8 *p = linebuf + y * 3;
        for (; y <= y2; y++)
        {
            *p++ = b;
            *p++ = g;
            *p++ = r;
        }
    }
}

// ----------------------

void S3D_2D_QuadAllignedFillAlpha(u8 *buf, int x1, int y1, int x2, int y2,
                                  int r, int g, int b, int a)
{
    if (y1 > y2)
        swapints(&y1, &y2);
    if (x1 > x2)
        swapints(&x1, &x2);

    // y1 < y2, x1 < x2

    if (y2 < 0)
        return;
    else if (y2 >= 240)
        y2 = 240 - 1;

    if (x2 < 0)
        return;
    else if (x2 >= 400)
        x2 = 400 - 1;

    if (y1 >= 240)
        return;
    else if (y1 < 0)
        y1 = 0;

    if (x1 >= 400)
        return;
    else if (x1 < 0)
        x1 = 0;

    u8 *linebuf = &(buf[240 * x1 * 3]);

    int one_minus_alpha = 255 - a;

    for (; x1 <= x2; x1++, linebuf += 240 * 3)
    {
        int y = y1;
        u8 *p = linebuf + y * 3;
        for (; y <= y2; y++)
        {
            p[0] = ((p[0] * one_minus_alpha) + b * a) / 256;
            p[1] = ((p[1] * one_minus_alpha) + g * a) / 256;
            p[2] = ((p[2] * one_minus_alpha) + r * a) / 256;
            p += 3;
        }
    }
}

// -----------------------------------------------------------------------------

void S3D_2D_QuadFill(u8 *buf, int x1, int y1, int x2, int y2, int x3, int y3,
                     int x4, int y4, int r, int g, int b)
{
    y1 = int2fx(y1);
    y2 = int2fx(y2);
    y3 = int2fx(y3);
    y4 = int2fx(y4);

    // Sort: x1 <= x2 <= x3 <= x4
    {
        if (x1 > x2)
            swapints_pairs(&x1, &x2, &y1, &y2);

        if (x2 > x3)
        {
            swapints_pairs(&x2, &x3, &y2, &y3);
            if (x1 > x2)
                swapints_pairs(&x1, &x2, &y1, &y2);
        }

        if (x3 > x4)
        {
            swapints_pairs(&x3, &x4, &y3, &y4);
            if (x2 > x3)
            {
                swapints_pairs(&x2, &x3, &y2, &y3);
                if (x1 > x2)
                    swapints_pairs(&x1, &x2, &y1, &y2);
            }
        }
    }

    u8 *linebuf = &(buf[240 * x1 * 3]);

    if (x1 == x2) // 1 = 2
    {
        if (x2 == x3) // 1 = 2 = 3
        {
            if (x3 == x4) // 1 = 2 = 3 = 4
            {
                int ymin = min(min(y1, y2), min(y3, y4));
                int ymax = max(max(y1, y2), max(y3, y4));
                if ((u32)x1 < 400)
                {
                    _s3d_vertical_line(linebuf, x1, fx2int(ymin), fx2int(ymax),
                                       r, g, b);
                }
            }
            else // 1 = 2 = 3 : 4
            {
                int ymin = min(min(y1, y2), y3);
                int ymax = max(max(y1, y2), y3);

                int dsy = fxdiv64(y4 - ymin, int2fx(x4 - x1));
                int dey = fxdiv64(y4 - ymax, int2fx(x4 - x1));

                int sx = x1;
                int sy = ymin, ey = ymax;

                for (; sx < x4; sx++, sy += dsy, ey += dey, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line(linebuf, sx, fx2int(sy), fx2int(ey),
                                           r, g, b);
                    }
                }

                _s3d_plot_safe(buf, x4, fx2int(y4), r, g, b);
            }
        }
        else // 1 = 2 : 3
        {
            if (x3 == x4) // 1 = 2 : 3 = 4
            {
                int ysmin = min(y1, y2);
                int ysmax = max(y1, y2);
                int yemin = min(y3, y4);
                int yemax = max(y3, y4);

                int dymin = fxdiv64(yemin - ysmin, int2fx(x4 - x1));
                int dymax = fxdiv64(yemax - ysmax, int2fx(x4 - x1));

                int sx = x1;
                int sy = ysmin, ey = ysmax;

                for (; sx < x4; sx++, sy += dymin, ey += dymax, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line(linebuf, sx, fx2int(sy), fx2int(ey),
                                           r, g, b);
                    }
                }

                if ((u32)sx < 400)
                {
                    _s3d_vertical_line(linebuf, x4, fx2int(y3), fx2int(y4),
                                       r, g, b);
                }
            }
            else // 1 = 2 : 3 : 4
            {
                if (y1 < y2)
                    swapints(&y1, &y2); // Make y1 > y2

                // Convex quads only!
                int vertex_joined_to_3 = 1;
                int dy1 = fxdiv64(y3 - y1, int2fx(x3 - x1));
                int temp = fxdiv64(y4 - y1, int2fx(x4 - x1));
                if (dy1 < temp)
                {
                    vertex_joined_to_3 = 2;
                    dy1 = temp;
                }

                int dy2 = min(fxdiv64(y3 - y2, int2fx(x3 - x2)),
                              fxdiv64(y4 - y2, int2fx(x4 - x2)));

                int sx = x1;
                int sy = y1, ey = y2;

                for (; sx < x3; sx++, sy += dy1, ey += dy2, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line(linebuf, sx, fx2int(sy), fx2int(ey),
                                           r, g, b);
                    }
                }

                if (vertex_joined_to_3 == 1)
                {
                    dy1 = fxdiv64(y4 - y3, int2fx(x4 - x3));
                    sy = y3;
                }
                else // if (vertex_joined_to_3 == 2)
                {
                    dy2 = fxdiv64(y4 - y3, int2fx(x4 - x3));
                    ey = y3;
                }

                for (; sx < x4; sx++, sy += dy1, ey += dy2, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line(linebuf, sx, fx2int(sy), fx2int(ey),
                                           r, g, b);
                    }
                }

                _s3d_plot_safe(buf, x4, fx2int(y4), r, g, b);
            }
        }
    }
    else // 1 : 2
    {
        if (x2 == x3) // 1 : 2 = 3
        {
            if (x3 == x4) // 1 : 2 = 3 = 4
            {
                int ymin = min(min(y2, y3), y4);
                int ymax = max(max(y2, y3), y4);

                int dsy = fxdiv64(ymin - y1, int2fx(x4 - x1));
                int dey = fxdiv64(ymax - y1, int2fx(x4 - x1));

                int sx = x1;
                int sy = y1, ey = y1;

                for (; sx <= x4; sx++, sy += dsy, ey += dey, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line(linebuf, sx, fx2int(sy), fx2int(ey),
                                           r, g, b);
                    }
                }
            }
            else // 1 : 2 = 3 : 4
            {
                int sx = x1;
                int sy = y1, ey = y1;

                int dy2 = fxdiv64(y2 - y1, int2fx(x2 - x1));
                int dy3 = fxdiv64(y3 - y1, int2fx(x3 - x1));

                for (; sx < x3; sx++, sy += dy2, ey += dy3, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line(linebuf, sx, fx2int(sy), fx2int(ey),
                                           r, g, b);
                    }
                }

                sy = y2, ey = y3;

                dy2 = fxdiv64(y4 - y2, int2fx(x4 - x2));
                dy3 = fxdiv64(y4 - y3, int2fx(x4 - x3));

                for (; sx < x4; sx++, sy += dy2, ey += dy3, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line(linebuf, sx, fx2int(sy), fx2int(ey),
                                           r, g, b);
                    }
                }

                _s3d_plot_safe(buf, x4, fx2int(y4), r, g, b);
            }
        }
        else // 1 : 2 : 3
        {
            if (x3 == x4) // 1 : 2 : 3 = 4
            {
                if (y3 < y4)
                    swapints(&y3, &y4); // Make y3 > y4

                int vertex_joined_to_2 = 0;

                int dy1, dy2;
                // Convex quads only!
                if (y1 > y2)
                {
                    // 1 to 3 | 1 to 2
                    vertex_joined_to_2 = 4; // 2 to 4
                    dy1 = fxdiv64(y3 - y1, int2fx(x3 - x1));
                    dy2 = fxdiv64(y2 - y1, int2fx(x2 - x1));
                }
                else
                {
                    // 1 to 2 | 1 to 4
                    vertex_joined_to_2 = 3; // 2 to 3
                    dy1 = fxdiv64(y2 - y1, int2fx(x2 - x1));
                    dy2 = fxdiv64(y4 - y1, int2fx(x4 - x1));
                }

                int sx = x1;
                int sy = y1, ey = y1;

                for (; sx < x2; sx++, sy += dy1, ey += dy2, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line(linebuf, sx, fx2int(sy), fx2int(ey),
                                           r, g, b);
                    }
                }

                if (vertex_joined_to_2 == 3)
                {
                    // 2 to 3 | keep
                    dy1 = fxdiv64(y3 - y2, int2fx(x3 - x2));
                    sy = y2;
                }
                else // if (vertex_joined_to_2 == 4)
                {
                    // keep | 2 to 4
                    dy2 = fxdiv64(y4 - y2, int2fx(x4 - x2));
                    ey = y2;
                }

                for (; sx <= x4; sx++, sy += dy1, ey += dy2, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line(linebuf, sx, fx2int(sy), fx2int(ey),
                                           r, g, b);
                    }
                }
            }
            else // 1 : 2 : 3 : 4
            {
                // Convex only!

                int dy12 = fxdiv64(y2 - y1, int2fx(x2 - x1));
                int dy13 = fxdiv64(y3 - y1, int2fx(x3 - x1));
                int dy14 = fxdiv64(y4 - y1, int2fx(x4 - x1));

                int dys = min(min(dy12, dy13), dy14);
                int dye = max(max(dy12, dy13), dy14);

                int sx = x1;
                int sy = y1, ey = y1;

                for (; sx < x2; sx++, sy += dys, ey += dye, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line(linebuf, sx, fx2int(sy), fx2int(ey),
                                           r, g, b);
                    }
                }

                int dy23 = fxdiv64(y3 - y2, int2fx(x3 - x2));
                int dy24 = fxdiv64(y4 - y2, int2fx(x4 - x2));

                if (abs(sy - y2) < abs(ey - y2))
                {
                    // change sy
                    sy = y2;
                    dys = min(dy23, dy24);
                }
                else
                {
                    // change ey
                    ey = y2;
                    dye = max(dy23, dy24);
                }

                for (; sx < x3; sx++, sy += dys, ey += dye, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line(linebuf, sx, fx2int(sy), fx2int(ey),
                                           r, g, b);
                    }
                }

                int dy34 = fxdiv64(y4 - y3, int2fx(x4 - x3));

                if (abs(sy - y3) < abs(ey - y3))
                {
                    // change sy
                    sy = y3;
                    dys = dy34;
                }
                else
                {
                    // change ey
                    ey = y3;
                    dye = dy34;
                }

                for (; sx < x4; sx++, sy += dys, ey += dye, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line(linebuf, sx, fx2int(sy), fx2int(ey),
                                           r, g, b);
                    }
                }

                _s3d_plot_safe(buf, x4, fx2int(y4), r, g, b);
            }
        }
    }
}

// ----------------------

void S3D_2D_QuadFillAlpha(u8 *buf, int x1, int y1, int x2, int y2,
                          int x3, int y3, int x4, int y4,
                          int r, int g, int b, int a)
{
    y1 = int2fx(y1);
    y2 = int2fx(y2);
    y3 = int2fx(y3);
    y4 = int2fx(y4);

    // Sort: x1 <= x2 <= x3 <= x4
    {
        if (x1 > x2)
            swapints_pairs(&x1, &x2, &y1, &y2);

        if (x2 > x3)
        {
            swapints_pairs(&x2, &x3, &y2, &y3);
            if (x1 > x2)
                swapints_pairs(&x1, &x2, &y1, &y2);
        }

        if (x3 > x4)
        {
            swapints_pairs(&x3, &x4, &y3, &y4);
            if (x2 > x3)
            {
                swapints_pairs(&x2, &x3, &y2, &y3);
                if (x1 > x2)
                    swapints_pairs(&x1, &x2, &y1, &y2);
            }
        }
    }

    u8 *linebuf = &(buf[240 * x1 * 3]);

    if (x1 == x2) // 1 = 2
    {
        if (x2 == x3) // 1 = 2 = 3
        {
            if (x3 == x4) // 1 = 2 = 3 = 4
            {
                int ymin = min(min(y1, y2), min(y3, y4));
                int ymax = max(max(y1, y2), max(y3, y4));
                if ((u32)x1 < 400)
                {
                    _s3d_vertical_line_alpha(linebuf, x1,
                                             fx2int(ymin), fx2int(ymax),
                                             r, g, b, a);
                }
            }
            else // 1 = 2 = 3 : 4
            {
                int ymin = min(min(y1, y2), y3);
                int ymax = max(max(y1, y2), y3);

                int dsy = fxdiv64(y4 - ymin, int2fx(x4 - x1));
                int dey = fxdiv64(y4 - ymax, int2fx(x4 - x1));

                int sx = x1;
                int sy = ymin, ey = ymax;

                for (; sx < x4; sx++, sy += dsy, ey += dey, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line_alpha(linebuf, sx,
                                                 fx2int(sy), fx2int(ey),
                                                 r, g, b, a);
                    }
                }

                _s3d_plot_safe_alpha(buf, x4, fx2int(y4), r, g, b, a);
            }
        }
        else // 1 = 2 : 3
        {
            if (x3 == x4) // 1 = 2 : 3 = 4
            {
                int ysmin = min(y1, y2);
                int ysmax = max(y1, y2);
                int yemin = min(y3, y4);
                int yemax = max(y3, y4);

                int dymin = fxdiv64(yemin - ysmin, int2fx(x4 - x1));
                int dymax = fxdiv64(yemax - ysmax, int2fx(x4 - x1));

                int sx = x1;
                int sy = ysmin, ey = ysmax;

                for (; sx < x4; sx++, sy += dymin, ey += dymax, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line_alpha(linebuf, sx,
                                                 fx2int(sy), fx2int(ey),
                                                 r, g, b, a);
                    }
                }

                if ((u32)sx < 400)
                {
                    _s3d_vertical_line_alpha(linebuf, x4,
                                             fx2int(y3), fx2int(y4),
                                             r, g, b, a);
                }
            }
            else // 1 = 2 : 3 : 4
            {
                if (y1 < y2)
                    swapints(&y1, &y2); // Make y1 > y2

                //Convex quads only!
                int vertex_joined_to_3 = 1;
                int dy1 = fxdiv64(y3 - y1, int2fx(x3 - x1));
                int temp = fxdiv64(y4 - y1, int2fx(x4 - x1));
                if (dy1 < temp)
                {
                    vertex_joined_to_3 = 2;
                    dy1 = temp;
                }

                int dy2 = min(fxdiv64(y3 - y2, int2fx(x3 - x2)),
                              fxdiv64(y4 - y2, int2fx(x4 - x2)));

                int sx = x1;
                int sy = y1, ey = y2;

                for (; sx < x3; sx++, sy += dy1, ey += dy2, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line_alpha(linebuf, sx,
                                                 fx2int(sy), fx2int(ey),
                                                 r, g, b, a);
                    }
                }
                if (vertex_joined_to_3 == 1)
                {
                    dy1 = fxdiv64(y4 - y3, int2fx(x4 - x3));
                    sy = y3;
                }
                else // if (vertex_joined_to_3 == 2)
                {
                    dy2 = fxdiv64(y4 - y3, int2fx(x4 - x3));
                    ey = y3;
                }

                for (; sx < x4; sx++, sy += dy1, ey += dy2, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line_alpha(linebuf, sx,
                                                 fx2int(sy), fx2int(ey),
                                                 r, g, b, a);
                    }
                }

                _s3d_plot_safe_alpha(buf, x4, fx2int(y4), r, g, b, a);
            }
        }
    }
    else // 1 : 2
    {
        if (x2 == x3) // 1 : 2 = 3
        {
            if (x3 == x4) // 1 : 2 = 3 = 4
            {
                int ymin = min(min(y2, y3), y4);
                int ymax = max(max(y2, y3), y4);

                int dsy = fxdiv64(ymin - y1, int2fx(x4 - x1));
                int dey = fxdiv64(ymax - y1, int2fx(x4 - x1));

                int sx = x1;
                int sy = y1, ey = y1;

                for (; sx <= x4; sx++, sy += dsy, ey += dey, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line_alpha(linebuf, sx,
                                                 fx2int(sy), fx2int(ey),
                                                 r, g, b, a);
                    }
                }
            }
            else // 1 : 2 = 3 : 4
            {
                int sx = x1;
                int sy = y1, ey = y1;

                int dy2 = fxdiv64(y2 - y1, int2fx(x2 - x1));
                int dy3 = fxdiv64(y3 - y1, int2fx(x3 - x1));

                for (; sx < x3; sx++, sy += dy2, ey += dy3, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line_alpha(linebuf, sx,
                                                 fx2int(sy), fx2int(ey),
                                                 r, g, b, a);
                    }
                }

                sy = y2, ey = y3;

                dy2 = fxdiv64(y4 - y2, int2fx(x4 - x2));
                dy3 = fxdiv64(y4 - y3, int2fx(x4 - x3));

                for (; sx < x4; sx++, sy += dy2, ey += dy3, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line_alpha(linebuf, sx,
                                                 fx2int(sy), fx2int(ey),
                                                 r, g, b, a);
                    }
                }

                _s3d_plot_safe_alpha(buf, x4, fx2int(y4), r, g, b, a);
            }
        }
        else // 1 : 2 : 3
        {
            if (x3 == x4) // 1 : 2 : 3 = 4
            {
                if (y3 < y4)
                    swapints(&y3, &y4); // Make y3 > y4

                int vertex_joined_to_2 = 0;

                int dy1, dy2;
                // Convex quads only!
                if (y1 > y2)
                {
                    // 1 to 3 | 1 to 2
                    vertex_joined_to_2 = 4; // 2 to 4
                    dy1 = fxdiv64(y3 - y1, int2fx(x3 - x1));
                    dy2 = fxdiv64(y2 - y1, int2fx(x2 - x1));
                }
                else
                {
                    // 1 to 2 | 1 to 4
                    vertex_joined_to_2 = 3; // 2 to 3
                    dy1 = fxdiv64(y2 - y1, int2fx(x2 - x1));
                    dy2 = fxdiv64(y4 - y1, int2fx(x4 - x1));
                }

                int sx = x1;
                int sy = y1, ey = y1;

                for (; sx < x2; sx++, sy += dy1, ey += dy2, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line_alpha(linebuf, sx,
                                                 fx2int(sy), fx2int(ey),
                                                 r, g, b, a);
                    }
                }

                if (vertex_joined_to_2 == 3)
                {
                    // 2 to 3 | keep
                    dy1 = fxdiv64(y3 - y2, int2fx(x3 - x2));
                    sy = y2;
                }
                else // if (vertex_joined_to_2 == 4)
                {
                    // keep | 2 to 4
                    dy2 = fxdiv64(y4 - y2, int2fx(x4 - x2));
                    ey = y2;
                }

                for (; sx <= x4; sx++, sy += dy1, ey += dy2, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line_alpha(linebuf, sx,
                                                 fx2int(sy), fx2int(ey),
                                                 r, g, b, a);
                    }
                }
            }
            else // 1 : 2 : 3 : 4
            {
                // Convex only!

                int dy12 = fxdiv64(y2 - y1, int2fx(x2 - x1));
                int dy13 = fxdiv64(y3 - y1, int2fx(x3 - x1));
                int dy14 = fxdiv64(y4 - y1, int2fx(x4 - x1));

                int dys = min(min(dy12, dy13), dy14);
                int dye = max(max(dy12, dy13), dy14);

                int sx = x1;
                int sy = y1, ey = y1;

                for (; sx < x2; sx++, sy += dys, ey += dye, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line_alpha(linebuf, sx,
                                                 fx2int(sy), fx2int(ey),
                                                 r, g, b, a);
                    }
                }

                int dy23 = fxdiv64(y3 - y2, int2fx(x3 - x2));
                int dy24 = fxdiv64(y4 - y2, int2fx(x4 - x2));

                if (abs(sy - y2) < abs(ey - y2))
                {
                    // change sy
                    sy = y2;
                    dys = min(dy23, dy24);
                }
                else
                {
                    // change ey
                    ey = y2;
                    dye = max(dy23, dy24);
                }

                for (; sx < x3; sx++, sy += dys, ey += dye, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line_alpha(linebuf, sx,
                                                 fx2int(sy), fx2int(ey),
                                                 r, g, b, a);
                    }
                }

                int dy34 = fxdiv64(y4 - y3, int2fx(x4 - x3));

                if (abs(sy - y3) < abs(ey - y3))
                {
                    // change sy
                    sy = y3;
                    dys = dy34;
                }
                else
                {
                    // change ey
                    ey = y3;
                    dye = dy34;
                }

                for (; sx < x4; sx++, sy += dys, ey += dye, linebuf += 240 * 3)
                {
                    if ((u32)sx < 400)
                    {
                        _s3d_vertical_line_alpha(linebuf, sx,
                                                 fx2int(sy), fx2int(ey),
                                                 r, g, b, a);
                    }
                }

                _s3d_plot_safe_alpha(buf, x4, fx2int(y4), r, g, b, a);
            }
        }
    }
}
