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

#include <string.h>

#include <xmp.h>

#include <3ds.h>

#include "sound.h"

//-------------------------------------------------------------------------------------------------------

#define SOUND_VOLUME_MAX (0x8000) // doesn't affect PCM samples, only IMA-ADPCM

//-------------------------------------------------------------------------------------------------------

#define SAMPLERATE 44000
#define BUFFERSIZE ((((SAMPLERATE / 60) + 3) & ~3) * 2 * 6)

//-------------------------------------------------------------------------------------------------------

static xmp_context ctx;
static u64 sound_tick, now;
static u8 * audioBuffer, * lastPosition, * bufferEnd;
static u64 ticks_per_sample;
static int song_channel = -1;

static int module_initialised = 0;
static int sound_inited = 0;
static int sound_paused = 0;

//-------------------------------------------------------------------------------------------------------

#define MAX_SFX (20)

typedef struct {
    u8 * ptr;
    u32 size;
    int ref;
} _sfx_data_s;

static _sfx_data_s SFX[MAX_SFX];

int Sound_LoadSfx(int ref, const void * sfx_data, const unsigned int sfx_size)
{
    if(sound_inited == 0) return 1;

    int i;
    for(i = 0; i < MAX_SFX; i++)
    {
        if(SFX[i].ptr == NULL)
            break;
    }

    if(i == MAX_SFX) return 2; // too many sfx

    u8 * b = (u8*)linearAlloc(sfx_size);
    if(b == NULL) return 3; // couldn't allocate

    SFX[i].ptr = b;
    SFX[i].size = sfx_size;
    SFX[i].ref = ref;

    memcpy(b,sfx_data,sfx_size);
    GSPGPU_FlushDataCache(b,sfx_size);

    return 0;
}

int Sound_PlaySfx(int ref)
{
    if(sound_inited == 0) return 1;

    int chn = -1;

    int i;
    for(i = 0; i < 32; i++) if(csndChannels & BIT(i)) if(i != song_channel)
    {
        u8 playing;
        if(csndIsPlaying(i,&playing) == 0)
        {
            if(playing == 0)
            {
                chn = i;
                break;
            }
        }
    }

    if(chn == -1) return 2; // not enough channels...

    for(i = 0; i < MAX_SFX; i++)
    {
        if(SFX[i].ref == ref)
            break;
    }

    if(i == MAX_SFX) return 3; // couldn't find reference

    csndPlaySound(chn,SOUND_FORMAT_16BIT | SOUND_ONE_SHOT, SAMPLERATE,
                      1.0, 0.0,
                (u32*)SFX[i].ptr, (u32*)SFX[i].ptr, SFX[i].size);

    return 0;
}

//-------------------------------------------------------------------------------------------------------

static inline u32 _CSND_convertsamplerate(u32 samplerate)
{
    return (u32)(6.7027964E+07f / ((float)samplerate));
}

void Sound_Init(void)
{
    if(sound_inited) return;

    if(csndInit()) // error
        return;

    audioBuffer = (u8*)linearAlloc(BUFFERSIZE);
    if(audioBuffer == NULL)
    {
        csndExit();
        return;
    }
    memset(audioBuffer,0,BUFFERSIZE);
    GSPGPU_FlushDataCache(audioBuffer,BUFFERSIZE);

    memset(SFX,0,sizeof(SFX));

    ticks_per_sample = _CSND_convertsamplerate(SAMPLERATE) * 4;

    ctx = xmp_create_context();
    if(ctx)
    {
        sound_inited = 1;
    }
    else
    {
        linearFree(audioBuffer);
        csndExit();
    }

    song_channel = -1;

    sound_paused = 0;
}

void Sound_Play(const void * song_data, const unsigned int song_size)
{
    if(sound_inited == 0) return;

    if(module_initialised) Sound_Stop();

    if(xmp_load_module_from_memory(ctx,(void*)song_data,song_size) == 0)
    {
        if(xmp_start_player(ctx, SAMPLERATE, XMP_FORMAT_MONO) == 0)
        {
            module_initialised = 1;

            xmp_play_buffer(ctx,audioBuffer,BUFFERSIZE,0);

            memset(audioBuffer,0,BUFFERSIZE);
            GSPGPU_FlushDataCache(audioBuffer,BUFFERSIZE);

            lastPosition = audioBuffer;
            bufferEnd = audioBuffer + BUFFERSIZE;

            int i;
            for(i = 0; i < 32; i++) if(csndChannels & BIT(i))
            {
                song_channel = i;
                break;
            }

            csndPlaySound(song_channel,SOUND_FORMAT_16BIT | SOUND_REPEAT, SAMPLERATE,
                      1.0, 0.0,
                (u32*)audioBuffer, (u32*)audioBuffer, BUFFERSIZE);

            sound_tick = svcGetSystemTick();

            CSND_SetVol(song_channel, SOUND_VOLUME_MAX,SOUND_VOLUME_MAX);

            sound_paused = 0;
        }
        else
        {
            xmp_release_module(ctx);
        }
    }
}

void Sound_Resume(void)
{
    if(sound_inited == 0) return;

    if(module_initialised == 0) return;

    memset(audioBuffer,0,BUFFERSIZE);
    GSPGPU_FlushDataCache(audioBuffer,BUFFERSIZE);

    lastPosition = audioBuffer;

    csndPlaySound(song_channel,SOUND_FORMAT_16BIT | SOUND_REPEAT, SAMPLERATE,
                      1.0, 0.0,
                (u32*)audioBuffer, (u32*)audioBuffer, BUFFERSIZE);

    sound_tick = svcGetSystemTick();

    sound_paused = 0;
}

void Sound_Pause(void)
{
    sound_paused = 1;

    memset(audioBuffer,0,BUFFERSIZE);
    GSPGPU_FlushDataCache(audioBuffer,BUFFERSIZE);
}

void Sound_Handle(void)
{
    if(sound_inited == 0) return;

    if(module_initialised == 0) return;

    if(sound_paused) return;

    now = svcGetSystemTick();

    s64 elapsed = now - sound_tick;

    u64 fill_bytes = 2 * (elapsed / ticks_per_sample);

    sound_tick = now - (elapsed % ticks_per_sample);

    if( (lastPosition + fill_bytes) > bufferEnd )
    {
        int size = bufferEnd - lastPosition;
        xmp_play_buffer(ctx,lastPosition,size,0);
        GSPGPU_FlushDataCache(lastPosition,size);
        lastPosition = audioBuffer;
        fill_bytes -= size;
    }

    if(fill_bytes)
    {
        xmp_play_buffer(ctx,lastPosition,fill_bytes,0);
        GSPGPU_FlushDataCache(lastPosition,fill_bytes);
        lastPosition += fill_bytes;
        if (lastPosition == bufferEnd) lastPosition = audioBuffer;
    }
}

void Sound_Stop(void)
{
    if(sound_inited == 0) return;

    if(module_initialised == 0) return;

    CSND_SetVol(song_channel, 0,0);

    memset(audioBuffer,0,BUFFERSIZE);
    GSPGPU_FlushDataCache(audioBuffer,BUFFERSIZE);

    xmp_end_player(ctx);

    xmp_release_module(ctx);

    module_initialised = 0;

    song_channel = -1;
}

void Sound_End(void)
{
    if(sound_inited == 0) return;

    if(module_initialised) Sound_Stop();

    linearFree(audioBuffer);

    int i;
    for(i = 0; i < MAX_SFX; i++) // free SFX
    {
        if(SFX[i].ptr) linearFree(SFX[i].ptr);
    }

    xmp_free_context(ctx);

    for(i = 0; i < 32; i++) // Mute all channels
        if(csndChannels & BIT(i))
            CSND_SetVol(i, 0,0);

    csndExit();

    sound_inited = 0;
}
