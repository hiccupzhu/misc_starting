
/*!
 ************************************************************************
 * \file mv-search.h
 *
 * \brief
 *   array definition for motion search
 *
 * \author
 *    Inge Lille-Langoy               <inge.lille-langoy@telenor.com>   \n
 *    Alexis Michael Tourapis         <alexis.tourapis@dolby.com>       \n
 *
 ************************************************************************
 */

#ifndef _MV_SEARCH_H_
#define _MV_SEARCH_H_

extern int* mvbits;

extern int *byte_abs;

extern int (*BiPredME)      (Macroblock *, imgpel *, short, int, int, char  ***, short  ****,
                       int, int, int, short[2], short[2], short[2], short[2], int, int, int, int, int);

extern int (*SubPelBiPredME)(imgpel* orig_pic, short ref, int list, int pic_pix_x, int pic_pix_y,
                             int blocktype, short pred_mv1[2], short pred_mv2[2], short mv1[2], short mv2[2], 
                             int search_pos2, int search_pos4, int min_mcost, int* lambda_factor, int apply_weights);
extern int (*SubPelME)      (imgpel* orig_pic, short ref, int list, int list_offset, int pic_pix_x, int pic_pix_y, 
                             int blocktype, short pred_mv[2], short mv[2], 
                             int search_pos2, int search_pos4, int min_mcost, int* lambda_factor, int apply_weights);

extern void SetMotionVectorPredictor (Macroblock *currMB, short  pmv[2], char   **refPic, short  ***tmp_mv,
                               short  ref_frame, int list, int block_x, int block_y, int blockshape_x, int blockshape_y);

extern void PrepareMEParams      (int apply_weights, int ChromaMEEnable, int list, int ref);
extern void PrepareBiPredMEParams(int apply_weights, int ChromaMEEnable, int list, int list_offset, int ref);

#endif

