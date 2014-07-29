
/*!
 ************************************************************************
 * \file
 *    mc_prediction.h
 *
 * \brief
 *    motion compensation header
 *
 * \author
 *    Alexis Michael Tourapis         <alexismt@ieee.org>                 \n
 *
 ************************************************************************/

#ifndef _MC_PREDICTION_H_
#define _MC_PREDICTION_H_
#include "mbuffer.h"

void LumaPrediction      ( Macroblock* currMB, int, int, int, int, int, int, int, short, short, short );
void LumaPredictionBi    ( Macroblock* currMB, int, int, int, int, int, int, short, short, int );
void ChromaPrediction    ( Macroblock* currMB, int, int, int, int, int, int, int, int, short, short, short );
void ChromaPrediction4x4 ( Macroblock* currMB, int, int, int, int, int, int, short, short, short);   

// function pointer for different ways of obtaining chroma interpolation
void (*OneComponentChromaPrediction4x4)         (imgpel* , int , int , short*** , StorablePicture *listX, int );
void OneComponentChromaPrediction4x4_regenerate (imgpel* , int , int , short*** , StorablePicture *listX, int );
void OneComponentChromaPrediction4x4_retrieve   (imgpel* , int , int , short*** , StorablePicture *listX, int );

void IntraChromaPrediction ( Macroblock *currMB, int*, int*, int*);
void IntraChromaRDDecision ( Macroblock *currMB, RD_PARAMS);

void ComputeResidue    (imgpel **curImg, imgpel mb_pred[MB_BLOCK_SIZE][MB_BLOCK_SIZE], int img_m7[MB_BLOCK_SIZE][MB_BLOCK_SIZE], int mb_y, int mb_x, int opix_y, int opix_x, int width, int height);
void SampleReconstruct (imgpel **curImg, imgpel mb_pred[MB_BLOCK_SIZE][MB_BLOCK_SIZE], int img_m7[MB_BLOCK_SIZE][MB_BLOCK_SIZE], int mb_y, int mb_x, int opix_y, int opix_x, int width, int height, int max_imgpel_value, int dq_bits);
#endif

