// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

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
