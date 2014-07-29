
/*!
 *************************************************************************************
 * \file quant8x8.c
 *
 * \brief
 *    Quantization process for a 8x8 block
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
#include "quant8x8.h"


/*!
************************************************************************
* \brief
*    Quantization initialization function
*
************************************************************************
*/
void init_quant_8x8(InputParameters *params, ImageParameters *img, Slice *currSlice)
{
  // We may wish to have all these parameters switched at the slice level for speed up.
  if (params->UseRDOQuant == 1)
  {
    quant_8x8 = quant_8x8_trellis;
    quant_8x8cavlc = quant_8x8cavlc_trellis;
  }
  else if (img->AdaptiveRounding)
  {
    quant_8x8 = quant_8x8_around;
    quant_8x8cavlc = quant_8x8cavlc_around;
  }
  else
  {
    quant_8x8 = quant_8x8_normal;
    quant_8x8cavlc = quant_8x8cavlc_normal;
  }
}

