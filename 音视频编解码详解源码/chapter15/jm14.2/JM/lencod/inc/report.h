
/*!
 ************************************************************************
 * \file report.h
 *
 * \brief
 *    headers for frame format related information
 *
 * \author
 *
 ************************************************************************
 */
#ifndef _REPORT_H_
#define _REPORT_H_
#include "contributors.h"
#include "global.h"
#include "enc_statistics.h"

void report ( ImageParameters *img, InputParameters *params, StatParameters *stats);
void information_init ( ImageParameters *img, InputParameters *params, StatParameters *stats);
void report_frame_statistic(void);
void report_stats_on_error (void);

#endif

