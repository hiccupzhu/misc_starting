/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Colorspace conversion functions -
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

#include <string.h>				/* memcpy */

#include "../global.h"
#include "colorspace.h"

/* yv12 to yv12 copy function */
void
yv12_to_yv12(uint8_t * y_dst, uint8_t * u_dst, uint8_t * v_dst,
				int y_dst_stride, int uv_dst_stride,
				uint8_t * y_src, uint8_t * u_src, uint8_t * v_src,
				int y_src_stride, int uv_src_stride,
				int width, int height, int vflip)
{
	int width2 = width / 2;
	int height2 = height / 2;
	int y;
/*
	if (vflip) {
		y_src += (height - 1) * y_src_stride;
		u_src += (height2 - 1) * uv_src_stride;
		v_src += (height2 - 1) * uv_src_stride;
		y_src_stride = -y_src_stride;
		uv_src_stride = -uv_src_stride;
	}
*/
	for (y = height; y; y--) {
		memcpy(y_dst, y_src, width);
		y_src += y_src_stride;
		y_dst += y_dst_stride;
	}

	for (y = height2; y; y--) {
		memcpy(u_dst, u_src, width2);
		u_src += uv_src_stride;
		u_dst += uv_dst_stride;
	}

	for (y = height2; y; y--) {
		memcpy(v_dst, v_src, width2);
		v_src += uv_src_stride;
		v_dst += uv_dst_stride;
	}
}
