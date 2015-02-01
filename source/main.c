
//-------------------------------------------------------------------------------------------------------

#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "engine.h"
#include "utils.h"
#include "game.h"
#include "ttf_console.h"

//-------------------------------------------------------------------------------------------------------

#define CONFIG_3D_SLIDERSTATE (*(float*)0x1FF81080) //this should be in ctrulib...

//s32 __svcGetCurrentProcessorNumber(void);

//-------------------------------------------------------------------------------------------------------

int SecondaryThreadExit = 0;

Handle SecondaryThreadHandle, mutex_thread_drawing, mutex_sync_frame;

//-------------------------------------------------------------------------------------------------------

static m44 left_screen, right_screen;

static void ProjectionMatricesConfigure(void)
{
	// Far is > 2.5 z
	// Screen is at 2.5 z
	// Over screen is < 2.5 z

#define TRANSL_DIV_FACTOR (4.0f)
#define SHEAR_DIV_FACTOR  (2.0f)

	m44 p, t;
	
	float slider = CONFIG_3D_SLIDERSTATE;
	int transl = float2fx(slider/TRANSL_DIV_FACTOR);
	int shear = float2fx(slider/SHEAR_DIV_FACTOR);
	
	m44_CreateFrustum(&p, -float2fx(2.5)+shear, float2fx(2.5)+shear, -float2fx(1.5), float2fx(1.5), float2fx(5), float2fx(10));
	m44_CreateTranslation(&t,-transl,0,0);
	m44_Multiply(&p,&t,&left_screen);
	
	if(slider == 0.0f) return;
	m44_CreateFrustum(&p, -float2fx(2.5)-shear, float2fx(2.5)-shear, -float2fx(1.5), float2fx(1.5), float2fx(5), float2fx(10));
	m44_CreateTranslation(&t,+transl,0,0);
	m44_Multiply(&p,&t,&right_screen);
}

static void DrawScreens(void)
{
	S3D_BuffersSetup();
	
	//----------------------------------------
	
	svcWaitSynchronization(mutex_sync_frame, U64_MAX);
	svcReleaseMutex(mutex_thread_drawing);
	
	//----------------------------------------
	
	int screen = 0;
	S3D_ProjectionMatrixSet(screen, &left_screen);
	S3D_PolygonListClear(screen);
	Game_DrawScene(screen);
	S3D_PolygonListFlush(screen, 1);
	
	//----------------------------------------

	svcWaitSynchronization(mutex_thread_drawing, U64_MAX);
	svcReleaseMutex(mutex_sync_frame);
}

//-------------------------------------------------------------------------------------------------------

void SecondaryThreadFunction(u32 arg)
{
	//Running in CPU 1
	//Con_Print(b,0,60,"Thread 1 CPU: %d",__svcGetCurrentProcessorNumber());

	Timing_Start(1);

	while(SecondaryThreadExit == 0)
	{
		while(SecondaryThreadExit == 0)
		{
			if(svcWaitSynchronization(mutex_thread_drawing, U64_MAX) == 0) break;
		}
		
		if(SecondaryThreadExit) break;
		
		//----------------------------------------
		
		Timing_StartFrame(1);
		
		float slider = CONFIG_3D_SLIDERSTATE;
		if(slider > 0.0f)
		{
			int screen = 1;
			S3D_ProjectionMatrixSet(screen, &right_screen);
			S3D_PolygonListClear(screen);
			Game_DrawScene(screen);
			S3D_PolygonListFlush(screen, 1);
		}
		
		Timing_EndFrame(1);
		
		//----------------------------------------
		
		svcReleaseMutex(mutex_thread_drawing);
		
		while(SecondaryThreadExit == 0)
		{
			if(svcWaitSynchronization(mutex_sync_frame, U64_MAX) == 0) break;
		}
		
		svcReleaseMutex(mutex_sync_frame);
	}
	
	svcExitThread();
}

#define STACK_SIZE (0x2000)
u64 SecondaryThreadStack[STACK_SIZE/sizeof(u64)];

void ThreadInit(void)
{
	int disablecores = 1; // Run on the other CPU only
	u32 prio = 0x18; // 0x18 ... 0x3F. Lower is higher. main() is 0x30
	u32 arg = 0;
	
	s32 val = svcCreateThread(&SecondaryThreadHandle, SecondaryThreadFunction, arg,
					(u32*)&(SecondaryThreadStack[STACK_SIZE/sizeof(u64)]), prio, disablecores);
	if(val)
	{
		//error!
	}
	
	val = svcCreateMutex(&mutex_thread_drawing, true); // initially locked
	if(val)
	{
		//error!
	}
	
	val = svcCreateMutex(&mutex_sync_frame, false); // initially released
	if(val)
	{
		//error!
	}
}

void ThreadEnd(void)
{
	SecondaryThreadExit = 1;

	svcReleaseMutex(mutex_thread_drawing);
	svcReleaseMutex(mutex_sync_frame);
	
	svcWaitSynchronization(SecondaryThreadHandle, U64_MAX);
	
	svcCloseHandle(mutex_thread_drawing);
	svcCloseHandle(mutex_sync_frame);
	svcCloseHandle(SecondaryThreadHandle);
}

//-------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	//Running in CPU 0
	
	aptInit();
	gfxInitDefault();
	gfxSet3D(true); //OMG 3D!!!!1!!!
	
	//gfxSetDoubleBuffering(GFX_BOTTOM, false);
	
	//Try to get as much as CPU time as possible (?)
	u32 percent;
	{
		aptOpenSession();
		u32 i;
		for(i = 100; i > 1; i--)
		{
			APT_SetAppCpuTimeLimit(NULL,i);
			APT_GetAppCpuTimeLimit(NULL, &percent);
			if(i == percent) break;
		}
		aptCloseSession();
	}
	
	while(aptMainLoop())
	{
		hidScanInput();
		int keys = hidKeysHeld();
		if(keys & KEY_A) break; // break in order to return to hbmenu
		
		u8 * buf = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
		Game_DrawBottomScreen();
		Con_Print(buf,0,220-1,"Pong 3DS by AntonioND");
		Con_Print(buf,0,200-1,"(Antonio Niño Díaz)");
	
		Con_Print(buf,0,100,"CPU limit: %d%%",(int)percent);
		//Con_Print(buf,0,80,"Thread 0 CPU: %d",__svcGetCurrentProcessorNumber());

		Con_Print(buf,0,40,"A: Start.");
		Con_Print(buf,0,20,"SELECT: Screenshot.");
		Con_Print(buf,0,0,"START:  Exit.");
		
		gfxFlushBuffers();
		gfxSwapBuffers();
		
		gspWaitForVBlank();
	}

	fast_srand(svcGetSystemTick());
	
	ThreadInit();
	
	Game_Init();
	
	Timing_Start(0);
	
	// Main loop
	while(aptMainLoop())
	{
		Timing_StartFrame(0);
		
		hidScanInput();
		int keys = hidKeysHeld();
		if(keys & KEY_START) break; // break in order to return to hbmenu
		
		Game_Handle(keys);
		
		u8 * buf = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
		Con_Print(buf,0,170,"3D Slider: %f        ",CONFIG_3D_SLIDERSTATE);
		Con_Print(buf,0,150,"FPS: %d %d ",Timing_GetFPS(0),Timing_GetFPS(1));
		Con_Print(buf,0,130,"CPU: %d%% %d%% ",(int)Timing_GetCPUUsage(0),(int)Timing_GetCPUUsage(1));
		
		ProjectionMatricesConfigure();
		DrawScreens();
		
		gfxFlushBuffers();
		gfxSwapBuffers();
		
		if(keys & KEY_SELECT)
		{
			PNGScreenshot_Top(); // AFTER DRAWING SCREENS!!
			//PNGScreenshot_Bottom();
		}
		
		Timing_EndFrame(0);
		
		gspWaitForVBlank();
	}
	
	Game_End();
	
	ThreadEnd();

	gfxExit();
	aptExit();
	return 0;
}


	
