/*!
 ***************************************************************************
 * \file
 *    cconv_yuv2rgb.h
 *
 * \author
 *    Woo-Shik Kim
 *
 * \date
 *    29 May 2008
 *
 * \brief
 *    Headerfile for YUV to RGB color conversion
 **************************************************************************
 */

#ifndef _CCONV_YUV2RGB_H_
#define _CCONV_YUV2RGB_H_

#include "img_distortion.h"

void init_YUVtoRGB(void);
void YUVtoRGB(ImageStructure *YUV, ImageStructure *RGB);
int  create_RGB_memory(ImageParameters *img);
void delete_RGB_memory(void);

#endif

