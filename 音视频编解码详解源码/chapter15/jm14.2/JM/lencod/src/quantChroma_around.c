
/*!
 *************************************************************************************
 * \file quantChroma_around.c
 *
 * \brief
 *    Quantization process for a Chroma block
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
int quant_dc2x2_around(int (*tblock)[4], int qp, int* DCLevel, int* DCRun, 
                       int **fadjust, int levelscale, int invlevelscale, int **leveloffset,
                       const byte (*pos_scan)[2], int is_cavlc)
{
  static int coeff_ctr;

  static int *m7;
  static int scaled_coeff;

  int   level, run = 0;
  int   nonzero = FALSE;  
  int   qp_per = qp_per_matrix[qp];
  int   q_bits = Q_BITS + qp_per + 1;
  //const byte *p_scan = &pos_scan[0][0];
  int*  DCL = &DCLevel[0];
  int*  DCR = &DCRun[0];

  m7 = *tblock;

  // Quantization
  for (coeff_ctr=0; coeff_ctr < 4; coeff_ctr++)
  {
    // we need to update leveloffset to a 4x1 array that would contain offset info for 
    // every 2x2 DC position
    if (*m7)
    {
      scaled_coeff = iabs (*m7) * levelscale;
      level = (scaled_coeff + (leveloffset[0][0] << 1) ) >> q_bits;

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
int quant_dc4x2_around(int (*tblock)[4], int qp, int* DCLevel, int* DCRun, 
                       int **fadjust, int levelscale, int invlevelscale, int **leveloffset,
                       const byte (*pos_scan)[2], int is_cavlc)
{
  static int i,j, coeff_ctr;

  static int *m7;
  static int scaled_coeff;

  int   level, run = 0;
  int   nonzero = FALSE;  
  int   qp_per = qp_per_matrix[qp];
  int   q_bits = Q_BITS + qp_per + 1;
  const byte *p_scan = &pos_scan[0][0];
  int*  DCL = &DCLevel[0];
  int*  DCR = &DCRun[0];

  // Quantization
  for (coeff_ctr = 0; coeff_ctr < 8; coeff_ctr++)
  {
    j = *p_scan++;  // note that in this part, somehow coefficients were transposed from 2x4 to 4x2.
    i = *p_scan++;  

    m7 = &tblock[j][i];

    if (*m7 != 0)
    {
      scaled_coeff = iabs (*m7) * levelscale;
      level = (scaled_coeff + (leveloffset[0][0] << 1) ) >> q_bits;

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



