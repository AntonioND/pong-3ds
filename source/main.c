
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
	S3D_BufferSetScreen(GFX_LEFT);
	S3D_ProjectionMatrixSet(&left_screen);
	S3D_PolygonListClear();
	Game_DrawScene();
	S3D_PolygonListFlush(1);
	
	float slider = CONFIG_3D_SLIDERSTATE;
	if(slider == 0.0f) return;

	S3D_BufferSetScreen(GFX_RIGHT);
	S3D_ProjectionMatrixSet(&right_screen);
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

		Con_Print(buf,0,40,"A: Start.");
		Con_Print(buf,0,20,"SELECT: Screenshot.");
		Con_Print(buf,0,0,"START:  Exit.");
		
		gfxFlushBuffers();
		gfxSwapBuffers();
		
		if(keys & KEY_SELECT) Screenshot_PNG(); // AFTER DRAWING SCREENS!!

		gspWaitForVBlank();
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
		
		u8 * buf = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
		Con_Print(buf,0,170,"3D Slider: %f        ",CONFIG_3D_SLIDERSTATE);
		Con_Print(buf,0,150,"FPS: %d  ",Timing_GetFPS());
		Con_Print(buf,0,130,"CPU: %d%% ",(int)Timing_GetCPUUsage());
		
		ProjectionMatricesConfigure();
		DrawScreens();
		
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


	
