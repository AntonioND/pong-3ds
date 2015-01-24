
#include <string.h>

#include "matrix.h"

void v4_cross_product(v4 * v1, v4 * v2, v4 * res)
{
	// |  i  j  k |
	// | 1x 1y 1z |
	// | 2x 2y 2z |
	
	ptr_V4SET(res,
		fxmul(ptr_V4(v1,1),ptr_V4(v2,2)) - fxmul(ptr_V4(v1,2),ptr_V4(v2,1)),
		-fxmul(ptr_V4(v1,0),ptr_V4(v2,2)) + fxmul(ptr_V4(v1,2),ptr_V4(v2,0)),
		fxmul(ptr_V4(v1,0),ptr_V4(v2,1)) - fxmul(ptr_V4(v1,1),ptr_V4(v2,0)) );
}

//----------------------------------------------------------------------------

void m44_copy(m44 * src, m44 * dest)
{
	memcpy((void*)dest,(const void*)src,sizeof(m44)); 
}

void m44_identity(m44 * m)
{
	ptr_M44SET(m,	int2fx(1), int2fx(0), int2fx(0), int2fx(0),
					int2fx(0), int2fx(1), int2fx(0), int2fx(0),
					int2fx(0), int2fx(0), int2fx(1), int2fx(0),
					int2fx(0), int2fx(0), int2fx(0), int2fx(1)  );
}

void m44_add(m44 * m1, m44 * m2, m44 * dest)
{
	u32 i;
	for(i = 0; i < 16; i++)
		(*dest)[i] = (*m1)[i] + (*m2)[i];
}

void m44_multiply(m44 * m1, m44 * m2, m44 * dest)
{
	u32 row,col;
	for(col = 0; col < 4; col++) for(row = 0; row < 4; row++)
	{
		s32 sum = 0;
		u32 i;
		for(i = 0; i < 4; i++) 
			sum += fxmul64( ptr_M44(m1,row,i) , ptr_M44(m2,i,col) );
		ptr_M44(dest,row,col) = sum;
	}
}

//----------------------------------------------------------------------------

void m44_v4_multiply(m44 * m, v4 * v, v4 * dest)
{
	u32 row;
	for(row = 0; row < 4; row++)
	{
		s32 sum = 0;
		u32 i;
		for(i = 0; i < 4; i++) 
			sum += fxmul64( ptr_M44(m,row,i) , ptr_V4(v,i) );
		ptr_V4(dest,row) = sum;
	}
}

//----------------------------------------------------------------------------

void m44_create_ortho(m44 * m,u32 width, u32 height)
{
	m44 screen, ortho;
	M44SET(ortho,
	 fxdiv(float2fx(2),width),                 int2fx(0),  int2fx(0), int2fx(0),
	                int2fx(0), fxdiv(float2fx(2),height),  int2fx(0), int2fx(0),
	                int2fx(0),                 int2fx(0),  int2fx(1), int2fx(0),
	                int2fx(0),                 int2fx(0),  int2fx(0), int2fx(1)  );
	M44SET(screen,
		int2fx(400)>>1,          0,         0,  int2fx(400)>>1,
		       0, -int2fx(240)>>1,         0, int2fx(240)>>1,
			   0,          0, int2fx(1),         0,
			   0,          0,         0,  int2fx(1));
	
	m44_multiply(&screen,&ortho,m);
}

void m44_create_perspective(m44 * m,u32 width, u32 height, s32 znear)
{
	m44 screen, perspective;
	
	M44SET(perspective,	
	 fxdiv(fxmul(float2fx(2),znear),width),                 int2fx(0),  int2fx(0), int2fx(0),
	                int2fx(0), fxdiv(fxmul(float2fx(2),znear),height),  int2fx(0), int2fx(0),
	                int2fx(0),                 int2fx(0),  int2fx(1), int2fx(0),
	                int2fx(0),                 int2fx(0),  int2fx(1), int2fx(0)  );
	M44SET(screen,
		int2fx(400)>>1,          0,        0, int2fx(400)>>1,
		       0, -int2fx(240)>>1,         0, int2fx(240)>>1,
			   0,          0, int2fx(1),         0,
			   0,          0,         0,  int2fx(1));
	
	m44_multiply(&screen,&perspective,m);
}

void m44_create_frustum(m44 * m, int left, int right, int bottom, int top, int znear, int zfar)
{
	m44 screen, perspective;
	
	M44SET(perspective,	
	//	fxdiv( 2*znear,right-left ), 0, fxdiv(right+left,right-left), 0, // TODO: FIX!!!
		-fxdiv( 2*znear,right-left ), 0, fxdiv(right+left,right-left), 0,
		0,  fxdiv( 2*znear,top-bottom ),fxdiv(top+bottom,top-bottom), 0,
		0, 0, fxdiv(zfar+znear,zfar-znear), -fxdiv(2*fxmul(zfar,znear),zfar-znear),
		0, 0, -int2fx(1),0 );

	M44SET(screen,
		int2fx(400)>>1,          0,        0, int2fx(400)>>1,
		       0, -int2fx(240)>>1,         0, int2fx(240)>>1,
			   0,          0, int2fx(1),         0,
			   0,          0,         0,  int2fx(1));
	
	m44_multiply(&screen,&perspective,m);

}
//----------------------------------------------------------------------------

void m44_create_rotation_x(m44 * m, s32 angle)
{
	s32 sin = lu_sin(angle)>>4;
	s32 cos = lu_cos(angle)>>4;
	
	ptr_M44SET(m,	
		int2fx(1), int2fx(0), int2fx(0), int2fx(0),
		int2fx(0),       cos,      -sin, int2fx(0),
		int2fx(0),       sin,       cos, int2fx(0),
		int2fx(0), int2fx(0), int2fx(0), int2fx(1)  );
}

void m44_create_rotation_y(m44 * m, s32 angle)
{
	s32 sin = lu_sin(angle)>>4;
	s32 cos = lu_cos(angle)>>4;
	
	ptr_M44SET(m,	
		      cos, int2fx(0),       -sin, int2fx(0),
		int2fx(0), int2fx(1),  int2fx(0), int2fx(0),
			  sin, int2fx(0),        cos, int2fx(0),
		int2fx(0), int2fx(0),  int2fx(0), int2fx(1)  );
}

void m44_create_rotation_z(m44 * m, s32 angle)
{
	s32 sin = lu_sin(angle)>>4;
	s32 cos = lu_cos(angle)>>4;
	
	ptr_M44SET(m,	
		      cos,      -sin, int2fx(0), int2fx(0),
		      sin,       cos, int2fx(0), int2fx(0),
		int2fx(0), int2fx(0), int2fx(1), int2fx(0),
		int2fx(0), int2fx(0), int2fx(0), int2fx(1)  );
}

void m44_create_rotation_axis(m44 * m, s32 angle, s32 x, s32 y, s32 z)
{
	s32 sin = lu_sin(angle)>>4;
	s32 cos = lu_cos(angle)>>4;
	s32 one_minus_cos = int2fx(1) - cos;
	s32 x_sin = fxmul(x,sin);
	s32 y_sin = fxmul(y,sin);
	s32 z_sin = fxmul(z,sin);
	s32 x_y_one_minus_cos = fxmul(fxmul(x,y),one_minus_cos);
	s32 x_z_one_minus_cos = fxmul(fxmul(x,z),one_minus_cos);
	s32 y_z_one_minus_cos = fxmul(fxmul(y,z),one_minus_cos);
	
	ptr_M44SET(m,	
		cos+fxmul(fxmul(x,x),one_minus_cos),      
		x_y_one_minus_cos - z_sin,
		x_z_one_minus_cos + y_sin,
		int2fx(0),
		
	    x_y_one_minus_cos + z_sin,
		cos+fxmul(fxmul(y,y),one_minus_cos), 
		y_z_one_minus_cos - x_sin,
		int2fx(0),
		
		x_z_one_minus_cos - y_sin, 
		y_z_one_minus_cos + x_sin,
		cos+fxmul(fxmul(z,z),one_minus_cos), 
		int2fx(0),
		
		int2fx(0), 
		int2fx(0), 
		int2fx(0), 
		int2fx(1)  );
}

void m44_create_scale(m44 * m, s32 x, s32 y, s32 z)
{
	ptr_M44SET(m,	
		        x, int2fx(0), int2fx(0), int2fx(0),
		int2fx(0),         y, int2fx(0), int2fx(0),
		int2fx(0), int2fx(0),         z, int2fx(0),
		int2fx(0), int2fx(0), int2fx(0), int2fx(1)  );
}

void m44_create_translation(m44 * m, s32 x, s32 y, s32 z)
{
	ptr_M44SET(m,	
		int2fx(1), int2fx(0), int2fx(0),         x,
		int2fx(0), int2fx(1), int2fx(0),         y,
		int2fx(0), int2fx(0), int2fx(1),         z,
		int2fx(0), int2fx(0), int2fx(0), int2fx(1)  );
}
