
//----------------------------------------------------------------------------

#include "engine.h"

//----------------------------------------------------------------------------

m44 S3D_GLOBAL_MATRIX[2]; // global variable
m44 PROJECTION[2], MODELVIEW[2];
static u32 global_updated[2] = { 0, 0 };

#define MATRIX_STACK_SIZE (8)
static m44 matrix_stack[2][MATRIX_STACK_SIZE];
static s32 matrix_stack_ptr[2] = { 0, 0 };

//----------------------------------------------------------------------------

void S3D_ProjectionMatrixSet(int screen, m44 * m)
{
	m44_Copy(m,&PROJECTION[screen]);
	global_updated[screen] = 0;
}

//----------------------------------------------------------------------------

void S3D_ModelviewMatrixSet(int screen, m44 * m)
{
	m44_Copy(m,&MODELVIEW[screen]);
	global_updated[screen] = 0;
}

void S3D_ModelviewMatrixPush(int screen)
{
	//if(matrix_stack_ptr[screen] >= MATRIX_STACK_SIZE) while(1);
	m44_Copy(&MODELVIEW[screen],&(matrix_stack[screen][matrix_stack_ptr[screen]]));
	matrix_stack_ptr[screen]++;
}

void S3D_ModelviewMatrixPop(int screen)
{
	//if(matrix_stack_ptr[screen] < 0) while(1);
	matrix_stack_ptr[screen]--;
	m44_Copy(&(matrix_stack[screen][matrix_stack_ptr[screen]]),&MODELVIEW[screen]);
	global_updated[screen] = 0;
}

void S3D_ModelviewMatrixMultiply(int screen, m44 * m)
{
	m44 temp;
	m44_Copy(&MODELVIEW[screen],&temp);
	m44_Multiply(&temp,m,&MODELVIEW[screen]);
	global_updated[screen] = 0;
}

//----------------------------------------------------------------------------

inline void _s3d_global_matrix_update(int screen)
{
	if(global_updated[screen]) return;
	global_updated[screen] = 1;
	m44_Multiply(&PROJECTION[screen],&MODELVIEW[screen],&S3D_GLOBAL_MATRIX[screen]);
}

//----------------------------------------------------------------------------
