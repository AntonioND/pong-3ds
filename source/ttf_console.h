// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#ifndef __TTF_CONSOLE__
#define __TTF_CONSOLE__

#include <3ds.h>

#include <stdarg.h>

void Con_Print(u8 * buf, int x, int y, const char * format, ...);

#endif // __TTF_CONSOLE__
