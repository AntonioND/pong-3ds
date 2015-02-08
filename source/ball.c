
//--------------------------------------------------------------------------------------------------

#include <string.h>

#include "S3D/engine.h"
#include "game.h"
#include "ball.h"
#include "room.h"
#include "pad.h"

//--------------------------------------------------------------------------------------------------

typedef struct { // values in fixed point!
	s32 sx,sy,sz; // size
	
	s32 x,vx,ax; // coordinate, speed, acceleration
	s32 y,vy,ay;
	s32 z,vz,az;
	
	int r,g,b;
	
	u32 collisions;
} _ball_t;

static _ball_t BALL;

//--------------------------------------------------------------------------------------------------

void Ball_SetDimensions(int x, int y, int z)
{
	BALL.sx = x;
	BALL.sy = y;
	BALL.sz = z;
}

//--------------------------------------------------------------------------------------------------

void Ball_SetColor(int r, int g, int b)
{
	BALL.r = r;
	BALL.g = g;
	BALL.b = b;
}

//--------------------------------------------------------------------------------------------------

void Ball_Draw(int screen)
{
	int xmin = BALL.x - (BALL.sx/2);
	int xmax = BALL.x + (BALL.sx/2);
	int ymin = BALL.y - (BALL.sy/2);
	int ymax = BALL.y + (BALL.sy/2);
	int zmin = BALL.z - (BALL.sz/2);
	int zmax = BALL.z + (BALL.sz/2);
	
	S3D_PolygonColor(screen, BALL.r,BALL.g,BALL.b);
	
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

//--------------------------------------------------------------------------------------------------

void Ball_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax)
{
	if(xmin) *xmin = BALL.x - (BALL.sx/2);
	if(xmax) *xmax = BALL.x + (BALL.sx/2);
	if(ymin) *ymin = BALL.y - (BALL.sy/2);
	if(ymax) *ymax = BALL.y + (BALL.sy/2);
	if(zmin) *zmin = BALL.z - (BALL.sz/2);
	if(zmax) *zmax = BALL.z + (BALL.sz/2);
}

//--------------------------------------------------------------------------------------------------

void Ball_GetPosition(int * x, int * y, int * z)
{
	if(x) *x = BALL.x;
	if(y) *y = BALL.y;
	if(z) *z = BALL.z;
}

//--------------------------------------------------------------------------------------------------

void Ball_Init(void)
{
	memset(&BALL,0,sizeof(_ball_t));
	
	int roomxmin, roomxmax, roomymin, roomymax, roomzmin, roomzmax;
	Room_GetBounds(&roomxmin,&roomxmax,&roomymin,&roomymax,&roomzmin,&roomzmax);
	
	BALL.x = (roomxmax + roomxmin) / 2;
	BALL.y = 0; //(roomymax + roomymin) / 2;
	BALL.z = (roomzmax + roomzmin) / 2;
	
	BALL.vx = float2fx(0.05);
	BALL.vz = float2fx(0.05);
}

//--------------------------------------------------------------------------------------------------

static inline int abs(int a)
{
	return a > 0 ? a : -a;
}

static void _ball_UpdateCollisions(void)
{
	BALL.collisions = 0;
	
	int roomxmin, roomxmax, roomymin, roomymax, roomzmin, roomzmax;
	Room_GetBounds(&roomxmin,&roomxmax,&roomymin,&roomymax,&roomzmin,&roomzmax);

	int ballxmin, ballxmax, ballymin, ballymax, ballzmin, ballzmax;
	Ball_GetBounds(&ballxmin,&ballxmax,&ballymin,&ballymax,&ballzmin,&ballzmax);
	
	// Collision with only one pad
	int roomzmid = (roomzmin + roomzmax) / 2;
	
	int px,py,pz;
	int padxmin, padxmax, padymin, padymax, padzmin, padzmax;
	if(BALL.z > roomzmid)
	{
		Pad_P2GetBounds(&padxmin,&padxmax,&padymin,&padymax,&padzmin,&padzmax);
		Pad_P2GetPosition(&px,&py,&pz);
	}
	else
	{
		Pad_P1GetBounds(&padxmin,&padxmax,&padymin,&padymax,&padzmin,&padzmax);
		Pad_P1GetPosition(&px,&py,&pz);
	}

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
	
	// X
	{
		if(ballxmin <= roomxmin) BALL.collisions |= COLLISION_X_MIN;
		if(ballxmax >= roomxmax) BALL.collisions |= COLLISION_X_MAX;
		
		if(ball_coliding)
		{
			if(BALL.x < px) BALL.collisions |= COLLISION_X_MAX;
			else BALL.collisions |= COLLISION_X_MIN;
		}
	}
	
	// Y
	{
		if(ballymin <= roomymin) BALL.collisions |= COLLISION_Y_MIN;
		if(ballymax >= roomymax) BALL.collisions |= COLLISION_Y_MAX;
		
		if(ball_coliding)
		{
			if(BALL.y < py) BALL.collisions |= COLLISION_Y_MAX;
			else BALL.collisions |= COLLISION_Y_MIN;
		}
	}
	
	// Z
	{
		if(ballzmin <= roomzmin) BALL.collisions |= COLLISION_Z_MIN;
		if(ballzmax >= roomzmax) BALL.collisions |= COLLISION_Z_MAX;
		
		if(ball_coliding)
		{
			if(BALL.z < pz) BALL.collisions |= COLLISION_Z_MAX;
			else BALL.collisions |= COLLISION_Z_MIN;
		}
	}
}

void Ball_Handle(void)
{
	BALL.x += BALL.vx; BALL.y += BALL.vy; BALL.z += BALL.vz;
	
	_ball_UpdateCollisions();
	
	if(BALL.collisions & COLLISION_X_MIN)
	{
		BALL.vx = +abs(BALL.vx); BALL.ax = +abs(BALL.ax); BALL.x -= BALL.vx;
	}
	else if(BALL.collisions & COLLISION_X_MAX)
	{
		BALL.vx = -abs(BALL.vx); BALL.ax = -abs(BALL.ax); BALL.x -= BALL.vx;
	}
	
	if(BALL.collisions & COLLISION_Y_MIN)
	{
		BALL.vy = +abs(BALL.vy); BALL.ay = +abs(BALL.ay); BALL.y -= BALL.vy;
	}
	else if(BALL.collisions & COLLISION_Y_MAX)
	{
		BALL.vy = -abs(BALL.vy); BALL.ay = -abs(BALL.ay); BALL.y -= BALL.vy;
	}
	
	if(BALL.collisions & COLLISION_Z_MIN)
	{
		BALL.vz = +abs(BALL.vz); BALL.az = +abs(BALL.az); BALL.z -= BALL.vz;
	}
	else if(BALL.collisions & COLLISION_Z_MAX)
	{
		BALL.vz = -abs(BALL.vz); BALL.az = -abs(BALL.az); BALL.z -= BALL.vz;
	}
	
	_ball_UpdateCollisions();
	
	BALL.vx += BALL.ax; BALL.vy += BALL.ay; BALL.vz += BALL.az;
}

//--------------------------------------------------------------------------------------------------