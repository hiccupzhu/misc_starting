
/*!
 ************************************************************************
 * \file quant4x4.h
 *
 * \brief
 *    Quantization process header file
 *
 * \author
 *    Alexis Michael Tourapis         <alexismt@ieee.org>                
 *
 ************************************************************************
 */

#ifndef _QUANT8x8_H_
#define _QUANT8x8_H_

void init_quant_8x8(InputParameters *params, ImageParameters *img, Slice *currSlice);

int quant_8x8_normal(int (*tblock)[16], int block_y, int block_x, int qp, 
                     int*  ACLevel, int*  ACRun, 
                     int **fadjust8x8, int **levelscale, int **invlevelscale, int **leveloffset,
                     int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost);

int quant_8x8_around(int (*tblock)[16], int block_y, int block_x, int qp, 
                     int*  ACLevel, int*  ACRun, 
                     int **fadjust8x8, int **levelscale, int **invlevelscale, int **leveloffset,
                     int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost);

int quant_8x8_trellis (int (*tblock)[16], int block_y, int block_x, int qp,
                       int*  ACLevel, int*  ACRun, 
                       int **fadjust4x4, int **levelscale, int **invlevelscale, int **leveloffset,
                       int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost);

int (*quant_8x8)    (int (*tblock)[16], int block_y, int block_x, int qp, 
                     int*  ACLevel, int*  ACRun, 
                     int **fadjust8x8, int **levelscale, int **invlevelscale, int **leveloffset,
                     int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost);

int quant_8x8cavlc_around(int (*tblock)[16], int block_y, int block_x, int  qp,                 
                          int***  cofAC, 
                          int **fadjust8x8, int **levelscale, int **invlevelscale, int **leveloffset,
                          int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost);

int quant_8x8cavlc_normal(int (*tblock)[16], int block_y, int block_x, int  qp,                 
                          int***  cofAC, 
                          int **fadjust8x8, int **levelscale, int **invlevelscale, int **leveloffset,
                          int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost);

int quant_8x8cavlc_trellis(int (*tblock)[16], int block_y, int block_x, int  qp,                 
                           int***  cofAC, 
                           int **fadjust8x8, int **levelscale, int **invlevelscale, int **leveloffset,
                           int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost);

int (*quant_8x8cavlc)(int (*tblock)[16], int block_y, int block_x, int  qp,                 
                      int***  cofAC, 
                      int **fadjust8x8, int **levelscale, int **invlevelscale, int **leveloffset,
                      int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost);

#endif

