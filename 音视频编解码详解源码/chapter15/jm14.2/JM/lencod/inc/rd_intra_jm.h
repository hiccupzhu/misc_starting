/*!
 ***************************************************************************
 * \file
 *    rd_intra_jm.h
 *
 * \author
 *    Alexis Michael Tourapis
 *
 * \date
 *    2 January 2008
 *
 * \brief
 *    Headerfile for JM rd based intra mode decision
 **************************************************************************
 */

#ifndef _RD_INTRA_JM_H_
#define _RD_INTRA_JM_H_

void Intra16x16_Mode_Decision (Macroblock* currMB, int* i16mode, int is_cavlc);

#endif

