/*!
 *************************************************************************************
 * \file header.h
 *
 * \brief
 *    Prototypes for header.c
 *************************************************************************************
 */

#ifndef _HEADER_H_
#define _HEADER_H_

int FirstPartOfSliceHeader(void);
int RestOfSliceHeader(void);

void dec_ref_pic_marking(Bitstream *currStream);

void decode_poc(ImageParameters *img);
int dumppoc(ImageParameters *img);

#endif

