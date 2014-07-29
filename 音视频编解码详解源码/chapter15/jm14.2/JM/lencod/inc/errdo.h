/*!
 **************************************************************************
 *  \file errdo.h
 *  \brief  Header file for error resilient RDO (name of file should change)
 *
 *  \author 
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Peshala Pahalawatta                     <ppaha@dolby.com>
 *    - Alexis Michael Tourapis                 <atour@ieee.org>
 *
 **************************************************************************
 */

#ifndef _ERRDO_H_
#define _ERRDO_H_

//============= rate-distortion opt with packet losses ===========

void init_error_conceal(int concealment_type);

void compute_residue_block (ImageParameters *image, imgpel **imgY, int res_img[16][16], imgpel mb_pred[16][16], int b8block, int block_size);

void decode_one_b8block (ImageParameters *image, StorablePicture *enc_pic, int decoder, int mbmode, int block8x8, short mv_mode, short b8ref);
void decode_one_mb  (ImageParameters *image, StorablePicture *enc, int decoder, Macroblock* currMB);
void UpdateDecoders (InputParameters *params, ImageParameters *image, StorablePicture *enc_pic);

void (*error_conceal_picture)(ImageParameters *image, StorablePicture *enc_pic, int decoder);
void copy_conceal_picture(ImageParameters *image, StorablePicture *enc_pic, int decoder);

void errdo_store_best_block(ImageParameters* image, imgpel*** mbY, imgpel*** dec_img, int j0, int i0, int block_size);
void errdo_get_best_block(ImageParameters* image, imgpel*** dec_img, imgpel*** mbY, int j0, int block_size);

#endif

