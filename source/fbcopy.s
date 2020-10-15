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

@ void FastFramebufferCopy(const void * src, void * dst, u32 bottom_screen)

@ r0 = src
@ r1 = dst
@ r2 = bottom_screen

    .align  2
    .code   32
    .global FastFramebufferCopy

FastFramebufferCopy:
    push    {r3-r8}

    @ 256 pixels per iteration
    ands    r2,#1
    movne   r2,#(320*240)/256            @ Bottom
    moveq   r2,#((400*240)/256)&0x00FF   @ Top
    orreq   r2,#(((400*240)/256)&0xFF00) @ Top

.loop:

    .rept   8*4
    ldmia   r0!, {r3-r8}
    stmia   r1!, {r3-r8}  @ 8 pixels per instruction, 256 pixels per iteration
    .endr

    subs    r2,r2,#1
    bne     .loop

    pop     {r3-r8}
    bx      lr
