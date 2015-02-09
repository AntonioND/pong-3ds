
//--------------------------------------------------------------------------------------------------

#include <string.h>
#include <limits.h>

#include "S3D/engine.h"
#include "game.h"
#include "ball.h"
#include "rooms.h"
#include "pad.h"
#include "utils.h"

//--------------------------------------------------------------------------------------------------

static inline int abs(int a)
{
	return a > 0 ? a : -a;
}

static inline int min(int a, int b)
{
	return a < b ? a : b;
}

static inline int sgn(int a)
{
	if(a > 0) return 1;
	if(a < 0) return -1;
	return 0;
}

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

void Ball_GetSpeed(int * x, int * y, int * z)
{
	if(x) *x = BALL.vx;
	if(y) *y = BALL.vy;
	if(z) *z = BALL.vz;
}

void Ball_GetDimensions(int * x, int * y, int * z)
{
	if(x) *x = BALL.sx;
	if(y) *y = BALL.sy;
	if(z) *z = BALL.sz;
}

//--------------------------------------------------------------------------------------------------

static void _ball_SetZSpeedMinMax(void)
{
	//Limit min Z speed
	while(abs(BALL.vz) < float2fx(0.05) )
	{
		int v = fxsqrt( fxmul(BALL.vx,BALL.vx) + fxmul(BALL.vy,BALL.vy) + fxmul(BALL.vz,BALL.vz) );
		
		// BALL.vz shouldn't be 0 or this will fail...
		if(BALL.vz == 0) BALL.vz = float2fx(0.1);
		else BALL.vz = sgn(BALL.vz) * float2fx(0.1);
		
		int new_v = fxsqrt( fxmul(BALL.vx,BALL.vx) + fxmul(BALL.vy,BALL.vy) + fxmul(BALL.vz,BALL.vz) );
		
		int factor = fxdiv( v, new_v );
		BALL.vx = fxmul(BALL.vx,factor);
		BALL.vy = fxmul(BALL.vy,factor);
		BALL.vz = fxmul(BALL.vz,factor);
	}
	
	//Limit max Z relative speed
	while(1)
	{
		int v = fxsqrt( fxmul(BALL.vx,BALL.vx) + fxmul(BALL.vy,BALL.vy) + fxmul(BALL.vz,BALL.vz) );
		int factor = fxdiv( float2fx(1.0), v );
		int relative_vz = fxmul(BALL.vz,factor);
		
		if(abs(relative_vz) > float2fx(0.8)) // this is the same as a dot product by (0,0,1)
		{
			BALL.vz = fxmul(BALL.vz,float2fx(0.75));
			
			int new_v = fxsqrt( fxmul(BALL.vx,BALL.vx) + fxmul(BALL.vy,BALL.vy) + fxmul(BALL.vz,BALL.vz) );
			
			factor = fxdiv( v, new_v );
			BALL.vx = fxmul(BALL.vx,factor);
			BALL.vy = fxmul(BALL.vy,factor);
			BALL.vz = fxmul(BALL.vz,factor);
		}
		else
		{
			break;
		}
	}
}

//--------------------------------------------------------------------------------------------------

void Ball_Init(void)
{
	memset(&BALL,0,sizeof(_ball_t));
}

#define BALL_START_SPEED (float2fx(0.2))
#define BALL_MAX_SPEED (float2fx(0.4))

void Ball_Reset(void)
{
	int roomxmin, roomxmax, roomymin, roomymax, roomzmin, roomzmax;
	Room_GetBounds(&roomxmin,&roomxmax,&roomymin,&roomymax,&roomzmin,&roomzmax);
	
	if(Room_3DMovementEnabled())
	{
		BALL.x = float2fx(0.0);
		BALL.y = float2fx(0.0);
		BALL.z = (roomzmax + roomzmin) / 2;
		
		BALL.vx = (fast_rand() & int2fx(2)) - int2fx(1);
		BALL.vy = (fast_rand() & int2fx(2)) - int2fx(1);
		BALL.vz = (fast_rand() & int2fx(2)) - int2fx(1);
	}
	else
	{
		BALL.x = float2fx(0.0);
		BALL.y = roomymin + (BALL.sy/2);
		BALL.z = (roomzmax + roomzmin) / 2;
		
		BALL.vx = (fast_rand() & int2fx(2)) - int2fx(1);
		BALL.vy = float2fx(0.0);
		BALL.vz = (fast_rand() & int2fx(2)) - int2fx(1);
	}
	
	// Set speed vector norm to BALL_START_SPEED
	int v = fxsqrt( fxmul(BALL.vx,BALL.vx) + fxmul(BALL.vy,BALL.vy) + fxmul(BALL.vz,BALL.vz) );
	int factor = fxdiv( BALL_START_SPEED, v );
	BALL.vx = fxmul(BALL.vx,factor);
	BALL.vy = fxmul(BALL.vy,factor);
	BALL.vz = fxmul(BALL.vz,factor);
	
	_ball_SetZSpeedMinMax();
}

//--------------------------------------------------------------------------------------------------

// margins are only valid if there is a collision in that axis
static void _ball_UpdateCollisions(int * xmargin, int * ymargin, int * zmargin)
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
		
		if(ballxmin <= roomxmin)
		{
			BALL.collisions |= COLLISION_X_MIN;  xm = abs(roomxmin - ballxmin);
		}
		else if(ballxmax >= roomxmax)
		{
			BALL.collisions |= COLLISION_X_MAX;  xm = abs(ballxmax - roomxmax);
		}
		
		if(x_ball_coliding)
		{
			int temp = 0;
			if(BALL.x < px)
			{
				BALL.collisions |= COLLISION_X_MAX;  temp = abs(ballxmax - padxmin);
			}
			else
			{
				BALL.collisions |= COLLISION_X_MIN;  temp = abs(ballxmin - padxmax);
			}
			
			if(temp < xm) xm = temp; // save the greatest value to separate the ball
		}
		
		if(xmargin) *xmargin = xm+1;
	}
	
	// Y
	{
		int ym = INT_MAX;
		
		if(ballymin <= roomymin)
		{
			BALL.collisions |= COLLISION_Y_MIN;  ym = abs(roomymin - ballymin);
		}
		else if(ballymax >= roomymax)
		{
			BALL.collisions |= COLLISION_Y_MAX;  ym = abs(ballymax - roomymax);
		}
		
		if(y_ball_coliding)
		{
			int temp = 0;
			if(BALL.y < py)
			{
				BALL.collisions |= COLLISION_Y_MAX;  temp = abs(ballymax - padymin);
			}
			else
			{
				BALL.collisions |= COLLISION_Y_MIN;  temp = abs(ballymin - padymax);
			}
			
			if(temp < ym) ym = temp; // save the greatest value to separate the ball
		}
		
		if(ymargin) *ymargin = ym+1;
	}
	
	// Z
	{
		int zm = INT_MAX;
		/*
		if(ballzmin <= roomzmin)
		{
			BALL.collisions |= COLLISION_Z_MIN;  zm = abs(roomzmin - ballzmin);
		}
		else if(ballzmax >= roomzmax)
		{
			BALL.collisions |= COLLISION_Z_MAX;  zm = abs(ballzmax - roomzmax);
		}
		*/
		if(z_ball_coliding)
		{
			int temp = 0;
			if(BALL.z < pz)
			{
				BALL.collisions |= COLLISION_Z_MAX;  temp = abs(ballzmax - padzmin);
			}
			else
			{
				BALL.collisions |= COLLISION_Z_MIN;  temp = abs(ballzmin - padzmax);
			}
			
			if(temp < zm) zm = temp; // save the greatest value to separate the ball
		}
		
		if(zmargin) *zmargin = zm+1;
	}
}

void Ball_Handle(void)
{
	// Check if goal
	if(Game_StateMachineBallAddScoreEnabled())
	{
		int roomzmin, roomzmax;
		Room_GetBounds(NULL,NULL,NULL,NULL,&roomzmin,&roomzmax);
		
		int ballzmin, ballzmax;
		Ball_GetBounds(NULL,NULL,NULL,NULL,&ballzmin,&ballzmax);
		
		if( (ballzmin + BALL.vz) < roomzmin) // P2 scores
		{
			Game_PlayerScoreIncrease(1);
			Game_PlayerScoreStartDelay();
		}
		else if( (ballzmax + BALL.vz) > roomzmax) // P1 scores
		{
			Game_PlayerScoreIncrease(0);
			Game_PlayerScoreStartDelay();
		}
	}
	
	// Move
	
	if(!Game_StateMachineBallMovementEnabled()) return;
	
	BALL.x += BALL.vx; BALL.y += BALL.vy; BALL.z += BALL.vz;

	int xm,ym,zm;
	_ball_UpdateCollisions(&xm,&ym,&zm);
	
	int bounce = 0;
	
	if(BALL.collisions & COLLISION_X_MIN)
	{
		if(BALL.vx < 0)
		{
			BALL.x += xm - BALL.vx; BALL.vx = -BALL.vx;
			bounce = 1;
		}
	}
	else if(BALL.collisions & COLLISION_X_MAX)
	{
		if(BALL.vx > 0)
		{
			BALL.x -= xm + BALL.vx; BALL.vx = -BALL.vx;
			bounce = 1;
		}
	}
	
	if(BALL.collisions & COLLISION_Y_MIN)
	{
		if(BALL.vy < 0)
		{
			BALL.y += ym - BALL.vy; BALL.vy = -BALL.vy;
			bounce = 1;
		}
	}
	else if(BALL.collisions & COLLISION_Y_MAX)
	{
		if(BALL.vy > 0)
		{
			BALL.y -= ym + BALL.vy; BALL.vy = -BALL.vy;
			bounce = 1;
		}
	}
	
	int weird_bounce = 0;
	
	if(BALL.collisions & COLLISION_Z_MIN)
	{
		if(BALL.vz < 0)
		{
			BALL.z += zm - BALL.vz; BALL.vz = -BALL.vz;
			weird_bounce = 1;
		}
	}
	else if(BALL.collisions & COLLISION_Z_MAX)
	{
		if(BALL.vz > 0)
		{
			BALL.z -= zm + BALL.vz; BALL.vz = -BALL.vz;
			weird_bounce = 1;
		}
	}
	
	// Update speed
	
	if(bounce)
	{
		BALL.vx = fxmul(BALL.vx,float2fx(1.03)); // speed up
		BALL.vy = fxmul(BALL.vy,float2fx(1.03));
		BALL.vz = fxmul(BALL.vz,float2fx(1.03));
		
		int v = fxsqrt( fxmul(BALL.vx,BALL.vx) + fxmul(BALL.vy,BALL.vy) + fxmul(BALL.vz,BALL.vz) );
		if( v > BALL_MAX_SPEED ) // limit speed
		{
			v = fxdiv( BALL_MAX_SPEED, v );
			BALL.vx = fxmul(BALL.vx,v);
			BALL.vy = fxmul(BALL.vy,v);
			BALL.vz = fxmul(BALL.vz,v);
		}
	}
	
	if(weird_bounce)
	{
		int v = fxsqrt( fxmul(BALL.vx,BALL.vx) + fxmul(BALL.vy,BALL.vy) + fxmul(BALL.vz,BALL.vz) );
		
		#define BALL_VARIATION_RANGE (int2fx(1.0)>>3)
		BALL.vx += (fast_rand() & (BALL_VARIATION_RANGE-1)) - (BALL_VARIATION_RANGE>>1);
		if(Room_3DMovementEnabled()) BALL.vy += (fast_rand() & (BALL_VARIATION_RANGE-1)) - (BALL_VARIATION_RANGE>>1);
		
		int new_v = fxsqrt( fxmul(BALL.vx,BALL.vx) + fxmul(BALL.vy,BALL.vy) + fxmul(BALL.vz,BALL.vz) );
		
		int factor = fxdiv( v, new_v );
		BALL.vx = fxmul(BALL.vx,factor);
		BALL.vy = fxmul(BALL.vy,factor);
		BALL.vz = fxmul(BALL.vz,factor);
		
		_ball_SetZSpeedMinMax();
	}
	
	BALL.vx += BALL.ax; BALL.vy += BALL.ay; BALL.vz += BALL.az;
}

//--------------------------------------------------------------------------------------------------