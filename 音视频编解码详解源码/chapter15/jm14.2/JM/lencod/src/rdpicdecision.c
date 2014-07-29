
/*!
*************************************************************************************
* \file rdpicdecision.c
*
* \brief
*    Perform RD optimal decisions between multiple coded versions of the same picture
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*     - Alexis Michael Tourapis         <alexismt@ieee.org>
*************************************************************************************
*/

#include "global.h"
#include <math.h>


/*!
 ************************************************************************
 * \brief
 *    RD decision between possible encoding cases
 ************************************************************************
 */
int rd_pic_decision(double snrY_version1, double snrY_version2, int bits_version1, int bits_version2, double lambda_picture)
{
  double cost_version1, cost_version2;

  cost_version1 = (double) bits_version1 * lambda_picture + snrY_version1;
  cost_version2 = (double) bits_version2 * lambda_picture + snrY_version2;
  //printf("%d %d %.2f %.2f %.2f %.2f \n",bits_version1,bits_version2,snrY_version1,snrY_version2,cost_version1,cost_version2);
  if (cost_version2 > cost_version1 || (cost_version2 == cost_version1 && snrY_version2 >= snrY_version1) )
    return (0);
  else
    return (1);
}

/*!
 ************************************************************************
 * \brief
 *    Picture Coding Decision
 ************************************************************************
 */
int picture_coding_decision (Picture *picture1, Picture *picture2, int qp)
{
  double lambda_picture;
  int spframe = (img->type == SP_SLICE);
  int bframe = (img->type == B_SLICE);
  double sse_picture1, sse_picture2;

  if (params->successive_Bframe)
    lambda_picture = (qp < 20 ? 0.55 : 0.68) * pow (2, (qp - SHIFT_QP) / 3.0) * (bframe || spframe ? 2 : 1);
  else
    lambda_picture = (qp < 20 ? 0.55 : 0.68) * pow (2, (qp - SHIFT_QP) / 3.0);

  sse_picture1 = picture1->distortion.value[0] + picture1->distortion.value[1] + picture1->distortion.value[2];
  sse_picture2 = picture2->distortion.value[0] + picture2->distortion.value[1] + picture2->distortion.value[2];
 
  return rd_pic_decision(sse_picture1, sse_picture2, picture1->bits_per_picture, picture2->bits_per_picture, lambda_picture);
}

