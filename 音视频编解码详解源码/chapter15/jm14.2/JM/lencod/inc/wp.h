/*!
 ***************************************************************************
 * \file
 *    wp.h
 *
 * \author
 *    Alexis Michael Tourapis
 *
 * \date
 *    22. February 2008
 *
 * \brief
 *    Headerfile for weighted prediction support
 **************************************************************************
 */

#ifndef _WP_H_
#define _WP_H_

#include "wp_lms.h"
#include "wp_mcprec.h"

#define DEBUG_WP  0

void InitWP(InputParameters *params);
void (*EstimateWPBSlice) (ImageParameters *img, InputParameters *params);
void (*EstimateWPPSlice) (ImageParameters *img, InputParameters *params, int offset);
int  (*TestWPPSlice)     (ImageParameters *img, InputParameters *params, int offset);
int  (*TestWPBSlice)     (ImageParameters *img, InputParameters *params, int method);

void EstimateWPBSliceAlg0(ImageParameters *img, InputParameters *params);
void EstimateWPPSliceAlg0(ImageParameters *img, InputParameters *params, int offset);
int  TestWPPSliceAlg0    (ImageParameters *img, InputParameters *params, int offset);
int  TestWPBSliceAlg0    (ImageParameters *img, InputParameters *params, int method);

double ComputeImgSum     (imgpel **CurrentImage, int height, int width);

#endif

