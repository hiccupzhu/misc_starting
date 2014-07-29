/*!
 ***************************************************************************
 * \file transform8x8.c
 *
 * \brief
 *    8x8 transform functions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Yuri Vatis
 *    - Jan Muenster
 *    - Lowell Winger                   <lwinger@lsil.com>
 * \date
 *    12. October 2003
 **************************************************************************
 */

#include <math.h>
#include <limits.h>

#include "global.h"

#include "image.h"
#include "mb_access.h"
#include "elements.h"
#include "cabac.h"
#include "vlc.h"
#include "transform8x8.h"
#include "transform.h"
#include "macroblock.h"
#include "symbol.h"
#include "mc_prediction.h"
#include "quant8x8.h"
#include "rdoq.h"
#include "q_matrix.h"
#include "q_offsets.h"

int   cofAC8x8_chroma[2][4][2][18];
static int diff64[64];

//! single scan pattern
//static const byte SNGL_SCAN8x8[64][2] = {
const byte SNGL_SCAN8x8[64][2] = {
  {0,0}, {1,0}, {0,1}, {0,2}, {1,1}, {2,0}, {3,0}, {2,1},
  {1,2}, {0,3}, {0,4}, {1,3}, {2,2}, {3,1}, {4,0}, {5,0},
  {4,1}, {3,2}, {2,3}, {1,4}, {0,5}, {0,6}, {1,5}, {2,4},
  {3,3}, {4,2}, {5,1}, {6,0}, {7,0}, {6,1}, {5,2}, {4,3},
  {3,4}, {2,5}, {1,6}, {0,7}, {1,7}, {2,6}, {3,5}, {4,4},
  {5,3}, {6,2}, {7,1}, {7,2}, {6,3}, {5,4}, {4,5}, {3,6},
  {2,7}, {3,7}, {4,6}, {5,5}, {6,4}, {7,3}, {7,4}, {6,5},
  {5,6}, {4,7}, {5,7}, {6,6}, {7,5}, {7,6}, {6,7}, {7,7}
};


//! field scan pattern
//static const byte FIELD_SCAN8x8[64][2] = {   // 8x8
const byte FIELD_SCAN8x8[64][2] = {   // 8x8
  {0,0}, {0,1}, {0,2}, {1,0}, {1,1}, {0,3}, {0,4}, {1,2},
  {2,0}, {1,3}, {0,5}, {0,6}, {0,7}, {1,4}, {2,1}, {3,0},
  {2,2}, {1,5}, {1,6}, {1,7}, {2,3}, {3,1}, {4,0}, {3,2},
  {2,4}, {2,5}, {2,6}, {2,7}, {3,3}, {4,1}, {5,0}, {4,2},
  {3,4}, {3,5}, {3,6}, {3,7}, {4,3}, {5,1}, {6,0}, {5,2},
  {4,4}, {4,5}, {4,6}, {4,7}, {5,3}, {6,1}, {6,2}, {5,4},
  {5,5}, {5,6}, {5,7}, {6,3}, {7,0}, {7,1}, {6,4}, {6,5},
  {6,6}, {6,7}, {7,2}, {7,3}, {7,4}, {7,5}, {7,6}, {7,7}
};


//! array used to find expensive coefficients
static const byte COEFF_COST8x8[2][64] =
{
  {3,3,3,3,2,2,2,2,2,2,2,2,1,1,1,1,
   1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9}
};

static int **levelscale = NULL, **leveloffset = NULL;
static int **invlevelscale = NULL;
static int **fadjust8x8 = NULL;


/*!
************************************************************************
* \brief
*    Residual DPCM for Intra lossless coding
*
* \par Input:
*    block_x,block_y: Block position inside a macro block (0,8).
************************************************************************
*/
//For residual DPCM
int Residual_DPCM_8x8(int ipmode, int ores[16][16], int rres[16][16],int block_y, int block_x)
{
  int i,j;
  int temp[8][8];

  if(ipmode==VERT_PRED)
  { 

    for (j=0; j<8; j++)
     temp[0][j] = ores[block_y][block_x+j];

    for (i=1; i<8; i++) 
      for (j=0; j<8; j++)
        temp[i][j] =  ores[block_y+i][block_x+j] - ores[block_y+i-1][block_x+j];

    for (i = 0; i < 8; i++)
      for (j = 0; j < 8; j++)
        rres[block_y+i][block_x+j] = temp[i][j];
  }
  else  //HOR_PRED
  {
    for (i=0; i<8; i++)
     temp[i][0] = ores[block_y + i][block_x];

    for (i=0; i<8; i++)
      for (j=1; j<8; j++)
        temp[i][j] = ores[block_y+i][block_x+j] - ores[block_y+i][block_x+j-1];

    for (i=0; i<8; i++)
      for (j=0; j<8; j++)
        rres[block_y+i][block_x+j] = temp[i][j];
  }
  return 0;
}

/*!
************************************************************************
* \brief
*    Inverse residual DPCM for Intra lossless coding
*
* \par Input:
*    block_x,block_y: Block position inside a macro block (0,8).
************************************************************************
*/
//For residual DPCM
int Inv_Residual_DPCM_8x8(int m7[16][16], int block_y, int block_x)  
{
  int i;
  int temp[8][8];

  if(ipmode_DPCM==VERT_PRED)
  {
    for(i=0; i<8; i++)
    {
      temp[0][i] = m7[block_y+0][block_x+i];
      temp[1][i] = temp[0][i] + m7[block_y+1][block_x+i];
      temp[2][i] = temp[1][i] + m7[block_y+2][block_x+i];
      temp[3][i] = temp[2][i] + m7[block_y+3][block_x+i];
      temp[4][i] = temp[3][i] + m7[block_y+4][block_x+i];
      temp[5][i] = temp[4][i] + m7[block_y+5][block_x+i];
      temp[6][i] = temp[5][i] + m7[block_y+6][block_x+i];
      temp[7][i] = temp[6][i] + m7[block_y+7][block_x+i];
    }
    for(i=0; i<8; i++)
    {
      m7[block_y+1][block_x+i] = temp[1][i];
      m7[block_y+2][block_x+i] = temp[2][i];
      m7[block_y+3][block_x+i] = temp[3][i];
      m7[block_y+4][block_x+i] = temp[4][i];
      m7[block_y+5][block_x+i] = temp[5][i];
      m7[block_y+6][block_x+i] = temp[6][i];
      m7[block_y+7][block_x+i] = temp[7][i];
    }
  }
  else //HOR_PRED
  {
    for(i=0; i<8; i++)
    {
      temp[i][0] = m7[block_y+i][block_x+0];
      temp[i][1] = temp[i][0] + m7[block_y+i][block_x+1];
      temp[i][2] = temp[i][1] + m7[block_y+i][block_x+2];
      temp[i][3] = temp[i][2] + m7[block_y+i][block_x+3];
      temp[i][4] = temp[i][3] + m7[block_y+i][block_x+4];
      temp[i][5] = temp[i][4] + m7[block_y+i][block_x+5];
      temp[i][6] = temp[i][5] + m7[block_y+i][block_x+6];
      temp[i][7] = temp[i][6] + m7[block_y+i][block_x+7];
    }
    for(i=0; i<8; i++)
    {
      m7[block_y+i][block_x+1] = temp[i][1];
      m7[block_y+i][block_x+2] = temp[i][2];
      m7[block_y+i][block_x+3] = temp[i][3];
      m7[block_y+i][block_x+4] = temp[i][4];
      m7[block_y+i][block_x+5] = temp[i][5];
      m7[block_y+i][block_x+6] = temp[i][6];
      m7[block_y+i][block_x+7] = temp[i][7];
    }
  }
  return 0;
}

/*!
 *************************************************************************************
 * \brief
 *    8x8 Intra mode decision for a macroblock
 *************************************************************************************
 */

int Mode_Decision_for_new_Intra8x8Macroblock (Macroblock *currMB, double lambda, double *min_cost)
{
  int cur_cbp = 0, b8;
  double cost8x8;
  int cr_cbp[3] = { 0, 0, 0}; 

  *min_cost = (int)floor(6.0 * lambda + 0.4999);

  cmp_cbp[1] = cmp_cbp[2] = 0;
  if (params->rdopt == 0)
    Mode_Decision_for_new_8x8IntraBlocks = Mode_Decision_for_new_8x8IntraBlocks_JM_Low;
  else
    Mode_Decision_for_new_8x8IntraBlocks = Mode_Decision_for_new_8x8IntraBlocks_JM_High;
  for (b8=0; b8<4; b8++)
  {
    if (Mode_Decision_for_new_8x8IntraBlocks (currMB, b8, lambda, &cost8x8, cr_cbp))
    {
      cur_cbp |= (1<<b8);
    }
    *min_cost += cost8x8;

    if (img->P444_joined)
    {
      int k;
      for (k = 1; k < 3; k++)
      {
        if (cr_cbp[k])
        {
          cmp_cbp[k] |= (1<<b8);
          cur_cbp |= cmp_cbp[k];
          cmp_cbp[k] = cur_cbp;
        }
      }
    }
  }

  return cur_cbp;
}

/*!
 *************************************************************************************
 * \brief
 *    8x8 Intra mode decision for a macroblock - Low complexity
 *************************************************************************************
 */

int Mode_Decision_for_new_8x8IntraBlocks_JM_Low (Macroblock *currMB, int b8, double lambda, double *min_cost, int cr_cbp[3])
{
  int     ipmode, best_ipmode = 0, i, j, dummy;
  double  cost;
  int     nonzero = 0;
  int     block_x     = (b8 & 0x01) << 3;
  int     block_y     = (b8 >> 1) << 3;
  int     pic_pix_x   = img->pix_x + block_x;
  int     pic_pix_y   = img->pix_y + block_y;
  int     pic_opix_x   = img->opix_x + block_x;
  int     pic_opix_y   = img->opix_y + block_y;
  int     pic_block_x = pic_pix_x >> 2;
  int     pic_block_y = pic_pix_y >> 2;
  imgpel    *img_org, *img_prd;
  int       *residual;
  int left_available, up_available, all_available;
  int    (*mb_ores)[16] = img->mb_ores[0]; 
  imgpel (*mb_pred)[16] = img->mb_pred[0];
  int *mb_size = img->mb_size[IS_LUMA];

  char   upMode;
  char   leftMode;
  int    mostProbableMode;

  PixelPos left_block;
  PixelPos top_block;

  get4x4Neighbour(currMB, block_x - 1, block_y    , mb_size, &left_block);
  get4x4Neighbour(currMB, block_x,     block_y - 1, mb_size, &top_block );

  if (params->UseConstrainedIntraPred)
  {
    top_block.available  = top_block.available ? img->intra_block [top_block.mb_addr] : 0;
    left_block.available = left_block.available ? img->intra_block [left_block.mb_addr] : 0;
  }

  if(b8 >> 1)
    upMode    =  top_block.available ? img->ipredmode8x8[top_block.pos_y ][top_block.pos_x ] : -1;
  else
    upMode    =  top_block.available ? img->ipredmode   [top_block.pos_y ][top_block.pos_x ] : -1;
  if(b8 & 0x01)
    leftMode  = left_block.available ? img->ipredmode8x8[left_block.pos_y][left_block.pos_x] : -1;
  else
    leftMode  = left_block.available ? img->ipredmode[left_block.pos_y][left_block.pos_x] : -1;

  mostProbableMode  = (upMode < 0 || leftMode < 0) ? DC_PRED : upMode < leftMode ? upMode : leftMode;

  *min_cost = INT_MAX;

  ipmode_DPCM = NO_INTRA_PMODE; //For residual DPCM

  //===== INTRA PREDICTION FOR 8x8 BLOCK =====
  intrapred_8x8 (currMB, PLANE_Y, pic_pix_x, pic_pix_y, &left_available, &up_available, &all_available);

  if(img->P444_joined)
  { 
    select_plane(PLANE_U);
    intrapred_8x8(currMB, PLANE_U, pic_pix_x, pic_pix_y, &left_available, &up_available, &all_available);
    select_plane(PLANE_V);
    intrapred_8x8(currMB, PLANE_V, pic_pix_x, pic_pix_y, &left_available, &up_available, &all_available);
    select_plane(PLANE_Y);
  }

  //===== LOOP OVER ALL 8x8 INTRA PREDICTION MODES =====
  for (ipmode = 0; ipmode < NO_INTRA_PMODE; ipmode++)
  {
    if( (ipmode==DC_PRED) ||
      ((ipmode==VERT_PRED||ipmode==VERT_LEFT_PRED||ipmode==DIAG_DOWN_LEFT_PRED) && up_available ) ||
      ((ipmode==HOR_PRED||ipmode==HOR_UP_PRED) && left_available ) ||
      (all_available) )
    {
      cost  = (ipmode == mostProbableMode) ? 0 : (int)floor(4 * lambda );
      compute_comp_cost8x8(&pImgOrg[0][pic_opix_y], img->mpr_8x8[0][ipmode], pic_opix_x, &cost);
     
      if(img->P444_joined)
      {
        int m;
        for (m = PLANE_U; m <= PLANE_V; m++)
        compute_comp_cost8x8(&pImgOrg[m][pic_opix_y], img->mpr_8x8[m][ipmode], pic_opix_x, &cost);
      }

      if (cost < *min_cost)
      {
        best_ipmode = ipmode;
        *min_cost   = cost;
      }
    }
  }

  //===== set intra mode prediction =====
  img->ipredmode8x8[pic_block_y][pic_block_x] = (char) best_ipmode;
  ipmode_DPCM = best_ipmode; //For residual DPCM

  if(img->P444_joined)
  {
    ColorPlane k;
    CbCr_predmode_8x8[b8] = best_ipmode; 
    for (k = PLANE_U; k <= PLANE_V; k++)
    {
      cr_cbp[k] = 0; 
      select_plane(k);
      for (j=0; j<8; j++)
      {
        for (i=0; i<8; i++)
        {
          img->mb_pred[k][block_y+j][block_x+i]  = img->mpr_8x8[k][best_ipmode][j][i]; 
          img->mb_ores[k][block_y+j][block_x+i]   = pImgOrg[k][img->pix_y+block_y+j][img->pix_x+block_x+i] - img->mpr_8x8[k][best_ipmode][j][i];
        }
      }
      ipmode_DPCM=best_ipmode; 

      if (pDCT_8x8(currMB, k, b8, &dummy, 1))
        cr_cbp[k] = 1;
    }
    select_plane(PLANE_Y);
  }

  currMB->intra_pred_modes8x8[4*b8] = (mostProbableMode == best_ipmode)
    ? -1
    : (best_ipmode < mostProbableMode ? best_ipmode : best_ipmode-1);

  for(j = img->mb_y*4+(b8 >> 1)*2; j < img->mb_y*4+(b8 >> 1)*2 + 2; j++)   //loop 4x4s in the subblock for 8x8 prediction setting
    memset(&img->ipredmode8x8[j][img->mb_x*4+(b8 & 0x01)*2], best_ipmode, 2 * sizeof(char));

  // get prediction and prediction error
  for (j = block_y; j < block_y + 8; j++)
  {
    memcpy(&mb_pred[j][block_x],img->mpr_8x8[0][best_ipmode][j - block_y], 8 * sizeof(imgpel));
    img_org  = &pCurImg[img->opix_y+j][pic_opix_x];
    img_prd  = &mb_pred[j][block_x];
    residual = &mb_ores[j][block_x];
    for (i=0; i<8; i++)
    {
      *residual++ = *img_org++ - *img_prd++;
    }
  }

  ipmode_DPCM = best_ipmode;
  nonzero = pDCT_8x8 (currMB, PLANE_Y, b8, &dummy, 1);    
  return nonzero;
}

/*!
*************************************************************************************
* \brief
*    8x8 Intra mode decision for a macroblock - High complexity
*************************************************************************************
*/

int Mode_Decision_for_new_8x8IntraBlocks_JM_High (Macroblock *currMB, int b8, double lambda, double *min_cost, int cr_cbp[3])
{
  int     ipmode, best_ipmode = 0, i, j, k, y, dummy;
  int     c_nz, nonzero = 0;
  static  imgpel  rec8x8[3][8][8];
  double  rdcost = 0.0;
  int     block_x     = (b8 & 0x01) << 3;
  int     block_y     = (b8 >> 1) << 3;
  int     pic_pix_x   = img->pix_x + block_x;
  int     pic_pix_y   = img->pix_y + block_y;
  int     pic_opix_x   = img->opix_x + block_x;
  int     pic_opix_y   = img->opix_y + block_y;
  int     pic_block_x = pic_pix_x >> 2;
  int     pic_block_y = pic_pix_y >> 2;
  double  min_rdcost  = 1e30;
  imgpel    *img_org, *img_prd;
  int       *residual;
  extern  int ****cofAC8x8;
  static int fadjust8x8[2][16][16];
  static int fadjust8x8Cr[2][2][16][16];
  extern  int ****cofAC8x8CbCr[2];
  int uv, c_nzCbCr[3];
  int left_available, up_available, all_available;
  int    (*mb_ores)[16] = img->mb_ores[0]; 
  imgpel (*mb_pred)[16] = img->mb_pred[0];
  int *mb_size = img->mb_size[IS_LUMA];

  char   upMode;
  char   leftMode;
  int    mostProbableMode;

  PixelPos left_block;
  PixelPos top_block;

  get4x4Neighbour(currMB, block_x - 1, block_y    , mb_size, &left_block);
  get4x4Neighbour(currMB, block_x,     block_y - 1, mb_size, &top_block );

  if (params->UseConstrainedIntraPred)
  {
    top_block.available  = top_block.available ? img->intra_block [top_block.mb_addr] : 0;
    left_block.available = left_block.available ? img->intra_block [left_block.mb_addr] : 0;
  }

  if(b8 >> 1)
    upMode    =  top_block.available ? img->ipredmode8x8[top_block.pos_y ][top_block.pos_x ] : -1;
  else
    upMode    =  top_block.available ? img->ipredmode   [top_block.pos_y ][top_block.pos_x ] : -1;
  if(b8 & 0x01)
    leftMode  = left_block.available ? img->ipredmode8x8[left_block.pos_y][left_block.pos_x] : -1;
  else
    leftMode  = left_block.available ? img->ipredmode[left_block.pos_y][left_block.pos_x] : -1;

  mostProbableMode  = (upMode < 0 || leftMode < 0) ? DC_PRED : upMode < leftMode ? upMode : leftMode;

  *min_cost = INT_MAX;

  ipmode_DPCM = NO_INTRA_PMODE; //For residual DPCM

  //===== INTRA PREDICTION FOR 8x8 BLOCK =====
  intrapred_8x8 (currMB, PLANE_Y, pic_pix_x, pic_pix_y, &left_available, &up_available, &all_available);

  if(img->P444_joined)
  { 
    select_plane(PLANE_U);
    intrapred_8x8(currMB, PLANE_U, pic_pix_x, pic_pix_y, &left_available, &up_available, &all_available);
    select_plane(PLANE_V);
    intrapred_8x8(currMB, PLANE_V, pic_pix_x, pic_pix_y, &left_available, &up_available, &all_available);
    select_plane(PLANE_Y);
  }

  //===== LOOP OVER ALL 8x8 INTRA PREDICTION MODES =====
  for (ipmode = 0; ipmode < NO_INTRA_PMODE; ipmode++)
  {
    if( (ipmode==DC_PRED) ||
      ((ipmode==VERT_PRED||ipmode==VERT_LEFT_PRED||ipmode==DIAG_DOWN_LEFT_PRED) && up_available ) ||
      ((ipmode==HOR_PRED||ipmode==HOR_UP_PRED) && left_available ) ||
      (all_available) )
    {
      // get prediction and prediction error
      for (j=block_y; j < block_y  + 8; j++)
      {
        memcpy(&mb_pred[j][block_x],img->mpr_8x8[0][ipmode][j - block_y], 8 * sizeof(imgpel));
        img_org  = &pCurImg[img->opix_y+j][pic_opix_x];
        img_prd  = &mb_pred[j][block_x];
        residual = &mb_ores[j][block_x];
        for (i=0; i<8; i++)
        {
          *residual++ = *img_org++ - *img_prd++;
        }
      }
      if(img->P444_joined) 
      {
        for (k = PLANE_U; k <= PLANE_V; k++)
        {
          for (j=0; j<8; j++)
          {
            memcpy(&img->mb_pred[k][block_y+j][block_x],img->mpr_8x8[k][ipmode][j], 8 * sizeof(imgpel));
            for (i=0; i<8; i++)
            {
              img->mb_ores[k][block_y+j][block_x+i] = (int) (pImgOrg[k][pic_opix_y+j][pic_opix_x+i] - img->mpr_8x8[k][ipmode][j][i]);
              img->mb_ores[k][block_y+j][block_x+i] = img->mb_ores[k][block_y+j][block_x+i]; // line is temporary until we fix code to use img->mb_ores
            }
          }
        }
      }

      ipmode_DPCM=ipmode;

      //===== store the coding state =====
      // store_coding_state_cs_cm(currMB);
      // get and check rate-distortion cost

      if ((rdcost = RDCost_for_8x8IntraBlocks (currMB, &c_nz, b8, ipmode, lambda, min_rdcost, mostProbableMode, c_nzCbCr)) < min_rdcost)
      {
        //--- set coefficients ---
       memcpy(cofAC8x8[b8][0][0],img->cofAC[b8][0][0], 4 * 2 * 65 * sizeof(int));

        //--- set reconstruction ---
        for (y=0; y<8; y++)
        {
          memcpy(rec8x8[0][y],&enc_picture->imgY[pic_pix_y+y][pic_pix_x], 8 * sizeof(imgpel));
        }

        if (img->AdaptiveRounding)
        {
          for (j=block_y; j<block_y + 8; j++)
            memcpy(&fadjust8x8[1][j][block_x],&img->fadjust8x8[1][j][block_x], 8 * sizeof(int));

          if (img->P444_joined)
          {
            for (j=block_y; j<block_y + 8; j++)
            {
              memcpy(&fadjust8x8Cr[0][1][j][block_x],&img->fadjust8x8Cr[0][1][j][block_x], 8 * sizeof(int));
              memcpy(&fadjust8x8Cr[1][1][j][block_x],&img->fadjust8x8Cr[1][1][j][block_x], 8 * sizeof(int));
            }
          }            
        }

        if (img->P444_joined) 
        { 
          //--- set coefficients ---
          for (uv=0; uv < 2; uv++)
          {
            memcpy(cofAC8x8CbCr[uv][b8][0][0],img->cofAC[4+b8+4*uv][0][0], 2 * 4 * 65 * sizeof(int));

            cr_cbp[uv + 1] = c_nzCbCr[uv + 1];
            //--- set reconstruction ---
            for (y=0; y<8; y++)
            {
              memcpy(rec8x8[uv + 1][y],&enc_picture->imgUV[uv][pic_pix_y+y][pic_pix_x], 8 * sizeof(imgpel));
            }
          }
        }

        //--- flag if dct-coefficients must be coded ---
        nonzero = c_nz;

        //--- set best mode update minimum cost ---
        *min_cost   = rdcost;
        min_rdcost  = rdcost;
        best_ipmode = ipmode;
      }
      reset_coding_state_cs_cm(currMB);
    }
  }

  //===== set intra mode prediction =====
  img->ipredmode8x8[pic_block_y][pic_block_x] = (char) best_ipmode;
  ipmode_DPCM = best_ipmode; //For residual DPCM

  if(img->P444_joined)
  {
    ColorPlane k;
    CbCr_predmode_8x8[b8] = best_ipmode; 
    for (k = PLANE_U; k <= PLANE_V; k++)
    {
      cr_cbp[k] = 0; 
      select_plane(k);
      for (j=0; j<8; j++)
      {
        for (i=0; i<8; i++)
        {
          img->mb_pred[k][block_y+j][block_x+i] = img->mpr_8x8[k][best_ipmode][j][i]; 
          img->mb_ores[k][block_y+j][block_x+i] = pImgOrg[k][img->pix_y+block_y+j][img->pix_x+block_x+i] - img->mpr_8x8[k][best_ipmode][j][i];
        }
      }
      ipmode_DPCM = best_ipmode; 

      if (pDCT_8x8(currMB, k, b8, &dummy, 1))
        cr_cbp[k] = 1;
    }
    select_plane(PLANE_Y);
  }

  currMB->intra_pred_modes8x8[4*b8] = (mostProbableMode == best_ipmode)
    ? -1
    : (best_ipmode < mostProbableMode ? best_ipmode : best_ipmode-1);

  for(j = img->mb_y*4+(b8 >> 1)*2; j < img->mb_y*4+(b8 >> 1)*2 + 2; j++)   //loop 4x4s in the subblock for 8x8 prediction setting
    memset(&img->ipredmode8x8[j][img->mb_x*4+(b8 & 0x01)*2], best_ipmode, 2 * sizeof(char));

  //===== restore coefficients =====
  memcpy(img->cofAC[b8][0][0],cofAC8x8[b8][0][0], 4 * 2 * 65 * sizeof(int));


  if (img->AdaptiveRounding)
  {
    for (j=block_y; j< block_y + 8; j++)
      memcpy(&img->fadjust8x8[1][j][block_x], &fadjust8x8[1][j][block_x], 8 * sizeof(int));
    if (img->P444_joined)
    {
      for (j=0; j<8; j++)
      {
        memcpy(&img->fadjust8x8Cr[0][1][block_y+j][block_x], &fadjust8x8Cr[0][1][block_y+j][block_x], 8 * sizeof(int));
        memcpy(&img->fadjust8x8Cr[1][1][block_y+j][block_x], &fadjust8x8Cr[1][1][block_y+j][block_x], 8 * sizeof(int));
      }
    }
  }

  //===== restore reconstruction and prediction (needed if single coeffs are removed) =====
  for (y=0; y<8; y++)
  {
    memcpy(&enc_picture->imgY[pic_pix_y + y][pic_pix_x], rec8x8[0][y], 8 * sizeof(imgpel));
    memcpy(&mb_pred[block_y + y][block_x], img->mpr_8x8[0][best_ipmode][y], 8 * sizeof(imgpel));
  }
  if (img->P444_joined)
  {
    //===== restore coefficients =====
    memcpy(img->cofAC[4+b8+4*0][0][0], cofAC8x8CbCr[0][b8][0][0], 4 * 2 * 65 * sizeof(int));
    memcpy(img->cofAC[4+b8+4*1][0][0], cofAC8x8CbCr[1][b8][0][0], 4 * 2 * 65 * sizeof(int));

        //===== restore reconstruction and prediction (needed if single coeffs are removed) =====
    for (y=0; y<8; y++) 
    {
      memcpy(&enc_picture->imgUV[0][pic_pix_y+y][pic_pix_x], rec8x8[1][y], 8 * sizeof(imgpel));
      memcpy(&enc_picture->imgUV[1][pic_pix_y+y][pic_pix_x], rec8x8[2][y], 8 * sizeof(imgpel));
      memcpy(&img->mb_pred[1][block_y+y][block_x], img->mpr_8x8[1][best_ipmode][y], 8 * sizeof(imgpel));
      memcpy(&img->mb_pred[2][block_y+y][block_x], img->mpr_8x8[2][best_ipmode][y], 8 * sizeof(imgpel));
    }
  }

  return nonzero;
}

// Notation for comments regarding prediction and predictors.
// The pels of the 4x4 block are labelled a..p. The predictor pels above
// are labelled A..H, from the left I..P, and from above left X, as follows:
//
//  Z  A  B  C  D  E  F  G  H  I  J  K  L  M   N  O  P
//  Q  a1 b1 c1 d1 e1 f1 g1 h1
//  R  a2 b2 c2 d2 e2 f2 g2 h2
//  S  a3 b3 c3 d3 e3 f3 g3 h3
//  T  a4 b4 c4 d4 e4 f4 g4 h4
//  U  a5 b5 c5 d5 e5 f5 g5 h5
//  V  a6 b6 c6 d6 e6 f6 g6 h6
//  W  a7 b7 c7 d7 e7 f7 g7 h7
//  X  a8 b8 c8 d8 e8 f8 g8 h8


// Predictor array index definitions
#define P_Z (PredPel[0])
#define P_A (PredPel[1])
#define P_B (PredPel[2])
#define P_C (PredPel[3])
#define P_D (PredPel[4])
#define P_E (PredPel[5])
#define P_F (PredPel[6])
#define P_G (PredPel[7])
#define P_H (PredPel[8])
#define P_I (PredPel[9])
#define P_J (PredPel[10])
#define P_K (PredPel[11])
#define P_L (PredPel[12])
#define P_M (PredPel[13])
#define P_N (PredPel[14])
#define P_O (PredPel[15])
#define P_P (PredPel[16])
#define P_Q (PredPel[17])
#define P_R (PredPel[18])
#define P_S (PredPel[19])
#define P_T (PredPel[20])
#define P_U (PredPel[21])
#define P_V (PredPel[22])
#define P_W (PredPel[23])
#define P_X (PredPel[24])

/*!
 ************************************************************************
 * \brief
 *    Make intra 8x8 prediction according to all 9 prediction modes.
 *    The routine uses left and upper neighbouring points from
 *    previous coded blocks to do this (if available). Notice that
 *    inaccessible neighbouring points are signalled with a negative
 *    value in the predmode array .
 *
 *  \par Input:
 *     Starting point of current 8x8 block image posision
 *
 *  \par Output:
 *      none
 ************************************************************************
 */
void intrapred_8x8(Macroblock *currMB, ColorPlane pl, int img_x,int img_y, int *left_available, int *up_available, int *all_available)
{
  int i,j;
  int s0;
  static imgpel PredPel[25];  // array of predictor pels
  imgpel **img_enc = enc_picture->p_curr_img;
  imgpel *img_pel;
  static imgpel (*cur_pred)[16];
  imgpel (*curr_mpr_8x8)[16][16]  = img->mpr_8x8[pl];
  unsigned int dc_pred_value = img->dc_pred_value;
  int *mb_size = img->mb_size[IS_LUMA];

  int ioff = (img_x & 15);
  int joff = (img_y & 15);

  PixelPos pix_a[8];
  PixelPos pix_b, pix_c, pix_d;

  int block_available_up;
  int block_available_left;
  int block_available_up_left;
  int block_available_up_right;

  for (i=0;i<8;i++)
  {
    getNeighbour(currMB, ioff - 1, joff + i , mb_size, &pix_a[i]);
  }

  getNeighbour(currMB, ioff    , joff - 1, mb_size, &pix_b);
  getNeighbour(currMB, ioff + 8, joff - 1, mb_size, &pix_c);
  getNeighbour(currMB, ioff - 1, joff - 1, mb_size, &pix_d);

  pix_c.available = pix_c.available &&!(ioff == 8 && joff == 8);

  if (params->UseConstrainedIntraPred)
  {
    for (i=0, block_available_left=1; i<8;i++)
      block_available_left  &= pix_a[i].available ? img->intra_block[pix_a[i].mb_addr]: 0;

    block_available_up       = pix_b.available ? img->intra_block [pix_b.mb_addr] : 0;
    block_available_up_right = pix_c.available ? img->intra_block [pix_c.mb_addr] : 0;
    block_available_up_left  = pix_d.available ? img->intra_block [pix_d.mb_addr] : 0;
  }
  else
  {
    block_available_left     = pix_a[0].available;
    block_available_up       = pix_b.available;
    block_available_up_right = pix_c.available;
    block_available_up_left  = pix_d.available;
  }

  *left_available = block_available_left;
  *up_available   = block_available_up;
  *all_available  = block_available_up && block_available_left && block_available_up_left;

  i = (img_x & 15);
  j = (img_y & 15);

  // form predictor pels
  // form predictor pels
  if (block_available_up)
  {
    img_pel = &img_enc[pix_b.pos_y][pix_b.pos_x];
    P_A = *(img_pel++);
    P_B = *(img_pel++);
    P_C = *(img_pel++);
    P_D = *(img_pel++);
    P_E = *(img_pel++);
    P_F = *(img_pel++);
    P_G = *(img_pel++);
    P_H = *(img_pel);
  }
  else
  {
    P_A = P_B = P_C = P_D = P_E = P_F = P_G = P_H = dc_pred_value;
  }

  if (block_available_up_right)
  {
    img_pel = &img_enc[pix_c.pos_y][pix_c.pos_x];
    P_I = *(img_pel++);
    P_J = *(img_pel++);
    P_K = *(img_pel++);
    P_L = *(img_pel++);
    P_M = *(img_pel++);
    P_N = *(img_pel++);
    P_O = *(img_pel++);
    P_P = *(img_pel);

  }
  else
  {
    P_I = P_J = P_K = P_L = P_M = P_N = P_O = P_P = P_H;
  }

  if (block_available_left)
  {
    P_Q = img_enc[pix_a[0].pos_y][pix_a[0].pos_x];
    P_R = img_enc[pix_a[1].pos_y][pix_a[1].pos_x];
    P_S = img_enc[pix_a[2].pos_y][pix_a[2].pos_x];
    P_T = img_enc[pix_a[3].pos_y][pix_a[3].pos_x];
    P_U = img_enc[pix_a[4].pos_y][pix_a[4].pos_x];
    P_V = img_enc[pix_a[5].pos_y][pix_a[5].pos_x];
    P_W = img_enc[pix_a[6].pos_y][pix_a[6].pos_x];
    P_X = img_enc[pix_a[7].pos_y][pix_a[7].pos_x];
  }
  else
  {
    P_Q = P_R = P_S = P_T = P_U = P_V = P_W = P_X = dc_pred_value;
  }

  if (block_available_up_left)
  {
    P_Z = img_enc[pix_d.pos_y][pix_d.pos_x];
  }
  else
  {
    P_Z = dc_pred_value;
  }

  for(i=0;i<9;i++)
    curr_mpr_8x8[i][0][0]=-1;

  LowPassForIntra8x8Pred(&(P_Z), block_available_up_left, block_available_up, block_available_left);

  ///////////////////////////////
  // make DC prediction
  ///////////////////////////////
  s0 = 0;
  if (block_available_up && block_available_left)
  {
    // no edge
    s0 = rshift_rnd_sf((P_A + P_B + P_C + P_D + P_E + P_F + P_G + P_H + P_Q + P_R + P_S + P_T + P_U + P_V + P_W + P_X), 4);
  }
  else if (!block_available_up && block_available_left)
  {
    // upper edge
    s0 = rshift_rnd_sf((P_Q + P_R + P_S + P_T + P_U + P_V + P_W + P_X), 3);
  }
  else if (block_available_up && !block_available_left)
  {
    // left edge
    s0 = rshift_rnd_sf((P_A + P_B + P_C + P_D + P_E + P_F + P_G + P_H), 3);
  }
  else //if (!block_available_up && !block_available_left)
  {
    // top left corner, nothing to predict from
    s0 = dc_pred_value;
  }

  // store DC prediction
  cur_pred = curr_mpr_8x8[DC_PRED];
  for (j=0; j < BLOCK_SIZE_8x8; j++)
  {
    for (i=0; i < BLOCK_SIZE_8x8; i++)
    {
      cur_pred[j][i] = (imgpel) s0;
    }
  }


  ///////////////////////////////
  // make horiz and vert prediction
  ///////////////////////////////
  cur_pred = curr_mpr_8x8[VERT_PRED];
  for (i=0; i < BLOCK_SIZE_8x8; i++)
  {
    cur_pred[0][i] =
    cur_pred[1][i] =
    cur_pred[2][i] =
    cur_pred[3][i] =
    cur_pred[4][i] =
    cur_pred[5][i] =
    cur_pred[6][i] =
    cur_pred[7][i] = (imgpel)(&P_A)[i];
  }
  if(!block_available_up)
    cur_pred[0][0]=-1;

  cur_pred = curr_mpr_8x8[HOR_PRED];
  for (i=0; i < BLOCK_SIZE_8x8; i++)
  {
    cur_pred[i][0]  =
    cur_pred[i][1]  =
    cur_pred[i][2]  =
    cur_pred[i][3]  =
    cur_pred[i][4]  =
    cur_pred[i][5]  =
    cur_pred[i][6]  =
    cur_pred[i][7]  = (imgpel) (&P_Q)[i];
  }
  if(!block_available_left)
    cur_pred[0][0]=-1;

  ///////////////////////////////////
  // make diagonal down left prediction
  ///////////////////////////////////
  if (block_available_up)
  {
    // Mode DIAG_DOWN_LEFT_PRED
    cur_pred = curr_mpr_8x8[DIAG_DOWN_LEFT_PRED];
    cur_pred[0][0] = (imgpel) ((P_A + P_C + 2*(P_B) + 2) >> 2);
    cur_pred[0][1] =
    cur_pred[1][0] = (imgpel) ((P_B + P_D + 2*(P_C) + 2) >> 2);
    cur_pred[0][2] =
    cur_pred[1][1] =
    cur_pred[2][0] = (imgpel) ((P_C + P_E + 2*(P_D) + 2) >> 2);
    cur_pred[0][3] =
    cur_pred[1][2] =
    cur_pred[2][1] =
    cur_pred[3][0] = (imgpel) ((P_D + P_F + 2*(P_E) + 2) >> 2);
    cur_pred[0][4] =
    cur_pred[1][3] =
    cur_pred[2][2] =
    cur_pred[3][1] =
    cur_pred[4][0] = (imgpel) ((P_E + P_G + 2*(P_F) + 2) >> 2);
    cur_pred[0][5] =
    cur_pred[1][4] =
    cur_pred[2][3] =
    cur_pred[3][2] =
    cur_pred[4][1] =
    cur_pred[5][0] = (imgpel) ((P_F + P_H + 2*(P_G) + 2) >> 2);
    cur_pred[0][6] =
    cur_pred[1][5] =
    cur_pred[2][4] =
    cur_pred[3][3] =
    cur_pred[4][2] =
    cur_pred[5][1] =
    cur_pred[6][0] = (imgpel) ((P_G + P_I + 2*(P_H) + 2) >> 2);
    cur_pred[0][7] =
    cur_pred[1][6] =
    cur_pred[2][5] =
    cur_pred[3][4] =
    cur_pred[4][3] =
    cur_pred[5][2] =
    cur_pred[6][1] =
    cur_pred[7][0] = (imgpel) ((P_H + P_J + 2*(P_I) + 2) >> 2);
    cur_pred[1][7] =
    cur_pred[2][6] =
    cur_pred[3][5] =
    cur_pred[4][4] =
    cur_pred[5][3] =
    cur_pred[6][2] =
    cur_pred[7][1] = (imgpel) ((P_I + P_K + 2*(P_J) + 2) >> 2);
    cur_pred[2][7] =
    cur_pred[3][6] =
    cur_pred[4][5] =
    cur_pred[5][4] =
    cur_pred[6][3] =
    cur_pred[7][2] = (imgpel) ((P_J + P_L + 2*(P_K) + 2) >> 2);
    cur_pred[3][7] =
    cur_pred[4][6] =
    cur_pred[5][5] =
    cur_pred[6][4] =
    cur_pred[7][3] = (imgpel) ((P_K + P_M + 2*(P_L) + 2) >> 2);
    cur_pred[4][7] =
    cur_pred[5][6] =
    cur_pred[6][5] =
    cur_pred[7][4] = (imgpel) ((P_L + P_N + 2*(P_M) + 2) >> 2);
    cur_pred[5][7] =
    cur_pred[6][6] =
    cur_pred[7][5] = (imgpel) ((P_M + P_O + 2*(P_N) + 2) >> 2);
    cur_pred[6][7] =
    cur_pred[7][6] = (imgpel) ((P_N + P_P + 2*(P_O) + 2) >> 2);
    cur_pred[7][7] = (imgpel) ((P_O + 3*(P_P) + 2) >> 2);

    ///////////////////////////////////
    // make vertical left prediction
    ///////////////////////////////////
    cur_pred = curr_mpr_8x8[VERT_LEFT_PRED];
    cur_pred[0][0] = (imgpel) ((P_A + P_B + 1) >> 1);
    cur_pred[0][1] =
    cur_pred[2][0] = (imgpel) ((P_B + P_C + 1) >> 1);
    cur_pred[0][2] =
    cur_pred[2][1] =
    cur_pred[4][0] = (imgpel) ((P_C + P_D + 1) >> 1);
    cur_pred[0][3] =
    cur_pred[2][2] =
    cur_pred[4][1] =
    cur_pred[6][0] = (imgpel) ((P_D + P_E + 1) >> 1);
    cur_pred[0][4] =
    cur_pred[2][3] =
    cur_pred[4][2] =
    cur_pred[6][1] = (imgpel) ((P_E + P_F + 1) >> 1);
    cur_pred[0][5] =
    cur_pred[2][4] =
    cur_pred[4][3] =
    cur_pred[6][2] = (imgpel) ((P_F + P_G + 1) >> 1);
    cur_pred[0][6] =
    cur_pred[2][5] =
    cur_pred[4][4] =
    cur_pred[6][3] = (imgpel) ((P_G + P_H + 1) >> 1);
    cur_pred[0][7] =
    cur_pred[2][6] =
    cur_pred[4][5] =
    cur_pred[6][4] = (imgpel) ((P_H + P_I + 1) >> 1);
    cur_pred[2][7] =
    cur_pred[4][6] =
    cur_pred[6][5] = (imgpel) ((P_I + P_J + 1) >> 1);
    cur_pred[4][7] =
    cur_pred[6][6] = (imgpel) ((P_J + P_K + 1) >> 1);
    cur_pred[6][7] = (imgpel) ((P_K + P_L + 1) >> 1);
    cur_pred[1][0] = (imgpel) ((P_A + P_C + 2*P_B + 2) >> 2);
    cur_pred[1][1] =
    cur_pred[3][0] = (imgpel) ((P_B + P_D + 2*P_C + 2) >> 2);
    cur_pred[1][2] =
    cur_pred[3][1] =
    cur_pred[5][0] = (imgpel) ((P_C + P_E + 2*P_D + 2) >> 2);
    cur_pred[1][3] =
    cur_pred[3][2] =
    cur_pred[5][1] =
    cur_pred[7][0] = (imgpel) ((P_D + P_F + 2*P_E + 2) >> 2);
    cur_pred[1][4] =
    cur_pred[3][3] =
    cur_pred[5][2] =
    cur_pred[7][1] = (imgpel) ((P_E + P_G + 2*P_F + 2) >> 2);
    cur_pred[1][5] =
    cur_pred[3][4] =
    cur_pred[5][3] =
    cur_pred[7][2] = (imgpel) ((P_F + P_H + 2*P_G + 2) >> 2);
    cur_pred[1][6] =
    cur_pred[3][5] =
    cur_pred[5][4] =
    cur_pred[7][3] = (imgpel) ((P_G + P_I + 2*P_H + 2) >> 2);
    cur_pred[1][7] =
    cur_pred[3][6] =
    cur_pred[5][5] =
    cur_pred[7][4] = (imgpel) ((P_H + P_J + 2*P_I + 2) >> 2);
    cur_pred[3][7] =
    cur_pred[5][6] =
    cur_pred[7][5] = (imgpel) ((P_I + P_K + 2*P_J + 2) >> 2);
    cur_pred[5][7] =
    cur_pred[7][6] = (imgpel) ((P_J + P_L + 2*P_K + 2) >> 2);
    cur_pred[7][7] = (imgpel) ((P_K + P_M + 2*P_L + 2) >> 2);
  }

  ///////////////////////////////////
  // make diagonal down right prediction
  ///////////////////////////////////
  if (block_available_up && block_available_left && block_available_up_left)
  {
    // Mode DIAG_DOWN_RIGHT_PRED
    cur_pred = curr_mpr_8x8[DIAG_DOWN_RIGHT_PRED];
    cur_pred[7][0] = (imgpel) ((P_X + P_V + 2*(P_W) + 2) >> 2);
    cur_pred[6][0] =
    cur_pred[7][1] = (imgpel) ((P_W + P_U + 2*(P_V) + 2) >> 2);
    cur_pred[5][0] =
    cur_pred[6][1] =
    cur_pred[7][2] = (imgpel) ((P_V + P_T + 2*(P_U) + 2) >> 2);
    cur_pred[4][0] =
    cur_pred[5][1] =
    cur_pred[6][2] =
    cur_pred[7][3] = (imgpel) ((P_U + P_S + 2*(P_T) + 2) >> 2);
    cur_pred[3][0] =
    cur_pred[4][1] =
    cur_pred[5][2] =
    cur_pred[6][3] =
    cur_pred[7][4] = (imgpel) ((P_T + P_R + 2*(P_S) + 2) >> 2);
    cur_pred[2][0] =
    cur_pred[3][1] =
    cur_pred[4][2] =
    cur_pred[5][3] =
    cur_pred[6][4] =
    cur_pred[7][5] = (imgpel) ((P_S + P_Q + 2*(P_R) + 2) >> 2);
    cur_pred[1][0] =
    cur_pred[2][1] =
    cur_pred[3][2] =
    cur_pred[4][3] =
    cur_pred[5][4] =
    cur_pred[6][5] =
    cur_pred[7][6] = (imgpel) ((P_R + P_Z + 2*(P_Q) + 2) >> 2);
    cur_pred[0][0] =
    cur_pred[1][1] =
    cur_pred[2][2] =
    cur_pred[3][3] =
    cur_pred[4][4] =
    cur_pred[5][5] =
    cur_pred[6][6] =
    cur_pred[7][7] = (imgpel) ((P_Q + P_A + 2*(P_Z) + 2) >> 2);
    cur_pred[0][1] =
    cur_pred[1][2] =
    cur_pred[2][3] =
    cur_pred[3][4] =
    cur_pred[4][5] =
    cur_pred[5][6] =
    cur_pred[6][7] = (imgpel) ((P_Z + P_B + 2*(P_A) + 2) >> 2);
    cur_pred[0][2] =
    cur_pred[1][3] =
    cur_pred[2][4] =
    cur_pred[3][5] =
    cur_pred[4][6] =
    cur_pred[5][7] = (imgpel) ((P_A + P_C + 2*(P_B) + 2) >> 2);
    cur_pred[0][3] =
    cur_pred[1][4] =
    cur_pred[2][5] =
    cur_pred[3][6] =
    cur_pred[4][7] = (imgpel) ((P_B + P_D + 2*(P_C) + 2) >> 2);
    cur_pred[0][4] =
    cur_pred[1][5] =
    cur_pred[2][6] =
    cur_pred[3][7] = (imgpel) ((P_C + P_E + 2*(P_D) + 2) >> 2);
    cur_pred[0][5] =
    cur_pred[1][6] =
    cur_pred[2][7] = (imgpel) ((P_D + P_F + 2*(P_E) + 2) >> 2);
    cur_pred[0][6] =
    cur_pred[1][7] = (imgpel) ((P_E + P_G + 2*(P_F) + 2) >> 2);
    cur_pred[0][7] = (imgpel) ((P_F + P_H + 2*(P_G) + 2) >> 2);

    ///////////////////////////////////
    // make vertical right prediction
    ///////////////////////////////////
    cur_pred = curr_mpr_8x8[VERT_RIGHT_PRED];
    cur_pred[0][0] =
    cur_pred[2][1] =
    cur_pred[4][2] =
    cur_pred[6][3] = (imgpel) ((P_Z + P_A + 1) >> 1);
    cur_pred[0][1] =
    cur_pred[2][2] =
    cur_pred[4][3] =
    cur_pred[6][4] = (imgpel) ((P_A + P_B + 1) >> 1);
    cur_pred[0][2] =
    cur_pred[2][3] =
    cur_pred[4][4] =
    cur_pred[6][5] = (imgpel) ((P_B + P_C + 1) >> 1);
    cur_pred[0][3] =
    cur_pred[2][4] =
    cur_pred[4][5] =
    cur_pred[6][6] = (imgpel) ((P_C + P_D + 1) >> 1);
    cur_pred[0][4] =
    cur_pred[2][5] =
    cur_pred[4][6] =
    cur_pred[6][7] = (imgpel) ((P_D + P_E + 1) >> 1);
    cur_pred[0][5] =
    cur_pred[2][6] =
    cur_pred[4][7] = (imgpel) ((P_E + P_F + 1) >> 1);
    cur_pred[0][6] =
    cur_pred[2][7] = (imgpel) ((P_F + P_G + 1) >> 1);
    cur_pred[0][7] = (imgpel) ((P_G + P_H + 1) >> 1);
    cur_pred[1][0] =
    cur_pred[3][1] =
    cur_pred[5][2] =
    cur_pred[7][3] = (imgpel) ((P_Q + P_A + 2*P_Z + 2) >> 2);
    cur_pred[1][1] =
    cur_pred[3][2] =
    cur_pred[5][3] =
    cur_pred[7][4] = (imgpel) ((P_Z + P_B + 2*P_A + 2) >> 2);
    cur_pred[1][2] =
    cur_pred[3][3] =
    cur_pred[5][4] =
    cur_pred[7][5] = (imgpel) ((P_A + P_C + 2*P_B + 2) >> 2);
    cur_pred[1][3] =
    cur_pred[3][4] =
    cur_pred[5][5] =
    cur_pred[7][6] = (imgpel) ((P_B + P_D + 2*P_C + 2) >> 2);
    cur_pred[1][4] =
    cur_pred[3][5] =
    cur_pred[5][6] =
    cur_pred[7][7] = (imgpel) ((P_C + P_E + 2*P_D + 2) >> 2);
    cur_pred[1][5] =
    cur_pred[3][6] =
    cur_pred[5][7] = (imgpel) ((P_D + P_F + 2*P_E + 2) >> 2);
    cur_pred[1][6] =
    cur_pred[3][7] = (imgpel) ((P_E + P_G + 2*P_F + 2) >> 2);
    cur_pred[1][7] = (imgpel) ((P_F + P_H + 2*P_G + 2) >> 2);
    cur_pred[2][0] =
    cur_pred[4][1] =
    cur_pred[6][2] = (imgpel) ((P_R + P_Z + 2*P_Q + 2) >> 2);
    cur_pred[3][0] =
    cur_pred[5][1] =
    cur_pred[7][2] = (imgpel) ((P_S + P_Q + 2*P_R + 2) >> 2);
    cur_pred[4][0] =
    cur_pred[6][1] = (imgpel) ((P_T + P_R + 2*P_S + 2) >> 2);
    cur_pred[5][0] =
    cur_pred[7][1] = (imgpel) ((P_U + P_S + 2*P_T + 2) >> 2);
    cur_pred[6][0] = (imgpel) ((P_V + P_T + 2*P_U + 2) >> 2);
    cur_pred[7][0] = (imgpel) ((P_W + P_U + 2*P_V + 2) >> 2);

    ///////////////////////////////////
    // make horizontal down prediction
    ///////////////////////////////////
    cur_pred = img->mpr_8x8[0][HOR_DOWN_PRED];
    cur_pred[0][0] =
    cur_pred[1][2] =
    cur_pred[2][4] =
    cur_pred[3][6] = (imgpel) ((P_Q + P_Z + 1) >> 1);
    cur_pred[1][0] =
    cur_pred[2][2] =
    cur_pred[3][4] =
    cur_pred[4][6] = (imgpel) ((P_R + P_Q + 1) >> 1);
    cur_pred[2][0] =
    cur_pred[3][2] =
    cur_pred[4][4] =
    cur_pred[5][6] = (imgpel) ((P_S + P_R + 1) >> 1);
    cur_pred[3][0] =
    cur_pred[4][2] =
    cur_pred[5][4] =
    cur_pred[6][6] = (imgpel) ((P_T + P_S + 1) >> 1);
    cur_pred[4][0] =
    cur_pred[5][2] =
    cur_pred[6][4] =
    cur_pred[7][6] = (imgpel) ((P_U + P_T + 1) >> 1);
    cur_pred[5][0] =
    cur_pred[6][2] =
    cur_pred[7][4] = (imgpel) ((P_V + P_U + 1) >> 1);
    cur_pred[6][0] =
    cur_pred[7][2] = (imgpel) ((P_W + P_V + 1) >> 1);
    cur_pred[7][0] = (imgpel) ((P_X + P_W + 1) >> 1);
    cur_pred[0][1] =
    cur_pred[1][3] =
    cur_pred[2][5] =
    cur_pred[3][7] = (imgpel) ((P_Q + P_A + 2*P_Z + 2) >> 2);
    cur_pred[1][1] =
    cur_pred[2][3] =
    cur_pred[3][5] =
    cur_pred[4][7] = (imgpel) ((P_Z + P_R + 2*P_Q + 2) >> 2);
    cur_pred[2][1] =
    cur_pred[3][3] =
    cur_pred[4][5] =
    cur_pred[5][7] = (imgpel) ((P_Q + P_S + 2*P_R + 2) >> 2);
    cur_pred[3][1] =
    cur_pred[4][3] =
    cur_pred[5][5] =
    cur_pred[6][7] = (imgpel) ((P_R + P_T + 2*P_S + 2) >> 2);
    cur_pred[4][1] =
    cur_pred[5][3] =
    cur_pred[6][5] =
    cur_pred[7][7] = (imgpel) ((P_S + P_U + 2*P_T + 2) >> 2);
    cur_pred[5][1] =
    cur_pred[6][3] =
    cur_pred[7][5] = (imgpel) ((P_T + P_V + 2*P_U + 2) >> 2);
    cur_pred[6][1] =
    cur_pred[7][3] = (imgpel) ((P_U + P_W + 2*P_V + 2) >> 2);
    cur_pred[7][1] = (imgpel) ((P_V + P_X + 2*P_W + 2) >> 2);
    cur_pred[0][2] =
    cur_pred[1][4] =
    cur_pred[2][6] = (imgpel) ((P_Z + P_B + 2*P_A + 2) >> 2);
    cur_pred[0][3] =
    cur_pred[1][5] =
    cur_pred[2][7] = (imgpel) ((P_A + P_C + 2*P_B + 2) >> 2);
    cur_pred[0][4] =
    cur_pred[1][6] = (imgpel) ((P_B + P_D + 2*P_C + 2) >> 2);
    cur_pred[0][5] =
    cur_pred[1][7] = (imgpel) ((P_C + P_E + 2*P_D + 2) >> 2);
    cur_pred[0][6] = (imgpel) ((P_D + P_F + 2*P_E + 2) >> 2);
    cur_pred[0][7] = (imgpel) ((P_E + P_G + 2*P_F + 2) >> 2);
  }

  ///////////////////////////////////
  // make horizontal up prediction
  ///////////////////////////////////
  if (block_available_left)
  {
    cur_pred = curr_mpr_8x8[HOR_UP_PRED];
    cur_pred[0][0] = (imgpel) ((P_Q + P_R + 1) >> 1);
    cur_pred[1][0] =
    cur_pred[0][2] = (imgpel) ((P_R + P_S + 1) >> 1);
    cur_pred[2][0] =
    cur_pred[1][2] =
    cur_pred[0][4] = (imgpel) ((P_S + P_T + 1) >> 1);
    cur_pred[3][0] =
    cur_pred[2][2] =
    cur_pred[1][4] =
    cur_pred[0][6] = (imgpel) ((P_T + P_U + 1) >> 1);
    cur_pred[4][0] =
    cur_pred[3][2] =
    cur_pred[2][4] =
    cur_pred[1][6] = (imgpel) ((P_U + P_V + 1) >> 1);
    cur_pred[5][0] =
    cur_pred[4][2] =
    cur_pred[3][4] =
    cur_pred[2][6] = (imgpel) ((P_V + P_W + 1) >> 1);
    cur_pred[6][0] =
    cur_pred[5][2] =
    cur_pred[4][4] =
    cur_pred[3][6] = (imgpel) ((P_W + P_X + 1) >> 1);
    cur_pred[4][6] =
    cur_pred[4][7] =
    cur_pred[5][4] =
    cur_pred[5][5] =
    cur_pred[5][6] =
    cur_pred[5][7] =
    cur_pred[6][2] =
    cur_pred[6][3] =
    cur_pred[6][4] =
    cur_pred[6][5] =
    cur_pred[6][6] =
    cur_pred[6][7] =
    cur_pred[7][0] =
    cur_pred[7][1] =
    cur_pred[7][2] =
    cur_pred[7][3] =
    cur_pred[7][4] =
    cur_pred[7][5] =
    cur_pred[7][6] =
    cur_pred[7][7] = (imgpel) P_X;
    cur_pred[6][1] =
    cur_pred[5][3] =
    cur_pred[4][5] =
    cur_pred[3][7] = (imgpel) ((P_W + 3*P_X + 2) >> 2);
    cur_pred[5][1] =
    cur_pred[4][3] =
    cur_pred[3][5] =
    cur_pred[2][7] = (imgpel) ((P_X + P_V + 2*P_W + 2) >> 2);
    cur_pred[4][1] =
    cur_pred[3][3] =
    cur_pred[2][5] =
    cur_pred[1][7] = (imgpel) ((P_W + P_U + 2*P_V + 2) >> 2);
    cur_pred[3][1] =
    cur_pred[2][3] =
    cur_pred[1][5] =
    cur_pred[0][7] = (imgpel) ((P_V + P_T + 2*P_U + 2) >> 2);
    cur_pred[2][1] =
    cur_pred[1][3] =
    cur_pred[0][5] = (imgpel) ((P_U + P_S + 2*P_T + 2) >> 2);
    cur_pred[1][1] =
    cur_pred[0][3] = (imgpel) ((P_T + P_R + 2*P_S + 2) >> 2);
    cur_pred[0][1] = (imgpel) ((P_S + P_Q + 2*P_R + 2) >> 2);
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Prefiltering for Intra8x8 prediction
 *************************************************************************************
 */
void LowPassForIntra8x8Pred(imgpel *PredPel, int block_up_left, int block_up, int block_left)
{
  int i;
  static imgpel LoopArray[25];

  memcpy(LoopArray,PredPel, 25 * sizeof(imgpel));

  if(block_up)
  {
    if(block_up_left)
    {
      LoopArray[1] = ((PredPel[0] + (PredPel[1]<<1) + PredPel[2] + 2)>>2);
    }
    else
      LoopArray[1] = ((PredPel[1] + (PredPel[1]<<1) + PredPel[2] + 2)>>2);


    for(i = 2; i <16; i++)
    {
      LoopArray[i] = ((PredPel[i-1] + (PredPel[i]<<1) + PredPel[i+1] + 2)>>2);
    }
    LoopArray[16] = ((P_P + (P_P<<1) + P_O + 2)>>2);
  }

  if(block_up_left)
  {
    if(block_up && block_left)
    {
      LoopArray[0] = ((P_Q + (P_Z<<1) + P_A +2)>>2);
    }
    else
    {
      if(block_up)
        LoopArray[0] = ((P_Z + (P_Z<<1) + P_A +2)>>2);
      else
        if(block_left)
          LoopArray[0] = ((P_Z + (P_Z<<1) + P_Q +2)>>2);
    }
  }

  if(block_left)
  {
    if(block_up_left)
      LoopArray[17] = ((P_Z + (P_Q<<1) + P_R + 2)>>2);
    else
      LoopArray[17] = ((P_Q + (P_Q<<1) + P_R + 2)>>2);

    for(i = 18; i <24; i++)
    {
      LoopArray[i] = ((PredPel[i-1] + (PredPel[i]<<1) + PredPel[i+1] + 2)>>2);
    }
    LoopArray[24] = ((P_W + (P_X<<1) + P_X + 2) >> 2);
  }

  memcpy(PredPel, LoopArray, 25 * sizeof(imgpel));
}


/*!
 *************************************************************************************
 * \brief
 *    R-D Cost for an 8x8 Intra block
 *************************************************************************************
 */

double RDCost_for_8x8IntraBlocks(Macroblock *currMB, int *nonzero, int b8, int ipmode, double lambda, double min_rdcost, int mostProbableMode, int c_nzCbCr[3])
{
  double  rdcost = 0.0;
  int     dummy;
  int     rate;
  int64   distortion  = 0;
  int     block_x     = (b8 & 0x01) << 3;
  int     block_y     = (b8 >> 1) << 3;
  int     pic_pix_x   = img->pix_x + block_x;
  int     pic_pix_y   = img->pix_y + block_y;
  int     pic_opix_y  = img->opix_y + block_y;

  Slice          *currSlice =  img->currentSlice;
  SyntaxElement  se;
  const int      *partMap   = assignSE2partition[params->partition_mode];
  DataPartition  *dataPart;

  //===== perform DCT, Q, IQ, IDCT, Reconstruction =====
  dummy = 0;

  *nonzero = pDCT_8x8 (currMB, PLANE_Y, b8, &dummy, 1);

  //===== get distortion (SSD) of 8x8 block =====
  distortion += compute_SSE(&pCurImg[pic_opix_y], &enc_picture->imgY[pic_pix_y], pic_pix_x, pic_pix_x, 8, 8);

  if(img->P444_joined) 
  {
    ColorPlane k;

    for (k = PLANE_U; k <= PLANE_V; k++)
    {
      select_plane(k);
      /*    for (j=0; j<8; j++)   //KHHan, I think these line are not necessary
      {
      for (i=0; i<8; i++)
      {         
      img->mb_pred[k][block_y+j][block_x+i]  = img->mpr_8x8[k][ipmode][j][i];
      img->mb_ores[k][j][i] = pImgOrg[k][img->pix_y+block_y+j][img->pix_x+block_x+i] - img->mpr_8x8[k][ipmode][j][i];
      }
      }*/
      c_nzCbCr[k ]= pDCT_8x8(currMB, k, b8, &dummy,1);
      distortion += compute_SSE(&pImgOrg[k][pic_opix_y], &enc_picture->p_curr_img[pic_pix_y], pic_pix_x, pic_pix_x, 8, 8);
    }
	  ipmode_DPCM = NO_INTRA_PMODE;
    select_plane(PLANE_Y);
  }
  else if( img->yuv_format==YUV444 && IS_INDEPENDENT(params) )  //For residual DPCM
  {
    ipmode_DPCM = NO_INTRA_PMODE;  
  }

  //===== RATE for INTRA PREDICTION MODE  (SYMBOL MODE MUST BE SET TO CAVLC) =====
  se.value1 = (mostProbableMode == ipmode) ? -1 : ipmode < mostProbableMode ? ipmode : ipmode-1;

  //--- set position and type ---
  se.context = b8;
  se.type    = SE_INTRAPREDMODE;

  //--- choose data partition ---
  if (img->type!=B_SLICE)
    dataPart = &(currSlice->partArr[partMap[SE_INTRAPREDMODE]]);
  else
    dataPart = &(currSlice->partArr[partMap[SE_BFRAME]]);

  //--- encode and update rate ---
  writeIntraPredMode (&se, dataPart);

  rate = se.len;

  //===== RATE for LUMINANCE COEFFICIENTS =====

  if (currSlice->symbol_mode == CAVLC)
  {      
    if (img->P444_joined)
    {
      int b4;
      for(b4=0; b4<4; b4++)
      {
        rate  += writeCoeff4x4_CAVLC (currMB, LUMA, b8, b4, 0);
        rate  += writeCoeff4x4_CAVLC (currMB, CB, b8, b4, 0);
        rate  += writeCoeff4x4_CAVLC (currMB, CR, b8, b4, 0);
      }
    }
    else
    {
      rate  += writeCoeff4x4_CAVLC (currMB, LUMA, b8, 0, 0);
      rate  += writeCoeff4x4_CAVLC (currMB, LUMA, b8, 1, 0);
      rate  += writeCoeff4x4_CAVLC (currMB, LUMA, b8, 2, 0);
      rate  += writeCoeff4x4_CAVLC (currMB, LUMA, b8, 3, 0);
    }
  }
  else
  {
    rate  += writeCoeff8x8_CABAC (currMB, PLANE_Y, b8, 1);
    if(img->P444_joined)
    {
      rate  += writeCoeff8x8_CABAC (currMB, PLANE_U, b8, 1);
      rate  += writeCoeff8x8_CABAC (currMB, PLANE_V, b8, 1);
    }
  }

  rdcost = (double)distortion + lambda*(double)rate;

  return rdcost;
}

/*!
 ************************************************************************
 * \brief
 *    The routine performs transform,quantization,inverse transform, adds the diff.
 *    to the prediction and writes the result to the decoded luma frame. Includes the
 *    RD constrained quantization also.
 *
 * \par Input:
 *    b8: Block position inside a macro block (0,1,2,3).
 *
 * \par Output:
 *    nonzero: 0 if no levels are nonzero.  1 if there are nonzero levels.
 *    coeff_cost: Counter for nonzero coefficients, used to discard expensive levels.
 ************************************************************************
 */
int dct_8x8(Macroblock *currMB, ColorPlane pl, int b8, int *coeff_cost, int intra)
{
  int j;

  int nonzero = FALSE; 

  int block_x = 8*(b8 & 0x01);
  int block_y = 8*(b8 >> 1);
  int pl_off = b8+ (pl<<2);
  int*  ACLevel = img->cofAC[pl_off][0][0];
  int*  ACRun   = img->cofAC[pl_off][0][1];  
  imgpel **img_enc       = enc_picture->p_curr_img;
  imgpel (*mb_pred)[MB_BLOCK_SIZE] = img->mb_pred[pl];
  int    (*mb_ores)[MB_BLOCK_SIZE] = img->mb_ores[pl];
  int    (*mb_rres)[MB_BLOCK_SIZE] = img->mb_rres[pl];

  int max_imgpel_value   = img->max_imgpel_value;
  int qp = currMB->qp_scaled[pl];
  const byte (*pos_scan)[2] = currMB->is_field_mode ? FIELD_SCAN8x8 : SNGL_SCAN8x8;

  int qp_rem = qp_rem_matrix[qp];

  levelscale    = LevelScale8x8Comp   [pl][intra][qp_rem];
  invlevelscale = InvLevelScale8x8Comp[pl][intra][qp_rem];
  leveloffset   = LevelOffset8x8Comp  [pl][intra][qp];

  fadjust8x8 = img->AdaptiveRounding ? (pl ? &img->fadjust8x8Cr[pl-1][intra][block_y] : &img->fadjust8x8[intra][block_y]) :NULL;

  // Forward 8x8 transform
  forward8x8(mb_ores, mb_rres, block_y, block_x);

  // Quantization process
  nonzero = quant_8x8(&mb_rres[block_y], block_y, block_x, qp, ACLevel, ACRun, fadjust8x8, 
    levelscale, invlevelscale, leveloffset, coeff_cost, pos_scan, COEFF_COST8x8[params->disthres]);

  if (nonzero)
  {
    // Inverse 8x8 transform
    inverse8x8(mb_rres, mb_rres, block_y, block_x);

    // generate final block
    SampleReconstruct (img_enc, mb_pred, mb_rres, block_y, block_x, img->pix_y, img->pix_x + block_x, BLOCK_SIZE_8x8, BLOCK_SIZE_8x8, max_imgpel_value, DQ_BITS_8);
  }
  else // if (nonzero) => No transformed residual. Just use prediction.
  {      
    for( j=block_y; j< block_y + BLOCK_SIZE_8x8; j++)
    {
      memcpy(&(img_enc[img->pix_y + j][img->pix_x + block_x]),&(mb_pred[j][block_x]), BLOCK_SIZE_8x8 * sizeof(imgpel));
    }
  }  

  //  Decoded block moved to frame memory
  return nonzero;
}

/*!
 ************************************************************************
 * \brief
 *    The routine performs transform,quantization,inverse transform, adds the diff.
 *    to the prediction and writes the result to the decoded luma frame. Includes the
 *    RD constrained quantization also. Used for CAVLC.
 *
 * \par Input:
 *    b8: Block position inside a macro block (0,1,2,3).
 *
 * \par Output:
 *    nonzero: 0 if no levels are nonzero.  1 if there are nonzero levels.
 *    coeff_cost: Counter for nonzero coefficients, used to discard expensive levels.
 ************************************************************************
 */
int dct_8x8_cavlc(Macroblock *currMB, ColorPlane pl, int b8, int *coeff_cost, int intra)
{
  int j;
  int nonzero = FALSE; 

  int block_x = 8*(b8 & 0x01);
  int block_y = 8*(b8 >> 1);
  int pl_off = b8+ (pl<<2);
  imgpel **img_enc = enc_picture->p_curr_img;
  imgpel (*mb_pred)[MB_BLOCK_SIZE] = img->mb_pred[pl];  
  int    (*mb_ores)[MB_BLOCK_SIZE] = img->mb_ores[pl];   
  int    (*mb_rres)[MB_BLOCK_SIZE] = img->mb_rres[pl];   

  int max_imgpel_value   = img->max_imgpel_value;

  int qp = currMB->qp_scaled[pl];
  const byte (*pos_scan)[2] = currMB->is_field_mode ? FIELD_SCAN8x8 : SNGL_SCAN8x8;

  int qp_rem = qp_rem_matrix[qp];

  levelscale    = LevelScale8x8Comp   [pl][intra][qp_rem];
  invlevelscale = InvLevelScale8x8Comp[pl][intra][qp_rem];
  leveloffset   = LevelOffset8x8Comp  [pl][intra][qp];

  fadjust8x8 = img->AdaptiveRounding ? (pl ? &img->fadjust8x8Cr[pl-1][intra][block_y] : &img->fadjust8x8[intra][block_y]) :NULL;

  // Forward 8x8 transform
  forward8x8(mb_ores, mb_rres, block_y, block_x);

  // Quantization process
  nonzero = quant_8x8cavlc(&mb_rres[block_y], block_y, block_x, qp, img->cofAC[pl_off], fadjust8x8, 
    levelscale, invlevelscale, leveloffset, coeff_cost, pos_scan, COEFF_COST8x8[params->disthres]);

  if (nonzero)
  {
    // Inverse 8x8 transform
    inverse8x8(mb_rres, mb_rres, block_y, block_x);

    // generate final block
    SampleReconstruct (img_enc, mb_pred, mb_rres, block_y, block_x, img->pix_y, img->pix_x + block_x, BLOCK_SIZE_8x8, BLOCK_SIZE_8x8, max_imgpel_value, DQ_BITS_8);
  }
  else // if (nonzero) => No transformed residual. Just use prediction.
  {      
    for( j=block_y; j< block_y + BLOCK_SIZE_8x8; j++)
    {
      memcpy(&(img_enc[img->pix_y + j][img->pix_x + block_x]),&(mb_pred[j][block_x]), BLOCK_SIZE_8x8 * sizeof(imgpel));
    }
  }  

  //  Decoded block moved to frame memory
  return nonzero;
}

int dct_8x8_ls(Macroblock *currMB, ColorPlane pl, int b8, int *coeff_cost, int intra)
{
  int i,j,coeff_ctr;
  int scan_pos = 0,run = -1;
  int nonzero = FALSE;  

  int block_x = 8*(b8 & 0x01);
  int block_y = 8*(b8 >> 1);
  int pl_off = b8 + (pl<<2);
  int*  ACLevel = img->cofAC[pl_off][0][0];
  int*  ACRun   = img->cofAC[pl_off][0][1];  
  imgpel **img_enc       = enc_picture->p_curr_img;
  imgpel (*mb_pred)[MB_BLOCK_SIZE] = img->mb_pred[pl];
  int    (*mb_ores)[MB_BLOCK_SIZE] = img->mb_ores[pl];
  int    (*mb_rres)[MB_BLOCK_SIZE] = img->mb_rres[pl];

  int scan_poss[4] = { 0 }, runs[4] = { -1, -1, -1, -1 };
  int MCcoeff = 0;
  int *m7;
  int is_cavlc = (img->currentSlice->symbol_mode == CAVLC);

  const byte (*pos_scan)[2] = currMB->is_field_mode ? FIELD_SCAN8x8 : SNGL_SCAN8x8;

  int **fadjust8x8 = img->AdaptiveRounding ? (pl ? &img->fadjust8x8Cr[pl-1][intra][block_y] : &img->fadjust8x8[intra][block_y]) :NULL;

  runs[0]=runs[1]=runs[2]=runs[3]=-1;
  scan_poss[0] = scan_poss[1] = scan_poss[2] = scan_poss[3] = 0;

  if( (ipmode_DPCM < 2)&&(intra))
  {
    Residual_DPCM_8x8(ipmode_DPCM, mb_ores, mb_rres, block_y, block_x);
  }

  for (coeff_ctr=0; coeff_ctr < 64; coeff_ctr++)
  {
    i=pos_scan[coeff_ctr][0];
    j=pos_scan[coeff_ctr][1];

    run++;

    if (currMB->luma_transform_size_8x8_flag && is_cavlc)
    {
      MCcoeff = (coeff_ctr & 3);
      runs[MCcoeff]++;
    }

    m7 = &mb_rres[block_y + j][block_x + i];

    if (img->AdaptiveRounding)
    {
      fadjust8x8[j][block_x+i] = 0;
    }

    if (*m7 != 0)
    {
      nonzero = TRUE;

      if (currMB->luma_transform_size_8x8_flag && is_cavlc)
      {
        *m7 = iClip3(-CAVLC_LEVEL_LIMIT, CAVLC_LEVEL_LIMIT, *m7);
        *coeff_cost += MAX_VALUE;

        img->cofAC[pl_off][MCcoeff][0][scan_poss[MCcoeff]  ] = *m7;
        img->cofAC[pl_off][MCcoeff][1][scan_poss[MCcoeff]++] = runs[MCcoeff];
        ++scan_pos;
        runs[MCcoeff]=-1;
      }
      else
      {
        *coeff_cost += MAX_VALUE;
        ACLevel[scan_pos  ] = *m7;
        ACRun  [scan_pos++] = run;
        run=-1;                     // reset zero level counter
      }
    }
  }

  if (!currMB->luma_transform_size_8x8_flag || !is_cavlc)
    ACLevel[scan_pos] = 0;
  else
  {
    for(i=0; i<4; i++)
      img->cofAC[pl_off][i][0][scan_poss[i]] = 0;
  }

  if( (ipmode_DPCM < 2) && (intra))
  {
    Inv_Residual_DPCM_8x8(mb_rres, block_y, block_x);
  }

  for( j=block_y; j<block_y + BLOCK_SIZE_8x8; j++)
  {            
    for( i=block_x; i< block_x + BLOCK_SIZE_8x8; i++)
    {
      mb_rres[j][i] += mb_pred[j][i];
      img_enc[img->pix_y + j][img->pix_x + i]= (imgpel) mb_rres[j][i];
    }
  }  

  //  Decoded block moved to frame memory
  return nonzero;
}


/*static inline void compute_diff(int *diff, imgpel *cimg, imgpel *cmpr, int width)
{
  int i;
  for (i = 0; i < width; i++)
  {
    *(diff++) = *(cimg++) - *(cmpr++);
  }
}*/
/*!
*************************************************************************************
* \brief
*     distortion for an 8x8 Intra block 
*************************************************************************************
*/
void compute_comp_cost8x8(imgpel **cur_img, imgpel mpr8x8[16][16], int pic_opix_x, double *cost)
{
  int i, j;
  int *diff = &diff64[0];
  imgpel *cimg, *cmpr;

  for (j=0; j<8; j++)
  {
  //  compute_diff(diff, &cur_img[j][pic_opix_x], &mpr8x8[j][0], BLOCK_SIZE_8x8);

    cimg = &cur_img[j][pic_opix_x];
    cmpr = &mpr8x8[j][0];
    for (i=0; i<8; i++)
    {
      *diff++ = *cimg++ - *cmpr++;
    }

  }
  *cost += distortion8x8 (diff64);
}

