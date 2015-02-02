
//--------------------------------------------------------------------------------------------------

#include <string.h>

#include "S3D/engine.h"
#include "game.h"
#include "pad.h"

//--------------------------------------------------------------------------------------------------

typedef struct { // values in fixed point!
	s32 sx,sy,sz; // size
	
	s32 x,vx,ax; // coordinate, speed, acceleration
	s32 y,vy,ay;
	s32 z,vz,az;
	
	int r,g,b;
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

static void Pad_Draw(int screen, _pad_t * p)
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
	
	S3D_PolygonVertex(screen, xmax,ymax,float2fx(0.5)); 			
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
	Pad_Draw(screen,&PAD1);
}

void Pad_P2Draw(int screen)
{
	Pad_Draw(screen,&PAD2);
}

//--------------------------------------------------------------------------------------------------

void Pad_P1GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax)
{
	_pad_t * p = &PAD1;
	if(xmin) *xmin = p->x - (p->sx/2);
	if(xmax) *xmax = p->x + (p->sx/2);
	if(ymin) *ymin = p->y - (p->sy/2);
	if(ymax) *ymax = p->y + (p->sy/2);
	if(zmin) *zmin = p->z - (p->sz/2);
	if(zmax) *zmax = p->z + (p->sz/2);
}

void Pad_P2GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax)
{
	_pad_t * p = &PAD2;
	if(xmin) *xmin = p->x - (p->sx/2);
	if(xmax) *xmax = p->x + (p->sx/2);
	if(ymin) *ymin = p->y - (p->sy/2);
	if(ymax) *ymax = p->y + (p->sy/2);
	if(zmin) *zmin = p->z - (p->sz/2);
	if(zmax) *zmax = p->z + (p->sz/2);
}

//--------------------------------------------------------------------------------------------------

void Pad_InitAll(void)
{
	memset(&PAD1,0,sizeof(_pad_t));
	memset(&PAD2,0,sizeof(_pad_t));
}

//--------------------------------------------------------------------------------------------------

void Pad_HandleAll(void)
{
#warning "TODO"
}

//--------------------------------------------------------------------------------------------------