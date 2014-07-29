
/*!
 *************************************************************************************
 * \file quant8x8_trellis.c
 *
 * \brief
 *    Quantization process for a 4x4 block using trellis based quantization
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *
 *************************************************************************************
 */

#include "contributors.h"

#include <math.h>

#include "global.h"

#include "image.h"
#include "mb_access.h"
#include "vlc.h"
#include "transform.h"
#include "mc_prediction.h"
#include "q_offsets.h"
#include "q_matrix.h"
#include "quant8x8.h"
#include "rdoq.h"


extern const byte SNGL_SCAN[16][2]; // need to revisit
extern const byte FIELD_SCAN[16][2]; 
extern const byte FIELD_SCAN8x8[64][2];
extern const byte SNGL_SCAN8x8[64][2];

void rdoq_8x8_CABAC(int (*tblock)[16], int block_y, int block_x, int qp_per, int qp_rem, 
                    int **levelscale, int **leveloffset, const byte *p_scan, int levelTrellis[64]);

void rdoq_8x8_CAVLC(int (*tblock)[16], int block_y, int block_x, int qp_per, int qp_rem, 
                    int **levelscale, int **leveloffset, const byte *p_scan, int levelTrellis[4][16]);

/*!
 ************************************************************************
 * \brief
 *    Quantization process for All coefficients for a 8x8 block
 *
 * \par Input:
 *
 * \par Output:
 *
 ************************************************************************
 */
int quant_8x8_trellis(int (*tblock)[16], int block_y, int block_x, int  qp,                
                      int*  ACLevel, int*  ACRun, 
                      int **fadjust8x8, int **levelscale, int **invlevelscale, int **leveloffset,
                      int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost)
{
  static int i,j, coeff_ctr;

  static int *m7;
  static int scaled_coeff;
  int   level, run = 0;
  int   nonzero = FALSE;
  int   qp_per = qp_per_matrix[qp];
  int   qp_rem = qp_rem_matrix[qp];
  const byte *p_scan = &pos_scan[0][0];
  int*  ACL = &ACLevel[0];
  int*  ACR = &ACRun[0];
  int   levelTrellis[64];

  rdoq_8x8_CABAC(tblock, block_y, block_x, qp_per, qp_rem, levelscale, leveloffset, p_scan, levelTrellis);

  // Quantization
  for (coeff_ctr = 0; coeff_ctr < 64; coeff_ctr++)
  {
    i = *p_scan++;  // horizontal position
    j = *p_scan++;  // vertical position

    m7 = &tblock[j][block_x + i];
    if (*m7 != 0)
    {    
      scaled_coeff = iabs (*m7) * levelscale[j][i];
      level = levelTrellis[coeff_ctr];

      if (level != 0)
      {
        nonzero = TRUE;

        *coeff_cost += (level > 1) ? MAX_VALUE : c_cost[run];

        level  = isignab(level, *m7);
        *m7    = rshift_rnd_sf(((level * invlevelscale[j][i]) << qp_per), 6);
        *ACL++ = level;
        *ACR++ = run; 
        // reset zero level counter
        run    = 0;
      }
      else
      {
        run++;
        *m7 = 0;
      }      
    }
    else
    {
      run++;
    }
  }

  *ACL = 0;

  return nonzero;
}

/*!
 ************************************************************************
 * \brief
 *    Quantization process for All coefficients for a 8x8 block 
 *    CAVLC version
 *
 * \par Input:
 *
 * \par Output:
 *
 ************************************************************************
 */
int quant_8x8cavlc_trellis(int (*tblock)[16], int block_y, int block_x, int  qp, int***  cofAC, 
                           int **fadjust8x8, int **levelscale, int **invlevelscale, int **leveloffset,
                           int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost)
{
  static int i,j, k, coeff_ctr;

  static int *m7;
  int level, runs[4] = { 0 };
  int nonzero = FALSE; 
  int qp_per = qp_per_matrix[qp];  
  int qp_rem = qp_rem_matrix[qp];
  const byte *p_scan = &pos_scan[0][0];
  int*  ACL[4];  
  int*  ACR[4];

  int levelTrellis[4][16];

  rdoq_8x8_CAVLC(tblock, block_y, block_x, qp_per, qp_rem, levelscale, leveloffset, p_scan, levelTrellis);

  for (k = 0; k < 4; k++)
  {
    ACL[k] = &cofAC[k][0][0];
    ACR[k] = &cofAC[k][1][0];
  }

  // Quantization
  for (coeff_ctr = 0; coeff_ctr < 16; coeff_ctr++)
  {
    for (k = 0; k < 4; k++)
    {
      i = *p_scan++;  // horizontal position
      j = *p_scan++;  // vertical position

      m7 = &tblock[j][block_x + i];

      if (m7 != 0)
      {
        level = levelTrellis[k][coeff_ctr];

        if (level != 0)
        {
          nonzero=TRUE;

          *coeff_cost += (level > 1) ? MAX_VALUE : c_cost[runs[k]];

          level  = isignab(level, *m7);
          *m7    = rshift_rnd_sf(((level * invlevelscale[j][i]) << qp_per), 6);

          *(ACL[k])++ = level;
          *(ACR[k])++ = runs[k];
          // reset zero level counter
          runs[k] = 0;
        }
        else
        {        
          runs[k]++;
          *m7 = 0;      
        }
      }
      else
      {
        runs[k]++;
      }
    }
  }

  for(k = 0; k < 4; k++)
    *(ACL[k]) = 0;

  return nonzero;
}

/*!
************************************************************************
* \brief
*    Rate distortion optimized Quantization process for 
*    all coefficients in a 8x8 block
*
************************************************************************
*/
void rdoq_8x8_CABAC(int (*tblock)[16], int block_y, int block_x,int qp_per, int qp_rem, 
              int **levelscale, int **leveloffset, const byte *p_scan, int levelTrellis[64])
{
  levelDataStruct levelData[64];
  double  lambda_md=0;
  int kStart = 0, kStop = 0, noCoeff = 0;
  Macroblock *currMB = &img->mb_data[img->current_mb_nr];

  if ((img->type==B_SLICE) && img->nal_reference_idc)
  {
    lambda_md = img->lambda_md[5][img->masterQP];  
  }
  else
  {
    lambda_md = img->lambda_md[img->type][img->masterQP]; 
  }

  noCoeff = init_trellis_data_8x8_CABAC(tblock, block_x, qp_per, qp_rem, levelscale, leveloffset, p_scan, currMB, &levelData[0], &kStart, &kStop, CABAC);
  est_writeRunLevel_CABAC(levelData, levelTrellis, LUMA_8x8, lambda_md, kStart, kStop, noCoeff, 0);
}

/*!
************************************************************************
* \brief
*    Rate distortion optimized Quantization process for 
*    all coefficients in a 8x8 block
*
************************************************************************
*/
void rdoq_8x8_CAVLC(int (*tblock)[16], int block_y, int block_x, int qp_per, int qp_rem,
                    int **levelscale, int **leveloffset, const byte *p_scan, int levelTrellis[4][16])
{
  int k;
  levelDataStruct levelData[4][16]; 
  double lambda_md;
  Macroblock *currMB = &img->mb_data[img->current_mb_nr];
  int b8 = ((block_y/8)<<1)+block_x/8;

  if ((img->type==B_SLICE) && img->nal_reference_idc)
  {
    lambda_md = img->lambda_md[5][img->masterQP];  
  }
  else
  {
    lambda_md = img->lambda_md[img->type][img->masterQP]; 
  }

  init_trellis_data_8x8_CAVLC (tblock, block_x, qp_per, qp_rem, levelscale, leveloffset, p_scan, currMB, levelData);

  for (k = 0; k < 4; k++)
    est_RunLevel_CAVLC(levelData[k], levelTrellis[k], LUMA, b8, k, 16, lambda_md);
}

