
#ifndef __PAD__
#define __PAD__

//--------------------------------------------------------------------------------------------------

void Pad_P1SetDimensions(int x, int y, int z); // Player
void Pad_P2SetDimensions(int x, int y, int z); // AI

void Pad_P1SetColor(int r, int g, int b);
void Pad_P2SetColor(int r, int g, int b);

void Pad_P1Draw(int screen);
void Pad_P2Draw(int screen);

void Pad_P1GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax);
void Pad_P2GetBounds(int * xmin, int * xmax, int * ymin, int * ymax, int * zmin, int * zmax);

void Pad_InitAll(void);
void Pad_HandleAll(void);

//--------------------------------------------------------------------------------------------------

#endif //__PAD__
