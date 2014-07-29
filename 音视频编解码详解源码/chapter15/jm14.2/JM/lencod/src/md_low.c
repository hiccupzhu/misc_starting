
/*!
 ***************************************************************************
 * \file md_low.c
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
#include "q_around.h"


//==== MODULE PARAMETERS ====
static imgpel temp_imgY[16][16]; // to temp store the Y data for 8x8 transform
static imgpel temp_imgU[16][16];
static imgpel temp_imgV[16][16];


/*!
*************************************************************************************
* \brief
*    Mode Decision for a macroblock
*************************************************************************************
*/
void encode_one_macroblock_low (Macroblock *currMB)
{

  int         block, mode, i=0, j, k, dummy;
  char        best_pdir;
  RD_PARAMS   enc_mb;
  char        best_ref[2] = {0, -1};
  int         bmcost[5] = {INT_MAX};
  double      rd_cost = 0, min_rd_cost = 1e30;
  int         cost = 0;
  int         min_cost = INT_MAX, cost_direct=0, have_direct=0, i16mode=0;
  int         intra1 = 0;
  int         temp_cpb = 0;
  int         best_transform_flag = 0;
  int         cost8x8_direct = 0;
  short       islice      = (short) (img->type==I_SLICE);
  short       bslice      = (short) (img->type==B_SLICE);
  short       pslice      = (short) ((img->type==P_SLICE) || (img->type==SP_SLICE));
  short       intra       = (short) (islice || (pslice && img->mb_y==img->mb_y_upd && img->mb_y_upd!=img->mb_y_intra));
  int         lambda_mf[3];
  int         prev_mb_nr  = FmoGetPreviousMBNr(img->current_mb_nr);
  Macroblock* prevMB      = (prev_mb_nr >= 0) ? &img->mb_data[prev_mb_nr]:NULL ;
  Block8x8Info *b8x8info   = img->b8x8info;

  char   **ipredmodes = img->ipredmode;
  short   *allmvs = params->IntraProfile ? NULL: img->all_mv[0][0][0][0][0];
  int     ****i4p;  //for non-RD-opt. mode
  imgpel  (*mb_pred)[16] = img->mb_pred[0];
  int    is_cavlc = (img->currentSlice->symbol_mode == CAVLC);

  int tmp_8x8_flag, tmp_no_mbpart;
  // Fast Mode Decision
  short bipred_me, inter_skip = 0;


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
      if (enc_mb.valid[mode] && !inter_skip)
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

            // currently Bi prediction ME is only supported for modes 1, 2, 3
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
          if (mode>1 && block==0)
            SetRefAndMotionVectors (currMB, block, mode, best_pdir, best_ref[LIST_0], best_ref[LIST_1], bipred_me);
        } // for (block=0; block<(mode==1?1:2); block++)

        currMB->luma_transform_size_8x8_flag = 0;
        if (params->Transform8x8Mode) //for inter rd-off, set 8x8 to do 8x8 transform
        {
          SetModesAndRefframeForBlocks(currMB, mode);
          currMB->luma_transform_size_8x8_flag = TransformDecision(currMB, -1, &cost);
        }

        if ((!inter_skip) && (cost < min_cost))
        {
          best_mode = (short) mode;
          min_cost  = cost;
          best_transform_flag = currMB->luma_transform_size_8x8_flag;

          if (params->CtxAdptLagrangeMult == 1)
          {
            adjust_mb16x16_cost(cost);
          }
        }
      } // if (enc_mb.valid[mode])
    } // for (mode=1; mode<4; mode++)

    if ((!inter_skip) && enc_mb.valid[P8x8])
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
      if (tr4x4.mb_p8x8_cost < min_cost || tr8x8.mb_p8x8_cost < min_cost)
      {
        best_mode = P8x8;

        if (img->AdaptiveRounding)
          store_adaptive_rounding_parameters_luma (currMB, best_mode);		

        if (params->Transform8x8Mode == 2)
        {
          min_cost = tr8x8.mb_p8x8_cost;
          currMB->luma_transform_size_8x8_flag=1;
        }
        else if (params->Transform8x8Mode)
        {
          if (tr8x8.mb_p8x8_cost < tr4x4.mb_p8x8_cost)
          {
            min_cost = tr8x8.mb_p8x8_cost;
            currMB->luma_transform_size_8x8_flag=1;
          }
          else if(tr4x4.mb_p8x8_cost < tr8x8.mb_p8x8_cost)
          {
            min_cost = tr4x4.mb_p8x8_cost;
            currMB->luma_transform_size_8x8_flag=0;
          }
          else
          {
            if (GetBestTransformP8x8() == 0)
            {
              min_cost = tr4x4.mb_p8x8_cost;
              currMB->luma_transform_size_8x8_flag=0;
            }
            else
            {
              min_cost = tr8x8.mb_p8x8_cost;
              currMB->luma_transform_size_8x8_flag=1;
            }
          }
        }
        else
        {
          min_cost = tr4x4.mb_p8x8_cost;
          currMB->luma_transform_size_8x8_flag=0;
        }
      }// if ((tr4x4.mb_p8x8_cost < min_cost || tr8x8.mb_p8x8_cost < min_cost))
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
  tmp_8x8_flag = currMB->luma_transform_size_8x8_flag;  //save 8x8_flag
  tmp_no_mbpart = currMB->NoMbPartLessThan8x8Flag;      //save no-part-less
  if ((img->yuv_format != YUV400) && (img->yuv_format != YUV444))
    // precompute all chroma intra prediction modes
    IntraChromaPrediction(currMB, NULL, NULL, NULL);

  if (enc_mb.valid[0] && bslice) // check DIRECT MODE
  {
    if(have_direct)
    {
      switch(params->Transform8x8Mode)
      {
      case 1: // Mixture of 8x8 & 4x4 transform
        cost = ((cost8x8_direct < cost_direct) || !(enc_mb.valid[5] && enc_mb.valid[6] && enc_mb.valid[7]))
          ? cost8x8_direct : cost_direct;
        break;
      case 2: // 8x8 Transform only
        cost = cost8x8_direct;
        break;
      default: // 4x4 Transform only
        cost = cost_direct;
        break;
      }
    }
    else
    { //!have_direct
      cost = GetDirectCostMB (currMB, bslice);
    }
    if (cost!=INT_MAX)
    {
      cost -= (int)floor(16*enc_mb.lambda_md+0.4999);
    }

    if (cost <= min_cost)
    {
      if(active_sps->direct_8x8_inference_flag && params->Transform8x8Mode)
      {
        if(params->Transform8x8Mode==2)
          currMB->luma_transform_size_8x8_flag=1;
        else
        {
          if(cost8x8_direct < cost_direct)
            currMB->luma_transform_size_8x8_flag=1;
          else
            currMB->luma_transform_size_8x8_flag=0;
        }
      }
      else
        currMB->luma_transform_size_8x8_flag=0;

      //Rate control
      if (params->RCEnable)
        rc_store_diff(img->opix_x, img->opix_y, mb_pred);

      min_cost  = cost;
      best_mode = 0;
      tmp_8x8_flag = currMB->luma_transform_size_8x8_flag;
    }
    else
    {
      currMB->luma_transform_size_8x8_flag = tmp_8x8_flag; // restore if not best
      currMB->NoMbPartLessThan8x8Flag = tmp_no_mbpart; // restore if not best
    }
  }

  min_rd_cost = (double) min_cost;

  if (enc_mb.valid[I8MB]) // check INTRA8x8
  {
    currMB->luma_transform_size_8x8_flag = 1; // at this point cost will ALWAYS be less than min_cost

    currMB->mb_type = I8MB;
    temp_cpb = Mode_Decision_for_new_Intra8x8Macroblock (currMB, enc_mb.lambda_md, &rd_cost);


    if (rd_cost <= min_rd_cost) //HYU_NOTE. bug fix. 08/15/07
    {
      currMB->cbp = temp_cpb;
      if (img->P444_joined)
      {
        curr_cbp[0] = cmp_cbp[1];  
        curr_cbp[1] = cmp_cbp[2];
      }

      if(enc_mb.valid[I4MB])   //KHHan. bug fix. Oct.15.2007
      {
        //coeffs
        if (params->Transform8x8Mode != 2) 
        {
          i4p=cofAC; cofAC=img->cofAC; img->cofAC=i4p;
        }
      }

      for(j=0; j<MB_BLOCK_SIZE; j++)
      {
        memcpy(temp_imgY[j], &enc_picture->imgY[img->pix_y + j][img->pix_x], MB_BLOCK_SIZE * sizeof (imgpel));
      }

      if (img->P444_joined)
      {
        for(j=0; j<MB_BLOCK_SIZE; j++)
        {
          memcpy(temp_imgU[j], &enc_picture->imgUV[0][img->pix_y + j][img->pix_x], MB_BLOCK_SIZE * sizeof (imgpel));
          memcpy(temp_imgV[j], &enc_picture->imgUV[1][img->pix_y + j][img->pix_x], MB_BLOCK_SIZE * sizeof (imgpel));
        }
      }

      //Rate control
      if (params->RCEnable)
        rc_store_diff(img->opix_x, img->opix_y, mb_pred);

      min_rd_cost  = rd_cost; 
      best_mode = I8MB;
      tmp_8x8_flag = currMB->luma_transform_size_8x8_flag;
    }
    else
    {
      currMB->luma_transform_size_8x8_flag = tmp_8x8_flag; // restore if not best
      if (img->P444_joined)
      {
        cmp_cbp[1] = curr_cbp[0]; 
        cmp_cbp[2] = curr_cbp[1]; 
        currMB->cbp |= cmp_cbp[1];    
        currMB->cbp |= cmp_cbp[2];    
        cmp_cbp[1] = currMB->cbp;   
        cmp_cbp[2] = currMB->cbp;
      }
    }
  }

  if (enc_mb.valid[I4MB]) // check INTRA4x4
  {
    currMB->luma_transform_size_8x8_flag = 0;
    currMB->mb_type = I4MB;
    temp_cpb = Mode_Decision_for_Intra4x4Macroblock (currMB, enc_mb.lambda_md, &rd_cost, is_cavlc);

    if (rd_cost <= min_rd_cost) 
    {
      currMB->cbp = temp_cpb;

      //Rate control
      if (params->RCEnable)
        rc_store_diff(img->opix_x, img->opix_y, mb_pred);

      min_rd_cost  = rd_cost; 
      best_mode = I4MB;
      tmp_8x8_flag = currMB->luma_transform_size_8x8_flag;

      if (img->AdaptiveRounding)
        store_adaptive_rounding_parameters_luma (currMB, best_mode);
    }
    else
    {
      currMB->luma_transform_size_8x8_flag = tmp_8x8_flag; // restore if not best
      if (img->P444_joined)
      {
        cmp_cbp[1] = curr_cbp[0]; 
        cmp_cbp[2] = curr_cbp[1]; 
        currMB->cbp |= cmp_cbp[1];    
        currMB->cbp |= cmp_cbp[2];    
        cmp_cbp[1] = currMB->cbp;   
        cmp_cbp[2] = currMB->cbp;
      }
      //coeffs
      i4p=cofAC; cofAC=img->cofAC; img->cofAC=i4p;
    }
  }
  if (enc_mb.valid[I16MB]) // check INTRA16x16
  {
    currMB->luma_transform_size_8x8_flag = 0;
    intrapred_16x16 (currMB, PLANE_Y);
    if (img->P444_joined)
    {
      select_plane(PLANE_U);
      intrapred_16x16 (currMB, PLANE_U);
      select_plane(PLANE_V);
      intrapred_16x16 (currMB, PLANE_V);
      select_plane(PLANE_Y);
    }
    switch(params->FastIntra16x16)
    {
    case 0:
    default:
      find_sad_16x16 = find_sad_16x16_JM;
      break;
    }

    rd_cost = find_sad_16x16 (currMB, &i16mode);

    if (rd_cost < min_rd_cost)
    {
      //Rate control      
      if (params->RCEnable)
        rc_store_diff(img->opix_x,img->opix_y,img->mpr_16x16[0][i16mode]);

      best_mode   = I16MB;      
      min_rd_cost  = rd_cost; 
      currMB->cbp = pDCT_16x16 (currMB, PLANE_Y, i16mode, is_cavlc);

      if (img->AdaptiveRounding)
        store_adaptive_rounding_parameters_luma (currMB, best_mode);

      if (img->P444_joined)
      {
        select_plane(PLANE_U);
        cmp_cbp[1] = pDCT_16x16(currMB, PLANE_U, i16mode, is_cavlc);
        select_plane(PLANE_V);
        cmp_cbp[2] = pDCT_16x16(currMB, PLANE_V, i16mode, is_cavlc);   

        select_plane(PLANE_Y);
        currMB->cbp |= cmp_cbp[1];    
        currMB->cbp |= cmp_cbp[2];    
        cmp_cbp[1] = currMB->cbp;   
        cmp_cbp[2] = currMB->cbp;
      }

    }
    else
    {
      currMB->luma_transform_size_8x8_flag = tmp_8x8_flag; // restore
      currMB->NoMbPartLessThan8x8Flag = tmp_no_mbpart;     // restore
    }
  }

  intra1 = IS_INTRA(currMB);

  //=====  S E T   F I N A L   M A C R O B L O C K   P A R A M E T E R S ======
  //---------------------------------------------------------------------------
  {
    //===== set parameters for chosen mode =====
    SetModesAndRefframeForBlocks (currMB, best_mode);

    if (best_mode==P8x8)
    {
      if (currMB->luma_transform_size_8x8_flag && (cbp8_8x8ts == 0) && params->Transform8x8Mode != 2)
        currMB->luma_transform_size_8x8_flag = 0;

      SetCoeffAndReconstruction8x8 (currMB);

      memset(currMB->intra_pred_modes, DC_PRED, MB_BLOCK_PARTITIONS * sizeof(char));
      for (k=0, j = img->block_y; j < img->block_y + BLOCK_MULTIPLE; j++)
        memset(&ipredmodes[j][img->block_x], DC_PRED, BLOCK_MULTIPLE * sizeof(char));
    }
    else
    {
      //===== set parameters for chosen mode =====
      if (best_mode == I8MB)
      {
        memcpy(currMB->intra_pred_modes,currMB->intra_pred_modes8x8, MB_BLOCK_PARTITIONS * sizeof(char));
        for(j = img->block_y; j < img->block_y + BLOCK_MULTIPLE; j++)
          memcpy(&img->ipredmode[j][img->block_x],&img->ipredmode8x8[j][img->block_x], BLOCK_MULTIPLE * sizeof(char));

        //--- restore reconstruction for 8x8 transform ---
        for(j=0; j<MB_BLOCK_SIZE; j++)
        {
          memcpy(&enc_picture->imgY[img->pix_y + j][img->pix_x],temp_imgY[j], MB_BLOCK_SIZE * sizeof(imgpel));
        }
        if (img->P444_joined)
        {
          for(j=0; j<MB_BLOCK_SIZE; j++)
          {
            memcpy(&enc_picture->imgUV[0][img->pix_y + j][img->pix_x],temp_imgU[j], MB_BLOCK_SIZE * sizeof(imgpel)); 
            memcpy(&enc_picture->imgUV[1][img->pix_y + j][img->pix_x],temp_imgV[j], MB_BLOCK_SIZE * sizeof(imgpel));
          }
        }
      }

      if ((best_mode!=I4MB)&&(best_mode != I8MB))
      {
        memset(currMB->intra_pred_modes,DC_PRED, MB_BLOCK_PARTITIONS * sizeof(char));
        for(j = img->block_y; j < img->block_y + BLOCK_MULTIPLE; j++)
          memset(&ipredmodes[j][img->block_x],DC_PRED, BLOCK_MULTIPLE * sizeof(char));

        if (best_mode!=I16MB)
        {
          if((best_mode>=1) && (best_mode<=3))
            currMB->luma_transform_size_8x8_flag = best_transform_flag;
          LumaResidualCoding (currMB, is_cavlc);

          if (img->AdaptiveRounding)
            store_adaptive_rounding_parameters_luma (currMB, best_mode);

          if (img->P444_joined)
          {
            if((currMB->cbp==0 && cmp_cbp[1] == 0 && cmp_cbp[2] == 0) &&(best_mode==0))
              currMB->luma_transform_size_8x8_flag = 0;
          }
          else if((currMB->cbp==0)&&(best_mode==0))
            currMB->luma_transform_size_8x8_flag = 0;

          //Rate control
          if (params->RCEnable)
            rc_store_diff(img->opix_x,img->opix_y,mb_pred);
        }
      }
    }
    //check luma cbp for transform size flag
    if (((currMB->cbp&15) == 0) && !(IS_OLDINTRA(currMB) || currMB->mb_type == I8MB))
      currMB->luma_transform_size_8x8_flag = 0;

    // precompute all chroma intra prediction modes
    if ((img->yuv_format != YUV400) && (img->yuv_format != YUV444))
      IntraChromaPrediction(currMB, NULL, NULL, NULL);

    img->i16offset = 0;
    dummy = 0;

    if ((img->yuv_format != YUV400) && (img->yuv_format != YUV444))
      ChromaResidualCoding (currMB, is_cavlc);

    if (img->AdaptiveRounding)
      store_adaptive_rounding_parameters_chroma (currMB, best_mode);

    if (best_mode==I16MB)
    {
      img->i16offset = I16Offset  (currMB->cbp, i16mode);
    }

    SetMotionVectorsMB (currMB, bslice);

    //===== check for SKIP mode =====
    if(img->P444_joined)
    {
      if ((pslice) && best_mode==1 && currMB->cbp==0 && cmp_cbp[1] == 0 && cmp_cbp[2] == 0 &&
        enc_picture->motion.ref_idx[LIST_0][img->block_y][img->block_x]    == 0 &&
        enc_picture->motion.mv     [LIST_0][img->block_y][img->block_x][0] == allmvs[0] &&
        enc_picture->motion.mv     [LIST_0][img->block_y][img->block_x][1] == allmvs[1])
      {
        currMB->mb_type = currMB->b8mode[0] = currMB->b8mode[1] = currMB->b8mode[2] = currMB->b8mode[3] = 0;
        currMB->luma_transform_size_8x8_flag = 0;
      }
    }
    else if ((pslice) && best_mode==1 && currMB->cbp==0 &&
      enc_picture->motion.ref_idx[LIST_0][img->block_y][img->block_x]    == 0 &&
      enc_picture->motion.mv     [LIST_0][img->block_y][img->block_x][0] == allmvs[0] &&
      enc_picture->motion.mv     [LIST_0][img->block_y][img->block_x][1] == allmvs[1])
    {
      currMB->mb_type = currMB->b8mode[0] = currMB->b8mode[1] = currMB->b8mode[2] = currMB->b8mode[3] = 0;
      currMB->luma_transform_size_8x8_flag = 0;
    }

    if (img->MbaffFrameFlag || (params->UseRDOQuant && params->RDOQ_QP_Num > 1))
      set_mbaff_parameters(currMB);
  }

  // Rate control
  if(params->RCEnable && params->RCUpdateMode <= MAX_RC_MODE)
    rc_store_mad(currMB);
  update_qp_cbp(currMB, best_mode);

  rdopt->min_rdcost = min_rd_cost;
  rdopt->min_dcost = min_rd_cost;

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

  /*update adaptive rounding offset params*/
  if (img->AdaptiveRounding)
  {
    update_offset_params(currMB, best_mode, currMB->luma_transform_size_8x8_flag);
  }
}

