/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - MB Transfert/Quantization functions -
 *
 *  Copyright(C) 2001-2003  Peter Ross <pross@xvid.org>
 *               2001-2003  Michael Militzer <isibaar@xvid.org>
 *               2003       Edouard Gomez <ed.gomez@free.fr>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../portab.h"
#include "mbfunctions.h"

#include "../global.h"
#include "mem_transfer.h"
//#include "timer.h"
#include "../bitstream/mbcoding.h"
//#include "../bitstream/zigzag.h"
#include "../dct/fdct.h"
#include "../dct/idct.h"
#include "../quant/quant.h"
#include "../encoder.h"

#include "IMG_fdct_8x8.h"
#include "img_idct_8x8.h"

/*
 * Skip blocks having a coefficient sum below this value. This value will be
 * corrected according to the MB quantizer to avoid artifacts for quant==1
 */
#define PVOP_TOOSMALL_LIMIT 1
#define BVOP_TOOSMALL_LIMIT 3

/*****************************************************************************
 * Module functions
 ****************************************************************************/

void
MBTransQuantIntra(const MBParam * const pParam,
				  const FRAMEINFO * const frame,
				  MACROBLOCK * const pMB,
				  const uint32_t x_pos,
				  const uint32_t y_pos,
				  int16_t data[6 * 64],
				  int16_t qcoeff[6 * 64])
{
	uint32_t stride = pParam->edged_width;
	uint32_t stride2 = stride / 2;
	uint32_t next_block = stride * 8;
	uint8_t *pY_Cur, *pU_Cur, *pV_Cur;
	const IMAGE * const pCurrent = &frame->image;
	int scaler_lum, scaler_chr;
//    int i = 0;
	const int16_t mult = (uint16_t)multipliers[pMB->quant];
    
	scaler_lum = get_dc_scaler(pMB->quant, 1);
	scaler_chr = get_dc_scaler(pMB->quant, 0);

	/* Image pointers */
	pY_Cur = pCurrent->y + (y_pos << (4+0)) * stride  + (x_pos << (4+0));
	pU_Cur = pCurrent->u + (y_pos << (3+0)) * stride2 + (x_pos << (3+0));
	pV_Cur = pCurrent->v + (y_pos << (3+0)) * stride2 + (x_pos << (3+0));
    
	transfer_8to16copy(&data[0 * 64], pY_Cur,                 stride);
	transfer_8to16copy(&data[1 * 64], pY_Cur +8,              stride);
	transfer_8to16copy(&data[2 * 64], pY_Cur + next_block,    stride);
	transfer_8to16copy(&data[3 * 64], pY_Cur + next_block + 8,stride);
	transfer_8to16copy(&data[4 * 64], pU_Cur, stride2);
	transfer_8to16copy(&data[5 * 64], pV_Cur, stride2);

	IMG_fdct_8x8(data, 6);
	  
   	quant_h263_intra(&qcoeff[0 * 64], &data[0 * 64], pMB->quant, scaler_lum, scaler_chr,mult);
   	
	qcoeff[0*64] = DIV_DIV(data[0*64], (int32_t) scaler_lum);
	qcoeff[1*64] = DIV_DIV(data[1*64], (int32_t) scaler_lum);
	qcoeff[2*64] = DIV_DIV(data[2*64], (int32_t) scaler_lum);
	qcoeff[3*64] = DIV_DIV(data[3*64], (int32_t) scaler_lum);
	qcoeff[4*64] = DIV_DIV(data[4*64], (int32_t) scaler_chr);
	qcoeff[5*64] = DIV_DIV(data[5*64], (int32_t) scaler_chr);

	 dequant_h263_intra(&data[0 * 64], &qcoeff[0 * 64], pMB->quant, scaler_lum, scaler_chr);

	data[0*64] = qcoeff[0*64] * scaler_lum;
	data[1*64] = qcoeff[1*64] * scaler_lum;
	data[2*64] = qcoeff[2*64] * scaler_lum;
	data[3*64] = qcoeff[3*64] * scaler_lum;
	data[4*64] = qcoeff[4*64] * scaler_chr;
	data[5*64] = qcoeff[5*64] * scaler_chr;

	data[0*64] = CLIP(data[0*64],-2048, 2047)<<4;        
	data[1*64] = CLIP(data[1*64],-2048, 2047)<<4;   
	data[2*64] = CLIP(data[2*64],-2048, 2047)<<4;        
	data[3*64] = CLIP(data[3*64],-2048, 2047)<<4;   
	data[4*64] = CLIP(data[4*64],-2048, 2047)<<4;        
	data[5*64] = CLIP(data[5*64],-2048, 2047)<<4;

	IMG_idct_8x8_12q4(&data[0 * 64],6);

	transfer_16to8copy(pY_Cur ,                   &data[0 * 64], stride);
	transfer_16to8copy(pY_Cur +8 ,                &data[1 * 64], stride);
	transfer_16to8copy(pY_Cur + next_block,       &data[2 * 64], stride);
	transfer_16to8copy(pY_Cur + next_block + 8 ,  &data[3 * 64], stride);
	transfer_16to8copy(pU_Cur,                    &data[4 * 64], stride2);
	transfer_16to8copy(pV_Cur,                    &data[5 * 64], stride2);
}

uint8_t
MBTransQuantInter(const MBParam * const pParam,
				  const FRAMEINFO * const frame,
				  MACROBLOCK * const pMB,
				  const uint32_t x_pos,
				  const uint32_t y_pos,
				  int16_t data[6 * 64],
				  int16_t qcoeff[6 * 64])
{
	uint8_t cbp = 0;
	// uint32_t limit = 1;
	uint8_t *pY_Cur, *pU_Cur, *pV_Cur;
	uint32_t stride = pParam->edged_width;
	uint32_t stride2 = stride / 2;
	uint32_t next_block = stride * 8;
	const IMAGE * const pCurrent = &frame->image;
	const int16_t mult = (int16_t)multipliers[pMB->quant];
	int i;
	int sum = 0;

	pY_Cur = pCurrent->y + (y_pos << (4+0)) * stride  + (x_pos << (4+0));
	pU_Cur = pCurrent->u + (y_pos << (3+0)) * stride2 + (x_pos << (3+0));
	pV_Cur = pCurrent->v + (y_pos << (3+0)) * stride2 + (x_pos << (3+0));
	IMG_fdct_8x8(data, 6);

	for (i = 0; i < 6; i++) 
    {        
		 sum = quant_h263_inter(&qcoeff[i*64], &data[i*64], pMB->quant, pParam->mpeg_quant_matrices,mult);
 		if ((sum >= 1) || (qcoeff[i*64+1] != 0) || (qcoeff[i*64+8] != 0))
		{
			 dequant_h263_inter(&data[i * 64], &qcoeff[i * 64], pMB->quant, pParam->mpeg_quant_matrices);
            		IMG_idct_8x8_12q4(&data[i * 64],1);
			cbp |= 1 << (5 - i);
		}
	}

       if (cbp&32) transfer_16to8add(pY_Cur,                     &data[0 * 64], stride);
	if (cbp&16) transfer_16to8add(pY_Cur + 8,                 &data[1 * 64], stride);
	if (cbp& 8) transfer_16to8add(pY_Cur + next_block,        &data[2 * 64], stride);
	if (cbp& 4) transfer_16to8add(pY_Cur + next_block + 8,    &data[3 * 64], stride);
	if (cbp& 2) transfer_16to8add(pU_Cur,                     &data[4 * 64], stride2);
	if (cbp& 1) transfer_16to8add(pV_Cur,                     &data[5 * 64], stride2);

	return(cbp);
}

