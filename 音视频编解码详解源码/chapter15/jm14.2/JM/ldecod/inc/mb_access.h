
/*!
 *************************************************************************************
 * \file mb_access.h
 *
 * \brief
 *    Functions for macroblock neighborhoods
 *
 * \author
 *     Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Karsten Sühring          <suehring@hhi.de>
 *************************************************************************************
 */

#ifndef _MB_ACCESS_H_
#define _MB_ACCESS_H_

void CheckAvailabilityOfNeighbors(Macroblock *currMB);

void (*getNeighbour)   (Macroblock *currMB, int xN, int yN, int mb_size[2], PixelPos *pix);
void getAffNeighbour   (Macroblock *currMB, int xN, int yN, int mb_size[2], PixelPos *pix);
void getNonAffNeighbour(Macroblock *currMB, int xN, int yN, int mb_size[2], PixelPos *pix);
void get4x4Neighbour   (Macroblock *currMB, int xN, int yN, int mb_size[2], PixelPos *pix);

int  mb_is_available(int mbAddr, Macroblock *currMB);
void get_mb_pos (int mb_addr, int mb_size[2], int *x, int*y);
void (*get_mb_block_pos) (int mb_addr, int *x, int*y);
void get_mb_block_pos_normal (int mb_addr, int *x, int*y);
void get_mb_block_pos_mbaff (int mb_addr, int *x, int*y);



#endif
