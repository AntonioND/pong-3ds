
#ifndef __PAD__
#define __PAD__

//--------------------------------------------------------------------------------------------------

#include "game.h"

//--------------------------------------------------------------------------------------------------

void Pad_P1SetDimensions(int x, int y, int z); // Player (z min)
void Pad_P2SetDimensions(int x, int y, int z); // AI (z max)

void Pad_P1SetColor(int r, int g, int b);
void Pad_P2SetColor(int r, int g, int b);

void Pad_P1Draw(int screen);
void Pad_P2Draw(int screen);

void Pad_P1GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax); // Player
void Pad_P2GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax); // AI

void Pad_P1GetPosition(int * x, int * y, int * z);
void Pad_P2GetPosition(int * x, int * y, int * z);

void Pad_InitAll(void);
void Pad_ResetAll(void);
void Pad_HandleAll(void);

//--------------------------------------------------------------------------------------------------

#endif //__PAD__
