/*!
 *************************************************************************************
 * \file rdoq.c
 *
 * \brief
 *    Rate Distortion Optimized Quantization based on VCEG-AH21
 *************************************************************************************
 */

#include "contributors.h"

#include <math.h>
#include <float.h>

#include "global.h"
#include "image.h"
#include "fmo.h"
#include "macroblock.h"
#include "mb_access.h"
#include "ratectl.h"
#include "rdoq.h"
#include "mv-search.h"

#define RDOQ_BASE 0

const int estErr4x4[6][4][4] =
{
  {
    {25600, 27040, 25600, 27040}, 
    {27040, 25600, 27040, 25600}, 
    {25600, 27040, 25600, 27040}, 
    {27040, 25600, 27040, 25600} 
  },
  {
    {30976, 31360, 30976, 31360}, 
    {31360, 32400, 31360, 32400}, 
    {30976, 31360, 30976, 31360}, 
    {31360, 32400, 31360, 32400} 
  },
  {
    {43264, 40960, 43264, 40960}, 
    {40960, 40000, 40960, 40000}, 
    {43264, 40960, 43264, 40960}, 
    {40960, 40000, 40960, 40000} 
  },
  {
    {50176, 51840, 50176, 51840}, 
    {51840, 52900, 51840, 52900}, 
    {50176, 51840, 50176, 51840}, 
    {51840, 52900, 51840, 52900} 
  },
  {
    {65536, 64000, 65536, 64000}, 
    {64000, 62500, 64000, 62500}, 
    {65536, 64000, 65536, 64000}, 
    {64000, 62500, 64000, 62500} 
  },
  {
    {82944, 84640, 82944, 84640}, 
    {84640, 84100, 84640, 84100}, 
    {82944, 84640, 82944, 84640}, 
    {84640, 84100, 84640, 84100} 
  }
};

const int estErr8x8[6][8][8]={
  {
    {6553600, 6677056, 6400000, 6677056, 6553600, 6677056, 6400000, 6677056}, 
    {6677056, 6765201, 6658560, 6765201, 6677056, 6765201, 6658560, 6765201}, 
    {6400000, 6658560, 6553600, 6658560, 6400000, 6658560, 6553600, 6658560}, 
    {6677056, 6765201, 6658560, 6765201, 6677056, 6765201, 6658560, 6765201}, 
    {6553600, 6677056, 6400000, 6677056, 6553600, 6677056, 6400000, 6677056}, 
    {6677056, 6765201, 6658560, 6765201, 6677056, 6765201, 6658560, 6765201}, 
    {6400000, 6658560, 6553600, 6658560, 6400000, 6658560, 6553600, 6658560}, 
    {6677056, 6765201, 6658560, 6765201, 6677056, 6765201, 6658560, 6765201} 
  },
  {
    {7929856, 8156736, 8028160, 8156736, 7929856, 8156736, 8028160, 8156736}, 
    {8156736, 7537770, 7814560, 7537770, 8156736, 7537770, 7814560, 7537770}, 
    {8028160, 7814560, 7840000, 7814560, 8028160, 7814560, 7840000, 7814560}, 
    {8156736, 7537770, 7814560, 7537770, 8156736, 7537770, 7814560, 7537770}, 
    {7929856, 8156736, 8028160, 8156736, 7929856, 8156736, 8028160, 8156736}, 
    {8156736, 7537770, 7814560, 7537770, 8156736, 7537770, 7814560, 7537770}, 
    {8028160, 7814560, 7840000, 7814560, 8028160, 7814560, 7840000, 7814560}, 
    {8156736, 7537770, 7814560, 7537770, 8156736, 7537770, 7814560, 7537770} 
  },
  {
    {11075584, 10653696, 11151360, 10653696, 11075584, 10653696, 11151360, 10653696}, 
    {10653696, 11045652, 11109160, 11045652, 10653696, 11045652, 11109160, 11045652}, 
    {11151360, 11109160, 11289600, 11109160, 11151360, 11109160, 11289600, 11109160}, 
    {10653696, 11045652, 11109160, 11045652, 10653696, 11045652, 11109160, 11045652}, 
    {11075584, 10653696, 11151360, 10653696, 11075584, 10653696, 11151360, 10653696}, 
    {10653696, 11045652, 11109160, 11045652, 10653696, 11045652, 11109160, 11045652}, 
    {11151360, 11109160, 11289600, 11109160, 11151360, 11109160, 11289600, 11109160}, 
    {10653696, 11045652, 11109160, 11045652, 10653696, 11045652, 11109160, 11045652} 
  },
  {
    {12845056, 12503296, 12544000, 12503296, 12845056, 12503296, 12544000, 12503296}, 
    {12503296, 13050156, 12588840, 13050156, 12503296, 13050156, 12588840, 13050156}, 
    {12544000, 12588840, 12960000, 12588840, 12544000, 12588840, 12960000, 12588840}, 
    {12503296, 13050156, 12588840, 13050156, 12503296, 13050156, 12588840, 13050156}, 
    {12845056, 12503296, 12544000, 12503296, 12845056, 12503296, 12544000, 12503296}, 
    {12503296, 13050156, 12588840, 13050156, 12503296, 13050156, 12588840, 13050156}, 
    {12544000, 12588840, 12960000, 12588840, 12544000, 12588840, 12960000, 12588840}, 
    {12503296, 13050156, 12588840, 13050156, 12503296, 13050156, 12588840, 13050156} 
  },
  {
    {16777216, 16646400, 16384000, 16646400, 16777216, 16646400, 16384000, 16646400}, 
    {16646400, 16370116, 16692640, 16370116, 16646400, 16370116, 16692640, 16370116}, 
    {16384000, 16692640, 16646400, 16692640, 16384000, 16692640, 16646400, 16692640}, 
    {16646400, 16370116, 16692640, 16370116, 16646400, 16370116, 16692640, 16370116}, 
    {16777216, 16646400, 16384000, 16646400, 16777216, 16646400, 16384000, 16646400}, 
    {16646400, 16370116, 16692640, 16370116, 16646400, 16370116, 16692640, 16370116}, 
    {16384000, 16692640, 16646400, 16692640, 16384000, 16692640, 16646400, 16692640}, 
    {16646400, 16370116, 16692640, 16370116, 16646400, 16370116, 16692640, 16370116} 
  },
  {
    {21233664, 21381376, 21667840, 21381376, 21233664, 21381376, 21667840, 21381376}, 
    {21381376, 21381376, 21374440, 21381376, 21381376, 21381376, 21374440, 21381376}, 
    {21667840, 21374440, 21529600, 21374440, 21667840, 21374440, 21529600, 21374440}, 
    {21381376, 21381376, 21374440, 21381376, 21381376, 21381376, 21374440, 21381376}, 
    {21233664, 21381376, 21667840, 21381376, 21233664, 21381376, 21667840, 21381376}, 
    {21381376, 21381376, 21374440, 21381376, 21381376, 21381376, 21374440, 21381376}, 
    {21667840, 21374440, 21529600, 21374440, 21667840, 21374440, 21529600, 21374440}, 
    {21381376, 21381376, 21374440, 21381376, 21381376, 21381376, 21374440, 21381376} 
  }
};

extern int *mvbits;

double norm_factor_4x4;
double norm_factor_8x8;

extern void SetMotionVectorPredictor (Macroblock *currMB, short  pmv[2], char   **refPic,
                                      short  ***tmp_mv, short  ref_frame,
                                      int    list,      int mb_x, int mb_y, 
                                      int    blockshape_x, int blockshape_y);

/*!
****************************************************************************
* \brief
*    Initialize the parameters related to RDO_Q in slice level
****************************************************************************
*/
void init_rdoq_slice(int slice_type, int symbol_mode)
{
  //norm_factor_4x4 = (double) ((int64) 1 << (2 * DQ_BITS + 19)); // norm factor 4x4 is basically (1<<31)
  //norm_factor_8x8 = (double) ((int64) 1 << (2 * Q_BITS_8 + 9)); // norm factor 8x8 is basically (1<<41)
  norm_factor_4x4 = pow(2, (2 * DQ_BITS + 19));
  norm_factor_8x8 = pow(2, (2 * Q_BITS_8 + 9));
}
/*!
****************************************************************************
* \brief
*    Initialize levelData for Chroma DC
****************************************************************************
*/
int init_trellis_data_DC_cr(int (*tblock)[4], int qp_per, int qp_rem, 
                         int levelscale, int **leveloffset, const byte *p_scan, Macroblock *currMB,  
                         levelDataStruct *dataLevel, int* kStart, int* kStop, int type)
{
  int noCoeff = 0;
  int i, j, coeff_ctr, end_coeff_ctr = ( (type == CHROMA_DC) ? 4 : 8 );
  static int *m7;
  int q_bits = Q_BITS + qp_per + 1; 
  int q_offset = ( 1 << (q_bits - 1) );
  double err; 
  int level, lowerInt, k;
  double estErr = (double) estErr4x4[qp_rem][0][0] / norm_factor_4x4; // note that we could also use int64

  for (coeff_ctr = 0; coeff_ctr < end_coeff_ctr; coeff_ctr++)
  {
    j = *p_scan++;  // horizontal position
    i = *p_scan++;  // vertical position

    m7 = &tblock[j][i];
    if (*m7 == 0)
    {
      dataLevel->levelDouble = 0;
      dataLevel->level[0] = 0;
      dataLevel->noLevels = 1;
      err = 0.0;
      dataLevel->errLevel[0] = 0.0;
    }
    else
    {
      dataLevel->levelDouble = iabs(*m7 * levelscale);
      level = (dataLevel->levelDouble >> q_bits);

      lowerInt=( ((int)dataLevel->levelDouble - (level << q_bits)) < q_offset )? 1 : 0;
      dataLevel->level[0] = 0;
      if (level == 0 && lowerInt == 1)
      {
        dataLevel->noLevels = 1;
      }
      else if (level == 0 && lowerInt == 0)
      {
        dataLevel->level[1] = level + 1;
        dataLevel->noLevels = 2;
        *kStop = coeff_ctr;
        noCoeff++;
      }
      else if (level > 0 && lowerInt == 1)
      {
        dataLevel->level[1] = level;
        dataLevel->noLevels = 2;
        *kStop = coeff_ctr;
        noCoeff++;
      }
      else
      {
        dataLevel->level[1] = level;
        dataLevel->level[2] = level + 1;
        dataLevel->noLevels = 3;
        *kStop  = coeff_ctr;
        *kStart = coeff_ctr;
        noCoeff++;
      }

      for (k = 0; k < dataLevel->noLevels; k++)
      {
        err = (double)(dataLevel->level[k] << q_bits) - (double)dataLevel->levelDouble;
        dataLevel->errLevel[k] = (err * err * estErr); 
      }
    }
    dataLevel++;
  }
  return (noCoeff);
}

void trellis_mp(Macroblock *currMB, int CurrentMbAddr, Boolean prev_recode_mb)
{
  int masterQP = 0, deltaQP;
  int qp_left, qp_up;
#if RDOQ_BASE
  const int deltaQPTabB[] = {0,  1, -1,  2, 3, -2, 4,  5, -3};
  const int deltaQPTabP[] = {0, -1,  1, -2, 2, -3, 3, -4,  4};
#endif
  int   deltaQPCnt; 
  int   qp_anchor; 
  int   prev_mb = FmoGetPreviousMBNr(img->current_mb_nr);
  int   qp_offset = (img->type == B_SLICE) ? (params->RDOQ_QP_Num / 3): (params->RDOQ_QP_Num >> 1);

  masterQP = img->masterQP = img->qp;
  Motion_Selected = 0;
  rddata_trellis_best.min_rdcost = 1e30;

  if (params->symbol_mode == CABAC)
  {
    estRunLevel_CABAC(currMB, LUMA_4x4); 
    estRunLevel_CABAC(currMB, LUMA_16AC);
    estRunLevel_CABAC(currMB, LUMA_16DC);
    if (params->Transform8x8Mode)
      estRunLevel_CABAC(currMB, LUMA_8x8);
    if (params->yuv_format != YUV400)
    {
      estRunLevel_CABAC(currMB, CHROMA_AC);
      if (params->yuv_format == YUV420)
        estRunLevel_CABAC(currMB, CHROMA_DC);
      else
        estRunLevel_CABAC(currMB, CHROMA_DC_2x4);
    }
  }

  qp_left   = (currMB->mb_available_left) ? currMB->mb_available_left->qp : img->masterQP;
  qp_up     = (currMB->mb_available_up)   ? currMB->mb_available_up->qp   : img->masterQP;
  qp_anchor = (qp_left + qp_up + 1)>>1;

  for (deltaQPCnt=0; deltaQPCnt < params->RDOQ_QP_Num; deltaQPCnt++)
  {
    rdopt = &rddata_trellis_curr;
#if RDOQ_BASE
    if (img->type == B_SLICE)
      deltaQP = deltaQPTabB[deltaQPCnt];      
    else
      deltaQP = deltaQPTabP[deltaQPCnt];
#else

    // It seems that pushing the masterQP as first helps things when fast me is enabled. 
    // Could there be an issue with motion estimation?
    if (deltaQPCnt == 0)
      deltaQP = 0;
    else if (deltaQPCnt <= qp_offset)
      deltaQP = deltaQPCnt - 1 - qp_offset;
    else
      deltaQP = deltaQPCnt - qp_offset;
    //printf("qp %d %d %d\n", deltaQP,  deltaQPCnt, masterQP);
#endif

    img->qp = iClip3(-img->bitdepth_luma_qp_scale, 51, masterQP + deltaQP);
    deltaQP = masterQP - img->qp;    

#if 0
    if(deltaQP != 0 && !(img->qp - qp_anchor >= -2 && img->qp - qp_anchor <= 1) && currMB->mb_available_left && currMB->mb_available_up && img->type == P_SLICE)
      continue; 
    if(deltaQP != 0 && !(img->qp - qp_anchor >= -1 && img->qp - qp_anchor <= 2) && currMB->mb_available_left && currMB->mb_available_up && img->type == B_SLICE)
      continue;
#endif

    reset_macroblock(currMB, prev_mb);
    currMB->qp       = img->qp;
    currMB->delta_qp = currMB->qp - currMB->prev_qp;
    update_qp (img, currMB);

    encode_one_macroblock (currMB);

    if ( rddata_trellis_curr.min_rdcost < rddata_trellis_best.min_rdcost)
      copy_rddata_trellis(&rddata_trellis_best, rdopt);

    if (params->RDOQ_CP_MV)
      Motion_Selected = 1;

#if (!RDOQ_BASE)
    if (params->RDOQ_Fast)
    {
      if ((img->qp - rddata_trellis_best.qp > 1))
        break;
      if ((rddata_trellis_curr.cbp == 0) && (rddata_trellis_curr.mb_type != 0))
        break;
      if ((rddata_trellis_best.mb_type == 0) && (rddata_trellis_best.cbp == 0))
        break;
    }
#endif
  }

  reset_macroblock(currMB, prev_mb);
  rdopt = &rddata_trellis_best;

  copy_rdopt_data (currMB, FALSE);  // copy the MB data for Top MB from the temp buffers
  write_one_macroblock (currMB, 1, prev_recode_mb);
  img->qp = masterQP;
}

void trellis_sp(Macroblock *currMB, int CurrentMbAddr, Boolean prev_recode_mb)
{
  img->masterQP = img->qp;

  if (params->symbol_mode == CABAC)
  {
    estRunLevel_CABAC(currMB, LUMA_4x4); 
    estRunLevel_CABAC(currMB, LUMA_16AC);
    
    estRunLevel_CABAC(currMB, LUMA_16DC);
    if (params->Transform8x8Mode)
      estRunLevel_CABAC(currMB, LUMA_8x8);

    if (params->yuv_format != YUV400)
    {
      estRunLevel_CABAC(currMB, CHROMA_AC);

      if (params->yuv_format == YUV420)
        estRunLevel_CABAC(currMB, CHROMA_DC);
      else
        estRunLevel_CABAC(currMB, CHROMA_DC_2x4);
    }
  }

  encode_one_macroblock (currMB);
  write_one_macroblock (currMB, 1, prev_recode_mb);    
}

void trellis_coding(Macroblock *currMB, int CurrentMbAddr, Boolean prev_recode_mb)
{
  if (params->RDOQ_QP_Num > 1)
  {
    trellis_mp(currMB, CurrentMbAddr, prev_recode_mb);   
  }
  else
  {
    trellis_sp(currMB, CurrentMbAddr, prev_recode_mb);   
  }
}


void RDOQ_update_mode(RD_PARAMS *enc_mb, int bslice)
{
  int i;
  for(i=0; i<MAXMODE; i++)
    enc_mb->valid[i] = 0;

  enc_mb->valid[rdopt->mb_type] = 1;

  if(rdopt->mb_type  == P8x8)
  {            
    enc_mb->valid[4] = (params->InterSearch[bslice][4]);
    enc_mb->valid[5] = (params->InterSearch[bslice][5] && !(params->Transform8x8Mode==2));
    enc_mb->valid[6] = (params->InterSearch[bslice][6] && !(params->Transform8x8Mode==2));
    enc_mb->valid[7] = (params->InterSearch[bslice][7] && !(params->Transform8x8Mode==2));
  }
}

void copy_rddata_trellis (RD_DATA *dest, RD_DATA *src)
{
  int j; 

  dest->min_rdcost = src->min_rdcost;
  dest->min_dcost  = src->min_dcost;

  memcpy(&dest->rec_mbY[0][0],&src->rec_mbY[0][0], MB_PIXELS * sizeof(imgpel));

  if (img->yuv_format != YUV400) 
  {
    // we could allocate these dynamically to improve performance.
    memcpy(&dest->rec_mb_cr[0][0][0],&src->rec_mb_cr[0][0][0], 2 * MB_PIXELS * sizeof(imgpel));
  }

  memcpy(&dest->cofAC[0][0][0][0], &src->cofAC[0][0][0][0], (4 + img->num_blk8x8_uv) * 4 * 2 * 65 * sizeof(int));
  memcpy(&dest->cofDC[0][0][0], &src->cofDC[0][0][0], 3 * 2 * 18 * sizeof(int));

  dest->mb_type = src->mb_type;
  memcpy(dest->b8mode, src->b8mode, BLOCK_MULTIPLE * sizeof(short));
  memcpy(dest->b8pdir, src->b8pdir, BLOCK_MULTIPLE * sizeof(short));
  dest->cbp  = src->cbp;
  dest->mode = src->mode;
  dest->i16offset = src->i16offset;
  dest->c_ipred_mode = src->c_ipred_mode;
  dest->luma_transform_size_8x8_flag = src->luma_transform_size_8x8_flag;
  dest->NoMbPartLessThan8x8Flag = src->NoMbPartLessThan8x8Flag;
  dest->qp = src->qp;

  dest->prev_qp  = src->prev_qp;
  dest->prev_dqp = src->prev_dqp;
  dest->delta_qp = src->delta_qp;
  dest->prev_cbp = src->prev_cbp;
  dest->cbp_blk  = src->cbp_blk;
 

  if (img->type != I_SLICE)
  {
    // note that this is not copying the bipred mvs!!!
    memcpy(&dest->all_mv [0][0][0][0][0][0], &src->all_mv [0][0][0][0][0][0], 2 * img->max_num_references * 9 * 4 * 4 * 2 * sizeof(short));
    memcpy(&dest->pred_mv[0][0][0][0][0][0], &src->pred_mv[0][0][0][0][0][0], 2 * img->max_num_references * 9 * 4 * 4 * 2 * sizeof(short));
  }

  memcpy(dest->intra_pred_modes,src->intra_pred_modes, MB_BLOCK_PARTITIONS * sizeof(char));
  memcpy(dest->intra_pred_modes8x8,src->intra_pred_modes8x8, MB_BLOCK_PARTITIONS * sizeof(char));
  for(j = img->block_y; j < img->block_y + BLOCK_MULTIPLE; j++)
    memcpy(&dest->ipredmode[j][img->block_x],&src->ipredmode[j][img->block_x], BLOCK_MULTIPLE * sizeof(char));
  memcpy(&dest->refar[LIST_0][0][0], &src->refar[LIST_0][0][0], 2 * BLOCK_MULTIPLE * BLOCK_MULTIPLE * sizeof(char));
}                            

void updateMV_mp(int *m_cost, short ref, int list, int h, int v, int blocktype, int *lambda_factor, int block8x8)
{
  int       i, j;
  int       bsx       = params->blc_size[blocktype][0];
  int       bsy       = params->blc_size[blocktype][1];
  short     tmp_pred_mv[2];
  short*    pred_mv = img->pred_mv[list][ref][blocktype][v][h];
  short     all_mv[2];
  Macroblock *currMB = &img->mb_data[img->current_mb_nr];
  if ( (params->Transform8x8Mode == 1) && (blocktype == 4) && currMB->luma_transform_size_8x8_flag)
  {
    all_mv[0] = tmp_mv8[list][ref][v][h][0];
    all_mv[1] = tmp_mv8[list][ref][v][h][1];
    tmp_pred_mv[0] = tmp_pmv8[list][ref][v][h][0];
    tmp_pred_mv[1] = tmp_pmv8[list][ref][v][h][1];
    *m_cost   = motion_cost8[list][ref][block8x8];
  }
  else
  {
    all_mv[0] = rddata_trellis_best.all_mv[list][ref][blocktype][v][h][0];
    all_mv[1] = rddata_trellis_best.all_mv[list][ref][blocktype][v][h][1];
    tmp_pred_mv[0] = rddata_trellis_best.pred_mv[list][ref][blocktype][v][h][0];
    tmp_pred_mv[1] = rddata_trellis_best.pred_mv[list][ref][blocktype][v][h][1];
  }

  for (j = 0; j < (bsy>>2); j++)
  {
    for (i = 0; i < (bsx>>2); i++) 
      memcpy(img->all_mv[list][ref][blocktype][v+j][h+i], all_mv, 2 * sizeof(short));
  }

  SetMotionVectorPredictor (currMB, pred_mv, enc_picture->motion.ref_idx[list], enc_picture->motion.mv[list], ref, list, h<<2, v<<2, bsx, bsy);

  if ( (tmp_pred_mv[0] != pred_mv[0]) || (tmp_pred_mv[1] != pred_mv[1]) )
  {
    *m_cost -= MV_COST_SMP (lambda_factor[H_PEL], all_mv[0], all_mv[1], tmp_pred_mv[0], tmp_pred_mv[1]);
    *m_cost += MV_COST_SMP (lambda_factor[H_PEL], all_mv[0], all_mv[1], pred_mv[0], pred_mv[1]);
  }
}



