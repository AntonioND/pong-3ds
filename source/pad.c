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
#include <limits.h>

#include <3ds.h>

#include "S3D/engine.h"
#include "game.h"
#include "pad.h"
#include "ball.h"
#include "rooms.h"
#include "sound.h"

//--------------------------------------------------------------------------------------------------

typedef struct { // values in fixed point!
    s32 sx,sy,sz; // size

    s32 x,vx,ax; // coordinate, speed, acceleration
    s32 y,vy,ay; // start coordinates are (0.0, 0.0, Z)
    s32 z,vz,az;

    int r,g,b,a;

    u32 collisions;
} _pad_t;

static _pad_t PAD1, PAD2;

//--------------------------------------------------------------------------------------------------

void Pad_P1SetDimensions(int x, int y, int z) // Player
{
    _pad_t * p = &PAD1;
    p->sx = x;
    p->sy = y;
    p->sz = z;
}

void Pad_P2SetDimensions(int x, int y, int z) // AI
{
    _pad_t * p = &PAD2;
    p->sx = x;
    p->sy = y;
    p->sz = z;
}

//--------------------------------------------------------------------------------------------------

void Pad_P1SetColor(int r, int g, int b, int a) // Player
{
    _pad_t * p = &PAD1;
    p->r = r;
    p->g = g;
    p->b = b;
    p->a = a;
}

void Pad_P2SetColor(int r, int g, int b, int a) // AI
{
    _pad_t * p = &PAD2;
    p->r = r;
    p->g = g;
    p->b = b;
    p->a = a;
}

//--------------------------------------------------------------------------------------------------

static inline void _pad_DrawShadows(int screen, _pad_t * p)
{
    if(Room_3DMode() == GAME_MODE_3D) // 4 shadows
    {
        int xmin = p->x - (p->sx/2);
        int xmax = p->x + (p->sx/2);
        int ymin = p->y - (p->sy/2);
        int ymax = p->y + (p->sy/2);
        int zmin = p->z - (p->sz/2);
        int zmax = p->z + (p->sz/2);

        int roomxmin, roomxmax, roomymin, roomymax, roomzmin, roomzmax;
        Room_GetBounds(&roomxmin,&roomxmax,&roomymin,&roomymax,&roomzmin,&roomzmax);
        if(zmin > roomzmax) return;
        if(zmax < roomzmin) return;

        if(zmax > roomzmax) zmax = roomzmax;
        if(zmin < roomzmin) zmin = roomzmin;

        S3D_PolygonColorAlpha(screen, 0,0,0, 128);

        S3D_PolygonBegin(screen, S3D_QUADS);

        S3D_PolygonNormal(screen, float2fx(0.0),float2fx(-1.0),float2fx(0.0));

        S3D_PolygonVertex(screen, xmax,roomymax,zmin);
        S3D_PolygonVertex(screen, xmax,roomymax,zmax);
        S3D_PolygonVertex(screen, xmin,roomymax,zmax);
        S3D_PolygonVertex(screen, xmin,roomymax,zmin);

        S3D_PolygonNormal(screen, float2fx(-1.0),float2fx(0.0),float2fx(0.0));

        S3D_PolygonVertex(screen, roomxmax,ymin,zmin);
        S3D_PolygonVertex(screen, roomxmax,ymin,zmax);
        S3D_PolygonVertex(screen, roomxmax,ymax,zmax);
        S3D_PolygonVertex(screen, roomxmax,ymax,zmin);

        S3D_PolygonNormal(screen, float2fx(1.0),float2fx(0.0),float2fx(0.0));

        S3D_PolygonVertex(screen, roomxmin,ymin,zmin);
        S3D_PolygonVertex(screen, roomxmin,ymax,zmin);
        S3D_PolygonVertex(screen, roomxmin,ymax,zmax);
        S3D_PolygonVertex(screen, roomxmin,ymin,zmax);

        S3D_PolygonNormal(screen, float2fx(0.0),float2fx(1.0),float2fx(0.0));

        S3D_PolygonVertex(screen, xmin,roomymin,zmin);
        S3D_PolygonVertex(screen, xmin,roomymin,zmax);
        S3D_PolygonVertex(screen, xmax,roomymin,zmax);
        S3D_PolygonVertex(screen, xmax,roomymin,zmin);

        S3D_PolygonListFlush(screen, 0);
    }
    else // 1 shadow
    {
        int xmin = p->x - (p->sx/2);
        int xmax = p->x + (p->sx/2);
        int zmin = p->z - (p->sz/2);
        int zmax = p->z + (p->sz/2);

        int roomymin, roomzmin, roomzmax;
        Room_GetBounds(NULL,NULL,&roomymin,NULL,&roomzmin,&roomzmax);
        if(zmin > roomzmax) return;
        if(zmax < roomzmin) return;

        if(zmax > roomzmax) zmax = roomzmax;
        if(zmin < roomzmin) zmin = roomzmin;

        S3D_PolygonColorAlpha(screen, 0,0,0, 128);

        S3D_PolygonBegin(screen, S3D_QUADS);

        S3D_PolygonNormal(screen, float2fx(0.0),float2fx(1.0),float2fx(0.0));

        S3D_PolygonVertex(screen, xmin,roomymin,zmin);
        S3D_PolygonVertex(screen, xmin,roomymin,zmax);
        S3D_PolygonVertex(screen, xmax,roomymin,zmax);
        S3D_PolygonVertex(screen, xmax,roomymin,zmin);

        S3D_PolygonListFlush(screen, 0);
    }
}

static inline void _pad_Draw(int screen, _pad_t * p)
{
    int xmin = p->x - (p->sx/2);
    int xmax = p->x + (p->sx/2);
    int ymin = p->y - (p->sy/2);
    int ymax = p->y + (p->sy/2);
    int zmin = p->z - (p->sz/2);
    int zmax = p->z + (p->sz/2);

    S3D_PolygonColorAlpha(screen, p->r,p->g,p->b,p->a);

    S3D_PolygonBegin(screen, S3D_QUAD_STRIP);

    S3D_PolygonNormal(screen, float2fx(0.0),float2fx(-1.0),float2fx(0.0));

    S3D_PolygonVertex(screen, xmax,ymin,zmin);
    S3D_PolygonVertex(screen, xmax,ymin,zmax);
    S3D_PolygonVertex(screen, xmin,ymin,zmax);
    S3D_PolygonVertex(screen, xmin,ymin,zmin);

    S3D_PolygonNormal(screen, float2fx(-1.0),float2fx(0.0),float2fx(0.0));

    S3D_PolygonVertex(screen, xmin,ymax,zmax);
    S3D_PolygonVertex(screen, xmin,ymax,zmin);

    S3D_PolygonNormal(screen, float2fx(0.0),float2fx(1.0),float2fx(0.0));

    S3D_PolygonVertex(screen, xmax,ymax,zmax);
    S3D_PolygonVertex(screen, xmax,ymax,zmin);

    S3D_PolygonBegin(screen, S3D_QUAD_STRIP);

    S3D_PolygonNormal(screen, float2fx(0.0),float2fx(0.0),float2fx(-1.0));

    S3D_PolygonVertex(screen, xmin,ymin,zmin);
    S3D_PolygonVertex(screen, xmin,ymax,zmin);
    S3D_PolygonVertex(screen, xmax,ymax,zmin);
    S3D_PolygonVertex(screen, xmax,ymin,zmin);

    S3D_PolygonNormal(screen, float2fx(1.0),float2fx(0.0),float2fx(0.0));

    S3D_PolygonVertex(screen, xmax,ymax,zmax);
    S3D_PolygonVertex(screen, xmax,ymin,zmax);

    S3D_PolygonNormal(screen, float2fx(0.0),float2fx(0.0),float2fx(1.0));

    S3D_PolygonVertex(screen, xmin,ymax,zmax);
    S3D_PolygonVertex(screen, xmin,ymin,zmax);
}

void Pad_P1Draw(int screen)
{
    _pad_Draw(screen,&PAD1);
}

void Pad_P2Draw(int screen)
{
    _pad_Draw(screen,&PAD2);
}

void Pad_P1DrawShadows(int screen)
{
    _pad_DrawShadows(screen,&PAD1);
}

void Pad_P2DrawShadows(int screen)
{
    _pad_DrawShadows(screen,&PAD2);
}

//--------------------------------------------------------------------------------------------------

static inline void _pad_GetBounds(_pad_t * p, int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax)
{
    if(xmin) *xmin = p->x - (p->sx/2);
    if(xmax) *xmax = p->x + (p->sx/2);
    if(ymin) *ymin = p->y - (p->sy/2);
    if(ymax) *ymax = p->y + (p->sy/2);
    if(zmin) *zmin = p->z - (p->sz/2);
    if(zmax) *zmax = p->z + (p->sz/2);
}

void Pad_P1GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax)
{
    _pad_GetBounds(&PAD1,xmin,xmax,ymin,ymax,zmin,zmax);
}

void Pad_P2GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax)
{
    _pad_GetBounds(&PAD2,xmin,xmax,ymin,ymax,zmin,zmax);
}

//--------------------------------------------------------------------------------------------------

void Pad_P1GetPosition(int * x, int * y, int * z)
{
    _pad_t * p = &PAD1;
    if(x) *x = p->x;
    if(y) *y = p->y;
    if(z) *z = p->z;
}

void Pad_P2GetPosition(int * x, int * y, int * z)
{
    _pad_t * p = &PAD2;
    if(x) *x = p->x;
    if(y) *y = p->y;
    if(z) *z = p->z;
}

//--------------------------------------------------------------------------------------------------

void Pad_InitAll(void)
{
    memset(&PAD1,0,sizeof(_pad_t));
    memset(&PAD2,0,sizeof(_pad_t));
}

void Pad_ResetAll(void)
{
    int roomxmin, roomxmax, roomymin, roomymax, roomzmin, roomzmax;
    Room_GetBounds(&roomxmin,&roomxmax,&roomymin,&roomymax,&roomzmin,&roomzmax);

    if(Room_3DMode() == GAME_MODE_3D)
    {
        PAD1.x = float2fx(0.0);
        PAD1.y = float2fx(0.0);
        PAD1.z = roomzmin + (PAD1.sz/2);

        PAD2.x = float2fx(0.0);
        PAD2.y = float2fx(0.0);
        PAD2.z = roomzmax - (PAD2.sz/2);
    }
    else
    {
        PAD1.x = float2fx(0.0);
        PAD1.y = roomymin + (PAD1.sy/2);
        PAD1.z = roomzmin + (PAD1.sz/2);

        PAD2.x = float2fx(0.0);
        PAD2.y = roomymin + (PAD2.sy/2);
        PAD2.z = roomzmax - (PAD2.sz/2);
    }
}

//--------------------------------------------------------------------------------------------------

void Pad_P1Bounce(int speed, int acc)
{
    if(PAD1.ay == 0)
    {
        PAD1.ay = acc;
        PAD1.vy = speed;
    }
}

void Pad_P2Bounce(int speed, int acc)
{
    if(PAD2.ay == 0)
    {
        PAD2.ay = acc;
        PAD2.vy = speed;
    }
}

//--------------------------------------------------------------------------------------------------

static inline int abs(int a)
{
    return a > 0 ? a : -a;
}

static inline int min(int a, int b)
{
    return a < b ? a : b;
}

static inline int clamp(int min, int val, int max)
{
    if(val < min) return min;
    if(val > max) return max;
    return val;
}

#define PAD_MAX_SEPARATION_RANGE (float2fx(3.5)) // separation from wall

// margins are only valid if there is a collision in that axis
static void _pad_UpdateCollisions(_pad_t * p, int pad_number, int * xmargin, int * ymargin, int * zmargin)
{
    p->collisions = 0;

    int roomxmin, roomxmax, roomymin, roomymax, roomzmin, roomzmax;
    Room_GetBounds(&roomxmin,&roomxmax,&roomymin,&roomymax,&roomzmin,&roomzmax);

    if(pad_number == 1) // Player 1
    {
        roomzmax = roomzmin + PAD_MAX_SEPARATION_RANGE;
    }
    else // Player 2 / AI
    {
        roomzmin = roomzmax - PAD_MAX_SEPARATION_RANGE;
    }

    int ballxmin, ballxmax, ballymin, ballymax, ballzmin, ballzmax;
    Ball_GetBounds(&ballxmin,&ballxmax,&ballymin,&ballymax,&ballzmin,&ballzmax);

    int padxmin, padxmax, padymin, padymax, padzmin, padzmax;
    _pad_GetBounds(p,&padxmin,&padxmax,&padymin,&padymax,&padzmin,&padzmax);

    int x_ball_coliding = 0, y_ball_coliding = 0, z_ball_coliding = 0;
    {
        s32 xoverlap, yoverlap, zoverlap;
        xoverlap = _segments_overlap(padxmin,padxmax, ballxmin,ballxmax);
        if(xoverlap)
        {
            yoverlap = _segments_overlap(padymin,padymax, ballymin,ballymax);
            if(yoverlap)
            {
                zoverlap = _segments_overlap(padzmin,padzmax, ballzmin,ballzmax);
                if(zoverlap)
                {
                    int min_overlap = min(xoverlap,min(yoverlap,zoverlap));
                    if(min_overlap == xoverlap) x_ball_coliding = 1;
                    if(min_overlap == yoverlap) y_ball_coliding = 1;
                    if(min_overlap == zoverlap) z_ball_coliding = 1;
                }
            }
        }
    }

    int bx,by,bz;
    Ball_GetPosition(&bx,&by,&bz);

    // X
    {
        int xm = INT_MAX;

        if(padxmin <= roomxmin)
        {
            p->collisions |= COLLISION_X_MIN;  xm = abs(roomxmin - padxmin);
        }
        else if(padxmax >= roomxmax)
        {
            p->collisions |= COLLISION_X_MAX;  xm = abs(padxmax - roomxmax);
        }

        if(x_ball_coliding)
        {
            int temp = 0;
            if(p->x < bx)
            {
                p->collisions |= COLLISION_X_MAX;  temp = abs(padxmax - ballxmin);
            }
            else
            {
                p->collisions |= COLLISION_X_MIN;  temp = abs(padxmin - ballxmax);
            }

            if(temp < xm) xm = temp; // save the greatest value to separate the pad
        }

        if(xmargin) *xmargin = xm+1;
    }

    // Y
    {
        int ym = INT_MAX;

        if(padymin <= roomymin)
        {
            p->collisions |= COLLISION_Y_MIN;  ym = abs(roomymin - padymin);
        }
        else if(padymax >= roomymax)
        {
            p->collisions |= COLLISION_Y_MAX;  ym = abs(padymax - roomymax);
        }

        if(y_ball_coliding)
        {
            int temp = 0;
            if(p->y < by)
            {
                p->collisions |= COLLISION_Y_MAX;  temp = abs(padymax - ballymin);
            }
            else
            {
                p->collisions |= COLLISION_Y_MIN;  temp = abs(padymin - ballymax);
            }

            if(temp < ym) ym = temp; // save the greatest value to separate the pad
        }

        if(ymargin) *ymargin = ym+1;
    }

    // Z
    {
        int zm = INT_MAX;

        if(padzmin <= roomzmin)
        {
            p->collisions |= COLLISION_Z_MIN;  zm = abs(roomzmin - padzmin);
        }
        else if(padzmax >= roomzmax)
        {
            p->collisions |= COLLISION_Z_MAX;  zm = abs(padzmax - roomzmax);
        }

        if(z_ball_coliding)
        {
            int temp = 0;
            if(p->z < bz)
            {
                p->collisions |= COLLISION_Z_MAX;  temp = abs(padzmax - ballzmin);
            }
            else
            {
                p->collisions |= COLLISION_Z_MIN;  temp = abs(padzmin - ballzmax);
            }

            if(temp < zm) zm = temp; // save the greatest value to separate the pad
        }

        if(zmargin) *zmargin = zm+1;
    }
}

void Pad_HandleAll(void)
{
#define PAD_MAX_ACCELERATION (float2fx(0.1))
#define PAD_MAX_SPEED (float2fx(0.3))

#define BOUNCE_ENERGY_CONSERVED (float2fx(0.4))
#define BOUNCE_MIN_SPEED (float2fx(0.1))

    // Player 1
    {
        _pad_t * p = &PAD1;

        //----------------------
        // Player input

        circlePosition cp; // dx,dy (-0x9C ~ 0x9C)
        hidCircleRead(&cp);
        cp.dx = clamp(-0x90,cp.dx,0x90);
        cp.dy = clamp(-0x90,cp.dy,0x90);

        int keys = hidKeysHeld();
        if(keys & KEY_DLEFT) cp.dx = -0x90;
        else if(keys & KEY_DRIGHT) cp.dx = +0x90;
        if(keys & KEY_DUP) cp.dy = +0x90;
        else if(keys & KEY_DDOWN) cp.dy = -0x90;

        if(Room_3DMode() == GAME_MODE_3D)
        {
            if(!Game_StateMachinePadMovementEnabled())
            {
                cp.dx = cp.dy = 0;
            }

            if( (cp.dx*cp.dx + cp.dy*cp.dy) > (0x20*0x20) )
            {
                p->ax = fxmul(PAD_MAX_ACCELERATION,int2fx(cp.dx)) / 0x90;
                p->ay = fxmul(PAD_MAX_ACCELERATION,int2fx(cp.dy)) / 0x90;
            }
            else
            {
                p->ax = 0;
                p->ay = 0;

                p->vx = fxmul(p->vx,float2fx(0.8));
                p->vy = fxmul(p->vy,float2fx(0.8));
                p->vz = fxmul(p->vz,float2fx(0.8));
            }

            int v = fxsqrt( fxmul(p->vx,p->vx) + fxmul(p->vy,p->vy) + fxmul(p->vz,p->vz) );
            if( v > PAD_MAX_SPEED )
            {
                v = fxdiv( PAD_MAX_SPEED, v );
                p->vx = fxmul(p->vx,v);
                p->vy = fxmul(p->vy,v);
                p->vz = fxmul(p->vz,v);
            }
            else if( v < float2fx(0.05) )
            {
                p->vx = p->vy = p->vz = float2fx(0.0);
            }
        }
        else
        {
            if(!Game_StateMachinePadMovementEnabled())
            {
                cp.dx = cp.dy = 0;
            }
            else
            {
                if(Room_3DMode() == GAME_MODE_2D_BOUNCE)
                {
                    if(hidKeysDown() & KEY_A)
                        Pad_P1Bounce(float2fx(0.3),-float2fx(0.015));
                }
            }

            if( (cp.dx*cp.dx + cp.dy*cp.dy) > (0x20*0x20) )
            {
                p->ax = fxmul(PAD_MAX_ACCELERATION,int2fx(cp.dx)) / 0x90;
                p->az = fxmul(PAD_MAX_ACCELERATION,int2fx(cp.dy)) / 0x90;
            }
            else
            {
                p->ax = 0;
                p->az = 0;

                p->vx = fxmul(p->vx,float2fx(0.8));
                p->vz = fxmul(p->vz,float2fx(0.8));
            }

            int v = fxsqrt( fxmul(p->vx,p->vx) + fxmul(p->vz,p->vz) );
            if( v > PAD_MAX_SPEED )
            {
                v = fxdiv( PAD_MAX_SPEED, v );
                p->vx = fxmul(p->vx,v);
                p->vz = fxmul(p->vz,v);
            }
            else if( v < float2fx(0.05) )
            {
                p->vx = p->vz = float2fx(0.0);
            }
        }
        //----------------------

        p->x += p->vx; p->y += p->vy; p->z += p->vz;

        int xm,ym,zm;
        _pad_UpdateCollisions(p,1,&xm,&ym,&zm);

        if(p->collisions & COLLISION_X_MIN)
        {
            if(p->vx < 0)
            {
                p->x += xm; p->vx = 0; p->ax = 0;
            }
        }
        else if(p->collisions & COLLISION_X_MAX)
        {
            if(p->vx > 0)
            {
                p->x -= xm; p->vx = 0; p->ax = 0;
            }
        }

        if(Room_3DMode() == GAME_MODE_2D_BOUNCE)
        {
            if(p->collisions & COLLISION_Y_MIN)
            {
                if(p->vy < 0)
                {
                    p->y += ym - p->vy; p->vy = -fxmul(p->vy,BOUNCE_ENERGY_CONSERVED);
                    if(abs(p->vy) < BOUNCE_MIN_SPEED)
                    {
                        int roomymin;
                        Room_GetBounds(NULL,NULL,&roomymin,NULL,NULL,NULL);
                        p->y = roomymin + (p->sy / 2) + 1;
                        p->vy = 0;
                        p->ay = 0;
                    }
                }
            }
            else if(p->collisions & COLLISION_Y_MAX)
            {
                if(p->vy > 0)
                {
                    p->y -= ym; p->vy = 0; p->ay = 0;
                }
            }
        }
        else if(Room_3DMode() == GAME_MODE_3D)
        {
            if(p->collisions & COLLISION_Y_MIN)
            {
                if(p->vy < 0)
                {
                    p->y += ym; p->vy = 0; p->ay = 0;
                }
            }
            else if(p->collisions & COLLISION_Y_MAX)
            {
                if(p->vy > 0)
                {
                    p->y -= ym; p->vy = 0; p->ay = 0;
                }
            }
        }

        if(p->collisions & COLLISION_Z_MIN)
        {
            if(p->vz < 0)
            {
                p->z += zm; p->vz = 0; p->az = 0;
            }
        }
        else if(p->collisions & COLLISION_Z_MAX)
        {
            if(p->vz > 0)
            {
                p->z -= zm; p->vz = 0; p->az = 0;
            }
        }

        p->vx += p->ax; p->vy += p->ay; p->vz += p->az;
    }

    // Player 2
    {
        _pad_t * p = &PAD2;

        //----------------------
        // AI
        if(Room_3DMode() == GAME_MODE_3D)
        {
            int bx,by,bz;
            Ball_GetPosition(&bx,&by,&bz);

            int roomzmin, roomzmax;
            Room_GetBounds(NULL,NULL,NULL,NULL,&roomzmin,&roomzmax);
            int z_first_quarter = roomzmin + ( (roomzmax-roomzmin) / 4 );

            int bvz;
            Ball_GetSpeed(NULL,NULL,&bvz);

            int move_left_right = 0, move_up_down = 0;
            int dx = 0, dy = 0;

            // Calculate direction

            if(Game_StateMachinePadMovementEnabled())
            {
                if( (bvz > 0) && (bz > z_first_quarter) ) // if aproaching the pad
                {
                    int ballxmin, ballxmax, ballymin, ballymax;
                    Ball_GetBounds(&ballxmin,&ballxmax,&ballymin,&ballymax,NULL,NULL);

                    int padxmin, padxmax, padymin, padymax;
                    _pad_GetBounds(p,&padxmin,&padxmax,&padymin,&padymax,NULL,NULL);

                    int ballxsize, ballysize;
                    Ball_GetDimensions(&ballxsize,&ballysize,NULL);

                    if(_segments_overlap(ballxmin,ballxmax, padxmin, padxmax) < ballxsize)
                    {
                        move_left_right = 1;
                        dx = bx - p->x;
                    }

                    if(_segments_overlap(ballymin,ballymax, padymin, padymax) < ballysize)
                    {
                        move_up_down = 1;
                        dy = by - p->y;
                    }
                }
                else // go back to the center of the screen
                {
                    int padxmin, padxmax, padymin, padymax;
                    _pad_GetBounds(p,&padxmin,&padxmax,&padymin,&padymax,NULL,NULL);

                    if(_segments_overlap(-float2fx(0.1),float2fx(0.1), padxmin, padxmax) < float2fx(0.19))
                    {
                        move_left_right = 1;
                        dx = float2fx(0.0) - p->x;
                    }

                    if(_segments_overlap(-float2fx(0.1),float2fx(0.1), padymin, padymax) < float2fx(0.19))
                    {
                        move_up_down = 1;
                        dy = float2fx(0.0) - p->y;
                    }
                }
            }

            // Move

            if(move_left_right && move_up_down)
            {
                p->ax = dx;
                p->ay = dy;
                int len = fxsqrt( fxmul(p->ax,p->ax) + fxmul(p->ay,p->ay) );
                len = fxdiv( PAD_MAX_ACCELERATION, len );
                p->ax = fxmul(p->ax,len);
                p->ay = fxmul(p->ay,len);

                p->az = 0;
                p->vz = fxmul(p->vz,float2fx(0.8));
            }
            else if(move_left_right)
            {
                if(dx > 0) p->ax = +PAD_MAX_ACCELERATION;
                else p->ax = -PAD_MAX_ACCELERATION;

                p->ay = 0;
                p->az = 0;
                p->vy = fxmul(p->vy,float2fx(0.8));
                p->vz = fxmul(p->vz,float2fx(0.8));
            }
            else if(move_up_down)
            {
                if(dy > 0) p->ay = +PAD_MAX_ACCELERATION;
                else p->ay = -PAD_MAX_ACCELERATION;

                p->ax = 0;
                p->az = 0;
                p->vx = fxmul(p->vx,float2fx(0.8));
                p->vz = fxmul(p->vz,float2fx(0.8));
            }
            else
            {
                p->ax = 0;
                p->ay = 0;
                p->az = 0;
                p->vx = fxmul(p->vx,float2fx(0.8));
                p->vy = fxmul(p->vy,float2fx(0.8));
                p->vz = fxmul(p->vz,float2fx(0.8));
            }

            int v = fxsqrt( fxmul(p->vx,p->vx) + fxmul(p->vy,p->vy) + fxmul(p->vz,p->vz) );
            if( v > PAD_MAX_SPEED )
            {
                v = fxdiv( PAD_MAX_SPEED, v );
                p->vx = fxmul(p->vx,v);
                p->vy = fxmul(p->vy,v);
                p->vz = fxmul(p->vz,v);
            }
            else if( v < float2fx(0.05) )
            {
                p->vx = p->vy = p->vz = float2fx(0.0);
            }
        }
        else
        {
            int bx,by,bz;
            Ball_GetPosition(&bx,&by,&bz);

            int roomzmin, roomzmax;
            Room_GetBounds(NULL,NULL,NULL,NULL,&roomzmin,&roomzmax);
            int z_first_quarter = roomzmin + ( (roomzmax-roomzmin) / 4 );

            int bvz;
            Ball_GetSpeed(NULL,NULL,&bvz);

            int move_left_right = 0, move_up_down = 0;
            int dx = 0, dy = 0;

            int bounce = 0;

            // Calculate direction

            if(Game_StateMachinePadMovementEnabled())
            {
                if( (bvz > 0) && (bz > z_first_quarter) ) // if aproaching the pad
                {
                    int ballxmin, ballxmax, ballymin, ballymax;
                    Ball_GetBounds(&ballxmin,&ballxmax,&ballymin,&ballymax,NULL,NULL);

                    int padxmin, padxmax, padymin, padymax;
                    _pad_GetBounds(p,&padxmin,&padxmax,&padymin,&padymax,NULL,NULL);

                    int ballxsize, ballysize;
                    Ball_GetDimensions(&ballxsize,&ballysize,NULL);

                    if(_segments_overlap(ballxmin,ballxmax, padxmin, padxmax) < ballxsize)
                    {
                        move_left_right = 1;
                        dx = bx - p->x;
                    }

                    if(_segments_overlap(ballymin,ballymax, padymin, padymax) < ballysize)
                    {
                        move_up_down = 1;
                        dy = by - p->y;
                    }
                }
                else // go back to the center of the screen
                {
                    int padxmin, padxmax, padymin, padymax;
                    _pad_GetBounds(p,&padxmin,&padxmax,&padymin,&padymax,NULL,NULL);

                    if(_segments_overlap(-float2fx(0.1),float2fx(0.1), padxmin, padxmax) < float2fx(0.19))
                    {
                        move_left_right = 1;
                        dx = float2fx(0.0) - p->x;
                    }
                }
            }

            // Move

            if(Room_3DMode() == GAME_MODE_2D_BOUNCE)
            {
                if(move_up_down && (dy > 0) )
                    bounce = 1;
            }

            if(move_left_right)
            {
                if(dx > 0) p->ax = +PAD_MAX_ACCELERATION;
                else p->ax = -PAD_MAX_ACCELERATION;

                p->az = 0;
                p->vz = fxmul(p->vz,float2fx(0.8));
            }
            else
            {
                p->ax = 0;
                p->az = 0;
                p->vx = fxmul(p->vx,float2fx(0.8));
                p->vz = fxmul(p->vz,float2fx(0.8));
            }

            int v = fxsqrt( fxmul(p->vx,p->vx) + fxmul(p->vz,p->vz) );
            if( v > PAD_MAX_SPEED )
            {
                v = fxdiv( PAD_MAX_SPEED, v );
                p->vx = fxmul(p->vx,v);
                p->vz = fxmul(p->vz,v);
            }
            else if( v < float2fx(0.05) )
            {
                p->vx = p->vz = float2fx(0.0);
            }

            if(bounce)
                Pad_P2Bounce(float2fx(0.3),-float2fx(0.015));
        }
        //----------------------

        p->x += p->vx; p->y += p->vy; p->z += p->vz;

        int xm,ym,zm;
        _pad_UpdateCollisions(p,2,&xm,&ym,&zm);

        if(p->collisions & COLLISION_X_MIN)
        {
            if(p->vx < 0)
            {
                p->x += xm; p->vx = 0; p->ax = 0;
            }
        }
        else if(p->collisions & COLLISION_X_MAX)
        {
            if(p->vx > 0)
            {
                p->x -= xm; p->vx = 0; p->ax = 0;
            }
        }

        if(Room_3DMode() == GAME_MODE_2D_BOUNCE)
        {
            if(p->collisions & COLLISION_Y_MIN)
            {
                if(p->vy < 0)
                {
                    p->y += ym - p->vy; p->vy = -fxmul(p->vy,BOUNCE_ENERGY_CONSERVED);
                    if(abs(p->vy) < BOUNCE_MIN_SPEED)
                    {
                        int roomymin;
                        Room_GetBounds(NULL,NULL,&roomymin,NULL,NULL,NULL);
                        p->y = roomymin + (p->sy / 2) + 1;
                        p->vy = 0;
                        p->ay = 0;
                    }
                }
            }
            else if(p->collisions & COLLISION_Y_MAX)
            {
                if(p->vy > 0)
                {
                    p->y -= ym; p->vy = 0; p->ay = 0;
                }
            }
        }
        else if(Room_3DMode() == GAME_MODE_3D)
        {
            if(p->collisions & COLLISION_Y_MIN)
            {
                if(p->vy < 0)
                {
                    p->y += ym; p->vy = 0; p->ay = 0;
                }
            }
            else if(p->collisions & COLLISION_Y_MAX)
            {
                if(p->vy > 0)
                {
                    p->y -= ym; p->vy = 0; p->ay = 0;
                }
            }
        }

        if(p->collisions & COLLISION_Z_MIN)
        {
            if(p->vz < 0)
            {
                p->z += zm; p->vz = 0; p->az = 0;
            }
        }
        else if(p->collisions & COLLISION_Z_MAX)
        {
            if(p->vz > 0)
            {
                p->z -= zm; p->vz = 0; p->az = 0;
            }
        }

        p->vx += p->ax; p->vy += p->ay; p->vz += p->az;
    }
}
