
/*!
 *************************************************************************************
 * \file annexb.h
 *
 * \brief
 *    Annex B byte stream buffer handling.
 *
 *************************************************************************************
 */

#ifndef _ANNEXB_H_
#define _ANNEXB_H_

#include "nalucommon.h"

extern int IsFirstByteStreamNALU;
extern int LastAccessUnitExists;
extern int NALUCount;

int  GetAnnexbNALU (FILE *bitstream, NALU_t *nalu);
void OpenBitstreamFile (char *fn);
void CloseBitstreamFile(void);
void CheckZeroByteNonVCL(NALU_t *nalu);
void CheckZeroByteVCL(NALU_t *nalu);

#endif

