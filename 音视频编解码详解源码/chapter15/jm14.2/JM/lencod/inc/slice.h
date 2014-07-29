/*!
 ***************************************************************************
 * \file
 *    slice.h
 *
 * \date
 *    16 July 2008
 *
 * \brief
 *    Headerfile for slice-related functions
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Athanasios Leontaris            <aleon@dolby.com>
 *     - Karsten Sühring                 <suehring@hhi.de> 
 *     - Alexis Michael Tourapis         <alexismt@ieee.org> 

 **************************************************************************
 */

#ifndef _SLICE_H_
#define _SLICE_H_

#include "global.h"
#include "mbuffer.h"
#include "rdopt_coding_state.h"

extern CSptr cs_mb, cs_b8, cs_cm, cs_ib8, cs_ib4;

pic_parameter_set_rbsp_t *PicParSet[MAXPPS];

void poc_ref_pic_reorder_frame( StorablePicture **list, unsigned num_ref_idx_lX_active, int *reordering_of_pic_nums_idc, int *abs_diff_pic_num_minus1, int *long_term_pic_idx, int list_no );
void poc_ref_pic_reorder_field( StorablePicture **list, unsigned num_ref_idx_lX_active, int *reordering_of_pic_nums_idc, int *abs_diff_pic_num_minus1, int *long_term_pic_idx, int list_no );

void init_ref_pic_list_reordering( Slice *currSlice );
int  encode_one_slice ( int SliceGroupId, Picture *pic, int TotalCodedMBs );
void init_slice       ( Slice **currSlice, int start_mb_addr );
void free_slice_list  ( Picture *currPic );

void SetLambda( int j, int qp, double lambda_scale );
void SetLagrangianMultipliersOn( void );
void SetLagrangianMultipliersOff( void );

#endif
