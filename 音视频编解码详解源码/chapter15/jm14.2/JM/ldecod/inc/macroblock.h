
/*!
 ************************************************************************
 * \file macroblock.h
 *
 * \brief
 *    Arrays for macroblock encoding
 *
 * \author
 *    Inge Lille-Langoy               <inge.lille-langoy@telenor.com>
 *    Copyright (C) 1999 Telenor Satellite Services, Norway
 ************************************************************************
 */

#ifndef _MACROBLOCK_H_
#define _MACROBLOCK_H_


extern const byte QP_SCALE_CR[52];

//! look up tables for FRExt_chroma support
extern const unsigned char subblk_offset_x[3][8][4];
extern const unsigned char subblk_offset_y[3][8][4];

extern void set_interpret_mb_mode(int slice_type);
extern int  decode_one_macroblock(ImageParameters *img, Macroblock *currMB, StorablePicture *dec_picture);

#endif

