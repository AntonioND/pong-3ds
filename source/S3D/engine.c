// SPDX-License-Identifier: GPL-3.0-or-later
//
// Copyright (C) 2015, 2020 Antonio Niño Díaz
//
// Pong 3DS. Just a pong for the Nintendo 3DS.

#include "engine.h"
#include "private.h"

// -----------------------------------------------------------------------------

// Global state for both screens. Only a few values need to be initialized, the
// others start as 0.

s3d_state S3D[2] = {
    {
        .currmode = S3D_TRIANGLES,
        .draw_front = 1,
        .saved_r = 255,
        .saved_g = 255,
        .saved_b = 255,
        .currcolor_alpha = 255,
    },
    {
        .currmode = S3D_TRIANGLES,
        .draw_front = 1,
        .saved_r = 255,
        .saved_g = 255,
        .saved_b = 255,
        .currcolor_alpha = 255,
    }
};

// -----------------------------------------------------------------------------

void S3D_ProjectionMatrixSet(int screen, m44 *m)
{
    s3d_state *st = &S3D[screen];

    m44_Copy(m, &(st->PROJECTION));
    st->global_updated = 0;
}

// -----------------------------------------------------------------------------

void S3D_ModelviewMatrixSet(int screen, m44 *m)
{
    s3d_state *st = &S3D[screen];

    m44_Copy(m, &(st->MODELVIEW));
    st->global_updated = 0;
}

void S3D_ModelviewMatrixPush(int screen)
{
    s3d_state *st = &S3D[screen];

    // if (st->matrix_stack_ptr >= MATRIX_STACK_SIZE)
    //     while(1);

    m44_Copy(&(st->MODELVIEW),
             &(st->matrix_stack[st->matrix_stack_ptr]));
    st->matrix_stack_ptr++;
}

void S3D_ModelviewMatrixPop(int screen)
{
    s3d_state *st = &S3D[screen];

    // if (st->matrix_stack_ptr < 0)
    //     while(1);

    st->matrix_stack_ptr--;
    m44_Copy(&(st->matrix_stack[st->matrix_stack_ptr]),
             &(st->MODELVIEW));

    st->global_updated = 0;
}

void S3D_ModelviewMatrixMultiply(int screen, m44 *m)
{
    s3d_state *st = &S3D[screen];

    m44 temp;
    m44_Copy(&(st->MODELVIEW), &temp);
    m44_Multiply(&temp, m, &(st->MODELVIEW));

    st->global_updated = 0;
}

// -----------------------------------------------------------------------------

void _s3d_global_matrix_update(int screen)
{
    s3d_state *st = &S3D[screen];

    if (st->global_updated)
        return;

    st->global_updated = 1;

    m44_Multiply(&(st->PROJECTION), &(st->MODELVIEW),
                 &(st->GLOBAL_MATRIX));
}
