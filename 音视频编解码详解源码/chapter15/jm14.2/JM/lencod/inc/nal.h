
/*!
 **************************************************************************************
 * \file
 *    nal.h
 * \brief
 *    NAL related headers
 *
 * \date 4 Agust 2008
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
  *      - Karsten Sühring                 <suehring@hhi.de>
  *      - Alexis Michael Tourapis         <alexismt@ieee.org> ***************************************************************************************
 */


#ifndef _NAL_H_
#define _NAL_H_

#include "nalucommon.h"
#include "enc_statistics.h"

int  addCabacZeroWords(NALU_t *nalu, StatParameters *cur_stats);
void SODBtoRBSP (Bitstream *currStream);
int  RBSPtoEBSP(byte *NaluBuffer, unsigned char *rbsp, int rbsp_size);

#endif
