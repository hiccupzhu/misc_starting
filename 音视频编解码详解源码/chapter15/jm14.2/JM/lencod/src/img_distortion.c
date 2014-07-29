
/*!
 *************************************************************************************
 * \file img_distortion.c
 *
 * \brief
 *    Compute distortion for encoded image
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Woo-Shik Kim                    <wooshik.kim@usc.edu>
 *     - Alexis Michael Tourapis         <alexismt@ieee.org> 
 *************************************************************************************
 */

#include <math.h>
#include <time.h>
#include <sys/timeb.h>

#include "global.h"

#include "refbuf.h"
#include "mbuffer.h"
#include "img_luma.h"
#include "img_chroma.h"
#include "intrarefresh.h"
#include "fmo.h"
#include "sei.h"
#include "memalloc.h"
#include "nalu.h"
#include "ratectl.h"
#include "mb_access.h"
#include "output.h"
#include "cabac.h"
#include "context_ini.h"
#include "conformance.h"
#include "enc_statistics.h"

#include "q_matrix.h"
#include "q_offsets.h"
#include "quant4x4.h"
#include "quant8x8.h"
#include "wp.h"
#include "input.h"
#include "image.h"
#include "img_distortion.h"
#include "img_dist_snr.h"
#include "img_dist_ssim.h"
#include "img_dist_ms_ssim.h"
#include "cconv_yuv2rgb.h"

// images for distortion calculation
ImageStructure imgSRC, imgREF;
extern ImageStructure imgRGB_src, imgRGB_ref;

/*!
 ************************************************************************
 * \brief
 *    Metric accumulator
 ************************************************************************
 */
void accumulate_metric(float *ave_metric, float cur_metric, int frames)
{
  *ave_metric = (float) (*ave_metric * (frames - 1) + cur_metric) / frames;
}

/*!
 ************************************************************************
 * \brief
 *    Accumulate distortion for all components
 ************************************************************************
 */
void accumulate_average(DistMetric *metric, int frames)
{
  accumulate_metric(&metric->average[0],  metric->value[0],  frames);
  accumulate_metric(&metric->average[1],  metric->value[1],  frames);
  accumulate_metric(&metric->average[2],  metric->value[2],  frames);
}

/*!
 ************************************************************************
 * \brief
 *    Accumulate distortion for all components for slice_type
 ************************************************************************
 */
void accumulate_avslice(DistMetric *metric, int slice_type, int frames)
{
  accumulate_metric(&metric->avslice[slice_type][0], metric->value[0],  frames);
  accumulate_metric(&metric->avslice[slice_type][1], metric->value[1],  frames);
  accumulate_metric(&metric->avslice[slice_type][2], metric->value[2],  frames);
}

/*!
 ************************************************************************
 * \brief
 *    Find distortion for all three components
 ************************************************************************
 */
void find_distortion (void)
{
  int64 diff_cmp[3] = {0};

  //  Calculate SSE for Y, U and V.
  if (img->structure!=FRAME)
  {
    // Luma.
    diff_cmp[0] += compute_SSE(pCurImg, imgY_com, 0, 0, params->output.height, params->output.width);

    // Chroma.
    if (img->yuv_format != YUV400)
    {
      diff_cmp[1] += compute_SSE(pImgOrg[1], imgUV_com[0], 0, 0, params->output.height_cr, params->output.width_cr);
      diff_cmp[2] += compute_SSE(pImgOrg[2], imgUV_com[1], 0, 0, params->output.height_cr, params->output.width_cr);
    }
  }
  else
  {
    if( IS_INDEPENDENT(params) )
    {
      enc_picture = enc_frame_picture[0];     
    }
    pCurImg   = img_org_frm[0];
    pImgOrg[0] = img_org_frm[0];

    // Luma.
    diff_cmp[0] += compute_SSE(pImgOrg[0], enc_picture->imgY, 0, 0, params->output.height, params->output.width);

    // Chroma.
    if (img->yuv_format != YUV400)
    {
      pImgOrg[1] = img_org_frm[1];
      pImgOrg[2] = img_org_frm[2]; 

      diff_cmp[1] += compute_SSE(pImgOrg[1], enc_picture->imgUV[0], 0, 0, params->output.height_cr, params->output.width_cr);
      diff_cmp[2] += compute_SSE(pImgOrg[2], enc_picture->imgUV[1], 0, 0, params->output.height_cr, params->output.width_cr);
    }
  }

  // This should be assigned to the SSE structure. Should double check code.
  dist->metric[SSE].value[0] = (float) diff_cmp[0];
  dist->metric[SSE].value[1] = (float) diff_cmp[1];
  dist->metric[SSE].value[2] = (float) diff_cmp[2];
}

void select_img(ImageStructure *imgSRC, ImageStructure *imgREF)
{
  if (img->fld_flag != FALSE)
  {
    imgSRC->format = params->output;
    imgREF->format = params->output;

    imgREF->data[0] = pCurImg;
    imgSRC->data[0] = imgY_com;

    if (img->yuv_format != YUV400)
    {
      imgREF->data[1] = pImgOrg[1];
      imgREF->data[2] = pImgOrg[2];
      imgSRC->data[1] = imgUV_com[0];
      imgSRC->data[2] = imgUV_com[1];
    }
  }
  else
  {
    imgSRC->format = params->output;
    imgREF->format = params->output;

    imgREF->data[0] = img_org_frm[0];

    if ((params->PicInterlace == ADAPTIVE_CODING) || IS_INDEPENDENT(params))
    {
      enc_picture = enc_frame_picture[0];
    }
    imgSRC->data[0] = enc_picture->imgY;

    if (img->yuv_format != YUV400)
    {
      imgREF->data[1] = img_org_frm[1];
      imgREF->data[2] = img_org_frm[2];

      imgSRC->data[1] = enc_picture->imgUV[0];
      imgSRC->data[2] = enc_picture->imgUV[1];
    }
  }
}

void compute_distortion(void)
{
  if (params->Verbose != 0)
  {
    select_img(&imgSRC, &imgREF);

    find_snr (&imgREF, &imgSRC, &dist->metric[SSE], &dist->metric[PSNR]);
    if (params->Distortion[SSIM] == 1)
      find_ssim (&imgREF, &imgSRC, &dist->metric[SSIM]);
    if (params->Distortion[MS_SSIM] == 1)
      find_ms_ssim(&imgREF, &imgSRC, &dist->metric[MS_SSIM]);
    // RGB Distortion
    if(params->DistortionYUVtoRGB == 1)
    {
      YUVtoRGB(&imgREF, &imgRGB_ref);
      YUVtoRGB(&imgSRC, &imgRGB_src);
      find_snr (&imgRGB_ref, &imgRGB_src, &dist->metric[SSE_RGB], &dist->metric[PSNR_RGB]);
      if (params->Distortion[SSIM] == 1)
        find_ssim (&imgRGB_ref, &imgRGB_src, &dist->metric[SSIM_RGB]);
      if (params->Distortion[MS_SSIM] == 1)
        find_ms_ssim(&imgRGB_ref, &imgRGB_src, &dist->metric[MS_SSIM_RGB]);
    }
  }
}
