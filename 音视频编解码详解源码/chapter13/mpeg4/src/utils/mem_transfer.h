/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - 8<->16 bit buffer transfer header -
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

#ifndef _MEM_TRANSFER_H
#define _MEM_TRANSFER_H

/*****************************************************************************
 * transfer8to16 API
 ****************************************************************************/

void transfer_8to16copy(int16_t *  dst,
								   uint8_t *  src,
								   uint32_t stride);
#if 0
typedef TRANSFER_8TO16COPY *TRANSFER_8TO16COPY_PTR;

/* Our global function pointer - Initialized in xvid.c */
extern TRANSFER_8TO16COPY_PTR transfer_8to16copy;

/* Implemented functions */
extern TRANSFER_8TO16COPY transfer_8to16copy_c;
#endif


/*****************************************************************************
 * transfer16to8 API
 ****************************************************************************/

void transfer_16to8copy(uint8_t *  dst,
								   int16_t *  src,
								   uint32_t stride);
#if 0
typedef TRANSFER_16TO8COPY *TRANSFER_16TO8COPY_PTR;

/* Our global function pointer - Initialized in xvid.c */
extern TRANSFER_16TO8COPY_PTR transfer_16to8copy;

/* Implemented functions */
extern TRANSFER_16TO8COPY transfer_16to8copy_c;
#endif

/*****************************************************************************
 * transfer8to16 + substraction *writeback* op API
 ****************************************************************************/
void
transfer_8to16sub_test(int16_t *  dct,
					uint8_t *  cur,
					uint8_t *  ref,
					const uint32_t stride);

void
tran_8to16sub_16x16_asm_dm642(int16_t *  dct,
					uint8_t *  cur,
					uint8_t *  ref,
					const uint32_t stride,
					const uint32_t refStride);


void transfer_8to16sub(int16_t *  dct,
								  uint8_t * const cur,
								  const uint8_t *  ref,
								  const uint32_t stride);
#if 0
typedef TRANSFER_8TO16SUB *TRANSFER_8TO16SUB_PTR;

/* Our global function pointer - Initialized in xvid.c */
extern TRANSFER_8TO16SUB_PTR transfer_8to16sub;

/* Implemented functions */
extern TRANSFER_8TO16SUB transfer_8to16sub_c;

#endif

/*****************************************************************************
 * transfer8to16 + substraction op API - Bidirectionnal Version
 ****************************************************************************/

/*****************************************************************************
 * transfer16to8 + addition op API
 ****************************************************************************/

void transfer_16to8add(uint8_t *  dst,
								  int16_t *  src,
								  uint32_t stride);
#if 0
typedef TRANSFER_16TO8ADD *TRANSFER_16TO8ADD_PTR;

/* Our global function pointer - Initialized in xvid.c */
extern TRANSFER_16TO8ADD_PTR transfer_16to8add;

/* Implemented functions */
extern TRANSFER_16TO8ADD transfer_16to8add_c;
#endif

/*****************************************************************************
 * transfer8to8 + no op
 ****************************************************************************/

void transfer8x8_copy(uint8_t *  dst,
								 uint8_t *  src,
								 const uint32_t stride);
#if 0
typedef TRANSFER8X8_COPY *TRANSFER8X8_COPY_PTR;

/* Our global function pointer - Initialized in xvid.c */
extern TRANSFER8X8_COPY_PTR transfer8x8_copy;

/* Implemented functions */
extern TRANSFER8X8_COPY transfer8x8_copy_c;
#endif

void transfer_8to16copy_asm_dm642(int16_t *  dst,
								   uint8_t *  src,
								   uint32_t stride);

void transfer_16to8copy_asm_dm642(uint8_t *  dst,
								   int16_t *  src,
								   uint32_t stride);

void transfer_8to16sub_asm_dm642(int16_t *  dct,
								  uint8_t *  cur,
								  uint8_t *  ref,
								  const uint32_t curStride,
								  const uint32_t refStride);

void transfer_16to8add_asm_dm642(uint8_t *  dst,
								  int16_t *  src,
								  uint32_t stride);

static __inline void
transfer16x16_copy(uint8_t *  dst,
					uint8_t *  src,
					const uint32_t stride)
{
	transfer8x8_copy(dst, src, stride);
	transfer8x8_copy(dst + 8, src + 8, stride);
	transfer8x8_copy(dst + 8*stride, src + 8*stride, stride);
	transfer8x8_copy(dst + 8*stride + 8, src + 8*stride + 8, stride);
}


static __inline void
transfer32x32_copy(uint8_t *  dst,
					uint8_t *  src,
					const uint32_t stride)
{
	transfer16x16_copy(dst, src, stride);
	transfer16x16_copy(dst + 16, src + 16, stride);
	transfer16x16_copy(dst + 16*stride, src + 16*stride, stride);
	transfer16x16_copy(dst + 16*stride + 16, src + 16*stride + 16, stride);
}


#endif
