
//----------------------------------------------------------------------------

#include "engine.h"

//----------------------------------------------------------------------------

m44 S3D_GLOBAL_MATRIX; // global variable

static m44 PROJECTION, MODELVIEW;
static u32 global_updated = 0;

static m44 matrix_stack[8];
static s32 matrix_stack_ptr = 0;

//----------------------------------------------------------------------------

void S3D_ProyectionMatrixSet(m44 * m)
{
	m44_Copy(m,&PROJECTION);
	global_updated = 0;
}

//----------------------------------------------------------------------------

void S3D_ModelviewMatrixSet(m44 * m)
{
	m44_Copy(m,&MODELVIEW);
	global_updated = 0;
}

void S3D_ModelviewMatrixPush(void)
{
	//if(matrix_stack_ptr >= 8) while(1);
	m44_Copy(&MODELVIEW,&(matrix_stack[matrix_stack_ptr]));
	matrix_stack_ptr++;
}

void S3D_ModelviewMatrixPop(void)
{
	//if(matrix_stack_ptr < 0) while(1);
	matrix_stack_ptr--;
	m44_Copy(&(matrix_stack[matrix_stack_ptr]),&MODELVIEW);
	global_updated = 0;
}

void S3D_ModelviewMatrixMultiply(m44 * m)
{
	m44 temp;
	m44_Copy(&MODELVIEW,&temp);
	m44_Multiply(&temp,m,&MODELVIEW);
	global_updated = 0;
}

//----------------------------------------------------------------------------

inline void _s3d_global_matrix_update(void)
{
	if(global_updated) return;
	global_updated = 1;
	m44_Multiply(&PROJECTION,&MODELVIEW,&S3D_GLOBAL_MATRIX);
}

//----------------------------------------------------------------------------
