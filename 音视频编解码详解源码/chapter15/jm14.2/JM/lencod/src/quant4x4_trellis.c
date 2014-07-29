
/*!
 *************************************************************************************
 * \file quant4x4_trellis.c
 *
 * \brief
 *    Quantization process for a 4x4 block using trellis based quantization
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Qualcomm      
 *    - Limin Liu                                <limin.liu@dolby.com>
 *    - Alexis Michael Tourapis                  <alexismt@ieee.org>
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
#include "quant4x4.h"
#include "rdoq.h"




/*!
 ************************************************************************
 * \brief
 *    Quantization process for All coefficients for a 4x4 block
 *
 * \par Input:
 *
 * \par Output:
 *
 ************************************************************************
 */
int quant_4x4_trellis(int (*tblock)[16], int block_y, int block_x, int  qp,                
                      int*  ACLevel, int*  ACRun, 
                      int **fadjust4x4, int **levelscale, int **invlevelscale, int **leveloffset,
                      int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost, int is_cavlc)
{
  static int i,j, coeff_ctr;

  static int *m7;

  int   level, run = 0;
  int   nonzero = FALSE;
  int   qp_per = qp_per_matrix[qp];
  int   qp_rem = qp_rem_matrix[qp];
  const byte *p_scan = &pos_scan[0][0];
  int*  ACL = &ACLevel[0];
  int*  ACR = &ACRun[0];

  static int levelTrellis[16];

  rdoq_4x4(tblock, block_y, block_x, qp_per, qp_rem, levelscale, leveloffset, pos_scan, levelTrellis);

  // Quantization
  for (coeff_ctr = 0; coeff_ctr < 16; coeff_ctr++)
  {
    i = *p_scan++;  // horizontal position
    j = *p_scan++;  // vertical position

    m7 = &tblock[j][block_x + i];

    if (*m7 != 0)
    {    
      /*
      scaled_coeff = iabs (*m7) * levelscale[j][i];
      level = (scaled_coeff + leveloffset[j][i]) >> q_bits;
      */
      level = levelTrellis[coeff_ctr];

      if (level != 0)
      {
        if (is_cavlc)
          level = imin(level, CAVLC_LEVEL_LIMIT);

        *coeff_cost += (level > 1) ? MAX_VALUE : c_cost[run];

        level   = isignab(level, *m7);
        *m7     = rshift_rnd_sf(((level * invlevelscale[j][i]) << qp_per), 4);
        *ACL++  = level;
        *ACR++  = run; 
        // reset zero level counter
        run     = 0;
        nonzero = TRUE;        
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
*    Rate distortion optimized Quantization process for 
*    all coefficients in a 4x4 block (CAVLC)
*
************************************************************************
*/
void rdoq_4x4_CAVLC(int (*tblock)[16], int block_y, int block_x, int qp_per, int qp_rem, 
                    int **levelscale, int **leveloffset, const byte (*pos_scan)[2], int levelTrellis[])
{
  const byte *p_scan = &pos_scan[0][0];
  levelDataStruct levelData[16];  
  double  lambda_md = 0;
  Macroblock *currMB = &img->mb_data[img->current_mb_nr];
  int type = LUMA_4x4;
  int   pos_x   = block_x >> BLOCK_SHIFT;
  int   pos_y   = block_y >> BLOCK_SHIFT;
  int   b8      = 2*(pos_y >> 1) + (pos_x >> 1);
  int   b4      = 2*(pos_y & 0x01) + (pos_x & 0x01);

  if ((img->type==B_SLICE) && img->nal_reference_idc)
  {
    lambda_md = img->lambda_md[5][img->masterQP];  
  }
  else
  {
    lambda_md = img->lambda_md[img->type][img->masterQP]; 
  }

  init_trellis_data_4x4_CAVLC(tblock, block_x, qp_per, qp_rem, levelscale, leveloffset, p_scan, currMB, &levelData[0], type);
  est_RunLevel_CAVLC(levelData, levelTrellis, LUMA, b8, b4, 16, lambda_md);
}
/*!
************************************************************************
* \brief
*    Rate distortion optimized Quantization process for 
*    all coefficients in a 4x4 block (CABAC)
*
************************************************************************
*/
void rdoq_4x4_CABAC(int (*tblock)[16], int block_y, int block_x, int qp_per, int qp_rem, 
              int **levelscale, int **leveloffset, const byte (*pos_scan)[2], int levelTrellis[])
{
  const byte *p_scan = &pos_scan[0][0];
  levelDataStruct levelData[16];
  double  lambda_md = 0;
  int kStart=0, kStop=0, noCoeff = 0, estBits;
  Macroblock *currMB = &img->mb_data[img->current_mb_nr];
  int type = LUMA_4x4;

  if ((img->type==B_SLICE) && img->nal_reference_idc)
  {
    lambda_md = img->lambda_md[5][img->masterQP];  
  }
  else
  {
    lambda_md = img->lambda_md[img->type][img->masterQP]; 
  }

  noCoeff = init_trellis_data_4x4_CABAC(tblock, block_x, qp_per, qp_rem, levelscale, leveloffset, p_scan, currMB, &levelData[0], &kStart, &kStop, type);
  estBits = est_write_and_store_CBP_block_bit(currMB, LUMA_4x4);
  est_writeRunLevel_CABAC(levelData, levelTrellis, LUMA_4x4, lambda_md, kStart, kStop, noCoeff, estBits);
}

/*!
 ************************************************************************
 * \brief
 *    Quantization process for All coefficients for a 4x4 block (LUMA_16AC or CHROMA_AC)
 *
 * \par Input:
 *
 * \par Output:
 *
 ************************************************************************
 */
int quant_ac4x4_trellis(int (*tblock)[16], int block_y, int block_x, int qp,                
                        int*  ACLevel, int*  ACRun, 
                        int **fadjust4x4, int **levelscale, int **invlevelscale, int **leveloffset,
                        int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost, int type, int is_calvc)
{
  static int i,j, coeff_ctr;

  static int *m7;
  int   level, run = 0;
  int   nonzero = FALSE;  
  int   qp_per = qp_per_matrix[qp];
  int   qp_rem = qp_rem_matrix[qp];
  const byte *p_scan = &pos_scan[1][0];
  int*  ACL = &ACLevel[0];
  int*  ACR = &ACRun[0];

  static int levelTrellis[16]; 

  rdoq_ac4x4(tblock, block_y, block_x, qp_per, qp_rem, levelscale, leveloffset, pos_scan, levelTrellis, type);

  // Quantization
  for (coeff_ctr = 1; coeff_ctr < 16; coeff_ctr++)
  {
    i = *p_scan++;  // horizontal position
    j = *p_scan++;  // vertical position

    m7 = &tblock[j][block_x + i];
    if (*m7 != 0)
    {    
      /*
      scaled_coeff = iabs (*m7) * levelscale[j][i];
      level = (scaled_coeff + leveloffset[j][i]) >> q_bits;
      */
      level=levelTrellis[coeff_ctr - 1];

      if (level != 0)
      {
        if (is_calvc)
          level = imin(level, CAVLC_LEVEL_LIMIT);

        *coeff_cost += (level > 1) ? MAX_VALUE : c_cost[run];

        level  = isignab(level, *m7);
        *m7    = rshift_rnd_sf(((level * invlevelscale[j][i]) << qp_per), 4);
        // inverse scale can be alternative performed as follows to ensure 16bit
        // arithmetic is satisfied.
        // *m7 = (qp_per<4) ? rshift_rnd_sf((level*invlevelscale[j][i]),4-qp_per) : (level*invlevelscale[j][i])<<(qp_per-4);
        *ACL++  = level;
        *ACR++  = run; 
        // reset zero level counter
        run     = 0;
        nonzero = TRUE;
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
*    Rate distortion optimized Quantization process for 
*    all coefficients in a 4x4 block (CAVLC)
*
************************************************************************
*/
void rdoq_ac4x4_CAVLC(int (*tblock)[16], int block_y, int block_x, int qp_per, int qp_rem, 
                    int **levelscale, int **leveloffset, const byte (*pos_scan)[2], int levelTrellis[], int type)
{
  const byte *p_scan = &pos_scan[1][0];
  levelDataStruct levelData[16];  
  double  lambda_md = 0;
  Macroblock *currMB = &img->mb_data[img->current_mb_nr];
  int   pos_x   = block_x >> BLOCK_SHIFT;
  int   pos_y   = block_y >> BLOCK_SHIFT;
  int   b8      = 2*(pos_y >> 1) + (pos_x >> 1);
  int   b4      = 2*(pos_y & 0x01) + (pos_x & 0x01);
  int   block_type = ( (type == CHROMA_AC) ? CHROMA_AC : LUMA_INTRA16x16AC);

  if ((img->type==B_SLICE) && img->nal_reference_idc)
  {
    lambda_md = img->lambda_md[5][img->masterQP];  
  }
  else
  {
    lambda_md = img->lambda_md[img->type][img->masterQP]; 
  }

  init_trellis_data_4x4_CAVLC(tblock, block_x, qp_per, qp_rem, levelscale, leveloffset, p_scan, currMB, &levelData[0], type);
  est_RunLevel_CAVLC(levelData, levelTrellis, block_type, b8, b4, 15, lambda_md);
}
/*!
************************************************************************
* \brief
*    Rate distortion optimized Quantization process for 
*    all coefficients in a 4x4 block (LUMA_16AC or CHROMA_AC) - CABAC
*
************************************************************************
*/
void rdoq_ac4x4_CABAC(int (*tblock)[16] , int block_y, int block_x, int qp_per, int qp_rem, 
                int **levelscale, int **leveloffset, const byte (*pos_scan)[2], int levelTrellis[], int type)
{
  const byte *p_scan = &pos_scan[1][0];
  levelDataStruct levelData[16];
  double  lambda_md=0;
  int kStart = 0, kStop = 0, noCoeff = 0, estBits;
  Macroblock *currMB = &img->mb_data[img->current_mb_nr];

  if ((img->type==B_SLICE) && img->nal_reference_idc)
  {
    lambda_md = img->lambda_md[5][img->masterQP];  
  }
  else
  {
    lambda_md = img->lambda_md[img->type][img->masterQP]; 
  }

  noCoeff = init_trellis_data_4x4_CABAC(tblock, block_x, qp_per, qp_rem, levelscale, leveloffset, p_scan, currMB, &levelData[0], &kStart, &kStop, type);
  estBits = est_write_and_store_CBP_block_bit(currMB, type);
  est_writeRunLevel_CABAC(levelData, levelTrellis, type, lambda_md, kStart, kStop, noCoeff, estBits);
}

/*!
 ************************************************************************
 * \brief
 *    Quantization process for All coefficients for a 4x4 DC block
 *
 * \par Input:
 *
 * \par Output:
 *
 ************************************************************************
 */
int quant_dc4x4_trellis(int (*tblock)[4], int qp, int* DCLevel, int* DCRun, 
                       int levelscale, int invlevelscale, int leveloffset, const byte (*pos_scan)[2], int is_calvc)
{
  static int i,j, coeff_ctr;

  static int *m7;

  int   level, run = 0;
  int   nonzero = FALSE;  
  int   qp_per = qp_per_matrix[qp];
  int   qp_rem = qp_rem_matrix[qp];
  const byte *p_scan = &pos_scan[0][0];
  int*  DCL = &DCLevel[0];
  int*  DCR = &DCRun[0];

  static int levelTrellis[16];

  rdoq_dc(tblock, qp_per, qp_rem, levelscale, leveloffset, pos_scan, levelTrellis, LUMA_16DC);

  // Quantization
  for (coeff_ctr = 0; coeff_ctr < 16; coeff_ctr++)
  {
    i = *p_scan++;  // horizontal position
    j = *p_scan++;  // vertical position

    m7 = &tblock[j][i];

    if (*m7 != 0)
    {    
      level = levelTrellis[coeff_ctr];

      if (level != 0)
      {
        if (is_calvc)
          level = imin(level, CAVLC_LEVEL_LIMIT);

        level   = isignab(level, *m7);
        *m7     = level;
        *DCL++  = level;
        *DCR++  = run;
        // reset zero level counter
        run     = 0;
        nonzero = TRUE;
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

  *DCL = 0;

  return nonzero;
}

/*!
************************************************************************
* \brief
*    Rate distortion optimized Quantization process for 
*    all coefficients in a luma DC block 
*
************************************************************************
*/
void rdoq_dc_CAVLC(int (*tblock)[4], int qp_per, int qp_rem, int levelscale, int leveloffset, const byte (*pos_scan)[2], int levelTrellis[], int type)
{
  const byte *p_scan = &pos_scan[0][0];
  levelDataStruct levelData[16];
  double  lambda_md = 0;
  Macroblock *currMB = &img->mb_data[img->current_mb_nr];
  
  if ((img->type==B_SLICE) && img->nal_reference_idc)
  {
    lambda_md = img->lambda_md[5][img->masterQP];  
  }
  else
  {
    lambda_md = img->lambda_md[img->type][img->masterQP]; 
  }

  init_trellis_data_DC_CAVLC(tblock, qp_per, qp_rem, levelscale, leveloffset, p_scan, currMB, &levelData[0], type);
  est_RunLevel_CAVLC(levelData, levelTrellis, LUMA_INTRA16x16DC, 0, 0, 16, lambda_md);
}


/*!
************************************************************************
* \brief
*    Rate distortion optimized Quantization process for 
*    all coefficients in a luma DC block 
*
************************************************************************
*/
void rdoq_dc_CABAC(int (*tblock)[4], int qp_per, int qp_rem, int levelscale, int leveloffset, const byte (*pos_scan)[2], int levelTrellis[], int type)
{
  const byte *p_scan = &pos_scan[0][0];
  levelDataStruct levelData[16];
  double  lambda_md = 0;
  int kStart=0, kStop=0, noCoeff = 0, estBits;
  Macroblock *currMB = &img->mb_data[img->current_mb_nr];

  if ((img->type==B_SLICE) && img->nal_reference_idc)
  {
    lambda_md = img->lambda_md[5][img->masterQP];  
  }
  else
  {
    lambda_md = img->lambda_md[img->type][img->masterQP]; 
  }

  noCoeff = init_trellis_data_DC_CABAC(tblock, qp_per, qp_rem, levelscale, leveloffset, p_scan, currMB, &levelData[0], &kStart, &kStop);
  estBits = est_write_and_store_CBP_block_bit(currMB, type);
  est_writeRunLevel_CABAC(levelData, levelTrellis, type, lambda_md, kStart, kStop, noCoeff, estBits);
}

