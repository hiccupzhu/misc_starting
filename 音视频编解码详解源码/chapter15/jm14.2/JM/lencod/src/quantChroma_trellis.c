
/*!
 *************************************************************************************
 * \file quantChroma_trellis.c
 *
 * \brief
 *    Quantization process for a Chroma block (trellis based)
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Alexis Michael Tourapis                  <alexismt@ieee.org>
 *
 *************************************************************************************
 */

#include "contributors.h"

#include <math.h>

#include "global.h"
#include "q_matrix.h"
#include "quant4x4.h"
#include "quantChroma.h"
#include "rdoq.h"

void rdoq_dc_cr(int (*tblock)[4], int qp_per, int qp_rem, int levelscale, int **leveloffset, const byte (*pos_scan)[2], int levelTrellis[16], int type);

/*!
 ************************************************************************
 * \brief
 *    Quantization process for All coefficients for a 2x2 DC block
 *
 * \par Input:
 *
 * \par Output:
 *
 ************************************************************************
 */
int quant_dc2x2_trellis(int (*tblock)[4], int qp, int* DCLevel, int* DCRun, 
                       int **fadjust, int levelscale, int invlevelscale, int **leveloffset,
                       const byte (*pos_scan)[2], int is_cavlc)
{
  static int coeff_ctr;

  static int *m7;

  int   level, run = 0;
  int   nonzero = FALSE;  
  int   qp_per = qp_per_matrix[qp];
  int   qp_rem = qp_rem_matrix[qp]; 
  //const byte *p_scan = &pos_scan[0][0];
  int*  DCL = &DCLevel[0];
  int*  DCR = &DCRun[0];

  static int levelTrellis[16];

  rdoq_dc_cr(tblock,qp_per,qp_rem, levelscale, leveloffset,pos_scan, levelTrellis, CHROMA_DC);

  m7 = *tblock;

  // Quantization
  for (coeff_ctr=0; coeff_ctr < 4; coeff_ctr++)
  {
    // we need to update leveloffset to a 4x1 array that would contain offset info for 
    // every 2x2 DC position
    if (*m7)
    {
      level = levelTrellis[coeff_ctr];

      if (level  != 0)
      {
        if (is_cavlc)
          level = imin(level, CAVLC_LEVEL_LIMIT);

        level = isignab(level, *m7);

        *m7++ = ((level * invlevelscale) << qp_per);

        *DCL++  = level;
        *DCR++  = run;
        // reset zero level counter
        run     = 0;
        nonzero = TRUE;
      }
      else
      {
        run++;
        *m7++ = 0;
      }
    }
    else
    {
      run++;
      m7++;
    }
  }

  *DCL = 0;

  return nonzero;
}

/*!
 ************************************************************************
 * \brief
 *    Quantization process for All coefficients for a 2x2 DC block
 *
 * \par Input:
 *
 * \par Output:
 *
 ************************************************************************
 */
int quant_dc4x2_trellis(int (*tblock)[4], int qp, int* DCLevel, int* DCRun, 
                       int **fadjust, int levelscale, int invlevelscale, int **leveloffset,
                       const byte (*pos_scan)[2], int is_cavlc)
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

  rdoq_dc_cr(tblock,qp_per,qp_rem, levelscale, leveloffset,pos_scan, levelTrellis, CHROMA_DC_2x4);

  for (coeff_ctr=0; coeff_ctr < 4; coeff_ctr++)
  {
    j = *p_scan++;  // note that in this part, somehow coefficients were transposed from 2x4 to 4x2.
    i = *p_scan++;  

    m7 = &tblock[j][i];

    if (*m7 != 0)
    {
      level = levelTrellis[coeff_ctr];

      if (level  != 0)
      {
        if (is_cavlc)
          level = imin(level, CAVLC_LEVEL_LIMIT);
        level = isignab(level, *m7);

        *m7 = ((level * invlevelscale) << qp_per);

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
*    all coefficients in a chroma DC block
*
************************************************************************
*/
void rdoq_dc_cr(int (*tblock)[4], int qp_per, int qp_rem, int levelscale, int **leveloffset, const byte (*pos_scan)[2], int levelTrellis[], int type)
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

  noCoeff = init_trellis_data_DC_cr(tblock, qp_per, qp_rem, levelscale, leveloffset, p_scan, currMB, &levelData[0], &kStart, &kStop, type);
  estBits = est_write_and_store_CBP_block_bit(currMB, type);
  est_writeRunLevel_CABAC(levelData, levelTrellis, type, lambda_md, kStart, kStop, noCoeff, estBits);
}





