
//-------------------------------------------------------------------------------------------------------

#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "engine.h"
#include "utils.h"
#include "game.h"

//-------------------------------------------------------------------------------------------------------

#define CONFIG_3D_SLIDERSTATE (*(float*)0x1FF81080) //this should be in ctrulib...

//-------------------------------------------------------------------------------------------------------

// Far is > 2.5 z
// Screen is at 2.5 z
// Over screen is < 2.5 z

#define TRANSL_DIV_FACTOR (4.0f)
#define SHEAR_DIV_FACTOR  (2.0f)

static void DrawLeft(void)
{
	float slider = CONFIG_3D_SLIDERSTATE;
	int transl = float2fx(slider/TRANSL_DIV_FACTOR);
	int shear = float2fx(slider/SHEAR_DIV_FACTOR);
	
	m44 p, t, r;
	m44_CreateFrustum(&p, -float2fx(2.5)+shear, float2fx(2.5)+shear, -float2fx(1.5), float2fx(1.5), float2fx(5), float2fx(10));
	m44_CreateTranslation(&t,-transl,0,0);
	m44_Multiply(&p,&t,&r);
	S3D_ProyectionMatrixSet(&r);
	
	S3D_BufferSetScreen(GFX_LEFT);
	S3D_PolygonListClear();
	
	Game_DrawScene();
	
	S3D_PolygonListFlush(1);
}

static void DrawRight(void)
{
	float slider = CONFIG_3D_SLIDERSTATE;
	if(slider == 0.0f) return;
	int transl = float2fx(slider/TRANSL_DIV_FACTOR);
	int shear = float2fx(slider/SHEAR_DIV_FACTOR);	

	m44 p, t, r;
	m44_CreateFrustum(&p, -float2fx(2.5)-shear, float2fx(2.5)-shear, -float2fx(1.5), float2fx(1.5), float2fx(5), float2fx(10));
	m44_CreateTranslation(&t,+transl,0,0);
	m44_Multiply(&p,&t,&r);
	S3D_ProyectionMatrixSet(&r);
	
	S3D_BufferSetScreen(GFX_RIGHT);
	S3D_PolygonListClear();
	
	Game_DrawScene();
	
	S3D_PolygonListFlush(1);
}

//-------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	aptInit();
	gfxInitDefault();
	gfxSet3D(true); //OMG 3D!!!!1!!!
	consoleInit(GFX_BOTTOM, NULL); //Initialize console on bottom screen.
	
	//To move the cursor you have tu print "\x1b[r;cH", where r and c are respectively
	//the row and column where you want your cursor to move
	//The top screen has 30 rows and 50 columns
	//The bottom screen has 30 rows and 40 columns
	printf("\x1b[0;16HPong 3DS");
	printf("\x1b[2;14Hby AntonioND");
	printf("\x1b[3;10H(Antonio Ni%co Diaz)",165);
	printf("\x1b[28;5HSELECT: Screenshot.");
	printf("\x1b[29;5HSTART:  Exit.");
	
	//Try to get as much as CPU time as possible
	{
		aptOpenSession();
		u32 i, percent;
		for(i = 100; i > 1; i--)
		{
			APT_SetAppCpuTimeLimit(NULL,i);
			APT_GetAppCpuTimeLimit(NULL, &percent);
			if(i == percent) break;
		}
		aptCloseSession();
		printf("\x1b[15;5HCPU limit: %d%%",(int)percent);
	}
	
	fast_srand(svcGetSystemTick());
	
	Game_Init();
	
	Timing_Start();
	
	// Main loop
	while(aptMainLoop())
	{
		Timing_StartFrame();
		
		hidScanInput();
		int keys = hidKeysHeld();
		if(keys & KEY_START) break; // break in order to return to hbmenu
		
		S3D_FramebuffersClearTopScreen(0,0,0);
		
		Game_HandleAndDraw(keys);
		
		printf("\x1b[8;5H3D Slider: %f        ",CONFIG_3D_SLIDERSTATE);
		printf("\x1b[10;5HFPS: %d  ",Timing_GetFPS());
		printf("\x1b[11;5HCPU: %.2f%%   ",Timing_GetCPUUsage());
		
		DrawLeft();
		DrawRight();
		
		gfxFlushBuffers();
		gfxSwapBuffers();
		
		if(keys & KEY_SELECT) Screenshot_PNG(); // AFTER DRAWING SCREENS!!
		
		Timing_EndFrame();
		
		gspWaitForVBlank();
	}
	
	Game_End();

	gfxExit();
	aptExit();
	return 0;
}


	
