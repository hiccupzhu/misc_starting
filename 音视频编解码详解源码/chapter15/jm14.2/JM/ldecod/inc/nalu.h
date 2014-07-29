
/*!
 **************************************************************************************
 * \file
 *    parset.h
 * \brief
 *    Picture and Sequence Parameter Sets, encoder operations
 *
 * \date 25 November 2002
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Stephan Wenger        <stewe@cs.tu-berlin.de>
 ***************************************************************************************
 */


#ifndef _NALU_H_
#define _NALU_H_

#include "nalucommon.h"

extern FILE *bits;

//int GetAnnexbNALU (NALU_t *nalu);
int read_next_nalu(FILE *bitstream, NALU_t *nalu);
int NALUtoRBSP (NALU_t *nalu);

#endif
