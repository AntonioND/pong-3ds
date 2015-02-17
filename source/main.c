
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

#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "S3D/engine.h"
#include "utils.h"
#include "game.h"
#include "ttf_console.h"
#include "sound.h"

//-------------------------------------------------------------------------------------------------------

#define CONFIG_3D_SLIDERSTATE (*(float*)0x1FF81080) //this should be in ctrulib...

//-------------------------------------------------------------------------------------------------------

static int SecondaryThreadExit = 0;

static Handle SecondaryThreadHandle, MutexThreadDrawing, MutexSyncFrame;

//-------------------------------------------------------------------------------------------------------

static m44 left_screen, right_screen;

static void ProjectionMatricesConfigure(void)
{
	// Far is > 2.5 z
	// Screen is at 2.5 z
	// Over screen is < 2.5 z

#define TRANSL_DIV_FACTOR (8.0f)
#define SHEAR_DIV_FACTOR  (4.0f)

	m44 p, t;
	
	float slider = CONFIG_3D_SLIDERSTATE;
	int transl = float2fx(slider/TRANSL_DIV_FACTOR);
	int shear = float2fx(slider/SHEAR_DIV_FACTOR);

	m44_CreateFrustum(&p, -float2fx(2.5)+shear, float2fx(2.5)+shear, -float2fx(1.5), float2fx(1.5), float2fx(5), float2fx(10));
	m44_CreateTranslation(&t,+transl,0,0);
	m44_Multiply(&p,&t,&left_screen);
	
	if(slider == 0.0f) return;
	m44_CreateFrustum(&p, -float2fx(2.5)-shear, float2fx(2.5)-shear, -float2fx(1.5), float2fx(1.5), float2fx(5), float2fx(10));
	m44_CreateTranslation(&t,-transl,0,0);
	m44_Multiply(&p,&t,&right_screen);
}

static void DrawScreens(void)
{
	S3D_BuffersSetup();
	
	//----------------------------------------
	
	ProjectionMatricesConfigure();
	
	//----------------------------------------
	
	svcWaitSynchronization(MutexSyncFrame, U64_MAX); // Don't wait forever in main() thread
	svcReleaseMutex(MutexThreadDrawing);
	
	//----------------------------------------
	
	int screen = 0;
	S3D_ProjectionMatrixSet(screen, &left_screen);
	S3D_PolygonListClear(screen);
	Game_DrawScreenTop(screen);
	
	//----------------------------------------
	
	Game_DrawScreenBottom(); // Draw bottom screen while the other thread finishes its work.
	
	//----------------------------------------

	svcWaitSynchronization(MutexThreadDrawing, U64_MAX); // Don't wait forever in main() thread
	svcReleaseMutex(MutexSyncFrame);
}

//-------------------------------------------------------------------------------------------------------

void SecondaryThreadFunction(u32 arg)
{
	Timing_Start(1);

	while(SecondaryThreadExit == 0)
	{
		while(SecondaryThreadExit == 0)
		{
			if(svcWaitSynchronization(MutexThreadDrawing, U64_MAX) == 0) break;
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
			Game_DrawScreenTop(screen);
		}
		
		Sound_Handle();
		
		Timing_EndFrame(1);
		
		//----------------------------------------
		
		svcReleaseMutex(MutexThreadDrawing);
		
		while(SecondaryThreadExit == 0)
		{
			if(svcWaitSynchronization(MutexSyncFrame, U64_MAX) == 0) break;
		}
		
		svcReleaseMutex(MutexSyncFrame);
	}
	
	svcExitThread();
}

#define STACK_SIZE (0x2000)
static u64 SecondaryThreadStack[STACK_SIZE/sizeof(u64)]; // u64 to align the address

int Thread_Init(void) // Start thread in CPU 1
{
	int disablecores = 1; // Run on the other CPU only
	u32 prio = 0x18; // 0x18 ... 0x3F. Lower is higher. main() is 0x30
	u32 arg = 0;
	
	s32 val = svcCreateThread(&SecondaryThreadHandle, SecondaryThreadFunction, arg,
					(u32*)&(SecondaryThreadStack[STACK_SIZE/sizeof(u64)]), prio, disablecores);
	if(val)
	{
		return 1;
	}
	
	val = svcCreateMutex(&MutexThreadDrawing, true); // initially locked
	if(val)
	{
		svcCloseHandle(SecondaryThreadHandle);
		return 1;
	}
	
	val = svcCreateMutex(&MutexSyncFrame, false); // initially released
	if(val)
	{
		svcCloseHandle(MutexThreadDrawing);
		svcCloseHandle(SecondaryThreadHandle);
		return 1;
	}
	
	return 0;
}

void Thread_End(void)
{
	SecondaryThreadExit = 1;

	svcReleaseMutex(MutexThreadDrawing);
	svcReleaseMutex(MutexSyncFrame);
	
	while(1)
	{
		// This will hang the CPU if the secondary thread can't exit, but I prefer the game to
		// hang than returning to the loader with the secondary thread running in the background.
		if(svcWaitSynchronization(SecondaryThreadHandle, U64_MAX) == 0) break;
	}
	
	svcCloseHandle(MutexThreadDrawing);
	svcCloseHandle(MutexSyncFrame);
	svcCloseHandle(SecondaryThreadHandle);
}

//-------------------------------------------------------------------------------------------------------

void SetMaxCpuTimeLimit(void)
{
	//Try to get as much as CPU time as possible (?)

	aptOpenSession();
	APT_SetAppCpuTimeLimit(NULL,80);
//	u32 i, percent;
//	for(i = 100; i > 1; i--)
//	{
//		APT_SetAppCpuTimeLimit(NULL,i);
//		APT_GetAppCpuTimeLimit(NULL, &percent);
//		if(i == percent) break;
//	}
	aptCloseSession();
}

//-------------------------------------------------------------------------------------------------------

int main(int argc, char **argv) // Running in CPU 0
{
	aptInit();
	gfxInitDefault();
	gfxSet3D(true); //OMG 3D!!!!1!!!
	
	SetMaxCpuTimeLimit();

	fast_srand(svcGetSystemTick());
	
	if(Thread_Init() == 0)
	{
		Sound_Init();
		
		Game_Init();
		
		Timing_Start(0);
		
		// Main loop
		while(aptMainLoop())
		{
			Timing_StartFrame(0);
			
			hidScanInput(); // Once per frame
			
			if(hidKeysHeld() & KEY_SELECT) break; // break in order to return to hbmenu
			
			Game_Handle();
			
			DrawScreens();
			
			if(hidKeysDown() & KEY_Y)
			{
				PNGScreenshot_Top(); // AFTER DRAWING SCREENS!!
				//PNGScreenshot_Bottom();
				Sound_ResetHandler();
			}
			
			gfxFlushBuffers();
			gfxSwapBuffers();
			
			Timing_EndFrame(0);
			
			gspWaitForVBlank();
		}
		
		Game_End();
		
		Sound_Stop();
		Sound_End();
		
		Thread_End();
	}
	
	gfxExit();
	aptExit();
	return 0;
}


	
