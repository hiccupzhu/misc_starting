
/*!
 *************************************************************************************
 * \file quant4x4.c
 *
 * \brief
 *    Quantization process for a 4x4 block
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Alexis Michael Tourapis                  <alexismt@ieee.org>
 *
 *************************************************************************************
 */

#include "contributors.h"
#include "global.h"
#include "quant4x4.h"

/*!
************************************************************************
* \brief
*    Quantization initialization function
*
************************************************************************
*/
void init_quant_4x4(InputParameters *params, ImageParameters *img, Slice *currSlice)
{
  if (params->UseRDOQuant == 1)
  {
    quant_4x4     = quant_4x4_trellis;
    if (params->RDOQ_DC == 1)
      quant_dc4x4 = quant_dc4x4_trellis;
    else
      quant_dc4x4 = quant_dc4x4_normal;
    quant_ac4x4   = quant_ac4x4_trellis;
    if (currSlice->symbol_mode == CAVLC)
    {
      rdoq_4x4   = rdoq_4x4_CAVLC;
      rdoq_dc    = rdoq_dc_CAVLC;
      rdoq_ac4x4 = rdoq_ac4x4_CAVLC;
    }
    else
    {
      rdoq_4x4   = rdoq_4x4_CABAC;
      rdoq_dc    = rdoq_dc_CABAC;
      rdoq_ac4x4 = rdoq_ac4x4_CABAC;
    }
  }
  else if (img->AdaptiveRounding)
  {
    quant_4x4     = quant_4x4_around;
    quant_dc4x4   = quant_dc4x4_normal;
    quant_ac4x4   = quant_ac4x4_around;
  }
  else
  {
    quant_4x4   = quant_4x4_normal;
    quant_dc4x4 = quant_dc4x4_normal;
    quant_ac4x4 = quant_ac4x4_normal;
  }
}

