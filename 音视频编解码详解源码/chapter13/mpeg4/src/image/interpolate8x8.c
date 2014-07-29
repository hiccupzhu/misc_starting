

#if 0
#include "../portab.h"
#include "../global.h"
#include "interpolate8x8.h"

/* dst = interpolate(src) */

void
interpolate8x8_halfpel_h(uint8_t * const dst,
						   const uint8_t * const src,
						   const uint32_t stride,
						   const uint32_t rounding)
{
	uintptr_t j;
	

	if (rounding)
		for (j = 0; j < 8*stride; j+=stride)
		{
				dst[j + 0] = (uint8_t)((src[j + 0] + src[j + 1] )>>1);
				dst[j + 1] = (uint8_t)((src[j + 1] + src[j + 2] )>>1);
				dst[j + 2] = (uint8_t)((src[j + 2] + src[j + 3] )>>1);
				dst[j + 3] = (uint8_t)((src[j + 3] + src[j + 4] )>>1);
				dst[j + 4] = (uint8_t)((src[j + 4] + src[j + 5] )>>1);
				dst[j + 5] = (uint8_t)((src[j + 5] + src[j + 6] )>>1);
				dst[j + 6] = (uint8_t)((src[j + 6] + src[j + 7] )>>1);
				dst[j + 7] = (uint8_t)((src[j + 7] + src[j + 8] )>>1);
		}
    #if 1
	else
		for (j = 0; j < 8*stride; j+=stride)		/* forward or backwards? Who knows ... */
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
   #endif     
}


void
interpolate8x8_halfpel_v(uint8_t * const dst,
						   const uint8_t * const src,
						   const uint32_t stride,
						   const uint32_t rounding)
{
	uintptr_t j;


 	if (rounding)
		for (j = 0; j < 8*stride; j+=stride)		/* forward is better. Some automatic prefetch perhaps. */
		{
				dst[j + 0] = (uint8_t)((src[j + 0] + src[j + stride + 0] )>>1);
				dst[j + 1] = (uint8_t)((src[j + 1] + src[j + stride + 1] )>>1);
				dst[j + 2] = (uint8_t)((src[j + 2] + src[j + stride + 2] )>>1);
				dst[j + 3] = (uint8_t)((src[j + 3] + src[j + stride + 3] )>>1);
				dst[j + 4] = (uint8_t)((src[j + 4] + src[j + stride + 4] )>>1);
				dst[j + 5] = (uint8_t)((src[j + 5] + src[j + stride + 5] )>>1);
				dst[j + 6] = (uint8_t)((src[j + 6] + src[j + stride + 6] )>>1);
				dst[j + 7] = (uint8_t)((src[j + 7] + src[j + stride + 7] )>>1);
		}
#if 1
	else
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
#endif
}


void
interpolate8x8_halfpel_hv(uint8_t * const dst,
							const uint8_t * const src,
							const uint32_t stride,
							const uint32_t rounding)
{
	uintptr_t j;

	if (rounding)
		for (j = 0; j < 8*stride; j+=stride)
		{
				dst[j + 0] = (uint8_t)((src[j+0] + src[j+1] + src[j+stride+0] + src[j+stride+1] +1)>>2);
				dst[j + 1] = (uint8_t)((src[j+1] + src[j+2] + src[j+stride+1] + src[j+stride+2] +1)>>2);
				dst[j + 2] = (uint8_t)((src[j+2] + src[j+3] + src[j+stride+2] + src[j+stride+3] +1)>>2);
				dst[j + 3] = (uint8_t)((src[j+3] + src[j+4] + src[j+stride+3] + src[j+stride+4] +1)>>2);
				dst[j + 4] = (uint8_t)((src[j+4] + src[j+5] + src[j+stride+4] + src[j+stride+5] +1)>>2);
				dst[j + 5] = (uint8_t)((src[j+5] + src[j+6] + src[j+stride+5] + src[j+stride+6] +1)>>2);
				dst[j + 6] = (uint8_t)((src[j+6] + src[j+7] + src[j+stride+6] + src[j+stride+7] +1)>>2);
				dst[j + 7] = (uint8_t)((src[j+7] + src[j+8] + src[j+stride+7] + src[j+stride+8] +1)>>2);
		}
    #if 1 
	else
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
   #endif     
}
#endif


#include <stdlib.h>
#include <string.h>				/* memcpy, memset */
#include <math.h>

#include "../portab.h"
#include "../global.h"			/* XVID_CSP_XXX's */
#include "../xvid.h"			/* XVID_CSP_XXX's */
#include "image.h"
#include "colorspace.h"
#include "interpolate8x8.h"
#include "../utils/mem_align.h"

void copy_data(uint8_t *dst,uint8_t *src,uint32_t stride,uint32_t blockLen)
{
    uint32_t i,j,k=0;
    for (j = 0; j < blockLen*stride; j+=stride)
	{
	    for (i=0; i<blockLen; i++)
        {
			dst[k++] = src[j + i];
	    }
	}
}

#if 0
/* bframe encoding requires image-based u,v interpolation */
void
image_interpolate(const IMAGE * refn,
				  IMAGE * refh,
				  IMAGE * refv,
				  IMAGE * refhv,
				  uint32_t edged_width,
				  uint32_t edged_height,
				  uint32_t quarterpel,
				  uint32_t rounding)
{
	const uint32_t offset = EDGE_SIZE2 * (edged_width + 1); /* we only interpolate half of the edge area */
	uint32_t stride_add = 7 * edged_width;

	uint8_t *n_ptr, *h_ptr, *v_ptr, *hv_ptr;
	uint32_t x, y;
	int n,m;
	n = m = 0;
	
	stride_add = stride_add + EDGE_SIZE;
	n_ptr = refn->y - offset;
	h_ptr = refh->y - offset;
	v_ptr = refv->y - offset;

	hv_ptr = refhv->y - offset;

		for (y = 0; y < (edged_height - EDGE_SIZE); y += 8) {
			for (x = 0; x < (edged_width - EDGE_SIZE); x += 8) {
				interpolate8x8_halfpel_h(h_ptr, n_ptr, edged_width, rounding);
				interpolate8x8_halfpel_v(v_ptr, n_ptr, edged_width, rounding);
				interpolate8x8_halfpel_hv(hv_ptr, n_ptr, edged_width, rounding);

				n_ptr += 8;
				h_ptr += 8;
				v_ptr += 8;
				hv_ptr += 8;
				n += 1;
			}
			n = 0;
			m += 1;

			h_ptr += stride_add;
			v_ptr += stride_add;
			hv_ptr += stride_add;
			n_ptr += stride_add;
		}

}
#endif

void
mc16x16_hpel_copy_c(uint8_t *  dst,
						  uint8_t *  src,
							   const uint32_t stride,
							   const uint32_t rounding)
{
	uintptr_t i,j,k=0;
    // should not be here
    //_nassert((int)(dst)%8 == 0);
    //#pragma MUST_ITERATE(16,16,16)
    printf("copy mode...\n");
	for (j = 0; j < 16*stride; j+=stride)
	{
	    for(i = 0; i < 16; i++)
        {
			dst[k++] = src[j + i];
	    }
	}
}

void 
mc16x16_hpel_h_c(uint8_t *  dst,
			            uint8_t *  src,
			            uint32_t stride,
					    uint32_t rounding)
{
	uintptr_t i,j,k=0;

    //_nassert((int)(dst)%8 == 0);
    //#pragma MUST_ITERATE(16,16,16)
    
	for (j = 0; j < 16*stride; j+=stride)
	{
	    for(i=0; i<16; i++)
        {
			dst[k++] = (uint8_t)((src[j + i] + src[j + (i+ 1)] + 1)>>1);
        }
	}
}

void
mc16x16_hpel_v_c(uint8_t *  dst,
							   uint8_t *  src,
							   const uint32_t stride,
							   const uint32_t rounding)
{
	uintptr_t i,j,k=0;
    //_nassert((int)(dst)%8 == 0);
    //#pragma MUST_ITERATE(16,16,16)

    for (j = 0; j < 16*stride; j += stride)
    {
  	    for(i=0; i<16; i++)
        {
		    dst[k++] = (uint8_t)((src[j + i] + src[j + stride + i] + 1)>>1);
        }
    }
}
