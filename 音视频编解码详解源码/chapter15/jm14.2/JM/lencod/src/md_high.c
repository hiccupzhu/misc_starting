/*!
 ***************************************************************************
 * \file md_high.c
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


/*!
*************************************************************************************
* \brief
*    Mode Decision for a macroblock
*************************************************************************************
*/
void encode_one_macroblock_high (Macroblock *currMB)
{
  int         max_index = 9;
  int         block, index, mode, i, j, ctr16x16;
  char        best_pdir;
  RD_PARAMS   enc_mb;
  double      min_rdcost = 1e30;
  double      min_dcost = 1e30;
  char        best_ref[2] = {0, -1};
  int         bmcost[5] = {INT_MAX};
  int         cost=0;
  int         min_cost = INT_MAX, cost_direct=0, have_direct=0, i16mode=0;
  int         intra1 = 0;
  int         cost8x8_direct = 0;
  int         mb_available_up;
  int         mb_available_left;
  int         mb_available_up_left;
  int         best8x8l0ref, best8x8l1ref; 

  short       islice      = (short) (img->type==I_SLICE);
  short       bslice      = (short) (img->type==B_SLICE);
  short       pslice      = (short) ((img->type==P_SLICE) || (img->type==SP_SLICE));
  short       intra       = (short) (islice || (pslice && img->mb_y==img->mb_y_upd && img->mb_y_upd!=img->mb_y_intra));
  int         lambda_mf[3];
  int         is_cavlc = (img->currentSlice->symbol_mode == CAVLC);

  int         prev_mb_nr    = FmoGetPreviousMBNr(img->current_mb_nr);
  Macroblock* prevMB        = (prev_mb_nr >= 0) ? &img->mb_data[prev_mb_nr]:NULL ;
  imgpel    (*mb_pred)[16] = img->mb_pred[0];
  Block8x8Info *b8x8info   = img->b8x8info;

  short   min_chroma_pred_mode, max_chroma_pred_mode;

  short   inter_skip = 0;
  short   bipred_me = 0;
  double min_rate = 0;

  if(params->SearchMode == UM_HEX)
  {
    UMHEX_decide_intrabk_SAD();
  }
  else if (params->SearchMode == UM_HEX_SIMPLE)
  {
    smpUMHEX_decide_intrabk_SAD();
  }

  intra |= RandomIntra (img->current_mb_nr);    // Forced Pseudo-Random Intra

  //===== Setup Macroblock encoding parameters =====
  init_enc_mb_params(currMB, &enc_mb, intra, bslice);

  // reset chroma intra predictor to default
  currMB->c_ipred_mode = DC_PRED_8;

  //=====   S T O R E   C O D I N G   S T A T E   =====
  //---------------------------------------------------
  store_coding_state (currMB, cs_cm);

  if (!intra)
  {
    //===== set direct motion vectors =====
    best_mode = 1;
    if (bslice)
    {
      Get_Direct_Motion_Vectors (currMB);
    }

    if (params->CtxAdptLagrangeMult == 1)
    {
      get_initial_mb16x16_cost(currMB);
    }

    //===== MOTION ESTIMATION FOR 16x16, 16x8, 8x16 BLOCKS =====
    for (min_cost=INT_MAX, mode=1; mode<4; mode++)
    {
      bipred_me = 0;
      b8x8info->bipred8x8me[mode][0] = 0;
      if (enc_mb.valid[mode])
      {
        for (cost=0, block=0; block<(mode==1?1:2); block++)
        {
          update_lambda_costs(&enc_mb, lambda_mf);
          PartitionMotionSearch (currMB, mode, block, lambda_mf);

          //--- set 4x4 block indizes (for getting MV) ---
          j = (block==1 && mode==2 ? 2 : 0);
          i = (block==1 && mode==3 ? 2 : 0);

          //--- get cost and reference frame for List 0 prediction ---
          bmcost[LIST_0] = INT_MAX;
          list_prediction_cost(currMB, LIST_0, block, mode, &enc_mb, bmcost, best_ref);

          if (bslice)
          {
            //--- get cost and reference frame for List 1 prediction ---
            bmcost[LIST_1] = INT_MAX;
            list_prediction_cost(currMB, LIST_1, block, mode, &enc_mb, bmcost, best_ref);

            // Compute bipredictive cost between best list 0 and best list 1 references
            list_prediction_cost(currMB, BI_PRED, block, mode, &enc_mb, bmcost, best_ref);

            // currently Bi prediction ME is only supported for modes 1, 2, 3 and ref 0
            if (is_bipred_enabled(mode))
            {
              list_prediction_cost(currMB, BI_PRED_L0, block, mode, &enc_mb, bmcost, 0);
              list_prediction_cost(currMB, BI_PRED_L1, block, mode, &enc_mb, bmcost, 0);
            }
            else
            {
              bmcost[BI_PRED_L0] = INT_MAX;
              bmcost[BI_PRED_L1] = INT_MAX;
            }

            // Determine prediction list based on mode cost
            determine_prediction_list(mode, bmcost, best_ref, &best_pdir, &cost, &bipred_me);
          }
          else // if (bslice)
          {
            best_pdir  = 0;
            cost      += bmcost[LIST_0];
          }


          assign_enc_picture_params(mode, best_pdir, block, enc_mb.list_offset[LIST_0], best_ref[LIST_0], best_ref[LIST_1], bslice, bipred_me);
          //----- set reference frame and direction parameters -----
          set_block8x8_info(b8x8info, mode, block, best_ref, best_pdir, bipred_me);

          //--- set reference frames and motion vectors ---
          if (mode>1 && block == 0)
            SetRefAndMotionVectors (currMB, block, mode, best_pdir, best_ref[LIST_0], best_ref[LIST_1], bipred_me);
        } // for (block=0; block<(mode==1?1:2); block++)

        if (cost < min_cost)
        {
          best_mode = mode;
          min_cost  = cost;
          if (params->CtxAdptLagrangeMult == 1)
          {
            adjust_mb16x16_cost(cost);
          }
        }
      } // if (enc_mb.valid[mode])
    } // for (mode=1; mode<4; mode++)

    if (enc_mb.valid[P8x8])
    {
      giRDOpt_B8OnlyFlag = 1;

      tr8x8.mb_p8x8_cost = INT_MAX;
      tr4x4.mb_p8x8_cost = INT_MAX;
      //===== store coding state of macroblock =====
      store_coding_state (currMB, cs_mb);

      currMB->all_blk_8x8 = -1;

      if (params->Transform8x8Mode)
      {
        tr8x8.mb_p8x8_cost = 0;
        //===========================================================
        // Check 8x8 partition with transform size 8x8
        //===========================================================
        //=====  LOOP OVER 8x8 SUB-PARTITIONS  (Motion Estimation & Mode Decision) =====
        for (cost_direct=cbp8x8=cbp_blk8x8=cnt_nonz_8x8=0, block = 0; block < 4; block++)
        {
          submacroblock_mode_decision(&enc_mb, &tr8x8, currMB, cofAC8x8ts[0][block], cofAC8x8ts[1][block], cofAC8x8ts[2][block],
            &have_direct, bslice, block, &cost_direct, &cost, &cost8x8_direct, 1, is_cavlc);
          set_subblock8x8_info(b8x8info, P8x8, block, &tr8x8);
        }

        // following params could be added in RD_8x8DATA structure
        cbp8_8x8ts      = cbp8x8;
        cbp_blk8_8x8ts  = cbp_blk8x8;
        cnt_nonz8_8x8ts = cnt_nonz_8x8;
        currMB->luma_transform_size_8x8_flag = 0; //switch to 4x4 transform size

        //--- re-set coding state (as it was before 8x8 block coding) ---
        //reset_coding_state (currMB, cs_mb);
      }// if (params->Transform8x8Mode)


      if (params->Transform8x8Mode != 2)
      {
        tr4x4.mb_p8x8_cost = 0;
        //=================================================================
        // Check 8x8, 8x4, 4x8 and 4x4 partitions with transform size 4x4
        //=================================================================
        //=====  LOOP OVER 8x8 SUB-PARTITIONS  (Motion Estimation & Mode Decision) =====
        for (cost_direct=cbp8x8=cbp_blk8x8=cnt_nonz_8x8=0, block=0; block<4; block++)
        {
          submacroblock_mode_decision(&enc_mb, &tr4x4, currMB, cofAC8x8[block], cofAC8x8CbCr[0][block], cofAC8x8CbCr[1][block],
            &have_direct, bslice, block, &cost_direct, &cost, &cost8x8_direct, 0, is_cavlc);
          set_subblock8x8_info(b8x8info, P8x8, block, &tr4x4);
        }
        //--- re-set coding state (as it was before 8x8 block coding) ---
        // reset_coding_state (currMB, cs_mb);
      }// if (params->Transform8x8Mode != 2)

      //--- re-set coding state (as it was before 8x8 block coding) ---
      reset_coding_state (currMB, cs_mb);

      // This is not enabled yet since mpr has reverse order.
      if (params->RCEnable)
        rc_store_diff(img->opix_x, img->opix_y, mb_pred);

      //check cost for P8x8 for non-rdopt mode
      giRDOpt_B8OnlyFlag = 0;
    }
    else // if (enc_mb.valid[P8x8])
    {
      tr4x4.mb_p8x8_cost = INT_MAX;
    }

    // Find a motion vector for the Skip mode
    if(pslice)
      FindSkipModeMotionVector (currMB);
  }
  else // if (!intra)
  {
    min_cost = INT_MAX;
  }

  //========= C H O O S E   B E S T   M A C R O B L O C K   M O D E =========
  //-------------------------------------------------------------------------
  if ((img->yuv_format != YUV400) && !IS_INDEPENDENT(params))
  {
    // precompute all new chroma intra prediction modes
    IntraChromaPrediction(currMB, &mb_available_up, &mb_available_left, &mb_available_up_left);

    if (params->FastCrIntraDecision) 
    {           
      IntraChromaRDDecision(currMB, enc_mb);

      min_chroma_pred_mode = (short) currMB->c_ipred_mode;
      max_chroma_pred_mode = (short) currMB->c_ipred_mode;
    }
    else 
    {
      min_chroma_pred_mode = DC_PRED_8;
      max_chroma_pred_mode = PLANE_8;
    }
  }
  else
  {
    min_chroma_pred_mode = DC_PRED_8;        
    max_chroma_pred_mode = DC_PRED_8;
  }

  for (currMB->c_ipred_mode=min_chroma_pred_mode; currMB->c_ipred_mode<=max_chroma_pred_mode; currMB->c_ipred_mode++)
  {
    // bypass if c_ipred_mode is not allowed
    if ( (img->yuv_format != YUV400) &&
      (  ((!intra || !params->IntraDisableInterOnly) && params->ChromaIntraDisable == 1 && currMB->c_ipred_mode!=DC_PRED_8) 
      || (currMB->c_ipred_mode == VERT_PRED_8 && !mb_available_up) 
      || (currMB->c_ipred_mode == HOR_PRED_8  && !mb_available_left) 
      || (currMB->c_ipred_mode == PLANE_8     && (!mb_available_left || !mb_available_up || !mb_available_up_left))))
      continue;        

    //===== GET BEST MACROBLOCK MODE =====
    for (ctr16x16=0, index=0; index < max_index; index++)
    {
      mode = mb_mode_table[index];

      if (img->yuv_format != YUV400)
      {           
        mode = mb_mode_table[index];
        i16mode = 0; 
      }
      //--- for INTER16x16 check all prediction directions ---
      if (mode == 1 && bslice)
      {
        update_prediction_for_mode16x16(b8x8info, ctr16x16, &index);
        ctr16x16++;
      }

      // Skip intra modes in inter slices if best mode is inter <P8x8 with cbp equal to 0    
      if (img->P444_joined)
      {
        if (params->SkipIntraInInterSlices && !intra && mode >= I16MB 
          && best_mode <=3 && currMB->cbp == 0 && cmp_cbp[1] == 0 && cmp_cbp[2] == 0)
          continue;
      }
      else
      {

        if (params->SkipIntraInInterSlices && !intra && mode >= I4MB && best_mode <=3 && currMB->cbp == 0)
          continue;
      }

      // check if weights are in valid range for biprediction.
      if (bslice && active_pps->weighted_bipred_idc == 1 && mode < P8x8) 
      {
        int cur_blk, cur_comp;
        int weight_sum;
        Boolean invalid_mode = FALSE;
        for (cur_blk = 0; cur_blk < 4; cur_blk ++)
        {
          if (b8x8info->best8x8pdir[mode][cur_blk] == 2)
          { 
            for (cur_comp = 0; cur_comp < (active_sps->chroma_format_idc == YUV400 ? 1 : 3) ; cur_comp ++)
            {
              best8x8l0ref = (int) b8x8info->best8x8l0ref[mode][cur_blk];
              best8x8l1ref = (int) b8x8info->best8x8l1ref[mode][cur_blk];
              weight_sum = wbp_weight[0][best8x8l0ref][best8x8l1ref][cur_comp] + wbp_weight[1][best8x8l0ref][best8x8l1ref][cur_comp];

              if (weight_sum < -128 ||  weight_sum > 127) 
              {
                invalid_mode = TRUE;
                break;
              }
            }
            if (invalid_mode == TRUE)
              break;
          }
        }
        if (invalid_mode == TRUE)
          continue;
      }
      if (enc_mb.valid[mode])
        compute_mode_RD_cost(mode, currMB, &enc_mb, &min_rdcost, &min_dcost, &min_rate, i16mode, bslice, &inter_skip, is_cavlc);

    }// for (ctr16x16=0, index=0; index<max_index; index++)
  }// for (currMB->c_ipred_mode=DC_PRED_8; currMB->c_ipred_mode<=max_chroma_pred_mode; currMB->c_ipred_mode++)                     

#ifdef BEST_NZ_COEFF
  for (j=0;j<4;j++)
    for (i=0; i<(4+img->num_blk8x8_uv); i++)
      img->nz_coeff[img->current_mb_nr][j][i] = gaaiMBAFF_NZCoeff[j][i]; 
#endif

  intra1 = IS_INTRA(currMB);

  //=====  S E T   F I N A L   M A C R O B L O C K   P A R A M E T E R S ======
  //---------------------------------------------------------------------------
  update_qp_cbp_tmp(currMB, cbp, best_mode);
  set_stored_macroblock_parameters (currMB);

  // Rate control
  if(params->RCEnable && params->RCUpdateMode <= MAX_RC_MODE)
    rc_store_mad(currMB);
  update_qp_cbp(currMB, best_mode);

  rdopt->min_rdcost = min_rdcost;
  rdopt->min_dcost = min_dcost;

  if ( (img->MbaffFrameFlag)
    && (img->current_mb_nr%2)
    && (currMB->mb_type ? 0:((bslice) ? !currMB->cbp:1))  // bottom is skip
    && (prevMB->mb_type ? 0:((bslice) ? !prevMB->cbp:1))
    && !(field_flag_inference(currMB) == enc_mb.curr_mb_field)) // top is skip
  {
    rdopt->min_rdcost = 1e30;  // don't allow coding of a MB pair as skip if wrong inference
  }

  //===== Decide if this MB will restrict the reference frames =====
  if (params->RestrictRef)
    update_refresh_map(intra, intra1, currMB);

  if(params->SearchMode == UM_HEX)
  {
    UMHEX_skip_intrabk_SAD(best_mode, listXsize[enc_mb.list_offset[LIST_0]]);
  }
  else if(params->SearchMode == UM_HEX_SIMPLE)
  {
    smpUMHEX_skip_intrabk_SAD(best_mode, listXsize[enc_mb.list_offset[LIST_0]]);
  }

  //--- constrain intra prediction ---
  if(params->UseConstrainedIntraPred && (img->type==P_SLICE || img->type==B_SLICE))
  {
    img->intra_block[img->current_mb_nr] = IS_INTRA(currMB);
  }
}

