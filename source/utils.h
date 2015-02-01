
#ifndef __UTILS__
#define __UTILS__

//----------------------------------------------------------------------------------

#include <3ds.h>

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