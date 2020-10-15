// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#ifndef UTILS_H__
#define UTILS_H__

#include <3ds.h>

// in fbcopy.s
void FastFramebufferCopy(const void *src, void *dst, u32 bottom_screen);

// -----------------------------------------------------------------------------

void Timing_Start(int cpu_core);
void Timing_EndFrame(int cpu_core);
void Timing_StartFrame(int cpu_core);
int Timing_GetFPS(int cpu_core);
float Timing_GetCPUUsage(int cpu_core);

// -----------------------------------------------------------------------------

#define FAST_RAND_MAX (0x7FFF)

void fast_srand(uint64_t seed);
uint32_t fast_rand(void);

// -----------------------------------------------------------------------------

void PNGScreenshot_Top(void);
void PNGScreenshot_Bottom(void);

#endif // UTILS_H__
