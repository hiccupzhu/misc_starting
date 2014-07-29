
/*!
 ***************************************************************************
 * \file rdopt.c
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

#include <math.h>
#include <limits.h>

#include "global.h"

#include "rdopt.h"
#include "q_around.h"
#include "rdopt_coding_state.h"
#include "memalloc.h"
#include "mb_access.h"
#include "elements.h"
#include "intrarefresh.h"
#include "image.h"
#include "transform8x8.h"
#include "cabac.h"
#include "vlc.h"
#include "me_umhex.h"
#include "ratectl.h"            // head file for rate control
#include "mode_decision.h"
#include "rd_intra_jm.h"
#include "fmo.h"
#include "macroblock.h"
#include "symbol.h"
#include "q_offsets.h"
#include "conformance.h"
#include "errdo.h"

imgpel pred[16][16];

#define FASTMODE 1
//#define RESET_STATE


imgpel   rec_mbY[16][16], rec_mb_cr[2][16][16];    // reconstruction values

int lrec_rec[MB_BLOCK_SIZE][MB_BLOCK_SIZE],lrec_rec_uv[2][MB_BLOCK_SIZE][MB_BLOCK_SIZE]; // store the transf. and quantized coefficients for SP frames

static int diff[16];
static int diff4x4[64];
static int diff8x8[64];
RD_8x8DATA tr4x4, tr8x8;


int   ****cofAC=NULL, ****cofAC8x8=NULL;        // [8x8block][4x4block][level/run][scan_pos]
int   ***cofDC=NULL;                       // [yuv][level/run][scan_pos]
int   **cofAC4x4=NULL, ****cofAC4x4intern=NULL; // [level/run][scan_pos]
int   cbp, cbp8x8, cnt_nonz_8x8;
int   cbp_blk8x8;
char  l0_refframe[4][4], l1_refframe[4][4];
short b8mode[4], b8pdir[4], b8bipred_me[4];

CSptr cs_mb=NULL, cs_b8=NULL, cs_cm=NULL, cs_ib8=NULL, cs_ib4=NULL;
int   best_c_imode;
int   best_i16offset;
short best_mode;


//mixed transform sizes definitions
int   luma_transform_size_8x8_flag;

short all_mv8x8[2][2][4][4][2];       //[8x8_data/temp_data][LIST][block_x][block_y][MVx/MVy]
short pred_mv8x8[2][2][4][4][2];

int   ****cofAC8x8ts[3] = {NULL, NULL, NULL};        // [plane][8x8block][4x4block][level/run][scan_pos]
int   ****cofAC8x8CbCr[2];
int   **cofAC4x4CbCr[2];
int   ****cofAC4x4CbCrintern[2];

int64    cbp_blk8_8x8ts;
int      cbp8_8x8ts;
int      cost8_8x8ts;
int      cnt_nonz8_8x8ts;

// adaptive langrangian parameters
double mb16x16_cost;
double lambda_mf_factor;

void StoreMV8x8(int dir);
void RestoreMV8x8(int dir);
// end of mixed transform sizes definitions

char  b4_ipredmode[16], b4_intra_pred_modes[16];

/*!
 ************************************************************************
 * \brief
 *    delete structure for RD-optimized mode decision
 ************************************************************************
 */
void clear_rdopt (InputParameters *params)
{
  free_mem_DCcoeff (cofDC);
  free_mem_ACcoeff (cofAC);
  free_mem_ACcoeff (cofAC8x8);
  free_mem_ACcoeff (cofAC4x4intern);

  if (params->Transform8x8Mode)
  {
    free_mem_ACcoeff (cofAC8x8ts[0]);
    //if (img->P444_joined)
    {
      free_mem_ACcoeff (cofAC8x8ts[1]);
      free_mem_ACcoeff (cofAC8x8ts[2]);
    }
  }
  //if (img->P444_joined)
  {
    free_mem_ACcoeff (cofAC8x8CbCr[0]);
    free_mem_ACcoeff (cofAC8x8CbCr[1]);
    free_mem_ACcoeff (cofAC4x4CbCrintern[0]);
    free_mem_ACcoeff (cofAC4x4CbCrintern[1]);
  }

  if (params->AdaptiveRounding)
  {
    clear_adaptive_rounding (params);
  }

  // structure for saving the coding state
  delete_coding_state (cs_mb);
  delete_coding_state (cs_b8);
  delete_coding_state (cs_cm);
  delete_coding_state (cs_ib8);
  delete_coding_state (cs_ib4);
}


/*!
 ************************************************************************
 * \brief
 *    create structure for RD-optimized mode decision
 ************************************************************************
 */
void init_rdopt (InputParameters *params)
{
  rdopt = NULL;

  get_mem_DCcoeff (&cofDC);
  get_mem_ACcoeff (&cofAC);
  get_mem_ACcoeff (&cofAC8x8);
  get_mem_ACcoeff (&cofAC4x4intern);
  cofAC4x4 = cofAC4x4intern[0][0];

  if (params->Transform8x8Mode)
  {
    get_mem_ACcoeff (&cofAC8x8ts[0]);
    //if (img->P444_joined)
    {
      get_mem_ACcoeff (&cofAC8x8ts[1]);
      get_mem_ACcoeff (&cofAC8x8ts[2]);
    }
  }

  //if (img->P444_joined)
  {
    get_mem_ACcoeff (&cofAC8x8CbCr[0]);
    get_mem_ACcoeff (&cofAC8x8CbCr[1]);

    get_mem_ACcoeff (&cofAC4x4CbCrintern[0]);
    get_mem_ACcoeff (&cofAC4x4CbCrintern[1]);
    cofAC4x4CbCr[0] = cofAC4x4CbCrintern[0][0][0];
    cofAC4x4CbCr[1] = cofAC4x4CbCrintern[1][0][0];    
  }

  SetLagrangianMultipliers = params->rdopt == 0 ? SetLagrangianMultipliersOff : SetLagrangianMultipliersOn;

  switch (params->rdopt)
  {
  case 0:
    encode_one_macroblock = encode_one_macroblock_low;
    break;
  case 1:
  default:
    encode_one_macroblock = encode_one_macroblock_high;
    break;
  case 2:
    encode_one_macroblock = encode_one_macroblock_highfast;
    break;
  case 3:
    encode_one_macroblock = encode_one_macroblock_highloss;
    break;
  }
  if (params->AdaptiveRounding)
  {
    setup_adaptive_rounding (params);
  }

  // structure for saving the coding state
  cs_mb  = create_coding_state ();
  cs_b8  = create_coding_state ();
  cs_cm  = create_coding_state ();
  cs_ib8 = create_coding_state ();
  cs_ib4 = create_coding_state ();

  if (params->CtxAdptLagrangeMult == 1)
  {
    mb16x16_cost = CALM_MF_FACTOR_THRESHOLD;
    lambda_mf_factor = 1.0;
  }

  getDistortion = distortionSSE;
}



/*!
 *************************************************************************************
 * \brief
 *    Updates the pixel map that shows, which reference frames are reliable for
 *    each MB-area of the picture.
 *
 * \note
 *    The new values of the pixel_map are taken from the temporary buffer refresh_map
 *
 *************************************************************************************
 */
void UpdatePixelMap()
{
  int mx,my,y,x,i,j;
  if (img->type==I_SLICE)
  {
    memset(pixel_map, 1, img->height * img->width * sizeof(byte));
  }
  else
  {
    for (my=0; my<img->height >> 3; my++)
      for (mx=0; mx<img->width >> 3;  mx++)
      {
        j = my*8 + 8;
        i = mx*8 + 8;
        if (refresh_map[my][mx])
        {
          for (y=my*8; y<j; y++)
            memset(&pixel_map[y][mx*8], 1, 8 * sizeof(byte));
        }
        else
        {
          for (y=my*8; y<j; y++)
            for (x=mx*8; x<i; x++)
            {
              pixel_map[y][x] = imin(pixel_map[y][x] + 1, params->num_ref_frames+1);
            }
        }
      }
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Checks if a given reference frame is reliable for the current
 *    macroblock, given the motion vectors that the motion search has
 *    returned.
 *
 * \return
 *    If the return value is 1, the reference frame is reliable. If it
 *    is 0, then it is not reliable.
 *
 * \note
 *    A specific area in each reference frame is assumed to be unreliable
 *    if the same area has been intra-refreshed in a subsequent frame.
 *    The information about intra-refreshed areas is kept in the pixel_map.
 *
 *************************************************************************************
 */
int CheckReliabilityOfRef (int block, int list_idx, int ref, int mode)
{
  int y,x, block_y, block_x, dy, dx, y_pos, x_pos, yy, xx, pres_x, pres_y;
  int maxold_x  = img->width-1;
  int maxold_y  = img->height-1;
  int ref_frame = ref + 1;

  int by0 = (mode>=4?2*(block >> 1):mode==2?2*block:0);
  int by1 = by0 + (mode>=4||mode==2?2:4);
  int bx0 = (mode>=4?2*(block & 0x01):mode==3?2*block:0);
  int bx1 = bx0 + (mode>=4||mode==3?2:4);

  for (block_y=by0; block_y<by1; block_y++)
  {
    for (block_x=bx0; block_x<bx1; block_x++)
    {
      y_pos  = img->all_mv[list_idx][ref][mode][block_y][block_x][1];
      y_pos += (img->block_y + block_y) * BLOCK_SIZE * 4;
      x_pos  = img->all_mv[list_idx][ref][mode][block_y][block_x][0];
      x_pos += (img->block_x + block_x) * BLOCK_SIZE * 4;

      /* Here we specify which pixels of the reference frame influence
      the reference values and check their reliability. This is
      based on the function Get_Reference_Pixel */

      dy = y_pos & 3;
      dx = x_pos & 3;

      y_pos = (y_pos - dy) >> 2;
      x_pos = (x_pos - dx) >> 2;

      if (dy==0 && dx==0) //full-pel
      {
        for (y=y_pos ; y < y_pos + BLOCK_SIZE ; y++)
          for (x=x_pos ; x < x_pos + BLOCK_SIZE ; x++)
            if (pixel_map[iClip3(0,maxold_y,y)][iClip3(0,maxold_x,x)] < ref_frame)
              return 0;
      }
      else  /* other positions */
      {
        if (dy == 0)
        {
          for (y = y_pos ; y < y_pos + BLOCK_SIZE ; y++)
          {
            pres_y = iClip3(0, maxold_y, y);
            for (x = x_pos ; x < x_pos + BLOCK_SIZE ; x++)
            {
              for(xx = -2 ; xx < 4 ; xx++) 
              {
                pres_x = iClip3(0, maxold_x, x + xx);
                if (pixel_map[pres_y][pres_x] < ref_frame)
                  return 0;
              }
            }
          }
        }
        else if (dx == 0)
        {
          for (y = y_pos ; y < y_pos + BLOCK_SIZE ; y++)
            for (x=x_pos ; x < x_pos + BLOCK_SIZE ; x++)
            {
              pres_x = iClip3(0,maxold_x,x);
              for(yy=-2;yy<4;yy++) 
              {
                pres_y = iClip3(0,maxold_y, yy + y);
                if (pixel_map[pres_y][pres_x] < ref_frame)
                  return 0;
              }
            }
        }
        else if (dx == 2)
        {
          for (y = y_pos ; y < y_pos + BLOCK_SIZE ; y++)
            for (x = x_pos ; x < x_pos + BLOCK_SIZE ; x++)
            {
              for(yy=-2;yy<4;yy++) 
              {
                pres_y = iClip3(0,maxold_y, yy + y);
                for(xx=-2;xx<4;xx++) 
                {
                  pres_x = iClip3(0,maxold_x, xx + x);
                  if (pixel_map[pres_y][pres_x] < ref_frame)
                    return 0;
                }
              }
            }
        }
        else if (dy == 2)
        {
          for (y = y_pos ; y < y_pos + BLOCK_SIZE ; y++)
            for (x = x_pos ; x < x_pos + BLOCK_SIZE ; x++)
            {
              for(xx=-2;xx<4;xx++) 
              {
                pres_x = iClip3(0,maxold_x, xx + x);
                for(yy=-2;yy<4;yy++) 
                {
                  pres_y = iClip3(0,maxold_y, yy + y);
                  if (pixel_map[pres_y][pres_x] < ref_frame)
                    return 0;
                }
              }
            }
        }
        else
        {
          for (y = y_pos ; y < y_pos + BLOCK_SIZE ; y++)
          {
            for (x = x_pos ; x < x_pos + BLOCK_SIZE ; x++)
            {
              pres_y = dy == 1 ? y : y + 1;
              pres_y = iClip3(0,maxold_y,pres_y);

              for(xx=-2;xx<4;xx++)
              {
                pres_x = iClip3(0,maxold_x,xx + x);
                if (pixel_map[pres_y][pres_x] < ref_frame)
                  return 0;
              }

              pres_x = dx == 1 ? x : x + 1;
              pres_x = iClip3(0,maxold_x,pres_x);

              for(yy=-2;yy<4;yy++)
              {
                pres_y = iClip3(0,maxold_y, yy + y);
                if (pixel_map[pres_y][pres_x] < ref_frame)
                  return 0;
              }
            }
          }
        }
      }
    }
  }
  return 1;
}



/*!
 *************************************************************************************
 * \brief
 *    R-D Cost for an 4x4 Intra block
 *************************************************************************************
 */
double RDCost_for_4x4IntraBlocks (Macroblock    *currMB,
                                  int*    nonzero,
                                  int     b8,
                                  int     b4,
                                  int     ipmode,
                                  double  lambda,
                                  int     mostProbableMode,
                                  int     c_nzCbCr[3],
                                  int     is_cavlc)
{
  double  rdcost;
  int     dummy = 0, rate;
  int64   distortion  = 0;
  int     block_x     = ((b8 & 0x01) << 3) + ((b4 & 0x01) << 2);
  int     block_y     = ((b8 >> 1) << 3) + ((b4 >> 1) << 2);
  int     pic_pix_x   = img->pix_x  + block_x;
  int     pic_pix_y   = img->pix_y  + block_y;
  int     pic_opix_y  = img->opix_y + block_y;

  Slice          *currSlice = img->currentSlice;
  SyntaxElement  se;
  const int      *partMap   = assignSE2partition[params->partition_mode];
  DataPartition  *dataPart;

  //===== perform DCT, Q, IQ, IDCT, Reconstruction =====
  //select_dct(img, currMB);
  *nonzero = pDCT_4x4 (currMB, PLANE_Y, block_x, block_y, &dummy, 1, is_cavlc);

  //===== get distortion (SSD) of 4x4 block =====
  distortion += compute_SSE(&pCurImg[pic_opix_y], &enc_picture->imgY[pic_pix_y], pic_pix_x, pic_pix_x, 4, 4);

  if(img->P444_joined)
  {
    ColorPlane k;
    for (k = PLANE_U; k <= PLANE_V; k++)
    {
      select_plane(k);
      c_nzCbCr[k] = pDCT_4x4(currMB, k, block_x, block_y, &dummy, 1, is_cavlc);
      distortion += compute_SSE(&pCurImg[pic_opix_y], &enc_picture->p_curr_img[pic_pix_y], pic_pix_x, pic_pix_x, 4, 4);
    }
    select_plane(PLANE_Y);
  }
  ipmode_DPCM=NO_INTRA_PMODE;

  //===== RATE for INTRA PREDICTION MODE  (SYMBOL MODE MUST BE SET TO CAVLC) =====
  se.value1 = (mostProbableMode == ipmode) ? -1 : ipmode < mostProbableMode ? ipmode : ipmode - 1;

  //--- set position and type ---
  se.context = (b8 << 2) + b4;
  se.type    = SE_INTRAPREDMODE;

  //--- choose data partition ---
  dataPart = &(currSlice->partArr[partMap[SE_INTRAPREDMODE]]);
  //--- encode and update rate ---
  writeIntraPredMode (&se, dataPart);
  rate = se.len;

  //===== RATE for LUMINANCE COEFFICIENTS =====
  if (is_cavlc)
  {
    rate  += writeCoeff4x4_CAVLC (currMB, LUMA, b8, b4, 0);
    if(img->P444_joined) 
    {
      rate  += writeCoeff4x4_CAVLC (currMB, CB, b8, b4, 0);
      rate  += writeCoeff4x4_CAVLC (currMB, CR, b8, b4, 0);
    }
  }
  else
  {
    rate  += writeCoeff4x4_CABAC (currMB, PLANE_Y, b8, b4, 1);
    if(img->P444_joined) 
    {
      rate  += writeCoeff4x4_CABAC (currMB, PLANE_U, b8, b4, 1);
      rate  += writeCoeff4x4_CABAC (currMB, PLANE_V, b8, b4, 1);
    }
  }
  //reset_coding_state (currMB, cs_cm);
  rdcost = (double)distortion + lambda * (double) rate;

  return rdcost;
}

/*!
*************************************************************************************
* \brief
*    R-D Cost for an 8x8 Partition
*************************************************************************************
*/
double RDCost_for_8x8blocks (Macroblock *currMB, // --> Current macroblock to code
                             int*    cnt_nonz,   // --> number of nonzero coefficients
                             int64*  cbp_blk,    // --> cbp blk
                             double  lambda,     // <-- lagrange multiplier
                             int     block,      // <-- 8x8 block number
                             int     mode,       // <-- partitioning mode
                             short   pdir,       // <-- prediction direction
                             short   l0_ref,     // <-- L0 reference picture
                             short   l1_ref,     // <-- L1 reference picture
                             short   bipred_me,  // <-- bi prediction mode
                             int     is_cavlc
                             )     
{
  int  k;
  int  rate=0;
  int64 distortion=0;
  int  dummy = 0, mrate;
  int  fw_mode, bw_mode;
  int  cbp     = 0;
  int  pax     = 8*(block & 0x01);
  int  pay     = 8*(block >> 1);
  int  i0      = pax >> 2;
  int  j0      = pay >> 2;
  int  bframe  = (img->type==B_SLICE);
  int  direct  = (bframe && mode==0);
  int  b8value = B8Mode2Value (mode, pdir);

  SyntaxElement se;
  Slice         *currSlice = img->currentSlice;
  DataPartition *dataPart;
  const int     *partMap   = assignSE2partition[params->partition_mode];

  EncodingEnvironmentPtr eep_dp;

  //=====
  //=====  GET COEFFICIENTS, RECONSTRUCTIONS, CBP
  //=====
  currMB->bipred_me[block] = bipred_me;
  
  if (direct)
  {
    if (direct_pdir[img->block_y+j0][img->block_x+i0]<0) // mode not allowed
      return (1e20);
    else
      *cnt_nonz = LumaResidualCoding8x8 (currMB, &cbp, cbp_blk, block, direct_pdir[img->block_y+j0][img->block_x+i0], 0, 0,
      (short)imax(0,direct_ref_idx[LIST_0][img->block_y+j0][img->block_x+i0]),
      direct_ref_idx[LIST_1][img->block_y+j0][img->block_x+i0], is_cavlc);
  }
  else
  {
    if (pdir == 2 && active_pps->weighted_bipred_idc == 1)
    {
      int weight_sum = (active_pps->weighted_bipred_idc == 1)? wbp_weight[0][l0_ref][l1_ref][0] + wbp_weight[1][l0_ref][l1_ref][0] : 0;
      if (weight_sum < -128 ||  weight_sum > 127)
      {
        return (1e20);
      }
    }

    fw_mode   = (pdir==0||pdir==2 ? mode : 0);
    bw_mode   = (pdir==1||pdir==2 ? mode : 0);
    *cnt_nonz = LumaResidualCoding8x8 (currMB, &cbp, cbp_blk, block, pdir, fw_mode, bw_mode, l0_ref, l1_ref, is_cavlc);
  }

  if(img->P444_joined) 
  {
    *cnt_nonz += ( coeff_cost_cr[1] + coeff_cost_cr[2] );
  }

  // RDOPT with losses
  if (params->rdopt==3 && img->type!=B_SLICE)
  {
    //===== get residue =====
    // We need the reconstructed prediction residue for the simulated decoders.
    //compute_residue_b8block (img, &enc_picture->p_img[0][img->pix_y], decs->res_img[0], block, -1);
    compute_residue_block (img, &enc_picture->p_img[0][img->pix_y], decs->res_img[0], img->mb_pred[0], block, 8);

    //=====
    //=====   GET DISTORTION
    //=====
    for (k=0; k<params->NoOfDecoders ;k++)
    {
      decode_one_b8block (img, enc_picture, k, P8x8, block, mode, l0_ref);
      distortion += compute_SSE(&pCurImg[img->opix_y+pay], &enc_picture->p_dec_img[0][k][img->opix_y+pay], img->opix_x+pax, img->opix_x+pax, 8, 8);
    }
    distortion /= params->NoOfDecoders;
  }
  else
  {    
    distortion += compute_SSE(&pCurImg[img->opix_y + pay], &enc_picture->imgY[img->pix_y + pay], img->opix_x + pax, img->pix_x + pax, 8, 8);

    if (img->P444_joined)
    {
      distortion += compute_SSE(&pImgOrg[1][img->opix_y + pay], &enc_picture->imgUV[0][img->pix_y + pay], img->opix_x + pax, img->pix_x + pax, 8, 8);
      distortion += compute_SSE(&pImgOrg[2][img->opix_y + pay], &enc_picture->imgUV[1][img->pix_y + pay], img->opix_x + pax, img->pix_x + pax, 8, 8);
    }    
  }

  if(img->P444_joined) 
  {   
    cbp |= cmp_cbp[1];
    cbp |= cmp_cbp[2];

    cmp_cbp[1] = cbp;
    cmp_cbp[2] = cbp;
  }

  //=====
  //=====   GET RATE
  //=====
  //----- block 8x8 mode -----
  if (is_cavlc)
  {
    ue_linfo (b8value, dummy, &mrate, &dummy);
    rate += mrate;
  }
  else
  {
    se.value1 = b8value;
    se.type   = SE_MBTYPE;
    dataPart  = &(currSlice->partArr[partMap[se.type]]);
    writeB8_typeInfo(&se, dataPart);
    rate += se.len;
  }

  //----- motion information -----
  if (!direct)
  {
    if ((img->num_ref_idx_l0_active > 1 ) && (pdir==0 || pdir==2))
      rate  += writeReferenceFrame (currMB, mode, i0, j0, 1, l0_ref);

    if ((img->num_ref_idx_l1_active > 1 && img->type== B_SLICE ) && (pdir==1 || pdir==2))
    {
      rate  += writeReferenceFrame (currMB, mode, i0, j0, 0, l1_ref);
    }

    if (pdir==0 || pdir==2)
    {
      rate  += writeMotionVector8x8 (currMB, i0, j0, i0 + 2, j0 + 2, l0_ref, LIST_0, mode, currMB->bipred_me[block]);
    }
    if (pdir==1 || pdir==2)
    {
      rate  += writeMotionVector8x8 (currMB, i0, j0, i0 + 2, j0 + 2, l1_ref, LIST_1, mode, currMB->bipred_me[block]);
    }
  }

  //----- coded block pattern (for CABAC only) -----
  if (!is_cavlc)
  {
    dataPart = &(currSlice->partArr[partMap[SE_CBP]]);
    eep_dp   = &(dataPart->ee_cabac);
    mrate    = arienco_bits_written (eep_dp);
    writeCBP_BIT_CABAC (currMB, block, ((*cnt_nonz>0)?1:0), cbp8x8, 1, eep_dp, img->currentSlice->tex_ctx);
    mrate    = arienco_bits_written (eep_dp) - mrate;
    rate    += mrate;
  }

  //----- luminance coefficients -----
  if (*cnt_nonz)
  {
    rate += writeCoeff8x8 (currMB, PLANE_Y, block, mode, currMB->luma_transform_size_8x8_flag);
  }

  if(img->P444_joined)
  {
    rate += writeCoeff8x8( currMB, PLANE_U, block, mode, currMB->luma_transform_size_8x8_flag );
    rate += writeCoeff8x8( currMB, PLANE_V, block, mode, currMB->luma_transform_size_8x8_flag );
  }

  return (double)distortion + lambda * (double)rate;
}


/*!
 *************************************************************************************
 * \brief
 *    Gets mode offset for intra16x16 mode
 *************************************************************************************
 */
int I16Offset (int cbp, int i16mode)
{
  return (cbp&15?13:1) + i16mode + ((cbp&0x30)>>2);
}


/*!
 *************************************************************************************
 * \brief
 *    Sets modes and reference frames for a macroblock
 *************************************************************************************
 */
void SetModesAndRefframeForBlocks (Macroblock *currMB, int mode)
{
  int i,j,k,l; 
  int  bframe  = (img->type==B_SLICE);
  int  block_x, block_y, block8x8, block4x4;
  int  cur_ref;
  int  clist;
  char cref[2], curref, bestref;
  Block8x8Info *b8x8info = img->b8x8info;

  //--- macroblock type ---
  currMB->mb_type = mode;
  
   for( i = 0; i < 4; i++) 
   {
     currMB->bipred_me[i] = b8x8info->bipred8x8me[mode][i];
   }

  //--- block 8x8 mode and prediction direction ---
  switch (mode)
  {
  case 0:
    memset(currMB->b8mode, 0, 4 * sizeof(short));
    if (bframe)
    {      
      for(i=0;i<4;i++)
      {
        currMB->b8pdir[i] = direct_pdir[img->block_y + ((i >> 1)<<1)][img->block_x + ((i & 0x01)<<1)];
      }
    }
    else
    {
      memset(currMB->b8pdir, 0, 4 * sizeof(short));
    }
    break;
  case 1:
  case 2:
  case 3:
    for(i=0;i<4;i++)
    {
      currMB->b8mode[i] = mode;
      currMB->b8pdir[i] = b8x8info->best8x8pdir[mode][i];
    }
    break;
  case P8x8:
    memcpy(currMB->b8mode, b8x8info->best8x8mode, 4 * sizeof(short));
    for(i=0;i<4;i++)
    {
      currMB->b8pdir[i]   = b8x8info->best8x8pdir[mode][i];
    }
    break;
  case I4MB:
    for(i=0;i<4;i++)
    {
      currMB->b8mode[i] = IBLOCK;
      currMB->b8pdir[i] = -1;
    }
    break;
  case I16MB:
    memset(currMB->b8mode, 0, 4 * sizeof(short));
    for(i=0;i<4;i++)
    {
      currMB->b8pdir[i] = -1;
    }
    break;
  case I8MB:
    for(i=0;i<4;i++)
    {
      currMB->b8mode[i] = I8MB;
      currMB->b8pdir[i] = -1;
    }
    //switch to 8x8 transform
    currMB->luma_transform_size_8x8_flag = 1;
    break;
  case IPCM:
    for(i=0;i<4;i++)
    {
      currMB->b8mode[i] = IPCM;
      currMB->b8pdir[i] = -1;
    }
    currMB->luma_transform_size_8x8_flag = 0;
    break;
  default:
    printf ("Unsupported mode in SetModesAndRefframeForBlocks!\n");
    exit (1);
  }

#define IS_FW ((b8x8info->best8x8pdir[mode][k]==0 || b8x8info->best8x8pdir[mode][k]==2) && (mode!=P8x8 || b8x8info->best8x8mode[k]!=0 || !bframe))
#define IS_BW ((b8x8info->best8x8pdir[mode][k]==1 || b8x8info->best8x8pdir[mode][k]==2) && (mode!=P8x8 || b8x8info->best8x8mode[k]!=0))
  
  //--- reference frame arrays ---
  if (mode==0 || mode==I4MB || mode==I16MB || mode==I8MB)
  {
    if (bframe)
    {
      if (!mode) // Direct
      {
        for (clist = LIST_0; clist <= LIST_1; clist++)
        {
          for (j = img->block_y; j < img->block_y + 4; j++)
          {
            memcpy(&enc_picture->motion.ref_idx[clist][j][img->block_x], &direct_ref_idx[clist][j][img->block_x], 4 * sizeof(char));
          }
        }
      }
      else // Intra
      {
        for (clist = LIST_0; clist <= LIST_1; clist++)
        {
          for (j = img->block_y; j < img->block_y + 4; j++)
          {
            memset(&enc_picture->motion.ref_idx[clist][j][img->block_x],-1, 4 * sizeof(char));
          }
        }
      }
    }
    else
    {
      if (!mode) // Skip
      {
        for (j = img->block_y; j < img->block_y + 4; j++)
          memset(&enc_picture->motion.ref_idx[LIST_0][j][img->block_x],0, 4 * sizeof(char));
      }
      else // Intra
      {
        for (j = img->block_y; j < img->block_y + 4; j++)
          memset(&enc_picture->motion.ref_idx[LIST_0][j][img->block_x],-1, 4 * sizeof(char));
      }
    }
  }
  else
  {
    if (bframe)
    {
      if (mode == 1 || mode == 2 || mode == 3) 
      {
        for (block8x8 = 0; block8x8 < 4; block8x8++)
        {
          for (clist = LIST_0; clist <= LIST_1; clist++)
          {
            bestref = (clist == LIST_0) ? b8x8info->best8x8l0ref[mode][block8x8] :  b8x8info->best8x8l1ref[mode][block8x8];
            if ( b8x8info->best8x8pdir[mode][block8x8] == 2)
            {
              if (b8x8info->bipred8x8me[mode][block8x8])
                curref = 0;
              else  
                curref = bestref;
            }
            else
            {
              curref = (clist == b8x8info->best8x8pdir[mode][block8x8]) ? bestref : -1;
            }
            for (block4x4 = 0; block4x4 < 4; block4x4++)
            {
              block_x = img->block_x + 2 * (block8x8 & 0x01) + (block4x4 & 0x01);
              block_y = img->block_y + 2 * (block8x8 >> 1) + (block4x4 >> 1);
              enc_picture->motion.ref_idx[clist][block_y][block_x] = curref;
            }
          }
        }
      }
      else
      {
        for (j=0;j<4;j++)
        {
          block_y = img->block_y + j;
          for (i=0;i<4;i++)
          {
            block_x = img->block_x + i;
            k = 2*(j >> 1) + (i >> 1);
            l = 2*(j & 0x01) + (i & 0x01);

            if(mode == P8x8 && b8x8info->best8x8mode[k]==0)
            {
              enc_picture->motion.ref_idx[LIST_0][block_y][block_x] = direct_ref_idx[LIST_0][block_y][block_x];
              enc_picture->motion.ref_idx[LIST_1][block_y][block_x] = direct_ref_idx[LIST_1][block_y][block_x];
            }
            else
            {
              enc_picture->motion.ref_idx[LIST_0][block_y][block_x] = (IS_FW ? b8x8info->best8x8l0ref[mode][k] : -1);
              enc_picture->motion.ref_idx[LIST_1][block_y][block_x] = (IS_BW ? b8x8info->best8x8l1ref[mode][k] : -1);
            }
          }        
        }
      }
    }
    else
    {
      if (mode == 1)
      {
        cref[0] = b8x8info->best8x8pdir[mode][0] == 0 ? b8x8info->best8x8l0ref[mode][0] : -1;
        j = img->block_y;
        memset(&enc_picture->motion.ref_idx[LIST_0][j++][img->block_x], cref[0], 4 * sizeof(char));
        memset(&enc_picture->motion.ref_idx[LIST_0][j++][img->block_x], cref[0], 4 * sizeof(char));
        memset(&enc_picture->motion.ref_idx[LIST_0][j++][img->block_x], cref[0], 4 * sizeof(char));
        memset(&enc_picture->motion.ref_idx[LIST_0][j  ][img->block_x], cref[0], 4 * sizeof(char));
      }
      else if (mode == 2)
      {
        cref[0] = b8x8info->best8x8pdir[mode][0] == 0 ? b8x8info->best8x8l0ref[mode][0] : -1;
        j = img->block_y;
        memset(&enc_picture->motion.ref_idx[LIST_0][j++][img->block_x], cref[0], 4 * sizeof(char));
        memset(&enc_picture->motion.ref_idx[LIST_0][j++][img->block_x], cref[0], 4 * sizeof(char));
        cref[0] = b8x8info->best8x8pdir[mode][2] == 0 ? b8x8info->best8x8l0ref[mode][2] : -1;
        memset(&enc_picture->motion.ref_idx[LIST_0][j++][img->block_x], cref[0], 4 * sizeof(char));
        memset(&enc_picture->motion.ref_idx[LIST_0][j  ][img->block_x], cref[0], 4 * sizeof(char));
      }      
      else if (mode == 3)
      {
        j = img->block_y;
        i = img->block_x;
        cref[0] = (b8x8info->best8x8pdir[mode][0] == 0) ? b8x8info->best8x8l0ref[mode][0] : -1;
        enc_picture->motion.ref_idx[LIST_0][j  ][i++] = cref[0];
        enc_picture->motion.ref_idx[LIST_0][j  ][i++] = cref[0];
        cref[0] = (b8x8info->best8x8pdir[mode][1] == 0) ? b8x8info->best8x8l0ref[mode][1] : -1;
        enc_picture->motion.ref_idx[LIST_0][j  ][i++] = cref[0];
        enc_picture->motion.ref_idx[LIST_0][j++][i  ] = cref[0];
        memcpy(&enc_picture->motion.ref_idx[LIST_0][j++][img->block_x], &enc_picture->motion.ref_idx[LIST_0][img->block_y][img->block_x], 4 * sizeof(char));
        memcpy(&enc_picture->motion.ref_idx[LIST_0][j++][img->block_x], &enc_picture->motion.ref_idx[LIST_0][img->block_y][img->block_x], 4 * sizeof(char));
        memcpy(&enc_picture->motion.ref_idx[LIST_0][j  ][img->block_x], &enc_picture->motion.ref_idx[LIST_0][img->block_y][img->block_x], 4 * sizeof(char));
      }      
      else
      {
        for (j=0;j<4;j++)
        {
          block_y = img->block_y + j;
          for (i=0;i<4;i++)
          {
            block_x = img->block_x + i;
            k = 2*(j >> 1) + (i >> 1);
            l = 2*(j & 0x01) + (i & 0x01);
            enc_picture->motion.ref_idx[LIST_0][block_y][block_x] = (IS_FW ? b8x8info->best8x8l0ref[mode][k] : -1);
          }
        }
      }
    }
  }

  if (bframe)
  {
    for (clist = LIST_0; clist <= LIST_1; clist++)
    {
      for (j = img->block_y; j < img->block_y + 4; j++)
        for (i = img->block_x; i < img->block_x + 4;i++)
        {
          cur_ref = (int) enc_picture->motion.ref_idx[clist][j][i];
          enc_picture->motion.ref_pic_id [clist][j][i] = (cur_ref>=0
            ? enc_picture->ref_pic_num[clist + currMB->list_offset][cur_ref]
            : -1);
        }
    }
  }
  else
  {
    for (j = img->block_y; j < img->block_y + 4; j++)
      for (i = img->block_x; i < img->block_x + 4;i++)
      {
        cur_ref = (int) enc_picture->motion.ref_idx[LIST_0][j][i];
        enc_picture->motion.ref_pic_id [LIST_0][j][i] = (cur_ref>=0
          ? enc_picture->ref_pic_num[LIST_0 + currMB->list_offset][cur_ref]
          : -1);
      }
  }

#undef IS_FW
#undef IS_BW
}





/*!
 *************************************************************************************
 * \brief
 *    Sets Coefficients and reconstruction for an 8x8 block
 *************************************************************************************
 */
void SetCoeffAndReconstruction8x8 (Macroblock* currMB)
{
  int block, k, j, i, uv;
  int cur_ref;

  //============= MIXED TRANSFORM SIZES FOR 8x8 PARTITION ==============
  //--------------------------------------------------------------------
  int l;
  int bframe = img->type==B_SLICE;

  if (currMB->luma_transform_size_8x8_flag)
  {
    //============= set mode and ref. frames ==============
    for(i = 0;i<4;i++)
    {
      currMB->b8mode[i]    = tr8x8.part8x8mode[i];
      currMB->b8pdir[i]    = tr8x8.part8x8pdir[i];
      currMB->bipred_me[i] = tr8x8.part8x8bipred[i];
     }

    if (bframe)
    {
      for (j = 0;j<4;j++)
      {
        for (i = 0;i<4;i++)
        {
          k = 2*(j >> 1)+(i >> 1);
          l = 2*(j & 0x01)+(i & 0x01);
          enc_picture->motion.ref_idx[LIST_0][img->block_y+j][img->block_x+i] = ((currMB->b8pdir[k] & 0x01) == 0) ? tr8x8.part8x8l0ref[k] : - 1;
          enc_picture->motion.ref_idx[LIST_1][img->block_y+j][img->block_x+i] =  (currMB->b8pdir[k] > 0)          ? tr8x8.part8x8l1ref[k] : - 1;
        }
      }
    }
    else
    {
      for (j = 0;j<4;j++)
      {
        for (i = 0;i<4;i++)
        {
          k = 2*(j >> 1)+(i >> 1);
          l = 2*(j & 0x01)+(i & 0x01);
          enc_picture->motion.ref_idx[LIST_0][img->block_y+j][img->block_x+i] = tr8x8.part8x8l0ref[k];
        }
      }
    }


    for (j = img->block_y;j<img->block_y + BLOCK_MULTIPLE;j++)
    {
      for (i = img->block_x;i<img->block_x + BLOCK_MULTIPLE;i++)
      {
        cur_ref = (int) enc_picture->motion.ref_idx[LIST_0][j][i];

        enc_picture->motion.ref_pic_id [LIST_0][j][i] =(cur_ref>=0
          ? enc_picture->ref_pic_num[LIST_0 + currMB->list_offset][cur_ref]
          : -1);
      }
    }

    if (bframe)
    {
      for (j = img->block_y; j < img->block_y + BLOCK_MULTIPLE; j++)
      {
        for (i = img->block_x;i<img->block_x + BLOCK_MULTIPLE;i++)
        {
          cur_ref = (int) enc_picture->motion.ref_idx[LIST_1][j][i];

          enc_picture->motion.ref_pic_id [LIST_1][j][i] = (cur_ref>=0
            ? enc_picture->ref_pic_num[LIST_1 + currMB->list_offset][cur_ref]
            : -1);
        }

      }
    }

    //====== set the mv's for 8x8 partition with transform size 8x8 ======
    //save the mv data for 4x4 transform

    StoreMV8x8(1);
    //set new mv data for 8x8 transform
    RestoreMV8x8(0);

    //============= get pre-calculated data ==============
    //restore coefficients from 8x8 transform

    memcpy (img->cofAC[0][0][0],cofAC8x8ts[0][0][0][0], 4 * 4 * 2 * 65 * sizeof(int));

    if (img->P444_joined)
    {
      for (uv=0; uv<2; uv++)
      {
        for (block = 0; block<4; block++)
        {
          memcpy (img->cofAC[4+block+uv*4][0][0],cofAC8x8ts[uv + 1][block][0][0], 4 * 2 * 65 * sizeof(int));
        }
      }
    }
    //restore reconstruction
    if (cnt_nonz8_8x8ts <= _LUMA_8x8_COEFF_COST_ &&
      ((currMB->qp_scaled[0])!=0 || img->lossless_qpprime_flag==0) &&
      (img->type!=SP_SLICE))// modif ES added last condition (we probably never go there so is the next modification useful ? check)
    {
      currMB->cbp     = 0;
      currMB->cbp_blk = 0;

      for (j = 0; j < MB_BLOCK_SIZE; j++)
      {
        memcpy(&enc_picture->imgY[img->pix_y+j][img->pix_x], tr8x8.mpr8x8[j], MB_BLOCK_SIZE * sizeof(imgpel));
      }
      if(img->type==SP_SLICE &&(!si_frame_indicator && !sp2_frame_indicator ))
      {
        for (j = 0; j < MB_BLOCK_SIZE; j++)
        {
          memcpy(&lrec[img->pix_y+j][img->pix_x],tr8x8.lrec[j], MB_BLOCK_SIZE * sizeof(int));
        }
      }

      memset( img->cofAC[0][0][0], 0, 4 * 4 * 2 * 65 * sizeof(int));

      if(img->P444_joined)
      {
        for (j = 0; j < MB_BLOCK_SIZE; j++)
        {
          memcpy(&enc_picture->imgUV[0][img->pix_y+j][img->pix_x], tr8x8.mpr8x8CbCr[0][j], MB_BLOCK_SIZE * sizeof(imgpel));
          memcpy(&enc_picture->imgUV[1][img->pix_y+j][img->pix_x], tr8x8.mpr8x8CbCr[1][j], MB_BLOCK_SIZE * sizeof(imgpel));
        }
        for (uv=0; uv<2; uv++)
        {
          for (block = 0; block<4; block++)
          {
             memset( img->cofAC[4+block+uv*4][0][0], 0, 4 * 2 * 65 * sizeof(int));
          }
        }
      }
    }
    else
    {
      currMB->cbp     = cbp8_8x8ts;
      currMB->cbp_blk = cbp_blk8_8x8ts;
      for (j = 0; j < MB_BLOCK_SIZE; j++)
      {
        memcpy (&enc_picture->imgY[img->pix_y+j][img->pix_x],tr8x8.rec_mbY8x8[j], MB_BLOCK_SIZE * sizeof(imgpel));
      }

      if(img->type==SP_SLICE &&(!si_frame_indicator && !sp2_frame_indicator))
      {
        for (j = 0; j < MB_BLOCK_SIZE; j++)
        {
          memcpy (&lrec[img->pix_y+j][img->pix_x],tr8x8.lrec[j], MB_BLOCK_SIZE * sizeof(int));
        }
      }

      if (img->P444_joined) 
      {
        cmp_cbp[1] = cmp_cbp[2] = cbp8_8x8ts;
        for (j = 0; j < MB_BLOCK_SIZE; j++)
        {
          memcpy (&enc_picture->imgUV[0][img->pix_y+j][img->pix_x],tr8x8.rec_mb8x8_cr[0][j], MB_BLOCK_SIZE * sizeof(imgpel)); 
          memcpy (&enc_picture->imgUV[1][img->pix_y+j][img->pix_x],tr8x8.rec_mb8x8_cr[1][j], MB_BLOCK_SIZE * sizeof(imgpel)); 
        }
      }
    }
  }
  else
  {
    //============= get pre-calculated data ==============
    //---------------------------------------------------
    //--- restore coefficients ---
    memcpy (img->cofAC[0][0][0],cofAC8x8[0][0][0], (4+img->num_blk8x8_uv) * 4 * 2 * 65 * sizeof(int));

    if (img->P444_joined) 
    {
      for (block = 0; block<4; block++)
      {
        memcpy (img->cofAC[block+4][0][0],cofAC8x8CbCr[0][block][0][0], 4 * 2 * 65 * sizeof(int));     
        memcpy (img->cofAC[block+8][0][0],cofAC8x8CbCr[1][block][0][0], 4 * 2 * 65 * sizeof(int));   
      }
    }

    if (cnt_nonz_8x8<=5 && img->type!=SP_SLICE &&
      ((currMB->qp_scaled[0])!=0 || img->lossless_qpprime_flag==0))
    {
      currMB->cbp     = 0;
      currMB->cbp_blk = 0;
      for (j = 0; j < MB_BLOCK_SIZE; j++)
      {
        memcpy (&enc_picture->imgY[img->pix_y+j][img->pix_x],tr4x4.mpr8x8[j], MB_BLOCK_SIZE * sizeof(imgpel));
      }
      if(img->type ==SP_SLICE &&(!si_frame_indicator && !sp2_frame_indicator))
      {
        for (j = 0; j < MB_BLOCK_SIZE; j++)
        {
          memcpy (&lrec[img->pix_y+j][img->pix_x],tr4x4.lrec[j], MB_BLOCK_SIZE * sizeof(int)); // restore coeff. SP frame
        }
      }

      memset( img->cofAC[0][0][0], 0, 4 * 4 * 2 * 65 * sizeof(int));

      if (img->P444_joined)
      {
        for (j = 0; j < MB_BLOCK_SIZE; j++)
        {
          memcpy (&enc_picture->imgUV[0][img->pix_y+j][img->pix_x],tr4x4.mpr8x8CbCr[0][j], MB_BLOCK_SIZE * sizeof(imgpel));    
          memcpy (&enc_picture->imgUV[1][img->pix_y+j][img->pix_x],tr4x4.mpr8x8CbCr[1][j], MB_BLOCK_SIZE * sizeof(imgpel));  
        }
        for (uv=0; uv<2; uv++)
        {
          for (block = 0; block<4; block++)
          {
            memset( img->cofAC[4+block+uv*4][0][0], 0, 4 * 2 * 65 * sizeof(int));
          }
        }
      }
    }
    else
    {
      currMB->cbp     = cbp8x8;
      currMB->cbp_blk = cbp_blk8x8;
      for (j = 0; j < MB_BLOCK_SIZE; j++)
      {
        memcpy (&enc_picture->imgY[img->pix_y+j][img->pix_x],tr4x4.rec_mbY8x8[j], MB_BLOCK_SIZE * sizeof(imgpel));
      }

      if (params->rdopt == 3)
      {
        errdo_get_best_block(img, enc_picture->p_dec_img[0], decs->dec_mbY8x8, 0, MB_BLOCK_SIZE);
      }

      if(img->type==SP_SLICE &&(!si_frame_indicator && !sp2_frame_indicator))
      {
        for (j = 0; j < MB_BLOCK_SIZE; j++)
        {
          memcpy (&lrec[img->pix_y+j][img->pix_x],tr4x4.lrec[j], MB_BLOCK_SIZE * sizeof(int));
        }
      }
      if (img->P444_joined)
      {
        cmp_cbp[1] = cmp_cbp[2] = cbp8x8;
        for (j = 0; j < MB_BLOCK_SIZE; j++)
        {
          memcpy (&enc_picture->imgUV[0][img->pix_y+j][img->pix_x],tr4x4.rec_mb8x8_cr[0][j], MB_BLOCK_SIZE * sizeof(imgpel));
          memcpy (&enc_picture->imgUV[1][img->pix_y+j][img->pix_x],tr4x4.rec_mb8x8_cr[1][j], MB_BLOCK_SIZE * sizeof(imgpel));
        }
      }
    }
  }
}


/*!
 *************************************************************************************
 * \brief
 *    Sets motion vectors for a macroblock
 *************************************************************************************
 */
void SetMotionVectorsMB (Macroblock* currMB, int bframe)
{
  int i, j, k, l, mode8, pdir8, ref, by, bx;
  short ******all_mv  = img->all_mv;
  short ******pred_mv = img->pred_mv;
  short bipred_me8;
  int  l1_ref;
  int jdiv, jmod;

  // copy all the motion vectors into rdopt structure
  // Can simplify this by copying the MV's of the best mode (TBD)
  // Should maybe add code to check for Intra only profiles
  if (img->MbaffFrameFlag || (params->UseRDOQuant && params->RDOQ_QP_Num > 1))
  {
    memcpy(&rdopt->pred_mv [0][0][0][0][0][0], &pred_mv [0][0][0][0][0][0], 2 * img->max_num_references * 9 * 4 * 4 * 2 * sizeof(short));
    memcpy(&rdopt->all_mv  [0][0][0][0][0][0], &all_mv  [0][0][0][0][0][0], 2 * img->max_num_references * 9 * 4 * 4 * 2 * sizeof(short));
  }

  if (!bframe)
  {
    for (j = 0; j<4; j++)
    {
      jmod = j & 0x01;
      jdiv = j >>   1;
      by    = img->block_y+j;
      for (i = 0; i<4; i++)
      {
        mode8 = currMB->b8mode[k=2*jdiv+(i>>1)];
        l     = 2*jmod + (i & 0x01);

        bx   = img->block_x+i;

        pdir8 = currMB->b8pdir[k];
        ref    = enc_picture->motion.ref_idx[LIST_0][by][bx];

        if (pdir8>=0)
        {
          enc_picture->motion.mv[LIST_0][by][bx][0] = all_mv [LIST_0][ ref][mode8][j][i][0];
          enc_picture->motion.mv[LIST_0][by][bx][1] = all_mv [LIST_0][ ref][mode8][j][i][1];
        }
        else
        {
          enc_picture->motion.mv[LIST_0][by][bx][0] = 0;
          enc_picture->motion.mv[LIST_0][by][bx][1] = 0;
        }
      }
    }
  }
  else
  {
    for (j = 0; j<4; j++)
    {
      jmod = j & 0x01;
      jdiv = j >>   1;
      by    = img->block_y+j;
      for (i = 0; i<4; i++)
      {
        mode8 = currMB->b8mode[k=2*jdiv+(i>>1)];
        bipred_me8  = currMB->bipred_me[k]; 
        l     = 2*jmod + (i & 0x01);

        bx    = img->block_x+i;

        pdir8 = currMB->b8pdir[k];
        ref    = enc_picture->motion.ref_idx[LIST_0][by][bx];
        l1_ref = enc_picture->motion.ref_idx[LIST_1][by][bx];

        if ( bipred_me8 && (pdir8 == 2) && is_bipred_enabled(currMB->mb_type))
        {
          all_mv  = img->bipred_mv[bipred_me8 - 1]; 
          ref = 0;
          l1_ref = 0;
        }
        else
        {
          all_mv  = img->all_mv;
        }

        if (pdir8==-1) // intra
        {
          enc_picture->motion.mv[LIST_0][by][bx][0] = 0;
          enc_picture->motion.mv[LIST_0][by][bx][1] = 0;
          enc_picture->motion.mv[LIST_1][by][bx][0] = 0;
          enc_picture->motion.mv[LIST_1][by][bx][1] = 0;
        }
        else if (pdir8==0) // list 0
        {
          enc_picture->motion.mv[LIST_0][by][bx][0]   = all_mv [LIST_0][ ref][mode8][j][i][0];
          enc_picture->motion.mv[LIST_0][by][bx][1]   = all_mv [LIST_0][ ref][mode8][j][i][1];
          enc_picture->motion.mv[LIST_1][by][bx][0]   = 0;
          enc_picture->motion.mv[LIST_1][by][bx][1]   = 0;
          enc_picture->motion.ref_idx[LIST_1][by][bx] = -1;
        }
        else if (pdir8==1) // list 1
        {
          enc_picture->motion.mv[LIST_0][by][bx][0]   = 0;
          enc_picture->motion.mv[LIST_0][by][bx][1]   = 0;
          enc_picture->motion.ref_idx[LIST_0][by][bx] = -1;
          enc_picture->motion.mv[LIST_1][by][bx][0]   = all_mv [LIST_1][l1_ref][mode8][j][i][0];
          enc_picture->motion.mv[LIST_1][by][bx][1]   = all_mv [LIST_1][l1_ref][mode8][j][i][1];
        }
        else if (pdir8==2) // bipredictive
        {
          enc_picture->motion.mv[LIST_0][by][bx][0] = all_mv [LIST_0][   ref][mode8][j][i][0];
          enc_picture->motion.mv[LIST_0][by][bx][1] = all_mv [LIST_0][   ref][mode8][j][i][1];
          enc_picture->motion.mv[LIST_1][by][bx][0] = all_mv [LIST_1][l1_ref][mode8][j][i][0];
          enc_picture->motion.mv[LIST_1][by][bx][1] = all_mv [LIST_1][l1_ref][mode8][j][i][1];

          // copy all the motion vectors into rdopt structure
          // Can simplify this by copying the MV's of the best mode (TBD)
          // Should maybe add code to check for Intra only profiles
          if (img->MbaffFrameFlag || (params->UseRDOQuant && params->RDOQ_QP_Num > 1))
          {
            rdopt->all_mv [LIST_0][   ref][mode8][j][i][0] = all_mv [LIST_0][   ref][mode8][j][i][0];
            rdopt->all_mv [LIST_0][   ref][mode8][j][i][1] = all_mv [LIST_0][   ref][mode8][j][i][1];
            rdopt->all_mv [LIST_1][l1_ref][mode8][j][i][0] = all_mv [LIST_1][l1_ref][mode8][j][i][0];
            rdopt->all_mv [LIST_1][l1_ref][mode8][j][i][1] = all_mv [LIST_1][l1_ref][mode8][j][i][1];
          }
        }
        else
        {
          error("invalid direction mode", 255);
        }
      }
    }
  }
}



/*!
 *************************************************************************************
 * \brief
 *    R-D Cost for a macroblock
 *************************************************************************************
 */
int RDCost_for_macroblocks (Macroblock  *currMB,   // <-- Current Macroblock to code
                            double   lambda,       // <-- lagrange multiplier
                            int      mode,         // <-- modus (0-COPY/DIRECT, 1-16x16, 2-16x8, 3-8x16, 4-8x8(+), 5-Intra4x4, 6-Intra16x16)
                            double*  min_rdcost,   // <-> minimum rate-distortion cost
                            double*  min_dcost,   // <-> distortion of mode which has minimum rate-distortion cost.
                            double*  min_rate,     // --> bitrate of mode which has minimum rate-distortion cost.
                            int i16mode,
                            int is_cavlc)
{
  int         i, j, k; //, k, ****ip4;
  int         j1, j2;
  int         rate = 0, coeff_rate = 0;
  int64       distortion = 0;
  double      rdcost;
  int         prev_mb_nr  = FmoGetPreviousMBNr(img->current_mb_nr);  
  Macroblock  *prevMB   = (prev_mb_nr >= 0) ? &img->mb_data[prev_mb_nr] : NULL;
  int         bframe    = (img->type==B_SLICE);
  int         tmp_cc;
  int         use_of_cc =  (img->type!=I_SLICE &&  is_cavlc);
  int         cc_rate, dummy;
  double      dummy_d;
  imgpel     (*mb_pred)[16] = img->mb_pred[0];
  imgpel     (*curr_mpr_16x16)[16][16] = img->mpr_16x16[0];

  //=====
  //=====  SET REFERENCE FRAMES AND BLOCK MODES
  //=====
  SetModesAndRefframeForBlocks (currMB, mode);
  
  //=====
  //=====  GET COEFFICIENTS, RECONSTRUCTIONS, CBP
  //=====
  if (bframe && mode==0)
  {
    int block_x = (img->pix_x >> 2);
    int block_y = (img->pix_y >> 2);
    for (j = block_y; j < block_y + 4;j++)
      for (i = block_x; i < block_x + 4;i++)
        if (direct_pdir[j][i] < 0)
          return 0;
  }

  // Test MV limits for Skip Mode. This could be necessary for MBAFF case Frame MBs.
  if ((img->MbaffFrameFlag) && (!currMB->mb_field) && (img->type==P_SLICE) && (mode==0) )
  {
    if (out_of_bounds_mvs(img, img->all_mv[0][0][0][0][0], Q_PEL))
      return 0;
  }

  if (img->AdaptiveRounding)
  {
    memset(&(img->fadjust4x4[0][0][0]), 0, MB_PIXELS * sizeof(int));
    memset(&(img->fadjust8x8[0][0][0]), 0, MB_PIXELS * sizeof(int));
    if (img->yuv_format != 0)
    {
      memset(&(img->fadjust4x4Cr[0][0][0][0]), 0, img->mb_cr_size_y * img->mb_cr_size_x * sizeof(int));
      memset(&(img->fadjust4x4Cr[1][0][0][0]), 0, img->mb_cr_size_y * img->mb_cr_size_x  * sizeof(int));
      memset(&(img->fadjust8x8Cr[0][0][0][0]), 0, img->mb_cr_size_y * img->mb_cr_size_x  * sizeof(int));
      memset(&(img->fadjust8x8Cr[1][0][0][0]), 0, img->mb_cr_size_y * img->mb_cr_size_x  * sizeof(int));
    }
  }

  if (mode<P8x8)
  {
    LumaResidualCoding (currMB, is_cavlc);

    // This code seems unnecessary 
    if(mode==0 && currMB->cbp!=0 && (img->type != B_SLICE || img->NoResidueDirect==1))
      return 0;
    if (img->P444_joined)      
    {
      if(mode==0 && (currMB->cbp==0 && cmp_cbp[1] == 0 && cmp_cbp[2] == 0)&& currMB->luma_transform_size_8x8_flag == 1) //for B_skip, luma_transform_size_8x8_flag=0 only
        return 0;
    }
    else
    {
      if(mode==0 && currMB->cbp==0 && currMB->luma_transform_size_8x8_flag == 1) //for B_skip, luma_transform_size_8x8_flag=0 only        
        return 0;
    }
  }
  else if (mode==P8x8)
  {
    SetCoeffAndReconstruction8x8 (currMB);
  }
  else if (mode==I4MB)
  {
    currMB->cbp = Mode_Decision_for_Intra4x4Macroblock (currMB, lambda, &dummy_d, is_cavlc);
  }
  else if (mode==I16MB)
  {
    Intra16x16_Mode_Decision  (currMB, &i16mode, is_cavlc);
  }
  else if(mode==I8MB)
  {
    currMB->cbp = Mode_Decision_for_new_Intra8x8Macroblock(currMB, lambda, &dummy_d);
  }
  else if(mode==IPCM)
  {
    for (j = 0; j < MB_BLOCK_SIZE; j++)
    {
      memcpy(&enc_picture->imgY[j + img->pix_y][img->opix_x], &pCurImg[j + img->opix_y][img->opix_x], MB_BLOCK_SIZE * sizeof(imgpel));
    }
    if ((img->yuv_format != YUV400) && !IS_INDEPENDENT(params))
    {
      // CHROMA
      for (j = 0; j<img->mb_cr_size_y; j++)
      {
        j1 = j + img->opix_c_y;
        j2 = j + img->pix_c_y;
        memcpy(&enc_picture->imgUV[0][j2][img->opix_c_x], &pImgOrg[1][j1][img->opix_c_x], img->mb_cr_size_x * sizeof(imgpel));
        memcpy(&enc_picture->imgUV[1][j2][img->opix_c_x], &pImgOrg[2][j1][img->opix_c_x], img->mb_cr_size_x * sizeof(imgpel));
      }
    }
    for (j=0;j<4;j++)
      for (i=0; i<(4+img->num_blk8x8_uv); i++)
        img->nz_coeff[img->current_mb_nr][j][i] = 16;

  }

  if (params->rdopt==3 && img->type!=B_SLICE)
  {
    // We need the reconstructed prediction residue for the simulated decoders.
    // Should we be handing imgY here or p_curr_img instead of p_img[0]? This could make it more generic
    ////compute_residue_mb (img, &enc_picture->p_img[0][img->pix_y], decs->res_img[0], mode == I16MB ? i16mode : -1);
    compute_residue_block (img, &enc_picture->p_curr_img[img->pix_y], decs->res_img[0], mode == I16MB ? img->mpr_16x16[0][i16mode] : img->mb_pred[0], 0, 16);
  }

  //Rate control
  if (params->RCEnable)
  {
    if (mode == I16MB)
      memcpy(pred, curr_mpr_16x16[i16mode], MB_PIXELS * sizeof(imgpel));
    else
      memcpy(pred, mb_pred, MB_PIXELS * sizeof(imgpel));
  }

  img->i16offset = 0;
  dummy = 0;

  if (((img->yuv_format!=YUV400) && (active_sps->chroma_format_idc != YUV444)) && (mode != IPCM))
    ChromaResidualCoding (currMB, is_cavlc);

  if (mode==I16MB)
    img->i16offset = I16Offset  (currMB->cbp, i16mode);

  //=====
  //=====   GET DISTORTION
  //=====
  // LUMA
  if (params->rdopt == 3 && img->type!=B_SLICE)
  {
    if (mode != P8x8)
    {
      for (k = 0; k<params->NoOfDecoders ;k++)
      {
        decode_one_mb (img, enc_picture, k, currMB);
        distortion += compute_SSE(&pCurImg[img->opix_y], &enc_picture->p_dec_img[0][k][img->opix_y], img->opix_x, img->opix_x, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
      }
    }
    else
    {
      for (k = 0; k<params->NoOfDecoders ;k++)
      {
        distortion += compute_SSE(&pCurImg[img->opix_y], &enc_picture->p_dec_img[0][k][img->opix_y], img->opix_x, img->opix_x, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
      }
    }
    distortion /= params->NoOfDecoders;

    if ((img->yuv_format != YUV400) && (active_sps->chroma_format_idc != YUV444))
    {
      // CHROMA
      distortion += compute_SSE(&pImgOrg[1][img->opix_c_y], &enc_picture->imgUV[0][img->pix_c_y], img->opix_c_x, img->pix_c_x, img->mb_cr_size_y, img->mb_cr_size_x);
      distortion += compute_SSE(&pImgOrg[2][img->opix_c_y], &enc_picture->imgUV[1][img->pix_c_y], img->opix_c_x, img->pix_c_x, img->mb_cr_size_y, img->mb_cr_size_x);
    }
  }
  else
  {
    distortion = getDistortion(currMB);
  }

  //=====   S T O R E   C O D I N G   S T A T E   =====
  //---------------------------------------------------
  store_coding_state (currMB, cs_cm);

  //=====
  //=====   GET RATE
  //=====
  //----- macroblock header -----
  if (use_of_cc)
  {
    if (currMB->mb_type!=0 || (bframe && currMB->cbp!=0))
    {
      // cod counter and macroblock mode are written ==> do not consider code counter
      tmp_cc = img->cod_counter;
      rate   = writeMBLayer (currMB, 1, &coeff_rate);
      ue_linfo (tmp_cc, dummy, &cc_rate, &dummy);
      rate  -= cc_rate;
      img->cod_counter = tmp_cc;
    }
    else
    {
      // cod counter is just increased  ==> get additional rate
      ue_linfo (img->cod_counter + 1, dummy, &rate,    &dummy);
      ue_linfo (img->cod_counter    , dummy, &cc_rate, &dummy);
      rate -= cc_rate;
    }
  }
  else
  {
    rate = writeMBLayer (currMB, 1, &coeff_rate);
  }

  //=====   R E S T O R E   C O D I N G   S T A T E   =====
  //-------------------------------------------------------
  reset_coding_state (currMB, cs_cm);

  rdcost = (double)distortion + lambda * dmax(0.5,(double)rate);

  if (rdcost >= *min_rdcost ||
    ((currMB->qp_scaled[0]) == 0 && img->lossless_qpprime_flag == 1 && distortion != 0))
  {
#if FASTMODE
    // Reordering RDCost comparison order of mode 0 and mode 1 in P_SLICE
    // if RDcost of mode 0 and mode 1 is same, we choose best_mode is 0
    // This might not always be good since mode 0 is more biased towards rate than quality.
    if((img->type!=P_SLICE || mode != 0 || rdcost != *min_rdcost) || IS_FREXT_PROFILE(params->ProfileIDC))
#endif
      return 0;
  }


  if ((img->MbaffFrameFlag) && (mode ? 0: ((img->type == B_SLICE) ? !currMB->cbp:1)))  // AFF and current is skip
  {
    if (img->current_mb_nr & 0x01) //bottom
    {
      if (prevMB->mb_type ? 0:((img->type == B_SLICE) ? !prevMB->cbp:1)) //top is skip
      {
        if (!(field_flag_inference(currMB) == currMB->mb_field)) //skip only allowed when correct inference
          return 0;
      }
    }
  }

  //=====   U P D A T E   M I N I M U M   C O S T   =====
  //-----------------------------------------------------
  *min_rdcost = rdcost;
  *min_dcost = (double) distortion;
  *min_rate = lambda * (double)coeff_rate;

#ifdef BEST_NZ_COEFF
  for (j=0;j<4;j++)
    memcpy(&gaaiMBAFF_NZCoeff[j][0], &img->nz_coeff[img->current_mb_nr][j][0], (4 + img->num_blk8x8_uv) * sizeof(int));
#endif

  return 1;
}


/*!
 *************************************************************************************
 * \brief
 *    Store macroblock parameters
 *************************************************************************************
 */
void store_macroblock_parameters (Macroblock *currMB, int mode)
{
  int  j, ****i4p, ***i3p;
  int  bframe   = (img->type==B_SLICE);

  //--- store best mode ---
  best_mode = mode;
  best_c_imode = currMB->c_ipred_mode;
  best_i16offset = img->i16offset;

  
  memcpy(b8mode, currMB->b8mode, BLOCK_MULTIPLE * sizeof(short));
  memcpy(b8bipred_me, currMB->bipred_me, BLOCK_MULTIPLE * sizeof(short));
  memcpy(b8pdir, currMB->b8pdir, BLOCK_MULTIPLE * sizeof(short));
  memcpy(b4_intra_pred_modes,   currMB->intra_pred_modes, MB_BLOCK_PARTITIONS * sizeof(char));
  memcpy(b8_intra_pred_modes8x8,currMB->intra_pred_modes8x8, MB_BLOCK_PARTITIONS * sizeof(char));

  for (j = 0 ; j < BLOCK_MULTIPLE; j++)
  {
    memcpy(&b4_ipredmode[j * BLOCK_MULTIPLE],&img->ipredmode   [img->block_y + j][img->block_x],BLOCK_MULTIPLE * sizeof(char));
    memcpy(b8_ipredmode8x8[j],               &img->ipredmode8x8[img->block_y + j][img->block_x],BLOCK_MULTIPLE * sizeof(char));
  }
  //--- reconstructed blocks ----
  for (j = 0; j < MB_BLOCK_SIZE; j++)
  {
    memcpy(rec_mbY[j], &enc_picture->imgY[img->pix_y+j][img->pix_x], MB_BLOCK_SIZE * sizeof(imgpel));
  }
  if((img->type==SP_SLICE) && (si_frame_indicator==0 && sp2_frame_indicator==0))
  {
    for (j = 0; j < MB_BLOCK_SIZE; j++)
    {
      memcpy(lrec_rec[j], &lrec[img->pix_y+j][img->pix_x], MB_BLOCK_SIZE * sizeof(int));//store coefficients SP frame
    }
  }

  if (img->AdaptiveRounding)
    store_adaptive_rounding_parameters (currMB, mode);

  if (img->yuv_format != YUV400)
  {
    int k;
    for (k = 0; k < 2; k++)
    {
      for (j = 0; j<img->mb_cr_size_y; j++)
      {
        memcpy(rec_mb_cr[k][j], &enc_picture->imgUV[k][img->pix_c_y+j][img->pix_c_x], img->mb_cr_size_x * sizeof(imgpel));
      }
    }

    if((img->type==SP_SLICE) && (si_frame_indicator==0 && sp2_frame_indicator==0))
    {
      //store uv coefficients SP frame
      for (k = 0; k < 2; k++)
      {
        for (j = 0; j<img->mb_cr_size_y; j++)
        {
          memcpy(lrec_rec_uv[k][j],&lrec_uv[k][img->pix_c_y+j][img->pix_c_x], img->mb_cr_size_x * sizeof(int));
        }
      }
    }
  }

  //--- store results of decoders ---
  if (params->rdopt == 3 && img->type!=B_SLICE)
  {
    errdo_store_best_block(img, decs->dec_mbY, enc_picture->p_dec_img[0], 0, 0, MB_BLOCK_SIZE);
  }

  //--- coeff, cbp, kac ---
  if (mode || bframe)
  {
    i4p=cofAC; cofAC=img->cofAC; img->cofAC=i4p;
    i3p=cofDC; cofDC=img->cofDC; img->cofDC=i3p;
    cbp     = currMB->cbp;
    curr_cbp[0] = cmp_cbp[1];  
    curr_cbp[1] = cmp_cbp[2]; 

    cur_cbp_blk[0] = currMB->cbp_blk;
  }
  else
  {
    cur_cbp_blk[0] = cbp = 0;
    cmp_cbp[1] = cmp_cbp[2] = 0; 
  }

  //--- store transform size ---
  luma_transform_size_8x8_flag = currMB->luma_transform_size_8x8_flag;


  for (j = 0; j<4; j++)
    memcpy(l0_refframe[j],&enc_picture->motion.ref_idx[LIST_0][img->block_y+j][img->block_x], BLOCK_MULTIPLE * sizeof(char));

  if (bframe)
  {
    for (j = 0; j<4; j++)
      memcpy(l1_refframe[j],&enc_picture->motion.ref_idx[LIST_1][img->block_y+j][img->block_x], BLOCK_MULTIPLE * sizeof(char));
  }
}


/*!
 *************************************************************************************
 * \brief
 *    Set stored macroblock parameters
 *************************************************************************************
 */
void set_stored_macroblock_parameters (Macroblock *currMB)
{
  imgpel     **imgY  = enc_picture->imgY;
  imgpel    ***imgUV = enc_picture->imgUV;

  int         mode   = best_mode;
  int         bframe = (img->type==B_SLICE);
  int         i, j, k, ****i4p, ***i3p;
  int         block_x, block_y;
  char    **ipredmodes = img->ipredmode;
  short   *cur_mv, total_bipred_me;

  //===== reconstruction values =====

  // Luma
  for (j = 0; j < MB_BLOCK_SIZE; j++)
  {
    memcpy(&imgY[img->pix_y+j][img->pix_x],rec_mbY[j], MB_BLOCK_SIZE * sizeof(imgpel));
  }

  if (img->MbaffFrameFlag || (params->UseRDOQuant && params->RDOQ_QP_Num > 1))
  {
    for (j = 0; j < MB_BLOCK_SIZE; j++)
      memcpy(rdopt->rec_mbY[j],rec_mbY[j], MB_BLOCK_SIZE * sizeof(imgpel));
  }

  if((img->type==SP_SLICE) &&(si_frame_indicator==0 && sp2_frame_indicator==0 ))
  {
    for (j = 0; j < MB_BLOCK_SIZE; j++)
      memcpy(&lrec[img->pix_y+j][img->pix_x],lrec_rec[j], MB_BLOCK_SIZE * sizeof(int)); //restore coeff SP frame
  }

  if (img->AdaptiveRounding)
  {
    update_offset_params(currMB, mode,luma_transform_size_8x8_flag);
  }

  if (img->yuv_format != YUV400)
  {
    int k;
    for (k = 0; k < 2; k++)
    {
      for (j = 0; j<img->mb_cr_size_y; j++)
      {
        memcpy(&imgUV[k][img->pix_c_y+j][img->pix_c_x], rec_mb_cr[k][j], img->mb_cr_size_x * sizeof(imgpel));
      }
    }

    if (img->MbaffFrameFlag || (params->UseRDOQuant && params->RDOQ_QP_Num > 1))
    {
      for (k = 0; k < 2; k++)
      {
        for (j = 0; j<img->mb_cr_size_y; j++)
        {
          memcpy(rdopt->rec_mb_cr[k][j],rec_mb_cr[k][j], img->mb_cr_size_x * sizeof(imgpel));
        }
      }
    }

    if((img->type==SP_SLICE) &&(!si_frame_indicator && !sp2_frame_indicator))
    {
      for (k = 0; k < 2; k++)
      {
        for (j = 0; j<img->mb_cr_size_y; j++)
        {
          memcpy(&lrec_uv[k][img->pix_c_y+j][img->pix_c_x],lrec_rec_uv[k][j], img->mb_cr_size_x * sizeof(int));
        }
      }
    }
  }

  //===== coefficients and cbp =====
  i4p=cofAC; cofAC=img->cofAC; img->cofAC=i4p;
  i3p=cofDC; cofDC=img->cofDC; img->cofDC=i3p;
  currMB->cbp      = cbp;
  currMB->cbp_blk = cur_cbp_blk[0];
  cmp_cbp[1] = curr_cbp[0]; 
  cmp_cbp[2] = curr_cbp[1]; 
  currMB->cbp |= cmp_cbp[1];
  currMB->cbp |= cmp_cbp[2];
  cmp_cbp[1] = currMB->cbp; 
  cmp_cbp[2] = currMB->cbp;

  //==== macroblock type ====
  currMB->mb_type = mode;

  memcpy(currMB->b8mode, b8mode, BLOCK_MULTIPLE * sizeof(short));
  memcpy(currMB->b8pdir, b8pdir, BLOCK_MULTIPLE * sizeof(short));
  memcpy(currMB->bipred_me, b8bipred_me, BLOCK_MULTIPLE * sizeof(short));

  if (img->MbaffFrameFlag || (params->UseRDOQuant && params->RDOQ_QP_Num > 1))
  {
    rdopt->mode = mode;
    rdopt->i16offset = img->i16offset;
    rdopt->cbp = cbp;
    rdopt->cbp_blk = cur_cbp_blk[0];
    rdopt->mb_type  = mode;

    rdopt->prev_qp  = currMB->prev_qp;
    rdopt->prev_dqp = currMB->prev_dqp;
    rdopt->delta_qp = currMB->delta_qp;
    rdopt->qp       = currMB->qp;
    rdopt->prev_cbp = currMB->prev_cbp;

    memcpy(rdopt->cofAC[0][0][0], img->cofAC[0][0][0], (4+img->num_blk8x8_uv) * 4 * 2 * 65 * sizeof(int));
    memcpy(rdopt->cofDC[0][0], img->cofDC[0][0], 3 * 2 * 18 * sizeof(int));

    memcpy(rdopt->b8mode,b8mode, BLOCK_MULTIPLE * sizeof(short));
    memcpy(rdopt->b8pdir,b8pdir, BLOCK_MULTIPLE * sizeof(short));
  }

  //if P8x8 mode and transform size 4x4 choosen, restore motion vector data for this transform size
  if (mode == P8x8 && !luma_transform_size_8x8_flag && params->Transform8x8Mode)
    RestoreMV8x8(1);

  //==== transform size flag ====
  if (img->P444_joined)
  {
    if (((currMB->cbp == 0) && cmp_cbp[1] == 0 && cmp_cbp[2] == 0) && !(IS_OLDINTRA(currMB) || currMB->mb_type == I8MB))
      currMB->luma_transform_size_8x8_flag = 0;
    else
      currMB->luma_transform_size_8x8_flag = luma_transform_size_8x8_flag;
  }
  else
  {

    if (((currMB->cbp & 15) == 0) && !(IS_OLDINTRA(currMB) || currMB->mb_type == I8MB))
      currMB->luma_transform_size_8x8_flag = 0;
    else
      currMB->luma_transform_size_8x8_flag = luma_transform_size_8x8_flag;
  }

  rdopt->luma_transform_size_8x8_flag  = currMB->luma_transform_size_8x8_flag;

  if (params->rdopt == 3 && img->type!=B_SLICE)  
  {
    errdo_get_best_block(img, enc_picture->p_dec_img[0], decs->dec_mbY, 0, MB_BLOCK_SIZE);
  }

  //==== reference frames =====
  for (j = 0; j < 4; j++)
  {
    block_y = img->block_y + j;
    for (i = 0; i < 4; i++)
    {
      block_x = img->block_x + i;
      k = 2*(j >> 1)+(i >> 1);

      // backward prediction or intra
      if ((currMB->b8pdir[k] == 1) || IS_INTRA(currMB))
      {
        enc_picture->motion.ref_idx    [LIST_0][block_y][block_x]    = -1;
        enc_picture->motion.ref_pic_id [LIST_0][block_y][block_x]    = -1;
        enc_picture->motion.mv         [LIST_0][block_y][block_x][0] = 0;
        enc_picture->motion.mv         [LIST_0][block_y][block_x][1] = 0;

        if (img->MbaffFrameFlag || (params->UseRDOQuant && params->RDOQ_QP_Num > 1))
          rdopt->refar[LIST_0][j][i] = -1;
      }
      else
      {
        if (currMB->bipred_me[k] && (currMB->b8pdir[k] == 2) && is_bipred_enabled(currMB->mb_type))
        {
          cur_mv = img->bipred_mv[currMB->bipred_me[k] - 1][LIST_0][0][currMB->b8mode[k]][j][i]; 

          enc_picture->motion.ref_idx    [LIST_0][block_y][block_x] = 0;
          enc_picture->motion.ref_pic_id [LIST_0][block_y][block_x] = enc_picture->ref_pic_num[LIST_0 + currMB->list_offset][0];
          enc_picture->motion.mv         [LIST_0][block_y][block_x][0] = cur_mv[0];
          enc_picture->motion.mv         [LIST_0][block_y][block_x][1] = cur_mv[1];

          if (img->MbaffFrameFlag || (params->UseRDOQuant && params->RDOQ_QP_Num > 1))
            rdopt->refar[LIST_0][j][i] = 0;
        }
        else
        {
          char cur_ref = l0_refframe[j][i];
          enc_picture->motion.ref_idx    [LIST_0][block_y][block_x] = cur_ref;
          enc_picture->motion.ref_pic_id [LIST_0][block_y][block_x] = enc_picture->ref_pic_num[LIST_0 + currMB->list_offset][(short)cur_ref];
          memcpy(enc_picture->motion.mv  [LIST_0][block_y][block_x], img->all_mv[LIST_0][(short)cur_ref][currMB->b8mode[k]][j][i], 2 * sizeof(short));

          if (img->MbaffFrameFlag || (params->UseRDOQuant && params->RDOQ_QP_Num > 1))
            rdopt->refar[LIST_0][j][i] = cur_ref;
        }
      }

      // forward prediction or intra
      if ((currMB->b8pdir[k] == 0) || IS_INTRA(currMB))
      {
        enc_picture->motion.ref_idx    [LIST_1][block_y][block_x]    = -1;
        enc_picture->motion.ref_pic_id [LIST_1][block_y][block_x]    = -1;
        enc_picture->motion.mv         [LIST_1][block_y][block_x][0] = 0;
        enc_picture->motion.mv         [LIST_1][block_y][block_x][1] = 0;

        if (img->MbaffFrameFlag || (params->UseRDOQuant && params->RDOQ_QP_Num > 1))
          rdopt->refar[LIST_1][j][i] = -1;
      }
    }
  }

  if (bframe)
  {
    for (j=0; j<4; j++)
    {
      block_y = img->block_y + j;
      for (i=0; i<4; i++)
      {
        block_x = img->block_x + i;
        k = 2*(j >> 1)+(i >> 1);

        // forward
        if (IS_INTRA(currMB)||(currMB->b8pdir[k] == 0))
        {
          enc_picture->motion.ref_idx    [LIST_1][block_y][block_x]    = -1;
          enc_picture->motion.ref_pic_id [LIST_1][block_y][block_x]    = -1;
          enc_picture->motion.mv         [LIST_1][block_y][block_x][0] = 0;
          enc_picture->motion.mv         [LIST_1][block_y][block_x][1] = 0;

          if (img->MbaffFrameFlag || (params->UseRDOQuant && params->RDOQ_QP_Num > 1))
            rdopt->refar[LIST_1][j][i] = -1;
        }
        else
        {
          if (currMB->bipred_me[k] && (currMB->b8pdir[k] == 2) && is_bipred_enabled(currMB->mb_type))
          {
            cur_mv = img->bipred_mv[currMB->bipred_me[k] - 1][LIST_1][0][currMB->b8mode[k]][j][i]; 

            enc_picture->motion.ref_idx    [LIST_1][block_y][block_x] = 0;
            enc_picture->motion.ref_pic_id [LIST_1][block_y][block_x] = enc_picture->ref_pic_num[LIST_1 + currMB->list_offset][0];
            enc_picture->motion.mv         [LIST_1][block_y][block_x][0] = cur_mv[0];
            enc_picture->motion.mv         [LIST_1][block_y][block_x][1] = cur_mv[1];

            if (img->MbaffFrameFlag || (params->UseRDOQuant && params->RDOQ_QP_Num > 1))
              rdopt->refar[LIST_1][j][i] = 0;
          }
          else
          {
            enc_picture->motion.ref_idx    [LIST_1][block_y][block_x] = l1_refframe[j][i];
            enc_picture->motion.ref_pic_id [LIST_1][block_y][block_x] = enc_picture->ref_pic_num[LIST_1 + currMB->list_offset][(short)l1_refframe[j][i]];
            memcpy(enc_picture->motion.mv  [LIST_1][block_y][block_x], img->all_mv[LIST_1][(short)l1_refframe[j][i]][currMB->b8mode[k]][j][i], 2 * sizeof(short));

            if (img->MbaffFrameFlag || (params->UseRDOQuant && params->RDOQ_QP_Num > 1))
              rdopt->refar[LIST_1][j][i] = l1_refframe[j][i];
          }
        }
      }
    }
  }

  //==== intra prediction modes ====
  currMB->c_ipred_mode = best_c_imode;
  img->i16offset = best_i16offset;

  if(currMB->mb_type == I8MB)
  {
    memcpy(currMB->intra_pred_modes8x8,b8_intra_pred_modes8x8, MB_BLOCK_PARTITIONS * sizeof(char));
    memcpy(currMB->intra_pred_modes,b8_intra_pred_modes8x8, MB_BLOCK_PARTITIONS * sizeof(char));
    for(j = 0; j < BLOCK_MULTIPLE; j++)
    {
      memcpy(&img->ipredmode[img->block_y+j][img->block_x],    b8_ipredmode8x8[j], BLOCK_MULTIPLE * sizeof(char));
      memcpy(&img->ipredmode8x8[img->block_y+j][img->block_x], b8_ipredmode8x8[j], BLOCK_MULTIPLE * sizeof(char));
    }
  }
  else if (mode!=I4MB && mode!=I8MB)
  {
    memset(currMB->intra_pred_modes,DC_PRED, MB_BLOCK_PARTITIONS * sizeof(char));
    for(j = img->block_y; j < img->block_y + BLOCK_MULTIPLE; j++)
      memset(&img->ipredmode[j][img->block_x], DC_PRED, BLOCK_MULTIPLE * sizeof(char));
  }
  // Residue Color Transform
  else if (mode == I4MB)
  {
    memcpy(currMB->intra_pred_modes,b4_intra_pred_modes, MB_BLOCK_PARTITIONS * sizeof(char));
    for(j = 0; j < BLOCK_MULTIPLE; j++)
      memcpy(&img->ipredmode[img->block_y + j][img->block_x],&b4_ipredmode[BLOCK_MULTIPLE * j], BLOCK_MULTIPLE * sizeof(char));
  }

  if (img->MbaffFrameFlag || (params->UseRDOQuant && params->RDOQ_QP_Num > 1))
  {
    rdopt->c_ipred_mode = currMB->c_ipred_mode;
    rdopt->i16offset = img->i16offset;
    memcpy(rdopt->intra_pred_modes,currMB->intra_pred_modes, MB_BLOCK_PARTITIONS * sizeof(char));
    memcpy(rdopt->intra_pred_modes8x8,currMB->intra_pred_modes8x8, MB_BLOCK_PARTITIONS * sizeof(char));
    for(j = img->block_y; j < img->block_y +BLOCK_MULTIPLE; j++)
      memcpy(&rdopt->ipredmode[j][img->block_x],&ipredmodes[j][img->block_x], BLOCK_MULTIPLE * sizeof(char));
  }

  //==== motion vectors =====
  SetMotionVectorsMB (currMB, bframe);
  total_bipred_me  = 0;
 
  for (i = 0; i < 4; i++)
  {
    total_bipred_me += currMB->bipred_me[i];
  }
  if (total_bipred_me && currMB->mb_type > 1 && currMB->mb_type < 4)
  {
     UpdateMotionVectorPredictor(currMB, currMB->mb_type);
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Update motion vector predictors if biprediction mode is selected (B_SLICE only)
 *************************************************************************************
 */
void UpdateMotionVectorPredictor(Macroblock* currMB, int mb_type) 
{
  int v = 0, h = 0, block_x, block_y, list, i, ref;
  short * pred_mv;
  int   parttype  = (mb_type<4?mb_type:4);
  int   bsx       = params->blc_size[parttype][0];
  int   bsy       = params->blc_size[parttype][1];
  int   step_h    = (params->part_size[parttype][0]);
  int   step_v    = (params->part_size[parttype][1]);

  for (i = 0; i < 4; i++)
  {
    if (currMB->bipred_me[i])
    {
      v = i >> 1;
      h = i & 0x01;
      break;
    }
  }

  while(1)
  {
    block_x = img->block_x + h;
    block_y = img->block_y + v;
    for (list = 0; list < 2; list++)
    {
      ref = enc_picture->motion.ref_idx[list][block_y][block_x];
      ref = (ref < 0 )? 0 : ref;
      pred_mv = img->pred_mv[list][ref][parttype][v][h];
      SetMotionVectorPredictor (currMB, pred_mv, enc_picture->motion.ref_idx[list], enc_picture->motion.mv[list], ref, list, h << 2, v << 2, bsx, bsy);
    }

    v += step_v;
    h += step_h;
    if (v >= 4 && h >= 4)
      break;
    if (v >= 4)
      v = 0;
    if (h >= 4)
      h = 0;
  }
}
 

/*!
 *************************************************************************************
 * \brief
 *    Set reference frames and motion vectors
 *************************************************************************************
 */
void SetRefAndMotionVectors (Macroblock *currMB, int block, int mode, int pdir, int fwref, int bwref, short bipred_me)
{
  int     k, i, j=0;
  int     bslice  = (img->type==B_SLICE);
  int     pmode   = (mode==1||mode==2||mode==3?mode:4);
  int     j0      = ((block >> 1)<<1);
  int     i0      = ((block & 0x01)<<1);
  int     j1      = j0 + (params->part_size[pmode][1]);
  int     i1      = i0 + (params->part_size[pmode][0]);
  int     block_x, block_y;
  short   *cur_mv;
  int64 ref_pic_num;
  char *ref_idx;

  if (pdir < 0)
  {
    for (k = LIST_0; k <= LIST_1; k++)
    {
      for (j = img->block_y + j0; j < img->block_y + j1; j++)
      {
        for (i=img->block_x + i0; i<img->block_x +i1; i++)
          enc_picture->motion.ref_pic_id[k][j][i] = -1;

        memset(&enc_picture->motion.ref_idx[k][j][img->block_x + i0], -1, (params->part_size[pmode][0]) * sizeof(char));
        memset(enc_picture->motion.mv      [k][j][img->block_x + i0], 0, 2*(params->part_size[pmode][0]) * sizeof(short));
      }
    }
    return;
  }

  if (!bslice)
  {
    int64 ref_pic_num = enc_picture->ref_pic_num[LIST_0+currMB->list_offset][fwref];
    for (j = j0; j < j1; j++)
    {
      block_y = img->block_y + j;
      for (block_x = img->block_x + i0; block_x < img->block_x + i1; block_x++)
        enc_picture->motion.ref_pic_id[LIST_0][block_y][block_x] = ref_pic_num;

      memset(&enc_picture->motion.ref_idx[LIST_0][block_y][img->block_x + i0], fwref, (params->part_size[pmode][0]) * sizeof(char));
      memcpy(enc_picture->motion.mv      [LIST_0][block_y][img->block_x + i0], img->all_mv[LIST_0][fwref][mode][j][i0], 2 * (i1 - i0) * sizeof(short));
    }
    return;
  }
  else //bslice
  {
    if ((pdir == 0 || pdir == 2))
    {
      if (bipred_me && (pdir == 2) && is_bipred_enabled(mode))
      {
        for (j=j0; j<j1; j++)
        {
          block_y = img->block_y + j;
          for (i=i0; i<i1; i++)
          {
            block_x = img->block_x + i;

            cur_mv = img->bipred_mv[bipred_me - 1][LIST_0][0][mode][j][i]; 

            enc_picture->motion.mv        [LIST_0][block_y][block_x][0] = cur_mv[0];
            enc_picture->motion.mv        [LIST_0][block_y][block_x][1] = cur_mv[1];
            enc_picture->motion.ref_idx   [LIST_0][block_y][block_x]    = 0;
            enc_picture->motion.ref_pic_id[LIST_0][block_y][block_x]    = enc_picture->ref_pic_num[LIST_0+currMB->list_offset][0];
          }
        }
      }
      else
      {
        if (mode==0)
        {
          for (j=j0; j<j1; j++)
          {
            block_y = img->block_y + j;
            ref_idx = enc_picture->motion.ref_idx[LIST_0][block_y];
            memcpy(&ref_idx[img->block_x + i0], &direct_ref_idx[LIST_0][block_y][img->block_x + i0], (i1 - i0) * sizeof(char));
            memcpy(&enc_picture->motion.mv[LIST_0][block_y][img->block_x + i0][0], img->all_mv[LIST_0][fwref][mode][j][i0], 2 * (i1 - i0) * sizeof(short));
            for (block_x = img->block_x + i0; block_x < img->block_x + i1; block_x++)
            {              
              enc_picture->motion.ref_pic_id[LIST_0][block_y][block_x] = 
                enc_picture->ref_pic_num[LIST_0+currMB->list_offset][(short)ref_idx[block_x]];
            }            
          }
        }
        else
        {
          for (j=j0; j<j1; j++)
          {
            block_y = img->block_y + j;
            ref_pic_num = enc_picture->ref_pic_num[LIST_0+currMB->list_offset][fwref];
            memcpy(&enc_picture->motion.mv[LIST_0][block_y][img->block_x + i0][0], img->all_mv[LIST_0][fwref][mode][j][i0], 2 * (i1 - i0) * sizeof(short));
            memset(&enc_picture->motion.ref_idx[LIST_0][block_y][img->block_x + i0], fwref, (i1 - i0) * sizeof(char));
            for (block_x = img->block_x + i0; block_x < img->block_x + i1; block_x++)
            {              
              enc_picture->motion.ref_pic_id[LIST_0][block_y][block_x] = ref_pic_num ;
            }
          }
        }
      }  
    }
    else
    {
      for (j=j0; j<j1; j++)
      {
        block_y = img->block_y + j;
        for (i=img->block_x + i0; i<img->block_x +i1; i++)
          enc_picture->motion.ref_pic_id[LIST_0][block_y][i] = -1;

        memset(&enc_picture->motion.ref_idx[LIST_0][block_y][img->block_x + i0], -1, (i1 - i0) * sizeof(char));
        memset(enc_picture->motion.mv[LIST_0][block_y][img->block_x + i0], 0, 2 * (i1 - i0) * sizeof(short));
      }
    }


    if ((pdir==1 || pdir==2))
    {
      if (bipred_me && (pdir == 2) && is_bipred_enabled(mode))
      {
        for (j=j0; j<j1; j++)
        {
          block_y = img->block_y + j;

          for (i=i0; i<i1; i++)
          {
            block_x = img->block_x + i;

            cur_mv = img->bipred_mv[bipred_me - 1][LIST_1][0][mode][j][i]; 

            enc_picture->motion.mv        [LIST_1][block_y][block_x][0] = cur_mv[0];
            enc_picture->motion.mv        [LIST_1][block_y][block_x][1] = cur_mv[1];
            enc_picture->motion.ref_idx   [LIST_1][block_y][block_x]    = 0;
            enc_picture->motion.ref_pic_id[LIST_1][block_y][block_x]    = enc_picture->ref_pic_num[LIST_1+currMB->list_offset][0];
          }
        }        
      }
      else
      {
        if (mode==0)
        {
          for (j=j0; j<j1; j++)
          {
            block_y = img->block_y + j;

            ref_idx = enc_picture->motion.ref_idx[LIST_1][block_y];
            memcpy(&ref_idx[img->block_x + i0], &direct_ref_idx[LIST_1][block_y][img->block_x + i0], (i1 - i0) * sizeof(char));            
            memcpy(&enc_picture->motion.mv[LIST_1][block_y][img->block_x + i0][0], img->all_mv[LIST_1][(int) ref_idx[img->block_x + i0]][mode][j][i0], 2 * (i1 - i0) * sizeof(short));
            for (i=i0; i<i1; i++)
            {
              block_x = img->block_x + i;
              enc_picture->motion.ref_pic_id[LIST_1][block_y][block_x] = enc_picture->ref_pic_num[LIST_1+currMB->list_offset][(short)ref_idx[block_x]];
            }            
          }
        }
        else
        {
          for (j=j0; j<j1; j++)
          {
            block_y = img->block_y + j;
            ref_pic_num = enc_picture->ref_pic_num[LIST_1+currMB->list_offset][bwref];
            memcpy(&enc_picture->motion.mv[LIST_1][block_y][img->block_x + i0][0], img->all_mv[LIST_1][bwref][mode][j][i0], 2 * (i1 - i0) * sizeof(short));
            memset(&enc_picture->motion.ref_idx[LIST_1][block_y][img->block_x + i0], bwref, (i1 - i0) * sizeof(char));
            for (block_x = img->block_x + i0; block_x < img->block_x + i1; block_x++)
            {
              enc_picture->motion.ref_pic_id[LIST_1][block_y][block_x] = ref_pic_num ;
            }
          }
        }
      }  
    }
    else
    {
      for (j=j0; j<j1; j++)
      {
        block_y = img->block_y + j;
        memset(enc_picture->motion.mv[LIST_1][block_y][img->block_x + i0], 0, 2 * (i1 - i0) * sizeof(short));
        memset(&enc_picture->motion.ref_idx[LIST_1][block_y][img->block_x + i0], -1, (i1 - i0)  * sizeof(char));
        for (block_x = img->block_x + i0; block_x < img->block_x + i1; block_x++)
        {
          enc_picture->motion.ref_pic_id[LIST_1][block_y][block_x]    = -1;
        }
      }
    }
  }
}

/*!
 *************************************************************************************
 * \brief
 *    skip macroblock field inference
 * \return
 *    inferred field flag
 *************************************************************************************
 */
byte field_flag_inference(Macroblock *currMB)
{
  byte mb_field;

  if (currMB->mbAvailA)
  {
    mb_field = img->mb_data[currMB->mbAddrA].mb_field;
  }
  else
  {
    // check top macroblock pair
    if (currMB->mbAvailB)
      mb_field = img->mb_data[currMB->mbAddrB].mb_field;
    else
      mb_field = 0;
  }

  return mb_field;
}

/*!
 *************************************************************************************
 * \brief
 *    Store motion vectors for 8x8 partition
 *************************************************************************************
 */

void StoreMVBlock8x8(int dir, int block8x8, int mode, int l0_ref, int l1_ref, int pdir8, int bipred_me, int bframe)
{
  int j, i0, j0, ii, jj;
  short ******all_mv  = img->all_mv;
  short ******pred_mv = img->pred_mv;
  short (*lc_l0_mv8x8)[4][2] = all_mv8x8[dir][LIST_0];
  short (*lc_l1_mv8x8)[4][2] = all_mv8x8[dir][LIST_1];
  short (*lc_pr_mv8x8)[4][2] = NULL;

  if (bframe && bipred_me)
  {
    all_mv = img->bipred_mv[bipred_me - 1];
  }

  i0 = (block8x8 & 0x01) << 1;
  j0 = (block8x8 >> 1) << 1;
  ii = i0+2;
  jj = j0+2;

  if (!bframe)
  {
    if (pdir8>=0) //(mode8!=IBLOCK)&&(mode8!=I16MB))  // && ref != -1)
    {
      lc_pr_mv8x8 = pred_mv8x8[dir][LIST_0];
      for (j=j0; j<jj; j++)
      {
        memcpy(&lc_l0_mv8x8[j][i0][0], &all_mv[LIST_0][l0_ref][4][j][i0][0],  (ii - i0) * 2 * sizeof(short));
        memcpy(&lc_pr_mv8x8[j][i0][0], &pred_mv[LIST_0][l0_ref][4][j][i0][0], (ii - i0) * 2 * sizeof(short));
      }
    }
  }
  else
  {
    if (pdir8 == 0) // list0
    {
      lc_pr_mv8x8 = pred_mv8x8[dir][LIST_0];
      for (j=j0; j<jj; j++)
      {
        memcpy(&lc_l0_mv8x8[j][i0][0], &all_mv[LIST_0][l0_ref][mode][j][i0][0],  (ii - i0) * 2 * sizeof(short));
        memcpy(&lc_pr_mv8x8[j][i0][0], &pred_mv[LIST_0][l0_ref][mode][j][i0][0], (ii - i0) * 2 * sizeof(short));
      }
    }
    else if (pdir8 == 1) // list1
    {
      lc_pr_mv8x8 = pred_mv8x8[dir][LIST_1];
      for (j=j0; j<jj; j++)
      {
        memcpy(&lc_l1_mv8x8[j][i0][0], &all_mv[LIST_1][l1_ref][mode][j][i0][0],  (ii - i0) * 2 * sizeof(short));
        memcpy(&lc_pr_mv8x8[j][i0][0], &pred_mv[LIST_1][l1_ref][mode][j][i0][0], (ii - i0) * 2 * sizeof(short));
      }
    }
    else if (pdir8==2) // bipred
    {
      lc_pr_mv8x8 = pred_mv8x8[dir][LIST_0];
      for (j=j0; j<jj; j++)
      {
        memcpy(&lc_l0_mv8x8[j][i0][0], &all_mv[LIST_0][l0_ref][mode][j][i0][0],  (ii - i0) * 2 * sizeof(short));
        memcpy(&lc_pr_mv8x8[j][i0][0], &pred_mv[LIST_0][l0_ref][mode][j][i0][0], (ii - i0) * 2 * sizeof(short));
      }
      lc_pr_mv8x8 = pred_mv8x8[dir][LIST_1];
      for (j=j0; j<jj; j++)
      {        
        memcpy(&lc_l1_mv8x8[j][i0][0], &all_mv[LIST_1][l1_ref][mode][j][i0][0],  (ii - i0) * 2 * sizeof(short));
        memcpy(&lc_pr_mv8x8[j][i0][0], &pred_mv[LIST_1][l1_ref][mode][j][i0][0], (ii - i0) * 2 * sizeof(short));
      }
    }
    else
    {
      error("invalid direction mode", 255);
    }
  }
}



/*!
 *************************************************************************************
 * \brief
 *    Store motion vectors of 8x8 partitions of one macroblock
 *************************************************************************************
 */
void StoreMV8x8(int dir)
{
  int block8x8;

  int bframe = (img->type == B_SLICE);

  for (block8x8=0; block8x8<4; block8x8++)
    StoreMVBlock8x8(dir, block8x8, tr8x8.part8x8mode[block8x8], tr8x8.part8x8l0ref[block8x8],
    tr8x8.part8x8l1ref[block8x8], tr8x8.part8x8pdir[block8x8], tr8x8.part8x8bipred[block8x8], bframe);
}

/*!
*************************************************************************************
* \brief
*    Restore motion vectors for 8x8 partition
*************************************************************************************
*/
void RestoreMVBlock8x8(int dir, int block8x8, RD_8x8DATA tr, int bframe)
{
  int j, i0, j0, ii, jj;
  short ******all_mv  = img->all_mv;
  short ******pred_mv = img->pred_mv;
  short (*lc_l0_mv8x8)[4][2] = all_mv8x8[dir][LIST_0];
  short (*lc_l1_mv8x8)[4][2] = all_mv8x8[dir][LIST_1];
  short (*lc_pr_mv8x8)[4][2] = NULL;

  short pdir8     = tr.part8x8pdir [block8x8];
  short mode      = tr.part8x8mode [block8x8];
  short l0_ref    = tr.part8x8l0ref[block8x8];
  short l1_ref    = tr.part8x8l1ref[block8x8];
  short bipred_me = tr.part8x8bipred[block8x8];

  if(bframe && bipred_me)
  {
    all_mv = img->bipred_mv[bipred_me - 1];
  }

  i0 = (block8x8 & 0x01) << 1;
  j0 = (block8x8 >> 1) << 1;
  ii = i0+2;
  jj = j0+2;

  if (!bframe)
  {
    if (pdir8>=0) //(mode8!=IBLOCK)&&(mode8!=I16MB))  // && ref != -1)
    {
      lc_pr_mv8x8 = pred_mv8x8[dir][LIST_0];
      for (j=j0; j<jj; j++)
      {
        memcpy(&all_mv[LIST_0][l0_ref][4][j][i0][0],  &lc_l0_mv8x8[j][i0][0], 2 * 2 * sizeof(short));
        memcpy(&pred_mv[LIST_0][l0_ref][4][j][i0][0], &lc_pr_mv8x8[j][i0][0], 2 * 2 * sizeof(short));
      }
    }
  }
  else
  {
    if (pdir8==0) // forward
    {
      lc_pr_mv8x8 = pred_mv8x8[dir][LIST_0];
      for (j=j0; j<jj; j++)
      {
        memcpy(&all_mv[LIST_0][l0_ref][mode][j][i0][0],  &lc_l0_mv8x8[j][i0][0], 2 * 2 * sizeof(short));
        memcpy(&pred_mv[LIST_0][l0_ref][mode][j][i0][0], &lc_pr_mv8x8[j][i0][0], 2 * 2 * sizeof(short));
      }
    }
    else if (pdir8==1) // backward
    {
      lc_pr_mv8x8 = pred_mv8x8[dir][LIST_1];
      for (j=j0; j<jj; j++)
      {
        memcpy(&all_mv[LIST_1][l1_ref][mode][j][i0][0],  &lc_l1_mv8x8[j][i0][0], 2 * 2 * sizeof(short));
        memcpy(&pred_mv[LIST_1][l1_ref][mode][j][i0][0], &lc_pr_mv8x8[j][i0][0], 2 * 2 * sizeof(short));
      }
    }
    else if (pdir8==2) // bidir
    {
      lc_pr_mv8x8 = pred_mv8x8[dir][LIST_0];
      for (j=j0; j<jj; j++)
      {
        memcpy(&all_mv[LIST_0][l0_ref][mode][j][i0][0],  &lc_l0_mv8x8[j][i0][0], (ii - i0) * 2 * sizeof(short));
        memcpy(&pred_mv[LIST_0][l0_ref][mode][j][i0][0], &lc_pr_mv8x8[j][i0][0], (ii - i0) * 2 * sizeof(short));
      }
      
      lc_pr_mv8x8 = pred_mv8x8[dir][LIST_1];
      for (j=j0; j<jj; j++)
      {        
        memcpy(&all_mv[LIST_1][l1_ref][mode][j][i0][0],  &lc_l1_mv8x8[j][i0][0], (ii - i0) * 2 * sizeof(short));
        memcpy(&pred_mv[LIST_1][l1_ref][mode][j][i0][0], &lc_pr_mv8x8[j][i0][0], (ii - i0) * 2 * sizeof(short));
      }
    }
    else
    {
      error("invalid direction mode", 255);
    }
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Restore motion vectors of 8x8 partitions of one macroblock
 *************************************************************************************
 */
void RestoreMV8x8(int dir)
{
  int block8x8;

  int bframe = (img->type == B_SLICE);

  for (block8x8=0; block8x8<4; block8x8++)
    RestoreMVBlock8x8(dir, block8x8, tr8x8, bframe);
}


/*!
 *************************************************************************************
 * \brief
 *    Store predictors for 8x8 partition
 *************************************************************************************
 */

void StoreNewMotionVectorsBlock8x8(int dir, int block8x8, int mode, int l0_ref, int l1_ref, int pdir8, int bipred_me, int bframe)
{
  int j, i0, j0, ii, jj;
  short ******all_mv  = img->all_mv;
  short ******pred_mv = img->pred_mv;
  short (*lc_l0_mv8x8)[4][2] = all_mv8x8[dir][LIST_0];
  short (*lc_l1_mv8x8)[4][2] = all_mv8x8[dir][LIST_1];
  short (*lc_pr_mv8x8)[4][2] = NULL;
  
  if (bframe && bipred_me)
  {
    all_mv  = img->bipred_mv[bipred_me - 1];
  }

  i0 = (block8x8 & 0x01) << 1;
  j0 = (block8x8 >> 1) << 1;
  ii = i0+2;
  jj = j0+2;

  if (pdir8<0)
  {
    for (j=j0; j<jj; j++)
    {
      memset(&lc_l0_mv8x8[j][i0], 0, 4 * sizeof(short));
    }
    for (j=j0; j<jj; j++)
    {
      memset(&lc_l1_mv8x8[j][i0], 0, 4 * sizeof(short));
    }
    return;
  }

  if (!bframe)
  {

    lc_pr_mv8x8 = pred_mv8x8[dir][LIST_0];
    for (j=j0; j<jj; j++)
    {
      memcpy(&lc_l0_mv8x8[j][i0][0], &all_mv [LIST_0][l0_ref][4][j][i0][0], 4 * sizeof(short));
      memcpy(&lc_pr_mv8x8[j][i0][0], &pred_mv[LIST_0][l0_ref][4][j][i0][0], 4 * sizeof(short));
    }

    
    for (j=j0; j<jj; j++)
    {
      memset(&lc_l1_mv8x8[j][i0], 0, 4 * sizeof(short));
    }
    return;
  }
  else
  {
    if ((pdir8==0 || pdir8==2))
    {
      lc_pr_mv8x8 = pred_mv8x8[dir][LIST_0];

      for (j=j0; j<jj; j++)
      {
        memcpy(&lc_l0_mv8x8[j][i0][0], &all_mv[LIST_0][l0_ref][mode][j][i0][0],  4 * sizeof(short));
        memcpy(&lc_pr_mv8x8[j][i0][0], &pred_mv[LIST_0][l0_ref][mode][j][i0][0], 4 * sizeof(short));
      }
    }
    else
    {
      for (j=j0; j<jj; j++)
        memset(&lc_l0_mv8x8[j][i0], 0, 4 * sizeof(short));
    }

    if ((pdir8==1 || pdir8==2))
    {
      lc_pr_mv8x8 = pred_mv8x8[dir][LIST_1];

      for (j=j0; j<jj; j++)
      {
        memcpy(&lc_l1_mv8x8[j][i0][0], &all_mv[LIST_1][l1_ref][mode][j][i0][0],  4 * sizeof(short));
        memcpy(&lc_pr_mv8x8[j][i0][0], &pred_mv[LIST_1][l1_ref][mode][j][i0][0], 4 * sizeof(short));
      }
    }
    else
    {
      for (j=j0; j<jj; j++)
        memset(&lc_l1_mv8x8[j][i0], 0, 4 * sizeof(short));
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Makes the decision if 8x8 tranform will be used (for RD-off)
 ************************************************************************
 */
int GetBestTransformP8x8()
{
  int    block_y, block_x, pic_pix_y, pic_pix_x, i, j, k;
  int    mb_y, mb_x, block8x8;
  int    cost8x8=0, cost4x4=0;
  int    *diff_ptr;

  if(params->Transform8x8Mode==2) //always allow 8x8 transform
    return 1;

  for (block8x8=0; block8x8<4; block8x8++)
  {
    mb_y = (block8x8 >>   1) << 3;
    mb_x = (block8x8 & 0x01) << 3;
    //===== loop over 4x4 blocks =====
    k=0;
    for (block_y = mb_y; block_y < mb_y + 8; block_y += 4)
    {
      pic_pix_y = img->opix_y + block_y;

      //get cost for transform size 4x4
      for (block_x = mb_x; block_x<mb_x + 8; block_x += 4)
      {
        pic_pix_x = img->opix_x + block_x;

        //===== get displaced frame difference ======
        diff_ptr=&diff4x4[k];
        for (j=0; j<4; j++)
        {
          for (i=0; i<4; i++, k++)
          {
            //4x4 transform size
            diff4x4[k] = pCurImg[pic_pix_y+j][pic_pix_x+i] - tr4x4.mpr8x8[j+block_y][i+block_x];
            //8x8 transform size
            diff8x8[k] = pCurImg[pic_pix_y+j][pic_pix_x+i] - tr8x8.mpr8x8[j+block_y][i+block_x];
          }
        }

        cost4x4 += distortion4x4 (diff_ptr);
      }
    }
    cost8x8 += distortion8x8 (diff8x8);
  }
  return (cost8x8 < cost4x4);
}

/*!
************************************************************************
* \brief
*    Sets MBAFF RD parameters
************************************************************************
*/
void set_mbaff_parameters(Macroblock  *currMB)
{
  int  j;
  int  mode         = best_mode;
  int  bframe       = (img->type==B_SLICE);
  char **ipredmodes = img->ipredmode;


  //===== reconstruction values =====
  for (j=0; j < MB_BLOCK_SIZE; j++)
    memcpy(rdopt->rec_mbY[j],&enc_picture->imgY[img->pix_y + j][img->pix_x], MB_BLOCK_SIZE * sizeof(imgpel));

  if (img->yuv_format != YUV400)
  {
    for (j=0; j<img->mb_cr_size_y; j++)
    {
      memcpy(rdopt->rec_mb_cr[0][j], &enc_picture->imgUV[0][img->pix_c_y + j][img->pix_c_x], img->mb_cr_size_x * sizeof(imgpel));
      memcpy(rdopt->rec_mb_cr[1][j], &enc_picture->imgUV[1][img->pix_c_y + j][img->pix_c_x], img->mb_cr_size_x * sizeof(imgpel));
    }
  }

  //===== coefficients and cbp =====
  rdopt->mode      = mode;
  rdopt->i16offset = img->i16offset;
  rdopt->cbp       = currMB->cbp;
  rdopt->cbp_blk   = currMB->cbp_blk;
  rdopt->mb_type   = currMB->mb_type;

  rdopt->luma_transform_size_8x8_flag = currMB->luma_transform_size_8x8_flag;

  if(rdopt->mb_type == 0 && mode != 0)
  {
    mode=0;
    rdopt->mode=0;
  }

  memcpy(rdopt->cofAC[0][0][0], img->cofAC[0][0][0], (4+img->num_blk8x8_uv) * 4 * 2 * 65 * sizeof(int));
  memcpy(rdopt->cofDC[0][0], img->cofDC[0][0], 3 * 2 * 18 * sizeof(int));

  memcpy(rdopt->b8mode, currMB->b8mode, BLOCK_MULTIPLE * sizeof(short));
  memcpy(rdopt->b8pdir, currMB->b8pdir, BLOCK_MULTIPLE * sizeof(short));

  //==== reference frames =====
  if (bframe)
  {
    if (params->BiPredMERefinements == 1)
    {      
      int i, j, k;
      for (j = 0; j < BLOCK_MULTIPLE; j++)
      {
        for (i = 0; i < BLOCK_MULTIPLE; i++)
        {
          k = 2*(j >> 1)+(i >> 1);
          if (currMB->bipred_me[k] == 0)
          {
            rdopt->refar[LIST_0][j][i] = enc_picture->motion.ref_idx[LIST_0][img->block_y + j][img->block_x + i];
            rdopt->refar[LIST_1][j][i] = enc_picture->motion.ref_idx[LIST_1][img->block_y + j][img->block_x + i];
          }
          else
          {
            rdopt->refar[LIST_0][j][i] = 0;
            rdopt->refar[LIST_1][j][i] = 0;
          }
        }
      }
    }
    else
    {
      for (j = 0; j < BLOCK_MULTIPLE; j++)
      {
        memcpy(rdopt->refar[LIST_0][j],&enc_picture->motion.ref_idx[LIST_0][img->block_y + j][img->block_x] , BLOCK_MULTIPLE * sizeof(char));
        memcpy(rdopt->refar[LIST_1][j],&enc_picture->motion.ref_idx[LIST_1][img->block_y + j][img->block_x] , BLOCK_MULTIPLE * sizeof(char));
      }
    }
  }
  else
  {
    for (j = 0; j < BLOCK_MULTIPLE; j++)
      memcpy(rdopt->refar[LIST_0][j],&enc_picture->motion.ref_idx[LIST_0][img->block_y + j][img->block_x] , BLOCK_MULTIPLE * sizeof(char));
  }

  memcpy(rdopt->intra_pred_modes,currMB->intra_pred_modes, MB_BLOCK_PARTITIONS * sizeof(char));
  memcpy(rdopt->intra_pred_modes8x8,currMB->intra_pred_modes8x8, MB_BLOCK_PARTITIONS * sizeof(char));
  for (j = img->block_y; j < img->block_y + 4; j++)
  {
    memcpy(&rdopt->ipredmode[j][img->block_x],&ipredmodes[j][img->block_x], BLOCK_MULTIPLE * sizeof(char));
  }
}

/*!
************************************************************************
* \brief
*    store coding state (for rd-optimized mode decision), used for 8x8 transformation
************************************************************************
*/
void store_coding_state_cs_cm(Macroblock *currMB)
{
  store_coding_state(currMB, cs_cm);
}

/*!
************************************************************************
* \brief
*    restore coding state (for rd-optimized mode decision), used for 8x8 transformation
************************************************************************
*/
void reset_coding_state_cs_cm(Macroblock *currMB)
{
  reset_coding_state(currMB, cs_cm);
}

void assign_enc_picture_params(int mode, char best_pdir, int block, int list_offset, int best_l0_ref, int best_l1_ref, int bframe, short bipred_me)
{
  int i,j;
  int block_x, block_y;
  int list, maxlist, bestref;
  short ***curr_mv = NULL;
  int64 curr_ref_idx = 0;
 
  int start_x = 0, start_y = 0, end_x = BLOCK_MULTIPLE, end_y = BLOCK_MULTIPLE; 
  switch (mode)
  {
    case 1:
      start_x = 0;
      start_y = 0;
      end_x   = BLOCK_MULTIPLE;
      end_y   = BLOCK_MULTIPLE;
      break;
    case 2:
      start_x = 0;
      start_y = block * 2;
      end_x   = BLOCK_MULTIPLE;
      end_y   = (block + 1) * 2;
      break;
    case 3:
      start_x = block * 2;
      start_y = 0;
      end_x   = (block + 1) * 2;
      end_y   = BLOCK_MULTIPLE;
      break;
    default:
      break;
  }

  maxlist  = bframe ? 1: 0;
  for (list = 0; list <= maxlist; list++)
  {
    bestref = (list == 0) ? best_l0_ref : best_l1_ref;
    switch (bipred_me)
    {
      case 0:
        curr_mv = img->all_mv[list][bestref][mode];
        curr_ref_idx = enc_picture->ref_pic_num[list + list_offset][bestref];
        break;
      case 1:
        curr_mv = img->bipred_mv[0][list][0][mode] ; //best_l0_ref has to be zero in this case
        curr_ref_idx = enc_picture->ref_pic_num[list + list_offset][0];
        break;
      case 2:
        curr_mv = img->bipred_mv[1][list][0][mode] ; //best_l0_ref has to be zero in this case
        curr_ref_idx = enc_picture->ref_pic_num[list + list_offset][0];
        break;
      default:
        break;
    }

    for (j = start_y; j < end_y; j++)
    {
      block_y = img->block_y + j;
      for (i = start_x; i < end_x; i++)
      {
        block_x = img->block_x + i;
        if ((best_pdir != 2) && (best_pdir != list))
        {
            enc_picture->motion.ref_idx    [list][block_y][block_x]    = -1;
            enc_picture->motion.ref_pic_id [list][block_y][block_x]    = -1;
            enc_picture->motion.mv         [list][block_y][block_x][0] = 0;
            enc_picture->motion.mv         [list][block_y][block_x][1] = 0;
        }
        else
      {
          enc_picture->motion.ref_pic_id [list][block_y][block_x]    = curr_ref_idx;
          enc_picture->motion.mv         [list][block_y][block_x][0] = curr_mv[j][i][0];
          enc_picture->motion.mv         [list][block_y][block_x][1] = curr_mv[j][i][1];
        }
      }
    }
  }
}


/*!
 *************************************************************************************
 * \brief
 *    Set block 8x8 mode information
 *************************************************************************************
 */
void set_block8x8_info(Block8x8Info *b8x8info, int mode, int block,  char best_ref[2], char best_pdir, short bipred_me)
{
  int i;
  //----- set reference frame and direction parameters -----
  if (mode==3)
  {
    b8x8info->best8x8l0ref [3][block  ] = b8x8info->best8x8l0ref [3][  block+2] = best_ref[LIST_0];
    b8x8info->best8x8pdir  [3][block  ] = b8x8info->best8x8pdir  [3][  block+2] = best_pdir;
    b8x8info->best8x8l1ref [3][block  ] = b8x8info->best8x8l1ref [3][  block+2] = best_ref[LIST_1];
    b8x8info->bipred8x8me  [3][block  ] = b8x8info->bipred8x8me  [3][  block+2] = bipred_me;
  }
  else if (mode==2)
  {
    b8x8info->best8x8l0ref [2][2*block] = b8x8info->best8x8l0ref [2][2*block+1] = best_ref[LIST_0];
    b8x8info->best8x8pdir  [2][2*block] = b8x8info->best8x8pdir  [2][2*block+1] = best_pdir;
    b8x8info->best8x8l1ref [2][2*block] = b8x8info->best8x8l1ref [2][2*block+1] = best_ref[LIST_1];
    b8x8info->bipred8x8me  [2][2*block] = b8x8info->bipred8x8me  [2][2*block+1] = bipred_me;
  }
  else
  {
    memset(&b8x8info->best8x8l0ref [1][0], best_ref[LIST_0], 4 * sizeof(char));
    memset(&b8x8info->best8x8l1ref [1][0], best_ref[LIST_1], 4 * sizeof(char));
    memset(&b8x8info->best8x8pdir  [1][0], best_pdir, 4 * sizeof(char));
    for (i = 0; i< 4; i++)
      b8x8info->bipred8x8me  [1][i] =  bipred_me;
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Set block 8x8 mode information for P8x8 mode
 *************************************************************************************
 */
void set_subblock8x8_info(Block8x8Info *b8x8info,int mode, int block, RD_8x8DATA *tr)
{          
  b8x8info->best8x8mode         [block] = tr->part8x8mode  [block];
  b8x8info->best8x8pdir   [mode][block] = tr->part8x8pdir  [block];
  b8x8info->best8x8l0ref  [mode][block] = tr->part8x8l0ref [block];
  b8x8info->best8x8l1ref  [mode][block] = tr->part8x8l1ref [block];
  b8x8info->bipred8x8me   [mode][block] = tr->part8x8bipred[block];
}



void update_refresh_map(int intra, int intra1, Macroblock *currMB)
{
  if (params->RestrictRef==1)
  {
    // Modified for Fast Mode Decision. Inchoon Choi, SungKyunKwan Univ.
    if (params->rdopt<2)
    {
      refresh_map[2*img->mb_y  ][2*img->mb_x  ] = (intra ? 1 : 0);
      refresh_map[2*img->mb_y  ][2*img->mb_x+1] = (intra ? 1 : 0);
      refresh_map[2*img->mb_y+1][2*img->mb_x  ] = (intra ? 1 : 0);
      refresh_map[2*img->mb_y+1][2*img->mb_x+1] = (intra ? 1 : 0);
    }
    else if (params->rdopt == 3)
    {
      refresh_map[2*img->mb_y  ][2*img->mb_x  ] = (intra1==0 && (currMB->mb_type==I16MB || currMB->mb_type==I4MB) ? 1 : 0);
      refresh_map[2*img->mb_y  ][2*img->mb_x+1] = (intra1==0 && (currMB->mb_type==I16MB || currMB->mb_type==I4MB) ? 1 : 0);
      refresh_map[2*img->mb_y+1][2*img->mb_x  ] = (intra1==0 && (currMB->mb_type==I16MB || currMB->mb_type==I4MB) ? 1 : 0);
      refresh_map[2*img->mb_y+1][2*img->mb_x+1] = (intra1==0 && (currMB->mb_type==I16MB || currMB->mb_type==I4MB) ? 1 : 0);
    }
  }
  else if (params->RestrictRef==2)
  {
    refresh_map[2*img->mb_y  ][2*img->mb_x  ] = (currMB->mb_type==I16MB || currMB->mb_type==I4MB ? 1 : 0);
    refresh_map[2*img->mb_y  ][2*img->mb_x+1] = (currMB->mb_type==I16MB || currMB->mb_type==I4MB ? 1 : 0);
    refresh_map[2*img->mb_y+1][2*img->mb_x  ] = (currMB->mb_type==I16MB || currMB->mb_type==I4MB ? 1 : 0);
    refresh_map[2*img->mb_y+1][2*img->mb_x+1] = (currMB->mb_type==I16MB || currMB->mb_type==I4MB ? 1 : 0);
  }
}

int valid_intra_mode(int ipmode)
{
  if (params->IntraDisableInterOnly==0 || img->type != I_SLICE)
  {
    if (params->Intra4x4ParDisable && (ipmode==VERT_PRED||ipmode==HOR_PRED))
      return 0;

    if (params->Intra4x4DiagDisable && (ipmode==DIAG_DOWN_LEFT_PRED||ipmode==DIAG_DOWN_RIGHT_PRED))
      return 0;

    if (params->Intra4x4DirDisable && ipmode>=VERT_RIGHT_PRED)
      return 0;
  }
  return 1;
}

void compute_comp_cost(imgpel **cur_img, imgpel prd_img[16][16], int pic_opix_x, int *cost)
{
  int j, i, *d;
  imgpel *cur_line, *prd_line;
  d = &diff[0];

  for (j = 0; j < BLOCK_SIZE; j++)
  {
    cur_line = &cur_img[j][pic_opix_x];
    prd_line = prd_img[j];

    for (i = 0; i < BLOCK_SIZE; i++)
    {
      *d++ = *cur_line++ - *prd_line++;
    }
  }      
  *cost += distortion4x4 (diff);
}


void generate_pred_error(imgpel **cur_img, imgpel prd_img[16][16], imgpel cur_prd[16][16], 
                         int m7[16][16], int pic_opix_x, int block_x)
{
  int j, i, *m7_line;
  imgpel *cur_line, *prd_line;

  for (j = 0; j < BLOCK_SIZE; j++)
  {
    m7_line = &m7[j][block_x];
    cur_line = &cur_img[j][pic_opix_x];
    prd_line = prd_img[j];
    memcpy(&cur_prd[j][block_x], prd_line, BLOCK_SIZE * sizeof(imgpel));

    for (i = 0; i < BLOCK_SIZE; i++)
    {
      *m7_line++ = (int) (*cur_line++ - *prd_line++);
    }
  }        
}





