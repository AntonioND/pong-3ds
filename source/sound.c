
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

#include <string.h>

#include <xmp.h>

#include <3ds.h>

#include "sound.h"

//-------------------------------------------------------------------------------------------------------

#define SAMPLERATE 44000
#define BUFFERSIZE ((((SAMPLERATE / 60) + 3) & ~3) * 2 * 6)

//-------------------------------------------------------------------------------------------------------

static xmp_context ctx;
static u64 sound_tick, now;
static u8 * audioBuffer, * lastPosition, * bufferEnd;
static u64 ticks_per_sample;
static int channel = -1;

static int module_initialised = 0;
static int sound_inited = 0;

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
	GSPGPU_FlushDataCache(NULL,audioBuffer,BUFFERSIZE);
	
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
			GSPGPU_FlushDataCache(NULL,audioBuffer,BUFFERSIZE);

			lastPosition = audioBuffer;
			bufferEnd = audioBuffer + BUFFERSIZE;

			int i;
			for(i = 0; i < 32; i++) if(csndChannels & BIT(i))
			{
				channel = i;
				break;
			}

			csndPlaySound(channel,SOUND_FORMAT_16BIT | SOUND_REPEAT, SAMPLERATE,
				(u32*)audioBuffer, (u32*)audioBuffer, BUFFERSIZE);

			sound_tick = svcGetSystemTick();

			CSND_SetVol(channel, 0xFFFF,0xFFFF);
		}
		else
		{
			xmp_release_module(ctx);
		}
	}
}

void Sound_ResetHandler(void)
{
	if(sound_inited == 0) return;
	
	if(module_initialised == 0) return;
	
	memset(audioBuffer,0,BUFFERSIZE);
	GSPGPU_FlushDataCache(NULL,audioBuffer,BUFFERSIZE);
	
	lastPosition = audioBuffer;
	
	csndPlaySound(channel,SOUND_FORMAT_16BIT | SOUND_REPEAT, SAMPLERATE,
				(u32*)audioBuffer, (u32*)audioBuffer, BUFFERSIZE);
	
	sound_tick = svcGetSystemTick();
}

void Sound_Handle(void)
{
	if(sound_inited == 0) return;
	
	if(module_initialised == 0) return;

	now = svcGetSystemTick();

	s64 elapsed = now - sound_tick;

	u64 fill_bytes = 2 * (elapsed / ticks_per_sample);

	sound_tick = now - (elapsed % ticks_per_sample);

	if( (lastPosition + fill_bytes) > bufferEnd )
	{
		int size = bufferEnd - lastPosition;
		xmp_play_buffer(ctx,lastPosition,size,0);
		GSPGPU_FlushDataCache(NULL,lastPosition,size);
		lastPosition = audioBuffer;
		fill_bytes -= size;
	}

	if(fill_bytes)
	{
		xmp_play_buffer(ctx,lastPosition,fill_bytes,0);
		GSPGPU_FlushDataCache(NULL,lastPosition,fill_bytes);
		lastPosition += fill_bytes;
		if (lastPosition == bufferEnd) lastPosition = audioBuffer;
	}
}

void Sound_Stop(void)
{
	if(sound_inited == 0) return;
	
	if(module_initialised == 0) return;
	
	CSND_SetVol(channel, 0,0);
	
	memset(audioBuffer,0,BUFFERSIZE);
	GSPGPU_FlushDataCache(NULL,audioBuffer,BUFFERSIZE);
	
	xmp_end_player(ctx);
	
	xmp_release_module(ctx);
	
	module_initialised = 0;
}

void Sound_End(void)
{
	if(sound_inited == 0) return;
	
	if(module_initialised) Sound_Stop();
	
	linearFree(audioBuffer);
	
	xmp_free_context(ctx);

    csndExit();
	
	sound_inited = 0;
}

//-------------------------------------------------------------------------------------------------------
