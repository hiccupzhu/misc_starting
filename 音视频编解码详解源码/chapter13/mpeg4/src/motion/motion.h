/**************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  -  Motion header  -
 *
 *  This program is an implementation of a part of one or more MPEG-4
 *  Video tools as specified in ISO/IEC 14496-2 standard.  Those intending
 *  to use this software module in hardware or software products are
 *  advised that its use may infringe existing patents or copyrights, and
 *  any such use would be at such party's own risk.  The original
 *  developer of this software module and his/her company, and subsequent
 *  editors and their companies, will have no liability for use of this
 *  software or modifications or derivatives thereof.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 *  $Id$
 *
 ***************************************************************************/

#ifndef _MOTION_H_
#define _MOTION_H_

#include "../portab.h"
#include "../global.h"

/* K = 1 */
extern const uint32_t roundtab_79[4];


/** MotionEstimation **/

int MotionEstimation(MBParam *  pParam,
					FRAMEINFO *  current,
					FRAMEINFO *  reference,
					IMAGE *  pRefH,
					IMAGE *  pRefV,
					IMAGE *  pRefHV,
					//const IMAGE * const pGMC,
					const uint32_t iLimit);


/** MotionCompensation **/

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
					//const int32_t quarterpel,
					//const int reduced_resolution,
					const int32_t rounding);

#endif							/* _MOTION_H_ */
