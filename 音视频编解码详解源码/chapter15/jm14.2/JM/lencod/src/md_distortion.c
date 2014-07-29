
/*!
 ***************************************************************************
 * \file md_distortion.c
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
#include "mv-search.h"


int64 compute_SSE(imgpel **imgRef, imgpel **imgSrc, int xRef, int xSrc, int ySize, int xSize)
{
  static int i, j;
  static imgpel *lineRef, *lineSrc;
  int64 distortion = 0;
  

  for (j = 0; j < ySize; j++)
  {
    lineRef = &imgRef[j][xRef];    
    lineSrc = &imgSrc[j][xSrc];

    for (i = 0; i < xSize; i++)
      //distortion += img->quad[( *lineRef++ - *lineSrc++ )];
      distortion += iabs2( *lineRef++ - *lineSrc++ );
  }
  return distortion;
}

/*!
*************************************************************************************
* \brief
*    SSE distortion calculation for a macroblock
*************************************************************************************
*/
int64 distortionSSE(Macroblock *currMB) 
{
  int64 distortionY = 0;
  int64 distortionCr[2] = {0};

  // LUMA
  distortionY = compute_SSE(&pCurImg[img->opix_y], &enc_picture->p_curr_img[img->pix_y], img->opix_x, img->pix_x, MB_BLOCK_SIZE, MB_BLOCK_SIZE);

  //if (img->yuv_format != YUV400 && )
  if ((img->yuv_format != YUV400) && !IS_INDEPENDENT(params))
  {
    // CHROMA
    distortionCr[0] = compute_SSE(&pImgOrg[1][img->opix_c_y], &enc_picture->imgUV[0][img->pix_c_y], img->opix_c_x, img->pix_c_x, img->mb_cr_size_y, img->mb_cr_size_x);
    distortionCr[1] = compute_SSE(&pImgOrg[2][img->opix_c_y], &enc_picture->imgUV[1][img->pix_c_y], img->opix_c_x, img->pix_c_x, img->mb_cr_size_y, img->mb_cr_size_x);
  }

  return (int64)( distortionY * params->WeightY + distortionCr[0] * params->WeightCb + distortionCr[1] * params->WeightCr );
}


