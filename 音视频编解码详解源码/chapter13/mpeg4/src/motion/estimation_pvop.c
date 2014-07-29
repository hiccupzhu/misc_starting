/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Motion Estimation for P- and S- VOPs  -
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	/* memcpy */

#include "../encoder.h"
#include "../prediction/mbprediction.h"
#include "../global.h"
//#include "../utils/timer.h"
#include "../image/interpolate8x8.h"
#include "estimation.h"
#include "motion.h"
#include "sad.h"
#include "motion_inlines.h"

/*
static const int xvid_me_lambda_vec8[32] =
{	0,40,41,53,60,67,76,85,
	94,105,117,129,143,159,176,194,
	215,238,264,293,325,361,403,451,
	506,570,646,738,851,994,1186,1460
};
*/

int
xvid_me_SkipDecisionP(const IMAGE * current, const IMAGE * reference,
							const int x, const int y,
							const uint32_t stride, const uint32_t iQuant, int rrv)
{
	int offset = (x + y*stride)*8;

	uint32_t sadC = sad8(current->u + offset,
						reference->u + offset,
						stride);

	if (sadC > iQuant * MAX_CHROMA_SAD_FOR_SKIP)
		return 0;

	sadC += sad8(current->v + offset,
				reference->v + offset,
				stride);

	if (sadC > iQuant * MAX_CHROMA_SAD_FOR_SKIP)
		return 0;

	return 1;


}

	/*
	 * pmv are filled with:
	 *  [0]: Median (or whatever is correct in a special case)
	 *  [1]: left neighbour
	 *  [2]: top neighbour
	 *  [3]: topright neighbour
	 * psad are filled with:
	 *  [0]: minimum of [1] to [3]
	 *  [1]: left neighbour's SAD (NB:[1] to [3] are actually not needed)
	 *  [2]: top neighbour's SAD
	 *  [3]: topright neighbour's SAD
	 */

static __inline void
get_pmvdata2(const MACROBLOCK * const mbs,
		const int mb_width,
		const int bound,
		const int x,
		const int y,
		VECTOR * const pmv,
		int32_t * const psad)
{
	int lx, ly, lz;		/* left */
	int tx, ty, tz;		/* top */
	int rx, ry, rz;		/* top-right */
	int lpos, tpos, rpos;
	int num_cand = 0, last_cand = 1;

	lx = x - 1;	ly = y;		lz = 1;
	tx = x;		ty = y - 1;	tz = 2;
	rx = x + 1;	ry = y - 1;	rz = 2;

	lpos = lx + ly * mb_width;
	rpos = rx + ry * mb_width;
	tpos = tx + ty * mb_width;

	if (lpos >= bound && lx >= 0) {
		num_cand++;
		last_cand = 1;
		pmv[1] = mbs[lpos].mvs[lz];
		psad[1] = mbs[lpos].sad8[lz];
	} else {
		pmv[1] = zeroMV;
		psad[1] = MV_MAX_ERROR;
	}

	if (tpos >= bound) {
		num_cand++;
		last_cand = 2;
		pmv[2]= mbs[tpos].mvs[tz];
		psad[2] = mbs[tpos].sad8[tz];
	} else {
		pmv[2] = zeroMV;
		psad[2] = MV_MAX_ERROR;
	}

	if (rpos >= bound && rx < mb_width) {
		num_cand++;
		last_cand = 3;
		pmv[3] = mbs[rpos].mvs[rz];
		psad[3] = mbs[rpos].sad8[rz];
	} else {
		pmv[3] = zeroMV;
		psad[3] = MV_MAX_ERROR;
	}

	/* original pmvdata() compatibility hack */
	if (x == 0 && y == 0) {
		pmv[0] = pmv[1] = pmv[2] = pmv[3] = zeroMV;
		psad[0] = 0;
		psad[1] = psad[2] = psad[3] = MV_MAX_ERROR;
		return;
	}

	/* if only one valid candidate preictor, the invalid candiates are set to the canidate */
	if (num_cand == 1) {
		pmv[0] = pmv[last_cand];
		psad[0] = psad[last_cand];
		return;
	}

	if ((MVequal(pmv[1], pmv[2])) && (MVequal(pmv[1], pmv[3]))) {
		pmv[0] = pmv[1];
		psad[0] = MIN(MIN(psad[1], psad[2]), psad[3]);
		return;
	}

	/* set median, minimum */

	pmv[0].x =
		MIN(MAX(pmv[1].x, pmv[2].x),
			MIN(MAX(pmv[2].x, pmv[3].x), MAX(pmv[1].x, pmv[3].x)));
	pmv[0].y =
		MIN(MAX(pmv[1].y, pmv[2].y),
			MIN(MAX(pmv[2].y, pmv[3].y), MAX(pmv[1].y, pmv[3].y)));

	psad[0] = MIN(MIN(psad[1], psad[2]), psad[3]);

}


static void
ModeDecision_SAD(SearchData * const Data,
				MACROBLOCK * const pMB,
				const MACROBLOCK * const pMBs,
				const int x, const int y,
				const MBParam * const pParam,
				//const uint32_t MotionFlags,
				//const uint32_t VopFlags,
				const uint32_t VolFlags,
				const IMAGE * const pCurrent,
				const IMAGE * const pRef,
//				const IMAGE * const vGMC,
				const int coding_type)
{
	int mode = MODE_INTER;
	//int mcsel = 0;
	//int inter4v = 0;//(VopFlags & XVID_VOP_INTER4V) && (pMB->dquant == 0);
	const uint32_t iQuant = pMB->quant;

	const int skip_possible = 1;//(coding_type == P_VOP) && (1);//pMB->dquant == 0

	int sad;
	int InterBias = MV16_INTER_BIAS;

#if 0
	if (Data->iMinSAD[0] < Data->iMinSAD[1] + Data->iMinSAD[2] +//inter4v == 0 || 
		Data->iMinSAD[3] + Data->iMinSAD[4] + IMV16X16 * (int32_t)iQuant) {
		mode = MODE_INTER;
		sad = Data->iMinSAD[0];
	}
#else 
	mode = MODE_INTER;
	sad = Data->iMinSAD[0];
#endif

#if 1
	/* final skip decision, a.k.a. "the vector you found, really that good?" */
	if (skip_possible && (pMB->sad16 < (int)iQuant * MAX_SAD00_FOR_SKIP))
		if ( (100*sad)/(pMB->sad16+1) > FINAL_SKIP_THRESH)
			if (xvid_me_SkipDecisionP(pCurrent, pRef, x, y, Data->iEdgedWidth/2, iQuant, 0)) 
            {
				mode = MODE_NOT_CODED;
				sad = 0;
			}
#else	//for DMA of P
	/* final skip decision, a.k.a. "the vector you found, really that good?" */
	if (skip_possible && (pMB->sad16 < (int)iQuant * MAX_SAD00_FOR_SKIP))
		if ( (100*sad)/(pMB->sad16+1) > FINAL_SKIP_THRESH)
			if (xvid_me_SkipDecisionP(pCurrent, pRef, x, 0, Data->iEdgedWidth/2, iQuant, 0)) 
            {
				mode = MODE_NOT_CODED;
				sad = 0;
			}

#endif
	/* intra decision */

	if (iQuant > 10) InterBias += 60 * (iQuant - 10); /* to make high quants work */
	if (y != 0)
		if ((pMB - pParam->mb_width)->mode == MODE_INTRA ) InterBias -= 80;
	if (x != 0)
		if ((pMB - 1)->mode == MODE_INTRA ) InterBias -= 80;

	if (InterBias < sad) 
    {
		int32_t deviation;
		deviation = dev16(Data->Cur, Data->iEdgedWidth);

		if (deviation < (sad - InterBias)) mode = MODE_INTRA;
	}

	pMB->sad16 = pMB->sad8[0] = pMB->sad8[1] = pMB->sad8[2] = pMB->sad8[3] = sad;

	if (mode == MODE_INTER) {
		pMB->mvs[0] = pMB->mvs[1] = pMB->mvs[2] = pMB->mvs[3] = Data->currentMV[0];
		pMB->pmvs[0].x = Data->currentMV[0].x - Data->predMV.x;
		pMB->pmvs[0].y = Data->currentMV[0].y - Data->predMV.y;
		//printf("MB : y = %d , x = %d  currentMV[0].x = %d $$ currentMV[0].y = %d\n",y,x,Data->currentMV[0].x,Data->currentMV[0].y);
		//printf("predMV.x = %d $$ predMV.y = %d \n",Data->predMV.x,Data->predMV.y);
	} else	/* INTRA, NOT_CODED */
		ZeroMacroblockP(pMB, 0);

	pMB->mode = mode;
}

static __inline void
PreparePredictionsP(VECTOR * const pmv, int x, int y, int iWcount,
			int iHcount, const MACROBLOCK * const prevMB, int rrv)
{
	/* this function depends on get_pmvdata which means that it sucks. It should get the predictions by itself */
	if ( (y != 0) && (x < (iWcount-1)) ) {		/* [5] top-right neighbour */
		pmv[5].x = EVEN(pmv[3].x);
		pmv[5].y = EVEN(pmv[3].y);
	} else pmv[5].x = pmv[5].y = 0;

	if (x != 0) {
		pmv[3].x = EVEN(pmv[1].x); 
		pmv[3].y = EVEN(pmv[1].y);
	}/* pmv[3] is left neighbour */
	else pmv[3].x = pmv[3].y = 0;

	if (y != 0) {
		pmv[4].x = EVEN(pmv[2].x); 
		pmv[4].y = EVEN(pmv[2].y);
	}/* [4] top neighbour */
	else pmv[4].x = pmv[4].y = 0;

	/* [1] median prediction */
	pmv[1].x = EVEN(pmv[0].x); 
	pmv[1].y = EVEN(pmv[0].y);

	pmv[0].x = pmv[0].y = 0; /* [0] is zero; not used in the loop (checked before) but needed here for make_mask */

	pmv[2].x = EVEN(prevMB->mvs[0].x); /* [2] is last frame */
	pmv[2].y = EVEN(prevMB->mvs[0].y);

	if ((x < iWcount-1) && (y < iHcount-1)) {
		pmv[6].x = EVEN((prevMB+1+iWcount)->mvs[0].x); /* [6] right-down neighbour in last frame */
		pmv[6].y = EVEN((prevMB+1+iWcount)->mvs[0].y);
	} else pmv[6].x = pmv[6].y = 0;

}


static void
SearchP(IMAGE *  pRef,
		uint8_t *  pRefH,
		uint8_t *  pRefV,
		uint8_t *  pRefHV,
		IMAGE *  pCur,
		const int x,
		const int y,
		//const uint32_t MotionFlags,
		//const uint32_t VopFlags,
		SearchData *  Data,
		MBParam *  pParam,
		MACROBLOCK *  pMBs,
		MACROBLOCK *  prevMBs,
		MACROBLOCK *  pMB)
{

	int i, threshA;
	VECTOR pmv[7];
	// int inter4v = 0;//(VopFlags & XVID_VOP_INTER4V) && (pMB->dquant == 0);


	get_range(&Data->min_dx, &Data->max_dx, &Data->min_dy, &Data->max_dy, x, y,
						pParam->width, pParam->height, Data->iFcode);

	get_pmvdata2(pMBs, pParam->mb_width, 0, x, y, pmv, Data->temp);

	i =1;
	Data->Cur = pCur->y + (x + y * Data->iEdgedWidth) * 16;

	Data->RefP[0] = pRef->y + (x + Data->iEdgedWidth*y) * 16;
	Data->RefP[2] = pRefH + (x + Data->iEdgedWidth*y) * 16;
	Data->RefP[1] = pRefV + (x + Data->iEdgedWidth*y) * 16;
	Data->RefP[3] = pRefHV + (x + Data->iEdgedWidth*y) * 16;

	Data->lambda16 = xvid_me_lambda_vec16[pMB->quant];
	Data->dir = 0;

	Data->currentMV[0].x = 0;
	Data->currentMV[0].y = 0;
	Data->predMV = pmv[0];

	i = d_mv_bits(0, 0, Data->predMV, Data->iFcode, 0, 0);
	Data->iMinSAD[0] = pMB->sad16 + ((Data->lambda16 * i * pMB->sad16)>>10);

	if (x | y)
    {
		threshA = Data->temp[0]; /* that's where we keep this SAD atm */
        	if (threshA < 512) threshA = 512;
			else if (threshA > 1024) threshA = 1024;
	} 
    else
    {
		threshA = 512;
    }
    
	PreparePredictionsP(pmv, x, y, pParam->mb_width, pParam->mb_height,
					prevMBs + x + y * pParam->mb_width, 0);

/* main loop. checking all predictions (but first, which is 0,0 and has been checked in MotionEstimation())*/

	for (i = 1; i < 7; i++)
		if (!vector_repeats(pmv, i))
		{
			CheckCandidate16no4v(pmv[i].x, pmv[i].y, Data, i);
			if (Data->iMinSAD[0] <= threshA) { i++; break; }
		}
		
	if (  (Data->iMinSAD[0] > threshA) &&
		   ((!MVequal(Data->currentMV[0], (prevMBs+x+y*pParam->mb_width)->mvs[0])) || (Data->iMinSAD[0] >= (prevMBs+x+y*pParam->mb_width)->sad16))
	    )
	{
		int mask = make_mask(pmv, i, Data->dir); /* all vectors pmv[0..i-1] have been checked */
		xvid_me_AdvDiamondSearch(Data->currentMV->x, Data->currentMV->y, Data, mask);
	}
	xvid_me_SubpelRefine(Data);

}

int
MotionEstimation(MBParam *  pParam,
				 FRAMEINFO *  current,
				 FRAMEINFO *  reference,
				 IMAGE *  pRefH,
				 IMAGE *  pRefV,
				 IMAGE *  pRefHV,
				 const uint32_t iLimit)
{
	MACROBLOCK *  pMBs = current->mbs;
	IMAGE *  pCurrent = &current->image;
	IMAGE *  pRef = &reference->image;
	
	uint32_t mb_width = pParam->mb_width;
	uint32_t mb_height = pParam->mb_height;
	const uint32_t iEdgedWidth = pParam->edged_width;

	uint32_t x, y;
	// uint32_t iIntra = 0;

	/* some pre-initialized thingies for SearchP */
	SearchData Data;
	memset(&Data, 0, sizeof(SearchData));
	Data.iEdgedWidth = iEdgedWidth;
	Data.iFcode = current->fcode;
	Data.rounding = pParam->m_rounding_type;
	
	for (y = 0; y < mb_height; y++)
    {
		for (x = 0; x < mb_width; x++)	
        {
			MACROBLOCK *pMB = &pMBs[x + y * pParam->mb_width];
			pMB->sad16 = 
				sad16v(pCurrent->y + (x + y * iEdgedWidth) * 16,
							pRef->y + (x + y * iEdgedWidth) * 16,
							pParam->edged_width, pMB->sad8 );
			
			/* initial skip decision */
			/* no early skip for GMC (global vector = skip vector is unknown!)  */
			SearchP(pRef, pRefH->y, pRefV->y, pRefHV->y, pCurrent, x,
					y,
					&Data, pParam, pMBs, reference->mbs, pMB);
            
			ModeDecision_SAD(&Data, pMB, pMBs, x, y, pParam,
								current->vol_flags,
								pCurrent, pRef, current->coding_type);//pGMC, 

		}
	}

	return 0;
}
