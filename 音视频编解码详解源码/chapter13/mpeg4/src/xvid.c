/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Native API implementation  -
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
#include "xvid.h"
#include "encoder.h"

#if defined(_DEBUG)
unsigned int xvid_debug = 0; /* xvid debug mask */
#endif

/*****************************************************************************
 * XviD Native encoder entry point
 *
 * This function is just a wrapper to all the option cases.
 *
 * Returned values : XVID_ERR_FAIL when opt is invalid
 *                   else returns the wrapped function result
 *
 ****************************************************************************/

int
xvid_encore(void *handle,
			int opt,
			void *param1,
			void *param2)
{
	switch (opt) {
	case XVID_ENC_ENCODE:

		return enc_encode((Encoder *) handle,
							  (xvid_enc_frame_t *) param1,
							  (xvid_enc_stats_t *) param2);

	case XVID_ENC_CREATE:
		return enc_create((xvid_enc_create_t *) param1);

	case XVID_ENC_DESTROY:
		return enc_destroy((Encoder *) handle);

	default:
		return XVID_ERR_FAIL;
	}
}
