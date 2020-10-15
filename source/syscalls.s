// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

.arm
.align 4

.global __svcGetCurrentProcessorNumber
.type __svcGetCurrentProcessorNumber, %function
__svcGetCurrentProcessorNumber:
	svc 0x11
	bx   lr
