
//-------------------------------------------------------------------------------------------------------

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

//-------------------------------------------------------------------------------------------------------

#ifndef __UTILS__
#define __UTILS__

//----------------------------------------------------------------------------------

#include <3ds.h>

//----------------------------------------------------------------------------------

// in fbcopy.s
void FastFramebufferCopy(const void * src, void * dst, u32 bottom_screen);

//----------------------------------------------------------------------------------

void Timing_Start(int cpu_core);
void Timing_EndFrame(int cpu_core);
void Timing_StartFrame(int cpu_core);
int Timing_GetFPS(int cpu_core);
float Timing_GetCPUUsage(int cpu_core);

//----------------------------------------------------------------------------------

#define FAST_RAND_MAX (0x7FFF)

inline void fast_srand(uint64_t seed);
inline uint32_t fast_rand(void);

//----------------------------------------------------------------------------------

void PNGScreenshot_Top(void);
void PNGScreenshot_Bottom(void);

//----------------------------------------------------------------------------------

#endif //__UTILS__