
/*!
*************************************************************************************
* \file wp_mcprec.c
*
* \brief
*    Improved Motion Compensation Precision Scheme using Weighted Prediction
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*     - Athanasios Leontaris            <aleon@dolby.com>
*     - Alexis Michael Tourapis         <alexismt@ieee.org>
*************************************************************************************
*/
#include "contributors.h"

#include "global.h"
#include "image.h"
#include "slice.h"
#include "wp_mcprec.h"

/*!
************************************************************************
* \brief
*    Initialize WPR object structure
************************************************************************
*/

void wpxInitWPXObject( void )
{
  pWPX = (WPXObject *)malloc( sizeof( WPXObject ) );
  if ( pWPX == NULL )
  {
    fprintf( stderr, "\n Error initializing memory for WPXObject. Exiting...\n" );
    exit(1);
  }

  // wp_ref_l0
  if ((pWPX->wp_ref_list[LIST_0] = (WeightedPredRefX *) calloc(MAX_REFERENCE_PICTURES, sizeof(WeightedPredRefX))) == NULL)
    no_mem_exit("wpxInitWPXObject: pWPX->wp_ref_list[0]");
  // wp_ref_l1
  if ((pWPX->wp_ref_list[LIST_1] = (WeightedPredRefX *) calloc(MAX_REFERENCE_PICTURES, sizeof(WeightedPredRefX))) == NULL)
    no_mem_exit("wpxInitWPXObject: pWPX->wp_ref_list[1]");
}

/*!
************************************************************************
* \brief
*    Free WPR object structure
************************************************************************
*/

void wpxFreeWPXObject( void )
{
  if ( pWPX != NULL )
  {
    free( pWPX );
  }

  // wp_ref_l0
  if (pWPX->wp_ref_list[LIST_0])
    free(pWPX->wp_ref_list[LIST_0]);
  pWPX->wp_ref_list[LIST_0] = NULL;
  // wp_ref_l1
  if (pWPX->wp_ref_list[LIST_1])
    free(pWPX->wp_ref_list[LIST_1]);
  pWPX->wp_ref_list[LIST_1] = NULL;
}

/*!
************************************************************************
* \brief
*    Initialize WPR coding passes
************************************************************************
*/

void wpxInitWPXPasses( InputParameters *params )
{
  // initialize number of wp reference frames in each list
  pWPX->num_wp_ref_list[LIST_0] = 0;
  pWPX->num_wp_ref_list[LIST_1] = 0;

  switch( params->WPMCPrecision )
  {
  default:
  case 0:
    break;
  case 1:
    pWPX->wp_rd_passes[0].algorithm = WP_REGULAR;
    pWPX->wp_rd_passes[1].algorithm = WP_MCPREC_MINUS0;      
    break;
  case 2:
    pWPX->wp_rd_passes[0].algorithm = WP_REGULAR;
    pWPX->wp_rd_passes[1].algorithm = WP_MCPREC_MINUS0;
    pWPX->wp_rd_passes[2].algorithm = WP_MCPREC_MINUS1;      
    break;
  }
}

/*!
************************************************************************
* \brief
*    Modifies ref_pic_list for all lists
************************************************************************
*/

void wpxModifyRefPicList( ImageParameters *img )
{
  unsigned int i, j, cloned_refs;
  static int   default_order_list0[32];
  static int   default_order_list1[32];
  static int   re_order[32], *list_order;  
  int          pred_list;

  StorablePicture **list;

  // set memory
  memset( (void *)default_order_list0, 1<<20, 32 * sizeof( int ) );
  memset( (void *)default_order_list1, 1<<20, 32 * sizeof( int ) );
  memset( (void *)re_order,            1<<20, 32 * sizeof( int ) );

  // First assign default list orders
  list = listX[LIST_0];
  for (i=0; i<(unsigned int)(img->num_ref_idx_l0_active); i++)
  {
    default_order_list0[i] = list[i]->pic_num;
  }
  if ( img->type == B_SLICE )
  {
    list = listX[LIST_1];
    for (i=0; i<(unsigned int)(img->num_ref_idx_l1_active); i++)
    {
      default_order_list1[i] = list[i]->pic_num;
    }
  }

  // obtain the ref_pic_list using POC-based reordering if img->type != B_SLICE
  if ( img->type == P_SLICE )
  {
    int  list_sign[32];
    int  poc_diff[32];
    int  tmp_value;
    int  abs_poc_dist;
    
    for (i=0; i<dpb.ref_frames_in_buffer; i++)
    {
      re_order[i] = dpb.fs_ref[i]->frame->pic_num;
      if (dpb.fs_ref[i]->is_used==3 && (dpb.fs_ref[i]->frame->used_for_reference)&&(!dpb.fs_ref[i]->frame->is_long_term))
      {
        abs_poc_dist = iabs(dpb.fs_ref[i]->frame->poc - enc_picture->poc) ;
        poc_diff[i]  = abs_poc_dist;
        list_sign[i] = (enc_picture->poc < dpb.fs_ref[i]->frame->poc) ? +1 : -1;
      }
    }

    // sort these references based on poc (temporal) distance
    for (i=0; i< dpb.ref_frames_in_buffer-1; i++)
    {
      for (j=i+1; j< dpb.ref_frames_in_buffer; j++)
      {
        if (poc_diff[i]>poc_diff[j] || (poc_diff[i] == poc_diff[j] && list_sign[j] > list_sign[i]))
        {
          tmp_value = poc_diff[i];
          poc_diff[i] = poc_diff[j];
          poc_diff[j] = tmp_value;
          tmp_value  = re_order[i];
          re_order[i] = re_order[j];
          re_order[j] = tmp_value ;
          tmp_value  = list_sign[i];
          list_sign[i] = list_sign[j];
          list_sign[j] = tmp_value ;
        }
      }
    }
  }
  // end of POC-based reordering of P_SLICE

  // loop over two lists
  for ( pred_list = 0; pred_list < (1 + ( img->type == B_SLICE ? 1 : 0 )); pred_list++ )
  {
    if ( pred_list == LIST_0 )
    {
      list_order = (img->type == P_SLICE) ? re_order : default_order_list0;
    }
    else
    {
      list_order = default_order_list1;
    }

    // check algorithms (more flexibility for the future...)
    switch( pWPX->curr_wp_rd_pass->algorithm )
    {
    default:
    case WP_MCPREC_PLUS0:
    case WP_MCPREC_PLUS1:
    case WP_MCPREC_MINUS0:
    case WP_MCPREC_MINUS1:
      // ref 0 and 1 point to to the same reference
      cloned_refs = pWPX->num_wp_ref_list[pred_list] = 2;
      for ( j = 0; j < cloned_refs; j++ )
      {
        pWPX->wp_ref_list[pred_list][j].PicNum = list_order[0];
      }
      // shift the rest
      for ( j = cloned_refs; j < dpb.ref_frames_in_buffer; j++ )
      {
        pWPX->wp_ref_list[pred_list][j].PicNum = list_order[j - (cloned_refs - 1)];
        pWPX->num_wp_ref_list[pred_list]++;
      }
      break;
    case WP_MCPREC_MINUS_PLUS0:
      // ref 0 and 1 point to to the same reference
      cloned_refs = pWPX->num_wp_ref_list[pred_list] = 3;
      for ( j = 0; j < cloned_refs; j++ )
      {
        pWPX->wp_ref_list[pred_list][j].PicNum = list_order[0];
      }
      // shift the rest
      for ( j = cloned_refs; j < dpb.ref_frames_in_buffer; j++ )
      {
        pWPX->wp_ref_list[pred_list][j].PicNum = list_order[j - (cloned_refs - 1)];
        pWPX->num_wp_ref_list[pred_list]++;
      }      
      break;
    }
    // constrain list length
    pWPX->num_wp_ref_list[pred_list] = imin( pWPX->num_wp_ref_list[pred_list],
      ( pred_list == LIST_0 ) ? img->num_ref_idx_l0_active : img->num_ref_idx_l1_active );
  }
}

/*!
************************************************************************
* \brief
*    Determine whether it is fine to determine WP parameters
************************************************************************
*/

int wpxDetermineWP( InputParameters *params, ImageParameters *img, int clist, int n )
{
  int i, j, determine_wp = 0;
  int default_weight[3];
  // we assume it's the same as in the WP functions
  int luma_log_weight_denom   = 5;
  int chroma_log_weight_denom = 5;
  int cur_list = LIST_0;

  default_weight[0] = 1 << luma_log_weight_denom;
  default_weight[1] = 1 << chroma_log_weight_denom;
  default_weight[2] = 1 << chroma_log_weight_denom;

  if ( !params->WPMCPrecision )
  {
    determine_wp = 1;
  }
  else
  {
    determine_wp = 0;
    // check slice type
    if ( img->type == P_SLICE )
    {
      for (i = 0; i < 3; i++)
      {
        wp_weight[clist][n][i] = default_weight[i];
      }      
    }
    else if ( img->type == B_SLICE )
    {
      cur_list = ((clist % 2) ==  LIST_1) ? LIST_0 : LIST_1;

      // single-list prediction
      for (i = 0; i < 3; i++)
      {
        wp_weight[clist][n][i] = default_weight[i];
      }
      // bi-pred
      for (j = 0; j < listXsize[cur_list]; j++)
      {
        for (i = 0; i < 3; i++)
          wbp_weight[clist][n][j][i] = default_weight[i];
      }      
    }
    // algorithm consideration
    switch( pWPX->curr_wp_rd_pass->algorithm )
    {
    case WP_MCPREC_PLUS0:
      for (i = 0; i < 3; i++)
        wp_offset[clist][n][i] = (n == 1) ? 1 : 0;
      break;
    case WP_MCPREC_MINUS0:
      for (i = 0; i < 3; i++)
        wp_offset[clist][n][i] = (n == 1) ? -1 : 0;
      break;
    case WP_MCPREC_PLUS1:
      for (i = 0; i < 3; i++)
        wp_offset[clist][n][i] = (n == 0) ? 1 : 0;
      break;
    case WP_MCPREC_MINUS1:
      for (i = 0; i < 3; i++)
        wp_offset[clist][n][i] = (n == 0) ? -1 : 0;
      break;
    case WP_MCPREC_MINUS_PLUS0:
      for (i = 0; i < 3; i++)
        wp_offset[clist][n][i] = (n == 1) ? -1 : ((n == 2) ? 1 : 0);
      break;
    default:
    case WP_REGULAR:
      determine_wp = 1;
      break;
    }    
    // check list (play with the WP factors)
    if ( img->type == B_SLICE && cur_list == LIST_0 )
    {
      for (i = 0; i < 3; i++)
        wp_offset[clist][n][i] *= 2;
    }
    // zero out chroma offsets
    for (i = 1; i < 3; i++)
    {
      wp_offset[clist][n][i] = 0;
    }
  }

  return determine_wp;
}

/*!
************************************************************************
* \brief
*    Modify number of references
************************************************************************
*/

void wpxAdaptRefNum( ImageParameters *img )
{
  if ( pWPX->curr_wp_rd_pass->algorithm == WP_REGULAR )
  {
    switch( img->type )
    {
    default:
    case I_SLICE:
      break;
    case P_SLICE:
      if ( img->num_ref_idx_l0_active == ( params->P_List0_refs * ((img->structure !=0) + 1) ) )
      {
        listXsize[LIST_0] = img->num_ref_idx_l0_active = imax( ((img->structure !=0) + 1), img->num_ref_idx_l0_active - ((img->structure !=0) + 1) );
      }
      break;
    case B_SLICE:
      if ( img->num_ref_idx_l0_active == ( params->B_List0_refs * ((img->structure !=0) + 1) ) )
      {
        listXsize[LIST_0] = img->num_ref_idx_l0_active = imax( ((img->structure !=0) + 1), img->num_ref_idx_l0_active - ((img->structure !=0) + 1) );
      }
      if ( img->num_ref_idx_l1_active == ( params->B_List1_refs * ((img->structure !=0) + 1) ) )
      {
        listXsize[LIST_1] = img->num_ref_idx_l1_active = imax( ((img->structure !=0) + 1), img->num_ref_idx_l1_active - ((img->structure !=0) + 1) );
      }
      break;
    }    
  }
}

/*!
************************************************************************
* \brief
*    Reorder lists
************************************************************************
*/

void wpxReorderLists( ImageParameters *img, Slice *currSlice )
{
  int i, num_ref;

  wpxModifyRefPicList( img );

  alloc_ref_pic_list_reordering_buffer(currSlice);

  for (i = 0; i < img->num_ref_idx_l0_active + 1; i++)
  {
    currSlice->reordering_of_pic_nums_idc_l0[i] = 3;
    currSlice->abs_diff_pic_num_minus1_l0[i] = 0;
    currSlice->long_term_pic_idx_l0[i] = 0;
  }

  if (img->type == B_SLICE) // type should be part of currSlice not img
  {
    for (i = 0; i < img->num_ref_idx_l1_active + 1; i++)
    {
      currSlice->reordering_of_pic_nums_idc_l1[i] = 3;
      currSlice->abs_diff_pic_num_minus1_l1[i] = 0;
      currSlice->long_term_pic_idx_l1[i] = 0;
    }
  }

  // LIST_0
  num_ref = img->num_ref_idx_l0_active;

  poc_ref_pic_reorder_frame(listX[LIST_0], num_ref,
    currSlice->reordering_of_pic_nums_idc_l0,
    currSlice->abs_diff_pic_num_minus1_l0,
    currSlice->long_term_pic_idx_l0, LIST_0);
  // reference picture reordering
  reorder_ref_pic_list(listX[LIST_0], &listXsize[LIST_0],
    img->num_ref_idx_l0_active - 1,
    currSlice->reordering_of_pic_nums_idc_l0,
    currSlice->abs_diff_pic_num_minus1_l0,
    currSlice->long_term_pic_idx_l0);

  if ( img->type == B_SLICE )
  {
    // LIST_1
    num_ref = img->num_ref_idx_l1_active;

    poc_ref_pic_reorder_frame(listX[LIST_1], num_ref,
      currSlice->reordering_of_pic_nums_idc_l1,
      currSlice->abs_diff_pic_num_minus1_l1,
      currSlice->long_term_pic_idx_l1, LIST_1);
    // reference picture reordering
    reorder_ref_pic_list(listX[LIST_1], &listXsize[LIST_1],
      img->num_ref_idx_l1_active - 1,
      currSlice->reordering_of_pic_nums_idc_l1,
      currSlice->abs_diff_pic_num_minus1_l1,
      currSlice->long_term_pic_idx_l1);
  }
}
