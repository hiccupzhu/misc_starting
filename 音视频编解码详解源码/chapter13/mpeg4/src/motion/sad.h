/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Sum Of Absolute Difference header  -
 *
 *  Copyright(C) 2001-2003 Peter Ross <pross@xvid.org>
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

#ifndef _ENCODER_SAD_H_
#define _ENCODER_SAD_H_

#include "../portab.h"

uint32_t sad16(uint8_t *  cur,
					   uint8_t *  ref,
					   uint32_t stride);

uint32_t sad8(const uint8_t *  cur,
			 const uint8_t *  ref,
			 const uint32_t stride);

uint32_t dev16(const uint8_t *  cur,
			  const uint32_t stride);

uint32_t sad16v(const uint8_t *  cur,
				const uint8_t *  ref,
				const uint32_t stride, int32_t *sad8);

unsigned sad16_asm_dm642
(
    const unsigned char * srcImg,  /* 16x16 source block */
    const unsigned char * refImg,  /* Reference image    */
    int nPitch,
    int nRefPitch
);

// 源与目地均为8byte对齐
unsigned sad16_align_asm_dm642
(
    const unsigned char * srcImg,  /* 16x16 source block */
    const unsigned char * refImg,  /* Reference image    */
    int nSrcPitch,
    int nRefPitch
);

unsigned sad8_asm_dm642
(
    const unsigned char * srcImg,  /* 16x16 source block */
    const unsigned char * refImg,  /* Reference image    */
    int pitch
);

unsigned dev16_asm_dm642
(
    const uint8_t *  cur,
	const uint32_t stride
);
#endif							/* _ENCODER_SAD_H_ */
