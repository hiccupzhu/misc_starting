
/*!
 *************************************************************************************
 * \file mb_access.h
 *
 * \brief
 *    Functions for macroblock neighborhoods
 *
 * \author
 *     Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Karsten Sühring                 <suehring@hhi.de> 
 *     - Alexis Michael Tourapis         <alexismt@ieee.org>  
 *************************************************************************************
 */

#ifndef _MB_ACCESS_H_
#define _MB_ACCESS_H_

void CheckAvailabilityOfNeighbors(Macroblock *currMB);

void (*getNeighbour)         (Macroblock *currMb, int xN, int yN, int mb_size[2], PixelPos *pix);
void getAffNeighbour         (Macroblock *currMb, int xN, int yN, int mb_size[2], PixelPos *pix);
void getNonAffNeighbour      (Macroblock *currMb, int xN, int yN, int mb_size[2], PixelPos *pix);
void get4x4Neighbour         (Macroblock *currMb, int xN, int yN, int mb_size[2], PixelPos *pix);
int  mb_is_available         (int mbAddr, Macroblock *currMb);
void get_mb_pos              (int mb_addr, int mb_size[2], int *x, int*y);
void (*get_mb_block_pos)     (int mb_addr, int *x, int*y);
void get_mb_block_pos_normal (int mb_addr, int *x, int*y);
void get_mb_block_pos_mbaff  (int mb_addr, int *x, int*y);


#endif
