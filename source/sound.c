
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

#include <xmp.h>

#include <3ds.h>

//-------------------------------------------------------------------------------------------------------

#define SAMPLERATE 44000
#define BUFFERSIZE ((((SAMPLERATE / 60) + 3) & ~3) * 2 * 6)

//-------------------------------------------------------------------------------------------------------

static xmp_context ctx;
static struct xmp_module_info mi;
static struct xmp_frame_info fi;
static u64 sound_tick = 0, now;
static u8 * audioBuffer;
static u8 * lastPosition, * bufferEnd;
static u64 ticks_per_sample;
static int module_initialised = 0, module_loaded = 0;
static int channel = -1;

//-------------------------------------------------------------------------------------------------------

static inline u32 CSND_convertsamplerate(u32 samplerate)
{
	return (u32)(6.7027964E+07f / ((float)samplerate));
}

void Sound_Init(void)
{
	csndInit();
	
	audioBuffer = (u8*)linearAlloc(BUFFERSIZE);

    lastPosition = NULL, bufferEnd = NULL;

    ticks_per_sample = CSND_convertsamplerate(SAMPLERATE) * 4;

    ctx = xmp_create_context();
}

#include "song_xm_bin.h"

void Sound_Play(void)
{
	module_initialised = 0;
	module_loaded = 0;
	
	if(xmp_load_module_from_memory(ctx,(void*)song_xm_bin,song_xm_bin_size) == 0)
	{
		module_loaded = 1;
		
		if(xmp_start_player(ctx, SAMPLERATE, XMP_FORMAT_MONO ) == 0)
        {
			module_initialised = 1;

			xmp_get_module_info(ctx, &mi);

			xmp_play_buffer(ctx,audioBuffer,BUFFERSIZE,0);

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
				(u32*)audioBuffer, (u32*)audioBuffer,
				BUFFERSIZE);

			sound_tick = svcGetSystemTick();
		}
	}
}

void Sound_Handle(void)
{
	if(module_initialised)
	{
		now = svcGetSystemTick();

		s64 elapsed = now - sound_tick;

		u64 fill_bytes = 2 * (elapsed / ticks_per_sample);

		sound_tick = now - (elapsed % ticks_per_sample);

		if (lastPosition + fill_bytes > bufferEnd)
		{
			int size = bufferEnd - lastPosition;
			xmp_play_buffer(ctx,lastPosition,size,0);
			GSPGPU_FlushDataCache(NULL,lastPosition,size);
			lastPosition = audioBuffer;
			fill_bytes -= size;
		}

		if (fill_bytes)
		{
			xmp_play_buffer(ctx,lastPosition,fill_bytes,0);
			GSPGPU_FlushDataCache(NULL,lastPosition,fill_bytes);
			lastPosition += fill_bytes;
			if (lastPosition == bufferEnd) lastPosition = audioBuffer;
		}

		xmp_get_frame_info(ctx, &fi);
	}
}

void Sound_Stop(void)
{
	if(module_loaded)
	{
		xmp_release_module(ctx);
		module_loaded = 0;
		
		if(module_initialised)
		{
			module_initialised = 0;
			
			CSND_SetVol(channel, 0,0);
			CSND_SetPlayState(channel,0);
			xmp_end_player(ctx);
		}
	}
}

void Sound_End(void)
{
	linearFree(audioBuffer);
	
	xmp_free_context(ctx);

    csndExit();
}

//-------------------------------------------------------------------------------------------------------
