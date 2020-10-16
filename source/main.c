// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "S3D/engine.h"
#include "game.h"
#include "sound.h"
#include "ttf_console.h"
#include "utils.h"

// -----------------------------------------------------------------------------

static int SecondaryThreadExit = 0;

static Thread SecondaryThreadHandle;

static Handle MutexThreadDrawing, MutexSyncFrame;

// -----------------------------------------------------------------------------

static m44 left_screen, right_screen;

static void ProjectionMatricesConfigure(void)
{
    // Far is > 2.5 z
    // Screen is at 2.5 z
    // Over screen is < 2.5 z

#define TRANSL_DIV_FACTOR (8.0f)
#define SHEAR_DIV_FACTOR (4.0f)

    m44 p, t;

    float slider = osGet3DSliderState();
    int transl = float2fx(slider / TRANSL_DIV_FACTOR);
    int shear = float2fx(slider / SHEAR_DIV_FACTOR);

    m44_CreateFrustum(&p, -float2fx(2.5) + shear, float2fx(2.5) + shear,
                      -float2fx(1.5), float2fx(1.5), float2fx(5), float2fx(10));
    m44_CreateTranslation(&t, +transl, 0, 0);
    m44_Multiply(&p, &t, &left_screen);

    if (slider == 0.0f)
        return;

    m44_CreateFrustum(&p, -float2fx(2.5) - shear, float2fx(2.5) - shear,
                      -float2fx(1.5), float2fx(1.5), float2fx(5), float2fx(10));
    m44_CreateTranslation(&t, -transl, 0, 0);
    m44_Multiply(&p, &t, &right_screen);
}

static void DrawScreens(void)
{
    S3D_BuffersSetup();

    // ----------------------------------------

    ProjectionMatricesConfigure();

    // ----------------------------------------

    // Don't wait forever in main() thread
    svcWaitSynchronization(MutexSyncFrame, U64_MAX);
    svcReleaseMutex(MutexThreadDrawing);

    // ----------------------------------------

    int screen = 0;
    S3D_ProjectionMatrixSet(screen, &left_screen);
    S3D_PolygonListClear(screen);
    Game_DrawScreenTop(screen);

    // ----------------------------------------

    // Draw bottom screen while the other thread finishes its work.
    Game_DrawScreenBottom();

    // ----------------------------------------

    // Don't wait forever in main() thread
    svcWaitSynchronization(MutexThreadDrawing, U64_MAX);
    svcReleaseMutex(MutexSyncFrame);
}

// -----------------------------------------------------------------------------

void SecondaryThreadFunction(u32 arg)
{
    Timing_Start(1);

    while (SecondaryThreadExit == 0)
    {
        while (SecondaryThreadExit == 0)
        {
            if (svcWaitSynchronization(MutexThreadDrawing, U64_MAX) == 0)
                break;
        }

        if (SecondaryThreadExit)
            break;

        // ----------------------------------------

        Timing_StartFrame(1);

        float slider = osGet3DSliderState();
        if (slider > 0.0f)
        {
            int screen = 1;
            S3D_ProjectionMatrixSet(screen, &right_screen);
            S3D_PolygonListClear(screen);
            Game_DrawScreenTop(screen);
        }

        Sound_Handle();

        Timing_EndFrame(1);

        // ----------------------------------------

        svcReleaseMutex(MutexThreadDrawing);

        while (SecondaryThreadExit == 0)
        {
            if (svcWaitSynchronization(MutexSyncFrame, U64_MAX) == 0)
                break;
        }

        svcReleaseMutex(MutexSyncFrame);
    }

    svcExitThread();
}

// Start secondary thread in a secondary CPU (CPU1 in Old 3DS, CPU2 in New 3DS)
int Thread_Init(void)
{
    void *arg = NULL;
    size_t stack_size = 0x2000;

    // The priority of child threads must be higher (aka the value is lower)
    // than that of the main thread, otherwise there is thread starvation due to
    // stdio being locked.
    s32 prio = 0;
    svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);

    // For the secondary thread use CPU1 in Old 3DS and CPU2 in New 3DS.
    int affinity;
    bool is_new_3ds = false;
    APT_CheckNew3DS(&is_new_3ds);
    if (is_new_3ds)
    {
        affinity = 2;
    }
    else
    {
        affinity = 1;
    }

    SecondaryThreadHandle = threadCreate((ThreadFunc)SecondaryThreadFunction,
                                         arg, stack_size, prio - 1, affinity,
                                         false);

    // TODO: Check that the CPU ID is the right one with svcGetProcessorID()?

    uint32_t val;

    val = svcCreateMutex(&MutexThreadDrawing, true); // Initially locked
    if (val)
    {
        return 1;
    }

    val = svcCreateMutex(&MutexSyncFrame, false); // Initially released
    if (val)
    {
        svcCloseHandle(MutexThreadDrawing);
        return 1;
    }

    return 0;
}

void Thread_End(void)
{
    SecondaryThreadExit = 1;

    svcReleaseMutex(MutexThreadDrawing);
    svcReleaseMutex(MutexSyncFrame);

    // This will hang the CPU if the secondary thread can't exit, but I prefer
    // the game to hang than returning to the loader with the secondary thread
    // running in the background.
    threadJoin(SecondaryThreadHandle, U64_MAX);
    threadFree(SecondaryThreadHandle);

    svcCloseHandle(MutexThreadDrawing);
    svcCloseHandle(MutexSyncFrame);
}

// -----------------------------------------------------------------------------

void SetMaxCpuTimeLimit(void)
{
    // Try to get as much as CPU time as possible (?)

    APT_SetAppCpuTimeLimit(80);
    // u32 i, percent;
    // for (i = 100; i > 1; i--)
    // {
    //     APT_SetAppCpuTimeLimit(NULL, i);
    //     APT_GetAppCpuTimeLimit(NULL, &percent);
    //     if (i == percent)
    //         break;
    // }
}

// -----------------------------------------------------------------------------

int main(int argc, char **argv) // Running in CPU 0
{
    gfxInitDefault();
    gfxSet3D(true); // Enable stereoscopic 3D

    SetMaxCpuTimeLimit();

    fast_srand(svcGetSystemTick());

    if (Thread_Init() == 0)
    {
        Sound_Init();

        Game_Init();

        Timing_Start(0);

        // Main loop
        while (aptMainLoop())
        {
            Timing_StartFrame(0);

            hidScanInput(); // Once per frame

            if (hidKeysHeld() & KEY_SELECT)
                break; // Break in order to return to hbmenu

            Game_Handle();

            DrawScreens();

            if (hidKeysDown() & KEY_Y)
            {
                Sound_Pause();
                PNGScreenshot_Top(); // Do this after drawing screens!
                //PNGScreenshot_Bottom();
                Sound_Resume();
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

    return 0;
}
