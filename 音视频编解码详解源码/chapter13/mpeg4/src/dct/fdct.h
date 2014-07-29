/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Forward DCT header  -
 *
 *  Copyright(C) 2001-2003 Michael Militzer <isibaar@xvid.org>
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

#ifndef _FDCT_H_
#define _FDCT_H_

void fdct(short *const block);
void IMG_fdct_8x8_c(short *dct_data, unsigned num_fdcts);

#endif							/* _FDCT_H_ */
