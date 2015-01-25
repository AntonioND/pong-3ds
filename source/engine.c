
#include <stdlib.h>
#include <string.h>

#include "engine.h"

m44 PROJECTION, MODELVIEW, GLOBAL;
u32 global_updated = 0;

void proyection_matrix_set(m44 * m)
{
	memcpy((void*)&PROJECTION,(const void*)m,sizeof(m44));
	global_updated = 0;
}

void modelview_matrix_set(m44 * m)
{
	memcpy((void*)&MODELVIEW,(const void*)m,sizeof(m44));
	global_updated = 0;
}

m44 matrix_stack[8];
s32 matrix_stack_ptr = 0;

void modelview_matrix_push(void)
{
	//if(matrix_stack_ptr >= 8) while(1);
	m44_copy(&MODELVIEW,&(matrix_stack[matrix_stack_ptr]));
	matrix_stack_ptr++;
}

void modelview_matrix_pop(void)
{
	//if(matrix_stack_ptr < 0) while(1);
	matrix_stack_ptr--;
	m44_copy(&(matrix_stack[matrix_stack_ptr]),&MODELVIEW);
	global_updated = 0;
}

void modelview_matrix_multiply(m44 * m)
{
	m44 temp;
	m44_copy(&MODELVIEW,&temp);
	m44_multiply(&temp,m,&MODELVIEW);
	global_updated = 0;
}


void __global_matrix_update(void)
{
	global_updated = 1;
	m44_multiply(&PROJECTION,&MODELVIEW,&GLOBAL);
}

