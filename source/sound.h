// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#ifndef SOUND_H__
#define SOUND_H__

void Sound_Init(void);
void Sound_Play(const void *song_data, const unsigned int song_size);
void Sound_Pause(void);
void Sound_Resume(void);
void Sound_Handle(void);
void Sound_Stop(void);
void Sound_End(void);

// -----------------------------------------------------------------------------

// Returns 0 if OK
int Sound_LoadSfx(int ref, const void *sfx_data, const unsigned int sfx_size);

// Volume: 0 ~ 0xFFFF. returns 0 if OK
// For some reason the CSND calls in Sound_PlaySfx() are really slow...
int Sound_PlaySfx(int ref);

#endif // SOUND_H__
