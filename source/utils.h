
#ifndef __UTILS__
#define __UTILS__

//----------------------------------------------------------------------------------

#include <3ds.h>

//----------------------------------------------------------------------------------

void Timing_Start(void);
void Timing_EndFrame(void);
void Timing_StartFrame(void);
int Timing_GetFPS(void);
float Timing_GetCPUUsage(void);

//----------------------------------------------------------------------------------

#define FAST_RAND_MAX (0x7FFF)

inline void fast_srand(uint64_t seed);
inline uint32_t fast_rand(void);

//----------------------------------------------------------------------------------

void Screenshot_PNG(void);

//----------------------------------------------------------------------------------

#endif //__UTILS__