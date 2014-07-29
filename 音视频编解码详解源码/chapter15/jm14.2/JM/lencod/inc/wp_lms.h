/*!
 ***************************************************************************
 * \file
 *    wp_lms.h
 *
 * \author
 *    Alexis Michael Tourapis
 *
 * \date
 *    22. February 2008
 *
 * \brief
 *    Headerfile for weighted prediction support using LMS
 **************************************************************************
 */

#ifndef _WP_LMS_H_
#define _WP_LMS_H_

void EstimateWPPSliceAlg1(ImageParameters *img, InputParameters *params, int offset);
void EstimateWPBSliceAlg1(ImageParameters *img, InputParameters *params);
int  TestWPPSliceAlg1    (ImageParameters *img, InputParameters *input, int offset);
int  TestWPBSliceAlg1    (ImageParameters *img, InputParameters *input, int method);

#endif

