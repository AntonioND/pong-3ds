// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#ifndef __SOUND__
#define __SOUND__

void Sound_Init(void);
void Sound_Play(const void * song_data, const unsigned int song_size);
void Sound_Pause(void);
void Sound_Resume(void);
void Sound_Handle(void);
void Sound_Stop(void);
void Sound_End(void);

//------------------------------------------------------------------------------

int Sound_LoadSfx(int ref, const void * sfx_data, const unsigned int sfx_size); // returns 0 if OK
int Sound_PlaySfx(int ref); // volume: 0 ~ 0xFFFF. returns 0 if OK
// For some reason the CSND calls in Sound_PlaySfx() are really slow...

#endif //__SOUND__
