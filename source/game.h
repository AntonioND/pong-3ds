
#ifndef __GAME__
#define __GAME__

//--------------------------------------------------------------------------------------------------

#define COLLISION_X_MIN (1)
#define COLLISION_X_MAX (2)
#define COLLISION_Y_MIN (4)
#define COLLISION_Y_MAX (8)
#define COLLISION_Z_MIN (16)
#define COLLISION_Z_MAX (32)

//--------------------------------------------------------------------------------------------------

typedef enum {
	GAME_ROOM_MENU,
	GAME_ROOM_1
} _game_room_e;

//--------------------------------------------------------------------------------------------------

void Game_Init(void);

void Game_Handle(void);

void Game_DrawScreenTop(int screen);
void Game_DrawScreenBottom(void);

void Game_End(void);

//--------------------------------------------------------------------------------------------------

// returns how much the segments overlap
static inline s32 _segments_overlap(s32 amin, s32 amax, s32 bmin, s32 bmax)
{
	if(amin < bmin)
	{
		if(amax < bmin) return 0;
		else if(amax < bmax) return amax - bmin;
		else return bmax - bmin;
	}
	else
	{
		if(bmax < amin) return 0;
		else if(bmax < amax) return bmax - amin;
		else return amax - amin;
	}
}

//--------------------------------------------------------------------------------------------------

#endif //__GAME__

