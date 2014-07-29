
/*!
 *************************************************************************************
 * \file image.c
 *
 * \brief
 *    Code one image/slice
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Inge Lille-Langoy               <inge.lille-langoy@telenor.com>
 *     - Rickard Sjoberg                 <rickard.sjoberg@era.ericsson.se>
 *     - Jani Lainema                    <jani.lainema@nokia.com>
 *     - Sebastian Purreiter             <sebastian.purreiter@mch.siemens.de>
 *     - Byeong-Moon Jeon                <jeonbm@lge.com>
 *     - Yoon-Seong Soh                  <yunsung@lge.com>
 *     - Thomas Stockhammer              <stockhammer@ei.tum.de>
 *     - Detlev Marpe                    <marpe@hhi.de>
 *     - Guido Heising
 *     - Thomas Wedi                     <wedi@tnt.uni-hannover.de>
 *     - Ragip Kurceren                  <ragip.kurceren@nokia.com>
 *     - Antti Hallapuro                 <antti.hallapuro@nokia.com>
 *     - Alexis Michael Tourapis         <alexismt@ieee.org>
 *     - Athanasios Leontaris            <aleon@dolby.com>
 *************************************************************************************
 */
#include "contributors.h"

#include <math.h>
#include <time.h>
#include <sys/timeb.h>

#include "global.h"

#include "filehandle.h"
#include "mbuffer.h"
#include "img_luma.h"
#include "img_chroma.h"
#include "img_distortion.h"
#include "intrarefresh.h"
#include "slice.h"
#include "fmo.h"
#include "sei.h"
#include "memalloc.h"
#include "nalu.h"
#include "ratectl.h"
#include "mb_access.h"
#include "cabac.h"
#include "context_ini.h"
#include "enc_statistics.h"
#include "conformance.h"

#include "q_matrix.h"
#include "q_offsets.h"
#include "wp.h"
#include "input.h"
#include "image.h"
#include "errdo.h"


extern pic_parameter_set_rbsp_t *PicParSet[MAXPPS];

extern void DeblockFrame(ImageParameters *img, imgpel **, imgpel ***);

static void code_a_picture(Picture *pic);
static void field_picture(Picture *top, Picture *bottom);
static void prepare_enc_frame_picture (StorablePicture **stored_pic);
static void writeout_picture(Picture *pic);
static byte picture_structure_decision(Picture *frame, Picture *top, Picture *bot);
static void distortion_fld (Picture *field_pic);
static void field_mode_buffer (ImageParameters *img, InputParameters *params);
static void frame_mode_buffer (ImageParameters *img, InputParameters *params);
static void init_frame(ImageParameters *img);
static void init_field(ImageParameters *img);
static void put_buffer_frame(ImageParameters *img);
static void put_buffer_top(ImageParameters *img);
static void put_buffer_bot(ImageParameters *img);
static void PaddAutoCropBorders (FrameFormat output, int img_size_x, int img_size_y, int img_size_x_cr, int img_size_y_cr);
static void rdPictureCoding(void);

#ifdef _ADAPT_LAST_GROUP_
int *last_P_no;
int *last_P_no_frm;
int *last_P_no_fld;
#endif
int FrameNumberInFile;

static void ReportFirstframe(time_t tmp_time, time_t me_time);
static void ReportIntra(time_t tmp_time, time_t me_time);
static void ReportSP(time_t tmp_time, time_t me_time);
static void ReportP(time_t tmp_time, time_t me_time);
static void ReportB(time_t tmp_time, time_t me_time);
static void ReportNALNonVLCBits(time_t tmp_time, time_t me_time);

StorablePicture *enc_picture;
StorablePicture **enc_frame_picture;
StorablePicture **enc_field_picture;
StorablePicture *enc_frame_picture_JV[MAX_PLANE];  //!< enc_frame_picture to be used during 4:4:4 independent mode encoding

void MbAffPostProc(void)
{
  imgpel temp[32][16];

  imgpel ** imgY  = enc_picture->imgY;
  imgpel ***imgUV = enc_picture->imgUV;
  int i, y, x0, y0, uv;

  if (img->yuv_format != YUV400)
  {
    for (i=0; i<(int)img->PicSizeInMbs; i+=2)
    {
      if (enc_picture->motion.mb_field[i])
      {
        get_mb_pos(i, img->mb_size[IS_LUMA], &x0, &y0);
        for (y=0; y<(2*MB_BLOCK_SIZE);y++)
          memcpy(&temp[y],&imgY[y0+y][x0], MB_BLOCK_SIZE * sizeof(imgpel));

        for (y=0; y<MB_BLOCK_SIZE;y++)
        {
          memcpy(&imgY[y0+(2*y)][x0],temp[y], MB_BLOCK_SIZE * sizeof(imgpel));
          memcpy(&imgY[y0+(2*y + 1)][x0],temp[y+ MB_BLOCK_SIZE], MB_BLOCK_SIZE * sizeof(imgpel));
        }

        x0 = x0 / (16/img->mb_cr_size_x);
        y0 = y0 / (16/img->mb_cr_size_y);

        for (uv=0; uv<2; uv++)
        {
          for (y=0; y < (2 * img->mb_cr_size_y); y++)
            memcpy(&temp[y],&imgUV[uv][y0+y][x0], img->mb_cr_size_x * sizeof(imgpel));

          for (y=0; y<img->mb_cr_size_y;y++)
          {
            memcpy(&imgUV[uv][y0+(2*y)][x0],temp[y], img->mb_cr_size_x * sizeof(imgpel));
            memcpy(&imgUV[uv][y0+(2*y + 1)][x0],temp[y+ img->mb_cr_size_y], img->mb_cr_size_x * sizeof(imgpel));
          }
        }
      }
    }
  }
  else
  {
    for (i=0; i<(int)img->PicSizeInMbs; i+=2)
    {
      if (enc_picture->motion.mb_field[i])
      {
        get_mb_pos(i, img->mb_size[IS_LUMA], &x0, &y0);
        for (y=0; y<(2*MB_BLOCK_SIZE);y++)
          memcpy(&temp[y],&imgY[y0+y][x0], MB_BLOCK_SIZE * sizeof(imgpel));

        for (y=0; y<MB_BLOCK_SIZE;y++)
        {
          memcpy(&imgY[y0+(2*y)][x0],temp[y], MB_BLOCK_SIZE * sizeof(imgpel));
          memcpy(&imgY[y0+(2*y + 1)][x0],temp[y+ MB_BLOCK_SIZE], MB_BLOCK_SIZE * sizeof(imgpel));
        }
      }
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Sets slice type
 *
 ************************************************************************
 */
void set_slice_type(int slice_type)
{
  img->type = slice_type;            // set slice type
  img->RCMinQP = params->RCMinQP[img->type];
  img->RCMaxQP = params->RCMaxQP[img->type];
}

void code_a_plane(ImageParameters *img, Picture *pic)
{
  unsigned int NumberOfCodedMBs = 0;
  int SliceGroup = 0;
  // The slice_group_change_cycle can be changed here.
  // FmoInit() is called before coding each picture, frame or field
  img->slice_group_change_cycle=1;
  FmoInit(img, active_pps, active_sps);
  FmoStartPicture ();           //! picture level initialization of FMO

  CalculateQuantParam();
  CalculateOffsetParam();

  if(params->Transform8x8Mode)
  {
    CalculateQuant8Param();
    CalculateOffset8Param();
  }

  reset_pic_bin_count();
  img->bytes_in_picture = 0;

  while (NumberOfCodedMBs < img->PicSizeInMbs)       // loop over slices
  {
    // Encode one SLice Group
    while (!FmoSliceGroupCompletelyCoded (SliceGroup))
    {
      // Encode the current slice
      NumberOfCodedMBs += encode_one_slice (SliceGroup, pic, NumberOfCodedMBs);
      FmoSetLastMacroblockInSlice (img->current_mb_nr);
      // Proceed to next slice
      img->current_slice_nr++;
      stats->bit_slice = 0;
    }
    // Proceed to next SliceGroup
    SliceGroup++;
  }
  FmoEndPicture ();

  if ((params->SkipDeBlockNonRef == 0) || (img->nal_reference_idc != 0))
    DeblockFrame (img, enc_picture->imgY, enc_picture->imgUV); //comment out to disable deblocking filter 
}
/*!
 ************************************************************************
 * \brief
 *    Encodes a picture
 *
 *    This is the main picture coding loop.. It is called by all this
 *    frame and field coding stuff after the img-> elements have been
 *    set up.  Not sure whether it is useful for MB-adaptive frame/field
 *    coding
 ************************************************************************
 */
static void code_a_picture(Picture *pic)
{
  int pl;
  int idr_refresh;

  // currently this code only supports fixed enhancement layer distance
  if ( params->idr_period && !params->adaptive_idr_period )
    idr_refresh = (( ( img->frm_number - img->lastIDRnumber ) % params->idr_period ) == 0);
  else if ( params->idr_period && params->adaptive_idr_period == 1 )
    idr_refresh = (( ( img->frm_number - imax(img->lastIntraNumber, img->lastIDRnumber) ) % params->idr_period ) == 0);
  else
    idr_refresh = (img->frm_number == 0);

  img->currentPicture = pic;

  img->currentPicture->idr_flag = ((!IMG_NUMBER) && (!(img->structure==BOTTOM_FIELD)))
    || (idr_refresh && (img->type == I_SLICE || img->type==SI_SLICE)&& (!(img->structure==BOTTOM_FIELD)));

  pic->no_slices = 0;

  RandomIntraNewPicture ();     //! Allocates forced INTRA MBs (even for fields!)

  if( IS_INDEPENDENT( params ) )
  {
    for( pl=0; pl<MAX_PLANE; pl++ )
    {
      img->current_mb_nr = 0;
      img->current_slice_nr = 0;
      img->SumFrameQP = 0;
      img->colour_plane_id = pl;
      
      code_a_plane(img, pic);
    }
  }
  else
  {
    code_a_plane(img, pic);
  }

  if (img->MbaffFrameFlag)
    MbAffPostProc();
}
void update_global_stats(StatParameters *cur_stats)
{  
  int i, j, k;
  for (i = 0; i < 4; i++)
    stats->intra_chroma_mode[i]    += cur_stats->intra_chroma_mode[i];

  for (i = 0; i < 5; i++)
  {
    stats->quant[i]                += cur_stats->quant[i];
    stats->num_macroblocks[i]      += cur_stats->num_macroblocks[i];
    stats->bit_use_mb_type [i]     += cur_stats->bit_use_mb_type[i];
    stats->bit_use_header  [i]     += cur_stats->bit_use_header[i];
    stats->tmp_bit_use_cbp [i]     += cur_stats->tmp_bit_use_cbp[i];
    stats->bit_use_coeffC  [i]     += cur_stats->bit_use_coeffC[i];
    stats->bit_use_coeff[0][i]     += cur_stats->bit_use_coeff[0][i];
    stats->bit_use_coeff[1][i]     += cur_stats->bit_use_coeff[1][i]; 
    stats->bit_use_coeff[2][i]     += cur_stats->bit_use_coeff[2][i]; 
    stats->bit_use_delta_quant[i]  += cur_stats->bit_use_delta_quant[i];
    stats->bit_use_stuffingBits[i] += cur_stats->bit_use_stuffingBits[i];

    for (k = 0; k < 2; k++)
      stats->b8_mode_0_use[i][k] += cur_stats->b8_mode_0_use[i][k];

    for (j = 0; j < 15; j++)
    {
      stats->mode_use[i][j]     += cur_stats->mode_use[i][j];
      stats->bit_use_mode[i][j] += cur_stats->bit_use_mode[i][j];
      for (k = 0; k < 2; k++)
        stats->mode_use_transform[i][j][k] += cur_stats->mode_use_transform[i][j][k];
    }
  }
}

void free_pictures(int stored_pic)
{
  int i;
  for (i = 0; i < 6; i++)
  {
    if (i != stored_pic)
      free_storable_picture(enc_frame_picture[i]);        
  }
}

/*!
 ************************************************************************
 * \brief
 *    Encodes one frame
 ************************************************************************
 */
int encode_one_frame (void)
{
  static int prev_frame_no = 0; // POC200301
  static int consecutive_non_reference_pictures = 0; // POC200301
  int        i, j;
  int   nplane;

  //Rate control
  int bits = 0;

#ifdef _LEAKYBUCKET_
  //extern long Bit_Buffer[20000];
  extern unsigned long total_frame_buffer;
#endif

  TIME_T start_time;
  TIME_T end_time;
  time_t  tmp_time;

  me_time = 0;
  img->rd_pass = 0;

  if( IS_INDEPENDENT(params) )
  {
    for( nplane=0; nplane<MAX_PLANE; nplane++ ){
      enc_frame_picture_JV[nplane] = NULL;
    }
  }

  for (i = 0; i < 6; i++)
    enc_frame_picture[i]  = NULL;

  gettime(&start_time);          // start time in ms

  //Rate control
  img->write_macroblock = FALSE;
  /*
  //Shankar Regunathan (Oct 2002)
  //Prepare Panscanrect SEI payload
  UpdatePanScanRectInfo ();
  //Prepare Arbitrarydata SEI Payload
  UpdateUser_data_unregistered ();
  //Prepare Registered data SEI Payload
  UpdateUser_data_registered_itu_t_t35 ();
  //Prepare RandomAccess SEI Payload
  UpdateRandomAccess ();
  */

  if ((params->ResendPPS) && (img->frm_number !=0))
  {
    stats->bit_ctr_parametersets_n = write_PPS(0, 0);
    stats->bit_ctr_parametersets  += stats->bit_ctr_parametersets_n;
  }

  put_buffer_frame (img);      // sets the pointers to the frame structures
                               // (and not to one of the field structures)
  init_frame (img);

  ReadOneFrame (FrameNumberInFile, params->infile_header, &params->source, &params->output);
  PaddAutoCropBorders (params->output, img->width, img->height, img->width_cr, img->height_cr);

  // set parameters for direct mode and deblocking filter
  img->direct_spatial_mv_pred_flag = params->direct_spatial_mv_pred_flag;

  img->DFDisableIdc                = params->DFDisableIdc[img->nal_reference_idc > 0][img->type];
  img->DFAlphaC0Offset             = params->DFAlpha     [img->nal_reference_idc > 0][img->type];
  img->DFBetaOffset                = params->DFBeta      [img->nal_reference_idc > 0][img->type];

  img->AdaptiveRounding            = params->AdaptiveRounding; 

  // Following code should consider optimal coding mode. Currently also does not support
  // multiple slices per frame.

  dist->frame_ctr++;

  if(img->type == SP_SLICE)
  {
    if(params->sp2_frame_indicator)
    { // switching SP frame encoding
      sp2_frame_indicator=1;
      read_SP_coefficients();
    }
  }
  else
  {
    sp2_frame_indicator=0;
  }
  if ( params->WPMCPrecision )
  {
    wpxInitWPXPasses(params);
    pWPX->curr_wp_rd_pass = pWPX->wp_rd_passes;
    pWPX->curr_wp_rd_pass->algorithm = WP_REGULAR;
  }


  if (params->PicInterlace == FIELD_CODING)
  {
    //Rate control
    if ( params->RCEnable && params->RCUpdateMode <= MAX_RC_MODE )
      generic_RC->FieldControl = 1;

    img->field_picture = 1;  // we encode fields
    field_picture (field_pic[0], field_pic[1]);
    img->fld_flag = TRUE;
  }
  else
  {
    int tmpFrameQP;
    //Rate control
    if ( params->RCEnable && params->RCUpdateMode <= MAX_RC_MODE )
      generic_RC->FieldControl = 0;

    img->field_picture = 0; // we encode a frame

    //Rate control
    if(params->RCEnable)
      rc_init_frame(FrameNumberInFile);

    if (params->GenerateMultiplePPS)
      active_pps = PicParSet[0];

    frame_picture (frame_pic[0], 0);

    if ((params->RDPictureIntra || img->type!=I_SLICE) && params->RDPictureDecision)
    {
      rdPictureCoding();
    }

    tmpFrameQP = img->SumFrameQP; // call it here since rdPictureCoding buffers it and may modify it

    if ((img->type==SP_SLICE) && (si_frame_indicator==0) && (params->si_frame_indicator))
    {
      // once the picture has been encoded as a primary SP frame encode as an SI frame
      si_frame_indicator=1;
      frame_picture (frame_pic_si, 0);
    }
    if ((img->type == SP_SLICE) && (params->sp_output_indicator))
    {
      // output the transformed and quantized coefficients (useful for switching SP frames)
      output_SP_coefficients();
    }

    if (params->PicInterlace == ADAPTIVE_CODING)
    {
      //Rate control
      if ( params->RCEnable && params->RCUpdateMode <= MAX_RC_MODE )
        generic_RC->FieldControl=1;
      img->write_macroblock = FALSE;
      img->bot_MB = FALSE;

      img->field_picture = 1;  // we encode fields
      field_picture (field_pic[0], field_pic[1]);

      if(img->rd_pass == 0)
        img->fld_flag = picture_structure_decision (frame_pic[0], field_pic[0], field_pic[1]);
      else if(img->rd_pass == 1)
        img->fld_flag = picture_structure_decision (frame_pic[1], field_pic[0], field_pic[1]);
      else
        img->fld_flag = picture_structure_decision (frame_pic[2], field_pic[0], field_pic[1]);

      if ( img->fld_flag )
        tmpFrameQP = img->SumFrameQP;

      update_field_frame_contexts (img->fld_flag);

      //Rate control
      if ( params->RCEnable && params->RCUpdateMode <= MAX_RC_MODE )
        generic_RC->FieldFrame = !(img->fld_flag) ? 1 : 0;
    }
    else
      img->fld_flag = FALSE;

    img->SumFrameQP = tmpFrameQP;
  }

  stats->frame_counter++;
  stats->frame_ctr[img->type]++;


  // Here, img->structure may be either FRAME or BOTTOM FIELD depending on whether AFF coding is used
  // The picture structure decision changes really only the fld_flag

  if (img->fld_flag)            // field mode (use field when fld_flag=1 only)
  {
    field_mode_buffer (img, params);
    writeout_picture (field_pic[0]);
    writeout_picture (field_pic[1]);
  }
  else                          //frame mode
  {
    frame_mode_buffer (img, params);

    if (img->type==SP_SLICE && si_frame_indicator == 1)
    {
      writeout_picture (frame_pic_si);
      si_frame_indicator=0;
    }
    else
    {
      writeout_picture (frame_pic[img->rd_pass]);
    }
  }

  if (frame_pic_si)
  {
    free_slice_list(frame_pic_si);
  }

  for (i = 0; i < img->frm_iter; i++)
  {
    if (frame_pic[i])
    {
      free_slice_list(frame_pic[i]);
    }
  }

  if (field_pic)
  {
    for (i = 0; i < 2; i++)
    {
      if (field_pic[i])
        free_slice_list(field_pic[i]);
    }
  }

  /*
  // Tian Dong (Sept 2002)
  // in frame mode, the newly reconstructed frame has been inserted to the mem buffer
  // and it is time to prepare the spare picture SEI payload.
  if (params->InterlaceCodingOption == FRAME_CODING
  && params->SparePictureOption && img->type != B_SLICE)
  CalculateSparePicture ();
  */

  //Rate control
  if(params->RCEnable)
  {
    // we could add here a function pointer!
    bits = (int) (stats->bit_ctr - stats->bit_ctr_n);
    if ( params->RCUpdateMode <= MAX_RC_MODE )
      rc_update_pict_frame_ptr(quadratic_RC, bits);
  }

  if (params->PicInterlace == FRAME_CODING)
  {
    if ((params->rdopt == 3) && (img->nal_reference_idc != 0))
    {
      UpdateDecoders (params, img, enc_picture);      // simulate packet losses and move decoded image to reference buffers
    }

    if (params->RestrictRef)
      UpdatePixelMap ();
  }

  compute_distortion();

  // redundant pictures: save reconstruction to calculate SNR and replace reference picture

  if(params->redundant_pic_flag)
  {
    int k;
    if(key_frame)
    {
      for(j=0; j<img->height; j++)
      {
        memcpy(imgY_tmp[j], enc_frame_picture[0]->imgY[j], img->width * sizeof(imgpel));
      }

      for (k = 0; k < 2; k++)
      {
        for(j=0; j<img->height_cr; j++)
        {
          memcpy(imgUV_tmp[k][j], enc_frame_picture[0]->imgUV[k][j], img->width_cr * sizeof(imgpel));
        }
      }
    }

    if(redundant_coding)
    {
      for(j=0; j<img->height; j++)
      {
        memcpy(enc_frame_picture[0]->imgY[j], imgY_tmp[j], img->width * sizeof(imgpel));
      }
      for (k = 0; k < 2; k++)
      {
        for(j=0; j<img->height_cr; j++)
        {
          memcpy(enc_frame_picture[0]->imgUV[k][j], imgUV_tmp[k][j], img->width_cr * sizeof(imgpel));
        }
      }
    }
  }

 
  if (params->PicInterlace == ADAPTIVE_CODING)
  {
    if (img->fld_flag)
    {
      // store bottom field
      store_picture_in_dpb(enc_field_picture[1]);
      free_storable_picture(enc_frame_picture[0]);
      free_storable_picture(enc_frame_picture[1]);
      free_storable_picture(enc_frame_picture[2]);
      update_global_stats(&enc_field_picture[0]->stats);
      update_global_stats(&enc_field_picture[1]->stats);
    }
    else
    {
      // replace top with frame
      if (img->rd_pass==2)
      {
        replace_top_pic_with_frame(enc_frame_picture[2]);
        free_storable_picture(enc_frame_picture[0]);
        free_storable_picture(enc_frame_picture[1]);
      }
      else if (img->rd_pass==1)
      {
        replace_top_pic_with_frame(enc_frame_picture[1]);
        free_storable_picture(enc_frame_picture[0]);
        free_storable_picture(enc_frame_picture[2]);
      }
      else
      {
        if(params->redundant_pic_flag==0 || (key_frame==0))
        {
          replace_top_pic_with_frame(enc_frame_picture[0]);
          free_storable_picture(enc_frame_picture[1]);
          free_storable_picture(enc_frame_picture[2]);
        }
      }
      free_storable_picture(enc_field_picture[1]);
      update_global_stats(&enc_frame_picture[img->rd_pass]->stats);
    }
  }
  else
  {
    if (img->fld_flag)
    {
      store_picture_in_dpb(enc_field_picture[1]);
      update_global_stats(&enc_field_picture[0]->stats);
      update_global_stats(&enc_field_picture[1]->stats);
    }
    else
    {
      if ((params->redundant_pic_flag != 1) || (key_frame == 0))
      {
        store_picture_in_dpb (enc_frame_picture[img->rd_pass]);
        update_global_stats(&enc_frame_picture[img->rd_pass]->stats);
        free_pictures(img->rd_pass);
      }
    }
  }

  img->AverageFrameQP = isign(img->SumFrameQP) * ((iabs(img->SumFrameQP) + (int) (img->FrameSizeInMbs >> 1))/ (int) img->FrameSizeInMbs);

  if ( params->RCEnable && params->RCUpdateMode <= MAX_RC_MODE && img->type != B_SLICE && params->basicunit < img->FrameSizeInMbs )
    quadratic_RC->CurrLastQP = img->AverageFrameQP;

#ifdef _LEAKYBUCKET_
  // Store bits used for this frame and increment counter of no. of coded frames
  Bit_Buffer[total_frame_buffer] = (int) (stats->bit_ctr - stats->bit_ctr_n);
  total_frame_buffer++;
#endif

  // POC200301: Verify that POC coding type 2 is not used if more than one consecutive
  // non-reference frame is requested or if decoding order is different from output order
  if (img->pic_order_cnt_type == 2)
  {
    if (!img->nal_reference_idc) consecutive_non_reference_pictures++;
    else consecutive_non_reference_pictures = 0;

    if (frame_no < prev_frame_no || consecutive_non_reference_pictures>1)
      error("POC type 2 cannot be applied for the coding pattern where the encoding /decoding order of pictures are different from the output order.\n", -1);
    prev_frame_no = frame_no;
  }

  gettime(&end_time);    // end time in ms
  tmp_time  = timediff(&start_time, &end_time);
  tot_time += tmp_time;

  if (stats->bit_ctr_parametersets_n!=0)
    ReportNALNonVLCBits(tmp_time, me_time);

  if (img->frm_number == 0)
    ReportFirstframe(tmp_time,me_time);
  else
  {
    //Rate control
    if(params->RCEnable)
    {
      if ((!params->PicInterlace) && (!params->MbInterlace))
        bits = (int) (stats->bit_ctr - stats->bit_ctr_n);
      else if ( params->RCUpdateMode <= MAX_RC_MODE )
      {
        bits = (int)(stats->bit_ctr - (quadratic_RC->Pprev_bits)); // used for rate control update
        quadratic_RC->Pprev_bits = stats->bit_ctr;
      }
    }

    stats->bit_counter[img->type] += stats->bit_ctr - stats->bit_ctr_n;

    switch (img->type)
    {
    case I_SLICE:
    case SI_SLICE:
      ReportIntra(tmp_time,me_time);
      break;
    case SP_SLICE:
      ReportSP(tmp_time,me_time);
      break;
    case B_SLICE:
      ReportB(tmp_time,me_time);
      break;
    default:      // P
      ReportP(tmp_time,me_time);
    }
  }

  if (params->Verbose == 0)
  {
    //for (i = 0; i <= (img->number & 0x0F); i++)
    //printf(".");
    //printf("                              \r");
    printf("Completed Encoding Frame %05d.\r",frame_no);
  }
  // Flush output statistics
  fflush(stdout);

  //Rate control
  if(params->RCEnable)
    rc_update_picture_ptr( bits );
  stats->bit_ctr_n = stats->bit_ctr;

  stats->bit_ctr_parametersets_n = 0;

  if ( img->type == I_SLICE && img->nal_reference_idc)
  {
    //img->lastINTRA = frame_no;
    // Lets also handle the possibility of backward GOPs and hierarchical structures 
    if ( !(img->b_frame_to_code) )
    {
      img->lastINTRA       = imax(img->lastINTRA, frame_no);
      img->lastIntraNumber = img->frm_number;
    }
    if ( img->currentPicture->idr_flag )
    {
      img->lastIDRnumber = img->frm_number;
    }
  }

  return ((IMG_NUMBER == 0)? 0 : 1);
}


/*!
 ************************************************************************
 * \brief
 *    This function write out a picture
 * \return
 *    0 if OK,                                                         \n
 *    1 in case of error
 *
 ************************************************************************
 */
static void writeout_picture(Picture *pic)
{
  int partition, slice;
  Slice  *currSlice;

  img->currentPicture = pic;

  // loop over all slices of the picture
  for (slice=0; slice < pic->no_slices; slice++)
  {
    currSlice = pic->slices[slice];

    // loop over the partitions
    for (partition=0; partition < currSlice->max_part_nr; partition++)
    {
      // write only if the partition has content
      if (currSlice->partArr[partition].bitstream->write_flag )
      {
        stats->bit_ctr += WriteNALU (currSlice->partArr[partition].nal_unit);
      }
    }
  }
}


void copy_params(StorablePicture *enc_picture, seq_parameter_set_rbsp_t *active_sps)
{
  enc_picture->frame_mbs_only_flag = active_sps->frame_mbs_only_flag;
  enc_picture->frame_cropping_flag = active_sps->frame_cropping_flag;
  enc_picture->chroma_format_idc   = active_sps->chroma_format_idc;

  if (active_sps->frame_cropping_flag)
  {
    enc_picture->frame_cropping_rect_left_offset   = active_sps->frame_cropping_rect_left_offset;
    enc_picture->frame_cropping_rect_right_offset  = active_sps->frame_cropping_rect_right_offset;
    enc_picture->frame_cropping_rect_top_offset    = active_sps->frame_cropping_rect_top_offset;
    enc_picture->frame_cropping_rect_bottom_offset = active_sps->frame_cropping_rect_bottom_offset;
  }
  else
  {
    enc_picture->frame_cropping_rect_left_offset   = 0;
    enc_picture->frame_cropping_rect_right_offset  = 0;
    enc_picture->frame_cropping_rect_top_offset    = 0;
    enc_picture->frame_cropping_rect_bottom_offset = 0;
  }
}

/*!
 ************************************************************************
 * \brief
 *    Prepare and allocate an encoded frame picture structure
 ************************************************************************
 */
static void prepare_enc_frame_picture (StorablePicture **stored_pic)
{
  (*stored_pic)              = alloc_storable_picture ((PictureStructure) img->structure, img->width, img->height, img->width_cr, img->height_cr);
  
  img->ThisPOC               = img->framepoc;
  (*stored_pic)->poc         = img->framepoc;
  (*stored_pic)->top_poc     = img->toppoc;
  (*stored_pic)->bottom_poc  = img->bottompoc;
  (*stored_pic)->frame_poc   = img->framepoc;
  (*stored_pic)->pic_num     = img->frame_num;
  (*stored_pic)->frame_num   = img->frame_num;
  (*stored_pic)->coded_frame = 1;
  (*stored_pic)->MbaffFrameFlag = img->MbaffFrameFlag = (params->MbInterlace != FRAME_CODING);
  
  get_mb_block_pos           = img->MbaffFrameFlag ? get_mb_block_pos_mbaff : get_mb_block_pos_normal;
  getNeighbour               = img->MbaffFrameFlag ? getAffNeighbour : getNonAffNeighbour;
  enc_picture                = *stored_pic;

  copy_params(enc_picture, active_sps);
}

static void calc_picture_bits(Picture *frame)
{
  int i, j;
  Slice *thisSlice = NULL;

  frame->bits_per_picture = 0;

  for ( i = 0; i < frame->no_slices; i++ )
  {
    thisSlice = frame->slices[i];

    for ( j = 0; j < thisSlice->max_part_nr; j++ )
      frame->bits_per_picture += 8 * ((thisSlice->partArr[j]).bitstream)->byte_pos;
  }
}
/*!
 ************************************************************************
 * \brief
 *    Encodes a frame picture
 ************************************************************************
 */
void frame_picture (Picture *frame, int rd_pass)
{
  int nplane;
  img->SumFrameQP = 0;
  img->structure = FRAME;
  img->PicSizeInMbs = img->FrameSizeInMbs;
  //set mv limits to frame type
  update_mv_limits(img, FALSE);


  if( IS_INDEPENDENT(params) )
  {
    for( nplane=0; nplane<MAX_PLANE; nplane++ )
    {
      prepare_enc_frame_picture( &enc_frame_picture_JV[nplane] );      
    }
  }
  else
  {
    prepare_enc_frame_picture( &enc_frame_picture[rd_pass] );
  }


  img->fld_flag = FALSE;
  code_a_picture(frame);

  if( IS_INDEPENDENT(params) )
  {
    make_frame_picture_JV();
  }

  calc_picture_bits(frame);

  if (img->structure==FRAME)
  {
    find_distortion ();
    frame->distortion = dist->metric[SSE];
  }
}


/*!
 ************************************************************************
 * \brief
 *    Encodes a field picture, consisting of top and bottom field
 ************************************************************************
 */
static void field_picture (Picture *top, Picture *bottom)
{
  //Rate control
  int old_pic_type;              // picture type of top field used for rate control
  int TopFieldBits;
  img->SumFrameQP = 0;
  //set mv limits to field type
  update_mv_limits(img, TRUE);

  //Rate control
  old_pic_type = img->type;

  img->number *= 2;
  img->buf_cycle *= 2;
  img->height    = (params->output.height + img->auto_crop_bottom) / 2;
  img->height_cr = img->height_cr_frame / 2;
  img->fld_flag  = TRUE;
  img->PicSizeInMbs = img->FrameSizeInMbs/2;
  // Top field

  enc_field_picture[0]              = alloc_storable_picture ((PictureStructure) img->structure, img->width, img->height, img->width_cr, img->height_cr);
  enc_field_picture[0]->poc         = img->toppoc;
  enc_field_picture[0]->frame_poc   = img->toppoc;
  enc_field_picture[0]->pic_num     = img->frame_num;
  enc_field_picture[0]->frame_num   = img->frame_num;
  enc_field_picture[0]->coded_frame = 0;
  enc_field_picture[0]->MbaffFrameFlag = img->MbaffFrameFlag = FALSE;
  get_mb_block_pos = get_mb_block_pos_normal;
  getNeighbour = getNonAffNeighbour;
  img->ThisPOC = img->toppoc;

  img->structure = TOP_FIELD;
  enc_picture = enc_field_picture[0];
  copy_params(enc_picture, active_sps);

  put_buffer_top (img);
  init_field (img);
  if (img->type == B_SLICE)       //all I- and P-frames
    nextP_tr_fld--;


  img->fld_flag = TRUE;

  //Rate control
  if(params->RCEnable && params->RCUpdateMode <= MAX_RC_MODE)
    rc_init_top_field();

  code_a_picture(field_pic[0]);
  enc_picture->structure = TOP_FIELD;

  store_picture_in_dpb(enc_field_picture[0]);

  calc_picture_bits(top);

  //Rate control
  TopFieldBits=top->bits_per_picture;

  //  Bottom field
  enc_field_picture[1]  = alloc_storable_picture ((PictureStructure) img->structure, img->width, img->height, img->width_cr, img->height_cr);
  enc_field_picture[1]->poc=img->bottompoc;
  enc_field_picture[1]->frame_poc = img->bottompoc;
  enc_field_picture[1]->pic_num = img->frame_num;
  enc_field_picture[1]->frame_num = img->frame_num;
  enc_field_picture[1]->coded_frame = 0;
  enc_field_picture[1]->MbaffFrameFlag = img->MbaffFrameFlag = FALSE;
  get_mb_block_pos = get_mb_block_pos_normal;
  getNeighbour = getNonAffNeighbour;

  img->ThisPOC = img->bottompoc;
  img->structure = BOTTOM_FIELD;
  enc_picture = enc_field_picture[1];
  copy_params(enc_picture, active_sps);
  put_buffer_bot (img);
  img->number++;

  init_field (img);

  if (img->type == B_SLICE)       //all I- and P-frames
    nextP_tr_fld++;             //check once coding B field

 if (img->type == I_SLICE && params->IntraBottom!=1)
   set_slice_type((params->BRefPictures == 2) ? B_SLICE : P_SLICE);

  img->fld_flag = TRUE;

  //Rate control
  if(params->RCEnable && params->RCUpdateMode <= MAX_RC_MODE)
    rc_init_bottom_field( TopFieldBits );

  enc_picture->structure = BOTTOM_FIELD;
  code_a_picture(field_pic[1]);

  calc_picture_bits(bottom);

  // the distortion for a field coded frame (consisting of top and bottom field)
  // lives in the top->distortion variables, the bottom-> are dummies
  distortion_fld (top);
}

/*!
 ************************************************************************
 * \brief
 *    form frame picture from two field pictures
 ************************************************************************
 */
static void combine_field(void)
{
  int i, k;

  for (i = 0; i < (img->height >> 1); i++)
  {
    memcpy(imgY_com[i*2],     enc_field_picture[0]->imgY[i], img->width*sizeof(imgpel));     // top field
    memcpy(imgY_com[i*2 + 1], enc_field_picture[1]->imgY[i], img->width*sizeof(imgpel)); // bottom field
  }

  if (img->yuv_format != YUV400)
  {
    for (k = 0; k < 2; k++)
    {
      for (i = 0; i < (img->height_cr >> 1); i++)
      {
        memcpy(imgUV_com[k][i*2],     enc_field_picture[0]->imgUV[k][i], img->width_cr*sizeof(imgpel));
        memcpy(imgUV_com[k][i*2 + 1], enc_field_picture[1]->imgUV[k][i], img->width_cr*sizeof(imgpel));
      }
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Distortion Field
 ************************************************************************
 */
static void distortion_fld (Picture *field_pic)
{

  img->number /= 2;
  img->buf_cycle /= 2;
  img->height    = (params->output.height + img->auto_crop_bottom);
  img->height_cr = img->height_cr_frame;

  combine_field ();

  pCurImg   = img_org_frm[0];
  pImgOrg[0] = img_org_frm[0];

  if (params->yuv_format != YUV400)
  {
    pImgOrg[1] = img_org_frm[1];
    pImgOrg[2] = img_org_frm[2];
  }

  find_distortion ();   // find snr from original frame picture
  field_pic->distortion = dist->metric[SSE];
}


/*!
 ************************************************************************
 * \brief
 *    RD decision of frame and field coding
 ************************************************************************
 */
static byte decide_fld_frame(float snr_frame_Y, float snr_field_Y, int bit_field, int bit_frame, double lambda_picture)
{
  double cost_frame, cost_field;

  cost_frame = bit_frame * lambda_picture + snr_frame_Y;
  cost_field = bit_field * lambda_picture + snr_field_Y;

  if (cost_field > cost_frame)
    return FALSE;
  else
    return TRUE;
}

/*!
 ************************************************************************
 * \brief
 *    Picture Structure Decision
 ************************************************************************
 */
static byte picture_structure_decision (Picture *frame, Picture *top, Picture *bot)
{
  double lambda_picture;
  int bframe = (img->type == B_SLICE);
  float sse_frame, sse_field;
  int bit_frame, bit_field;

  lambda_picture = 0.68 * pow (2, img->bitdepth_lambda_scale + ((img->qp - SHIFT_QP) / 3.0)) * (bframe ? 1 : 1);

  sse_frame = frame->distortion.value[0] + frame->distortion.value[1] + frame->distortion.value[2];
  //! all distrortions of a field picture are accumulated in the top field
  sse_field = top->distortion.value[0] + top->distortion.value[1] + top->distortion.value[2];

  bit_field = top->bits_per_picture + bot->bits_per_picture;
  bit_frame = frame->bits_per_picture;
  return decide_fld_frame (sse_frame, sse_field, bit_field, bit_frame, lambda_picture);
}


/*!
 ************************************************************************
 * \brief
 *    Field Mode Buffer
 ************************************************************************
 */
static void field_mode_buffer (ImageParameters *img, InputParameters *params)
{
  put_buffer_frame (img);
}


/*!
 ************************************************************************
 * \brief
 *    Frame Mode Buffer
 ************************************************************************
 */
static void frame_mode_buffer (ImageParameters *img, InputParameters *params)
{
  put_buffer_frame (img);

  if ((params->PicInterlace != FRAME_CODING)||(params->MbInterlace != FRAME_CODING))
  {
    img->height = img->height / 2;
    img->height_cr = img->height_cr / 2;
    img->number *= 2;

    put_buffer_top (img);

    img->number++;
    put_buffer_bot (img);

    img->number /= 2;         // reset the img->number to field
    img->height = (params->output.height + img->auto_crop_bottom);
    img->height_cr = img->height_cr_frame;

    put_buffer_frame (img);
  }
}


/*!
 ************************************************************************
 * \brief
 *    mmco initializations should go here
 ************************************************************************
 */
static void init_dec_ref_pic_marking_buffer(void)
{
  img->dec_ref_pic_marking_buffer=NULL;
}


/*!
 ************************************************************************
 * \brief
 *    Initializes the parameters for a new frame
 ************************************************************************
 */
static void init_frame (ImageParameters *img)
{
  int i, j;
  int prevP_no, nextP_no;

  last_P_no = last_P_no_frm;

  img->current_mb_nr = 0;
  img->current_slice_nr = 0;
  stats->bit_slice = 0;

  img->mb_y = img->mb_x = 0;
  img->block_y = img->pix_y = img->pix_c_y = 0;
  img->block_x = img->pix_x = img->block_c_x = img->pix_c_x = 0;

  // The 'slice_nr' of each macroblock is set to -1 here, to guarantee the correct encoding
  // with FMO (if no FMO, encoding is correct without following assignment),
  // for which MBs may not be encoded with scan order
  if( IS_INDEPENDENT(params) )
  {
    for( j=0; j<MAX_PLANE; j++ ){
      for(i=0;i< ((int) (img->FrameSizeInMbs));i++)
        img->mb_data_JV[j][i].slice_nr=-1;
    }
  }
  else
  {
    for(i = 0; i < ((int) (img->FrameSizeInMbs)); i++)
      img->mb_data[i].slice_nr = -1;
  }

  if (img->b_frame_to_code == 0)
  {
    img->tr = start_tr_in_this_IGOP + IMG_NUMBER * (params->jumpd + 1);

#ifdef _ADAPT_LAST_GROUP_
    if (params->last_frame && img->frm_number + 1 == params->no_frames)
      img->tr = params->last_frame;
#endif

    if (IMG_NUMBER != 0 && params->successive_Bframe != 0)     // B pictures to encode
      nextP_tr_frm = img->tr;
 
    //Rate control
    if(!params->RCEnable)                  // without using rate control
    {
      if (img->type == I_SLICE)
      {
#ifdef _CHANGE_QP_
        //QP oscillation for secondary SP frames
        if ((params->qp2start > 0 && img->tr >= params->qp2start && params->sp2_frame_indicator==0)||
          ( (params->qp2start > 0) && ( ( (img->tr ) % (2*params->qp2start) ) >=params->qp2start ) && (params->sp2_frame_indicator==1)))
          img->qp = params->qp02;
        else
#endif
          img->qp = params->qp0;   // set quant. parameter for I-frame
      }
      else
      {
#ifdef _CHANGE_QP_
        //QP oscillation for secondary SP frames
        if ((params->qp2start > 0 && img->tr >= params->qp2start && params->sp2_frame_indicator==0)||
          ( (params->qp2start > 0) && ( ( (img->tr ) % (2*params->qp2start) ) >=params->qp2start ) && (params->sp2_frame_indicator==1)))
          img->qp = params->qpN2 + (img->nal_reference_idc ? 0 : params->DispPQPOffset);
        else
#endif
          img->qp = params->qpN + (img->nal_reference_idc ? 0 : params->DispPQPOffset);

        if (img->type == SP_SLICE)
        {
          if ( (params->qp2start > 0) && ( ( (img->tr ) % (2*params->qp2start) ) >=params->qp2start ))
          {
            img->qp   = params->qpN2 - (params->qpN - params->qpsp);
            img->qpsp = params->qpN2 - (params->qpN - params->qpsp_pred);
          }
          else
          {
            img->qp = params->qpsp;
            img->qpsp = params->qpsp_pred;
          }
        }
      }
    }

    img->mb_y_intra = img->mb_y_upd;  //  img->mb_y_intra indicates which GOB to intra code for this frame

    if (params->intra_upd > 0) // if error robustness, find next GOB to update
    {
      img->mb_y_upd = (img->frm_number / params->intra_upd) % (img->height / MB_BLOCK_SIZE);
    }
  }
  else
  {
    img->p_interval = params->jumpd + 1;
    prevP_no = start_tr_in_this_IGOP + (IMG_NUMBER - 1) * img->p_interval;
    nextP_no = start_tr_in_this_IGOP + (IMG_NUMBER) * img->p_interval;

#ifdef _ADAPT_LAST_GROUP_
    last_P_no[0] = prevP_no;
    for (i = 1; i < img->buf_cycle; i++)
      last_P_no[i] = last_P_no[i - 1] - img->p_interval;

    if (params->last_frame && img->frm_number + 1 == params->no_frames)
    {
      nextP_no = params->last_frame;
      img->p_interval = nextP_no - prevP_no;
    }
#endif

    img->b_interval =
      ((double) (params->jumpd + 1) / (params->successive_Bframe + 1.0) );

    if (params->HierarchicalCoding == 3)
      img->b_interval = 1.0;

    if (params->HierarchicalCoding)
      img->tr = prevP_no + (int) (img->b_interval  * (double) (1 + gop_structure[img->b_frame_to_code - 1].display_no));      // from prev_P
    else
      img->tr = prevP_no + (int) (img->b_interval * (double) img->b_frame_to_code);      // from prev_P


    if (img->tr >= nextP_no)
      img->tr = nextP_no - 1;
    //Rate control
    if(!params->RCEnable && params->HierarchicalCoding == 0)                  // without using rate control
    {
#ifdef _CHANGE_QP_
      //QP oscillation for secondary SP frames
      if ((params->qp2start > 0 && img->tr >= params->qp2start && params->sp2_frame_indicator==0)||
        ( (params->qp2start > 0) && ( ( (img->tr ) % (2*params->qp2start) ) >=params->qp2start ) && (params->sp2_frame_indicator==1)))
      {
        img->qp = params->qpB2;
      }
      else
#endif
      {
        img->qp = params->qpB;
      }

      if (img->nal_reference_idc)
      {
#ifdef _CHANGE_QP_
        //QP oscillation for secondary SP frames
        if ((params->qp2start > 0 && img->tr >= params->qp2start && params->sp2_frame_indicator==0)||
          ( (params->qp2start > 0) && ( ( (img->tr ) % (2*params->qp2start) ) >=params->qp2start ) && (params->sp2_frame_indicator==1)))
        {
          img->qp = iClip3(-img->bitdepth_luma_qp_scale,51,params->qpB2 + params->qpBRS2Offset);
        }
        else
#endif
        {
          img->qp = iClip3(-img->bitdepth_luma_qp_scale,51,params->qpB + params->qpBRSOffset);
        }
      }
    }
    else if (!params->RCEnable && params->HierarchicalCoding !=0)
    {
      // Note that _CHANGE_QP_ does not anymore work for gop_structure. Needs to be fixed
      img->qp =  gop_structure[img->b_frame_to_code - 1].slice_qp;
    }
  }

  UpdateSubseqInfo (img->layer);        // Tian Dong (Sept 2002)
  UpdateSceneInformation (FALSE, 0, 0, -1); // JVT-D099, scene information SEI, nothing included by default

  //! Commented out by StW, needs fixing in SEI.h to keep the trace file clean
  //  PrepareAggregationSEIMessage ();

  // write tone mapping SEI message
  if (params->ToneMappingSEIPresentFlag)
  {
    UpdateToneMapping();
  }
  PrepareAggregationSEIMessage ();
  Write_SEI_NALU(0);

  img->no_output_of_prior_pics_flag = 0;
  img->long_term_reference_flag = 0;

  init_dec_ref_pic_marking_buffer();
}

/*!
 ************************************************************************
 * \brief
 *    Initializes the parameters for a new field
 ************************************************************************
 */
static void init_field (ImageParameters *img)
{
  int i;
  int prevP_no, nextP_no;

  last_P_no = last_P_no_fld;

  img->current_mb_nr = 0;
  img->current_slice_nr = 0;
  stats->bit_slice = 0;

  params->jumpd *= 2;
  params->successive_Bframe *= 2;
  img->number /= 2;
  img->buf_cycle /= 2;

  img->mb_y = img->mb_x = 0;
  img->block_y = img->pix_y = img->pix_c_y = 0; // define vertical positions
  img->block_x = img->pix_x = img->block_c_x = img->pix_c_x = 0;        // define horizontal positions

  if (!img->b_frame_to_code)
  {
    img->tr = img->number * (params->jumpd + 2) + img->fld_type;

#ifdef _ADAPT_LAST_GROUP_
    if (params->last_frame && img->number + 1 == params->no_frames)
      img->tr = params->last_frame;
#endif
    if (img->number != 0 && params->successive_Bframe != 0)    // B pictures to encode
      nextP_tr_fld = img->tr;

      //Rate control
    if(!params->RCEnable)                  // without using rate control
    {
      if (img->type == I_SLICE)
      {
#ifdef _CHANGE_QP_
        //QP oscillation for secondary SP frames
        if ((params->qp2start > 0 && img->tr >= params->qp2start && params->sp2_frame_indicator==0)||
          ( (params->qp2start > 0) && ( ( (img->tr ) % (2*params->qp2start) ) >=params->qp2start ) && (params->sp2_frame_indicator==1)))
          img->qp = params->qp02;
        else
#endif
          img->qp = params->qp0;   // set quant. parameter for I-frame
      }
      else
      {
#ifdef _CHANGE_QP_
        //QP oscillation for secondary SP frames
        if ((params->qp2start > 0 && img->tr >= params->qp2start && params->sp2_frame_indicator==0)||
          ( (params->qp2start > 0) && ( ( (img->tr ) % (2*params->qp2start) ) >=params->qp2start ) && (params->sp2_frame_indicator==1)))
          img->qp = params->qpN2 + (img->nal_reference_idc ? 0 : params->DispPQPOffset);
        else
#endif
          img->qp = params->qpN + (img->nal_reference_idc ? 0 : params->DispPQPOffset);
        if (img->type == SP_SLICE)
        {
          if ( (params->qp2start > 0) && ( ( (img->tr ) % (2*params->qp2start) ) >=params->qp2start ))
          {
            img->qp = params->qpN2-(params->qpN-params->qpsp);
            img->qpsp = params->qpN2-(params->qpN-params->qpsp_pred);
          }
          else
          {
            img->qp = params->qpsp;
            img->qpsp = params->qpsp_pred;
          }
        }
      }
    }
    img->mb_y_intra = img->mb_y_upd;  //  img->mb_y_intra indicates which GOB to intra code for this frame

    if (params->intra_upd > 0) // if error robustness, find next GOB to update
    {
      img->mb_y_upd =
        (img->number / params->intra_upd) % (img->width / MB_BLOCK_SIZE);
    }
  }
  else
  {
    img->p_interval = params->jumpd + 2;
    prevP_no = (img->number - 1) * img->p_interval + img->fld_type;
    nextP_no = img->number * img->p_interval + img->fld_type;

#ifdef _ADAPT_LAST_GROUP_
    if (!img->fld_type)       // top field
    {
      last_P_no[0] = prevP_no + 1;
      last_P_no[1] = prevP_no;
      for (i = 1; i <= img->buf_cycle; i++)
      {
        last_P_no[2 * i] = last_P_no[2 * i - 2] - img->p_interval;
        last_P_no[2 * i + 1] = last_P_no[2 * i - 1] - img->p_interval;
      }
    }
    else                      // bottom field
    {
      last_P_no[0] = nextP_no - 1;
      last_P_no[1] = prevP_no;
      for (i = 1; i <= img->buf_cycle; i++)
      {
        last_P_no[2 * i] = last_P_no[2 * i - 2] - img->p_interval;
        last_P_no[2 * i + 1] = last_P_no[2 * i - 1] - img->p_interval;
      }
    }

    if (params->last_frame && img->number + 1 == params->no_frames)
    {
      nextP_no = params->last_frame;
      img->p_interval = nextP_no - prevP_no;
    }
#endif

    img->b_interval =
      ((double) (params->jumpd + 1) / (params->successive_Bframe + 1.0) );

    if (params->HierarchicalCoding == 3)
      img->b_interval = 1.0;

    if (params->HierarchicalCoding)
      img->tr = prevP_no + (int) ((img->b_interval + 1.0) * (double) (1 + gop_structure[img->b_frame_to_code - 1].display_no));      // from prev_P
    else
      img->tr = prevP_no + (int) ((img->b_interval + 1.0) * (double) img->b_frame_to_code);      // from prev_P


    if (img->tr >= nextP_no)
      img->tr = nextP_no - 1; // ?????
    //Rate control
    if(!params->RCEnable && params->HierarchicalCoding == 0)                  // without using rate control
    {
#ifdef _CHANGE_QP_
      //QP oscillation for secondary SP frames
      if ((params->qp2start > 0 && img->tr >= params->qp2start && params->sp2_frame_indicator==0)||
        ( (params->qp2start > 0) && ( ( (img->tr ) % (2*params->qp2start) ) >=params->qp2start ) && (params->sp2_frame_indicator==1)))
      {
        img->qp = params->qpB2;
      }
      else
#endif
        img->qp = params->qpB;
      if (img->nal_reference_idc)
      {

#ifdef _CHANGE_QP_
        //QP oscillation for secondary SP frames
        if ((params->qp2start > 0 && img->tr >= params->qp2start && params->sp2_frame_indicator==0)||
          ( (params->qp2start > 0) && ( ( (img->tr ) % (2*params->qp2start) ) >=params->qp2start ) && (params->sp2_frame_indicator==1)))
        {
          img->qp = iClip3(-img->bitdepth_luma_qp_scale,51,params->qpB2 + params->qpBRS2Offset);
        }
        else
#endif
          img->qp = iClip3(-img->bitdepth_luma_qp_scale,51,params->qpB + params->qpBRSOffset);

      }
    }
    else if (!params->RCEnable && params->HierarchicalCoding != 0)
    {
      img->qp =  gop_structure[img->b_frame_to_code - 1].slice_qp;
    }
  }
  params->jumpd /= 2;
  params->successive_Bframe /= 2;
  img->buf_cycle *= 2;
  img->number = 2 * img->number + img->fld_type;
}



/*!
 ************************************************************************
 * \brief
 *    Upsample 4 times, store them in out4x.  Color is simply copied
 *
 * \par Input:
 *    srcy, srcu, srcv, out4y, out4u, out4v
 *
 * \par Side Effects_
 *    Uses (writes) img4Y_tmp.  This should be moved to a static variable
 *    in this module
 ************************************************************************/
void UnifiedOneForthPix (StorablePicture *s)
{
  int ypadded_size = s->size_y_padded;
  int xpadded_size = s->size_x_padded;

  // don't upsample twice
  if (s->imgY_sub)
    return;
  // Y component
  get_mem4Dpel (&(s->imgY_sub), 4, 4, ypadded_size, xpadded_size);
  if (NULL == s->imgY_sub)
    no_mem_exit("alloc_storable_picture: s->imgY_sub");

  s->p_img_sub[0] = s->imgY_sub;
  s->p_curr_img_sub = s->p_img_sub[0];

  if ( params->ChromaMCBuffer || img->P444_joined)
  {
    // UV components
    if ( img->yuv_format != YUV400 )
    {
      if ( img->yuv_format == YUV420 )
      {
        get_mem5Dpel (&(s->imgUV_sub), 2, 8, 8, ypadded_size>>1, xpadded_size>>1);
      }
      else if ( img->yuv_format == YUV422 )
      {
        get_mem5Dpel (&(s->imgUV_sub), 2, 4, 8, ypadded_size, xpadded_size>>1);
      }
      else
      { // YUV444
        get_mem5Dpel (&(s->imgUV_sub), 2, 4, 4, ypadded_size, xpadded_size);
      }
      s->p_img_sub[1] = s->imgUV_sub[0];
      s->p_img_sub[2] = s->imgUV_sub[1];
    }
    else
    {
      s->p_img_sub[1] = NULL;
      s->p_img_sub[2] = NULL;
    }
  }
  else
  {
    s->p_img_sub[1] = NULL;
    s->p_img_sub[2] = NULL;
  }
  s->p_curr_img = s->imgY;
  s->p_curr_img_sub = s->imgY_sub;

  // derive the subpixel images for first component
  getSubImagesLuma ( s );

  // and the sub-images for U and V
  if ( (img->yuv_format != YUV400) && (params->ChromaMCBuffer) )
  {
    if (img->P444_joined)
    {
      //U
      select_plane(PLANE_U);
      getSubImagesLuma (s);
      //V
      select_plane(PLANE_V);
      getSubImagesLuma (s);
      //Y
      select_plane(PLANE_Y);
    }
    else
      getSubImagesChroma( s );
  }
}

/*!
 ************************************************************************
 * \brief
 *    Upsample 4 times, store them in out4x.  Color is simply copied
 *    for 4:4:4 Independent mode
 *
 * \par Input:
 *    nplane
 *
 ************************************************************************/
void UnifiedOneForthPix_JV (int nplane, StorablePicture *s)
{
  int ypadded_size = s->size_y_padded;
  int xpadded_size = s->size_x_padded;

  if( nplane == 0 )
  {
    // don't upsample twice
    if (s->imgY_sub)
      return;
    // Y component
    get_mem4Dpel (&(s->imgY_sub), 4, 4, ypadded_size, xpadded_size);
    if (NULL == s->imgY_sub)
      no_mem_exit("UnifiedOneForthPix_JV: s->imgY_sub");

    // don't upsample twice
    if (s->imgUV_sub)
      return;
    // Y component
    get_mem5Dpel (&(s->imgUV_sub), 2, 4, 4, ypadded_size, xpadded_size);
    if (NULL == s->imgUV_sub)
      no_mem_exit("UnifiedOneForthPix_JV: s->imgUV_sub");

    s->p_img[0] = s->imgY;
    s->p_img[1] = s->imgUV[0];
    s->p_img[2] = s->imgUV[1];

    s->p_img_sub[0] = s->imgY_sub;
    s->p_img_sub[1] = s->imgUV_sub[0];
    s->p_img_sub[2] = s->imgUV_sub[1];
  }

  // derive the subpixel images for first component
  s->colour_plane_id = nplane;
  s->p_curr_img = s->p_img[nplane];
  s->p_curr_img_sub = s->p_img_sub[nplane];

  getSubImagesLuma ( s );
}



  /*!
 ************************************************************************
 * \brief
 *    Just a placebo
 ************************************************************************
 */
Boolean dummy_slice_too_big (int bits_slice)
{
  return FALSE;
}


static inline void copy_motion_vectors_MB (ImageParameters *img, RD_DATA *rdopt)
{
  memcpy(&img->all_mv [LIST_0][0][0][0][0][0], &rdopt->all_mv [LIST_0][0][0][0][0][0], 2 * img->max_num_references * 9 * 4 * 4 * 2 * sizeof(short));  
  memcpy(&img->pred_mv[LIST_0][0][0][0][0][0], &rdopt->pred_mv[LIST_0][0][0][0][0][0], 2 * img->max_num_references * 9 * 4 * 4 * 2 * sizeof(short));
}


/*!
***************************************************************************
// For MB level field/frame coding
***************************************************************************
*/
void copy_rdopt_data (Macroblock *currMB, int bot_block)
{
  int i, j;

  int bframe = (img->type == B_SLICE);
  int mode;
  short b8mode, b8pdir;
  int block_y;

  int list_offset = currMB->list_offset;

  mode                = rdopt->mode;
  currMB->mb_type     = rdopt->mb_type;    // copy mb_type
  currMB->cbp         = rdopt->cbp;        // copy cbp
  currMB->cbp_blk     = rdopt->cbp_blk;    // copy cbp_blk
  img->i16offset      = rdopt->i16offset;

  currMB->prev_qp  = rdopt->prev_qp;
  currMB->prev_dqp = rdopt->prev_dqp;
  currMB->prev_cbp = rdopt->prev_cbp;
  currMB->delta_qp = rdopt->delta_qp;
  currMB->qp       = rdopt->qp;
  update_qp (img, currMB);

  currMB->c_ipred_mode = rdopt->c_ipred_mode;

  memcpy(img->cofAC[0][0][0],rdopt->cofAC[0][0][0], (4 + img->num_blk8x8_uv) * 4 * 2 * 65 * sizeof(int));
  memcpy(img->cofDC[0][0],rdopt->cofDC[0][0], 3 * 2 * 18 * sizeof(int));

  for (j = 0; j < BLOCK_MULTIPLE; j++)
  {
    block_y = img->block_y + j;
    memcpy(&enc_picture->motion.ref_idx[LIST_0][block_y][img->block_x], rdopt->refar[LIST_0][j], BLOCK_MULTIPLE * sizeof(char));
    for (i = 0; i < BLOCK_MULTIPLE; i++)
      enc_picture->motion.ref_pic_id [LIST_0][block_y][img->block_x + i] =
      enc_picture->ref_pic_num[LIST_0 + list_offset][(short)enc_picture->motion.ref_idx[LIST_0][block_y][img->block_x+i]];
  }
  if (bframe)
  {
    for (j = 0; j < BLOCK_MULTIPLE; j++)
    {
      block_y = img->block_y + j;
      memcpy(&enc_picture->motion.ref_idx[LIST_1][block_y][img->block_x], rdopt->refar[LIST_1][j], BLOCK_MULTIPLE * sizeof(char));
      for (i = 0; i < BLOCK_MULTIPLE; i++)
        enc_picture->motion.ref_pic_id [LIST_1][block_y][img->block_x + i] =
        enc_picture->ref_pic_num[LIST_1 + list_offset][(short)enc_picture->motion.ref_idx[LIST_1][block_y][img->block_x + i]];
    }
  }

  //===== reconstruction values =====
  for (j = 0; j < MB_BLOCK_SIZE; j++)
    memcpy(&enc_picture->imgY[img->pix_y + j][img->pix_x],rdopt->rec_mbY[j], MB_BLOCK_SIZE * sizeof(imgpel));

  if (img->yuv_format != YUV400)
  {
    for (j = 0; j < img->mb_cr_size_y; j++)
    {
      memcpy(&enc_picture->imgUV[0][img->pix_c_y + j][img->pix_c_x], rdopt->rec_mb_cr[0][j], img->mb_cr_size_x * sizeof(imgpel));
      memcpy(&enc_picture->imgUV[1][img->pix_c_y + j][img->pix_c_x], rdopt->rec_mb_cr[1][j], img->mb_cr_size_x * sizeof(imgpel));
    }
  }

  memcpy(currMB->b8mode, rdopt->b8mode, 4 * sizeof(short));
  memcpy(currMB->b8pdir, rdopt->b8pdir, 4 * sizeof(short));


  currMB->luma_transform_size_8x8_flag = rdopt->luma_transform_size_8x8_flag;

  //==== intra prediction modes ====
  if (mode == P8x8)
  {
    memcpy(currMB->intra_pred_modes,rdopt->intra_pred_modes, MB_BLOCK_PARTITIONS * sizeof(char));
    for (j = img->block_y; j < img->block_y + BLOCK_MULTIPLE; j++)
      memcpy(&img->ipredmode[j][img->block_x],&rdopt->ipredmode[j][img->block_x], BLOCK_MULTIPLE * sizeof(char));
  }
  else if (mode != I4MB && mode != I8MB)
  {
    memset(currMB->intra_pred_modes,DC_PRED, MB_BLOCK_PARTITIONS * sizeof(char));
    for (j = img->block_y; j < img->block_y + BLOCK_MULTIPLE; j++)
      memset(&img->ipredmode[j][img->block_x],DC_PRED, BLOCK_MULTIPLE * sizeof(char));
  }
  else if (mode == I4MB || mode == I8MB)
  {
    memcpy(currMB->intra_pred_modes,rdopt->intra_pred_modes, MB_BLOCK_PARTITIONS * sizeof(char));
    memcpy(currMB->intra_pred_modes8x8,rdopt->intra_pred_modes8x8, MB_BLOCK_PARTITIONS * sizeof(char));
    for (j = img->block_y; j < img->block_y + BLOCK_MULTIPLE; j++) 
    {
      memcpy(&img->ipredmode[j][img->block_x],&rdopt->ipredmode[j][img->block_x], BLOCK_MULTIPLE * sizeof(char));
    }
  }

  if (img->MbaffFrameFlag || (params->UseRDOQuant && params->RDOQ_QP_Num > 1))
  {
    // motion vectors
    copy_motion_vectors_MB (img, rdopt);

    if (!IS_INTRA(currMB))
    {
      memset(currMB->bipred_me, 0, 4* sizeof(short));

      for (j = 0; j < 4; j++)
        for (i = 0; i < 4; i++)
        {
          b8mode = currMB->b8mode[i/2+2*(j/2)];
          b8pdir = currMB->b8pdir[i/2+2*(j/2)];

          if (b8pdir!=1)
          {
            enc_picture->motion.mv[LIST_0][j+img->block_y][i+img->block_x][0] = rdopt->all_mv[LIST_0][(short)rdopt->refar[LIST_0][j][i]][b8mode][j][i][0];
            enc_picture->motion.mv[LIST_0][j+img->block_y][i+img->block_x][1] = rdopt->all_mv[LIST_0][(short)rdopt->refar[LIST_0][j][i]][b8mode][j][i][1];
          }
          else
          {
            enc_picture->motion.mv[LIST_0][j+img->block_y][i+img->block_x][0] = 0;
            enc_picture->motion.mv[LIST_0][j+img->block_y][i+img->block_x][1] = 0;
          }
          if (bframe)
          {
            if (b8pdir!=0)
            {
              enc_picture->motion.mv[LIST_1][j+img->block_y][i+img->block_x][0] = rdopt->all_mv[LIST_1][(short)rdopt->refar[LIST_1][j][i]][b8mode][j][i][0];
              enc_picture->motion.mv[LIST_1][j+img->block_y][i+img->block_x][1] = rdopt->all_mv[LIST_1][(short)rdopt->refar[LIST_1][j][i]][b8mode][j][i][1];
            }
            else
            {
              enc_picture->motion.mv[LIST_1][j+img->block_y][i+img->block_x][0] = 0;
              enc_picture->motion.mv[LIST_1][j+img->block_y][i+img->block_x][1] = 0;
            }
          }
        }
    }
    else
    {
      for (j = 0; j < 4; j++)
        memset(enc_picture->motion.mv[LIST_0][j+img->block_y][img->block_x], 0, 2 * BLOCK_MULTIPLE * sizeof(short));
      if (bframe)
      {
        for (j = 0; j < 4; j++)
          memset(enc_picture->motion.mv[LIST_1][j+img->block_y][img->block_x], 0, 2 * BLOCK_MULTIPLE * sizeof(short));
      }
    }
  }
} // end of copy_rdopt_data


static void ReportNALNonVLCBits(time_t tmp_time, time_t me_time)
{
  //! Need to add type (i.e. SPS, PPS, SEI etc).
  if (params->Verbose != 0)
  printf ("%04d(NVB)%8d \n", frame_no, stats->bit_ctr_parametersets_n);
}

static void ReportFirstframe(time_t tmp_time, time_t me_time)
{
  if (params->Verbose == 1)
  {
    printf ("%04d(IDR)%8d   %2d %7.3f %7.3f %7.3f %9d %7d    %3s    %d\n",
      frame_no, (int) (stats->bit_ctr - stats->bit_ctr_n),
      img->AverageFrameQP, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], (int)tmp_time, (int)me_time,
      img->fld_flag ? "FLD" : "FRM", img->nal_reference_idc);
  }
  else if (params->Verbose == 2)
  {
    int lambda = (int) img->lambda_me[I_SLICE][img->AverageFrameQP][0];
    if (params->Distortion[SSIM] == 1)
      printf ("%04d(IDR)%8d %1d %2d %2d %7.3f %7.3f %7.3f %7.4f %7.4f %7.4f %9d %7d    %3s %5d   %2d %2d  %d   %d\n",
      frame_no,(int) (stats->bit_ctr - stats->bit_ctr_n),0,
      img->AverageFrameQP, lambda, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], dist->metric[SSIM].value[0], dist->metric[SSIM].value[1], dist->metric[SSIM].value[2], (int)tmp_time, (int)me_time,
      img->fld_flag ? "FLD" : "FRM", intras, img->num_ref_idx_l0_active, img->num_ref_idx_l1_active,img->rd_pass, img->nal_reference_idc);
    else
      printf ("%04d(IDR)%8d %1d %2d %2d %7.3f %7.3f %7.3f %9d %7d    %3s %5d   %2d %2d  %d   %d\n",
      frame_no,(int) (stats->bit_ctr - stats->bit_ctr_n),0,
      img->AverageFrameQP, lambda, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], (int)tmp_time, (int)me_time,
      img->fld_flag ? "FLD" : "FRM", intras, img->num_ref_idx_l0_active, img->num_ref_idx_l1_active,img->rd_pass, img->nal_reference_idc);
  }

  stats->bit_counter[I_SLICE] = stats->bit_ctr;
  stats->bit_ctr = 0;
}

static void ReportIntra(time_t tmp_time, time_t me_time)
{
  if (params->Verbose == 1)
  {
    if (img->currentPicture->idr_flag == TRUE)
      printf ("%04d(IDR)%8d   %2d %7.3f %7.3f %7.3f %9d %7d    %3s    %d\n",
      frame_no, (int) (stats->bit_ctr - stats->bit_ctr_n),
      img->AverageFrameQP, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], (int)tmp_time, (int)me_time,
      img->fld_flag ? "FLD" : "FRM", img->nal_reference_idc);
    else
      printf ("%04d(I)  %8d   %2d %7.3f %7.3f %7.3f %9d %7d    %3s    %d\n",
      frame_no, (int) (stats->bit_ctr - stats->bit_ctr_n),
      img->AverageFrameQP, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], (int)tmp_time, (int)me_time,
      img->fld_flag ? "FLD" : "FRM", img->nal_reference_idc);
  }
  else if (params->Verbose == 2)
  {
    int lambda = (int) img->lambda_me[I_SLICE][img->AverageFrameQP][0];
    if (params->Distortion[SSIM] == 1)
    {
      if (img->currentPicture->idr_flag == TRUE)
        printf ("%04d(IDR)%8d %1d %2d %2d %7.3f %7.3f %7.3f %7.4f %7.4f %7.4f %9d %7d    %3s %5d   %2d %2d  %d   %d\n",
        frame_no, (int) (stats->bit_ctr - stats->bit_ctr_n), 0,
        img->AverageFrameQP, lambda, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], dist->metric[SSIM].value[0], dist->metric[SSIM].value[1], dist->metric[SSIM].value[2], (int)tmp_time, (int)me_time,
        img->fld_flag ? "FLD" : "FRM", intras, 
        img->num_ref_idx_l0_active, img->num_ref_idx_l1_active,img->rd_pass, img->nal_reference_idc);
      else
        printf ("%04d(I)  %8d %1d %2d %2d %7.3f %7.3f %7.3f %7.4f %7.4f %7.4f %9d %7d    %3s %5d   %2d %2d  %d   %d\n",
        frame_no, (int) (stats->bit_ctr - stats->bit_ctr_n), 0,
        img->AverageFrameQP, lambda, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], dist->metric[SSIM].value[0], dist->metric[SSIM].value[1], dist->metric[SSIM].value[2], (int)tmp_time, (int)me_time,
        img->fld_flag ? "FLD" : "FRM", intras, img->num_ref_idx_l0_active, img->num_ref_idx_l1_active,img->rd_pass, img->nal_reference_idc);
    }
    else
    {
      if (img->currentPicture->idr_flag == TRUE)
        printf ("%04d(IDR)%8d %1d %2d %2d %7.3f %7.3f %7.3f %9d %7d    %3s %5d   %2d %2d  %d   %d\n",
        frame_no, (int) (stats->bit_ctr - stats->bit_ctr_n), 0,
        img->AverageFrameQP, lambda, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], (int)tmp_time, (int)me_time,
        img->fld_flag ? "FLD" : "FRM", intras, 
        img->num_ref_idx_l0_active, img->num_ref_idx_l1_active,img->rd_pass, img->nal_reference_idc);
      else
        printf ("%04d(I)  %8d %1d %2d %2d %7.3f %7.3f %7.3f %9d %7d    %3s %5d   %2d %2d  %d   %d\n",
        frame_no, (int) (stats->bit_ctr - stats->bit_ctr_n), 0,
        img->AverageFrameQP, lambda, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], (int)tmp_time, (int)me_time,
        img->fld_flag ? "FLD" : "FRM", intras, img->num_ref_idx_l0_active, img->num_ref_idx_l1_active,img->rd_pass, img->nal_reference_idc);
    }
  }
}

static void ReportSP(time_t tmp_time, time_t me_time)
{
  if (params->Verbose == 1)
  {
    printf ("%04d(SP) %8d   %2d %7.3f %7.3f %7.3f %9d %7d    %3s    %d\n",
      frame_no, (int) (stats->bit_ctr - stats->bit_ctr_n),
      img->AverageFrameQP, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], (int)tmp_time, (int)me_time,
      img->fld_flag ? "FLD" : "FRM", img->nal_reference_idc);
  }
  else if (params->Verbose == 2)
  {
    int lambda = (int) img->lambda_me[SP_SLICE][img->AverageFrameQP][0];    
    if (params->Distortion[SSIM] == 1)
      printf ("%04d(SP) %8d %1d %2d %2d %7.3f %7.3f %7.3f %7.4f %7.4f %7.4f %9d %7d    %3s %5d   %2d %2d  %d   %d\n",
      frame_no, (int) (stats->bit_ctr - stats->bit_ctr_n), active_pps->weighted_pred_flag,
      img->AverageFrameQP, lambda, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], dist->metric[SSIM].value[0], dist->metric[SSIM].value[1], dist->metric[SSIM].value[2], (int)tmp_time, (int)me_time,
      img->fld_flag ? "FLD" : "FRM", intras, img->num_ref_idx_l0_active, img->num_ref_idx_l1_active,img->rd_pass, img->nal_reference_idc);
    else
      printf ("%04d(SP) %8d %1d %2d %2d %7.3f %7.3f %7.3f %9d %7d    %3s %5d   %2d %2d  %d   %d\n",
      frame_no, (int) (stats->bit_ctr - stats->bit_ctr_n), active_pps->weighted_pred_flag,
      img->AverageFrameQP, lambda, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], (int)tmp_time, (int)me_time,
      img->fld_flag ? "FLD" : "FRM", intras, img->num_ref_idx_l0_active, img->num_ref_idx_l1_active,img->rd_pass, img->nal_reference_idc);
  }
}

static void ReportB(time_t tmp_time, time_t me_time)
{
  if (params->Verbose == 1)
  {
    printf ("%04d(B)  %8d   %2d %7.3f %7.3f %7.3f %9d %7d    %3s    %d\n",
      frame_no, (int) (stats->bit_ctr - stats->bit_ctr_n),
      img->AverageFrameQP, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], (int)tmp_time, (int)me_time,
      img->fld_flag ? "FLD" : "FRM", img->nal_reference_idc);
  }
  else if (params->Verbose == 2)
  {
    int lambda = (int) img->lambda_me[img->nal_reference_idc ? 5 : B_SLICE][img->AverageFrameQP][0];    
    if (params->Distortion[SSIM] == 1)
      printf ("%04d(B)  %8d %1d %2d %2d %7.3f %7.3f %7.3f %7.4f %7.4f %7.4f %9d %7d    %3s %5d %1d %2d %2d  %d   %d\n",
      frame_no, (int) (stats->bit_ctr - stats->bit_ctr_n), active_pps->weighted_bipred_idc,
      img->AverageFrameQP, lambda, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], dist->metric[SSIM].value[0], dist->metric[SSIM].value[1], dist->metric[SSIM].value[2], (int)tmp_time, (int)me_time,
      img->fld_flag ? "FLD" : "FRM", intras, img->direct_spatial_mv_pred_flag, img->num_ref_idx_l0_active, img->num_ref_idx_l1_active, img->rd_pass, img->nal_reference_idc);
    else
      printf ("%04d(B)  %8d %1d %2d %2d %7.3f %7.3f %7.3f %9d %7d    %3s %5d %1d %2d %2d  %d   %d\n",
      frame_no, (int) (stats->bit_ctr - stats->bit_ctr_n), active_pps->weighted_bipred_idc,
      img->AverageFrameQP, lambda, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], (int)tmp_time, (int)me_time,
      img->fld_flag ? "FLD" : "FRM", intras, img->direct_spatial_mv_pred_flag, img->num_ref_idx_l0_active, img->num_ref_idx_l1_active, img->rd_pass, img->nal_reference_idc);
  }


}


static void ReportP(time_t tmp_time, time_t me_time)
{
  if (params->Verbose == 1)
  {
    if(params->redundant_pic_flag==0)
    {
      printf ("%04d(P)  %8d   %2d %7.3f %7.3f %7.3f %9d %7d    %3s    %d\n",
        frame_no, (int) (stats->bit_ctr - stats->bit_ctr_n),
        img->AverageFrameQP, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], (int)tmp_time, (int)me_time,
        img->fld_flag ? "FLD" : "FRM", img->nal_reference_idc);
    }
    else
    {
      if(!redundant_coding)
      {
        printf ("%04d(P)  %8d   %2d %7.3f %7.3f %7.3f %9d %7d    %3s    %d\n",
          frame_no, (int) (stats->bit_ctr - stats->bit_ctr_n),
          img->AverageFrameQP, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], (int)tmp_time, (int)me_time,
          img->fld_flag ? "FLD" : "FRM", img->nal_reference_idc);
      }
      else
      { // report a redundant picture.
        printf ("    (R)  %8d   %2d %7.3f %7.3f %7.3f %9d %7d    %3s    %d\n",
          (int) (stats->bit_ctr - stats->bit_ctr_n),
          img->AverageFrameQP, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], (int)tmp_time, (int)me_time,
          img->fld_flag ? "FLD" : "FRM", img->nal_reference_idc);
      }
    }
  }
  else if (params->Verbose == 2)
  {
    int lambda = (int) img->lambda_me[P_SLICE][img->AverageFrameQP][0];    
    if (params->Distortion[SSIM] == 1)
      printf ("%04d(P)  %8d %1d %2d %2d %7.3f %7.3f %7.3f %7.4f %7.4f %7.4f %9d %7d    %3s %5d   %2d %2d  %d   %d\n",
      frame_no, (int) (stats->bit_ctr - stats->bit_ctr_n), active_pps->weighted_pred_flag,
      img->AverageFrameQP, lambda, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], dist->metric[SSIM].value[0], dist->metric[SSIM].value[1], dist->metric[SSIM].value[2], (int)tmp_time, (int)me_time,
      img->fld_flag ? "FLD" : "FRM", intras, img->num_ref_idx_l0_active, img->num_ref_idx_l1_active, img->rd_pass, img->nal_reference_idc);
    else
      printf ("%04d(P)  %8d %1d %2d %2d %7.3f %7.3f %7.3f %9d %7d    %3s %5d   %2d %2d  %d   %d\n",
      frame_no, (int) (stats->bit_ctr - stats->bit_ctr_n), active_pps->weighted_pred_flag,
      img->AverageFrameQP, lambda, dist->metric[PSNR].value[0], dist->metric[PSNR].value[1], dist->metric[PSNR].value[2], (int)tmp_time, (int)me_time,
      img->fld_flag ? "FLD" : "FRM", intras, img->num_ref_idx_l0_active, img->num_ref_idx_l1_active, img->rd_pass, img->nal_reference_idc);
  }
}

/*!
 ************************************************************************
 * \brief
 *    Padding of automatically added border for picture sizes that are not
 *     multiples of macroblock/macroblock pair size
 *
 * \param output
 *    Image dimensions
 * \param img_size_x
 *    coded image horizontal size (luma)
 * \param img_size_y
 *    code image vertical size (luma)
 * \param img_size_x_cr
 *    coded image horizontal size (chroma)
 * \param img_size_y_cr
 *    code image vertical size (chroma)
 ************************************************************************
 */
static void PaddAutoCropBorders (FrameFormat output, int img_size_x, int img_size_y, int img_size_x_cr, int img_size_y_cr)
{
  int x, y;

  // Luma or 1st component
  //padding right border
  if (output.width < img_size_x)
    for (y=0; y < output.height; y++)
      for (x = output.width; x < img_size_x; x++)
        pImgOrg[0] [y][x] = pImgOrg[0][y][x-1];

  //padding bottom border
  if (output.height < img_size_y)
    for (y = output.height; y<img_size_y; y++)
      memcpy(pImgOrg[0][y], pImgOrg[0][y - 1], img_size_x * sizeof(imgpel));

  // Chroma or all other components
  if (img->yuv_format != YUV400)
  {
    int k;

    for (k = 1; k < 3; k++)
    {
      //padding right border
      if (output.width_cr < img_size_x_cr)
        for (y=0; y < output.height_cr; y++)
          for (x = output.width_cr; x < img_size_x_cr; x++)
            pImgOrg [k][y][x] = pImgOrg[k][y][x-1];

      //padding bottom border
      if (output.height_cr < img_size_y_cr)
        for (y = output.height_cr; y<img_size_y_cr; y++)
          memcpy(pImgOrg[k][y], pImgOrg[k][y - 1], img_size_x_cr * sizeof(imgpel));
    }
  }
}



/*!
 ************************************************************************
 * \brief
 *    Calculates the absolute frame number in the source file out
 *    of various variables in img-> and params->
 * \return
 *    frame number in the file to be read
 * \par side effects
 *    global variable frame_no updated -- dunno, for what this one is necessary
 ************************************************************************
 */
int CalculateFrameNumber(void)
{
  int frm_sign = (IMG_NUMBER && (IMG_NUMBER <= params->intra_delay)) ? -1 : 1;
  int delay    = (IMG_NUMBER <= params->intra_delay) ? params->intra_delay : 0;
  
  if (img->b_frame_to_code)
  {
    if ((IMG_NUMBER && (IMG_NUMBER <= params->intra_delay)))
    {
      if (params->HierarchicalCoding)
        frame_no = start_tr_in_this_IGOP + (params->intra_delay - IMG_NUMBER) * (params->jumpd + 1) + (int) (img->b_interval * (double) (1 + gop_structure[img->b_frame_to_code - 1].display_no));
      else
        frame_no = start_tr_in_this_IGOP + (params->intra_delay - IMG_NUMBER) * (params->jumpd + 1) + (int) (img->b_interval * (double) img->b_frame_to_code);
    }
    else
    {
      if (params->HierarchicalCoding)
        frame_no = start_tr_in_this_IGOP + (IMG_NUMBER - 1) * (params->jumpd + 1) + (int) (img->b_interval * (double) (1 + gop_structure[img->b_frame_to_code - 1].display_no));
      else
        frame_no = start_tr_in_this_IGOP + (IMG_NUMBER - 1) * (params->jumpd + 1) + (int) (img->b_interval * (double) img->b_frame_to_code);
    }
    //printf("frame_no %d %d %d\n",frame_no,IMG_NUMBER - 1,(frm_sign * (IMG_NUMBER - 1) + params->intra_delay));
  }
  else
  {
    if (params->idr_period && params->EnableIDRGOP && img->frm_number && ((!params->adaptive_idr_period && (img->frm_number - img->lastIDRnumber ) % params->idr_period == 0)
      || (params->adaptive_idr_period == 1 && (img->frm_number - imax(img->lastIntraNumber, img->lastIDRnumber) ) % params->idr_period == 0)) )
    {
      delay = params->intra_delay;
      img->rewind_frame += params->jumpd;
      start_frame_no_in_this_IGOP = img->frm_number;
      start_tr_in_this_IGOP = (img->frm_number) * (params->jumpd + 1) - img->rewind_frame;      
    }

    frame_no = start_tr_in_this_IGOP + (frm_sign * IMG_NUMBER + delay) * (params->jumpd + 1);     
#ifdef _ADAPT_LAST_GROUP_
    if (params->last_frame && (img->frm_number + 1) == params->no_frames)
      frame_no = params->last_frame;
#endif
  }

  return frame_no;
}




/*!
 ************************************************************************
 * \brief
 *    point to frame coding variables
 ************************************************************************
 */
static void put_buffer_frame(ImageParameters *img)
{
  pCurImg    = img_org_frm[0];
  pImgOrg[0] = img_org_frm[0];    
  if (img->yuv_format != YUV400)
  {
    pImgOrg[1] = img_org_frm[1];
    pImgOrg[2] = img_org_frm[2];
  }
}

/*!
 ************************************************************************
 * \brief
 *    point to top field coding variables
 ************************************************************************
 */
static void put_buffer_top(ImageParameters *img)
{
  img->fld_type = 0;

  pCurImg    = img_org_top[0];
  pImgOrg[0] = img_org_top[0];
  if (img->yuv_format != YUV400)
  {
    pImgOrg[1] = img_org_top[1];
    pImgOrg[2] = img_org_top[2];
  }
}

/*!
 ************************************************************************
 * \brief
 *    point to bottom field coding variables
 ************************************************************************
 */
static void put_buffer_bot(ImageParameters *img)
{
  img->fld_type = 1;

  pCurImg    = img_org_bot[0];  
  pImgOrg[0] = img_org_bot[0];
  if (img->yuv_format != YUV400)
  {
    pImgOrg[1] = img_org_bot[1];
    pImgOrg[2] = img_org_bot[2];
  }
}

/*!
 ************************************************************************
 * \brief
 *    performs multi-pass encoding of same picture using different
 *    coding conditions
 ************************************************************************
 */

static void rdPictureCoding(void)
{
  int   second_qp = img->qp, rd_qp = img->qp;
  int   previntras = intras;
  int   prevtype = img->type;
  int   skip_encode = 0;
  pic_parameter_set_rbsp_t *sec_pps;
  int   tmpFrameQP = img->SumFrameQP;
  float rateRatio = 1.0F;

  if ( params->RCEnable )
    rc_save_state();
  if ( params->WPMCPrecision )
    pWPX->curr_wp_rd_pass = pWPX->wp_rd_passes + 1;

  if (img->type!=I_SLICE && params->GenerateMultiplePPS)
  {
    if (img->type==P_SLICE)
    {
      if ((params->RDPSliceWeightOnly != 2) && (TestWPPSlice(img, params, 0) == 1))
      {
        active_pps = PicParSet[1];
        if ( params->WPMCPrecision )
          pWPX->curr_wp_rd_pass->algorithm = WP_REGULAR;
      }
      else if ( params->WPMCPrecision )
        active_pps = PicParSet[1];
      else
      {
        skip_encode = params->RDPSliceWeightOnly;
        active_pps = PicParSet[0];
        if (!img->AdaptiveRounding)
        {
          img->qp-=1;
          if ( params->RCEnable )
            rateRatio = 1.15F;
        }
      }
    }
    else
    {
      active_pps = PicParSet[2];
      if ( params->WPMCPrecision )
        pWPX->curr_wp_rd_pass->algorithm = WP_REGULAR;
    }
  }
  else
  {
    if (!img->AdaptiveRounding)
    {
      img->qp-=1;
      if ( params->RCEnable )
        rateRatio = 1.15F;
    }
  }

  sec_pps = active_pps;
  second_qp = img->qp;

  img->write_macroblock = FALSE;

  if (skip_encode)
  {
    img->rd_pass = 0;
    enc_frame_picture[1] = NULL;
  }
  else
  {
    if(params->RCEnable)
      rc_init_frame_rdpic( rateRatio );

    img->qp = iClip3( img->RCMinQP, img->RCMaxQP, img->qp );
    frame_picture (frame_pic[1],1);
    img->rd_pass=picture_coding_decision(frame_pic[0], frame_pic[1], rd_qp);
  }

  //      update_rd_picture_contexts (img->rd_pass);
  if (img->rd_pass==0)
  {
    enc_picture=enc_frame_picture[0];
    if (img->type!=I_SLICE && params->GenerateMultiplePPS)
    {
      img->qp = rd_qp;
      active_pps = PicParSet[0];
    }
    else
    {
      img->qp = rd_qp;
    }
    intras = previntras;
    p_frame_pic = frame_pic[0];
  }
  else
  {
    previntras  = intras;
    p_frame_pic = frame_pic[1];
    tmpFrameQP  = img->SumFrameQP;

    if(params->RCEnable)
      rc_save_state();
  }
  // Final Encoding pass - note that we should
  // make this more flexible in a later version.
  if ( params->RCEnable )
    rateRatio = 1.0F;

  if ( params->WPMCPrecision )
    pWPX->curr_wp_rd_pass = pWPX->wp_rd_passes + 2;
  if (img->type != I_SLICE )
  {
    skip_encode = 0;
    img->qp    = rd_qp;

    if (img->type == P_SLICE && (intras * 100 )/img->FrameSizeInMbs >=75)
    {
      set_slice_type( I_SLICE );
      active_pps = PicParSet[0];
    }
    else if (img->type==P_SLICE)
    {
      if (params->GenerateMultiplePPS)
      {
        if ((params->RDPSliceWeightOnly != 2) && (TestWPPSlice(img, params, 0) == 1))
        {
          active_pps = PicParSet[1];
          if ( params->WPMCPrecision )
            pWPX->curr_wp_rd_pass->algorithm = WP_REGULAR;
        }
        else if ( params->WPMCPrecision == 2 )
          active_pps = PicParSet[1];
        else if (params->RDPSliceBTest && active_sps->profile_idc != 66)
        {
          set_slice_type( B_SLICE );
          active_pps = PicParSet[0];
        }
        else
        {
          skip_encode = params->RDPSliceWeightOnly;
          active_pps = PicParSet[0];
          if (!img->AdaptiveRounding)
          {
            img->qp+=1;
            if ( params->RCEnable )
              rateRatio = 0.85F;
          }
        }
      }
    }
    else
    {
      if (params->GenerateMultiplePPS && (params->RDBSliceWeightOnly != 2) && TestWPBSlice(img, params, 0) == 1)
      {
        active_pps = PicParSet[1];
        if ( params->WPMCPrecision )
          pWPX->curr_wp_rd_pass->algorithm = WP_REGULAR;
      }
      else if ( params->WPMCPrecision == 2 && (params->WPMCPrecBSlice == 2 || (params->WPMCPrecBSlice == 1 && img->nal_reference_idc) ) )
        active_pps = PicParSet[1];
      else
      {
        skip_encode = (params->RDBSliceWeightOnly == 1);
        img->qp = rd_qp + (img->nal_reference_idc ? - 1 : 1);
        if ( params->RCEnable )
          rateRatio = img->nal_reference_idc ? 1.15F : 0.85F;
        if ( params->WPMCPrecision )
          pWPX->curr_wp_rd_pass->algorithm = WP_REGULAR;
      }
    }
  }
  else
  {
    active_pps = PicParSet[0];
    if (!img->AdaptiveRounding)
      img->qp    = (rd_qp + 1);
  }

  img->write_macroblock = FALSE;

  if (skip_encode)
  {
    enc_frame_picture[2] = NULL;
    img->qp = rd_qp;
  }
  else
  {
    if(params->RCEnable)
      rc_init_frame_rdpic( rateRatio );

    img->qp = iClip3( img->RCMinQP, img->RCMaxQP, img->qp );
    frame_picture (frame_pic[2],2);

    if (img->rd_pass==0)
      img->rd_pass  = 2 * picture_coding_decision(frame_pic[0], frame_pic[2], rd_qp);
    else
      img->rd_pass +=     picture_coding_decision(frame_pic[1], frame_pic[2], rd_qp);

    if ( params->RCEnable && img->rd_pass == 2 )
      rc_save_state();

    if ( img->rd_pass == 2 )
      tmpFrameQP = img->SumFrameQP;
  }

  //update_rd_picture_contexts (img->rd_pass);
  if (img->rd_pass==0)
  {
    enc_picture = enc_frame_picture[0];
    set_slice_type( prevtype );
    active_pps  = PicParSet[0];
    img->qp     = rd_qp;
    intras      = previntras;
  }
  else if (img->rd_pass==1)
  {
    enc_picture = enc_frame_picture[1];
    set_slice_type( prevtype );
    active_pps  = sec_pps;
    img->qp     = second_qp;
    intras      = previntras;
  }
  if ( params->RCEnable )
    rc_restore_state();
  img->SumFrameQP = tmpFrameQP;
}

/*!
*************************************************************************************
* Brief
*     Output SP frames coefficients
*************************************************************************************
*/
void output_SP_coefficients()
{
  int i,k;
  FILE *SP_coeff_file;
  if(number_sp2_frames==0)
  {
    if ((SP_coeff_file = fopen(params->sp_output_filename,"wb")) == NULL)
    {
      printf ("Fatal: cannot open SP output file '%s', exit (-1)\n", params->sp_output_filename);
      exit (-1);
    }
    number_sp2_frames++;
  }
  else
  {
    if ((SP_coeff_file = fopen(params->sp_output_filename,"ab")) == NULL)
    {
      printf ("Fatal: cannot open SP output file '%s', exit (-1)\n", params->sp_output_filename);
      exit (-1);
    }
  }

  for(i=0;i<img->height;i++)
  {
    fwrite(lrec[i],sizeof(int),img->width,SP_coeff_file);
  }
  for(k=0;k<2;k++)
  {
    for(i=0;i<img->height_cr;i++)
    {
      fwrite(lrec_uv[k][i],sizeof(int),img->width_cr,SP_coeff_file);
    }
  }
  fclose(SP_coeff_file);
}

/*!
*************************************************************************************
* Brief
*     Read SP frames coefficients
*************************************************************************************
*/
void read_SP_coefficients()
{
  int i,k;
  FILE *SP_coeff_file;

  if ( (params->qp2start > 0) && ( ( (img->tr ) % (2*params->qp2start) ) >=params->qp2start ))
  {
    if ((SP_coeff_file = fopen(params->sp2_input_filename1,"rb")) == NULL)
    {
      printf ("Fatal: cannot open SP input file '%s', exit (-1)\n", params->sp2_input_filename2);
      exit (-1);
    }
  }
  else
  {
    if ((SP_coeff_file = fopen(params->sp2_input_filename2,"rb")) == NULL)
    {
      printf ("Fatal: cannot open SP input file '%s', exit (-1)\n", params->sp2_input_filename1);
      exit (-1);
    }
  }

  if (0 != fseek (SP_coeff_file, img->size * 3/2*number_sp2_frames*sizeof(int), SEEK_SET))
  {
    printf ("Fatal: cannot seek in SP input file, exit (-1)\n");
    exit (-1);
  }
  number_sp2_frames++;

  for(i=0;i<img->height;i++)
  {
    if(img->width!=(int)fread(lrec[i],sizeof(int),img->width,SP_coeff_file))
    {
      printf ("Fatal: cannot read in SP input file, exit (-1)\n");
      exit (-1);
    }
  }

  for(k=0;k<2;k++)
  {
    for(i=0;i<img->height_cr;i++)
    {
      if(img->width_cr!=(int)fread(lrec_uv[k][i],sizeof(int),img->width_cr,SP_coeff_file))
      {
        printf ("Fatal: cannot read in SP input file, exit (-1)\n");
        exit (-1);
      }
    }
  }
  fclose(SP_coeff_file);
}


/*!
*************************************************************************************
* Brief
*     Select appropriate image plane (for 444 coding)
*************************************************************************************
*/
void select_plane(ColorPlane color_plane)
{
  pCurImg                     = pImgOrg[color_plane];
  enc_picture->p_curr_img     = enc_picture->p_img[color_plane];
  enc_picture->p_curr_img_sub = enc_picture->p_img_sub[color_plane];
  img->max_imgpel_value       = img->max_imgpel_value_comp[color_plane];
  img->dc_pred_value          = img->dc_pred_value_comp[color_plane];
  img->curr_res               = img->mb_rres [color_plane];
  img->curr_prd               = img->mb_pred [color_plane];
}

