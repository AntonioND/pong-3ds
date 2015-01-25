
#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "engine.h"

#define CONFIG_3D_SLIDERSTATE (*(float*)0x1FF81080) 

//----------------------------------------------------------------------------------

#define TICKS_PER_SEC (268123480)

int frame_count;
u64 last_ticks = 0;
int _FPS;

static void FPS_IncreaseCount(void)
{
	frame_count ++;
}

static void FPS_UpdateValue(void)
{
	if(svcGetSystemTick() >= last_ticks + TICKS_PER_SEC)
	{
		last_ticks = svcGetSystemTick();
		_FPS = frame_count;
		frame_count = 0;	
	}
}

static int FPS_Get(void)
{
	return _FPS;
}

//----------------------------------------------------------------------------------

//Xorshift RNGs, by George Marsaglia (The Florida State University)

#define FAST_RAND_MAX (0x7FFF)

static uint64_t x=123456789,y=362436069,z=521288629,w=88675123;

static inline uint64_t xor128(void)
{
    uint64_t t;
    t=(x^(x<<11)); x=y; y=z; z=w;
    return ( w=(w^(w>>19))^(t^(t>>8)) );
}

inline void fast_srand(uint64_t seed)
{
    x = seed; // Some way to seed the RNG is needed, even in the paper is not specified...
}

inline uint32_t fast_rand(void)
{
    return xor128()&FAST_RAND_MAX;
}

//-------------------------------------------------------------------------------------------------------

typedef struct {
	s32 x,vx,ax;
	
	u32 x_col_old, z_col_old;
	u32 x_col, z_col;
} _pad_t;

typedef struct {
	s32 x,vx;
	s32 z,vz;
} _ball_t;

_pad_t pad, pad_ia;

_ball_t ball;

void draw_scenario(int r, int g, int b)
{
	set_current_color(r,g,b);

	polygon_begin(P_QUAD_STRIP);
	
	polygon_normal(float2fx(0.0),float2fx(1.0),float2fx(0.0));
	
	polygon_vertex(float2fx(-7),float2fx(-1),float2fx(10.5)); 			
	polygon_vertex(float2fx(-7),float2fx(-1),float2fx(-0.5)); 			
	polygon_vertex(float2fx(7),float2fx(-1),float2fx(-0.5)); 			
	polygon_vertex(float2fx(7),float2fx(-1),float2fx(10.5)); 
}

void draw_ball(int r, int g, int b)
{
	set_current_color(r,g,b);
	
	polygon_begin(P_QUAD_STRIP);
	
	polygon_normal(float2fx(-1.0),float2fx(0.0),float2fx(0.0));
	
	polygon_vertex(float2fx(-0.75),float2fx(-1),float2fx(-0.75));
	polygon_vertex(float2fx(-0.75),float2fx(-1),float2fx(0.75));
	polygon_vertex(float2fx(-0.75),float2fx(0.5),float2fx(0.75)); 
	polygon_vertex(float2fx(-0.75),float2fx(0.5),float2fx(-0.75));

	polygon_normal(float2fx(0.0),float2fx(1.0),float2fx(0.0));

	polygon_vertex(float2fx(0.75),float2fx(0.5),float2fx(0.75)); 			
	polygon_vertex(float2fx(0.75),float2fx(0.5),float2fx(-0.75));
	
	polygon_begin(P_QUAD_STRIP);

	polygon_normal(float2fx(0.0),float2fx(0.0),float2fx(-1.0));
	
	polygon_vertex(float2fx(-0.75),float2fx(-1),float2fx(-0.75));
	polygon_vertex(float2fx(-0.75),float2fx(0.5),float2fx(-0.75));
	polygon_vertex(float2fx(0.75),float2fx(0.5),float2fx(-0.75));
	polygon_vertex(float2fx(0.75),float2fx(-1),float2fx(-0.75));

	polygon_normal(float2fx(1.0),float2fx(0.0),float2fx(0.0));

	polygon_vertex(float2fx(0.75),float2fx(0.5),float2fx(0.75));
	polygon_vertex(float2fx(0.75),float2fx(-1),float2fx(0.75));
	
	polygon_normal(float2fx(0.0),float2fx(0.0),float2fx(1.0));

	polygon_vertex(float2fx(-0.75),float2fx(0.5),float2fx(0.75));
	polygon_vertex(float2fx(-0.75),float2fx(-1),float2fx(0.75));
}

void draw_pad(int r, int g, int b)
{
	set_current_color(r,g,b);
	
	polygon_begin(P_QUAD_STRIP);
	
	polygon_normal(float2fx(-1.0),float2fx(0.0),float2fx(0.0));
	
	polygon_vertex(float2fx(-2),float2fx(-1),float2fx(-0.5));
	polygon_vertex(float2fx(-2),float2fx(-1),float2fx(0.5));
	polygon_vertex(float2fx(-2),float2fx(1),float2fx(0.5)); 
	polygon_vertex(float2fx(-2),float2fx(1),float2fx(-0.5));
	
	polygon_normal(float2fx(0.0),float2fx(1.0),float2fx(0.0));
	
	polygon_vertex(float2fx(2),float2fx(1),float2fx(0.5)); 			
	polygon_vertex(float2fx(2),float2fx(1),float2fx(-0.5));
	
	polygon_begin(P_QUAD_STRIP);
	
	polygon_normal(float2fx(0.0),float2fx(0.0),float2fx(-1.0));
	
	polygon_vertex(float2fx(-2),float2fx(-1),float2fx(-0.5));
	polygon_vertex(float2fx(-2),float2fx(1),float2fx(-0.5));
	polygon_vertex(float2fx(2),float2fx(1),float2fx(-0.5));
	polygon_vertex(float2fx(2),float2fx(-1),float2fx(-0.5));
	
	polygon_normal(float2fx(1.0),float2fx(0.0),float2fx(0.0));

	polygon_vertex(float2fx(2),float2fx(1),float2fx(0.5));
	polygon_vertex(float2fx(2),float2fx(-1),float2fx(0.5));
	
	polygon_normal(float2fx(0.0),float2fx(0.0),float2fx(1.0));

	polygon_vertex(float2fx(-2),float2fx(1),float2fx(0.5));
	polygon_vertex(float2fx(-2),float2fx(-1),float2fx(0.5));
}

static inline u32 _segments_overlap(s32 amin, s32 amax, s32 bmin, s32 bmax)
{
	if(amin < bmin) { if(amax < bmin) return 0; } //completely out (left)
	else if(amin > bmax) return 0; // completely out (right)
	return 1; //partially or totally overlapping
}


void DrawScene()
{
	m44 m;
	m44_create_translation(&m,0,int2fx(-2),int2fx(12));
	modelview_matrix_set(&m);
	m44_create_rotation_x(&m,-0x1000);
	modelview_matrix_multiply(&m);
	
	{
		lights_enable(LIGHT_N(0)|LIGHT_N(1));
		light_set_color(0, 255,255,255);
		light_set_color(1, 255,255,255);
		light_set_dir(0, float2fx(0.6),float2fx(-0.6),float2fx(0.6));
		light_set_dir(1, float2fx(-0.6),float2fx(-0.6),float2fx(0.6));
		
		//---------------------------------------------------
		//                 Draw scenario
		//---------------------------------------------------
		
		//There is no zbuffer, so let's draw the models from farthest to nearest.
		
		// Camera rotation effect...
		m44_create_rotation_y(&m,pad.x);
		//m44_create_rotation_axis(&m, angle1, float2fx(0.58), float2fx(0.58), float2fx(0.58));
		modelview_matrix_multiply(&m);
		
		m44_create_rotation_z(&m,-pad.x);
		modelview_matrix_multiply(&m);
		
		draw_scenario(0,0,255);
		
		//---------------------------------------------------
		//                     IA
		//---------------------------------------------------
		
		modelview_matrix_push();
		
		m44_create_translation(&m,pad_ia.x,0,int2fx(10));
		modelview_matrix_multiply(&m);
		
		draw_pad(255,0,0);
		
		modelview_matrix_pop();
		
		//---------------------------------------------------
		//                     Ball
		//---------------------------------------------------
		
		modelview_matrix_push();
		
		m44_create_translation(&m,ball.x,0,ball.z);
		modelview_matrix_multiply(&m);
		
		draw_ball(128,128,128);
		
		modelview_matrix_pop();
		
		//---------------------------------------------------
		//                     Player
		//---------------------------------------------------
		
		modelview_matrix_push();
		
		m44_create_translation(&m,pad.x,0,0);
		modelview_matrix_multiply(&m);
		
		draw_pad(0,255,0);
		
		modelview_matrix_pop();
	}
}


//Far is > 2.5 z
//Screen is at 2.5 z
//Over screen is < 2.5 z

#define TRANSL_DIV_FACTOR (4.0f)
#define SHEAR_DIV_FACTOR  (2.0f)

void DrawLeft(void)
{
	float slider = CONFIG_3D_SLIDERSTATE;
	int transl = float2fx(slider/TRANSL_DIV_FACTOR);
	int shear = float2fx(slider/SHEAR_DIV_FACTOR);
	
	m44 p, t, r;
	m44_create_frustum(&p, -float2fx(2.5)+shear, float2fx(2.5)+shear, -float2fx(1.5), float2fx(1.5), float2fx(5), float2fx(10));
	m44_create_translation(&t,-transl,0,0);
	m44_multiply(&p,&t,&r);
	proyection_matrix_set(&r);
	
	set_current_buffer(GFX_LEFT);
	DrawScene();
}

void DrawRight(void)
{
	float slider = CONFIG_3D_SLIDERSTATE;
	if(slider == 0.0f) return;
	int transl = float2fx(slider/TRANSL_DIV_FACTOR);
	int shear = float2fx(slider/SHEAR_DIV_FACTOR);	

	m44 p, t, r;
	m44_create_frustum(&p, -float2fx(2.5)-shear, float2fx(2.5)-shear, -float2fx(1.5), float2fx(1.5), float2fx(5), float2fx(10));
	m44_create_translation(&t,+transl,0,0);
	m44_multiply(&p,&t,&r);
	proyection_matrix_set(&r);
	
	set_current_buffer(GFX_RIGHT);
	DrawScene();
}


//-------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	gfxInitDefault();
	gfxSet3D(true); //OMG 3D!!!!1!!!
	consoleInit(GFX_BOTTOM, NULL); //Initialize console on bottom screen.
	
	//To move the cursor you have tu print "\x1b[r;cH", where r and c are respectively
	//the row and column where you want your cursor to move
	//The top screen has 30 rows and 50 columns
	//The bottom screen has 30 rows and 40 columns
	printf("\x1b[0;16HPong 3DS");
	printf("\x1b[2;14Hby AntonioND");

	printf("\x1b[29;10HPress Start to exit.");
	
	fast_srand(svcGetSystemTick());
	
	pad.x = 0; pad.vx = 0; pad.ax = 0;
	pad.x_col_old = 0; pad.z_col_old = 0; pad.x_col = 0; pad.z_col = 0;
	pad_ia.x = 0; pad_ia.vx = 0; pad_ia.ax = 0;
	pad_ia.x_col_old = 0; pad_ia.z_col_old = 0; pad_ia.x_col = 0; pad_ia.z_col = 0;
	
	ball.x = int2fx(0);
	ball.z = int2fx(5);	
	ball.vx = (int2fx(1)>>4) + (fast_rand() & ((int2fx(1)>>3)-1));
	ball.vx = fast_rand() & 1 ? ball.vx : -ball.vx;
	ball.vz = (int2fx(1)>>3) + (fast_rand() & ((int2fx(1)>>3)-1));
	ball.vz = fast_rand() & 1 ? ball.vz : -ball.vz;
	
	// Main loop
	while(aptMainLoop())
	{
		FPS_UpdateValue();
		hidScanInput();
		
		int keys = hidKeysHeld();
		if(keys & KEY_START) break; // break in order to return to hbmenu
		
		clear_screen_buffers();
		
		DrawLeft();
		DrawRight();
		
		printf("\x1b[8;5H3D Slider: %f        ",CONFIG_3D_SLIDERSTATE);
		printf("\x1b[10;5HFPS: %d  ",FPS_Get());
		
		{
			//---------------------------------------------------
			//                 Handle game
			//---------------------------------------------------
			
			//Player
			
			if(keys & KEY_RIGHT)
			{
				if(pad.vx < 0) pad.vx = 0;
				else if(pad.vx < 128) pad.vx += 32;
			}
			else if(keys & KEY_LEFT)
			{
				if(pad.vx > 0) pad.vx = 0;
				else if(pad.vx > -128) pad.vx -= 32;
			}
			else
			{
				if(pad.vx > 0) pad.vx -= 16; 
				else if(pad.vx < 0) pad.vx += 16;
			}
			pad.x += pad.vx;
			if(pad.x < int2fx(-5)) { pad.x = int2fx(-5); pad.vx = 0; }
			else if(pad.x > int2fx(5)) { pad.x = int2fx(5); pad.vx = 0; }
			
			//IA
			
			if(ball.z > int2fx(3) && ball.vz > 0)
			{
				if((pad_ia.x - ball.x) < int2fx(-1))
				{
					if(pad_ia.vx < 0) pad_ia.vx = 0;
					else if(pad_ia.vx < 128) pad_ia.vx += 32;
				}
				else if((pad_ia.x - ball.x) > int2fx(1)) 
				{
					if(pad_ia.vx > 0) pad_ia.vx = 0;
					else if(pad_ia.vx > -128) pad_ia.vx -= 32;
				}
				else
				{
					if(pad_ia.vx > 0) pad_ia.vx -= 16; 
					else if(pad_ia.vx < 0) pad_ia.vx += 16;
				}
			}
			else
			{
				if(pad_ia.vx > 0) pad_ia.vx -= 16; 
				else if(pad_ia.vx < 0) pad_ia.vx += 16;
			}
			pad_ia.x += pad_ia.vx;
			if(pad_ia.x < int2fx(-5)) { pad_ia.x = int2fx(-5); pad_ia.vx = 0; }
			else if(pad_ia.x > int2fx(5)) { pad_ia.x = int2fx(5); pad_ia.vx = 0; }
			
			//Ball
			
			pad.x_col_old = pad.x_col; pad.z_col_old = pad.z_col;
			pad_ia.x_col_old = pad_ia.x_col; pad_ia.z_col_old = pad_ia.z_col;
			
			ball.x += ball.vx;
			if(abs(ball.x) > float2fx(6.25)) ball.vx = -ball.vx;
			ball.z += ball.vz;
			
			pad.z_col = 0;
			pad_ia.z_col = 0;
			pad.x_col = 0;
			pad_ia.x_col = 0;
			
			if(_segments_overlap(ball.z-float2fx(0.75),ball.z+float2fx(0.75),float2fx(-0.5),float2fx(0.5)))
			{
				pad.z_col = 1;
			}
			else if(_segments_overlap(ball.z-float2fx(0.75),ball.z+float2fx(0.75),float2fx(10-0.5),float2fx(10+0.5)))
			{
				pad_ia.z_col = 1;
			}
			
			if(_segments_overlap(ball.x-float2fx(0.75),ball.x+float2fx(0.75),
						pad.x-float2fx(2),pad.x+float2fx(2)))
			{
				pad.x_col = 1;
			}
			
			if(_segments_overlap(ball.x-float2fx(0.75),ball.x+float2fx(0.75),
						pad_ia.x-float2fx(2),pad_ia.x+float2fx(2)))
			{
				pad_ia.x_col = 1;
			}
			
			if(pad.x_col && pad.z_col)
			{
				if(pad.x_col ^ pad.x_col_old) ball.vx = -ball.vx;
				if(pad.z_col ^ pad.z_col_old) ball.vz = -ball.vz;
			}
			else if(pad_ia.x_col && pad_ia.z_col)
			{
				if(pad_ia.x_col ^ pad_ia.x_col_old) ball.vx = -ball.vx;
				if(pad_ia.z_col ^ pad_ia.z_col_old) ball.vz = -ball.vz;
			}
			
			if(ball.z > float2fx(10.375)) ball.vz = -ball.vz;
			else if(ball.z < float2fx(-0.375)) ball.vz = -ball.vz;
			
			//if(ball.z > float2fx(10.375)) score_player ++;
			//else if(ball.z < float2fx(-0.375)) score_ia ++;
			
			//---------------------------------------------------
			//
			//---------------------------------------------------
		}
		
		FPS_IncreaseCount();
		
		flush_screen_buffers();
		
		gspWaitForVBlank();
	}

	gfxExit();
	return 0;
}








	


	
