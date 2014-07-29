/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Motion Compensation related code  -
 *
 *  Copyright(C) 2002 Peter Ross <pross@xvid.org>
 *               2003 Christoph Lampert <gruel@web.de>
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

#include <stdio.h>

#include "../encoder.h"
#include "../utils/mbfunctions.h"
#include "../image/interpolate8x8.h"
//#include "../utils/timer.h"
#include "motion.h"

#if 0
static uint32_t __inline
log2bin(uint32_t value)
{
/* Changed by Chenm001 */
	int n = 0;
	while (value) {
		value >>= 1;
		n++;
	}
	return n;
}
#endif

/*
 * getref: calculate reference image pointer
 * the decision to use interpolation h/v/hv or the normal image is
 * based on dx & dy.
 */

static __inline const uint8_t *
get_ref(const uint8_t * const refn,
		const uint8_t * const refh,
		const uint8_t * const refv,
		const uint8_t * const refhv,
		const uint32_t x,
		const uint32_t y,
		//const uint32_t block,
		const int32_t dx,
		const int32_t dy,
		const int32_t stride)
{
	int nFlags = ((dx & 1) << 1) + (dy & 1);
	switch (nFlags) {
	case 0:
		return refn + (int) ((x  + dx / 2) + (y  + dy / 2) * stride);
	case 1:
		return refv + (int) ((x  + dx / 2) + (y  + (dy - 1) / 2) * stride);
	case 2:
		return refh + (int) ((x  + (dx - 1) / 2) + (y  + dy / 2) * stride);
	default:
		return refhv + (int) ((x  + (dx - 1) / 2) + (y  + (dy - 1) / 2) * stride);
	}
}

static __inline void
compensate16x16_interpolate(int16_t * const dct_codes,
							uint8_t * const cur,
							const uint8_t * const ref,
							const uint8_t * const refh,
							const uint8_t * const refv,
							const uint8_t * const refhv,
							uint8_t * const tmp,
							uint32_t x,
							uint32_t y,
							const int32_t dx,
							const int32_t dy,
							const int32_t stride,
							//const int quarterpel,
							//const int reduced_resolution,
							const int32_t rounding)
{
	 const uint8_t * ptr;// = get_ref(ref, refh, refv, refhv, x, y, dx, dy, stride);

	//if (!reduced_resolution)
	{
		ptr = get_ref(ref, refh, refv, refhv, x, y, dx, dy, stride);

		transfer_8to16sub(dct_codes, cur + y * stride + x, ptr, stride);
		transfer_8to16sub(dct_codes+64, cur + y * stride + x + 8, ptr + 8, stride);
		transfer_8to16sub(dct_codes+128, cur + y * stride + x + 8*stride,ptr + 8*stride, stride);
		transfer_8to16sub(dct_codes+192, cur + y * stride + x + 8*stride+8,	ptr + 8*stride + 8, stride);

	}

}

static void
CompensateChroma(	int dx, int dy,
					const int i, const int j,
					IMAGE * const Cur,
					const IMAGE * const Ref,
					uint8_t * const temp,
					int16_t * const coeff,
					const int32_t stride,
					const int rounding
					)
{ /* uv-block-based compensation */

		transfer_8to16sub(coeff, Cur->u + 8 * j * stride + 8 * i,
							interpolate8x8_switch2(temp, Ref->u, 8 * i, 8 * j,
													dx, dy, stride, rounding),
							stride);
		transfer_8to16sub(coeff + 64, Cur->v + 8 * j * stride + 8 * i,
 							interpolate8x8_switch2(temp, Ref->v, 8 * i, 8 * j,
													dx, dy, stride, rounding),
							stride);
}

void
MBMotionCompensation(MACROBLOCK * const mb,
					const uint32_t i,
					const uint32_t j,
					const IMAGE * const ref,
					const IMAGE * const refh,
					const IMAGE * const refv,
					const IMAGE * const refhv,
					//const IMAGE * const refGMC,
					IMAGE * const cur,
					int16_t * dct_codes,
					const uint32_t width,
					const uint32_t height,
					const uint32_t edged_width,
					const int32_t rounding)
{
	int32_t dx;
	int32_t dy;

	uint8_t * const tmp = refh->u;

	if ( (mb->mode == MODE_NOT_CODED) ) {	/* quick copy for early SKIP */
/* early SKIP is only activated in P-VOPs, not in S-VOPs, so mcsel can never be 1 */
#if 0
		transfer16x16_copy(cur->y + 16 * (i + j * edged_width),
						  ref->y + 16 * (i + j * edged_width),
						  edged_width);

		transfer8x8_copy(cur->u + 8 * (i + j * edged_width/2),
							ref->u + 8 * (i + j * edged_width/2),
							edged_width / 2);
		transfer8x8_copy(cur->v + 8 * (i + j * edged_width/2),
							ref->v + 8 * (i + j * edged_width/2),
							edged_width / 2);
#endif
		return;
	}
	
	dx = mb->mvs[0].x;//(quarterpel ? mb->qmvs[0].x : mb->mvs[0].x);
	dy = mb->mvs[0].y;//(quarterpel ? mb->qmvs[0].y : mb->mvs[0].y);

	//if ((mb->mode == MODE_NOT_CODED || mb->mode == MODE_INTER)) {
	//if (mb->mode == MODE_INTER)
	{
		/* ordinary compensation */

		compensate16x16_interpolate(&dct_codes[0 * 64], cur->y, ref->y, refh->y,
							refv->y, refhv->y, tmp, 16 * i, 16 * j, dx, dy,
							edged_width, rounding);


	}

	dx = (dx >> 1)+ roundtab_79[dx & 0x3];
	dy = (dy >> 1)+ roundtab_79[dy & 0x3];
	
	CompensateChroma(dx, dy, i, j, cur, ref, tmp,
					&dct_codes[4 * 64], edged_width / 2, rounding);

}
