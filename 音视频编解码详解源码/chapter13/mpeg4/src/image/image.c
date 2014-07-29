/**************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Image management functions -
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

//#include "font.h"		/* XXX: remove later */




int32_t
image_create(IMAGE * image,
			 uint32_t edged_width,
			 uint32_t edged_height)
{
	const uint32_t edged_width2 = edged_width / 2;
	const uint32_t edged_height2 = edged_height / 2;

	image->y =
		xvid_malloc(edged_width * (edged_height + 1) + SAFETY, CACHE_LINE);
	if (image->y == NULL) {
		return -1;
	}
	memset(image->y, 0, edged_width * (edged_height + 1) + SAFETY);

	image->u = xvid_malloc(edged_width2 * edged_height2 + SAFETY, CACHE_LINE);
	if (image->u == NULL) {
		xvid_free(image->y);
		image->y = NULL;
		return -1;
	}
	memset(image->u, 0, edged_width2 * edged_height2 + SAFETY);

	image->v = xvid_malloc(edged_width2 * edged_height2 + SAFETY, CACHE_LINE);
	if (image->v == NULL) {
		xvid_free(image->u);
		image->u = NULL;
		xvid_free(image->y);
		image->y = NULL;
		return -1;
	}
	memset(image->v, 0, edged_width2 * edged_height2 + SAFETY);

	image->y += EDGE_SIZE * edged_width + EDGE_SIZE;
	image->u += EDGE_SIZE2 * edged_width2 + EDGE_SIZE2;
	image->v += EDGE_SIZE2 * edged_width2 + EDGE_SIZE2;

	return 0;
}



void
image_destroy(IMAGE * image,
			  uint32_t edged_width,
			  uint32_t edged_height)
{
	const uint32_t edged_width2 = edged_width / 2;

	if (image->y) {
		xvid_free(image->y - (EDGE_SIZE * edged_width + EDGE_SIZE));
		image->y = NULL;
	}
	if (image->u) {
		xvid_free(image->u - (EDGE_SIZE2 * edged_width2 + EDGE_SIZE2));
		image->u = NULL;
	}
	if (image->v) {
		xvid_free(image->v - (EDGE_SIZE2 * edged_width2 + EDGE_SIZE2));
		image->v = NULL;
	}
}


void
image_swap(IMAGE * image1,
		   IMAGE * image2)
{
    SWAP(uint8_t*, image1->y, image2->y);
    SWAP(uint8_t*, image1->u, image2->u);
    SWAP(uint8_t*, image1->v, image2->v);
}


void
image_copy(IMAGE * image1,
		   IMAGE * image2,
		   uint32_t edged_width,
		   uint32_t height)
{
	memcpy(image1->y, image2->y, edged_width * height);
	memcpy(image1->u, image2->u, edged_width * height / 4);
	memcpy(image1->v, image2->v, edged_width * height / 4);
}

/* setedges bug was fixed in this BS version */
#define SETEDGES_BUG_BEFORE		18

void
image_setedges(IMAGE * image,
			   uint32_t edged_width,
			   uint32_t edged_height,
			   uint32_t width,
			   uint32_t height,
			   int bs_version)
{
	const uint32_t edged_width2 = edged_width / 2;
	uint32_t width2;
	uint32_t i;
	uint8_t *dst;
	uint8_t *src;

	dst = image->y - (EDGE_SIZE + EDGE_SIZE * edged_width);
	src = image->y;

	width2 = width/2;

	for (i = 0; i < EDGE_SIZE; i++) 
    {
		memset(dst, *src, EDGE_SIZE);
		memcpy(dst + EDGE_SIZE, src, width);
		memset(dst + edged_width - EDGE_SIZE, *(src + width - 1),
			   EDGE_SIZE);
		dst += edged_width;
	}

	for (i = 0; i < height; i++) 
    {
		memset(dst, *src, EDGE_SIZE);
		memset(dst + edged_width - EDGE_SIZE, src[width - 1], EDGE_SIZE);
		dst += edged_width;
		src += edged_width;
	}

	src -= edged_width;
	for (i = 0; i < EDGE_SIZE; i++) 
    {
		memset(dst, *src, EDGE_SIZE);
		memcpy(dst + EDGE_SIZE, src, width);
		memset(dst + edged_width - EDGE_SIZE, *(src + width - 1),
				   EDGE_SIZE);
		dst += edged_width;
	}


	/* U */
	dst = image->u - (EDGE_SIZE2 + EDGE_SIZE2 * edged_width2);
	src = image->u;

	for (i = 0; i < EDGE_SIZE2; i++) {
		memset(dst, *src, EDGE_SIZE2);
		memcpy(dst + EDGE_SIZE2, src, width2);
		memset(dst + edged_width2 - EDGE_SIZE2, *(src + width2 - 1),
			   EDGE_SIZE2);
		dst += edged_width2;
	}

	for (i = 0; i < height / 2; i++) {
		memset(dst, *src, EDGE_SIZE2);
		memset(dst + edged_width2 - EDGE_SIZE2, src[width2 - 1], EDGE_SIZE2);
		dst += edged_width2;
		src += edged_width2;
	}
	src -= edged_width2;
	for (i = 0; i < EDGE_SIZE2; i++) {
		memset(dst, *src, EDGE_SIZE2);
		memcpy(dst + EDGE_SIZE2, src, width2);
		memset(dst + edged_width2 - EDGE_SIZE2, *(src + width2 - 1),
			   EDGE_SIZE2);
		dst += edged_width2;
	}


	/* V */
	dst = image->v - (EDGE_SIZE2 + EDGE_SIZE2 * edged_width2);
	src = image->v;

	for (i = 0; i < EDGE_SIZE2; i++) {
		memset(dst, *src, EDGE_SIZE2);
		memcpy(dst + EDGE_SIZE2, src, width2);
		memset(dst + edged_width2 - EDGE_SIZE2, *(src + width2 - 1),
			   EDGE_SIZE2);
		dst += edged_width2;
	}

	for (i = 0; i < height / 2; i++) {
		memset(dst, *src, EDGE_SIZE2);
		memset(dst + edged_width2 - EDGE_SIZE2, src[width2 - 1], EDGE_SIZE2);
		dst += edged_width2;
		src += edged_width2;
	}
	src -= edged_width2;
	for (i = 0; i < EDGE_SIZE2; i++) {
		memset(dst, *src, EDGE_SIZE2);
		memcpy(dst + EDGE_SIZE2, src, width2);
		memset(dst + edged_width2 - EDGE_SIZE2, *(src + width2 - 1),
			   EDGE_SIZE2);
		dst += edged_width2;
	}
}


int
image_dump_yuv420(const IMAGE * image,
				  const uint32_t edged_width,
				  const uint32_t width,
				  const uint32_t height,
				  char *filename)
{
    FILE *f = fopen(filename, "ab" );
    uint32_t i;
	uint8_t *bmp1;
	
    fseek( f, 0, SEEK_END );

	bmp1 = image->y;
	for (i = 0; i < height; i++) {
		fwrite(bmp1, width, 1, f);
		bmp1 += edged_width;
	}
	
	bmp1 = image->u;
	for (i = 0; i < height / 2; i++) 
    {        
		fwrite(bmp1, width / 2, 1, f);
		bmp1 += edged_width / 2;
		
	}

	bmp1 = image->v;
	for (i = 0; i < height / 2; i++) 
    {        
		fwrite(bmp1, width / 2, 1, f);
		bmp1 += edged_width / 2;
		
	}
	
	fclose(f);

    return 0;
}


#if 1
void
image_interpolate(IMAGE *  refn,
				  IMAGE *  refh,
				  IMAGE *  refv,
				  IMAGE *  refhv,
				  uint32_t edged_width,
				  uint32_t edged_height,
			      uint32_t quarterpel,
				  uint32_t rounding)
{
	const uint32_t offset = EDGE_SIZE2 * (edged_width + 1); /* we only interpolate half of the edge area */
	uint32_t stride_add = 7 * edged_width;

	uint8_t *n_ptr, *h_ptr, *v_ptr, *hv_ptr;
	uint32_t x, y;
	//int n,m;
	//n = m = 0;
	
	stride_add = stride_add + EDGE_SIZE;
	n_ptr = refn->y - offset;

	h_ptr = refh->y - offset;
	v_ptr = refv->y - offset;
	hv_ptr = refhv->y - offset;

	for (y = 0; y < (edged_height - EDGE_SIZE); y += 8) 
	{
		for (x = 0; x < (edged_width - EDGE_SIZE); x += 8) 
		// for (x = 0; x < 208; x += 8) 
		{
			interpolate8x8_halfpel_h(h_ptr, n_ptr, edged_width, rounding);
			interpolate8x8_halfpel_v(v_ptr, n_ptr, edged_width, rounding);
			interpolate8x8_halfpel_hv(hv_ptr, n_ptr, edged_width, rounding);

			n_ptr += 8;
			h_ptr += 8;
			v_ptr += 8;
			hv_ptr += 8;
			//n += 1;
		}
		//n = 0;
		//m += 1;

		h_ptr += stride_add;
		v_ptr += stride_add;
		hv_ptr += stride_add;
		n_ptr += stride_add;
	}
}
#endif

int
image_input(IMAGE * image,
			uint32_t width,
			int height,
			uint32_t edged_width,
			uint8_t * src[4],
			int src_stride[4],
			int csp,
			int interlacing)
{
	const int edged_width2 = edged_width/2;
	//const int width2 = width/2;
	const int height2 = height/2;

	switch (csp & ~XVID_CSP_VFLIP) {

	case XVID_CSP_I420:	/* YCbCr == YUV == internal colorspace for MPEG */
		yv12_to_yv12(image->y, image->u, image->v, edged_width, edged_width2,
			src[0], src[0] + src_stride[0]*height, src[0] + src_stride[0]*height + (src_stride[0]/2)*height2,
			src_stride[0], src_stride[0]/2, width, height, (csp & XVID_CSP_VFLIP));
		break;
	case XVID_CSP_NULL:
		break;
	default :
		return -1;
	}

	return 0;
}
#if 1
long plane_sse(uint8_t * orig,
		   uint8_t * recon,
		   uint16_t stride,
		   uint16_t width,
		   uint16_t height)
{
	int diff, x, y;
	long sse=0;

    _nassert((int)(orig)%8 == 0);
    _nassert((int)(recon)%8 == 0);
    _nassert(width > 176);
    _nassert(height > 144);

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			diff = *(orig + x) - *(recon + x);
			sse += diff * diff;
		}
		orig += stride;
		recon += stride;
	}
	return sse;
}
#endif
