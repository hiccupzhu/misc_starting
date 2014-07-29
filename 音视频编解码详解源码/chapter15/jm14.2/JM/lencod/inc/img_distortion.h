
/*!
 ************************************************************************
 * \file img_distortion.h
 *
 * \brief
 *    Distortion related definitions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Alexis Michael Tourapis         <alexismt@ieee.org>
 *     - Woo-Shik Kim                    <wooshik.kim@usc.edu>
 *
 ************************************************************************
 */

#ifndef _IMG_DISTORTION_H_
#define _IMG_DISTORTION_H_

typedef struct
{  
  FrameFormat format;      //!< ImageStructure format Information
  imgpel **data[3];        //!< ImageStructure pixel data
} ImageStructure;

void accumulate_avslice(DistMetric metric[3], int slice_type, int frames);
void accumulate_average(DistMetric metric[3], int frames);
void find_distortion(void);
void select_img(ImageStructure *imgSRC, ImageStructure *imgREF);
void compute_distortion(void);

#endif

