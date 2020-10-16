// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#ifndef S3D_ENGINE_H__
#define S3D_ENGINE_H__

// ----------------------------------------------------------------------------
// -                      Software 3D engine. Main file.                      -
// ----------------------------------------------------------------------------

#include "draw.h"
#include "fxmath.h"
#include "polygon.h"

void S3D_ProjectionMatrixSet(int screen, m44 *m);

void S3D_ModelviewMatrixSet(int screen, m44 *m);
void S3D_ModelviewMatrixMultiply(int screen, m44 *m);
void S3D_ModelviewMatrixPush(int screen);
void S3D_ModelviewMatrixPop(int screen);

#endif // S3D_ENGINE_H__
