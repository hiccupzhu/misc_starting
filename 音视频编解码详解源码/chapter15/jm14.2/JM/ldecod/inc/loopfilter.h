/*!
 ************************************************************************
 *  \file
 *     loopfilter.h
 *  \brief
 *     external deblocking filter interface
 ************************************************************************
 */

#ifndef _LOOPFILTER_H_
#define _LOOPFILTER_H_

#include "global.h"
#include "mbuffer.h"

void DeblockPicture(ImageParameters *img, StorablePicture *p) ;

#endif //_LOOPFILTER_H_
