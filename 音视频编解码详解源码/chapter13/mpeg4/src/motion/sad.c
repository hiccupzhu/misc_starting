/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Sum Of Absolute Difference related code -
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

#include "../portab.h"
#include "../global.h"
#include "sad.h"

#include <stdlib.h>

uint32_t
sad16(uint8_t *  cur,
		uint8_t *  ref,
		uint32_t stride)
{

	uint32_t sad = 0;
	uint32_t j;
	uint8_t *  ptr_cur = cur;
	uint8_t *  ptr_ref = ref;

    _nassert((int)(ptr_cur)%8 == 0);
    
	for (j = 0; j < 16; j++) {
			sad += abs(ptr_cur[0] - ptr_ref[0]);
			sad += abs(ptr_cur[1] - ptr_ref[1]);
			sad += abs(ptr_cur[2] - ptr_ref[2]);
			sad += abs(ptr_cur[3] - ptr_ref[3]);
			sad += abs(ptr_cur[4] - ptr_ref[4]);
			sad += abs(ptr_cur[5] - ptr_ref[5]);
			sad += abs(ptr_cur[6] - ptr_ref[6]);
			sad += abs(ptr_cur[7] - ptr_ref[7]);
			sad += abs(ptr_cur[8] - ptr_ref[8]);
			sad += abs(ptr_cur[9] - ptr_ref[9]);
			sad += abs(ptr_cur[10] - ptr_ref[10]);
			sad += abs(ptr_cur[11] - ptr_ref[11]);
			sad += abs(ptr_cur[12] - ptr_ref[12]);
			sad += abs(ptr_cur[13] - ptr_ref[13]);
			sad += abs(ptr_cur[14] - ptr_ref[14]);
			sad += abs(ptr_cur[15] - ptr_ref[15]);

			ptr_cur += stride;
			ptr_ref += stride;

	}

	return sad;

}

uint32_t
sad8(const uint8_t *  cur,
	   const uint8_t *  ref,
	   const uint32_t stride)
{
	uint32_t sad = 0;
	uint32_t j;
	uint8_t const *  ptr_cur = cur;
	uint8_t const *  ptr_ref = ref;

    _nassert((int)(ptr_cur)%8 == 0);
    
    for (j = 0; j < 8; j++) 
    {
		sad += abs(ptr_cur[0] - ptr_ref[0]);
		sad += abs(ptr_cur[1] - ptr_ref[1]);
		sad += abs(ptr_cur[2] - ptr_ref[2]);
		sad += abs(ptr_cur[3] - ptr_ref[3]);
		sad += abs(ptr_cur[4] - ptr_ref[4]);
		sad += abs(ptr_cur[5] - ptr_ref[5]);
		sad += abs(ptr_cur[6] - ptr_ref[6]);
		sad += abs(ptr_cur[7] - ptr_ref[7]);

		ptr_cur += stride;
		ptr_ref += stride;

	}

	return sad;
}


/* average deviation from mean */
uint32_t
dev16(const uint8_t *  cur,
		const uint32_t stride)
{    
	uint32_t mean = 0;
	uint32_t dev = 0;
	uint32_t i, j;
	uint8_t const *ptr_cur = cur;

	_nassert((int)(ptr_cur)%8 == 0);

	for (j = 0; j < 16; j++) {

		for (i = 0; i < 16; i++)
			mean += *(ptr_cur + i);

		ptr_cur += stride;

	}

	// mean /= (16 * 16);
	mean >>= 8;
	ptr_cur = cur;

	for (j = 0; j < 16; j++) {

		for (i = 0; i < 16; i++)
		{
			uint32_t uTmp = _abs(*(ptr_cur + i) - (int32_t) mean);
			if (uTmp>= 255)
			{
				printf("debug here...\n");
			}
			// dev += _abs(*(ptr_cur + i) - (int32_t) mean);
			dev += uTmp;
		}

		ptr_cur += stride;

	}

	return dev;
}

uint32_t sad16v(const uint8_t *  cur,
			   const uint8_t *  ref,
			   const uint32_t stride,
			   int32_t *sad)
{
#if 1
	sad[0] = sad8(cur, ref, stride);
	sad[1] = sad8(cur + 8, ref + 8, stride);
	sad[2] = sad8(cur + 8*stride, ref + 8*stride, stride);
	sad[3] = sad8(cur + 8*stride + 8, ref + 8*stride + 8, stride);
#else
	sad[0] = sad8_asm_dm642(cur, ref, stride);
	sad[1] = sad8_asm_dm642(cur + 8, ref + 8, stride);
	sad[2] = sad8_asm_dm642(cur + 8*stride, ref + 8*stride, stride);
	sad[3] = sad8_asm_dm642(cur + 8*stride + 8, ref + 8*stride + 8, stride);
/*
	if (i!= sad[0] || j!= sad[1] || k != sad[2] || l != sad[3])
	{
		printf("debug here ...\n");
	}
*/
#endif

	return sad[0]+sad[1]+sad[2]+sad[3];
}
