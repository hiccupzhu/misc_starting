/*!
 *************************************************************************************
 * \file errdo_mc_prediction.h
 *
 * \brief
 *    definitions for motion compensated prediction
 *
 * \author
 *      Main contributors (see contributors.h for copyright, 
 *                         address and affiliation details)
 *      - Alexis Michael Tourapis  <alexismt@ieee.org>
 *      - Modified for use in hypothetical decoders at encoder
 *           by Peshala V. Pahalawatta <pesh@ieee.org>
 *
 *************************************************************************************
 */

#ifndef _ERRDO_MC_PREDICTION_H_
#define _ERRDO_MC_PREDICTION_H_

#include "global.h"
#include "mbuffer.h"

//extern StorablePicture *enc_picture;

extern void get_block_luma(int decoder, ColorPlane pl, StorablePicture *dec_picture, StorablePicture *list, int x_pos, int y_pos, int ver_block_size, int hor_block_size, ImageParameters *img, imgpel block[MB_BLOCK_SIZE][MB_BLOCK_SIZE]);
extern void get_block_chroma(int decoder, int uv, StorablePicture *dec_picture, StorablePicture *list, int x_pos, int y_pos, int hor_block_size, int ver_block_size, ImageParameters *img, imgpel block[MB_BLOCK_SIZE][MB_BLOCK_SIZE]);

//extern void intra_cr_decoding(Macroblock *currMB, int yuv, ImageParameters *img, int smb);
//extern void prepare_direct_params(Macroblock *currMB, StorablePicture *dec_picture, ImageParameters *img, short pmvl0[2], short pmvl1[2],char *l0_rFrame, char *l1_rFrame);

extern void perform_mc(int decoder, ColorPlane pl, StorablePicture *dec_picture, ImageParameters *img, int pred_dir, int l0_mode, int l1_mode, int i, int j, int list_offset, int block_size_x, int block_size_y, int curr_mb_field);
extern void perform_mc_concealment(int decoder, ColorPlane pl, StorablePicture *dec_picture, ImageParameters *img, int pred_dir, int i, int j, int block_size_x, int block_size_y);
#endif

