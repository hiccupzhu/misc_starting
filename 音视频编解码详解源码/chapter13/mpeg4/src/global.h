/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Global definitions  -
 *
 *  Copyright(C) 2002 Michael Militzer <isibaar@xvid.org>
 *
 *  This program is free software ; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation ; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY ; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program ; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * $Id$
 *
 ****************************************************************************/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "xvid.h"
#include "portab.h"

/* --- macroblock modes --- */

#define MODE_INTER		0
#define MODE_INTER_Q	1
#define MODE_INTER4V	2
#define	MODE_INTRA		3
#define MODE_INTRA_Q	4
#define MODE_NOT_CODED	16  // skip mode
#define MODE_NOT_CODED_GMC	17

/* --- bframe specific --- */

#define MODE_DIRECT			0
#define MODE_INTERPOLATE	1
#define MODE_BACKWARD		2
#define MODE_FORWARD		3
#define MODE_DIRECT_NONE_MV	4
#define MODE_DIRECT_NO4V	5


/*
 * vop coding types
 * intra, prediction, backward, sprite, not_coded
 */
#define I_VOP	0
#define P_VOP	1
#define B_VOP	2
#define S_VOP	3
#define N_VOP	4

/* convert mpeg-4 coding type i/p/b/s_VOP to XVID_TYPE_xxx */
static __inline int
coding2type(int coding_type)
{
	return coding_type + 1;
}

/* convert XVID_TYPE_xxx to bitstream coding type i/p/b/s_VOP */
static __inline int
type2coding(int xvid_type)
{
	return xvid_type - 1;
}


typedef struct
{
#if 0
	int x;
	int y;
#else
    int16_t x;
    int16_t y;
#endif
}
VECTOR;

typedef struct
{
	uint8_t *y;
	uint8_t *u;
	uint8_t *v;
}
IMAGE;

typedef struct
{
	uint32_t buf;
	uint32_t pos;
	uint32_t *tail;
	uint32_t *start;
	uint32_t length;
}
Bitstream;


#define MBPRED_SIZE  15


typedef struct
{
	/* decoder/encoder */
	VECTOR mvs[4];							//16

	short int pred_values[6][MBPRED_SIZE];	//6*15*2
	int acpred_directions[6];				//6*4

	int mode;								//4
	int quant;					/* absolute quant *///4

/* encoder specific */

	VECTOR pmvs[4];							//4*4
	int32_t sad8[4];			/* SAD values for inter4v-VECTORs */	//4*4
	int32_t sad16;				/* SAD value for inter-VECTOR */		//4

//	int dquant;
	int cbp;								//4
/* This structure has become way to big! What to do? Split it up?   */
}
MACROBLOCK;


static __inline uint32_t
get_dc_scaler(uint32_t quant,
			  uint32_t lum)
{
    // uint32_t nRet = 0;
    
	if (quant < 5)
		return 8;

	if (quant < 25 && !lum)
		return (quant + 13) / 2;

	if (quant < 9)
		return 2 * quant;

	if (quant < 25)
		return quant + 8;

	if (lum)
		return 2 * quant - 16;
	else
		return quant - 6;
}

/* useful macros */

#define MIN(X, Y) ((X)<(Y)?(X):(Y))
#define MAX(X, Y) ((X)>(Y)?(X):(Y))
/* #define ABS(X)    (((X)>0)?(X):-(X)) */
#define SIGN(X)   (((X)>0)?1:-1)
#define CLIP(X,AMIN,AMAX)   (((X)<(AMIN)) ? (AMIN) : ((X)>(AMAX)) ? (AMAX) : (X))
#define DIV_DIV(a,b)    (((a)>0) ? ((a)+((b)>>1))/(b) : ((a)-((b)>>1))/(b))
#define SWAP(_T_,A,B)    { _T_ tmp = A; A = B; B = tmp; }

int mem_compare(const void *ptr1, const char *name1,
                const void *ptr2, const char *name2, int len);


#define SCALEBITS	16
#define FIX(X)		((1L << SCALEBITS) / (X) + 1)

static const uint32_t multipliers[32] =
{
	0,       0,       FIX(4),  FIX(6),
	FIX(8),  FIX(10), FIX(12), FIX(14),
	FIX(16), FIX(18), FIX(20), FIX(22),
	FIX(24), FIX(26), FIX(28), FIX(30),
	FIX(32), FIX(34), FIX(36), FIX(38),
	FIX(40), FIX(42), FIX(44), FIX(46),
	FIX(48), FIX(50), FIX(52), FIX(54),
	FIX(56), FIX(58), FIX(60), FIX(62)
};

typedef void (*LUMA_MC_T)(uint8_t *  dst,
			                uint8_t *  src,
			                uint32_t stride,
					        uint32_t rounding);

extern LUMA_MC_T luma_mc[4];
#define MC_16X16_COPY   0   // (x=0,y=0)
#define MC_16X16_V      1   // (x=0,y=1)
#define MC_16X16_H      2   // (x=1,y=0)
#define MC_16X16_HV     3   // (x=1,y=1)

// for test
#ifndef DISABLE_TEST
// static uint8_t data_c[16*16], data_asm[16*16];
#endif

#endif							/* _GLOBAL_H_ */
