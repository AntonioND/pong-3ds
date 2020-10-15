/*
    Pong 3DS. Just a pong for the Nintendo 3DS.
    Copyright (C) 2015 Antonio Niño Díaz

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <3ds.h>

#include <stdio.h>
#include <stdarg.h>

//----------------------------------------------------------------------------------

static void _quad_blit_unsafe_ignorealpha_32(u8 * buf, const u32 * src, int x, int y, int w, int h)
{
    u8 * linebuf = &(buf[(240*x+y)*3]);

    while(w--)
    {
        int i = h;
        u8 * p = linebuf; linebuf += 240*3;
        while(i--)
        {
            u32 data = *src++;
            //if(data & 0xFF) // alpha
            //{
                *p++ = data>>8;
                *p++ = data>>16;
                *p++ = data>>24;
            //}
            //else
            //{
            //    p += 3; src += 3;
            //}
        }
    }
}

#include "liberation_sans_bold_bin.h"

int draw_char(u8 * buf, char c, int x, int y) // returns new x coordinate
{
    u32 * font = ((u32*)liberation_sans_bold_bin);

    u32 offset = font[(u8)c];
    if(offset == 0xFFFFFFFF) return x;

    u32 * data = &(font[offset]);
    u32 w = *data++;
    u32 h = *data++;
    if(w && h) // both dimensions != 0
        _quad_blit_unsafe_ignorealpha_32(buf, data, x,y, w,h);

    return x+w;
}

//----------------------------------------------------------------------------------

void Con_Print(u8 * buf, int x, int y, const char * format, ...)
{
    char buffer[2000];
    va_list args;
    va_start(args,format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    int x_ = x, y_ = y;

    int i = 0;
    while(1)
    {
        char c = buffer[i++];
        if(c == '\0') return;

        x_ = draw_char(buf,c,x_,y_);
    }
}
