
/*!
 ***************************************************************************
 * \file
 *    biaridecod.h
 *
 * \brief
 *    Headerfile for binary arithmetic decoder routines
 *
 * \author
 *    Detlev Marpe,
 *    Gabi Blättermann
 *    Copyright (C) 2000 HEINRICH HERTZ INSTITUTE All Rights Reserved.
 *
 * \date
 *    21. Oct 2000
 **************************************************************************
 */

#ifndef _BIARIDECOD_H_
#define _BIARIDECOD_H_


/************************************************************************
 * D e f i n i t i o n s
 ***********************************************************************
 */

void arideco_start_decoding(DecodingEnvironmentPtr eep, unsigned char *code_buffer, int firstbyte, int *code_len);
int  arideco_bits_read(DecodingEnvironmentPtr dep);
void arideco_done_decoding(DecodingEnvironmentPtr dep);
void biari_init_context (ImageParameters *img, BiContextTypePtr ctx, const int* ini);
void rescale_cum_freq(BiContextTypePtr bi_ct);
unsigned int biari_decode_symbol(DecodingEnvironmentPtr dep, BiContextTypePtr bi_ct );
unsigned int biari_decode_symbol_eq_prob(DecodingEnvironmentPtr dep);
unsigned int biari_decode_final(DecodingEnvironmentPtr dep);
unsigned int getbyte(DecodingEnvironmentPtr dep);
unsigned int getword(DecodingEnvironmentPtr dep);
#endif  // BIARIDECOD_H_

