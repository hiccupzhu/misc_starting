/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Motion Estimation related header -
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

#ifndef _ESTIMATION_H_
#define _ESTIMATION_H_

#include "../portab.h"
#include "../global.h"

/* very large value */
#define MV_MAX_ERROR	(4096 * 256)

/* INTER bias for INTER/INTRA decision; mpeg4 spec suggests 2*nb */
#define MV16_INTER_BIAS	450

/* vector map (vlc delta size) smoother parameters ! float !*/
#define NEIGH_TEND_16X16		10.5
#define NEIGH_TEND_8X8			40.0
#define NEIGH_8X8_BIAS			40

#define INITIAL_SKIP_THRESH		10
#define FINAL_SKIP_THRESH		50
#define MAX_SAD00_FOR_SKIP		20
#define MAX_CHROMA_SAD_FOR_SKIP	22

/* Parameters which control inter/inter4v decision */
#define IMV16X16				2

extern const int xvid_me_lambda_vec16[32];

/* fast ((A)/2)*2 */
#define EVEN(A)		(((A)<0?(A)+1:(A)) & ~1)
#define MVequal(A,B) ( ((A).x)==((B).x) && ((A).y)==((B).y) )
#define iDiamondSize 2

static const VECTOR zeroMV = { 0, 0 };

typedef struct
{
	/* data modified by CheckCandidates */
	int32_t iMinSAD[1];			/* smallest SADs found so far */
	VECTOR currentMV[1];		/* best vectors found so far */
	int temp[4];				/* temporary space */
	unsigned int dir;			/* 'direction', set when better vector is found */

	/* general fields */
	int max_dx, min_dx, max_dy, min_dy; /* maximum range */
	uint32_t rounding;			/* rounding type in use */
	VECTOR predMV;				/* vector which predicts current vector */
	uint8_t *  RefP[4];	/* reference pictures - N, V, H, HV, cU, cV */
	uint8_t *  Cur;		/* current picture */
	
	uint32_t lambda16;			/* how much vector bits weight */
	//uint32_t lambda8;			/* as above - for inter4v mode */
	uint32_t iEdgedWidth;		/* picture's stride */
	uint32_t iFcode;			/* current fcode */
} SearchData;

void CheckSubPelCandidate16no4v(const int x, const int y, 
                            SearchData * const data, 
                            const unsigned int Direction);

void CheckCandidate16no4v(const int x, const int y,
						  SearchData * const Data,
						  const unsigned int Direction);

int xvid_me_SkipDecisionP(const IMAGE * current, const IMAGE * reference,
					const int x, const int y,
					const uint32_t stride, const uint32_t iQuant, int rrv);

void xvid_me_AdvDiamondSearch(int x, int y, SearchData * const Data, int bDirection);

void xvid_me_SubpelRefine(SearchData * const data);


#endif							/* _ESTIMATION_H_ */
