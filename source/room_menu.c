
//-------------------------------------------------------------------------------------------------------

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

//-------------------------------------------------------------------------------------------------------

#include "S3D/engine.h"
#include "game.h"
#include "rooms.h"
#include "pad.h"
#include "ball.h"
#include "utils.h"
#include "sound.h"

//--------------------------------------------------------------------------------------------------

static void _draw_cube(int screen, int sx, int sy, int sz, int r, int g, int b)
{
	int xmin = -sx/2;
	int xmax = +sx/2;
	int ymin = -sy/2;
	int ymax = +sy/2;
	int zmin = -sz/2;
	int zmax = +sz/2;
	
	S3D_PolygonColor(screen, r,g,b);
	
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

static void _quad_blit_unsafe_32(u8 * buf, const u8 * src, int x, int y, int w, int h)
{
	u8 * linebuf = &(buf[(240*x+y)*3]);
	
	while(w--)
	{
		int i = h;
		u8 * p = linebuf; linebuf += 240*3;
		while(i--)
		{
			if(*src++) // alpha
			{
				*p++ = *src++; *p++ = *src++; *p++ = *src++;
			}
			else
			{
				p += 3; src += 3;
			}
		}
	}
}

static void _quad_blit_unsafe_real_alpha_32(u8 * buf, const u8 * src, int x, int y, int w, int h)
{
	u8 * linebuf = &(buf[(240*x+y)*3]);
	
	while(w--)
	{
		int i = h;
		u8 * p = linebuf; linebuf += 240*3;
		while(i--)
		{
			int a = *src++;
			if(a == 255)
			{
				*p++ = *src++; *p++ = *src++; *p++ = *src++;
			}
			else if(a == 0)
			{
				src += 3;
				p += 3;
			}
			else
			{
				int one_minus_alpha = 255 - a;
				
				int b = *src++;
				int g = *src++;
				int r = *src++;
				
				p[0] = ( (p[0] * one_minus_alpha) + b * a ) / 256;
				p[1] = ( (p[1] * one_minus_alpha) + g * a ) / 256;
				p[2] = ( (p[2] * one_minus_alpha) + r * a ) / 256;
				p += 3;
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------

static void HSV2RGB(int h, int s, int v, int * r, int * g, int * b) // h,s,v: fixed point 0.0 to 1.0 | | | r,b,g: 0..255
{
	if(s == 0)
	{
		int c = fx2int(v * 255);
		*r = c;
		*g = c;
		*b = c;
	}
	else
	{
		int vH = fx2ufrac(h) * 6;
		int vHI = vH - fx2ufrac(vH);
		int sector = fx2int(vH);
		
		int v0 = fx2int( v*255 );
		int v1 = fx2int( fxmul(v,int2fx(1)-s) * 255 );
		int v2 = fx2int( fxmul(v,int2fx(1)-fxmul(s,vH-vHI)) * 255 );
		int v3 = fx2int( fxmul(v,int2fx(1)-fxmul(s,int2fx(1)-(vH-vHI))) * 255 );
		
		switch(sector)
		{
			case 0:  *r = v0; *g = v3; *b = v1; break;
			case 1:  *r = v2; *g = v0; *b = v1; break;
			case 2:  *r = v1; *g = v0; *b = v3; break;
			case 3:  *r = v1; *g = v2; *b = v0; break;
			case 4:  *r = v3; *g = v1; *b = v0; break;
			case 5:  *r = v0; *g = v1; *b = v2; break;
			
			default: *r = 0;  *g = 0;  *b = 0;  break;
		}
	}
}

//--------------------------------------------------------------------------------------------------

#include "pong_3ds_png_bin.h"
#define PONG_3DS_WIDTH (354)
#define PONG_3DS_HEIGHT (48)

#include "by_antoniond_png_bin.h"
#define BY_ANTONIOND_WIDTH (136)
#define BY_ANTONIOND_HEIGHT (24)

static void Draw_2D_Signs(int screen)
{
	u8 * buf = S3D_BufferGet(screen);

	// Pong 3DS
	{
		int xbase = (400-PONG_3DS_WIDTH)/2;
		int ybase = (240-PONG_3DS_HEIGHT) - xbase;
		_quad_blit_unsafe_32(buf,pong_3ds_png_bin,xbase,ybase,
							 PONG_3DS_WIDTH,PONG_3DS_HEIGHT);
	}
	
	// By AntonioND
	{
		int xbase = 400-BY_ANTONIOND_WIDTH-10;
		int ybase = 10;
		_quad_blit_unsafe_real_alpha_32(buf,by_antoniond_png_bin,xbase,ybase,
							 BY_ANTONIOND_WIDTH,BY_ANTONIOND_HEIGHT);
	}
}

//--------------------------------------------------------------------------------------------------

static int cube_rotation_x, cube_rotation_y, cube_rotation_z;
static int cube_x, cube_y, cube_z;
static int cube_anim_frame;

//--------------------------------------------------------------------------------------------------

static inline void _draw_3d_cube(int screen)
{
	m44 m;
	m44_CreateTranslation(&m,  cube_x,cube_y, float2fx(2.5) + cube_z);
	S3D_ModelviewMatrixSet(screen, &m);	
	m44_CreateRotationZ(&m,cube_rotation_z);
	S3D_ModelviewMatrixMultiply(screen, &m);
	m44_CreateRotationY(&m,cube_rotation_y);
	S3D_ModelviewMatrixMultiply(screen, &m);
	m44_CreateRotationX(&m,cube_rotation_x);
	S3D_ModelviewMatrixMultiply(screen, &m);
	
	int r,g,b;
	HSV2RGB(cube_anim_frame*9,int2fx(1),int2fx(1),&r,&g,&b);
	
	_draw_cube(screen, float2fx(0.2),float2fx(0.2),float2fx(0.2), r,g,b);
}

static void Draw_3D_Cube_Far(int screen)
{
	if(cube_z > float2fx(0)) _draw_3d_cube(screen);
}

static void Draw_3D_Cube_Near(int screen)
{
	if(cube_z <= float2fx(0)) _draw_3d_cube(screen);
}

//--------------------------------------------------------------------------------------------------

#define NUM_QUADS (150)

typedef struct {
	int x,y,z; // position
	int vy;
	
	int rz; // rotation
	int vrz;
	
	int r,g,b;
} _quad_s;
static _quad_s Quad[NUM_QUADS];

static void Quad_Reset(int index)
{
	_quad_s * q = &(Quad[index]);
	
	q->x = (fast_rand() & (int2fx(4)-1)) - int2fx(2); // -2 ~ 2
	q->y = float2fx(2.0);
	q->z = (fast_rand() & (int2fx(4)-1)) - int2fx(1); // -1 ~ 3
	
	q->vy = -(  (fast_rand() & ((int2fx(1)>>6)-1) ) + float2fx(0.01)  );
	
	q->r = fast_rand() | ( ((fast_rand()&7) == 0) ? 255 : 0 );
	q->g = fast_rand() | ( ((fast_rand()&7) == 0) ? 255 : 0 );
	q->b = fast_rand() | ( ((fast_rand()&7) == 0) ? 255 : 0 );
	
	q->rz = 0;
	q->vrz = (fast_rand() & 0x3FF) + 0x100;
	if(fast_rand()&1) q->vrz = -q->vrz;
}

static void Draw_2D_Quads_Far(int screen)
{
	S3D_PolygonBegin(screen, S3D_QUADS);
	
	int i;
	for(i = 0; i < NUM_QUADS; i++)
	{
		_quad_s * q = &(Quad[i]);
		
		if(q->z > 0)
		{
			m44 m;
			m44_CreateTranslation(&m,  q->x,q->y,float2fx(2.5)+q->z);
			S3D_ModelviewMatrixSet(screen, &m);	
			m44_CreateRotationZ(&m,q->rz);
			S3D_ModelviewMatrixMultiply(screen, &m);
			
			S3D_PolygonColor(screen, q->r,q->g,q->b);
			
			S3D_PolygonVertex(screen, float2fx(+0.1),float2fx(+0.1),0);
			S3D_PolygonVertex(screen, float2fx(+0.1),float2fx(-0.1),0);
			S3D_PolygonVertex(screen, float2fx(-0.1),float2fx(-0.1),0);
			S3D_PolygonVertex(screen, float2fx(-0.1),float2fx(+0.1),0);
		}
	}
}

static void Draw_2D_Quads_Near(int screen)
{
	S3D_PolygonBegin(screen, S3D_QUADS);
	
	int i;
	for(i = 0; i < NUM_QUADS; i++)
	{
		_quad_s * q = &(Quad[i]);
		
		if(q->z < 0)
		{
			m44 m;
			m44_CreateTranslation(&m,  q->x,q->y,float2fx(2.5)+q->z);
			S3D_ModelviewMatrixSet(screen, &m);	
			m44_CreateRotationZ(&m,q->rz);
			S3D_ModelviewMatrixMultiply(screen, &m);
			
			S3D_PolygonColor(screen, q->r,q->g,q->b);
			
			S3D_PolygonVertex(screen, float2fx(+0.1),float2fx(+0.1),0);
			S3D_PolygonVertex(screen, float2fx(+0.1),float2fx(-0.1),0);
			S3D_PolygonVertex(screen, float2fx(-0.1),float2fx(-0.1),0);
			S3D_PolygonVertex(screen, float2fx(-0.1),float2fx(+0.1),0);
		}
	}
}

//--------------------------------------------------------------------------------------------------

static void CubeQuads_Handle(void)
{
	cube_x = fxmul(fxsin(3*(cube_anim_frame<<6)),float2fx(0.35));
	cube_y = fxmul(fxsin(5*(cube_anim_frame<<6)),float2fx(0.25));
	cube_z = fxmul(fxcos(8*(cube_anim_frame<<6)),float2fx(1.0));

	cube_rotation_z += 0x100;
	cube_rotation_y -= 0x200;
	cube_rotation_x += 0x300;
	
	cube_anim_frame ++;
	
	int i;
	for(i = 0; i < NUM_QUADS; i++)
	{
		_quad_s * q = &(Quad[i]);
		
		q->y += q->vy;
		q->rz += q->vrz;
		
		if(q->y < float2fx(-2.0)) Quad_Reset(i);
	}
}


static void CubeQuads_Init(void)
{
	cube_rotation_z = cube_rotation_y = cube_rotation_x = 0;
	cube_anim_frame = 0;
	
	int i;
	for(i = 0; i < NUM_QUADS; i++)
	{
		Quad_Reset(i);
		Quad[i].y = ( fast_rand() & (int2fx(4)-1) ) - int2fx(2);
	}
}

//--------------------------------------------------------------------------------------------------

void Room_Menu_Draw(int screen)
{
	//Configuration
	{
		S3D_SetCulling(screen, 1,0);
		S3D_LightAmbientColorSet(screen, 64,64,64);
		
		S3D_LightEnable(screen, S3D_LIGHT_N(0));
		S3D_LightDirectionalColorSet(screen, 0, 192,192,192);
		S3D_LightDirectionalVectorSet(screen, 0, float2fx(-0.38),float2fx(-0.76),float2fx(0.53));
	}
	
	Draw_3D_Cube_Far(screen);
	Draw_2D_Quads_Far(screen);
	S3D_PolygonListFlush(screen, 1);
	
	Draw_2D_Signs(screen);
	
	Draw_3D_Cube_Near(screen);
	Draw_2D_Quads_Near(screen);
	S3D_PolygonListFlush(screen, 1);
}

//--------------------------------------------------------------------------------------------------

#include "bottom_screen_menu_png_bin.h"
#include "bottom_screen_credits_1_png_bin.h"
#include "bottom_screen_credits_2_png_bin.h"
#include "bottom_screen_credits_3_png_bin.h"

static int selected_option = 0;
static int credits = 0;

void Room_Menu_Draw_Bottom(void)
{
	u8 * buf = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	
	if(credits)
	{
		if(credits == 1)
			FastFramebufferCopy(bottom_screen_credits_1_png_bin,buf,GFX_BOTTOM);
		else if(credits == 2)
			FastFramebufferCopy(bottom_screen_credits_2_png_bin,buf,GFX_BOTTOM);
		else if(credits == 3)
			FastFramebufferCopy(bottom_screen_credits_3_png_bin,buf,GFX_BOTTOM);
	}
	else
	{
		FastFramebufferCopy(bottom_screen_menu_png_bin,buf,GFX_BOTTOM);
	
		switch(selected_option)
		{
			case 1:
				S3D_2D_QuadAllignedFillAlpha(buf,  16,240-16-1,  151,240-55-1,  255,0,0, 128);
				break;
			case 2:
				S3D_2D_QuadAllignedFillAlpha(buf,  168,240-16-1,  303,240-55-1,  255,0,0, 128);
				break;
			case 3:
				S3D_2D_QuadAllignedFillAlpha(buf,   16,240-72-1,  151,240-111-1,  255,0,0, 128);
				break;
			case 4:
				S3D_2D_QuadAllignedFillAlpha(buf,  168,240-72-1,  303,240-111-1,  255,0,0, 128);
				break;
			default:
				break;
		}
	}
}

//--------------------------------------------------------------------------------------------------

void Room_Menu_GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax)
{
	if(xmin) *xmin = 0;
	if(xmax) *xmax = 0;
	if(ymin) *ymin = 0;
	if(ymax) *ymax = 0;
	if(zmin) *zmin = 0;
	if(zmax) *zmax = 0;
}

//--------------------------------------------------------------------------------------------------

#include "kaos_och_dekadens_mod_bin.h"

void Room_Menu_Init(void)
{
	Game_Pause(0);
	CubeQuads_Init();
	selected_option = 0;
	credits = 0;
	
	Sound_Play(kaos_och_dekadens_mod_bin,kaos_och_dekadens_mod_bin_size);
}

void Room_Menu_End(void)
{
	Sound_Stop();
}

void Room_Menu_Handle(void)
{
	CubeQuads_Handle();
	
	if(credits)
	{
		int keys = hidKeysUp();
		if(keys & KEY_TOUCH)
		{
			credits ++;
			if(credits == 4)
				credits = 0;
		}
	}
	else
	{
		int keys = hidKeysHeld();
		if(keys & KEY_TOUCH)
		{
			touchPosition tp;
			hidTouchRead(&tp);
			
			selected_option = 0;
			
			if( (tp.px>=16) && (tp.px<=151) )
			{
				if( (tp.py>=16) && (tp.py<=55) )
					selected_option = 1;
				else if( (tp.py>=72) && (tp.py<=111) )
					selected_option = 3;
			}
			else if( (tp.px>=168) && (tp.px<=303) )
			{
				if( (tp.py>=16) && (tp.py<=55) )
					selected_option = 2;
				else if( (tp.py>=72) && (tp.py<=111) )
					selected_option = 4;
			}
		}
		
		keys = hidKeysUp();
		if(keys & KEY_TOUCH)
		{
			switch(selected_option)
			{
				case 1:
					Room_SetNumber(GAME_ROOM_1);
					break;
				case 2:
					Room_SetNumber(GAME_ROOM_2);
					break;
				case 3:
					Room_SetNumber(GAME_ROOM_3);
					break;
				case 4:
					credits = 1;
					break;
				default:
					break;
			}
			
			selected_option = 0;
		}
	}
	
}

_3d_mode_e Room_Menu_3DMode(void)
{
	return GAME_MODE_2D;
}

//--------------------------------------------------------------------------------------------------
