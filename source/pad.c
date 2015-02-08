
//--------------------------------------------------------------------------------------------------

#include <string.h>

#include <3ds.h>

#include "S3D/engine.h"
#include "game.h"
#include "pad.h"
#include "ball.h"
#include "room.h"

//--------------------------------------------------------------------------------------------------

typedef struct { // values in fixed point!
	s32 sx,sy,sz; // size
	
	s32 x,vx,ax; // coordinate, speed, acceleration
	s32 y,vy,ay;
	s32 z,vz,az;
	
	int r,g,b;
	
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

void Pad_P1SetColor(int r, int g, int b) // Player
{
	_pad_t * p = &PAD1;
	p->r = r;
	p->g = g;
	p->b = b;
}

void Pad_P2SetColor(int r, int g, int b) // AI
{
	_pad_t * p = &PAD2;
	p->r = r;
	p->g = g;
	p->b = b;
}

//--------------------------------------------------------------------------------------------------

static inline void _pad_draw(int screen, _pad_t * p)
{
	int xmin = p->x - (p->sx/2);
	int xmax = p->x + (p->sx/2);
	int ymin = p->y - (p->sy/2);
	int ymax = p->y + (p->sy/2);
	int zmin = p->z - (p->sz/2);
	int zmax = p->z + (p->sz/2);
	
	S3D_PolygonColor(screen, p->r,p->g,p->b);
	
	S3D_PolygonBegin(screen, S3D_QUAD_STRIP);
	
	S3D_PolygonNormal(screen, float2fx(-1.0),float2fx(0.0),float2fx(0.0));
	
	S3D_PolygonVertex(screen, xmin,ymin,zmin);
	S3D_PolygonVertex(screen, xmin,ymin,zmax);
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
	_pad_draw(screen,&PAD1);
}

void Pad_P2Draw(int screen)
{
	_pad_draw(screen,&PAD2);
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
	PAD2.z = float2fx(10.0);
}

//--------------------------------------------------------------------------------------------------

static inline int abs(int a)
{
	return a > 0 ? a : -a;
}

static void _pad_UpdateCollisions(_pad_t * p)
{
	p->collisions = 0;
	
	int roomxmin, roomxmax, roomymin, roomymax, roomzmin, roomzmax;
	Room_GetBounds(&roomxmin,&roomxmax,&roomymin,&roomymax,&roomzmin,&roomzmax);
	
	int ballxmin, ballxmax, ballymin, ballymax, ballzmin, ballzmax;
	Ball_GetBounds(&ballxmin,&ballxmax,&ballymin,&ballymax,&ballzmin,&ballzmax);
	
	int padxmin, padxmax, padymin, padymax, padzmin, padzmax;
	_pad_GetBounds(p,&padxmin,&padxmax,&padymin,&padymax,&padzmin,&padzmax);
	
	int ball_coliding = 0;/*
	if(_segments_overlap(padxmin,padxmax, ballxmin,ballxmax))
	{
		if(_segments_overlap(padymin,padymax, ballymin,ballymax))
		{
			if(_segments_overlap(padymin,padymax, ballymin,ballymax))
			{
				ball_coliding = 1;
			}
		}
	}*/
	
	int bx,by,bz;
	Ball_GetPosition(&bx,&by,&bz);
	
	// X
	{
		if(padxmin <= roomxmin) p->collisions |= COLLISION_X_MIN;
		if(padxmax >= roomxmax) p->collisions |= COLLISION_X_MAX;
		
		if(ball_coliding)
		{
			if(p->x < bx) p->collisions |= COLLISION_X_MAX;
			else p->collisions |= COLLISION_X_MIN;
		}
	}
	
	// Y
	{
		if(padymin <= roomymin) p->collisions |= COLLISION_Y_MIN;
		if(padymax >= roomymax) p->collisions |= COLLISION_Y_MAX;
		
		if(ball_coliding)
		{
			if(p->y < by) p->collisions |= COLLISION_Y_MAX;
			else p->collisions |= COLLISION_Y_MIN;
		}
	}
	
	// Z
	{
		if(padzmin <= roomzmin) p->collisions |= COLLISION_Z_MIN;
		if(padzmax >= roomzmax) p->collisions |= COLLISION_Z_MAX;
		
		if(ball_coliding)
		{
			if(p->z < bz) p->collisions |= COLLISION_Z_MAX;
			else p->collisions |= COLLISION_Z_MIN;
		}
	}
}

void Pad_HandleAll(void)
{
	// Player 1
	{
		_pad_t * p = &PAD1;
		
		int keys = hidKeysHeld();
		if(keys & KEY_RIGHT) p->vx = +float2fx(0.25);
		else if(keys & KEY_LEFT) p->vx = -float2fx(0.25);
		else p->vx = 0;
		
		p->x += p->vx; p->y += p->vy; p->z += p->vz;
		
		_pad_UpdateCollisions(p);
		
		if(p->collisions & COLLISION_X_MIN)
		{
			if(p->vx < 0)
			{
				p->x -= p->vx; p->vx = 0; p->ax = 0;
			}
		}
		else if(p->collisions & COLLISION_X_MAX)
		{
			if(p->vx > 0)
			{
				p->x -= p->vx; p->vx = 0; p->ax = 0;
			}
		}
		
		if(p->collisions & COLLISION_Y_MIN)
		{
			if(p->vy < 0)
			{
				p->y -= p->vy; p->vy = 0; p->ay = 0;
			}
		}
		else if(p->collisions & COLLISION_Y_MAX)
		{
			if(p->vy > 0)
			{
				p->y -= p->vy; p->vy = 0; p->ay = 0;
			}
		}
		
		if(p->collisions & COLLISION_Z_MIN)
		{
			if(p->vz < 0)
			{
				p->z -= p->vz; p->vz = 0; p->az = 0;
			}
		}
		else if(p->collisions & COLLISION_Z_MAX)
		{
			if(p->vz > 0)
			{
				p->z -= p->vz; p->vz = 0; p->az = 0;
			}
		}
		
		_pad_UpdateCollisions(p);
		
		p->vx += p->ax; p->vy += p->ay; p->vz += p->az;
	}
	
	// Player 2
	{
		_pad_t * p = &PAD2;
		
		p->x += p->vx; p->y += p->vy; p->z += p->vz;
		
		_pad_UpdateCollisions(p);
		
		if(p->collisions & COLLISION_X_MIN)
		{
			p->vx = +abs(p->vx); p->ax = +abs(p->ax); p->x -= p->vx;
		}
		else if(p->collisions & COLLISION_X_MAX)
		{
			p->vx = -abs(p->vx); p->ax = -abs(p->ax); p->x -= p->vx;
		}
		
		if(p->collisions & COLLISION_Y_MIN)
		{
			p->vy = +abs(p->vy); p->ay = +abs(p->ay); p->y -= p->vy;
		}
		else if(p->collisions & COLLISION_Y_MAX)
		{
			p->vy = -abs(p->vy); p->ay = -abs(p->ay); p->y -= p->vy;
		}
		
		if(p->collisions & COLLISION_Z_MIN)
		{
			p->vz = +abs(p->vz); p->az = +abs(p->az); p->z -= p->vz;
		}
		else if(p->collisions & COLLISION_Z_MAX)
		{
			p->vz = -abs(p->vz); p->az = -abs(p->az); p->z -= p->vz;
		}
		
		_pad_UpdateCollisions(p);
		
		p->vx += p->ax; p->vy += p->ay; p->vz += p->az;
	}
}

//--------------------------------------------------------------------------------------------------