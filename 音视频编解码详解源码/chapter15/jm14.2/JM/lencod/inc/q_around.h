/*!
 ***************************************************************************
 * \file
 *    q_around.h
 *
 * \author
 *    Alexis Michael Tourapis
 *
 * \brief
 *    Headerfile for Quantization Adaptive Rounding
 **************************************************************************
 */

#ifndef _Q_AROUND_H_
#define _Q_AROUND_H_

extern int   **fadjust8x8, **fadjust4x4, ***fadjust4x4Cr, ***fadjust8x8Cr;

typedef struct around_offset
{
  int  **InterFAdjust4x4;
  int  **IntraFAdjust4x4;
  int  **InterFAdjust8x8; 
  int  **IntraFAdjust8x8;
  int ***InterFAdjust4x4Cr;
  int ***IntraFAdjust4x4Cr;
  int ***InterFAdjust8x8Cr;
  int ***IntraFAdjust8x8Cr;
} ARoundOffset;

// Create / Clear adaptive rounding variables
void setup_adaptive_rounding (InputParameters *params);
void clear_adaptive_rounding (InputParameters *params);

void store_adaptive_rounding_parameters        (Macroblock *currMB, int mode);
void store_adaptive_rounding_parameters_luma   (Macroblock *currMB, int mode);
void store_adaptive_rounding_parameters_chroma (Macroblock *currMB, int mode);

void store_adaptive_rounding (ImageParameters *img, int block_y, int block_x);
void update_adaptive_rounding(ImageParameters *img, int block_y, int block_x);
void update_offset_params    (Macroblock *currMB, int mode, int luma_transform_size_8x8_flag);

#endif

