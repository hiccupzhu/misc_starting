
/*!
 ***************************************************************************
 * \file mode_decision.c
 *
 * \brief
 *    Main macroblock mode decision functions and helpers
 *
 **************************************************************************
 */

#include <math.h>
#include <limits.h>
#include <float.h>

#include "global.h"
#include "rdopt_coding_state.h"
#include "mb_access.h"
#include "intrarefresh.h"
#include "image.h"
#include "transform8x8.h"
#include "ratectl.h"
#include "mode_decision.h"
#include "fmo.h"
#include "me_umhex.h"
#include "me_umhexsmp.h"
#include "macroblock.h"
#include "rdoq.h"
#include "errdo.h"


//==== MODULE PARAMETERS ====
const int  b8_mode_table[6]  = {0, 4, 5, 6, 7};         // DO NOT CHANGE ORDER !!!
const int  mb_mode_table[9]  = {0, 1, 2, 3, P8x8, I16MB, I4MB, I8MB, IPCM}; // DO NOT CHANGE ORDER !!!

//double *mb16x16_cost_frame;

/*!
*************************************************************************************
* \brief
*    Reset Valid Modes
*************************************************************************************
*/
void reset_valid_modes(RD_PARAMS *enc_mb)
{
  memset(enc_mb->valid, 0, MAXMODE * sizeof(short));
}


/*!
*************************************************************************************
* \brief
*    Initialize Encoding parameters for Macroblock
*************************************************************************************
*/
void init_enc_mb_params(Macroblock* currMB, RD_PARAMS *enc_mb, int intra, int bslice)
{
  int l,k;

  //Setup list offset
  enc_mb->list_offset[LIST_0] = LIST_0 + currMB->list_offset;
  enc_mb->list_offset[LIST_1] = LIST_1 + currMB->list_offset;

  enc_mb->curr_mb_field = ((img->MbaffFrameFlag)&&(currMB->mb_field));
 
  // Set valid modes  
  enc_mb->valid[I8MB]  = (!params->DisableIntraInInter || intra )?   params->Transform8x8Mode : 0;
  enc_mb->valid[I4MB]  = (!params->DisableIntraInInter || intra )? ((params->Transform8x8Mode == 2) ? 0 : 1) : 0;
  enc_mb->valid[I4MB]  = (!params->DisableIntra4x4  ) ? enc_mb->valid[I4MB] : 0;
  enc_mb->valid[I16MB] = (!params->DisableIntraInInter || intra )? 1 : 0;
  enc_mb->valid[I16MB] = (!params->DisableIntra16x16) ? enc_mb->valid[I16MB] : 0;
  enc_mb->valid[IPCM]  = (!params->DisableIntraInInter || intra )? params->EnableIPCM : 0;

  enc_mb->valid[0]     = (!intra && params->InterSearch[bslice][0]);
  enc_mb->valid[1]     = (!intra && params->InterSearch[bslice][1]);
  enc_mb->valid[2]     = (!intra && params->InterSearch[bslice][2]);
  enc_mb->valid[3]     = (!intra && params->InterSearch[bslice][3]);
  enc_mb->valid[4]     = (!intra && params->InterSearch[bslice][4]);
  enc_mb->valid[5]     = (!intra && params->InterSearch[bslice][5] && !(params->Transform8x8Mode==2));
  enc_mb->valid[6]     = (!intra && params->InterSearch[bslice][6] && !(params->Transform8x8Mode==2));
  enc_mb->valid[7]     = (!intra && params->InterSearch[bslice][7] && !(params->Transform8x8Mode==2));
  enc_mb->valid[P8x8]  = (enc_mb->valid[4] || enc_mb->valid[5] || enc_mb->valid[6] || enc_mb->valid[7]);
  enc_mb->valid[12]    = (img->type == SI_SLICE);
  
  if (params->UseRDOQuant && params->RDOQ_CP_Mode && (img->qp != img->masterQP) )  
    RDOQ_update_mode(enc_mb, bslice);

  if(img->type==SP_SLICE)
  {
    if(si_frame_indicator)
    {
      reset_valid_modes(enc_mb);
      if(check_for_SI16())
      {
        enc_mb->valid[I16MB] = 1;
      }
      else
      {
        enc_mb->valid[I4MB]  = 1;
      }
    }
    
    if(sp2_frame_indicator)
    {
      if(check_for_SI16())
      {
        reset_valid_modes(enc_mb);
        enc_mb->valid[I16MB] = 1;
      }
      else
      {
        enc_mb->valid[I8MB]  = 0;
        enc_mb->valid[IPCM]  = 0;
        enc_mb->valid[0]     = 0;
        enc_mb->valid[I16MB] = 0;
      }
    }
  }

  //===== SET LAGRANGE PARAMETERS =====
  // Note that these are now computed at the slice level to reduce
  // computations and cleanup code.
  if (bslice && img->nal_reference_idc)
  {
    enc_mb->lambda_md = img->lambda_md[5][img->masterQP];

    enc_mb->lambda_me[F_PEL] = img->lambda_me[5][img->masterQP][F_PEL];
    enc_mb->lambda_me[H_PEL] = img->lambda_me[5][img->masterQP][H_PEL];
    enc_mb->lambda_me[Q_PEL] = img->lambda_me[5][img->masterQP][Q_PEL];

    enc_mb->lambda_mf[F_PEL] = img->lambda_mf[5][img->masterQP][F_PEL];
    enc_mb->lambda_mf[H_PEL] = img->lambda_mf[5][img->masterQP][H_PEL];
    enc_mb->lambda_mf[Q_PEL] = img->lambda_mf[5][img->masterQP][Q_PEL];
  }
  else
  {
    enc_mb->lambda_md = img->lambda_md[img->type][img->masterQP];
    enc_mb->lambda_me[F_PEL] = img->lambda_me[img->type][img->masterQP][F_PEL];
    enc_mb->lambda_me[H_PEL] = img->lambda_me[img->type][img->masterQP][H_PEL];
    enc_mb->lambda_me[Q_PEL] = img->lambda_me[img->type][img->masterQP][Q_PEL];

    enc_mb->lambda_mf[F_PEL] = img->lambda_mf[img->type][img->masterQP][F_PEL];
    enc_mb->lambda_mf[H_PEL] = img->lambda_mf[img->type][img->masterQP][H_PEL];
    enc_mb->lambda_mf[Q_PEL] = img->lambda_mf[img->type][img->masterQP][Q_PEL];
  }
 
  if (!img->MbaffFrameFlag)
  {
    for (l = LIST_0; l < BI_PRED; l++)
    {
      for(k = 0; k < listXsize[l]; k++)
      {
        if(img->structure != listX[l][k]->structure)
        {
          if (img->structure == TOP_FIELD)
            listX[l][k]->chroma_vector_adjustment = -2;
          else if (img->structure == BOTTOM_FIELD)
            listX[l][k]->chroma_vector_adjustment = 2;
          else
            listX[l][k]->chroma_vector_adjustment= 0;
        }
        else
          listX[l][k]->chroma_vector_adjustment= 0;
      }
    }
  }
  else
  {
    if (enc_mb->curr_mb_field)
    {
      for (l = enc_mb->list_offset[LIST_0]; l <= enc_mb->list_offset[LIST_1]; l++)
      {
        for(k = 0; k < listXsize[l]; k++)
        {
          listX[l][k]->chroma_vector_adjustment= 0;
          if((img->current_mb_nr & 0x01) == 0 && listX[l][k]->structure == BOTTOM_FIELD)
            listX[l][k]->chroma_vector_adjustment = -2;
          if((img->current_mb_nr & 0x01) == 1 && listX[l][k]->structure == TOP_FIELD)
            listX[l][k]->chroma_vector_adjustment = 2;
        }
      }
    }
    else
    {
      for (l = enc_mb->list_offset[LIST_0]; l <= enc_mb->list_offset[LIST_1]; l++)
      {
        for(k = 0; k < listXsize[l]; k++)
          listX[l][k]->chroma_vector_adjustment = 0;
      }
    }
  }
}

/*!
*************************************************************************************
* \brief
*    computation of prediction list (including biprediction) cost
*************************************************************************************
*/
void list_prediction_cost(Macroblock *currMB, int list, int block, int mode, RD_PARAMS *enc_mb, int bmcost[5], char best_ref[2])
{
  short ref;
  int mcost;
  int cur_list = list < BI_PRED ? enc_mb->list_offset[list] : enc_mb->list_offset[LIST_0];

  //--- get cost and reference frame for forward prediction ---

  if (list < BI_PRED)
  {
    for (ref=0; ref < listXsize[cur_list]; ref++)
    {
      if (!img->checkref || list || ref==0 || (params->RestrictRef && CheckReliabilityOfRef (block, list, ref, mode)))
      {
        // limit the number of reference frames to 1 when switching SP frames are used
        if((!params->sp2_frame_indicator && !params->sp_output_indicator)||
          ((params->sp2_frame_indicator || params->sp_output_indicator) && (img->type != P_SLICE && img->type != SP_SLICE))||
          ((params->sp2_frame_indicator || params->sp_output_indicator) && ((img->type == P_SLICE || img->type == SP_SLICE) &&(ref==0))))
        {
          mcost  = (params->rdopt
            ? REF_COST (enc_mb->lambda_mf[Q_PEL], ref, cur_list)
            : (int) (2 * enc_mb->lambda_me[Q_PEL] * imin(ref, 1)));

          mcost += motion_cost[mode][list][ref][block];
          if (mcost < bmcost[list])
          {
            bmcost[list]   = mcost;
            best_ref[list] = (char)ref;
          }
        }
      }
    }
  }
  else if (list == BI_PRED)
  {
    if (active_pps->weighted_bipred_idc == 1)
    {
      int weight_sum = wbp_weight[0][(int) best_ref[LIST_0]][(int) best_ref[LIST_1]][0] + wbp_weight[1][(int) best_ref[LIST_0]][(int) best_ref[LIST_1]][0];
      
      if (weight_sum < -128 ||  weight_sum > 127)
      {
        bmcost[list] = INT_MAX;
      }
      else
      {
        bmcost[list]  = (params->rdopt
          ? (REF_COST  (enc_mb->lambda_mf[Q_PEL], (short)best_ref[LIST_0], cur_list)
          +  REF_COST  (enc_mb->lambda_mf[Q_PEL], (short)best_ref[LIST_1], cur_list + LIST_1))
          : (int) (2 * (enc_mb->lambda_me[Q_PEL] * (imin((short)best_ref[LIST_0], 1) + imin((short)best_ref[LIST_1], 1)))));
        bmcost[list] += BIDPartitionCost (currMB, mode, block, best_ref, enc_mb->lambda_mf[Q_PEL]);
      }
    }
    else
    {
      bmcost[list]  = (params->rdopt
        ? (REF_COST  (enc_mb->lambda_mf[Q_PEL], (short)best_ref[LIST_0], cur_list)
        +  REF_COST  (enc_mb->lambda_mf[Q_PEL], (short)best_ref[LIST_1], cur_list + LIST_1))
        : (int) (2 * (enc_mb->lambda_me[Q_PEL] * (imin((short)best_ref[LIST_0], 1) + imin((short)best_ref[LIST_1], 1)))));
      bmcost[list] += BIDPartitionCost (currMB, mode, block, best_ref, enc_mb->lambda_mf[Q_PEL]);
    }
  }
  else
  {
    bmcost[list]  = (params->rdopt
      ? (REF_COST (enc_mb->lambda_mf[Q_PEL], 0, cur_list)
      +  REF_COST (enc_mb->lambda_mf[Q_PEL], 0, cur_list + LIST_1))
      : (int) (4 * enc_mb->lambda_me[Q_PEL]));
    bmcost[list] += BPredPartitionCost(currMB, mode, block, 0, 0, enc_mb->lambda_mf[Q_PEL], !(list&1));
  }
}

int compute_ref_cost(RD_PARAMS *enc_mb, int ref, int list)
{
  return WEIGHTED_COST(enc_mb->lambda_mf[Q_PEL],((listXsize[enc_mb->list_offset[list]] <= 1)? 0:refbits[ref]));
}

/*!
*************************************************************************************
* \brief
*    Determination of prediction list based on simple distortion computation
*************************************************************************************
*/
void determine_prediction_list(int mode, int bmcost[5], char best_ref[2], char *best_pdir, int *cost, short *bipred_me)
{
  int bestlist;  
  
  *cost += iminarray ( bmcost, 5, &bestlist);
  
  if (bestlist <= BI_PRED)  //LIST_0, LIST_1 & BI_DIR
  {
    *best_pdir = bestlist; 
    *bipred_me = 0;
  }
  else                      //BI_PRED_L0 & BI_PRED_L1
  {
    *best_pdir = 2;    
    *bipred_me = bestlist - 2;
    best_ref[LIST_1] = 0;
    best_ref[LIST_0] = 0;
  }
}

/*!
*************************************************************************************
* \brief
*    RD decision process
*************************************************************************************
*/
void compute_mode_RD_cost(int mode,
                          Macroblock *currMB,
                          RD_PARAMS *enc_mb,
                          double *min_rdcost,
                          double *min_dcost,
                          double *min_rate,
                          int i16mode,
                          short bslice,
                          short *inter_skip,
                          int   is_cavlc)
{
  //--- transform size ---
  currMB->luma_transform_size_8x8_flag = params->Transform8x8Mode==2
    ?  (mode >= 1 && mode <= 3)
    || (mode == 0 && bslice && active_sps->direct_8x8_inference_flag)
    || ((mode == P8x8) && (enc_mb->valid[4]))
    :  0;
  //store_coding_state (currMB, cs_cm); // RD
  SetModesAndRefframeForBlocks (currMB, mode);

  // Encode with coefficients
  img->NoResidueDirect = 0;

  if ((params->FastCrIntraDecision )
    || (currMB->c_ipred_mode == DC_PRED_8 || (IS_INTRA(currMB) )))
  {
    while(1)
    {
      if (RDCost_for_macroblocks (currMB, enc_mb->lambda_md, mode, min_rdcost, min_dcost, min_rate, i16mode, is_cavlc))
      {
        //Rate control
        if (params->RCEnable)
        {
          if(mode == P8x8)
            rc_store_diff(img->opix_x, img->opix_y, currMB->luma_transform_size_8x8_flag == 1 ? tr8x8.mpr8x8 : tr4x4.mpr8x8);
          else
            rc_store_diff(img->opix_x, img->opix_y, pred);
        }

        store_macroblock_parameters (currMB, mode);

        if(params->rdopt == 2 && mode == 0 && params->EarlySkipEnable)
        {
          // check transform quantized coeff.
          if(currMB->cbp == 0)
            *inter_skip = 1;
        }
      }

      // Go through transform modes.
      // Note that if currMB->cbp is 0 one could choose to skip 8x8 mode
      // although this could be due to deadzoning decisions.
      //if (params->Transform8x8Mode==1 && currMB->cbp!=0)
      if (params->Transform8x8Mode==1)
      {
        //=========== try mb_types 1,2,3 with 8x8 transform ===========
        if ((mode >= 1 && mode <= 3) && currMB->luma_transform_size_8x8_flag == 0)
        {
          //try with 8x8 transform size
          currMB->luma_transform_size_8x8_flag = 1;
          continue;
        }
        //=========== try DIRECT-MODE with 8x8 transform ===========
        else if (mode == 0 && bslice && active_sps->direct_8x8_inference_flag && currMB->luma_transform_size_8x8_flag == 0)
        {
          //try with 8x8 transform size
          currMB->luma_transform_size_8x8_flag = 1;
          continue;
        }
        //=========== try mb_type P8x8 for mode 4 with 4x4/8x8 transform ===========
        else if ((mode == P8x8) && (enc_mb->valid[4]) && (currMB->luma_transform_size_8x8_flag == 0))
        {
          currMB->luma_transform_size_8x8_flag = 1; //check 8x8 partition for transform size 8x8
          continue;
        }
        else
        {
          currMB->luma_transform_size_8x8_flag = 0;
          break;
        }
      }
      else
        break;
    }

    // Encode with no coefficients. Currently only for direct. This could be extended to all other modes as in example.
    //if (mode < P8x8 && (*inter_skip == 0) && enc_mb->valid[mode] && currMB->cbp && (currMB->cbp&15) != 15 && !params->nobskip)
    if ( bslice && mode == 0 && (*inter_skip == 0) && enc_mb->valid[mode] 
    && currMB->cbp && (currMB->cbp&15) != 15 && !params->nobskip
    && !(currMB->qp_scaled[0] == 0 && img->lossless_qpprime_flag==1) )
    {
      img->NoResidueDirect = 1;

      if (RDCost_for_macroblocks (currMB, enc_mb->lambda_md, mode, min_rdcost, min_dcost, min_rate, i16mode, is_cavlc))
      {
        //Rate control
        if (params->RCEnable)
          rc_store_diff(img->opix_x,img->opix_y,pred);

        store_macroblock_parameters (currMB, mode);
      }
    }
  }
}


/*!
*************************************************************************************
* \brief
*    Mode Decision for an 8x8 sub-macroblock
*************************************************************************************
*/
void submacroblock_mode_decision(RD_PARAMS *enc_mb,
                                 RD_8x8DATA *dataTr,
                                 Macroblock *currMB,
                                 int ***cofACtr,
                                 int ***cofACCbCrtr1, 
                                 int ***cofACCbCrtr2,
                                 int *have_direct,
                                 short bslice,
                                 int block,
                                 int *cost_direct,
                                 int *cost,
                                 int *cost8x8_direct,
                                 int transform8x8,
                                 int is_cavlc)
{
  int64 curr_cbp_blk;
  double min_rdcost, rdcost = 0.0;
  int j0, i0, j1, i1;
  int i,j, k;
  int min_cost8x8, index;
  int mode;
  int direct4x4_tmp, direct8x8_tmp;
  int bmcost[5] = {INT_MAX};
  int cnt_nonz = 0;
  int dummy;
  int best_cnt_nonz = 0;
  int maxindex =  (transform8x8) ? 2 : 5;
  int block_x, block_y;
  int lambda_mf[3];
  static int fadjust[16][16], fadjustCr[2][16][16];
  int ***fadjustTransform = transform8x8? img->fadjust8x8 : img->fadjust4x4;
  int ****fadjustTransformCr   = (params->AdaptRndChroma || (img->P444_joined )) 
    ? (transform8x8 ? img->fadjust8x8Cr : img->fadjust4x4Cr): NULL;
  int lumaAdjustIndex   = transform8x8 ? 2 : 3;
  int chromaAdjustIndex = transform8x8 ? 0 : 2;
  short pdir, bipred_me = 0;


  char best_pdir = 0;
  char best_ref[2] = {0, -1};
  imgpel  (*mb_pred)[16] = img->mb_pred[0];

#ifdef BEST_NZ_COEFF
  int best_nz_coeff[2][2];
#endif

  //--- set coordinates ---
  j0 = ((block>>1)<<3);
  j1 = (j0>>2);
  i0 = ((block&0x01)<<3);
  i1 = (i0>>2);

#ifdef BEST_NZ_COEFF
  for(j = 0; j <= 1; j++)
  {
    for(i = 0; i <= 1; i++)
      best_nz_coeff[i][j] = img->nz_coeff[img->current_mb_nr][i1 + i][j1 + j] = 0;
  }
#endif

  if (transform8x8)
    currMB->luma_transform_size_8x8_flag = 1; //switch to transform size 8x8

  //--- store coding state before coding ---
  store_coding_state (currMB, cs_cm);

  //=====  LOOP OVER POSSIBLE CODING MODES FOR 8x8 SUB-PARTITION  =====
  for (min_cost8x8 = INT_MAX, min_rdcost = 1e20, index = (bslice?0:1); index < maxindex; index++)
  {
    mode = b8_mode_table[index];
    *cost = 0;
    if (enc_mb->valid[mode] && (transform8x8 == 0 || mode != 0 || (mode == 0 && active_sps->direct_8x8_inference_flag)))
    {
      curr_cbp_blk = 0;

      if (mode==0)
      {
        //--- Direct Mode ---
        if (!params->rdopt )
        {
          direct4x4_tmp = 0;
          direct8x8_tmp = 0;
          direct4x4_tmp = GetDirectCost8x8 ( currMB, block, &direct8x8_tmp);

          if ((direct4x4_tmp==INT_MAX)||(*cost_direct==INT_MAX))
          {
            *cost_direct = INT_MAX;
            if (transform8x8)
              *cost8x8_direct = INT_MAX;
          }
          else
          {
            *cost_direct += direct4x4_tmp;
            if (transform8x8)
              *cost8x8_direct += direct8x8_tmp;
          }
          (*have_direct) ++;

          if (transform8x8)
          {
            switch(params->Transform8x8Mode)
            {
            case 1: // Mixture of 8x8 & 4x4 transform
              if((direct8x8_tmp < direct4x4_tmp) || !(enc_mb->valid[5] && enc_mb->valid[6] && enc_mb->valid[7]))
                *cost = direct8x8_tmp;
              else
                *cost = direct4x4_tmp;
              break;
            case 2: // 8x8 Transform only
              *cost = direct8x8_tmp;
              break;
            default: // 4x4 Transform only
              *cost = direct4x4_tmp;
              break;
            }
            if (params->Transform8x8Mode==2)
              *cost = INT_MAX;
          }
          else
          {
            *cost = direct4x4_tmp;
          }
        }

        block_x = img->block_x + (block & 0x01)*2;
        block_y = img->block_y + (block & 0x02);
        best_ref[LIST_0] = direct_ref_idx[LIST_0][block_y][block_x];
        best_ref[LIST_1] = direct_ref_idx[LIST_1][block_y][block_x];
        best_pdir        = direct_pdir[block_y][block_x];
      } // if (mode==0)
      else
      {
        int64 ref_pic_num;
        char b_ref;
        //======= motion estimation for all reference frames ========
        //-----------------------------------------------------------
        memcpy(lambda_mf, enc_mb->lambda_mf, 3 * sizeof(int));
        if (params->CtxAdptLagrangeMult == 1)
        {
          lambda_mf[F_PEL] = (int)(lambda_mf[F_PEL] * lambda_mf_factor);
          lambda_mf[H_PEL] = (int)(lambda_mf[H_PEL] * lambda_mf_factor);
          lambda_mf[Q_PEL] = (int)(lambda_mf[Q_PEL] * lambda_mf_factor);
        }

        PartitionMotionSearch (currMB, mode, block, lambda_mf);

        //--- get cost and reference frame for LIST 0 prediction ---
        bmcost[LIST_0] = INT_MAX;
        list_prediction_cost(currMB, LIST_0, block, mode, enc_mb, bmcost, best_ref);

        //store LIST 0 reference index for every block
        block_x = img->block_x + (block & 0x01)*2;
        block_y = img->block_y + (block & 0x02);
        b_ref = best_ref[LIST_0];
        ref_pic_num = enc_picture->ref_pic_num[enc_mb->list_offset[LIST_0]][(short) b_ref];

        for (j = block_y; j< block_y + 2; j++)
        {
          memset(&enc_picture->motion.ref_idx [LIST_0][j][block_x], b_ref, 2 * sizeof(char));
        }

        for (j = block_y; j< block_y + 2; j++)
        {
          for (i = block_x; i < block_x + 2; i++)
          {
            enc_picture->motion.ref_pic_id[LIST_0][j][i] = ref_pic_num;
          }
        }

        if (bslice)
        {
          //--- get cost and reference frame for LIST 1 prediction ---
          bmcost[LIST_1] = INT_MAX;
          bmcost[BI_PRED] = INT_MAX;
          list_prediction_cost(currMB, LIST_1, block, mode, enc_mb, bmcost, best_ref);

          // Compute bipredictive cost between best list 0 and best list 1 references
          list_prediction_cost(currMB, BI_PRED, block, mode, enc_mb, bmcost, best_ref);
          
          // currently Bi prediction ME is only supported for modes 1, 2, 3 and only for ref 0 and only for ref 0
          if (is_bipred_enabled(mode))
          {
            list_prediction_cost(currMB, BI_PRED_L0, block, mode, enc_mb, bmcost, 0);
            list_prediction_cost(currMB, BI_PRED_L1, block, mode, enc_mb, bmcost, 0);
          }
          else
          {
            bmcost[BI_PRED_L0] = INT_MAX;
            bmcost[BI_PRED_L1] = INT_MAX;
          }

          //--- get prediction direction ----
          determine_prediction_list(mode, bmcost, best_ref, &best_pdir, cost, &bipred_me);
        
          //store backward reference index for every block
          for (k = LIST_0; k <= LIST_1; k++)
          {
            for (j = block_y; j< block_y + 2; j++)
            {
              memset(&enc_picture->motion.ref_idx[k][j][block_x], best_ref[k], 2 * sizeof(char));
            }
          }
        } // if (bslice)
        else
        {
          best_pdir = 0;
          *cost     = bmcost[LIST_0];
        }
      } // if (mode!=0)

      if (params->rdopt)
      {
        //--- get and check rate-distortion cost ---
        rdcost = RDCost_for_8x8blocks (currMB, &cnt_nonz, &curr_cbp_blk, enc_mb->lambda_md,
          block, mode, best_pdir, best_ref[LIST_0], best_ref[LIST_1], bipred_me, is_cavlc);
      }
      else
      {
        if (*cost!=INT_MAX)
          *cost += (REF_COST (enc_mb->lambda_mf[Q_PEL], B8Mode2Value (mode, best_pdir),
          enc_mb->list_offset[(best_pdir < 1 ? LIST_0 : LIST_1)]) - 1);
      }

      //--- set variables if best mode has changed ---
      if ( ( params->rdopt && rdcost < min_rdcost) || (!params->rdopt && *cost < min_cost8x8))
      {
        min_cost8x8                  = *cost;
        min_rdcost                   = rdcost;
        dataTr->part8x8mode  [block] = mode;
        dataTr->part8x8pdir  [block] = best_pdir;
        dataTr->part8x8l0ref [block] = best_ref[LIST_0];
        dataTr->part8x8l1ref [block] = best_ref[LIST_1];
        dataTr->part8x8bipred[block] = bipred_me;

        currMB->b8mode[block] = mode;

#ifdef BEST_NZ_COEFF
        if (cnt_nonz)
        {
          for(i = 0; i <= 1; i++)
          {
            for(j = 0; j <= 1; j++)  
              best_nz_coeff[i][j]= img->nz_coeff[img->current_mb_nr][i1 + i][j1 + j];
          }
        }
        else
        {
          for(i = 0; i <= 1; i++)
          {
            best_nz_coeff[i][0]= 0;
            best_nz_coeff[i][1]= 0;
          }
        }
#endif

        //--- store number of nonzero coefficients ---
        best_cnt_nonz  = cnt_nonz;

        if (params->rdopt)
        {
          //--- store block cbp ---
          cbp_blk8x8 &= (~(0x33 << (((block>>1)<<3)+((block & 0x01)<<1)))); // delete bits for block
          cbp_blk8x8 |= curr_cbp_blk;

          //--- store coefficients ---
          memcpy(&cofACtr[0][0][0],&img->cofAC[block][0][0][0], 4 * 2 * 65 * sizeof(int));

          if( img->P444_joined ) 
          {
            //--- store coefficients ---
            memcpy(&cofACCbCrtr1[0][0][0],&img->cofAC[block + 4][0][0][0], 4 * 2 * 65 * sizeof(int));
            memcpy(&cofACCbCrtr2[0][0][0],&img->cofAC[block + 8][0][0][0], 4 * 2 * 65 * sizeof(int));
          }
          
          //--- store reconstruction and prediction ---
          for (j=j0; j<j0 + BLOCK_SIZE_8x8; j++)
          {
            memcpy(&dataTr->rec_mbY8x8[j][i0],&enc_picture->imgY[img->pix_y + j][img->pix_x + i0], BLOCK_SIZE_8x8 * sizeof(imgpel));
            memcpy(&dataTr->mpr8x8[j][i0],&mb_pred[j][i0], BLOCK_SIZE_8x8 * sizeof(imgpel));
          }

          if (params->rdopt == 3)
          {
            errdo_store_best_block(img, decs->dec_mbY8x8, enc_picture->p_dec_img[0], i0, j0, BLOCK_SIZE_8x8);  //Store best 8x8 block for each hypothetical decoder
          }

          if(img->type==SP_SLICE && (!si_frame_indicator))
          {
            for (j=j0; j<j0 + BLOCK_SIZE_8x8; j++)
            {
              memcpy(&dataTr->lrec[j][i0],&lrec[img->pix_y + j][img->pix_x + i0], BLOCK_SIZE_8x8 * sizeof(int));
            }
          }
 
          if(img->P444_joined) 
          {
            for (j=j0; j<j0+8; j++)    
            {
              memcpy(&dataTr->rec_mb8x8_cr[0][j][i0],&enc_picture->imgUV[0][img->pix_y + j][img->pix_x + i0], BLOCK_SIZE_8x8 * sizeof(imgpel));
              memcpy(&dataTr->rec_mb8x8_cr[1][j][i0],&enc_picture->imgUV[1][img->pix_y + j][img->pix_x + i0], BLOCK_SIZE_8x8 * sizeof(imgpel));
              memcpy(&dataTr->mpr8x8CbCr[0][j][i0],&img->mb_pred[1][j][i0], BLOCK_SIZE_8x8 * sizeof(imgpel));
              memcpy(&dataTr->mpr8x8CbCr[1][j][i0],&img->mb_pred[2][j][i0], BLOCK_SIZE_8x8 * sizeof(imgpel));
            }
          }
          
        }
        if (img->AdaptiveRounding)
        {
          for (j=j0; j<j0+8; j++)
          {
            memcpy(&fadjust[j][i0], &fadjustTransform[0][j][i0], 8 * sizeof(int));
          }

          if (params->AdaptRndChroma || (img->P444_joined))
          {
            int j0_cr = (j0 * img->mb_cr_size_y) / MB_BLOCK_SIZE;
            int i0_cr = (i0 * img->mb_cr_size_x) / MB_BLOCK_SIZE;
            for (k = 0; k < 2; k++)
            {
              for (j=j0_cr; j < j0_cr + (img->mb_cr_size_y >> 1); j++)
              {
                memcpy(&fadjustCr[k][j][i0_cr], &fadjustTransformCr[k][0][j][i0_cr], (img->mb_cr_size_x >> 1) * sizeof(int));
              }
            }
          }
        }
        //--- store best 8x8 coding state ---
        if (block < 3)
          store_coding_state (currMB, cs_b8);
      } // if (rdcost <= min_rdcost)

      //--- re-set coding state as it was before coding with current mode was performed ---
      reset_coding_state (currMB, cs_cm);
    } // if ((enc_mb->valid[mode] && (transform8x8 == 0 || mode != 0 || (mode == 0 && active_sps->direct_8x8_inference_flag)))
  } // for (min_rdcost=1e30, index=(bslice?0:1); index<6; index++)

#ifdef BEST_NZ_COEFF
  for(i = 0; i <= 1; i++)  
  {
    for(j = 0; j <= 1; j++)
      img->nz_coeff[img->current_mb_nr][i1 + i][j1 + j] = best_nz_coeff[i][j];
  }
#endif

  if (!transform8x8)
    dataTr->mb_p8x8_cost += min_cost8x8;

  if (!params->rdopt)
  {
    if (transform8x8)
    {
      dataTr->mb_p8x8_cost += min_cost8x8;
      mode = dataTr->part8x8mode[block];
      pdir = dataTr->part8x8pdir[block];
    }
    else
    {
      mode = dataTr->part8x8mode[block];
      pdir = dataTr->part8x8pdir[block];
    }

    curr_cbp_blk  = 0;
    currMB->bipred_me[block] = dataTr->part8x8bipred[block];
    best_cnt_nonz = LumaResidualCoding8x8 (currMB, &dummy, &curr_cbp_blk, block, pdir,
      (pdir == 0 || pdir == 2 ? mode : 0), (pdir == 1 || pdir == 2 ? mode : 0), 
      dataTr->part8x8l0ref[block], dataTr->part8x8l1ref[block], is_cavlc);

    if (img->P444_joined)
      best_cnt_nonz += coeff_cost_cr[1] + coeff_cost_cr[2];

    cbp_blk8x8   &= (~(0x33 << (((block>>1)<<3)+((block & 0x01)<<1)))); // delete bits for block
    cbp_blk8x8   |= curr_cbp_blk;

    //--- store coefficients ---
    memcpy(cofACtr[0][0],img->cofAC[block][0][0],4 * 2 * 65 * sizeof(int));

    if(img->P444_joined) 
    {
      //--- store coefficients ---
      memcpy(cofACCbCrtr1[0][0],img->cofAC[block + 4][0][0], 4 * 2 * 65 * sizeof(int));
      memcpy(cofACCbCrtr2[0][0],img->cofAC[block + 8][0][0], 4 * 2 * 65 * sizeof(int));
    }


    //--- store reconstruction and prediction ---
    for (j = j0; j < j0 + BLOCK_SIZE_8x8; j++)
    {
      memcpy(&dataTr->rec_mbY8x8[j][i0], &enc_picture->imgY[img->pix_y + j][img->pix_x + i0], BLOCK_SIZE_8x8 * sizeof (imgpel));
    }
    for (j = j0; j < j0 + BLOCK_SIZE_8x8; j++)
    {
      memcpy(&dataTr->mpr8x8[j][i0], &mb_pred[j][i0], BLOCK_SIZE_8x8 * sizeof (imgpel));
    }


    //--- store reconstruction and prediction ---
    if(img->type==SP_SLICE &&(!si_frame_indicator))
    {
      for (j=j0; j < j0 + BLOCK_SIZE_8x8; j++)
      {
        memcpy(&dataTr->lrec[j][i0],&lrec[img->pix_y+j][img->pix_x+i0],BLOCK_SIZE_8x8 * sizeof(int)); // store coefficients for primary SP slice
      }
    }
    if(img->P444_joined) 
    {
      for (j=j0; j < j0 + BLOCK_SIZE_8x8; j++)
      {   
        memcpy(&dataTr->rec_mb8x8_cr[0][j][i0], &enc_picture->imgUV[0][img->pix_y + j][img->pix_x + i0], BLOCK_SIZE_8x8 * sizeof (imgpel));
        memcpy(&dataTr->rec_mb8x8_cr[1][j][i0], &enc_picture->imgUV[1][img->pix_y + j][img->pix_x + i0], BLOCK_SIZE_8x8 * sizeof (imgpel));
        memcpy(&dataTr->mpr8x8CbCr[0][j][i0], &img->mb_pred[1][j][i0], BLOCK_SIZE_8x8 * sizeof (imgpel)); 
        memcpy(&dataTr->mpr8x8CbCr[1][j][i0], &img->mb_pred[2][j][i0], BLOCK_SIZE_8x8 * sizeof (imgpel));
      }
    }   
  }

  //----- set cbp and count of nonzero coefficients ---
  if (best_cnt_nonz)
  {
    cbp8x8       |= (1 << block);
    cnt_nonz_8x8 += best_cnt_nonz;
  }

  if (!transform8x8)
  {
    if (block<3)
    {
      //===== re-set reconstructed block =====
      j0   = 8*(block >> 1);
      i0   = 8*(block & 0x01);
      for (j = j0; j < j0 + BLOCK_SIZE_8x8; j++)
      {
        memcpy(&enc_picture->imgY[img->pix_y + j][img->pix_x], dataTr->rec_mbY8x8[j], BLOCK_SIZE_8x8 * sizeof(imgpel));
      }

      if (params->rdopt == 3)
      {
        errdo_get_best_block(img, enc_picture->p_dec_img[0], decs->dec_mbY8x8, j0, BLOCK_SIZE_8x8);
      }

      if(img->type==SP_SLICE &&(!si_frame_indicator))
      {
        for (j = j0; j < j0 + BLOCK_SIZE_8x8; j++)
        {
          memcpy(&lrec[img->pix_y + j][img->pix_x], dataTr->lrec[j],2*BLOCK_SIZE*sizeof(imgpel)); // reset the coefficients for SP slice
        }
      }

      if(img->P444_joined) 
      {
        for (k=0; k<2; k++)
        {
          for (j = j0; j < j0 + BLOCK_SIZE_8x8; j++)
          {
            memcpy(&enc_picture->imgUV[k][img->pix_y + j][img->pix_x], dataTr->rec_mb8x8_cr[k][j], BLOCK_SIZE_8x8 * sizeof(imgpel));
          }
        }
      }
    } // if (block<3)
  }
  else
  {
    //======= save motion data for 8x8 partition for transform size 8x8 ========
    StoreNewMotionVectorsBlock8x8(0, block, dataTr->part8x8mode[block], dataTr->part8x8l0ref[block], dataTr->part8x8l1ref[block], dataTr->part8x8pdir[block], dataTr->part8x8bipred[block], bslice);
  }
  //===== set motion vectors and reference frames (prediction) =====
  SetRefAndMotionVectors (currMB, block, dataTr->part8x8mode[block], dataTr->part8x8pdir[block], dataTr->part8x8l0ref[block], dataTr->part8x8l1ref[block], dataTr->part8x8bipred[block]);

  //===== set the coding state after current block =====
  //if (transform8x8 == 0 || block < 3)
  if (block < 3)
    reset_coding_state (currMB, cs_b8);

  if (img->AdaptiveRounding)
  {
    for (j=j0; j < j0 + BLOCK_SIZE_8x8; j++)
    {
      memcpy(&fadjustTransform  [lumaAdjustIndex][j][i0], &fadjust[j][i0], BLOCK_SIZE_8x8 * sizeof(int));
    }

    if (params->AdaptRndChroma || (img->P444_joined))
    {
      int j0_cr = (j0 * img->mb_cr_size_y) >> MB_BLOCK_SHIFT;
      int i0_cr = (i0 * img->mb_cr_size_x) >> MB_BLOCK_SHIFT;
      for (k = 0; k < 2; k++)
      {
        for (j=j0_cr; j<j0_cr+(img->mb_cr_size_y >> 1); j++)
        {
          memcpy(&fadjustTransformCr[k][chromaAdjustIndex][j][i0_cr], &fadjustCr[k][j][i0_cr], (img->mb_cr_size_x >> 1) * sizeof(int));
        }
      }
    }
  }
}


/*!
*************************************************************************************
* \brief
*    Checks whether a primary SP slice macroblock was encoded as I16
*************************************************************************************
*/
int check_for_SI16()
{
  int i,j;
  for(i=img->pix_y;i<img->pix_y+MB_BLOCK_SIZE;i++)
  {
    for(j=img->pix_x;j<img->pix_x+MB_BLOCK_SIZE;j++)
      if(lrec[i][j]!=-16)
        return 0;
  }
  return 1;
}

void get_initial_mb16x16_cost(Macroblock* currMB)
{
  if (currMB->mb_available_left && currMB->mb_available_up)
  {
    mb16x16_cost = (mb16x16_cost_frame[img->current_mb_nr - 1] +
      mb16x16_cost_frame[img->current_mb_nr - (img->width>>4)] + 1)/2.0;
  }
  else if (currMB->mb_available_left)
  {
    mb16x16_cost = mb16x16_cost_frame[img->current_mb_nr - 1];
  }
  else if (currMB->mb_available_up)
  {
    mb16x16_cost = mb16x16_cost_frame[img->current_mb_nr - (img->width>>4)];
  }
  else
  {
    mb16x16_cost = CALM_MF_FACTOR_THRESHOLD;
  }

  lambda_mf_factor = mb16x16_cost < CALM_MF_FACTOR_THRESHOLD ? 1.0 : sqrt(mb16x16_cost / (CALM_MF_FACTOR_THRESHOLD * img->lambda_mf_factor[img->type][img->qp]));
}

void adjust_mb16x16_cost(int cost)
{
  mb16x16_cost = (double) cost;
  mb16x16_cost_frame[img->current_mb_nr] = mb16x16_cost;

  lambda_mf_factor = (mb16x16_cost < CALM_MF_FACTOR_THRESHOLD)
  ? 1.0
  : sqrt(mb16x16_cost / (CALM_MF_FACTOR_THRESHOLD * img->lambda_mf_factor[img->type][img->qp]));
}

void update_lambda_costs(RD_PARAMS *enc_mb, int lambda_mf[3])
{
  int MEPos;
  for (MEPos = 0; MEPos < 3; MEPos ++)
  {
    lambda_mf[MEPos] = params->CtxAdptLagrangeMult == 0 ? enc_mb->lambda_mf[MEPos] : (int)(enc_mb->lambda_mf[MEPos] * sqrt(lambda_mf_factor));
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Return array's minimum and its index
 *************************************************************************************
 */
int iminarray ( int arr[], int size, int *minind )
{
  int i; 
  int mincand = arr[0];
  *minind = 0;
  for ( i = 1; i < size; i++ )
  {
    if (arr[i] < mincand)
    {
      mincand = arr[i];
      *minind = i;
    }
  }
  return mincand;
} 

/*!
 *************************************************************************************
 * \brief
 *    Determines whether bi prediction is enabaled for current mode
 *************************************************************************************
 */
int is_bipred_enabled(int mode) 
{
  int enabled = 0;
  mode = (mode == P8x8) ? 4: mode;
  if (params->BiPredMotionEstimation)
  {
    if (mode > 0 && mode < 5)
    {
      enabled = (params->BiPredSearch[mode - 1]) ? 1: 0;
    }    
    else
    {
      enabled = 0;
    }
  }
  else
  {
    enabled = 0;
  }
  return enabled;
}


/*!
 *************************************************************************************
 * \brief
 *    Update prediction direction for mode P16x16 to check all prediction directions
 *************************************************************************************
 */
void update_prediction_for_mode16x16(Block8x8Info *b8x8info, int ctr16x16, int *index)
{
  char pdir = 0;
  short i, bipred_me = 0;
  switch (ctr16x16)
  {
    case 0:
      *index = *index - 1;
      break;
    case 1:
      pdir = 1;
      *index = *index - 1;
      break;
    case 2:
      pdir = 2;
      if (params->BiPredMotionEstimation)
      {
        *index = *index - 1;
      }
      break;
    case 3:
      pdir = 2;
      bipred_me = 1;
      *index = *index - 1;
      break;
    case 4:
      pdir = 2;
      bipred_me = 2;
      break;
    default:
      error("invalid 'ctr16x16' value", -1);
      break;
  }
  for (i = 0; i< 4; i++)
  {
    b8x8info->bipred8x8me[1][i] = bipred_me;
    b8x8info->best8x8pdir[1][i] = pdir;
  }
}
