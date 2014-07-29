
/*!
 ***************************************************************************
 * \file
 *    q_offsets.h
 *
 * \brief
 *    Headerfile for q_offsets array
 *
 * \date
 *    18. Nov 2004
 ***************************************************************************
 */

#ifndef _Q_OFFSETS_H_
#define _Q_OFFSETS_H_

extern int *****LevelOffset4x4Comp;
extern int *****LevelOffset8x8Comp;
extern short ***OffsetList4x4;
extern short ***OffsetList8x8;

extern const int OffsetBits;

extern int AdaptRndWeight;
extern int AdaptRndCrWeight;

void Init_QOffsetMatrix (void);
void CalculateOffsetParam(void);
void CalculateOffset8Param(void);
void free_QOffsets (void);
#endif
