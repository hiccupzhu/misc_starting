
/*!
 ************************************************************************
 * \file conformance.h
 *
 * \brief
 *   Level & Profile Related definitions  
 *
 * \author
 *    Alexis Michael Tourapis         <alexismt@ieee.org>       \n
 *
 ************************************************************************
 */

#ifndef _CONFORMANCE_H_
#define _CONFORMANCE_H_

// Vertical MV Limits (integer/halfpel/quarterpel)
// Currently only Integer Pel restrictions are used,
// since the way values are specified
// (i.e. mvlowbound = (levelmvlowbound + 1) and the way
// Subpel ME is performed, subpel will always be within range.

extern const int LEVELMVLIMIT[17][6];

void ProfileCheck(void);
void LevelCheck(void);

void update_mv_limits(ImageParameters *img, byte is_field);
void clip_mv_range(ImageParameters *img, int search_range, short mv[2], int res);
int out_of_bounds_mvs(ImageParameters *img, short mv[2], int res);

#endif

