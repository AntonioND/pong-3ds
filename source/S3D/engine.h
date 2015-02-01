
#ifndef __ENGINE__
#define __ENGINE__

//----------------------------------------------------------------------------
//-                      Software 3D engine. Main file.                      -
//----------------------------------------------------------------------------

#include "matrix.h"
#include "draw.h"
#include "polygon.h"

//----------------------------------------------------------------------------

void S3D_ProjectionMatrixSet(int screen, m44 * m);

//----------------------------------------------------------------------------

void S3D_ModelviewMatrixSet(int screen, m44 * m);
void S3D_ModelviewMatrixMultiply(int screen, m44 * m);
void S3D_ModelviewMatrixPush(int screen);
void S3D_ModelviewMatrixPop(int screen);

//----------------------------------------------------------------------------

#endif //__ENGINE__

