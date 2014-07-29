/*!
 ***************************************************************************
 * \file rd_intra_jm.c
 *
 * \brief
 *    Rate-Distortion optimized mode decision
 *
 * \author
 *    - Heiko Schwarz              <hschwarz@hhi.de>
 *    - Valeri George              <george@hhi.de>
 *    - Lowell Winger              <lwinger@lsil.com>
 *    - Alexis Michael Tourapis    <alexismt@ieee.org>
 * \date
 *    12. April 2001
 **************************************************************************
 */

#include <limits.h>

#include "global.h"

#include "image.h"
#include "macroblock.h"
#include "mb_access.h"
#include "rdopt_coding_state.h"
#include "mode_decision.h"
#include "rdopt.h"
#include "rd_intra_jm.h"
#include "q_around.h"

/*!
 *************************************************************************************
 * \brief
 *    Mode Decision for an 4x4 Intra block
 *************************************************************************************
 */
int Mode_Decision_for_4x4IntraBlocks_JM_High (Macroblock *currMB, int  b8,  int  b4,  double  lambda,  double*  min_cost, int cr_cbp[3], int is_cavlc)
{
  int    ipmode, best_ipmode = 0, i, j, y, dummy;
  int    c_nz, nonzero = 0;
  int*   ACLevel = img->cofAC[b8][b4][0];
  int*   ACRun   = img->cofAC[b8][b4][1];
  int    c_nzCbCr[3]= {999,999, 999};
  static imgpel  rec4x4[4][4];
  static imgpel  rec4x4CbCr[2][4][4];
  int    uv;

  double rdcost;
  int    block_x     = ((b8 & 0x01) << 3) + ((b4 & 0x01) << 2);
  int    block_y     = ((b8 >> 1) << 3)  + ((b4 >> 1) << 2);
  int    pic_pix_x   = img->pix_x  + block_x;
  int    pic_pix_y   = img->pix_y  + block_y;
  int    pic_opix_x  = img->opix_x + block_x;
  int    pic_opix_y  = img->opix_y + block_y;
  int    pic_block_x = pic_pix_x >> 2;
  int    pic_block_y = pic_pix_y >> 2;
  double min_rdcost  = 1e30;

  int left_available, up_available, all_available;

  char   upMode;
  char   leftMode;
  int    mostProbableMode;

  PixelPos left_block;
  PixelPos top_block;

  int  lrec4x4[4][4];

#ifdef BEST_NZ_COEFF
  int best_nz_coeff = 0;
  int best_coded_block_flag = 0;
  int bit_pos = 1 + ((((b8>>1)<<1)+(b4>>1))<<2) + (((b8&1)<<1)+(b4&1));
  static int64 cbp_bits;

  if (b8==0 && b4==0)
    cbp_bits = 0;
#endif

  get4x4Neighbour(currMB, block_x - 1, block_y    , img->mb_size[IS_LUMA], &left_block);
  get4x4Neighbour(currMB, block_x,     block_y - 1, img->mb_size[IS_LUMA], &top_block );

  // constrained intra pred
  if (params->UseConstrainedIntraPred)
  {
    left_block.available = left_block.available ? img->intra_block[left_block.mb_addr] : 0;
    top_block.available  = top_block.available  ? img->intra_block[top_block.mb_addr]  : 0;
  }

  upMode            =  top_block.available ? img->ipredmode[top_block.pos_y ][top_block.pos_x ] : -1;
  leftMode          = left_block.available ? img->ipredmode[left_block.pos_y][left_block.pos_x] : -1;

  mostProbableMode  = (upMode < 0 || leftMode < 0) ? DC_PRED : upMode < leftMode ? upMode : leftMode;

  *min_cost = INT_MAX;

  ipmode_DPCM = NO_INTRA_PMODE; ////For residual DPCM

  //===== INTRA PREDICTION FOR 4x4 BLOCK =====
  intrapred_4x4 (currMB, PLANE_Y, pic_pix_x, pic_pix_y, &left_available, &up_available, &all_available);

  if (img->P444_joined)
  {
    select_plane(PLANE_U);
    intrapred_4x4 (currMB, PLANE_U, pic_pix_x, pic_pix_y, &left_available, &up_available, &all_available);
    select_plane(PLANE_V);
    intrapred_4x4 (currMB, PLANE_V, pic_pix_x, pic_pix_y, &left_available, &up_available, &all_available);
    select_plane(PLANE_Y);
  }

  //===== LOOP OVER ALL 4x4 INTRA PREDICTION MODES =====
  for (ipmode = 0; ipmode < NO_INTRA_PMODE; ipmode++)
  {
    int available_mode =  (all_available) || (ipmode==DC_PRED) ||
      (up_available && (ipmode==VERT_PRED||ipmode==VERT_LEFT_PRED||ipmode==DIAG_DOWN_LEFT_PRED)) ||
      (left_available && (ipmode==HOR_PRED||ipmode==HOR_UP_PRED));

    if (valid_intra_mode(ipmode) == 0)
      continue;

    if( available_mode)
    {
      // get prediction and prediction error
      generate_pred_error(&pCurImg[pic_opix_y], img->mpr_4x4[0][ipmode], &img->mb_pred[0][block_y], &img->mb_ores[0][block_y], pic_opix_x, block_x);     

      if (img->yuv_format == YUV444)
      {
        ipmode_DPCM = ipmode;
        if (!IS_INDEPENDENT(params)) 
        {
          generate_pred_error(&pImgOrg[1][pic_opix_y], img->mpr_4x4[1][ipmode], &img->mb_pred[1][block_y], &img->mb_ores[1][block_y], pic_opix_x, block_x);
          generate_pred_error(&pImgOrg[2][pic_opix_y], img->mpr_4x4[2][ipmode], &img->mb_pred[2][block_y], &img->mb_ores[2][block_y], pic_opix_x, block_x);     
        }
      }

      //===== store the coding state =====
      //store_coding_state (currMB, cs_cm);
      // get and check rate-distortion cost
#ifdef BEST_NZ_COEFF
      currMB->cbp_bits[0] = cbp_bits;
#endif      
      if ((rdcost = RDCost_for_4x4IntraBlocks (currMB, &c_nz, b8, b4, ipmode, lambda, mostProbableMode, c_nzCbCr, is_cavlc)) < min_rdcost)
      {
        //--- set coefficients ---
        memcpy(cofAC4x4[0],ACLevel, 18 * sizeof(int));
        memcpy(cofAC4x4[1],ACRun,   18 * sizeof(int));

        //--- set reconstruction ---
        for (y=0; y<4; y++)
        {
          memcpy(rec4x4[y],&enc_picture->imgY[pic_pix_y+y][pic_pix_x], BLOCK_SIZE * sizeof(imgpel));
        }

        // SP/SI reconstruction
        if(img->type==SP_SLICE &&(!si_frame_indicator && !sp2_frame_indicator))
        {
          for (y=0; y<4; y++)
          {
            memcpy(lrec4x4[y],&lrec[pic_pix_y+y][pic_pix_x], BLOCK_SIZE * sizeof(int));// stores the mode coefficients
          }
        }

        if(img->P444_joined) 
        { 
          //--- set coefficients ---
          for (uv=0; uv < 2; uv++)
          {
            memcpy(cofAC4x4CbCr[uv][0],img->cofAC[b8+4+uv*4][b4][0], 18 * sizeof(int));
            memcpy(cofAC4x4CbCr[uv][1],img->cofAC[b8+4+uv*4][b4][1], 18 * sizeof(int));
            cr_cbp[uv + 1] = c_nzCbCr[uv + 1];

            //--- set reconstruction ---
            for (y=0; y<4; y++)
            {
              memcpy(rec4x4CbCr[uv][y],&enc_picture->imgUV[uv][pic_pix_y+y][pic_pix_x], BLOCK_SIZE * sizeof(imgpel));
            } 
          }
        }
        //--- flag if dct-coefficients must be coded ---
        nonzero = c_nz;

        //--- set best mode update minimum cost ---
        *min_cost     = rdcost;
        min_rdcost    = rdcost;
        best_ipmode   = ipmode;
#ifdef BEST_NZ_COEFF
        best_nz_coeff = img->nz_coeff [img->current_mb_nr][block_x4][block_y4];
        best_coded_block_flag = (int)((currMB->cbp_bits[0] >> bit_pos)&(int64)(1));
#endif
        //store_coding_state (currMB, cs_ib4);
        if (img->AdaptiveRounding)
        {
          store_adaptive_rounding (img, block_y, block_x);
        }

      }
#ifndef RESET_STATE
      reset_coding_state (currMB, cs_cm);
#endif      
    }
  }
#ifdef BEST_NZ_COEFF
  img->nz_coeff [img->current_mb_nr][block_x4][block_y4] = best_nz_coeff;
  cbp_bits &= (~(int64)(1<<bit_pos));
  cbp_bits |= (int64)(best_coded_block_flag<<bit_pos);
#endif
  //===== set intra mode prediction =====
  img->ipredmode[pic_block_y][pic_block_x] = (char) best_ipmode;
  currMB->intra_pred_modes[4*b8+b4] =
    (char) (mostProbableMode == best_ipmode ? -1 : (best_ipmode < mostProbableMode ? best_ipmode : best_ipmode-1));

  if(img->P444_joined)
  {
    ColorPlane k;
    for (k = PLANE_U; k <= PLANE_V; k++)
    {
      select_plane(k);
      for (j=0; j<4; j++)
      {
        for (i=0; i<4; i++)
        {
          img->mb_pred[k][block_y+j][block_x+i]  = img->mpr_4x4[k][best_ipmode][j][i];
          img->mb_ores[k][block_y+j][block_x+i]   = pImgOrg[k][img->pix_y+block_y+j][img->pix_x+block_x+i] - img->mpr_4x4[k][best_ipmode][j][i];
        }
      }
      cr_cbp[k] = pDCT_4x4(currMB, k, block_x,block_y,&dummy,1, is_cavlc);
    }
    select_plane(PLANE_Y);
  }
  //===== restore coefficients =====
  memcpy (ACLevel,cofAC4x4[0], 18 * sizeof(int));
  memcpy (ACRun,cofAC4x4[1], 18 * sizeof(int));

  //===== restore reconstruction and prediction (needed if single coeffs are removed) =====
  for (y=0; y<BLOCK_SIZE; y++)
  {
    memcpy (&enc_picture->imgY[pic_pix_y + y][pic_pix_x],rec4x4[y],    BLOCK_SIZE * sizeof(imgpel));
    memcpy (&img->mb_pred[0][block_y + y][block_x],img->mpr_4x4[0][best_ipmode][y], BLOCK_SIZE * sizeof(imgpel));
  }

  // SP/SI reconstuction
  if(img->type==SP_SLICE &&(!si_frame_indicator && !sp2_frame_indicator))
  {
    for (y=0; y<BLOCK_SIZE; y++)
    {
      memcpy (&lrec[pic_pix_y+y][pic_pix_x], lrec4x4[y], BLOCK_SIZE * sizeof(int));//restore coefficients when encoding primary SP frame
    }
  }
  if (img->P444_joined) 
  {
    for (uv=0; uv < 2; uv++ )
    {
      //===== restore coefficients =====
      memcpy(img->cofAC[b8+4+uv*4][b4][0], cofAC4x4CbCr[uv][0], 18 * sizeof(int));
      memcpy(img->cofAC[b8+4+uv*4][b4][1], cofAC4x4CbCr[uv][1], 18 * sizeof(int));
      //===== restore reconstruction and prediction (needed if single coeffs are removed) =====
      for (y=0; y<BLOCK_SIZE; y++)
      {
        memcpy(&enc_picture->imgUV[uv][pic_pix_y+y][pic_pix_x],rec4x4CbCr[uv][y], BLOCK_SIZE * sizeof(imgpel));
        memcpy(&img->mb_pred[uv + 1][block_y+y][block_x], img->mpr_4x4[uv + 1][best_ipmode][y], BLOCK_SIZE * sizeof(imgpel));
      } 
    }
  }

  if (img->AdaptiveRounding)
  {
    update_adaptive_rounding(img, block_y, block_x);
  }


  return nonzero;
}

/*!
 *************************************************************************************
 * \brief
 *    Mode Decision for an 4x4 Intra block
 *************************************************************************************
 */
int Mode_Decision_for_4x4IntraBlocks_JM_Low (Macroblock *currMB, int  b8,  int  b4,  double  lambda,  double*  min_cost, int cr_cbp[3], int is_cavlc)
{
  int     ipmode, best_ipmode = 0, i, j, cost, dummy;
  int     nonzero = 0;

  int  block_x     = ((b8 & 0x01) << 3) + ((b4 & 0x01) << 2);
  int  block_y     = ((b8 >> 1) << 3)  + ((b4 >> 1) << 2);
  int  pic_pix_x   = img->pix_x  + block_x;
  int  pic_pix_y   = img->pix_y  + block_y;
  int  pic_opix_x  = img->opix_x + block_x;
  int  pic_opix_y  = img->opix_y + block_y;
  int  pic_block_x = pic_pix_x >> 2;
  int  pic_block_y = pic_pix_y >> 2;

  int left_available, up_available, all_available;

  char   upMode;
  char   leftMode;
  int    mostProbableMode;

  PixelPos left_block;
  PixelPos top_block;

  int  fixedcost = (int) floor(4 * lambda );

#ifdef BEST_NZ_COEFF
  int best_nz_coeff = 0;
  int best_coded_block_flag = 0;
  int bit_pos = 1 + ((((b8>>1)<<1)+(b4>>1))<<2) + (((b8&1)<<1)+(b4&1));
  static int64 cbp_bits;

  if (b8==0 && b4==0)
    cbp_bits = 0;
#endif

  get4x4Neighbour(currMB, block_x - 1, block_y    , img->mb_size[IS_LUMA], &left_block);
  get4x4Neighbour(currMB, block_x,     block_y - 1, img->mb_size[IS_LUMA], &top_block );

  // constrained intra pred
  if (params->UseConstrainedIntraPred)
  {
    left_block.available = left_block.available ? img->intra_block[left_block.mb_addr] : 0;
    top_block.available  = top_block.available  ? img->intra_block[top_block.mb_addr]  : 0;
  }

  upMode            =  top_block.available ? img->ipredmode[top_block.pos_y ][top_block.pos_x ] : -1;
  leftMode          = left_block.available ? img->ipredmode[left_block.pos_y][left_block.pos_x] : -1;

  mostProbableMode  = (upMode < 0 || leftMode < 0) ? DC_PRED : upMode < leftMode ? upMode : leftMode;

  *min_cost = INT_MAX;

  ipmode_DPCM = NO_INTRA_PMODE; ////For residual DPCM

  //===== INTRA PREDICTION FOR 4x4 BLOCK =====
  intrapred_4x4 (currMB, PLANE_Y, pic_pix_x, pic_pix_y, &left_available, &up_available, &all_available);

  if (img->P444_joined)
  {
    select_plane(PLANE_U);
    intrapred_4x4 (currMB, PLANE_U, pic_pix_x, pic_pix_y, &left_available, &up_available, &all_available);
    select_plane(PLANE_V);
    intrapred_4x4 (currMB, PLANE_V, pic_pix_x, pic_pix_y, &left_available, &up_available, &all_available);
    select_plane(PLANE_Y);
  }

  //===== LOOP OVER ALL 4x4 INTRA PREDICTION MODES =====
  for (ipmode = 0; ipmode < NO_INTRA_PMODE; ipmode++)
  {
    int available_mode =  (all_available) || (ipmode==DC_PRED) ||
      (up_available && (ipmode==VERT_PRED||ipmode==VERT_LEFT_PRED||ipmode==DIAG_DOWN_LEFT_PRED)) ||
      (left_available && (ipmode==HOR_PRED||ipmode==HOR_UP_PRED));

    if (valid_intra_mode(ipmode) == 0)
      continue;

    if( available_mode)
    {
      cost  = (ipmode == mostProbableMode) ? 0 : fixedcost;

      compute_comp_cost(&pCurImg[pic_opix_y], img->mpr_4x4[0][ipmode], pic_opix_x, &cost);

      if (img->P444_joined)
      {
        compute_comp_cost(&pImgOrg[1][pic_opix_y], img->mpr_4x4[1][ipmode], pic_opix_x, &cost);
        compute_comp_cost(&pImgOrg[2][pic_opix_y], img->mpr_4x4[2][ipmode], pic_opix_x, &cost);
      }
      
      if (cost < *min_cost)
      {
        best_ipmode = ipmode;
        *min_cost   = cost;
      }
    }
  }

#ifdef BEST_NZ_COEFF
  img->nz_coeff [img->current_mb_nr][block_x4][block_y4] = best_nz_coeff;
  cbp_bits &= (~(int64)(1<<bit_pos));
  cbp_bits |= (int64)(best_coded_block_flag<<bit_pos);
#endif
  //===== set intra mode prediction =====
  img->ipredmode[pic_block_y][pic_block_x] = (char) best_ipmode;
  currMB->intra_pred_modes[4*b8+b4] =
    (char) (mostProbableMode == best_ipmode ? -1 : (best_ipmode < mostProbableMode ? best_ipmode : best_ipmode-1));

  // get prediction and prediction error
  generate_pred_error(&pCurImg[pic_opix_y], img->mpr_4x4[0][best_ipmode], &img->mb_pred[0][block_y], &img->mb_ores[0][block_y], pic_opix_x, block_x);

  ipmode_DPCM=best_ipmode;  

  select_dct(img, currMB);
  nonzero = cr_cbp[0] = pDCT_4x4 (currMB, PLANE_Y, block_x, block_y, &dummy, 1, is_cavlc);

  if (img->P444_joined)
  {
    ColorPlane k;
    for (k = PLANE_U; k <= PLANE_V; k++)
    {
      select_plane(k);
      for (j=0; j<4; j++)
      {
        for (i=0; i<4; i++)
        {
          img->mb_pred[k][block_y+j][block_x+i] = img->mpr_4x4[k][best_ipmode][j][i];    
          img->mb_ores[k][block_y+j][block_x+i] = pImgOrg[k][pic_opix_y+j][pic_opix_x+i] - img->mpr_4x4[k][best_ipmode][j][i]; 
        }
      }

      cr_cbp[k] = pDCT_4x4(currMB, k, block_x,block_y,&dummy,1, is_cavlc);
    }
    select_plane(PLANE_Y);
  }

  return nonzero;
}

/*!
 *************************************************************************************
 * \brief
 *    Mode Decision for an 8x8 Intra block
 *************************************************************************************
 */
int Mode_Decision_for_8x8IntraBlocks(Macroblock *currMB, int b8,double lambda,double *cost, int non_zero[3], int is_cavlc)
{
  int  b4;
  double  cost4x4;
  int CbCr_cbp[3]={0, 0, 0};

  memset(non_zero, 0, 3 * sizeof(int));

  *cost = (int)floor(6.0 * lambda + 0.4999);

  if (params->rdopt == 0)
    Mode_Decision_for_4x4IntraBlocks = Mode_Decision_for_4x4IntraBlocks_JM_Low;
  else
    Mode_Decision_for_4x4IntraBlocks = Mode_Decision_for_4x4IntraBlocks_JM_High;   

  for (b4=0; b4<4; b4++)
  {
    non_zero[0] |= Mode_Decision_for_4x4IntraBlocks (currMB, b8, b4, lambda, &cost4x4, CbCr_cbp, is_cavlc);
    non_zero[1] |= CbCr_cbp[1];
    non_zero[2] |= CbCr_cbp[2];
    *cost += cost4x4;
  }
#ifdef RESET_STATE
  //reset_coding_state (currMB, cs_cm);
#endif

  return non_zero[0];
}

/*!
 *************************************************************************************
 * \brief
 *    4x4 Intra mode decision for an macroblock
 *************************************************************************************
 */
int Mode_Decision_for_Intra4x4Macroblock (Macroblock *currMB, double lambda,  double* cost, int is_cavlc)
{
  int  cbp=0, b8;
  double cost8x8;
  int non_zero[3] = {0, 0, 0};

  cmp_cbp[1] = cmp_cbp[2] = 0;
  
  for (*cost=0, b8=0; b8<4; b8++)
  {
    if (Mode_Decision_for_8x8IntraBlocks (currMB, b8, lambda, &cost8x8, non_zero, is_cavlc))
    {
      cbp |= (1<<b8);
    }
    *cost += cost8x8;
    if (non_zero[1])
    {
      cmp_cbp[1] |= (1<<b8);
      cbp |= cmp_cbp[1];
      cmp_cbp[1] = cbp;
      cmp_cbp[2] = cbp;
    }
    if (non_zero[2])
    {
      cmp_cbp[2] |= (1<<b8);
      cbp |= cmp_cbp[2];
      cmp_cbp[1] = cbp;
      cmp_cbp[2] = cbp;
    }
  }
  return cbp;
}


/*!
*************************************************************************************
* \brief
*    Intra 16x16 mode decision
*************************************************************************************
*/
void Intra16x16_Mode_Decision (Macroblock* currMB, int* i16mode, int is_cavlc)
{
  /* generate intra prediction samples for all 4 16x16 modes */
  intrapred_16x16 (currMB, PLANE_Y);
  
  if (img->P444_joined)
  {
    select_plane(PLANE_U);
    intrapred_16x16 (currMB, PLANE_U);
    select_plane(PLANE_V);
    intrapred_16x16 (currMB, PLANE_V);
    select_plane(PLANE_Y);
  }
  find_sad_16x16 = find_sad_16x16_JM;

  find_sad_16x16 (currMB, i16mode);   /* get best new intra mode */

  currMB->cbp = pDCT_16x16 (currMB, PLANE_Y, *i16mode, is_cavlc);
  if (img->P444_joined)
  {
    select_plane(PLANE_U);
    cmp_cbp[1] = pDCT_16x16 (currMB, PLANE_U, *i16mode, is_cavlc);
    select_plane(PLANE_V);
    cmp_cbp[2] = pDCT_16x16 (currMB, PLANE_V, *i16mode, is_cavlc);
    select_plane(PLANE_Y);
    currMB->cbp |= cmp_cbp[1];
    currMB->cbp |= cmp_cbp[2];
    cmp_cbp[1] = currMB->cbp;
    cmp_cbp[2] = currMB->cbp;
  }
}

