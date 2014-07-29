/*****************************************************************************
 *
 *  H264 MPEG-4 VIDEO CODEC
 *  - H264 plugin: outputs PSNR to stdout (should disapear soon)  -
 *
 *  Copyright(C) 2003 Peter Ross <pross@H264.org>
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
 * $Id: plugin_psnr.c,v 1.2 2004/03/22 22:36:24 edgomez Exp $
 *
 ****************************************************************************/

#include <stdio.h>

#include "../H264.h"
#include "../image/image.h"


int H264_plugin_psnr(void * handle, int opt, void * param1, void * param2)
{
    switch(opt) {
    case H264_PLG_INFO:
	{
		H264_plg_info_t * info = (H264_plg_info_t*)param1;
		info->flags = H264_REQPSNR;
		return(0);
	}
	case H264_PLG_CREATE:
		*((void**)param2) = NULL; /* We don't have any private data to bind here */
    case H264_PLG_DESTROY:
    case H264_PLG_BEFORE:
	case H264_PLG_FRAME:
		return(0);
    case H264_PLG_AFTER:
	{
		H264_plg_data_t * data = (H264_plg_data_t*)param1;

		printf("y_psnr=%2.2f u_psnr=%2.2f v_psnr=%2.2f\n",
			   sse_to_PSNR(data->sse_y, data->width*data->height),
			   sse_to_PSNR(data->sse_u, data->width*data->height/4),
			   sse_to_PSNR(data->sse_v, data->width*data->height/4));

		return(0);
	}
    }

    return H264_ERR_FAIL;
}
