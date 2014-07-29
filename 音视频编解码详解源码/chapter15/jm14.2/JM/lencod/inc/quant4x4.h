
/*!
 ************************************************************************
 * \file quant4x4.h
 *
 * \brief
 *    Quantization process header file
 *
 * \author
 *    Limin Liu                       <lliu@dolby.com>
 *    Alexis Michael Tourapis         <alexismt@ieee.org>                
 *
 ************************************************************************
 */

#ifndef _QUANT4x4_H_
#define _QUANT4x4_H_

void init_quant_4x4(InputParameters *params, ImageParameters *img, Slice *currSlice);

int quant_4x4_normal (int (*tblock)[16], int block_y, int block_x, int qp, 
                      int*  ACLevel, int*  ACRun, 
                      int **fadjust4x4, int **levelscale, int **invlevelscale, int **leveloffset,
                      int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost, int is_calvc);

int quant_4x4_around (int (*tblock)[16], int block_y, int block_x, int qp, 
                      int*  ACLevel, int*  ACRun, 
                      int **fadjust4x4, int **levelscale, int **invlevelscale, int **leveloffset,
                      int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost, int is_calvc);

int quant_4x4_trellis(int (*tblock)[16], int block_y, int block_x, int qp,
                      int*  ACLevel, int*  ACRun, 
                      int **fadjust4x4, int **levelscale, int **invlevelscale, int **leveloffset,
                      int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost, int is_calvc);

int (*quant_4x4)     (int (*tblock)[16], int block_y, int block_x, int qp, 
                      int*  ACLevel, int*  ACRun, 
                      int **fadjust4x4, int **levelscale, int **invlevelscale, int **leveloffset,
                      int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost, int is_calvc);

int quant_dc4x4_normal (int (*tblock)[4], int qp, int* DCLevel, int* DCRun, 
                        int levelscale, int invlevelscale, int leveloffset, const byte (*pos_scan)[2], int is_calvc);

int quant_dc4x4_around (int (*tblock)[4], int qp, int* DCLevel, int* DCRun, 
                        int levelscale, int invlevelscale, int leveloffset, const byte (*pos_scan)[2], int is_calvc);

int quant_dc4x4_trellis(int (*tblock)[4], int qp, int* DCLevel, int* DCRun, 
                        int levelscale, int invlevelscale, int leveloffset, const byte (*pos_scan)[2], int is_calvc);

int (*quant_dc4x4)     (int (*tblock)[4], int qp, int*  DCLevel, int*  DCRun, 
                        int levelscale, int invlevelscale, int leveloffset, const byte (*pos_scan)[2], int is_calvc);

int quant_ac4x4_normal (int (*tblock)[16], int block_y, int block_x, int qp,                 
                        int*  ACLevel, int*  ACRun, 
                        int **fadjust4x4, int **levelscale, int **invlevelscale, int **leveloffset,
                        int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost, int type, int is_calvc);

int quant_ac4x4_around (int (*tblock)[16], int block_y, int block_x, int qp,                 
                        int*  ACLevel, int*  ACRun, 
                        int **fadjust4x4, int **levelscale, int **invlevelscale, int **leveloffset,
                        int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost, int type, int is_calvc);

int quant_ac4x4_trellis(int (*tblock)[16], int block_y, int block_x, int qp,                
                        int*  ACLevel, int*  ACRun, 
                        int **fadjust4x4, int **levelscale, int **invlevelscale, int **leveloffset,
                        int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost, int type, int is_calvc);

int (*quant_ac4x4)     (int (*tblock)[16], int block_y, int block_x, int qp,                 
                        int*  ACLevel, int*  ACRun, 
                        int **fadjust4x4, int **levelscale, int **invlevelscale, int **leveloffset,
                        int *coeff_cost, const byte (*pos_scan)[2], const byte *c_cost, int type, int is_calvc);

void rdoq_4x4_CAVLC    (int (*tblock)[16], int block_y, int block_x, int qp_per, int qp_rem, 
                        int **levelscale, int **leveloffset, const byte (*pos_scan)[2], int levelTrellis[16]);

void rdoq_4x4_CABAC    (int (*tblock)[16], int block_y, int block_x, int qp_per, int qp_rem, 
                        int **levelscale, int **leveloffset, const byte (*pos_scan)[2], int levelTrellis[16]);

void (*rdoq_4x4)       (int (*tblock)[16], int block_y, int block_x, int qp_per, int qp_rem, 
                        int **levelscale, int **leveloffset, const byte (*pos_scan)[2], int levelTrellis[16]);

void rdoq_dc_CAVLC     (int (*tblock)[4], int qp_per, int qp_rem, int levelscale, int leveloffset, 
                        const byte (*pos_scan)[2], int levelTrellis[16], int type);

void rdoq_dc_CABAC     (int (*tblock)[4], int qp_per, int qp_rem, int levelscale, int leveloffset, 
                        const byte (*pos_scan)[2], int levelTrellis[16], int type);

void (*rdoq_dc)        (int (*tblock)[4], int qp_per, int qp_rem, int levelscale, int leveloffset, 
                        const byte (*pos_scan)[2], int levelTrellis[16], int type);

void rdoq_ac4x4_CAVLC  (int (*tblock)[16] , int block_y, int block_x, int qp_per, int qp_rem, 
                        int **levelscale, int **leveloffset, const byte (*pos_scan)[2], int levelTrellis[16], int type);

void rdoq_ac4x4_CABAC  (int (*tblock)[16] , int block_y, int block_x, int qp_per, int qp_rem, 
                        int **levelscale, int **leveloffset, const byte (*pos_scan)[2], int levelTrellis[16], int type);

void (*rdoq_ac4x4)     (int (*tblock)[16] , int block_y, int block_x, int qp_per, int qp_rem, 
                        int **levelscale, int **leveloffset, const byte (*pos_scan)[2], int levelTrellis[16], int type);

#endif

