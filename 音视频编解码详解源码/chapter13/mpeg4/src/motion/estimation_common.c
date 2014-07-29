/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Motion Estimation shared functions -
 *
 *  Copyright(C) 2002 Christoph Lampert <gruel@web.de>
 *               2002 Michael Militzer <michael@xvid.org>
 *               2002-2003 Radoslaw Czyz <xvid@syskin.cjb.net>
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

#include "../encoder.h"
#include "../global.h"
#include "../image/interpolate8x8.h"
#include "estimation.h"
#include "motion.h"
#include "sad.h"
#include "motion_inlines.h"


/*****************************************************************************
 * Modified rounding tables
 * Original tables see ISO spec tables 7-6 -> 7-9
 ****************************************************************************/

/* K = 1 */
const uint32_t roundtab_79[4] =
{ 0, 1, 0, 0 };


const int xvid_me_lambda_vec16[32] = 
	{
		0, 11,12,14,16,18,20,22,
		25,28,31,34,38,42,46,51,
		56,62,69,77,85,95,106,118,
		133,150,170,194,223,261,311,383
	};

void
xvid_me_AdvDiamondSearch(int x, int y, SearchData * const data,
						 int bDirection)//, CheckFunc * const CheckCandidate)
{

/* directions: 1 - left (x-1); 2 - right (x+1), 4 - up (y-1); 8 - down (y+1) */

	unsigned int * const iDirection = &data->dir;

	for(;;) { /* forever */
		*iDirection = 0;
		if (bDirection & 1) CheckCandidate16no4v(x - iDiamondSize, y, data,1);
		if (bDirection & 2) CheckCandidate16no4v(x + iDiamondSize, y, data,2);
		if (bDirection & 4) CheckCandidate16no4v(x, y - iDiamondSize, data,4);
		if (bDirection & 8) CheckCandidate16no4v(x, y + iDiamondSize, data,8);

		/* now we're doing diagonal checks near our candidate */

		if (*iDirection) {		/* if anything found */
			bDirection = *iDirection;
			*iDirection = 0;
			x = data->currentMV->x; y = data->currentMV->y;
			if (bDirection & 3) {	/* our candidate is left or right */
				CheckCandidate16no4v(x, y + iDiamondSize, data,8);
				CheckCandidate16no4v(x, y - iDiamondSize, data,4);
			} else {			/* what remains here is up or down */
				CheckCandidate16no4v(x + iDiamondSize, y, data,2);
				CheckCandidate16no4v(x - iDiamondSize, y, data,1);
			}

			if (*iDirection) {
				bDirection += *iDirection;
				x = data->currentMV->x; y = data->currentMV->y;
			}
		} else {				/* about to quit, eh? not so fast.... */
			switch (bDirection) {

			case 1:		//1
				CheckCandidate16no4v(x - iDiamondSize, y - iDiamondSize, data,1 + 4);
				CheckCandidate16no4v(x - iDiamondSize, y + iDiamondSize, data,1 + 8);
				break;
			case 2:		//2
				CheckCandidate16no4v(x + iDiamondSize, y - iDiamondSize, data,2 + 4);
				CheckCandidate16no4v(x + iDiamondSize, y + iDiamondSize, data,2 + 8);
				break;
			case 4:		//4
				CheckCandidate16no4v(x + iDiamondSize, y - iDiamondSize, data,2 + 4);
				CheckCandidate16no4v(x - iDiamondSize, y - iDiamondSize, data,1 + 4);
				break;
			case 1 + 4:	//5
				CheckCandidate16no4v(x - iDiamondSize, y + iDiamondSize, data,1 + 8);
				CheckCandidate16no4v(x - iDiamondSize, y - iDiamondSize, data,1 + 4);
				CheckCandidate16no4v(x + iDiamondSize, y - iDiamondSize, data,2 + 4);
				break;
			case 2 + 4:	//6
				CheckCandidate16no4v(x - iDiamondSize, y - iDiamondSize, data,1 + 4);
				CheckCandidate16no4v(x + iDiamondSize, y - iDiamondSize, data,2 + 4);
				CheckCandidate16no4v(x + iDiamondSize, y + iDiamondSize, data,2 + 8);
				break;
			case 8:		//8
				CheckCandidate16no4v(x + iDiamondSize, y + iDiamondSize, data,2 + 8);
				CheckCandidate16no4v(x - iDiamondSize, y + iDiamondSize, data,1 + 8);
				break;
			case 1 + 8:	//9
				CheckCandidate16no4v(x + iDiamondSize, y - iDiamondSize, data,2 + 4);
				CheckCandidate16no4v(x + iDiamondSize, y + iDiamondSize, data,2 + 8);
				CheckCandidate16no4v(x - iDiamondSize, y + iDiamondSize, data,1 + 8);
				break;
			case 2 + 8:	//10
				CheckCandidate16no4v(x - iDiamondSize, y - iDiamondSize, data,1 + 4);
				CheckCandidate16no4v(x - iDiamondSize, y + iDiamondSize, data,1 + 8);
				CheckCandidate16no4v(x + iDiamondSize, y + iDiamondSize, data,2 + 8);
				break;
			default:		/* 1+2+4+8 == we didn't find anything at all */
				CheckCandidate16no4v(x - iDiamondSize, y - iDiamondSize, data,1 + 4);
				CheckCandidate16no4v(x - iDiamondSize, y + iDiamondSize, data,1 + 8);
				CheckCandidate16no4v(x + iDiamondSize, y - iDiamondSize, data,2 + 4);
				CheckCandidate16no4v(x + iDiamondSize, y + iDiamondSize, data,2 + 8);
				break;
			}
			if (!*iDirection) break;		/* ok, the end. really */
			bDirection = *iDirection;
			x = data->currentMV->x; y = data->currentMV->y;
		}
	}
}

#if 0
//半象素运动估计
void
xvid_me_SubpelRefine(SearchData * const data)
{
/* Do a half-pel or q-pel refinement */
	const VECTOR centerMV = *data->currentMV;//data->qpel_precision ? *data->currentQMV : *data->currentMV;

    // g_subPelCount++;
#if 0
	CheckCandidate16no4v(centerMV.x, centerMV.y - 1, data,0);
	//CheckCandidate16no4v(centerMV.x + 1, centerMV.y - 1,  data,0);
	CheckCandidate16no4v(centerMV.x + 1, centerMV.y,  data,0);
	//CheckCandidate16no4v(centerMV.x + 1, centerMV.y + 1,  data,0);
	CheckCandidate16no4v(centerMV.x, centerMV.y + 1,  data,0);
	//CheckCandidate16no4v(centerMV.x - 1, centerMV.y + 1,  data,0);
	CheckCandidate16no4v(centerMV.x - 1, centerMV.y,  data,0);
	//CheckCandidate16no4v(centerMV.x - 1, centerMV.y - 1,  data,0);
#else
	CheckSubPelCandidate16no4v(centerMV.x, centerMV.y - 1, data,0);
	CheckSubPelCandidate16no4v(centerMV.x + 1, centerMV.y,  data,0);
	CheckSubPelCandidate16no4v(centerMV.x, centerMV.y + 1,  data,0);
	CheckSubPelCandidate16no4v(centerMV.x - 1, centerMV.y,  data,0);
    
	//CheckCandidate16no4v(centerMV.x - 1, centerMV.y - 1,  data,0);
    //CheckCandidate16no4v(centerMV.x + 1, centerMV.y - 1,  data,0);
    //CheckCandidate16no4v(centerMV.x - 1, centerMV.y + 1,  data,0);
    //CheckCandidate16no4v(centerMV.x + 1, centerMV.y + 1,  data,0);
#endif
}

// 转汇编,展开inline
// integer pel
void
CheckCandidate16no4v(const int x, const int y, SearchData * const data, const unsigned int Direction)
{
	int32_t sad;//, xc, yc;
	uint8_t *  Reference;
	uint32_t t;
	VECTOR * current;
    uint32_t nRefFlag = (((x&1)<<1) | (y&1));

	int tmpx = x;
	int tmpy = y;
	if ( (x > data->max_dx) || ( x < data->min_dx)	|| (y > data->max_dy) || (y < data->min_dy) )
		return;
#if 1		
	 t = d_mv_bits(x, y, data->predMV, data->iFcode,0,0);
#else    
	tmpx -= data->predMV.x;
	t = (tmpx != 0 ? data->iFcode:0);
	tmpx = -abs(tmpx);
	tmpx >>= (data->iFcode - 1);
	t += r_mvtab[tmpx+63];

	tmpy -= data->predMV.y;
	t += (tmpy != 0 ? data->iFcode:0);
	tmpy = -abs(tmpy);
	tmpy >>= (data->iFcode - 1);
	t += r_mvtab[tmpy+63];
#endif
	Reference = GetReference(x, y, data);		
	current = data->currentMV;		
	//xc = x; yc = y;

  	//Reference = data->RefP[0]+((x>>1) + (y>>1)*data->iEdgedWidth);	
 
       //sad = sad16_asm_dm642(data->Cur, Reference, data->iEdgedWidth, data->iEdgedWidth);
	sad = sad16(data->Cur, Reference, data->iEdgedWidth);//, 256*4096);
	sad += (data->lambda16 * t * sad)>>10;
	current = data->currentMV;
	if (sad < *(data->iMinSAD)) 
	{
		*(data->iMinSAD) = sad;
		current->x = x; current->y = y;
		data->dir = Direction;
	}
}

// 转汇编,展开inline
void
CheckSubPelCandidate16no4v(const int x, const int y, SearchData * const data, const unsigned int Direction)
{
	int32_t sad;//, xc, yc;
//	uint8_t *  Reference;
	uint32_t t;
	VECTOR * current;
    uint8_t mc_dst[16*16];
    uint8_t *mc_src = NULL;
    int tmpx = x;
	int tmpy = y;
    // uint8_t data_c[16*16], data_asm[16*16];
    
	if ( (x > data->max_dx) || ( x < data->min_dx)	|| (y > data->max_dy) || (y < data->min_dy) )
		return;
		
	// t = d_mv_bits(x, y, data->predMV, data->iFcode,0,0);
	
    tmpx -= data->predMV.x;
	t = (tmpx != 0 ? data->iFcode:0);
	tmpx = -abs(tmpx);
	tmpx >>= (data->iFcode - 1);
	t += r_mvtab[tmpx+63];

	tmpy -= data->predMV.y;
	t += (tmpy != 0 ? data->iFcode:0);
	tmpy = -abs(tmpy);
	tmpy >>= (data->iFcode - 1);
	t += r_mvtab[tmpy+63];
    
    mc_src = (uint8_t *)(data->RefP[0]+(x>>1) + (y>>1)*data->iEdgedWidth);
    luma_mc[(((x&1)<<1) | (y&1))](mc_dst, mc_src,data->iEdgedWidth,0);
    
    sad = sad16_asm_dm642(data->Cur, mc_dst, data->iEdgedWidth,16);

	sad += (data->lambda16 * t * sad)>>10;
	current = data->currentMV;
	if (sad < *(data->iMinSAD)) 
    {
		*(data->iMinSAD) = sad;
		current->x = x; current->y = y;
		data->dir = Direction;
	}
}
#endif


//整象素运动估计
void
xvid_me_SubpelRefine(SearchData * const data)
{
/* Do a half-pel or q-pel refinement */
	const VECTOR centerMV = *data->currentMV;//data->qpel_precision ? *data->currentQMV : *data->currentMV;
#if 1
	CheckCandidate16no4v(centerMV.x, centerMV.y - 1, data,0);
	CheckCandidate16no4v(centerMV.x + 1, centerMV.y,  data,0);
	CheckCandidate16no4v(centerMV.x, centerMV.y + 1,  data,0);
	CheckCandidate16no4v(centerMV.x - 1, centerMV.y,  data,0);

#else
	CheckCandidate16no4v(centerMV.x, centerMV.y - 1, data,0);
	CheckCandidate16no4v(centerMV.x + 1, centerMV.y - 1,  data,0);
	CheckCandidate16no4v(centerMV.x + 1, centerMV.y,  data,0);
	CheckCandidate16no4v(centerMV.x + 1, centerMV.y + 1,  data,0);
	CheckCandidate16no4v(centerMV.x, centerMV.y + 1,  data,0);
	CheckCandidate16no4v(centerMV.x - 1, centerMV.y + 1,  data,0);
	CheckCandidate16no4v(centerMV.x - 1, centerMV.y,  data,0);
	CheckCandidate16no4v(centerMV.x - 1, centerMV.y - 1,  data,0);
#endif
}

void
CheckCandidate16no4v(const int x, const int y, SearchData * const data, const unsigned int Direction)
{
	int32_t sad;//, xc, yc;
	uint8_t * Reference;
	uint32_t t;
	VECTOR * current;
	
	if ( (x > data->max_dx) || ( x < data->min_dx)	|| (y > data->max_dy) || (y < data->min_dy) )
		return;
		
	t = d_mv_bits(x, y, data->predMV, data->iFcode,0,0);

	Reference = GetReference(x, y, data);	

	sad = sad16(data->Cur, Reference, data->iEdgedWidth);//, 256*4096);
	sad += (data->lambda16 * t * sad)>>10;
	current = data->currentMV;
	if (sad < *(data->iMinSAD)) {
		*(data->iMinSAD) = sad;
		current->x = x; current->y = y;
		data->dir = Direction;
	}
}


