
//--------------------------------------------------------------------------------------------------

#include <string.h>
#include <limits.h>

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

static inline int min(int a, int b)
{
	return a < b ? a : b;
}

// margins are only valid if there is a collision in that axis
static void _pad_UpdateCollisions(_pad_t * p, int * xmargin, int * ymargin, int * zmargin)
{
	p->collisions = 0;
	
	int roomxmin, roomxmax, roomymin, roomymax, roomzmin, roomzmax;
	Room_GetBounds(&roomxmin,&roomxmax,&roomymin,&roomymax,&roomzmin,&roomzmax);
	
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
	// Player 1
	{
		_pad_t * p = &PAD1;
		
		int keys = hidKeysHeld();
		if(keys & KEY_RIGHT) p->vx = +float2fx(0.25);
		else if(keys & KEY_LEFT) p->vx = -float2fx(0.25);
		else p->vx = 0;
		
		if(keys & KEY_UP) p->vz = +float2fx(0.25);
		else if(keys & KEY_DOWN) p->vz = -float2fx(0.25);
		else p->vz = 0;
		
		p->x += p->vx; p->y += p->vy; p->z += p->vz;
		
		int xm,ym,zm;
		_pad_UpdateCollisions(p,&xm,&ym,&zm);
		
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
		_pad_t * p = &PAD1;
		/*
		int keys = hidKeysHeld();
		if(keys & KEY_RIGHT) p->vx = +float2fx(0.25);
		else if(keys & KEY_LEFT) p->vx = -float2fx(0.25);
		else p->vx = 0;
		
		if(keys & KEY_UP) p->vz = +float2fx(0.25);
		else if(keys & KEY_DOWN) p->vz = -float2fx(0.25);
		else p->vz = 0;
		*/
		p->x += p->vx; p->y += p->vy; p->z += p->vz;
		
		int xm,ym,zm;
		_pad_UpdateCollisions(p,&xm,&ym,&zm);
		
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

//--------------------------------------------------------------------------------------------------