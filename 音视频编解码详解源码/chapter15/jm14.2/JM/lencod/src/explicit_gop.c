/*!
 *************************************************************************************
 * \file explicit_gop.c
 *
 * \brief
 *    Code for explicit gop support and hierarchical coding.
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Alexis Michael Tourapis                     <alexismt@ieee.org>
 *************************************************************************************
 */

#include "contributors.h"

#include <ctype.h>
#include <limits.h>
#include "global.h"

#include "explicit_gop.h"
#include "image.h"
#include "nalucommon.h"
#include "report.h"

/*!
************************************************************************
* \brief
*    Generation of hierarchical GOP
************************************************************************
*/
void create_hierarchy()
{
  int i, j;
  int centerB=params->successive_Bframe/2;
  GOP_DATA tmp;

  if (params->HierarchicalCoding == 1)
  {
    for (i=0;i<params->successive_Bframe;i++)
    {
      if (i < centerB)
      {
        gop_structure[i].slice_type = B_SLICE;
        gop_structure[i].display_no = i * 2 + 1;
        gop_structure[i].hierarchy_layer = 1;
        gop_structure[i].reference_idc = NALU_PRIORITY_LOW;
        gop_structure[i].slice_qp = imax(0, (params->qpB + (params->HierarchyLevelQPEnable ? -1: params->qpBRSOffset)));

      }
      else
      {
        gop_structure[i].slice_type = B_SLICE;
        gop_structure[i].display_no = (i - centerB) * 2;
        gop_structure[i].hierarchy_layer = 0;
        gop_structure[i].reference_idc = NALU_PRIORITY_DISPOSABLE;
        gop_structure[i].slice_qp = params->qpB;
      }
    }
    img->GopLevels = 2;
  }
  else
  {
    int GOPlevels = 1;
    int Bframes = params->successive_Bframe;
    int *curGOPLevelfrm,*curGOPLeveldist ;
    int curlevel = GOPlevels ;
    int i;

    while (((Bframes + 1 ) >> GOPlevels) > 1)
    {
      GOPlevels ++;
    }

    curlevel = GOPlevels;
    img->GopLevels = GOPlevels;
    if (NULL == (curGOPLevelfrm = (int*)malloc(GOPlevels * sizeof(int)))) no_mem_exit("create_hierarchy:curGOPLevelfrm");
    if (NULL == (curGOPLeveldist= (int*)malloc(GOPlevels * sizeof(int)))) no_mem_exit("create_hierarchy:curGOPLeveldist");

    for (i=0; i <params->successive_Bframe; i++)
    {
      gop_structure[i].display_no = i;
      gop_structure[i].slice_type = B_SLICE;
      gop_structure[i].hierarchy_layer = 0;
      gop_structure[i].reference_idc = NALU_PRIORITY_DISPOSABLE;
      gop_structure[i].slice_qp = params->qpB;
    }

    for (j = 1; j < GOPlevels; j++)
    {
      for (i = (1 << j) - 1; i < Bframes + 1 - (1 << j); i += (1 << j)) 
      {
        gop_structure[i].hierarchy_layer  = j;
        gop_structure[i].reference_idc  = NALU_PRIORITY_LOW;
        gop_structure[i].slice_qp = imax(0, params->qpB + (params->HierarchyLevelQPEnable ? -j: params->qpBRSOffset));
        //KHHan, for inter lossless code(referenced B picture)
        //if(!params->lossless_qpprime_y_zero_flag)
        //  gop_structure[i].slice_qp = imax(0, params->qpB + (params->HierarchyLevelQPEnable ? -j: params->qpBRSOffset));
        //else
        //  gop_structure[i].slice_qp = params->qpB;
      }
    }

    for (i = 1; i < Bframes; i++)
    {
      j = i;

      while (j > 0 && gop_structure[j].hierarchy_layer > gop_structure[j-1].hierarchy_layer)
      {
        tmp = gop_structure[j-1];
        gop_structure[j-1] = gop_structure[j];
        gop_structure[j] = tmp;
        j--;
      }
    }
  }
}


/*!
************************************************************************
* \brief
*    Initialization of GOP structure.
*
************************************************************************
*/
void init_gop_structure()
{
  int max_gopsize = params->HierarchicalCoding != 3 ? params->successive_Bframe  : params->jumpd;

  gop_structure = calloc(imax(10,max_gopsize), sizeof (GOP_DATA)); // +1 for reordering
  if (NULL==gop_structure)
    no_mem_exit("init_gop_structure: gop_structure");
}


/*!
************************************************************************
* \brief
*    Clear GOP structure
************************************************************************
*/
void clear_gop_structure()
{
  if (gop_structure)
    free(gop_structure);
}


/*!
************************************************************************
* \brief
*    Interpret GOP struct from input parameters
************************************************************************
*/
void interpret_gop_structure()
{

  int nLength = strlen(params->ExplicitHierarchyFormat);
  int i =0, k, dqp, display_no;
  int slice_read =0, order_read = 0, stored_read = 0, qp_read =0;
  int coded_frame = 0;

  if (nLength > 0)
  {

    for (i = 0; i < nLength ; i++)
    {
      //! First lets read slice type
      if (slice_read == 0)
      {
        switch (params->ExplicitHierarchyFormat[i])
        {
        case 'P':
        case 'p':
          gop_structure[coded_frame].slice_type=P_SLICE;
          break;
        case 'B':
        case 'b':
          gop_structure[coded_frame].slice_type=B_SLICE;
          break;
        case 'I':
        case 'i':
          gop_structure[coded_frame].slice_type=I_SLICE;
          break;
        default:
          snprintf(errortext, ET_SIZE, "Slice Type invalid in ExplicitHierarchyFormat param. Please check configuration file.");
          error (errortext, 400);
          break;
        }
        slice_read = 1;
      }
      else
      {
        //! Next is Display Order
        if (order_read == 0)
        {
          if (isdigit((int)(*(params->ExplicitHierarchyFormat+i))))
          {
            sscanf(params->ExplicitHierarchyFormat+i,"%d",&display_no);
            gop_structure[coded_frame].display_no = display_no;
            order_read = 1;
            if (display_no<0 || display_no>=params->jumpd)
            {
              snprintf(errortext, ET_SIZE, "Invalid Frame Order value. Frame position needs to be in [0,%d] range.",params->jumpd-1);
              error (errortext, 400);
            }
            for (k=0;k<coded_frame;k++)
            {
              if (gop_structure[k].display_no == display_no)
              {
                snprintf(errortext, ET_SIZE, "Frame Order value %d in frame %d already used for enhancement frame %d.",display_no,coded_frame,k);
                error (errortext, 400);
              }
            }
          }
          else
          {
            snprintf(errortext, ET_SIZE, "Slice Type needs to be followed by Display Order. Please check configuration file.");
            error (errortext, 400);
          }
        }
        else if (order_read == 1)
        {
          if (stored_read == 0 && !(isdigit((int)(*(params->ExplicitHierarchyFormat+i)))))
          {
            switch (params->ExplicitHierarchyFormat[i])
            {
            case 'E':
            case 'e':
              gop_structure[coded_frame].reference_idc = NALU_PRIORITY_DISPOSABLE;
              gop_structure[coded_frame].hierarchy_layer = 0;
              break;
            case 'R':
            case 'r':
              gop_structure[coded_frame].reference_idc= NALU_PRIORITY_LOW;
              gop_structure[coded_frame].hierarchy_layer = 1;
              img->GopLevels = 2;
              break;
            default:
              snprintf(errortext, ET_SIZE, "Reference_IDC invalid in ExplicitHierarchyFormat param. Please check configuration file.");
              error (errortext, 400);
              break;
            }
            stored_read = 1;
          }
          else if (stored_read == 1 && qp_read == 0)
          {
            if (isdigit((int)(*(params->ExplicitHierarchyFormat+i))))
            {
              sscanf(params->ExplicitHierarchyFormat+i,"%d",&dqp);

              if (gop_structure[coded_frame].slice_type == I_SLICE)
                gop_structure[coded_frame].slice_qp = params->qp0;
              else if (gop_structure[coded_frame].slice_type == P_SLICE)
                gop_structure[coded_frame].slice_qp = params->qpN;
              else
                gop_structure[coded_frame].slice_qp = params->qpB;

              gop_structure[coded_frame].slice_qp = iClip3(-img->bitdepth_luma_qp_scale, 51,gop_structure[coded_frame].slice_qp + dqp);
                qp_read = 1;
            }
            else
            {
              snprintf(errortext, ET_SIZE, "Reference_IDC needs to be followed by QP. Please check configuration file.");
              error (errortext, 400);
            }
          }
          else if (stored_read == 1 && qp_read == 1 && !(isdigit((int)(*(params->ExplicitHierarchyFormat+i)))) && (i < nLength - 2))
          {
            stored_read =0;
            qp_read=0;
            order_read=0;
            slice_read=0;
            i--;
            coded_frame ++;
            if (coded_frame >= params->jumpd )
            {
              snprintf(errortext, ET_SIZE, "Total number of frames in Enhancement GOP need to be fewer or equal to FrameSkip parameter.");
              error (errortext, 400);
            }
          }
        }
      }
    }
  }
  else
  {
    snprintf(errortext, ET_SIZE, "ExplicitHierarchyFormat is empty. Please check configuration file.");
    error (errortext, 400);
  }

  params->successive_Bframe = coded_frame + 1;
}


/*!
************************************************************************
* \brief
*    Encode Enhancement Layer.
************************************************************************
*/
void encode_enhancement_layer()
{
  //int previous_ref_idc = 1;

  if ((params->successive_Bframe != 0) && (IMG_NUMBER > 0) && (params->EnableIDRGOP == 0 || img->idr_gop_number)) // B-frame(s) to encode
  {
    set_slice_type( ( params->PReplaceBSlice ) ? P_SLICE : B_SLICE); // set slice type
    img->layer = (params->NumFramesInELSubSeq == 0) ? 0 : 1;
    img->nal_reference_idc = NALU_PRIORITY_DISPOSABLE;

    if (params->HierarchicalCoding)
    {
      for(img->b_frame_to_code = 1; img->b_frame_to_code <= params->successive_Bframe; img->b_frame_to_code++)
      {
        img->nal_reference_idc = NALU_PRIORITY_DISPOSABLE;
        set_slice_type( gop_structure[img->b_frame_to_code - 1].slice_type);

        if (img->last_ref_idc == 1)
        {
          img->frame_num++;                 //increment frame_num for each stored B slice
          img->frame_num %= max_frame_num;
        }
        
        img->nal_reference_idc = gop_structure[img->b_frame_to_code - 1].reference_idc;
        img->b_interval = ((double) (params->jumpd + 1) / (params->successive_Bframe + 1.0) );

        if (params->HierarchicalCoding == 3)
          img->b_interval = 1.0;

        if (IMG_NUMBER && (IMG_NUMBER <= params->intra_delay))
        {
          if(params->idr_period && ((!params->adaptive_idr_period && ( img->frm_number - img->lastIDRnumber ) % params->idr_period == 0)
            || (params->adaptive_idr_period == 1 && ( img->frm_number - imax(img->lastIntraNumber, img->lastIDRnumber) ) % params->idr_period == 0)) )
            img->toppoc = (int)(img->b_interval * (double)(1 + gop_structure[img->b_frame_to_code - 1].display_no));
          else
            img->toppoc = 2*((-img->frm_number + img->lastIDRnumber)*(params->jumpd + 1) 
            + (int)(img->b_interval * (double)(1 + gop_structure[img->b_frame_to_code -1].display_no)));

          if (img->b_frame_to_code == 1)
            img->delta_pic_order_cnt[0] = img->toppoc - 2*(start_tr_in_this_IGOP  + (params->intra_delay - IMG_NUMBER)*((params->jumpd + 1)));
          else
            img->delta_pic_order_cnt[0] = img->toppoc - 2*(start_tr_in_this_IGOP  + (params->intra_delay - IMG_NUMBER)*((params->jumpd + 1)) 
            + (int) (2.0 *img->b_interval * (double) (1 + gop_structure[img->b_frame_to_code - 2].display_no)));
        }
        else
        {
          if(params->idr_period && !params->adaptive_idr_period)
            img->toppoc = 2*((( ( img->frm_number - img->lastIDRnumber - params->intra_delay) % params->idr_period ) - 1)*(params->jumpd + 1) 
            + (int)(img->b_interval * (double)(1 + gop_structure[img->b_frame_to_code - 1].display_no)));
          else if(params->idr_period && params->adaptive_idr_period == 1)
            img->toppoc = 2*((( img->frm_number - img->lastIDRnumber - params->intra_delay ) - 1)*(params->jumpd + 1) 
            + (int)(img->b_interval * (double)(1 + gop_structure[img->b_frame_to_code - 1].display_no)));
          else
            img->toppoc = 2*((img->frm_number - img->lastIDRnumber - 1 - params->intra_delay)*(params->jumpd + 1) 
            + (int)(img->b_interval * (double)(1 + gop_structure[img->b_frame_to_code - 1].display_no)));

          if (img->b_frame_to_code == 1)
            img->delta_pic_order_cnt[0] = img->toppoc - 2*(start_tr_in_this_IGOP  + (img->frm_number - img->lastIDRnumber)*((params->jumpd + 1)));
          else
            img->delta_pic_order_cnt[0] = img->toppoc - 2*(start_tr_in_this_IGOP  + (img->frm_number - img->lastIDRnumber - 1)*((params->jumpd + 1)) 
            + (int) (2.0 *img->b_interval * (double) (1+ gop_structure[img->b_frame_to_code - 2].display_no)));
        }

        img->bottompoc = ((params->PicInterlace==FRAME_CODING)&&(params->MbInterlace==FRAME_CODING)) ? img->toppoc : img->toppoc + 1;
        img->framepoc = imin (img->toppoc, img->bottompoc);

        img->delta_pic_order_cnt[1]= 0;   
        FrameNumberInFile = CalculateFrameNumber();

        encode_one_frame();  // encode one frame
        
        img->last_ref_idc = img->nal_reference_idc ? 1 : 0;

        if (params->ReportFrameStats)
          report_frame_statistic();
      }
      img->b_frame_to_code = 0;
    }
    else
    {
      for(img->b_frame_to_code = 1; img->b_frame_to_code <= params->successive_Bframe; img->b_frame_to_code++)
      {
        img->nal_reference_idc = (params->BRefPictures == 1 ) ? NALU_PRIORITY_LOW : NALU_PRIORITY_DISPOSABLE;
        
        if (img->last_ref_idc)
        {
          img->frame_num++;                 //increment frame_num once for B-frames
          img->frame_num %= max_frame_num;
        }

        img->b_interval =
          ((double) (params->jumpd + 1) / (params->successive_Bframe + 1.0) );

        if (params->HierarchicalCoding == 3)
          img->b_interval = 1.0;

        if (IMG_NUMBER && (IMG_NUMBER <= params->intra_delay))
        { 
          if(params->idr_period && ((!params->adaptive_idr_period && ( img->frm_number - img->lastIDRnumber ) % params->idr_period == 0)
            || (params->adaptive_idr_period == 1 && ( img->frm_number - imax(img->lastIntraNumber, img->lastIDRnumber) ) % params->idr_period == 0)) )
            img->toppoc = (int) (img->b_interval * (double)img->b_frame_to_code);
          else
            img->toppoc = 2*((-img->frm_number + img->lastIDRnumber)*(params->jumpd + 1) 
            + (int) (img->b_interval * (double)img->b_frame_to_code));
        }
        else
        {
          if(params->idr_period && !params->adaptive_idr_period)
            img->toppoc = 2*((( ( img->frm_number - img->lastIDRnumber - params->intra_delay) % params->idr_period )-1)*(params->jumpd+1) + (int) (img->b_interval * (double)img->b_frame_to_code));
          else if(params->idr_period && params->adaptive_idr_period == 1)
            img->toppoc = 2*((( img->frm_number - img->lastIDRnumber - params->intra_delay )-1)*(params->jumpd+1) + (int) (img->b_interval * (double)img->b_frame_to_code));
          else
            img->toppoc = 2*((img->frm_number - img->lastIDRnumber - 1 - params->intra_delay)*(params->jumpd+1) + (int) (img->b_interval * (double)img->b_frame_to_code));
        }

        if ((params->PicInterlace==FRAME_CODING)&&(params->MbInterlace==FRAME_CODING))
          img->bottompoc = img->toppoc;     //progressive
        else
          img->bottompoc = img->toppoc+1;

        img->framepoc = imin (img->toppoc, img->bottompoc);

        //the following is sent in the slice header
        if (params->BRefPictures != 1)
        {
          img->delta_pic_order_cnt[0]= 2 * (img->b_frame_to_code - 1);
        }
        else
        {
          img->delta_pic_order_cnt[0]= -2;
        }

        img->delta_pic_order_cnt[1]= 0;   // POC200301
        FrameNumberInFile = CalculateFrameNumber();

        encode_one_frame();  // encode one B-frame
        
        img->last_ref_idc = img->nal_reference_idc ? 1 : 0;

        if (params->ReportFrameStats)
          report_frame_statistic();
      }
    }
  }
  img->b_frame_to_code = 0;
}

/*!
************************************************************************
* \brief
*    POC-based reference management (FRAME)
************************************************************************
*/

void poc_based_ref_management_frame_pic(int current_pic_num)
{
  unsigned i, pic_num = 0;

  int min_poc=INT_MAX;
  DecRefPicMarking_t *tmp_drpm,*tmp_drpm2;

  if (img->dec_ref_pic_marking_buffer!=NULL)
    return;

  if ((dpb.ref_frames_in_buffer + dpb.ltref_frames_in_buffer)==0)
    return;

  for (i=0; i<dpb.used_size;i++)
  {
    if (dpb.fs[i]->is_reference  && (!(dpb.fs[i]->is_long_term)) && dpb.fs[i]->poc < min_poc)
    {
      min_poc = dpb.fs[i]->frame->poc ;
      pic_num =  dpb.fs[i]->frame->pic_num;
    }
  }

  if (NULL==(tmp_drpm=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) no_mem_exit("poc_based_ref_management: tmp_drpm");
  tmp_drpm->Next=NULL;

  tmp_drpm->memory_management_control_operation = 0;

  if (NULL==(tmp_drpm2=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) no_mem_exit("poc_based_ref_management: tmp_drpm2");
  tmp_drpm2->Next=tmp_drpm;

  tmp_drpm2->memory_management_control_operation = 1;
  tmp_drpm2->difference_of_pic_nums_minus1 = current_pic_num - pic_num - 1;
  img->dec_ref_pic_marking_buffer = tmp_drpm2;

}

/*!
************************************************************************
* \brief
*    POC-based reference management (FIELD)
************************************************************************
*/

void poc_based_ref_management_field_pic(int current_pic_num)
{
  unsigned int i, pic_num1 = 0, pic_num2 = 0;

  int min_poc=INT_MAX;
  DecRefPicMarking_t *tmp_drpm,*tmp_drpm2, *tmp_drpm3;

  if (img->dec_ref_pic_marking_buffer!=NULL)
    return;

  if ((dpb.ref_frames_in_buffer+dpb.ltref_frames_in_buffer)==0)
    return;

  if ( img->structure == TOP_FIELD )
  {
    for (i=0; i<dpb.used_size;i++)
    {
      if (dpb.fs[i]->is_reference && (!(dpb.fs[i]->is_long_term)) && dpb.fs[i]->poc < min_poc)
      {      
        min_poc  = dpb.fs[i]->poc;
        pic_num1 = dpb.fs[i]->top_field->pic_num;
        pic_num2 = dpb.fs[i]->bottom_field->pic_num;
      }
    }
  }

  if (NULL==(tmp_drpm=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) no_mem_exit("poc_based_ref_management_field_pic: tmp_drpm");
  tmp_drpm->Next=NULL;
  tmp_drpm->memory_management_control_operation = 0;

  if ( img->structure == BOTTOM_FIELD )
  {
    img->dec_ref_pic_marking_buffer = tmp_drpm;
    return;
  }

  if (NULL==(tmp_drpm2=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) no_mem_exit("poc_based_ref_management_field_pic: tmp_drpm2");
  tmp_drpm2->Next=tmp_drpm;
  tmp_drpm2->memory_management_control_operation = 1;
  tmp_drpm2->difference_of_pic_nums_minus1 = current_pic_num - pic_num1 - 1;

  if (NULL==(tmp_drpm3=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t)))) no_mem_exit("poc_based_ref_management_field_pic: tmp_drpm3");
  tmp_drpm3->Next=tmp_drpm2;
  tmp_drpm3->memory_management_control_operation = 1;
  tmp_drpm3->difference_of_pic_nums_minus1 = current_pic_num - pic_num2 - 1;

  img->dec_ref_pic_marking_buffer = tmp_drpm3;
}

