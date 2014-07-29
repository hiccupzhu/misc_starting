/*****************************************************************************
 *
 *  H264 MPEG-4 VIDEO CODEC
 *  - H264 plugin: dump pgm files of original and encoded frames  -
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
 * $Id: plugin_dump.c,v 1.2 2004/03/22 22:36:24 edgomez Exp $
 *
 ****************************************************************************/

#include <stdio.h>

#include "../H264.h"
#include "../image/image.h"


int H264_plugin_dump(void * handle, int opt, void * param1, void * param2)
{
    switch(opt) {
    case H264_PLG_INFO :
	{
        H264_plg_info_t * info = (H264_plg_info_t*)param1;
        info->flags = H264_REQORIGINAL;
        return(0);
	}

    case H264_PLG_CREATE :
		*((void**)param2) = NULL; /* We don't have any private data to bind here */
    case H264_PLG_DESTROY :
    case H264_PLG_BEFORE :
	case H264_PLG_FRAME :
		return(0);

    case H264_PLG_AFTER :
	{
		H264_plg_data_t * data = (H264_plg_data_t*)param1;
		IMAGE img;
		char tmp[100];
		img.y = data->original.plane[0];
		img.u = data->original.plane[1];
		img.v = data->original.plane[2];
		sprintf(tmp, "ori-%03i.pgm", data->frame_num);
		image_dump_yuvpgm(&img, data->original.stride[0], data->width, data->height, tmp);

		img.y = data->current.plane[0];
		img.u = data->current.plane[1];
		img.v = data->current.plane[2];
		sprintf(tmp, "enc-%03i.pgm", data->frame_num);
		image_dump_yuvpgm(&img, data->reference.stride[0], data->width, data->height, tmp);
	}

	return(0);
    }

    return H264_ERR_FAIL;
}
