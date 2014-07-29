/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - CBP related function  -
 *
 *  Copyright(C) 2002-2003 Edouard Gomez <ed.gomez@free.fr>
 *               2003      Christoph Lampert <gruel@web.de>
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
#include "cbp.h"

uint32_t calc_cbp(const int16_t codes[6 * 64])
{
	int i, j;
	uint32_t cbp = 0;

	for (i = 0; i < 6; i++) {
		for (j=1; j<64;j++) {
			if (codes[64*i+j]) {
            	cbp |= 1 << (5-i);
				break;
			}
		}
	}
	return cbp;
}
