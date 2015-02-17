
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

#ifndef __SOUND__
#define __SOUND__

//--------------------------------------------------------------------------------------------------

void Sound_Init(void);
void Sound_Play(const void * song_data, const unsigned int song_size);
void Sound_ResetHandler(void); // call this when game logic has needed too much time to finish a frame
void Sound_Handle(void);
void Sound_Stop(void);
void Sound_End(void);

//--------------------------------------------------------------------------------------------------

int Sound_LoadSfx(int ref, const void * sfx_data, const unsigned int sfx_size); // returns 0 if OK
int Sound_PlaySfx(int ref); // volume: 0 ~ 0xFFFF. returns 0 if OK

//--------------------------------------------------------------------------------------------------

#endif //__SOUND__
