// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#ifndef TTF_CONSOLE_H__
#define TTF_CONSOLE_H__

#include <stdarg.h>

#include <3ds.h>

void Con_Print(u8 *buf, int x, int y, const char *format, ...);

#endif // TTF_CONSOLE_H__
