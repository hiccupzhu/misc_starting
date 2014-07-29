#include "myfdct.h"

short coeff_dct0[8] = {
	0x5a82,0x5a82,0x30fb,0x7641,
	0x18f8,	0x7d8a,0x471c,0x6a6d};
short temp0[64];

void myfdct(short *pIn)
{
	short *coeff_dct,*temp;
	short p5,p4,p3,m0,m1,m2;
	
	coeff_dct = (short *)&coeff_dct0;
	temp	  = (short *)&temp0;
	
	
}
