
/*!
 *************************************************************************************
 * \file mc_prediction.c
 *
 * \brief
 *    Motion Compensation
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Alexis Michael Tourapis         <alexismt@ieee.org>
 *
 *************************************************************************************
 */

#include "contributors.h"

#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <memory.h>
#include <math.h>

#include "global.h"

#include "macroblock.h"
#include "mc_prediction.h"
#include "refbuf.h"
#include "image.h"
#include "mb_access.h"

static int diff  [16];
static imgpel l0_pred[MB_PIXELS];
static imgpel l1_pred[MB_PIXELS];


static const unsigned char subblk_offset_x[3][8][4] =
{
  { 
    {0, 4, 0, 4},
    {0, 4, 0, 4},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}, 
  },
  { 
    {0, 4, 0, 4},
    {0, 4, 0, 4},
    {0, 4, 0, 4},
    {0, 4, 0, 4},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}, 
  },
  { 
    {0, 4, 0, 4},
    {8,12, 8,12},
    {0, 4, 0, 4},
    {8,12, 8,12},
    {0, 4, 0, 4},
    {8,12, 8,12},
    {0, 4, 0, 4},
    {8,12, 8,12}  
  }
};

static const unsigned char subblk_offset_y[3][8][4] =
{
  { 
    {0, 0, 4, 4},
    {0, 0, 4, 4},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}, 
  },
  { 
    {0, 0, 4, 4},
    {8, 8,12,12},
    {0, 0, 4, 4},
    {8, 8,12,12},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
  },
  {
    {0, 0, 4, 4},
    {0, 0, 4, 4},
    {8, 8,12,12},
    {8, 8,12,12},
    {0, 0, 4, 4},
    {0, 0, 4, 4},
    {8, 8,12,12},
    {8, 8,12,12} 
  }
};

/*!
 ************************************************************************
 * \brief
 *    Predict Luma block
 ************************************************************************
 */
static inline void OneComponentLumaPrediction ( imgpel*   mpred,       //!< array of prediction values (row by row)
                                               int    pic_pix_x,      //!< motion shifted horizontal coordinate of block
                                               int    pic_pix_y,      //!< motion shifted vertical   coordinate of block
                                               int    block_size_x,   //!< horizontal block size
                                               int    block_size_y,   //!< vertical block size
                                               StorablePicture *list //!< reference picture list
                                               )
{
  int     j;
  imgpel *ref_line = UMVLine4X (list->p_curr_img_sub, pic_pix_y, pic_pix_x);

  width_pad  = list->size_x_pad;
  height_pad = list->size_y_pad;

  for (j = 0; j < block_size_y; j++) 
  {
    memcpy(mpred, ref_line, block_size_x * sizeof(imgpel));
    ref_line += img_padded_size_x;
    mpred += block_size_x;
  }  
}


/*!
 ************************************************************************
 * \brief
 *    Predict one Luma block
 ************************************************************************
 */
void LumaPrediction ( Macroblock* currMB,//!< Current Macroblock
                     int   block_x,     //!< relative horizontal block coordinate of block
                     int   block_y,     //!< relative vertical   block coordinate of block
                     int   block_size_x,//!< relative horizontal block coordinate of block
                     int   block_size_y,//!< relative vertical   block coordinate of block
                     int   p_dir,       //!< prediction direction (0=list0, 1=list1, 2=bipred)
                     int   l0_mode,     //!< list0 prediction mode (1-7, 0=DIRECT if l1_mode=0)
                     int   l1_mode,     //!< list1 prediction mode (1-7, 0=DIRECT if l0_mode=0)
                     short l0_ref_idx,  //!< reference frame for list0 prediction (-1: Intra4x4 pred. with l0_mode)
                     short l1_ref_idx,   //!< reference frame for list1 prediction 
                     short bipred_me     //!< use bi prediction mv (0=no bipred, 1 = use set 1, 2 = use set 2)
                     )
{
  int  i, j;
  int  block_x4     = block_x + block_size_x;
  int  block_y4     = block_y + block_size_y;
  int  pic_opix_x   = ((img->opix_x + block_x) << 2) + IMG_PAD_SIZE_TIMES4;
  int  pic_opix_y   = ((img->opix_y + block_y) << 2) + IMG_PAD_SIZE_TIMES4;
  int  bx           = block_x >> 2;
  int  by           = block_y >> 2;
  imgpel* l0pred    = l0_pred;
  imgpel* l1pred    = l1_pred;  
  short****** mv_array = img->all_mv;
  short   *curr_mv = NULL;
  imgpel (*mb_pred)[16] = img->mb_pred[0];

  int  apply_weights = ( (active_pps->weighted_pred_flag  && (img->type== P_SLICE || img->type == SP_SLICE)) ||
    (active_pps->weighted_bipred_idc && (img->type== B_SLICE)));

  if (bipred_me && l0_ref_idx == 0 && l1_ref_idx == 0 && p_dir == 2 && is_bipred_enabled(l0_mode) && is_bipred_enabled(l1_mode))
    mv_array = img->bipred_mv[bipred_me - 1]; 

  switch (p_dir)
  {
  case 0:
    curr_mv = mv_array[LIST_0][l0_ref_idx][l0_mode][by][bx];
    OneComponentLumaPrediction (l0_pred, pic_opix_x + curr_mv[0], pic_opix_y + curr_mv[1], block_size_x, block_size_y, listX[LIST_0 + currMB->list_offset][l0_ref_idx]);
    break;
  case 1:
    curr_mv = mv_array[LIST_1][l1_ref_idx][l1_mode][by][bx];
    OneComponentLumaPrediction (l1_pred, pic_opix_x + curr_mv[0], pic_opix_y + curr_mv[1], block_size_x, block_size_y, listX[LIST_1 + currMB->list_offset][l1_ref_idx]);
    break;
  case 2:
    curr_mv = mv_array[LIST_0][l0_ref_idx][l0_mode][by][bx];
    OneComponentLumaPrediction (l0_pred, pic_opix_x + curr_mv[0], pic_opix_y + curr_mv[1], block_size_x, block_size_y, listX[LIST_0 + currMB->list_offset][l0_ref_idx]);
    curr_mv = mv_array[LIST_1][l1_ref_idx][l1_mode][by][bx];
    OneComponentLumaPrediction (l1_pred, pic_opix_x + curr_mv[0], pic_opix_y + curr_mv[1], block_size_x, block_size_y, listX[LIST_1 + currMB->list_offset][l1_ref_idx]);
    break;
  default:
    break;
  }

  if (apply_weights)
  {
    if (p_dir==2)
    {
      int wbp0 = wbp_weight[0][l0_ref_idx][l1_ref_idx][0];
      int wbp1 = wbp_weight[1][l0_ref_idx][l1_ref_idx][0];
      int offset = (wp_offset[0][l0_ref_idx][0] + wp_offset[1][l1_ref_idx][0] + 1)>>1;
      int wp_round = 2 * wp_luma_round;
      int weight_denom = luma_log_weight_denom + 1;
      for   (j=block_y; j<block_y4; j++)
        for (i=block_x; i<block_x4; i++)  
          mb_pred[j][i] = iClip1( img->max_imgpel_value, 
          ((wbp0 * *l0pred++ + wbp1 * *l1pred++ + wp_round) >> (weight_denom)) + offset); 
    }
    else if (p_dir==0)
    {
      int wp = wp_weight[0][l0_ref_idx][0];
      int offset = wp_offset[0][l0_ref_idx][0];
      for   (j=block_y; j<block_y4; j++)
        for (i=block_x; i<block_x4; i++)
          mb_pred[j][i] = iClip1( img->max_imgpel_value, 
          ((wp * *l0pred++  + wp_luma_round) >> luma_log_weight_denom) + offset);
    }
    else // (p_dir==1)
    {
      int wp = wp_weight[1][l1_ref_idx][0];
      int offset = wp_offset[1][l1_ref_idx][0];
      for   (j=block_y; j<block_y4; j++)
        for (i=block_x; i<block_x4; i++)
          mb_pred[j][i] = iClip1( img->max_imgpel_value, 
          ((wp * *l1pred++  + wp_luma_round) >> luma_log_weight_denom) + offset );
    }
  }
  else
  {
    if (p_dir==2)
    {
      for   (j=block_y; j<block_y4; j++)
        for (i=block_x; i<block_x4; i++)
          mb_pred[j][i] = (*l0pred++ + *l1pred++ + 1) >> 1;
    }
    else if (p_dir==0)
    {
      for (j=block_y; j<block_y4; j++)
      {
        memcpy(&(mb_pred[j][block_x]), l0pred, block_size_x * sizeof(imgpel));
        l0pred += block_size_x;
      }
    }
    else // (p_dir==1)
    {
      for (j=block_y; j<block_y4; j++)
      {
        memcpy(&(mb_pred[j][block_x]), l1pred, block_size_x * sizeof(imgpel));
        l1pred += block_size_x;
      }
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Predict one Luma block
 ************************************************************************
 */
void LumaPredictionBi ( Macroblock* currMB, //!< Current Macroblock
                        int   block_x,      //!< relative horizontal block coordinate of 4x4 block
                        int   block_y,      //!< relative vertical   block coordinate of 4x4 block
                        int   block_size_x, //!< horizontal block size
                        int   block_size_y, //!< vertical   block size
                        int   l0_mode,      //!< list0 prediction mode (1-7, 0=DIRECT if l1_mode=0)
                        int   l1_mode,      //!< list1 prediction mode (1-7, 0=DIRECT if l0_mode=0)
                        short l0_ref_idx,   //!< reference frame for list0 prediction (-1: Intra4x4 pred. with l0_mode)
                        short l1_ref_idx,   //!< reference frame for list1 prediction 
                        int   list          //!< current list for prediction.
                        )
{
  int  i, j;
  int  block_x4  = block_x + block_size_x;
  int  block_y4  = block_y + block_size_y;
  int  pic_opix_x = ((img->opix_x + block_x) << 2) + IMG_PAD_SIZE_TIMES4;
  int  pic_opix_y = ((img->opix_y + block_y) << 2) + IMG_PAD_SIZE_TIMES4;
  int  bx        = block_x >> 2;
  int  by        = block_y >> 2;
  imgpel* l0pred     = l0_pred;
  imgpel* l1pred     = l1_pred;

  int  apply_weights = ( (active_pps->weighted_pred_flag && (img->type == P_SLICE || img->type == SP_SLICE)) ||
    (active_pps->weighted_bipred_idc && (img->type == B_SLICE)));  
  short   ******mv_array = img->bipred_mv[list]; 
  short   *mv_arrayl0 = mv_array[LIST_0][l0_ref_idx][l0_mode][by][bx];
  short   *mv_arrayl1 = mv_array[LIST_1][l1_ref_idx][l1_mode][by][bx];
  imgpel (*mb_pred)[16] = img->mb_pred[0];

  OneComponentLumaPrediction (l0_pred, pic_opix_x + mv_arrayl0[0], pic_opix_y + mv_arrayl0[1], block_size_x, block_size_y, listX[0+currMB->list_offset][l0_ref_idx]);
  OneComponentLumaPrediction (l1_pred, pic_opix_x + mv_arrayl1[0], pic_opix_y + mv_arrayl1[1], block_size_x, block_size_y, listX[1+currMB->list_offset][l1_ref_idx]);

  if (apply_weights)
  {
    int wbp0 = wbp_weight[0][l0_ref_idx][l1_ref_idx][0];
    int wbp1 = wbp_weight[1][l0_ref_idx][l1_ref_idx][0];
    int offset = (wp_offset[0][l0_ref_idx][0] + wp_offset[1][l1_ref_idx][0] + 1)>>1;
    int wp_round = 2*wp_luma_round;
    int weight_denom = luma_log_weight_denom + 1;

    for   (j=block_y; j<block_y4; j++)
      for (i=block_x; i<block_x4; i++)
        mb_pred[j][i] = iClip1( img->max_imgpel_value,
        ((wbp0 * *l0pred++ + wbp1 * *l1pred++ + wp_round) >> weight_denom) + offset);
  }
  else
  {
    for   (j=block_y; j<block_y4; j++)
      for (i=block_x; i<block_x4; i++)
        mb_pred[j][i] = (*l0pred++ + *l1pred++ + 1) >> 1;
  }
}


/*!
 ************************************************************************
 * \brief
 *    Predict (on-the-fly) one component of a chroma 4x4 block
 ************************************************************************
 */
void OneComponentChromaPrediction4x4_regenerate (
                                 imgpel*     mpred,      //!< array to store prediction values
                                 int         block_c_x,  //!< horizontal pixel coordinate of 4x4 block
                                 int         block_c_y,  //!< vertical   pixel coordinate of 4x4 block
                                 short***    mv,         //!< motion vector array
                                 StorablePicture *list,
                                 int         uv)         //!< chroma component
{
  int     i, j, ii, jj, ii0, jj0, ii1, jj1, if0, if1, jf0, jf1;
  short*  mvb;

  int     f1_x = 64/img->mb_cr_size_x;
  int     f2_x=f1_x-1;

  int     f1_y = 64/img->mb_cr_size_y;
  int     f2_y=f1_y-1;

  int     f3=f1_x*f1_y, f4=f3>>1;
  int     list_offset = img->mb_data[img->current_mb_nr].list_offset;
  int     max_y_cr = (int) (list_offset ? (img->height_cr >> 1) - 1 : img->height_cr - 1);
  int     max_x_cr = (int) (img->width_cr - 1);
  int     jjx, iix;
  int     mb_cr_y_div4 = img->mb_cr_size_y>>2;
  int     mb_cr_x_div4 = img->mb_cr_size_x>>2;
  int     jpos;

  imgpel** refimage = list->imgUV[uv];

  for (j=block_c_y; j < block_c_y + BLOCK_SIZE; j++)
  {
    jjx = j/mb_cr_y_div4;
    jpos = (j + img->opix_c_y)*f1_y;

    for (i=block_c_x; i < block_c_x + BLOCK_SIZE; i++)
    {
      iix = i/mb_cr_x_div4;
      mvb  = mv [jjx][iix];

      ii   = (i + img->opix_c_x)*f1_x + mvb[0];
      jj   = jpos + mvb[1];

      if (active_sps->chroma_format_idc == 1)
        jj  += list->chroma_vector_adjustment;

      ii0  = iClip3 (0, max_x_cr, ii/f1_x);
      jj0  = iClip3 (0, max_y_cr, jj/f1_y);
      ii1  = iClip3 (0, max_x_cr, (ii+f2_x)/f1_x);
      jj1  = iClip3 (0, max_y_cr, (jj+f2_y)/f1_y);

      if1  = (ii&f2_x);  if0 = f1_x-if1;
      jf1  = (jj&f2_y);  jf0 = f1_y-jf1;

      *mpred++ = (if0 * jf0 * refimage[jj0][ii0] +
                  if1 * jf0 * refimage[jj0][ii1] +
                  if0 * jf1 * refimage[jj1][ii0] +
                  if1 * jf1 * refimage[jj1][ii1] + f4) / f3;
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Retrieve one component of a chroma 4x4 block from the buffer
 ************************************************************************
 */
void OneComponentChromaPrediction4x4_retrieve (imgpel*        mpred,      //!< array to store prediction values
                                 int         block_c_x,  //!< horizontal pixel coordinate of 4x4 block
                                 int         block_c_y,  //!< vertical   pixel coordinate of 4x4 block
                                 short***    mv,         //!< motion vector array
                                 StorablePicture *list,
                                 int         uv)         //!< chroma component
{
  int     j, ii, jj;
  short*  mvb;

  int     jjx;
  int     right_shift_x = 4 - chroma_shift_x;
  int     right_shift_y = 4 - chroma_shift_y;
  int     jpos;

  int     pos_x1 = block_c_x >> right_shift_x;
  int     pos_x2 = (block_c_x + 2) >> right_shift_x;
  int     ipos1 = ((block_c_x + img->opix_c_x    ) << chroma_shift_x) + IMG_PAD_SIZE_TIMES4;
  int     ipos2 = ((block_c_x + img->opix_c_x + 2) << chroma_shift_x) + IMG_PAD_SIZE_TIMES4;

  imgpel**** refsubimage = list->imgUV_sub[uv];
  imgpel *line_ptr;

  int jj_chroma = ((active_sps->chroma_format_idc == 1) ? list->chroma_vector_adjustment : 0) + IMG_PAD_SIZE_TIMES4;

  width_pad_cr  = list->size_x_cr_pad;
  height_pad_cr = list->size_y_cr_pad;

  for (j=block_c_y; j < block_c_y + BLOCK_SIZE; j++)
  {
    jjx = j >> right_shift_y; // translate into absolute block (luma) coordinates

    jpos = ( (j + img->opix_c_y) << chroma_shift_y ) + jj_chroma;

    mvb  = mv [jjx][pos_x1];

    ii   = ipos1 + mvb[0];
    jj   = jpos  + mvb[1];

    line_ptr = UMVLine8X_chroma ( refsubimage, jj, ii);
    *mpred++ = *line_ptr++;
    *mpred++ = *line_ptr;

    mvb  = mv [jjx][pos_x2];

    ii   = ipos2 + mvb[0];
    jj   = jpos  + mvb[1];

    line_ptr = UMVLine8X_chroma ( refsubimage, jj, ii);
    *mpred++ = *line_ptr++;
    *mpred++ = *line_ptr;
  }
}


/*!
 ************************************************************************
 * \brief
 *    Retrieve one component of a chroma block from the buffer
 ************************************************************************
 */
static inline 
void OneComponentChromaPrediction ( imgpel* mpred,      //!< array to store prediction values
                                   int    pic_pix_x,      //!< motion shifted horizontal coordinate of block
                                   int    pic_pix_y,      //!< motion shifted vertical  block
                                   int     block_size_x,   //!< horizontal block size
                                   int     block_size_y,   //!< vertical block size                                      
                                   StorablePicture *list, //!< reference picture list
                                   int         uv)         //!< chroma component
{
  int     j;
  imgpel *ref_line = UMVLine4X (list->imgUV_sub[uv], pic_pix_y, pic_pix_x);

  width_pad_cr  = list->size_x_cr_pad;
  height_pad_cr = list->size_y_cr_pad;

  for (j = 0; j < block_size_y; j++) 
  {
    memcpy(mpred, ref_line, block_size_x * sizeof(imgpel));
    ref_line += img_cr_padded_size_x;
    mpred += block_size_x;
  }
}

/*!
 ************************************************************************
 * \brief
 *    Predict an intra chroma 4x4 block
 ************************************************************************
 */
static inline
void IntraChromaPrediction4x4 (Macroblock* currMB, //! <-- Current Macroblock
                               int  uv,            //! <-- colour component
                               int  block_x,       //! <-- relative horizontal block coordinate of 4x4 block
                               int  block_y)       //! <-- relative vertical   block coordinate of 4x4 block
{
  int j;
  imgpel (*mb_pred)[16]       = img->mb_pred[ uv ];
  imgpel (*curr_mpr_16x16)[16] = img->mpr_16x16[uv][currMB->c_ipred_mode];

  //===== prediction =====
  for (j=block_y; j<block_y + BLOCK_SIZE; j++)
    memcpy(&mb_pred[j][block_x],&curr_mpr_16x16[j][block_x], BLOCK_SIZE * sizeof(imgpel));
}

/*!
 ************************************************************************
 * \brief
 *    Predict one chroma block
 ************************************************************************
 */
void ChromaPrediction ( Macroblock* currMB, // <-- Current Macroblock
                       int   uv,            // <-- colour component
                       int   block_x,       // <-- relative horizontal block coordinate of block
                       int   block_y,       // <-- relative vertical   block coordinate of block
                       int   block_size_x,  // <-- relative horizontal block coordinate of block
                       int   block_size_y,  // <-- relative vertical   block coordinate of block                        
                       int   p_dir,         // <-- prediction direction (0=list0, 1=list1, 2=bipred)
                       int   l0_mode,       // <-- list0  prediction mode (1-7, 0=DIRECT if l1_mode=0)
                       int   l1_mode,       // <-- list1 prediction mode (1-7, 0=DIRECT if l0_mode=0)
                       short l0_ref_idx,    // <-- reference frame for list0 prediction (if (<0) -> intra prediction)
                       short l1_ref_idx,    // <-- reference frame for list1 prediction 
                       short bipred_me      // <-- use bi prediction mv (0=no bipred, 1 = use set 1, 2 = use set 2)
                       )    
{
  int  i, j;
  int  block_x4     = block_x + block_size_x;
  int  block_y4     = block_y + block_size_y;
  int  pic_opix_x   = ((img->opix_c_x + block_x) << 2) + IMG_PAD_SIZE_TIMES4;
  int  pic_opix_y   = ((img->opix_c_y + block_y) << 2) + IMG_PAD_SIZE_TIMES4;
  int  bx           = block_x >> 2;
  int  by           = block_y >> 2;
  imgpel* l0pred     = l0_pred;
  imgpel* l1pred     = l1_pred;
  short****** mv_array = img->all_mv;    
  int max_imgpel_value_uv = img->max_imgpel_value_comp[1];
  int uv_comp = uv + 1;
  imgpel (*mb_pred)[16] = img->mb_pred[ uv_comp];

  int  apply_weights = ( (active_pps->weighted_pred_flag && (img->type == P_SLICE || img->type == SP_SLICE)) ||
    (active_pps->weighted_bipred_idc && (img->type == B_SLICE)));

  if (bipred_me && l0_ref_idx == 0 && l1_ref_idx == 0 && p_dir == 2 && is_bipred_enabled(l0_mode)  && is_bipred_enabled(l1_mode))
    mv_array = img->bipred_mv[bipred_me - 1]; 

  //===== INTRA PREDICTION =====
  if (p_dir==-1)
  {
    IntraChromaPrediction4x4 (currMB, uv_comp, block_x, block_y);
    return;
  }

  //===== INTER PREDICTION =====
  switch (p_dir)
  {
  case 0:
    OneComponentChromaPrediction (l0_pred, pic_opix_x + mv_array[LIST_0][l0_ref_idx][l0_mode][by][bx][0], pic_opix_y + mv_array[LIST_0][l0_ref_idx][l0_mode][by][bx][1], block_size_x, block_size_y, listX[0+currMB->list_offset][l0_ref_idx], uv);
    break;
  case 1: 
    OneComponentChromaPrediction (l1_pred, pic_opix_x + mv_array[LIST_1][l1_ref_idx][l1_mode][by][bx][0], pic_opix_y + mv_array[LIST_1][l1_ref_idx][l1_mode][by][bx][1], block_size_x, block_size_y, listX[1+currMB->list_offset][l1_ref_idx], uv);
    break;
  case 2:
    OneComponentChromaPrediction (l0_pred, pic_opix_x + mv_array[LIST_0][l0_ref_idx][l0_mode][by][bx][0], pic_opix_y + mv_array[LIST_0][l0_ref_idx][l0_mode][by][bx][1], block_size_x, block_size_y, listX[0+currMB->list_offset][l0_ref_idx], uv);
    OneComponentChromaPrediction (l1_pred, pic_opix_x + mv_array[LIST_1][l1_ref_idx][l1_mode][by][bx][0], pic_opix_y + mv_array[LIST_1][l1_ref_idx][l1_mode][by][bx][1], block_size_x, block_size_y, listX[1+currMB->list_offset][l1_ref_idx], uv);
    break;
  default:
    break;
  }

  if (apply_weights)
  {
    if (p_dir==2)
    {
      int wbp0 = wbp_weight[0][l0_ref_idx][l1_ref_idx][uv_comp];
      int wbp1 = wbp_weight[1][l0_ref_idx][l1_ref_idx][uv_comp];
      int offset = (wp_offset[0][l0_ref_idx][uv_comp] + wp_offset[1][l1_ref_idx][uv_comp] + 1)>>1;
      int wp_round = 2*wp_chroma_round;
      int weight_denom = luma_log_weight_denom + 1;


      for   (j=block_y; j<block_y4; j++)
        for (i=block_x; i<block_x4; i++)
          mb_pred[j][i] =  iClip1( max_imgpel_value_uv,
          ((wbp0 * *l0pred++ + wbp1 * *l1pred++ + wp_round) >> (weight_denom)) + (offset) );
    }
    else if (p_dir==0)
    {
      int wp = wp_weight[0][l0_ref_idx][uv_comp];
      int offset = wp_offset[0][l0_ref_idx][uv_comp];
      for   (j=block_y; j<block_y4; j++)
        for (i=block_x; i<block_x4; i++)
          mb_pred[j][i] = iClip1( max_imgpel_value_uv, (( wp * *l0pred++ + wp_chroma_round) >> chroma_log_weight_denom) +  offset);
    }
    else // (p_dir==1)
    {
      int wp = wp_weight[1][l1_ref_idx][uv_comp];
      int offset = wp_offset[1][l1_ref_idx][uv_comp];

      for   (j=block_y; j<block_y4; j++)
        for (i=block_x; i<block_x4; i++)
          mb_pred[j][i] = iClip1( max_imgpel_value_uv, ((wp * *l1pred++ + wp_chroma_round) >> chroma_log_weight_denom) + offset);
    }
  }
  else
  {
    if (p_dir==2)
    {
      for   (j=block_y; j<block_y4; j++)
        for (i=block_x; i<block_x4; i++)
          mb_pred[j][i] = (*l0pred++ + *l1pred++ + 1) >> 1;
    }
    else if (p_dir==0)
    {
      for (j=block_y; j<block_y4; j++)
      {
        memcpy(&(mb_pred[j][block_x]), l0pred, block_size_x * sizeof(imgpel));
        l0pred += block_size_x;
      }
    }
    else // (p_dir==1)
    {
      for (j=block_y; j<block_y4; j++)
      {
        memcpy(&(mb_pred[j][block_x]), l1pred, block_size_x * sizeof(imgpel));
        l1pred += block_size_x;
      }
    }
  }
}



/*!
 ************************************************************************
 * \brief
 *    Predict one chroma 4x4 block
 ************************************************************************
 */
void ChromaPrediction4x4 ( Macroblock* currMB, // <-- Current Macroblock
                           int   uv,           // <-- colour component
                           int   block_x,      // <-- relative horizontal block coordinate of 4x4 block
                           int   block_y,      // <-- relative vertical   block coordinate of 4x4 block
                           int   p_dir,        // <-- prediction direction (0=list0, 1=list1, 2=bipred)
                           int   l0_mode,      // <-- list0  prediction mode (1-7, 0=DIRECT if l1_mode=0)
                           int   l1_mode,      // <-- list1 prediction mode (1-7, 0=DIRECT if l0_mode=0)
                           short l0_ref_idx,   // <-- reference frame for list0 prediction (if (<0) -> intra prediction)
                           short l1_ref_idx,   // <-- reference frame for list1 prediction 
                           short bipred_me     // <-- use bi prediction mv (0=no bipred, 1 = use set 1, 2 = use set 2)
                           )   
{
  int  i, j;
  int  block_x4  = block_x + BLOCK_SIZE;
  int  block_y4  = block_y + BLOCK_SIZE;
  imgpel* l0pred     = l0_pred;
  imgpel* l1pred     = l1_pred;
  short****** mv_array = img->all_mv;
  int max_imgpel_value_uv = img->max_imgpel_value_comp[1];
  int uv_comp = uv + 1;
  imgpel (*mb_pred)[16] = img->mb_pred[uv_comp];
  int     list_offset = currMB->list_offset;
  
  int  apply_weights = ( (active_pps->weighted_pred_flag && (img->type == P_SLICE || img->type == SP_SLICE)) ||
    (active_pps->weighted_bipred_idc && (img->type == B_SLICE)));

  if (bipred_me && l0_ref_idx == 0 && l1_ref_idx == 0 && p_dir == 2 && is_bipred_enabled(l0_mode)  && is_bipred_enabled(l1_mode) )
    mv_array = img->bipred_mv[bipred_me - 1]; 
  //===== INTRA PREDICTION =====
  
  if (p_dir==-1)
  {
    IntraChromaPrediction4x4 (currMB, uv_comp, block_x, block_y);
    return;
  }

  //===== INTER PREDICTION =====
  switch (p_dir)
  {
  case 0: // LIST_0
    (*OneComponentChromaPrediction4x4) (l0_pred, block_x, block_y, mv_array[LIST_0][l0_ref_idx][l0_mode], listX[LIST_0 + list_offset][l0_ref_idx], uv);
    break;
  case 1: // LIST_1
    (*OneComponentChromaPrediction4x4) (l1_pred, block_x, block_y, mv_array[LIST_1][l1_ref_idx][l1_mode], listX[LIST_1 + list_offset][l1_ref_idx], uv);
    break;
  case 2: // BI_PRED
    (*OneComponentChromaPrediction4x4) (l0_pred, block_x, block_y, mv_array[LIST_0][l0_ref_idx][l0_mode], listX[LIST_0 + list_offset][l0_ref_idx], uv);
    (*OneComponentChromaPrediction4x4) (l1_pred, block_x, block_y, mv_array[LIST_1][l1_ref_idx][l1_mode], listX[LIST_1 + list_offset][l1_ref_idx], uv);
    break;
  default:
    break;
  }

  if (apply_weights)
  {
    if (p_dir==2)
    {
      int wbp0 = wbp_weight[0][l0_ref_idx][l1_ref_idx][uv_comp];
      int wbp1 = wbp_weight[1][l0_ref_idx][l1_ref_idx][uv_comp];
      int offset = (wp_offset[0][l0_ref_idx][uv_comp] + wp_offset[1][l1_ref_idx][uv_comp] + 1)>>1;
      int wp_round = 2 * wp_chroma_round;
      int weight_denom = luma_log_weight_denom + 1;

      for (j=block_y; j<block_y4; j++)
        for (i=block_x; i<block_x4; i++)
          mb_pred[j][i] =  iClip1( max_imgpel_value_uv,
          ((wbp0 * *l0pred++ + wbp1 * *l1pred++ + wp_round) >> (weight_denom)) + (offset) );
    }
    else if (p_dir==0)
    {
      int wp = wp_weight[0][l0_ref_idx][uv_comp];
      int offset = wp_offset[0][l0_ref_idx][uv_comp];
      for (j=block_y; j<block_y4; j++)
        for (i=block_x; i<block_x4; i++)
          mb_pred[j][i] = iClip1( max_imgpel_value_uv, (( wp * *l0pred++ + wp_chroma_round) >> chroma_log_weight_denom) +  offset);
    }
    else // (p_dir==1)
    {
      int wp = wp_weight[1][l1_ref_idx][uv_comp];
      int offset = wp_offset[1][l1_ref_idx][uv_comp];

      for (j=block_y; j<block_y4; j++)
        for (i=block_x; i<block_x4; i++)
          mb_pred[j][i] = iClip1( max_imgpel_value_uv, ((wp * *l1pred++ + wp_chroma_round) >> chroma_log_weight_denom) + offset);
    }
  }
  else
  {
    if (p_dir==2)
    {
      for (j=block_y; j<block_y4; j++)
        for (i=block_x; i<block_x4; i++)
          mb_pred[j][i] = (*l0pred++ + *l1pred++ + 1) >> 1;
    }
    else if (p_dir==0)
    {
      for (j=block_y; j<block_y4; j++)
      {
        memcpy(&(mb_pred[j][block_x]), l0pred, BLOCK_SIZE * sizeof(imgpel));
        l0pred += BLOCK_SIZE;
      }
    }
    else // (p_dir==1)
    {
      for (j=block_y; j<block_y4; j++)
      {
        memcpy(&(mb_pred[j][block_x]), l1pred, BLOCK_SIZE * sizeof(imgpel));
        l1pred += BLOCK_SIZE;
      }
    }
  }
}


/*!
 ************************************************************************
 * \brief
 *    Intra prediction of the chrminance layers of one macroblock
 ************************************************************************
 */
void IntraChromaPrediction (Macroblock *currMB, int *mb_up, int *mb_left, int*mb_up_left)
{
  static int s, s0, s1, s2, s3, i, j, k;
  static int ih,iv, ib, ic, iaa;
  int      uv;
  int      blk_x, blk_y;
  int      b8,b4;
  imgpel**  image;
  imgpel   vline[16];
  int      block_x, block_y;
  int      mb_available_up;
  int      mb_available_left[2];
  int      mb_available_up_left;

  int      mode;
  int      best_mode = DC_PRED_8;  //just an initilaization here, should always be overwritten
  int      cost;
  int      min_cost;
  PixelPos up;        //!< pixel position  p(0,-1)
  PixelPos left[17];  //!< pixel positions p(-1, -1..15)
  int      cr_MB_x = img->mb_cr_size_x;
  int      cr_MB_y = img->mb_cr_size_y;
  static imgpel (*cur_pred)[16];
  static imgpel (*curr_mpr_16x16)[16][16];
  static imgpel *hline;
  static imgpel *img_org, *img_prd;

  int      yuv = img->yuv_format - 1;
  int      dc_pred_value_chroma = img->dc_pred_value_comp[1];
  int      max_imgpel_value_uv  = img->max_imgpel_value_comp[1];

  static const int block_pos[3][4][4]= //[yuv][b8][b4]
  {
    { {0, 1, 2, 3},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}},
    { {0, 1, 2, 3},{2, 3, 2, 3},{0, 0, 0, 0},{0, 0, 0, 0}},
    { {0, 1, 2, 3},{1, 1, 3, 3},{2, 3, 2, 3},{3, 3, 3, 3}}
  };

  for (i=0;i<cr_MB_y+1;i++)
  {
    getNeighbour(currMB, -1 , i-1 , img->mb_size[IS_CHROMA], &left[i]);
  }
  getNeighbour(currMB, 0 , -1 , img->mb_size[IS_CHROMA], &up);


  mb_available_up                             = up.available;
  mb_available_up_left                        = left[0].available;
  mb_available_left[0] = mb_available_left[1] = left[1].available;

  if(params->UseConstrainedIntraPred)
  {
    mb_available_up = up.available ? img->intra_block[up.mb_addr] : 0;
    for (i=0, mb_available_left[0]=1; i<(cr_MB_y>>1);i++)
      mb_available_left[0]  &= left[i+1].available ? img->intra_block[left[i+1].mb_addr]: 0;
    for (i=(cr_MB_y>>1), mb_available_left[1]=1; i<cr_MB_y;i++)
      mb_available_left[1] &= left[i+1].available ? img->intra_block[left[i+1].mb_addr]: 0;
    mb_available_up_left = left[0].available ? img->intra_block[left[0].mb_addr]: 0;
  }

  if (mb_up)
    *mb_up = mb_available_up;
  if (mb_left)
    *mb_left = mb_available_left[0] && mb_available_left[1];
  if (mb_up_left)
    *mb_up_left = mb_available_up_left;


  // compute all chroma intra prediction modes for both U and V
  for (uv=0; uv<2; uv++)
  {
    image          = enc_picture->imgUV[uv];
    curr_mpr_16x16 = img->mpr_16x16[uv + 1];

    // DC prediction
    for(b8=0; b8<img->num_blk8x8_uv >> 1;b8++)
    {
      for (b4=0; b4<4; b4++)
      {
        block_y = subblk_offset_y[yuv][b8][b4];
        block_x = subblk_offset_x[yuv][b8][b4];
        blk_x = block_x;
        blk_y = block_y + 1;

        s=dc_pred_value_chroma;
        s0=s1=s2=s3=0;

        //===== get prediction value =====
        switch (block_pos[yuv][b8][b4])
        {
        case 0:  //===== TOP LEFT =====
          if      (mb_available_up)       
            for (i=blk_x;i<(blk_x+4);i++)  
              s0 += image[up.pos_y][up.pos_x + i];
          if      (mb_available_left[0])  
            for (i=blk_y;i<(blk_y+4);i++)  
              s2 += image[left[i].pos_y][left[i].pos_x];
          if      (mb_available_up && mb_available_left[0])  
            s  = (s0+s2+4) >> 3;
          else if (mb_available_up)                          
            s  = (s0   +2) >> 2;
          else if (mb_available_left[0])                     
            s  = (s2   +2) >> 2;
          break;
        case 1: //===== TOP RIGHT =====
          if      (mb_available_up)       
            for (i=blk_x;i<(blk_x+4);i++)  
              s1 += image[up.pos_y][up.pos_x + i];
          else if (mb_available_left[0])  
            for (i=blk_y;i<(blk_y+4);i++) 
              s2 += image[left[i].pos_y][left[i].pos_x];
          if      (mb_available_up)       
            s  = (s1   +2) >> 2;
          else if (mb_available_left[0])                    
            s  = (s2   +2) >> 2;
          break;
        case 2: //===== BOTTOM LEFT =====
          if      (mb_available_left[1])  
            for (i=blk_y;i<(blk_y+4);i++)  
              s3 += image[left[i].pos_y][left[i].pos_x];
          else if (mb_available_up)       
            for (i=blk_x;i<(blk_x+4);i++)  
              s0 += image[up.pos_y][up.pos_x + i];
          if      (mb_available_left[1])                     
            s  = (s3   +2) >> 2;
          else if (mb_available_up)                          
            s  = (s0   +2) >> 2;
          break;
        case 3: //===== BOTTOM RIGHT =====
          if      (mb_available_up)       
            for (i=blk_x;i<(blk_x+4);i++)  
              s1 += image[up.pos_y][up.pos_x + i];
          if      (mb_available_left[1])  
            for (i=blk_y;i<(blk_y+4);i++)  
              s3 += image[left[i].pos_y][left[i].pos_x];
          if      (mb_available_up && mb_available_left[1])  
            s  = (s1+s3+4) >> 3;
          else if (mb_available_up)                          
            s  = (s1   +2) >> 2;
          else if (mb_available_left[1])                     
            s  = (s3   +2) >> 2;
          break;
        }

        //===== prediction =====
        cur_pred = curr_mpr_16x16[DC_PRED_8];
        for (j=block_y; j<block_y+4; j++)
          for (i=block_x; i<block_x+4; i++)
          {
            cur_pred[j][i] = s;
          }
      }
    }

    // vertical prediction    
    if (mb_available_up)
    {
      cur_pred = curr_mpr_16x16[VERT_PRED_8];
      //memcpy(hline,&image[up.pos_y][up.pos_x], cr_MB_x * sizeof(imgpel));
      hline = &image[up.pos_y][up.pos_x];
      for (j=0; j<cr_MB_y; j++)
        memcpy(cur_pred[j], hline, cr_MB_x * sizeof(imgpel));
    }

    // horizontal prediction
    if (mb_available_left[0] && mb_available_left[1])
    {
      cur_pred = curr_mpr_16x16[HOR_PRED_8];
      for (i=0; i<cr_MB_y; i++)
        vline[i] = image[left[i+1].pos_y][left[i+1].pos_x];
      for (j=0; j<cr_MB_y; j++)
      {
        int predictor = vline[j];
        for (i=0; i<cr_MB_x; i++)        
          cur_pred[j][i] = predictor;
      }
    }

    // plane prediction
    if (mb_available_left[0] && mb_available_left[1] && mb_available_up && mb_available_up_left)
    {
      ih = (cr_MB_x>>1)*(hline[cr_MB_x-1] - image[left[0].pos_y][left[0].pos_x]);
      for (i=0;i<(cr_MB_x>>1)-1;i++)
        ih += (i+1)*(hline[(cr_MB_x>>1)+i] - hline[(cr_MB_x>>1)-2-i]);

      iv = (cr_MB_y>>1)*(vline[cr_MB_y-1] - image[left[0].pos_y][left[0].pos_x]);
      for (i=0;i<(cr_MB_y>>1)-1;i++)
        iv += (i+1)*(vline[(cr_MB_y>>1)+i] - vline[(cr_MB_y>>1)-2-i]);

      ib= ((cr_MB_x == 8?17:5)*ih+2*cr_MB_x)>>(cr_MB_x == 8?5:6);
      ic= ((cr_MB_y == 8?17:5)*iv+2*cr_MB_y)>>(cr_MB_y == 8?5:6);

      iaa=16*(hline[cr_MB_x-1] + vline[cr_MB_y-1]);
      cur_pred = curr_mpr_16x16[PLANE_8];
      for (j=0; j<cr_MB_y; j++)
        for (i=0; i<cr_MB_x; i++)
          cur_pred[j][i]= iClip1( max_imgpel_value_uv, (iaa+(i-(cr_MB_x>>1)+1)*ib+(j-(cr_MB_y>>1)+1)*ic+16)>>5);
    }
  }

  if (!params->rdopt)      // the rd-opt part does not work correctly (see encode_one_macroblock)
  {                       // since ipredmodes could be overwritten => encoder-decoder-mismatches
    // pick lowest cost prediction mode
    min_cost = INT_MAX;
    for (i=0;i<cr_MB_y;i++)
    {
      getNeighbour(currMB, 0 , i, img->mb_size[IS_CHROMA], &left[i]);
    }

    if ( img->MbaffFrameFlag && img->field_mode )
    {
      for (i=0;i<cr_MB_y;i++)
      {
        left[i].pos_y = left[i].pos_y >> 1;
      }
    }

    for (mode=DC_PRED_8; mode<=PLANE_8; mode++)
    {
      if ((img->type != I_SLICE || !params->IntraDisableInterOnly) && params->ChromaIntraDisable == 1 && mode!=DC_PRED_8)
        continue;

      if ((mode==VERT_PRED_8 && !mb_available_up) ||
        (mode==HOR_PRED_8 && (!mb_available_left[0] || !mb_available_left[1])) ||
        (mode==PLANE_8 && (!mb_available_left[0] || !mb_available_left[1] || !mb_available_up || !mb_available_up_left)))
        continue;

      cost = 0;
      for (uv = 1; uv < 3; uv++)
      {
        image = pImgOrg[uv];
        curr_mpr_16x16 = img->mpr_16x16[uv];
        for (block_y=0; block_y<cr_MB_y; block_y+=4)
          for (block_x = 0; block_x < cr_MB_x; block_x += 4)
          {
            for (k=0, j = block_y; j < block_y + 4; j++)
            {
              img_prd = curr_mpr_16x16[mode][j];
              img_org = &image[left[j].pos_y][left[j].pos_x];
              for (i = block_x; i < block_x + 4; i++)
                diff[k++] = img_org[i] - img_prd[i];
            }
            cost += distortion4x4(diff);
          }
      }
      if (cost < min_cost)
      {
        best_mode = mode;
        min_cost = cost;
      }
    }
    currMB->c_ipred_mode = best_mode;
  }
}

void ComputeResidue (imgpel **curImg, imgpel mpr[16][16], int img_m7[16][16], int mb_y, int mb_x, int opix_y, int opix_x, int width, int height)
{
  static imgpel *imgOrg, *imgPred;
  static int    *m7;
  int i, j;

  for (j = mb_y; j < mb_y + height; j++)
  {
    imgOrg = &curImg[opix_y + j][opix_x];    
    imgPred = &mpr[j][mb_x];
    m7 = &img_m7[j][mb_x]; 
    for (i = 0; i < width; i++)
    {
      *m7++ = *imgOrg++ - *imgPred++;
    }
  }
}

void SampleReconstruct (imgpel **curImg, imgpel mpr[16][16], int img_m7[16][16], int mb_y, int mb_x, int opix_y, int opix_x, int width, int height, int max_imgpel_value, int dq_bits)
{
  static imgpel *imgOrg, *imgPred;
  static int    *m7;
  int i, j;

  for (j = mb_y; j < mb_y + height; j++)
  {
    imgOrg = &curImg[opix_y + j][opix_x];
    imgPred = &mpr[j][mb_x];
    m7 = &img_m7[j][mb_x]; 
    for (i=0;i<width;i++)
      *imgOrg++ = iClip1( max_imgpel_value, rshift_rnd_sf(*m7++, dq_bits) + *imgPred++);
  }
}


