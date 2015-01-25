
// void fill_framebuffer(void * dst, u32 color, u32 bottom_screen) // color = 31 _BGR 0

// r0 = dst
// r1 = data
// r2 = words

	.align	2
	.code	32
	.global	fill_framebuffer
fill_framebuffer:
	push	{r4-r7}
	
	orr	r1,r1,r1,lsl #24  // r1 = RBGR
	
	mov	r3,r1,lsr #8      // (r3 = _RBG)
	orr	r3,r3,r3,lsl #24  // r3 = GRBG
	
	mov	r4,r3,lsr #8      // (r4 = _GRB)
	orr	r4,r4,r4,lsl #24  // r4 = BGRB

	
	// Duplicate
	mov	r5,r1
	mov	r6,r3
	mov	r7,r4
	
	// 256 pixels per iteration
	ands	r2,#1
	movne	r2,#(320*240)/256            //Bottom
	moveq	r2,#((400*240)/256)&0x00FF   //Top
	orreq	r2,#(((400*240)/256)&0xFF00) //Top
	
.loop:
	.rept	8*4
	stmia	r0!, {r1, r3-r7} // 8 pixels per instruction, 256 pixels per iteration
	.endr
	subs	r2,r2,#1
	bne		.loop
	
	pop		{r4-r7}
	bx	lr
