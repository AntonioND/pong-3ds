
#ifndef __ENGINE__
#define __ENGINE__

#include "matrix.h"
#include "plot.h"
#include "draw.h"

void proyection_matrix_set(m44 * m);

void modelview_matrix_set(m44 * m);
void modelview_matrix_multiply(m44 * m);
void modelview_matrix_push(void);
void modelview_matrix_pop(void);

#endif //__ENGINE__

