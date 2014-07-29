
/*!
 ***************************************************************************
 *
 * \file transform.h
 *
 * \brief
*    prototypes of transform functions
  *
 * \date
 *    10 July 2007
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    Alexis Michael Tourapis
 **************************************************************************/

#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

void forward4x4   (int (*block) [16], int (*tblock)[16], int pos_y, int pos_x);
void inverse4x4   (int (*tblock)[16], int (*block )[16], int pos_y, int pos_x);
void forward8x8   (int (*block) [16], int (*tblock)[16], int pos_y, int pos_x);
void inverse8x8   (int (*tblock)[16], int (*block )[16], int pos_y, int pos_x);
void hadamard4x4  (int (*block) [ 4], int (*tblock)[ 4]);
void ihadamard4x4 (int (*tblock)[ 4], int (*block) [ 4]);
void hadamard4x2  (int (*block) [ 4], int (*tblock)[ 4]);
void ihadamard4x2 (int (*tblock)[ 4], int (*block) [ 4]);
void hadamard2x2  (int (*block)[16], int tblock[4]);
void ihadamard2x2 (int block[4], int tblock[4]);

#endif //_TRANSFORM_H_
