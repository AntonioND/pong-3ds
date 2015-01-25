
#include <3ds.h>

#include <stdlib.h>
#include <stdio.h>

#include "utils.h"
#include "png_utils.h"

//----------------------------------------------------------------------------------

#define TICKS_PER_SEC (268123480)
#define TICKS_PER_VBL (268123480/60)

int frame_count;
u64 last_ticks = 0;
int _FPS = 0;
u64 cpu_last_ticks = 0;
float _CPU = 0;

void Timing_Start(void)
{
	last_ticks = svcGetSystemTick();
}

void Timing_EndFrame(void)
{
	frame_count ++;
	u64 inc_ticks = svcGetSystemTick() - cpu_last_ticks;
	_CPU = (float)inc_ticks*((float)100.0f/(float)TICKS_PER_VBL);
}

void Timing_StartFrame(void)
{
	if(svcGetSystemTick() >= last_ticks + TICKS_PER_SEC)
	{
		last_ticks += TICKS_PER_SEC;
		_FPS = frame_count;
		frame_count = 0;	
	}
	
	cpu_last_ticks = svcGetSystemTick();
}

int Timing_GetFPS(void)
{
	return _FPS;
}

float Timing_GetCPUUsage(void)
{
	return _CPU;
}


//----------------------------------------------------------------------------------

//Xorshift RNGs, by George Marsaglia (The Florida State University)

static uint64_t x=123456789,y=362436069,z=521288629,w=88675123;

static inline uint64_t xor128(void)
{
    uint64_t t;
    t=(x^(x<<11)); x=y; y=z; z=w;
    return ( w=(w^(w>>19))^(t^(t>>8)) );
}

inline void fast_srand(uint64_t seed)
{
    x = seed; // Some way to seed the RNG is needed, even in the paper is not specified...
}

inline uint32_t fast_rand(void)
{
    return xor128()&FAST_RAND_MAX;
}

//-------------------------------------------------------------------------------------------------------

#define CONFIG_3D_SLIDERSTATE (*(float*)0x1FF81080) //this should be in ctrulib...

void save_screenshot(void)
{
	u32 * dst = malloc(400*240*sizeof(u32));
	if(dst)
	{
		char filename[60];
		u64 time = osGetTime();
		int i,j;
		
		if(CONFIG_3D_SLIDERSTATE == 0.0f)
		{
			u8 * src = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
			for(i = 0; i < 400; i++) for(j = 240-1; j >= 0; j--)
			{
				int b = *src++;
				int g = *src++;
				int r = *src++;
				dst[j*400+i] = (b<<16)|(g<<8)|r;
			}
			
			snprintf(filename,sizeof(filename),"%llu.png",time); // long long unsigned int
			Save_PNG(filename, 400, 240, (void*)dst,0);
		}
		else
		{
			u8 * src = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
			for(i = 0; i < 400; i++) for(j = 240-1; j >= 0; j--)
			{
				int b = *src++;
				int g = *src++;
				int r = *src++;
				dst[j*400+i] = (b<<16)|(g<<8)|r;
			}
			snprintf(filename,sizeof(filename),"%llu_left.png",time); // long long unsigned int
			Save_PNG(filename, 400, 240, (void*)dst,0);
			
			src = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL);
			for(i = 0; i < 400; i++) for(j = 240-1; j >= 0; j--)
			{
				int b = *src++;
				int g = *src++;
				int r = *src++;
				dst[j*400+i] = (b<<16)|(g<<8)|r;
			}
			snprintf(filename,sizeof(filename),"%llu_right.png",time); // long long unsigned int
			Save_PNG(filename, 400, 240, (void*)dst,0);
		}
		
		free(dst);
	}
}
//-------------------------------------------------------------------------------------------------------
