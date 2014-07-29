/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - (de)Quantization related header  -
 *
 *  Copyright(C) 2003 Edouard Gomez <ed.gomez@free.fr>
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

#ifndef _QUANT_H_
#define _QUANT_H_

#include "../portab.h"

/*****************************************************************************
 * Common API for Intra (de)Quant functions
 ****************************************************************************/
#if 0
uint32_t quant_h263_intra(int16_t *  coeff,
									int16_t *  data,
									const uint32_t quant,
									const uint32_t dcscalar,
									uint16_t *  mpeg_quant_matrices);
uint32_t dequant_h263_intra(int16_t *  coeff,
									int16_t *  data,
									const uint32_t quant,
									const uint32_t dcscalar,
									uint16_t *  mpeg_quant_matrices);
#else
uint32_t
quant_h263_intra(int16_t *  coeff,
				   int16_t *  data,
				   const uint32_t quant,
				   const uint32_t scaler_lum,
				   const uint32_t scaler_chr,
				   const int16_t mult);

uint32_t
dequant_h263_intra(int16_t *  data,
					 int16_t *  coeff,
					 const uint32_t quant,
					 const uint32_t scaler_lum,
					 const uint32_t scaler_chr);				   
#endif
/*****************************************************************************
 * Common API for Inter (de)Quant functions
 ****************************************************************************/

uint32_t quant_h263_inter(int16_t *  coeff,
									int16_t *  data,
									const uint32_t quant,
									uint16_t *  mpeg_quant_matrices,
									const int16_t mult);

uint32_t dequant_h263_inter(int16_t *  coeff,
									int16_t *  data,
									const uint32_t quant,
									uint16_t *  mpeg_quant_matrices);

// asm 
uint32_t quant_h263_intra_asm_dm642(int16_t *  coeff,
									int16_t *  data,
									const uint32_t quant,
									const uint32_t scaler_lum,
									const uint32_t scaler_chr,
									const int16_t mult);

uint32_t dequant_h263_intra_asm_dm642(int16_t *  data,
									int16_t *  coeff,
									const uint32_t quant,
									const uint32_t scaler_lum,
									const uint32_t scaler_chr);

uint32_t quant_h263_inter_asm_dm642(int16_t *  coeff,
									int16_t *  data,
									const uint32_t quant,
									const int16_t mult);

uint32_t dequant_h263_inter_asm_dm642(int16_t *  data,
									int16_t *  coeff,
									const uint32_t quant,
									const uint32_t scaler_lum,
									const uint32_t scaler_chr);

uint32_t
quant_h263_inter_c(int16_t * coeff,
				   const int16_t * data,
				   const uint32_t quant,
				   const uint16_t * mpeg_quant_matrices);

#endif /* _QUANT_H_ */
