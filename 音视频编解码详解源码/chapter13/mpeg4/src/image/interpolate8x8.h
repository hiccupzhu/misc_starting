/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Interpolation related header  -
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

#ifndef _INTERPOLATE8X8_H_
#define _INTERPOLATE8X8_H_

#include "../utils/mem_transfer.h"
#define SAFETY	32
#define EDGE_SIZE2  (EDGE_SIZE/2)

void copy_data(uint8_t *dst,uint8_t *src,uint32_t stride,uint32_t blockLen);

void interpolate8x8_halfpel_h_asm_dm642(uint8_t *  dst,
					     uint8_t *  src,
						 uint32_t stride,
						 uint32_t rounding);

void interpolate8x8_halfpel_v_asm_dm642(uint8_t *  dst,
							   uint8_t *  src,
							   const uint32_t stride,
							   const uint32_t rounding);

void interpolate8x8_halfpel_hv_asm_dm642(uint8_t *  dst,
						  uint8_t *  src,
							   const uint32_t stride,
							   const uint32_t rounding);
							   
static void __inline
interpolate8x8_halfpel_h(uint8_t *  dst,
					     uint8_t *  src,
						 uint32_t stride,
						 uint32_t rounding)
{
	uintptr_t j;

    //_nassert((int)(dst)%8 == 0);
    //#pragma MUST_ITERATE(8,8,8)
    
	for (j = 0; j < 8*stride; j+=stride)
	{
			dst[j + 0] = (uint8_t)((src[j + 0] + src[j + 1] + 1)>>1);
			dst[j + 1] = (uint8_t)((src[j + 1] + src[j + 2] + 1)>>1);
			dst[j + 2] = (uint8_t)((src[j + 2] + src[j + 3] + 1)>>1);
			dst[j + 3] = (uint8_t)((src[j + 3] + src[j + 4] + 1)>>1);
			dst[j + 4] = (uint8_t)((src[j + 4] + src[j + 5] + 1)>>1);
			dst[j + 5] = (uint8_t)((src[j + 5] + src[j + 6] + 1)>>1);
			dst[j + 6] = (uint8_t)((src[j + 6] + src[j + 7] + 1)>>1);
			dst[j + 7] = (uint8_t)((src[j + 7] + src[j + 8] + 1)>>1);
	}
}

static void __inline
interpolate8x8_halfpel_v(uint8_t *  dst,
							   uint8_t *  src,
							   const uint32_t stride,
							   const uint32_t rounding)
{
	uintptr_t j;
    //_nassert((int)(dst)%8 == 0);
    //#pragma MUST_ITERATE(8,8,8)

    for (j = 0; j < 8*stride; j+=stride)
    {
		dst[j + 0] = (uint8_t)((src[j + 0] + src[j + stride + 0] + 1)>>1);
		dst[j + 1] = (uint8_t)((src[j + 1] + src[j + stride + 1] + 1)>>1);
		dst[j + 2] = (uint8_t)((src[j + 2] + src[j + stride + 2] + 1)>>1);
		dst[j + 3] = (uint8_t)((src[j + 3] + src[j + stride + 3] + 1)>>1);
		dst[j + 4] = (uint8_t)((src[j + 4] + src[j + stride + 4] + 1)>>1);
		dst[j + 5] = (uint8_t)((src[j + 5] + src[j + stride + 5] + 1)>>1);
		dst[j + 6] = (uint8_t)((src[j + 6] + src[j + stride + 6] + 1)>>1);
		dst[j + 7] = (uint8_t)((src[j + 7] + src[j + stride + 7] + 1)>>1);
    }
}

static void __inline
interpolate8x8_halfpel_hv(uint8_t *  dst,
						  uint8_t *  src,
							   const uint32_t stride,
							   const uint32_t rounding)
{
	uintptr_t j;
    //_nassert((int)(dst)%8 == 0);
    //#pragma MUST_ITERATE(8,8,8)
    // printf("test..\n");
	for (j = 0; j < 8*stride; j+=stride)
	{
		dst[j + 0] = (uint8_t)((src[j+0] + src[j+1] + src[j+stride+0] + src[j+stride+1] +2)>>2);
		dst[j + 1] = (uint8_t)((src[j+1] + src[j+2] + src[j+stride+1] + src[j+stride+2] +2)>>2);
		dst[j + 2] = (uint8_t)((src[j+2] + src[j+3] + src[j+stride+2] + src[j+stride+3] +2)>>2);
		dst[j + 3] = (uint8_t)((src[j+3] + src[j+4] + src[j+stride+3] + src[j+stride+4] +2)>>2);
		dst[j + 4] = (uint8_t)((src[j+4] + src[j+5] + src[j+stride+4] + src[j+stride+5] +2)>>2);
		dst[j + 5] = (uint8_t)((src[j+5] + src[j+6] + src[j+stride+5] + src[j+stride+6] +2)>>2);
		dst[j + 6] = (uint8_t)((src[j+6] + src[j+7] + src[j+stride+6] + src[j+stride+7] +2)>>2);
		dst[j + 7] = (uint8_t)((src[j+7] + src[j+8] + src[j+stride+7] + src[j+stride+8] +2)>>2);
	}
}

static __inline uint8_t *
interpolate8x8_switch2(uint8_t *  buffer,
					   uint8_t *  refn,
					  const int x,
					  const int y,
					  const int dx,
					  const int dy,
					  const uint32_t stride,
					  const uint32_t rounding)
{

	uint8_t *  src = refn + (int)((y + (dy>>1)) * stride + x + (dx>>1));

	switch (((dx & 1) << 1) + (dy & 1))	{ /* ((dx%2)?2:0)+((dy%2)?1:0) */
	case 0:
		return (uint8_t *)src;
	case 1:
		 interpolate8x8_halfpel_v(buffer, src, stride, rounding);
		//interpolate8x8_halfpel_v_asm_dm642(buffer, src, stride, rounding);
		break;
	case 2:
		 interpolate8x8_halfpel_h(buffer, src, stride, rounding);
		//interpolate8x8_halfpel_h_asm_dm642(buffer, src, stride, rounding);
		break;
	default:
		interpolate8x8_halfpel_hv(buffer, src, stride, rounding);// todo...
		break;
	}
	return buffer;
}

void
mc16x16_hpel_copy_c(uint8_t *  dst,
						  uint8_t *  src,
							   const uint32_t stride,
							   const uint32_t rounding);

void 
mc16x16_hpel_h_c(uint8_t *  dst,
			            uint8_t *  src,
			            uint32_t stride,
					    uint32_t rounding);

void
mc16x16_hpel_v_c(uint8_t *  dst,
							   uint8_t *  src,
							   const uint32_t stride,
							   const uint32_t rounding);

void
mc16x16_hpel_copy_asm_dm642(uint8_t *  dst,
						  uint8_t *  src,
							   const uint32_t stride,
							   const uint32_t rounding);

void 
mc16x16_hpel_h_asm_dm642(uint8_t *  dst,
			            uint8_t *  src,
			            uint32_t stride,
					    uint32_t rounding);

void
mc16x16_hpel_v_asm_dm642(uint8_t *  dst,
							   uint8_t *  src,
							   const uint32_t stride,
							   const uint32_t rounding);
							   
#endif
