
#ifndef __ENGINE__
#define __ENGINE__

//----------------------------------------------------------------------------
//-                      Software 3D engine. Main file.                      -
//----------------------------------------------------------------------------

#include "matrix.h"
#include "draw.h"
#include "polygon.h"

//----------------------------------------------------------------------------

void S3D_ProjectionMatrixSet(m44 * m);

//----------------------------------------------------------------------------

void S3D_ModelviewMatrixSet(m44 * m);
void S3D_ModelviewMatrixMultiply(m44 * m);
void S3D_ModelviewMatrixPush(void);
void S3D_ModelviewMatrixPop(void);

//----------------------------------------------------------------------------

#endif //__ENGINE__

