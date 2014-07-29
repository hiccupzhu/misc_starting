/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Image related header  -
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

#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdlib.h>

#include "../portab.h"
#include "../global.h"
#include "colorspace.h"
#include "../xvid.h"

#define EDGE_SIZE  32

void init_image(uint32_t cpu_flags);


static void __inline
image_null(IMAGE * image)
{
	image->y = image->u = image->v = NULL;
}

int32_t image_create(IMAGE * image,
					 uint32_t edged_width,
					 uint32_t edged_height);
void image_destroy(IMAGE * image,
				   uint32_t edged_width,
				   uint32_t edged_height);

void image_swap(IMAGE * image1,
				IMAGE * image2);

void image_copy(IMAGE * image1,
				IMAGE * image2,
				uint32_t edged_width,
				uint32_t height);

void image_setedges(IMAGE * image,
					uint32_t edged_width,
					uint32_t edged_height,
					uint32_t width,
					uint32_t height,
					int bs_version);

void image_interpolate(IMAGE * refn,
					   IMAGE * refh,
					   IMAGE * refv,
					   IMAGE * refhv,
					   uint32_t edged_width,
					   uint32_t edged_height,
   					   uint32_t quarterpel,
					   uint32_t rounding);

float image_psnr(IMAGE * orig_image,
				 IMAGE * recon_image,
				 uint16_t stride,
				 uint16_t width,
				 uint16_t height);
int image_dump_yuv420(const IMAGE * image,
					  const uint32_t edged_width,
					  const uint32_t width,
					  const uint32_t height,
					  char *filename);
					  
float sse_to_PSNR(long sse, int pixels);

long plane_sse(uint8_t * orig,
		   uint8_t * recon,
		   uint16_t stride,
		   uint16_t width,
		   uint16_t height);

//

int image_input(IMAGE * image,
				uint32_t width,
				int height,
				uint32_t edged_width,
				uint8_t * src[4],
				int src_stride[4],
				int csp,
				int interlaced);

#endif							/* _IMAGE_H_ */
