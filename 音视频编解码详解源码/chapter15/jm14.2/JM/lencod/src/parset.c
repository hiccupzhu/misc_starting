
/*!
 **************************************************************************************
 * \file
 *    parset.c
 * \brief
 *    Picture and Sequence Parameter set generation and handling
 *  \date 25 November 2002
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Stephan Wenger        <stewe@cs.tu-berlin.de>
 *
 **************************************************************************************
 */

#include <time.h>
#include <sys/timeb.h>

#include "global.h"

#include "contributors.h"
#include "nal.h"
#include "mbuffer.h"
#include "parset.h"
#include "vlc.h"
#include "q_matrix.h"

// Local helpers
static int IdentifyProfile(void);
static int IdentifyLevel(void);
static int GenerateVUI_parameters_rbsp(seq_parameter_set_rbsp_t *sps, Bitstream *bitstream);

extern ColocatedParams *Co_located;
extern ColocatedParams *Co_located_JV[MAX_PLANE];  //!< Co_located to be used during 4:4:4 independent mode encoding

pic_parameter_set_rbsp_t *PicParSet[MAXPPS];

static const byte ZZ_SCAN[16]  =
{  0,  1,  4,  8,  5,  2,  3,  6,  9, 12, 13, 10,  7, 11, 14, 15
};

static const byte ZZ_SCAN8[64] =
{  0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};


/*!
 *************************************************************************************
 * \brief
 *    generates a sequence and picture parameter set and stores these in global
 *    active_sps and active_pps
 *
 * \return
 *    A NALU containing the Sequence ParameterSet
 *
 *************************************************************************************
*/
void GenerateParameterSets (void)
{
  int i;
  seq_parameter_set_rbsp_t *sps = NULL;

  sps = AllocSPS();

  for (i=0; i<MAXPPS; i++)
  {
    PicParSet[i] = NULL;
  }


  GenerateSequenceParameterSet(sps, 0);

  if (params->GenerateMultiplePPS)
  {
    PicParSet[0] = AllocPPS();
    PicParSet[1] = AllocPPS();
    PicParSet[2] = AllocPPS();

    if (IS_FREXT_PROFILE(sps->profile_idc))
    {
      GeneratePictureParameterSet( PicParSet[0], sps, 0, 0, 0, params->cb_qp_index_offset, params->cr_qp_index_offset);
      GeneratePictureParameterSet( PicParSet[1], sps, 1, 1, 1, params->cb_qp_index_offset, params->cr_qp_index_offset);
      GeneratePictureParameterSet( PicParSet[2], sps, 2, 1, 2, params->cb_qp_index_offset, params->cr_qp_index_offset);

    }
    else
    {
      GeneratePictureParameterSet( PicParSet[0], sps, 0, 0, 0, params->chroma_qp_index_offset, 0);
      GeneratePictureParameterSet( PicParSet[1], sps, 1, 1, 1, params->chroma_qp_index_offset, 0);
      GeneratePictureParameterSet( PicParSet[2], sps, 2, 1, 2, params->chroma_qp_index_offset, 0);
    }
  }
  else
  {
    PicParSet[0] = AllocPPS();
    if (IS_FREXT_PROFILE(sps->profile_idc))
      GeneratePictureParameterSet( PicParSet[0], sps, 0, params->WeightedPrediction, params->WeightedBiprediction,
                                   params->cb_qp_index_offset, params->cr_qp_index_offset);
    else
      GeneratePictureParameterSet( PicParSet[0], sps, 0, params->WeightedPrediction, params->WeightedBiprediction,
                                   params->chroma_qp_index_offset, 0);
  }

  active_sps = sps;
  active_pps = PicParSet[0];
}

/*!
*************************************************************************************
* \brief
*    frees global parameter sets active_sps and active_pps
*
* \return
*    A NALU containing the Sequence ParameterSet
*
*************************************************************************************
*/
void FreeParameterSets (void)
{
  int i;
  for (i=0; i<MAXPPS; i++)
  {
    if ( NULL != PicParSet[i])
    {
      FreePPS(PicParSet[i]);
      PicParSet[i] = NULL;
    }
  }
  FreeSPS (active_sps);
}

/*!
*************************************************************************************
* \brief
*    int GenerateSeq_parameter_set_NALU (void);
*
* \note
*    Uses the global variables through GenerateSequenceParameterSet()
*    and GeneratePictureParameterSet
*
* \return
*    A NALU containing the Sequence ParameterSet
*
*************************************************************************************
*/

NALU_t *GenerateSeq_parameter_set_NALU (void)
{
  NALU_t *n = AllocNALU(64000);
  int RBSPlen = 0;
  int NALUlen;
  byte rbsp[MAXRBSPSIZE];

  RBSPlen = GenerateSeq_parameter_set_rbsp (active_sps, rbsp);
  NALUlen = RBSPtoNALU (rbsp, n, RBSPlen, NALU_TYPE_SPS, NALU_PRIORITY_HIGHEST, 0, 1);
  n->startcodeprefix_len = 4;

  return n;
}


/*!
*************************************************************************************
* \brief
*    NALU_t *GeneratePic_parameter_set_NALU (int PPS_id);
*
* \note
*    Uses the global variables through GenerateSequenceParameterSet()
*    and GeneratePictureParameterSet
*
* \return
*    A NALU containing the Picture Parameter Set
*
*************************************************************************************
*/

NALU_t *GeneratePic_parameter_set_NALU(int PPS_id)
{
  NALU_t *n = AllocNALU(64000);
  int RBSPlen = 0;
  int NALUlen;
  byte rbsp[MAXRBSPSIZE];

  RBSPlen = GeneratePic_parameter_set_rbsp (PicParSet[PPS_id], rbsp);
  NALUlen = RBSPtoNALU (rbsp, n, RBSPlen, NALU_TYPE_PPS, NALU_PRIORITY_HIGHEST, 0, 1);
  n->startcodeprefix_len = 4;

  return n;
}


/*!
 ************************************************************************
 * \brief
 *    GenerateSequenceParameterSet: extracts info from global variables and
 *    generates sequence parameter set structure
 *
 * \par
 *    Function reads all kinds of values from several global variables,
 *    including params-> and image-> and fills in the sps.  Many
 *    values are current hard-coded to defaults.
 *
 ************************************************************************
 */

void GenerateSequenceParameterSet( seq_parameter_set_rbsp_t *sps, //!< Sequence Parameter Set to be filled
                                   int SPS_id                     //!< SPS ID
                                   )
{
  unsigned i;
  unsigned n_ScalingList;
  int SubWidthC  [4]= { 1, 2, 2, 1};
  int SubHeightC [4]= { 1, 2, 1, 1};

  int frext_profile = ((IdentifyProfile()==FREXT_HP) ||
                      (IdentifyProfile()==FREXT_Hi10P) ||
                      (IdentifyProfile()==FREXT_Hi422) ||
                      (IdentifyProfile()==FREXT_Hi444) ||
                      (IdentifyProfile()==FREXT_CAVLC444));

  // *************************************************************************
  // Sequence Parameter Set
  // *************************************************************************
  assert (sps != NULL);
  // Profile and Level should be calculated using the info from the config
  // file.  Calculation is hidden in IndetifyProfile() and IdentifyLevel()
  sps->profile_idc = IdentifyProfile();
  sps->level_idc = IdentifyLevel();

  // needs to be set according to profile
  sps->constrained_set0_flag = FALSE;
  sps->constrained_set1_flag = FALSE;
  sps->constrained_set2_flag = FALSE;

  if ( (sps->level_idc == 9) && !IS_FREXT_PROFILE(sps->profile_idc) ) // Level 1.b
  {
    sps->constrained_set3_flag = TRUE;
    sps->level_idc = 11;
  }
  else if (frext_profile && params->IntraProfile)
  {
    sps->constrained_set3_flag = TRUE;
  }
  else
  {
    sps->constrained_set3_flag = FALSE;
  }

  // Parameter Set ID hard coded to zero
  sps->seq_parameter_set_id = SPS_id;

  // Fidelity Range Extensions stuff
  sps->bit_depth_luma_minus8   = params->output.bit_depth[0] - 8;
  sps->bit_depth_chroma_minus8 = params->output.bit_depth[1] - 8;
  img->lossless_qpprime_flag = params->lossless_qpprime_y_zero_flag & 
      (sps->profile_idc==FREXT_Hi444 || sps->profile_idc==FREXT_CAVLC444);

  //! POC stuff:
  //! The following values are hard-coded in init_poc().  Apparently,
  //! the poc implementation covers only a subset of the poc functionality.
  //! Here, the same subset is implemented.  Changes in the POC stuff have
  //! also to be reflected here
  sps->log2_max_frame_num_minus4 = log2_max_frame_num_minus4;
  sps->log2_max_pic_order_cnt_lsb_minus4 = log2_max_pic_order_cnt_lsb_minus4;

  sps->pic_order_cnt_type = params->pic_order_cnt_type;
  sps->num_ref_frames_in_pic_order_cnt_cycle = img->num_ref_frames_in_pic_order_cnt_cycle;
  sps->delta_pic_order_always_zero_flag = img->delta_pic_order_always_zero_flag;
  sps->offset_for_non_ref_pic = img->offset_for_non_ref_pic;
  sps->offset_for_top_to_bottom_field = img->offset_for_top_to_bottom_field;

  for (i=0; i<img->num_ref_frames_in_pic_order_cnt_cycle; i++)
  {
    sps->offset_for_ref_frame[i] = img->offset_for_ref_frame[i];
  }
  // End of POC stuff

  // Number of Reference Frames
  sps->num_ref_frames = params->num_ref_frames;

  //required_frame_num_update_behaviour_flag hardcoded to zero
  sps->gaps_in_frame_num_value_allowed_flag = FALSE;    // double check

  sps->frame_mbs_only_flag = (Boolean) !(params->PicInterlace || params->MbInterlace);

  // Picture size, finally a simple one :-)
  sps->pic_width_in_mbs_minus1        = (( params->output.width  + img->auto_crop_right)/16) -1;
  sps->pic_height_in_map_units_minus1 = (((params->output.height + img->auto_crop_bottom)/16)/ (2 - sps->frame_mbs_only_flag)) - 1;

  // a couple of flags, simple
  sps->mb_adaptive_frame_field_flag = (Boolean) (FRAME_CODING != params->MbInterlace);
  sps->direct_8x8_inference_flag = (Boolean) params->directInferenceFlag;

  // Sequence VUI not implemented, signalled as not present
  sps->vui_parameters_present_flag = (Boolean) ((params->rgb_input_flag && params->yuv_format==3) || params->EnableVUISupport);

  sps->chroma_format_idc = params->yuv_format;
  sps->separate_colour_plane_flag = ( sps->chroma_format_idc == YUV444 ) ? params->separate_colour_plane_flag : 0;

  if ( sps->vui_parameters_present_flag )
    GenerateVUIParameters(sps);

  // This should be moved somewhere else.
  {
    int PicWidthInMbs, PicHeightInMapUnits, FrameHeightInMbs;
    int width, height;
    int nplane;
    PicWidthInMbs = (sps->pic_width_in_mbs_minus1 +1);
    PicHeightInMapUnits = (sps->pic_height_in_map_units_minus1 +1);
    FrameHeightInMbs = ( 2 - sps->frame_mbs_only_flag ) * PicHeightInMapUnits;

    width = PicWidthInMbs * MB_BLOCK_SIZE;
    height = FrameHeightInMbs * MB_BLOCK_SIZE;

    if( IS_INDEPENDENT(params) )
    {
      for( nplane=0; nplane<MAX_PLANE; nplane++ )
      {
        Co_located_JV[nplane] = alloc_colocated (width, height,sps->mb_adaptive_frame_field_flag);           
      }
    }
    else
    {
      Co_located = alloc_colocated (width, height,sps->mb_adaptive_frame_field_flag);
    }
  }

  // Fidelity Range Extensions stuff
  if(frext_profile)
  {

    sps->seq_scaling_matrix_present_flag = (Boolean) (params->ScalingMatrixPresentFlag&1);
    n_ScalingList = (sps->chroma_format_idc != YUV444) ? 8 : 12;
    for(i=0; i<n_ScalingList; i++)
    {
      if(i<6)
        sps->seq_scaling_list_present_flag[i] = (params->ScalingListPresentFlag[i]&1);
      else
      {
        if(params->Transform8x8Mode)
          sps->seq_scaling_list_present_flag[i] = (params->ScalingListPresentFlag[i]&1);
        else
          sps->seq_scaling_list_present_flag[i] = 0;
      }
      if( sps->seq_scaling_matrix_present_flag == FALSE )
        sps->seq_scaling_list_present_flag[i] = 0;
    }
  }
  else
  {
    sps->seq_scaling_matrix_present_flag = FALSE;
    for(i=0; i<12; i++)
      sps->seq_scaling_list_present_flag[i] = 0;

  }


  if (img->auto_crop_right || img->auto_crop_bottom)
  {
    sps->frame_cropping_flag = TRUE;
    sps->frame_cropping_rect_left_offset=0;
    sps->frame_cropping_rect_top_offset=0;
    sps->frame_cropping_rect_right_offset=  (img->auto_crop_right / SubWidthC[sps->chroma_format_idc]);
    sps->frame_cropping_rect_bottom_offset= (img->auto_crop_bottom / (SubHeightC[sps->chroma_format_idc] * (2 - sps->frame_mbs_only_flag)));
    if (img->auto_crop_right % SubWidthC[sps->chroma_format_idc])
    {
      error("automatic frame cropping (width) not possible",500);
    }
    if (img->auto_crop_bottom % (SubHeightC[sps->chroma_format_idc] * (2 - sps->frame_mbs_only_flag)))
    {
      error("automatic frame cropping (height) not possible",500);
    }
  }
  else
  {
    sps->frame_cropping_flag = FALSE;
  }

}

/*!
 ************************************************************************
 * \brief
 *    GeneratePictureParameterSet:
 *    Generates a Picture Parameter Set structure
 *
 * \par
 *    Regarding the QP
 *    The previous software versions coded the absolute QP only in the
 *    slice header.  This is kept, and the offset in the PPS is coded
 *    even if we could save bits by intelligently using this field.
 *
 ************************************************************************
 */

void GeneratePictureParameterSet( pic_parameter_set_rbsp_t *pps, //!< Picture Parameter Set to be filled
                                  seq_parameter_set_rbsp_t *sps, //!< used Sequence Parameter Set
                                  int PPS_id,                    //!< PPS ID
                                  int WeightedPrediction,        //!< value of weighted_pred_flag
                                  int WeightedBiprediction,      //!< value of weighted_bipred_idc
                                  int cb_qp_index_offset,        //!< value of cb_qp_index_offset
                                  int cr_qp_index_offset         //!< value of cr_qp_index_offset
                                  )
{
  unsigned i;
  unsigned n_ScalingList;

  int frext_profile = ((IdentifyProfile()==FREXT_HP) ||
                      (IdentifyProfile()==FREXT_Hi10P) ||
                      (IdentifyProfile()==FREXT_Hi422) ||
                      (IdentifyProfile()==FREXT_Hi444) ||
                      (IdentifyProfile()==FREXT_CAVLC444));

  // *************************************************************************
  // Picture Parameter Set
  // *************************************************************************

  pps->seq_parameter_set_id = sps->seq_parameter_set_id;
  pps->pic_parameter_set_id = PPS_id;
  pps->entropy_coding_mode_flag = (params->symbol_mode == CAVLC ? FALSE : TRUE);

  // Fidelity Range Extensions stuff
  if(frext_profile)
  {
    pps->transform_8x8_mode_flag = (params->Transform8x8Mode ? TRUE:FALSE);
    pps->pic_scaling_matrix_present_flag = (Boolean) ((params->ScalingMatrixPresentFlag&2)>>1);
    n_ScalingList = (sps->chroma_format_idc != YUV444) ? 8 : 12;
    for(i=0; i<n_ScalingList; i++)
    {
      if(i<6)
        pps->pic_scaling_list_present_flag[i] = (params->ScalingListPresentFlag[i]&2)>>1;
      else
      {
        if(pps->transform_8x8_mode_flag)
          pps->pic_scaling_list_present_flag[i] = (params->ScalingListPresentFlag[i]&2)>>1;
        else
          pps->pic_scaling_list_present_flag[i] = 0;
      }
      if( pps->pic_scaling_matrix_present_flag == FALSE )
        pps->pic_scaling_list_present_flag[i] = 0;
    }
  }
  else
  {
    pps->pic_scaling_matrix_present_flag = FALSE;
    for(i=0; i<12; i++)
      pps->pic_scaling_list_present_flag[i] = 0;

    pps->transform_8x8_mode_flag = FALSE;
    params->Transform8x8Mode = 0;
  }

  // JVT-Fxxx (by Stephan Wenger, make this flag unconditional
  pps->pic_order_present_flag = img->pic_order_present_flag;


  // Begin FMO stuff
  pps->num_slice_groups_minus1 = params->num_slice_groups_minus1;


  //! Following set the parameter for different slice group types
  if (pps->num_slice_groups_minus1 > 0)
  {
     if ((pps->slice_group_id = calloc ((sps->pic_height_in_map_units_minus1+1)*(sps->pic_width_in_mbs_minus1+1), sizeof(byte))) == NULL)
       no_mem_exit ("GeneratePictureParameterSet: slice_group_id");

    switch (params->slice_group_map_type)
    {
    case 0:
      pps->slice_group_map_type = 0;
      for(i=0; i<=pps->num_slice_groups_minus1; i++)
      {
        pps->run_length_minus1[i]=params->run_length_minus1[i];
      }
      break;
    case 1:
      pps->slice_group_map_type = 1;
      break;
    case 2:
      // i loops from 0 to num_slice_groups_minus1-1, because no info for background needed
      pps->slice_group_map_type = 2;
      for(i=0; i<pps->num_slice_groups_minus1; i++)
      {
        pps->top_left[i] = params->top_left[i];
        pps->bottom_right[i] = params->bottom_right[i];
      }
     break;
    case 3:
    case 4:
    case 5:
      pps->slice_group_map_type = params->slice_group_map_type;
      pps->slice_group_change_direction_flag = (Boolean) params->slice_group_change_direction_flag;
      pps->slice_group_change_rate_minus1 = params->slice_group_change_rate_minus1;
      break;
    case 6:
      pps->slice_group_map_type = 6;
      pps->pic_size_in_map_units_minus1 =
        (((params->output.height + img->auto_crop_bottom)/MB_BLOCK_SIZE)/(2-sps->frame_mbs_only_flag))
        *((params->output.width  + img->auto_crop_right)/MB_BLOCK_SIZE) -1;

      for (i=0;i<=pps->pic_size_in_map_units_minus1; i++)
        pps->slice_group_id[i] = params->slice_group_id[i];

      break;
    default:
      printf ("Parset.c: slice_group_map_type invalid, default\n");
      assert (0==1);
    }
  }
// End FMO stuff

  pps->num_ref_idx_l0_active_minus1 = sps->frame_mbs_only_flag ? (sps->num_ref_frames-1) : (2 * sps->num_ref_frames - 1) ;   // set defaults
  pps->num_ref_idx_l1_active_minus1 = sps->frame_mbs_only_flag ? (sps->num_ref_frames-1) : (2 * sps->num_ref_frames - 1) ;   // set defaults

  pps->weighted_pred_flag = (Boolean) WeightedPrediction;
  pps->weighted_bipred_idc = WeightedBiprediction;

  pps->pic_init_qp_minus26 = 0;         // hard coded to zero, QP lives in the slice header
  pps->pic_init_qs_minus26 = 0;

  pps->chroma_qp_index_offset = cb_qp_index_offset;
  if (frext_profile)
  {
    pps->cb_qp_index_offset     = cb_qp_index_offset;
    pps->cr_qp_index_offset     = cr_qp_index_offset;
  }
  else
    pps->cb_qp_index_offset = pps->cr_qp_index_offset = pps->chroma_qp_index_offset;

  pps->deblocking_filter_control_present_flag = (Boolean) params->DFSendParameters;
  pps->constrained_intra_pred_flag = (Boolean) params->UseConstrainedIntraPred;

  // if redundant slice is in use.
  pps->redundant_pic_cnt_present_flag = (Boolean) params->redundant_pic_flag;
}

/*!
 *************************************************************************************
 * \brief
 *    syntax for scaling list matrix values
 *
 * \param scalingListinput
 *    input scaling list
 * \param scalingList
 *    scaling list to be used
 * \param sizeOfScalingList
 *    size of the scaling list
 * \param UseDefaultScalingMatrix
 *    usage of default Scaling Matrix
 * \param bitstream
 *    target bitstream for writing syntax
 *
 * \return
 *    size of the RBSP in bytes
 *
 *************************************************************************************
 */
int Scaling_List(short *scalingListinput, short *scalingList, int sizeOfScalingList, short *UseDefaultScalingMatrix, Bitstream *bitstream)
{
  int j, scanj;
  int len=0;
  int delta_scale, lastScale, nextScale;

  lastScale = 8;
  nextScale = 8;

  for(j=0; j<sizeOfScalingList; j++)
  {
    scanj = (sizeOfScalingList==16) ? ZZ_SCAN[j]:ZZ_SCAN8[j];

    if(nextScale!=0)
    {
      delta_scale = scalingListinput[scanj]-lastScale; // Calculate delta from the scalingList data from the input file
      if(delta_scale>127)
        delta_scale=delta_scale-256;
      else if(delta_scale<-128)
        delta_scale=delta_scale+256;

      len+=se_v ("   : delta_sl   ",                      delta_scale,                       bitstream);
      nextScale = scalingListinput[scanj];
      *UseDefaultScalingMatrix|=(scanj==0 && nextScale==0); // Check first matrix value for zero
    }

    scalingList[scanj] = (short) ((nextScale==0) ? lastScale:nextScale); // Update the actual scalingList matrix with the correct values
    lastScale = scalingList[scanj];
  }

  return len;
}


/*!
 *************************************************************************************
 * \brief
 *    int GenerateSeq_parameter_set_rbsp (seq_parameter_set_rbsp_t *sps, char *rbsp);
 *
 * \param sps
 *    sequence parameter structure
 * \param rbsp
 *    buffer to be filled with the rbsp, size should be at least MAXIMUMPARSETRBSPSIZE
 *
 * \return
 *    size of the RBSP in bytes
 *
 * \note
 *    Sequence Parameter VUI function is called, but the function implements
 *    an exit (-1)
 *************************************************************************************
 */
int GenerateSeq_parameter_set_rbsp (seq_parameter_set_rbsp_t *sps, byte *rbsp)
{
  Bitstream *bitstream;
  int len = 0, LenInBytes;
  unsigned i;
  unsigned n_ScalingList;

  assert (rbsp != NULL);

  if ((bitstream=calloc(1, sizeof(Bitstream)))==NULL) no_mem_exit("SeqParameterSet:bitstream");

  // .. and use the rbsp provided (or allocated above) for the data
  bitstream->streamBuffer = rbsp;
  bitstream->bits_to_go = 8;

  len+=u_v  (8, "SPS: profile_idc",                             sps->profile_idc,                               bitstream);

  len+=u_1  ("SPS: constrained_set0_flag",                      sps->constrained_set0_flag,    bitstream);
  len+=u_1  ("SPS: constrained_set1_flag",                      sps->constrained_set1_flag,    bitstream);
  len+=u_1  ("SPS: constrained_set2_flag",                      sps->constrained_set2_flag,    bitstream);
  len+=u_1  ("SPS: constrained_set3_flag",                      sps->constrained_set3_flag,    bitstream);
  len+=u_v  (4, "SPS: reserved_zero_4bits",                     0,                             bitstream);

  len+=u_v  (8, "SPS: level_idc",                               sps->level_idc,                                 bitstream);

  len+=ue_v ("SPS: seq_parameter_set_id",                    sps->seq_parameter_set_id,                      bitstream);

  // Fidelity Range Extensions stuff
  if( IS_FREXT_PROFILE(sps->profile_idc) )
  {
    len+=ue_v ("SPS: chroma_format_idc",                        sps->chroma_format_idc,                          bitstream);
    if(img->yuv_format == YUV444)
      len+=u_1  ("SPS: separate_colour_plane_flag",             sps->separate_colour_plane_flag,                 bitstream);
    len+=ue_v ("SPS: bit_depth_luma_minus8",                    sps->bit_depth_luma_minus8,                      bitstream);
    len+=ue_v ("SPS: bit_depth_chroma_minus8",                  sps->bit_depth_chroma_minus8,                    bitstream);
    len+=u_1  ("SPS: lossless_qpprime_y_zero_flag",             img->lossless_qpprime_flag,                      bitstream);
    //other chroma info to be added in the future

    len+=u_1 ("SPS: seq_scaling_matrix_present_flag",           sps->seq_scaling_matrix_present_flag,            bitstream);

    if(sps->seq_scaling_matrix_present_flag)
    {
      n_ScalingList = (sps->chroma_format_idc != YUV444) ? 8 : 12;
      for(i=0; i<n_ScalingList; i++)
      {
        len+=u_1 ("SPS: seq_scaling_list_present_flag",         sps->seq_scaling_list_present_flag[i],           bitstream);
        if(sps->seq_scaling_list_present_flag[i])
        {
          if(i<6)
            len+=Scaling_List(ScalingList4x4input[i], ScalingList4x4[i], 16, &UseDefaultScalingMatrix4x4Flag[i], bitstream);
          else
            len+=Scaling_List(ScalingList8x8input[i-6], ScalingList8x8[i-6], 64, &UseDefaultScalingMatrix8x8Flag[i-6], bitstream);
        }
      }
    }
  }

  len+=ue_v ("SPS: log2_max_frame_num_minus4",               sps->log2_max_frame_num_minus4,                 bitstream);
  len+=ue_v ("SPS: pic_order_cnt_type",                      sps->pic_order_cnt_type,                        bitstream);

  if (sps->pic_order_cnt_type == 0)
    len+=ue_v ("SPS: log2_max_pic_order_cnt_lsb_minus4",     sps->log2_max_pic_order_cnt_lsb_minus4,         bitstream);
  else if (sps->pic_order_cnt_type == 1)
  {
    len+=u_1  ("SPS: delta_pic_order_always_zero_flag",        sps->delta_pic_order_always_zero_flag,          bitstream);
    len+=se_v ("SPS: offset_for_non_ref_pic",                  sps->offset_for_non_ref_pic,                    bitstream);
    len+=se_v ("SPS: offset_for_top_to_bottom_field",          sps->offset_for_top_to_bottom_field,            bitstream);
    len+=ue_v ("SPS: num_ref_frames_in_pic_order_cnt_cycle",   sps->num_ref_frames_in_pic_order_cnt_cycle,     bitstream);
    for (i=0; i<sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
      len+=se_v ("SPS: offset_for_ref_frame",                  sps->offset_for_ref_frame[i],                      bitstream);
  }
  len+=ue_v ("SPS: num_ref_frames",                          sps->num_ref_frames,                            bitstream);
  len+=u_1  ("SPS: gaps_in_frame_num_value_allowed_flag",    sps->gaps_in_frame_num_value_allowed_flag,      bitstream);
  len+=ue_v ("SPS: pic_width_in_mbs_minus1",                 sps->pic_width_in_mbs_minus1,                   bitstream);
  len+=ue_v ("SPS: pic_height_in_map_units_minus1",          sps->pic_height_in_map_units_minus1,            bitstream);
  len+=u_1  ("SPS: frame_mbs_only_flag",                     sps->frame_mbs_only_flag,                       bitstream);
  if (!sps->frame_mbs_only_flag)
  {
    len+=u_1  ("SPS: mb_adaptive_frame_field_flag",            sps->mb_adaptive_frame_field_flag,              bitstream);
  }
  len+=u_1  ("SPS: direct_8x8_inference_flag",               sps->direct_8x8_inference_flag,                 bitstream);

  len+=u_1  ("SPS: frame_cropping_flag",                      sps->frame_cropping_flag,                       bitstream);
  if (sps->frame_cropping_flag)
  {
    len+=ue_v ("SPS: frame_cropping_rect_left_offset",          sps->frame_cropping_rect_left_offset,           bitstream);
    len+=ue_v ("SPS: frame_cropping_rect_right_offset",         sps->frame_cropping_rect_right_offset,          bitstream);
    len+=ue_v ("SPS: frame_cropping_rect_top_offset",           sps->frame_cropping_rect_top_offset,            bitstream);
    len+=ue_v ("SPS: frame_cropping_rect_bottom_offset",        sps->frame_cropping_rect_bottom_offset,         bitstream);
  }

  len+=u_1  ("SPS: vui_parameters_present_flag",             sps->vui_parameters_present_flag,               bitstream);

  if (sps->vui_parameters_present_flag)
    len+=GenerateVUI_parameters_rbsp(sps, bitstream);    // currently a dummy, asserting

  SODBtoRBSP(bitstream);     // copies the last couple of bits into the byte buffer

  LenInBytes=bitstream->byte_pos;

  free (bitstream);

  return LenInBytes;
}


/*!
 ***********************************************************************************************
 * \brief
 *    int GeneratePic_parameter_set_rbsp (pic_parameter_set_rbsp_t *sps, char *rbsp);
 *
 * \param pps
 *    picture parameter structure
 * \param rbsp
 *    buffer to be filled with the rbsp, size should be at least MAXIMUMPARSETRBSPSIZE
 *
 * \return
 *    size of the RBSP in bytes, negative in case of an error
 *
 * \note
 *    Picture Parameter VUI function is called, but the function implements
 *    an exit (-1)
 ************************************************************************************************
 */

int GeneratePic_parameter_set_rbsp (pic_parameter_set_rbsp_t *pps, byte *rbsp)
{
  Bitstream *bitstream;
  int len = 0, LenInBytes;
  unsigned i;
  unsigned n_ScalingList;
  unsigned NumberBitsPerSliceGroupId;
  int profile_idc;

  assert (rbsp != NULL);

  if ((bitstream=calloc(1, sizeof(Bitstream)))==NULL) no_mem_exit("PicParameterSet:bitstream");

  // .. and use the rbsp provided (or allocated above) for the data
  bitstream->streamBuffer = rbsp;
  bitstream->bits_to_go = 8;

  pps->pic_order_present_flag = img->pic_order_present_flag;

  len+=ue_v ("PPS: pic_parameter_set_id",                    pps->pic_parameter_set_id,                      bitstream);
  len+=ue_v ("PPS: seq_parameter_set_id",                    pps->seq_parameter_set_id,                      bitstream);
  len+=u_1  ("PPS: entropy_coding_mode_flag",                pps->entropy_coding_mode_flag,                  bitstream);
  len+=u_1  ("PPS: pic_order_present_flag",                  pps->pic_order_present_flag,                    bitstream);
  len+=ue_v ("PPS: num_slice_groups_minus1",                 pps->num_slice_groups_minus1,                   bitstream);

  // FMO stuff
  if(pps->num_slice_groups_minus1 > 0 )
  {
    len+=ue_v ("PPS: slice_group_map_type",                 pps->slice_group_map_type,                   bitstream);
    if (pps->slice_group_map_type == 0)
      for (i=0; i<=pps->num_slice_groups_minus1; i++)
        len+=ue_v ("PPS: run_length_minus1[i]",                           pps->run_length_minus1[i],                             bitstream);
    else if (pps->slice_group_map_type==2)
      for (i=0; i<pps->num_slice_groups_minus1; i++)
      {

        len+=ue_v ("PPS: top_left[i]",                          pps->top_left[i],                           bitstream);
        len+=ue_v ("PPS: bottom_right[i]",                      pps->bottom_right[i],                       bitstream);
      }
    else if (pps->slice_group_map_type == 3 ||
             pps->slice_group_map_type == 4 ||
             pps->slice_group_map_type == 5)
    {
      len+=u_1  ("PPS: slice_group_change_direction_flag",         pps->slice_group_change_direction_flag,         bitstream);
      len+=ue_v ("PPS: slice_group_change_rate_minus1",            pps->slice_group_change_rate_minus1,            bitstream);
    }
    else if (pps->slice_group_map_type == 6)
    {
      if (pps->num_slice_groups_minus1>=4)
        NumberBitsPerSliceGroupId=3;
      else if (pps->num_slice_groups_minus1>=2)
        NumberBitsPerSliceGroupId=2;
      else if (pps->num_slice_groups_minus1>=1)
        NumberBitsPerSliceGroupId=1;
      else
        NumberBitsPerSliceGroupId=0;

      len+=ue_v ("PPS: pic_size_in_map_units_minus1",                       pps->pic_size_in_map_units_minus1,             bitstream);
      for(i=0; i<=pps->pic_size_in_map_units_minus1; i++)
        len+= u_v  (NumberBitsPerSliceGroupId, "PPS: >slice_group_id[i]",   pps->slice_group_id[i],                        bitstream);
    }
  }
  // End of FMO stuff

  len+=ue_v ("PPS: num_ref_idx_l0_active_minus1",             pps->num_ref_idx_l0_active_minus1,              bitstream);
  len+=ue_v ("PPS: num_ref_idx_l1_active_minus1",             pps->num_ref_idx_l1_active_minus1,              bitstream);
  len+=u_1  ("PPS: weighted_pred_flag",                       pps->weighted_pred_flag,                        bitstream);
  len+=u_v  (2, "PPS: weighted_bipred_idc",                   pps->weighted_bipred_idc,                       bitstream);
  len+=se_v ("PPS: pic_init_qp_minus26",                      pps->pic_init_qp_minus26,                       bitstream);
  len+=se_v ("PPS: pic_init_qs_minus26",                      pps->pic_init_qs_minus26,                       bitstream);

  profile_idc = IdentifyProfile();
  if( IS_FREXT_PROFILE(profile_idc) )
    len+=se_v ("PPS: chroma_qp_index_offset",                 pps->cb_qp_index_offset,                        bitstream);
  else
    len+=se_v ("PPS: chroma_qp_index_offset",                 pps->chroma_qp_index_offset,                    bitstream);

  len+=u_1  ("PPS: deblocking_filter_control_present_flag",   pps->deblocking_filter_control_present_flag,    bitstream);
  len+=u_1  ("PPS: constrained_intra_pred_flag",              pps->constrained_intra_pred_flag,               bitstream);
  len+=u_1  ("PPS: redundant_pic_cnt_present_flag",           pps->redundant_pic_cnt_present_flag,            bitstream);

  // Fidelity Range Extensions stuff
  if( IS_FREXT_PROFILE(profile_idc) )
  {
    len+=u_1  ("PPS: transform_8x8_mode_flag",                pps->transform_8x8_mode_flag,                   bitstream);

    len+=u_1  ("PPS: pic_scaling_matrix_present_flag",        pps->pic_scaling_matrix_present_flag,           bitstream);

    if(pps->pic_scaling_matrix_present_flag)
    {
      n_ScalingList = 6 + ((active_sps->chroma_format_idc != 3) ? 2 : 6) * pps->transform_8x8_mode_flag;
      for(i=0; i<n_ScalingList; i++)  // SS-70226
      {
        len+=u_1  ("PPS: pic_scaling_list_present_flag",      pps->pic_scaling_list_present_flag[i],          bitstream);

        if(pps->pic_scaling_list_present_flag[i])
        {
          if(i<6)
            len+=Scaling_List(ScalingList4x4input[i], ScalingList4x4[i], 16, &UseDefaultScalingMatrix4x4Flag[i], bitstream);
          else
            len+=Scaling_List(ScalingList8x8input[i-6], ScalingList8x8[i-6], 64, &UseDefaultScalingMatrix8x8Flag[i-6], bitstream);
        }
      }
    }
    len+=se_v ("PPS: second_chroma_qp_index_offset",          pps->cr_qp_index_offset,                        bitstream);
  }

  SODBtoRBSP(bitstream);     // copies the last couple of bits into the byte buffer

  LenInBytes=bitstream->byte_pos;

  // Get rid of the helper structures
  free (bitstream);

  return LenInBytes;
}



/*!
 *************************************************************************************
 * \brief
 *    Returns the Profile
 *
 * \return
 *    Profile according to Annex A
 *
 * \note
 *    Function is currently a dummy.  Should "calculate" the profile from those
 *    config file parameters.  E.g.
 *
 *    Profile = Baseline;
 *    if (CABAC Used || Interlace used) Profile=Main;
 *    if (!Cabac Used) && (Bframes | SPframes) Profile = Streaming;
 *
 *************************************************************************************
 */
int IdentifyProfile(void)
{
  return params->ProfileIDC;
}

/*!
 *************************************************************************************
 * \brief
 *    Returns the Level
 *
 * \return
 *    Level according to Annex A
 *
 * \note
 *    This function is currently a dummy, but should calculate the level out of
 *    the config file parameters (primarily the picture size)
 *************************************************************************************
 */
int IdentifyLevel(void)
{
  return params->LevelIDC;
}


/*!
 *************************************************************************************
 * \brief
 *    Function body for VUI Parameter generation (to be done)
 *
 * \return
 *    exits with error message
 *************************************************************************************
 */
static int GenerateVUI_parameters_rbsp(seq_parameter_set_rbsp_t *sps, Bitstream *bitstream)
{
  int len=0;
  vui_seq_parameters_t *vui_seq_parameters = &(sps->vui_seq_parameters);

  len+=u_1 ("VUI: aspect_ratio_info_present_flag", vui_seq_parameters->aspect_ratio_info_present_flag, bitstream);
  if (vui_seq_parameters->aspect_ratio_info_present_flag)
  {        
    len+=u_v (8,"VUI: aspect_ratio_idc", vui_seq_parameters->aspect_ratio_idc, bitstream);
    if (vui_seq_parameters->aspect_ratio_idc == 255)
    {
      len+=u_v (16,"VUI: sar_width",  vui_seq_parameters->sar_width,  bitstream);
      len+=u_v (16,"VUI: sar_height", vui_seq_parameters->sar_height, bitstream);
    }
  }  

  len+=u_1 ("VUI: overscan_info_present_flag", vui_seq_parameters->overscan_info_present_flag, bitstream);
  if (vui_seq_parameters->overscan_info_present_flag)
  {
    len+=u_1 ("VUI: overscan_appropriate_flag", vui_seq_parameters->overscan_appropriate_flag, bitstream);
  } 

  len+=u_1 ("VUI: video_signal_type_present_flag", vui_seq_parameters->video_signal_type_present_flag, bitstream);
  if (vui_seq_parameters->video_signal_type_present_flag)
  {
    len+=u_v (3,"VUI: video_format", vui_seq_parameters->video_format, bitstream);
    len+=u_1 ("VUI: video_full_range_flag", vui_seq_parameters->video_full_range_flag, bitstream);
    len+=u_1 ("VUI: colour_description_present_flag", vui_seq_parameters->colour_description_present_flag, bitstream);
    if (vui_seq_parameters->colour_description_present_flag)
    {
      len+=u_v (8,"VUI: colour_primaries", vui_seq_parameters->colour_primaries, bitstream);
      len+=u_v (8,"VUI: transfer_characteristics", vui_seq_parameters->transfer_characteristics, bitstream);
      len+=u_v (8,"VUI: matrix_coefficients", vui_seq_parameters->matrix_coefficients, bitstream);
    }
  }

  len+=u_1 ("VUI: chroma_loc_info_present_flag", vui_seq_parameters->chroma_location_info_present_flag, bitstream);
  if (vui_seq_parameters->chroma_location_info_present_flag)
  {
    len+=ue_v ("VUI: chroma_sample_loc_type_top_field", vui_seq_parameters->chroma_sample_loc_type_top_field, bitstream);
    len+=ue_v ("VUI: chroma_sample_loc_type_bottom_field", vui_seq_parameters->chroma_sample_loc_type_bottom_field, bitstream);
  }

  len+=u_1 ("VUI: timing_info_present_flag", vui_seq_parameters->timing_info_present_flag, bitstream);
  // timing parameters
  if (vui_seq_parameters->timing_info_present_flag)
  {
    len+=u_v (32,"VUI: num_units_in_tick",  vui_seq_parameters->num_units_in_tick, bitstream);
    len+=u_v (32,"VUI: time_scale",         vui_seq_parameters->time_scale, bitstream);
    len+=u_1 ("VUI: fixed_frame_rate_flag", vui_seq_parameters->fixed_frame_rate_flag, bitstream);
  }
  // end of timing parameters
  // nal_hrd_parameters_present_flag
  len+=u_1 ("VUI: nal_hrd_parameters_present_flag", vui_seq_parameters->nal_hrd_parameters_present_flag, bitstream);
  if ( vui_seq_parameters->nal_hrd_parameters_present_flag )
  {
    len += WriteHRDParameters(sps, bitstream);
  }
  // vcl_hrd_parameters_present_flag
  len+=u_1 ("VUI: vcl_hrd_parameters_present_flag", vui_seq_parameters->vcl_hrd_parameters_present_flag, bitstream);
  if ( vui_seq_parameters->vcl_hrd_parameters_present_flag )
  {
    len += WriteHRDParameters(sps, bitstream);
  }
  if ( vui_seq_parameters->nal_hrd_parameters_present_flag || vui_seq_parameters->vcl_hrd_parameters_present_flag )
  {
    len+=u_1 ("VUI: low_delay_hrd_flag", vui_seq_parameters->low_delay_hrd_flag, bitstream );
  }
  len+=u_1 ("VUI: pic_struct_present_flag", vui_seq_parameters->pic_struct_present_flag, bitstream);

  len+=u_1 ("VUI: bitstream_restriction_flag", vui_seq_parameters->bitstream_restriction_flag, bitstream);
  if (vui_seq_parameters->bitstream_restriction_flag)
  {
    len+=u_1  ("VUI: motion_vectors_over_pic_boundaries_flag", vui_seq_parameters->motion_vectors_over_pic_boundaries_flag, bitstream);
    len+=ue_v ("VUI: max_bytes_per_pic_denom", vui_seq_parameters->max_bytes_per_pic_denom, bitstream);
    len+=ue_v ("VUI: max_bits_per_mb_denom", vui_seq_parameters->max_bits_per_mb_denom, bitstream);
    len+=ue_v ("VUI: log2_max_mv_length_horizontal", vui_seq_parameters->log2_max_mv_length_horizontal, bitstream);
    len+=ue_v ("VUI: log2_max_mv_length_vertical", vui_seq_parameters->log2_max_mv_length_vertical, bitstream);
    len+=ue_v ("VUI: num_reorder_frames", vui_seq_parameters->num_reorder_frames, bitstream);
    len+=ue_v ("VUI: max_dec_frame_buffering", vui_seq_parameters->max_dec_frame_buffering, bitstream);
  }

  return len;
}

/*!
 *************************************************************************************
 * \brief
 *    Function body for SEI message NALU generation
 *
 * \return
 *    A NALU containing the SEI messages
 *
 *************************************************************************************
 */
NALU_t *GenerateSEImessage_NALU()
{
  NALU_t *n = AllocNALU(64000);
  int RBSPlen = 0;
  int NALUlen;
  byte rbsp[MAXRBSPSIZE];

  RBSPlen = GenerateSEImessage_rbsp (NORMAL_SEI, rbsp);
  NALUlen = RBSPtoNALU (rbsp, n, RBSPlen, NALU_TYPE_SEI, NALU_PRIORITY_DISPOSABLE, 0, 1);
  n->startcodeprefix_len = 4;

  return n;
}


/*!
 *************************************************************************************
 * \brief
 *    int GenerateSEImessage_rbsp (int, bufferingperiod_information_struct*, char*)
 *
 *
 * \return
 *    size of the RBSP in bytes, negative in case of an error
 *
 * \note
 *************************************************************************************
 */
int GenerateSEImessage_rbsp (int id, byte *rbsp)
{
  Bitstream *bitstream;

  int len = 0, LenInBytes;
  assert (rbsp != NULL);

  if ((bitstream=calloc(1, sizeof(Bitstream)))==NULL) 
    no_mem_exit("SeqParameterSet:bitstream");

  // .. and use the rbsp provided (or allocated above) for the data
  bitstream->streamBuffer = rbsp;
  bitstream->bits_to_go = 8;

  {
    char sei_message[500] = "";
    char uuid_message[9] = "RandomMSG"; // This is supposed to be Random
    unsigned int i, message_size = strlen(params->SEIMessageText);
    struct TIMEB tstruct;
    ftime( &tstruct);    // start time

    if (message_size == 0)
    {
      message_size = 13;
      strncpy(sei_message,"Empty Message",message_size);
    }
    else
      strncpy(sei_message,params->SEIMessageText,message_size);

    len+=u_v (8,"SEI: last_payload_type_byte", 5, bitstream);
    message_size += 17;
    while (message_size > 254)
    {
      len+=u_v (8,"SEI: ff_byte",255, bitstream);
      message_size -= 255;
    }
    len+=u_v (8,"SEI: last_payload_size_byte",message_size, bitstream);

    // Lets randomize uuid based on time
    len+=u_v (32,"SEI: uuid_iso_iec_11578",(int) tstruct.timezone, bitstream);
    len+=u_v (32,"SEI: uuid_iso_iec_11578",(int) tstruct.time*1000+tstruct.millitm, bitstream);
    len+=u_v (32,"SEI: uuid_iso_iec_11578",(int) (uuid_message[0] << 24) + (uuid_message[1] << 16)  + (uuid_message[2] << 8) + (uuid_message[3] << 0), bitstream);
    len+=u_v (32,"SEI: uuid_iso_iec_11578",(int) (uuid_message[4] << 24) + (uuid_message[5] << 16)  + (uuid_message[6] << 8) + (uuid_message[7] << 0), bitstream);
    for (i = 0; i < strlen(sei_message); i++)
      len+=u_v (8,"SEI: user_data_payload_byte",sei_message[i], bitstream);

    len+=u_v (8,"SEI: user_data_payload_byte",   0, bitstream);
  }
  SODBtoRBSP(bitstream);     // copies the last couple of bits into the byte buffer

  LenInBytes=bitstream->byte_pos;

  free(bitstream);
  return LenInBytes;
}

/*!
 *************************************************************************************
 * \brief
 *    int WriteHRDParameters((seq_parameter_set_rbsp_t *sps, Bitstream *bitstream)
 *
 *
 * \return
 *    size of the RBSP in bytes, negative in case of an error
 *
 * \note
 *************************************************************************************
 */

int WriteHRDParameters(seq_parameter_set_rbsp_t *sps, Bitstream *bitstream)
{
  // hrd_parameters()
  int len = 0;
  unsigned int SchedSelIdx = 0;
  hrd_parameters_t *hrd = &(sps->vui_seq_parameters.nal_hrd_parameters);

  len+=ue_v ("VUI: cpb_cnt_minus1", hrd->cpb_cnt_minus1, bitstream);
  len+=u_v  (4, "VUI: bit_rate_scale", hrd->bit_rate_scale, bitstream);
  len+=u_v  (4, "VUI: cpb_size_scale", hrd->cpb_size_scale, bitstream);

  for( SchedSelIdx = 0; SchedSelIdx <= (hrd->cpb_cnt_minus1); SchedSelIdx++ )
  {
    len+=ue_v ("VUI: bit_rate_value_minus1", hrd->bit_rate_value_minus1[SchedSelIdx], bitstream);
    len+=ue_v ("VUI: cpb_size_value_minus1", hrd->cpb_size_value_minus1[SchedSelIdx], bitstream);
    len+=u_1  ("VUI: cbr_flag", hrd->cbr_flag[SchedSelIdx], bitstream);
  }

  len+=u_v  (5, "VUI: initial_cpb_removal_delay_length_minus1", hrd->initial_cpb_removal_delay_length_minus1, bitstream);
  len+=u_v  (5, "VUI: cpb_removal_delay_length_minus1", hrd->cpb_removal_delay_length_minus1, bitstream);
  len+=u_v  (5, "VUI: dpb_output_delay_length_minus1", hrd->dpb_output_delay_length_minus1, bitstream);
  len+=u_v  (5, "VUI: time_offset_length", hrd->time_offset_length, bitstream);

  return len;
}

/*!
 *************************************************************************************
 * \brief
 *    void GenerateVUIParameters(seq_parameter_set_rbsp_t *sps)
 *
 *
 * \return
 *    none
 *
 * \note
 *************************************************************************************
 */

void GenerateVUIParameters(seq_parameter_set_rbsp_t *sps)
{
  unsigned int          SchedSelIdx;
  hrd_parameters_t     *nal_hrd = &(sps->vui_seq_parameters.nal_hrd_parameters);
  hrd_parameters_t     *vcl_hrd = &(sps->vui_seq_parameters.vcl_hrd_parameters);
  vui_seq_parameters_t *vui     = &(sps->vui_seq_parameters);
  VUIParameters *iVui = &params->VUI;

  vui->aspect_ratio_info_present_flag      = (Boolean) iVui->aspect_ratio_info_present_flag;
  vui->aspect_ratio_idc                    = (unsigned int) iVui->aspect_ratio_idc;
  vui->sar_width                           = (unsigned int) iVui->sar_width;
  vui->sar_height                          = (unsigned int) iVui->sar_height;
  vui->overscan_info_present_flag          = (Boolean) iVui->overscan_info_present_flag;
  vui->overscan_appropriate_flag           = (Boolean) iVui->overscan_appropriate_flag;
  vui->video_signal_type_present_flag      = (Boolean) iVui->video_signal_type_present_flag;
  vui->video_format                        = (unsigned int) iVui->video_format;
  vui->video_full_range_flag               = (Boolean) iVui->video_full_range_flag;
  vui->colour_description_present_flag     = (Boolean) iVui->colour_description_present_flag;
  vui->colour_primaries                    = (unsigned int) iVui->colour_primaries;
  vui->transfer_characteristics            = (unsigned int) iVui->transfer_characteristics;
  vui->matrix_coefficients                 = (unsigned int) iVui->matrix_coefficients;
  vui->chroma_location_info_present_flag   = (Boolean) iVui->chroma_location_info_present_flag;
  vui->chroma_sample_loc_type_top_field    = (unsigned int) iVui->chroma_sample_loc_type_top_field;
  vui->chroma_sample_loc_type_bottom_field = (unsigned int) iVui->chroma_sample_loc_type_bottom_field;
  vui->timing_info_present_flag            = (Boolean) iVui->timing_info_present_flag;
  vui->num_units_in_tick                   = (unsigned int) iVui->num_units_in_tick;
  vui->time_scale                          = (unsigned int) iVui->time_scale;
  vui->fixed_frame_rate_flag               = (Boolean) iVui->fixed_frame_rate_flag;  

  // NAL HRD parameters
  vui->nal_hrd_parameters_present_flag             = (Boolean) iVui->nal_hrd_parameters_present_flag;  
  nal_hrd->cpb_cnt_minus1                          = (unsigned int) iVui->nal_cpb_cnt_minus1;
  nal_hrd->bit_rate_scale                          = (unsigned int) iVui->nal_bit_rate_scale;
  nal_hrd->cpb_size_scale                          = (unsigned int) iVui->nal_cpb_size_scale;
  for ( SchedSelIdx = 0; SchedSelIdx <= nal_hrd->cpb_cnt_minus1; SchedSelIdx++ )
  {
    nal_hrd->bit_rate_value_minus1[SchedSelIdx]    = (unsigned int) iVui->nal_bit_rate_value_minus1;
    nal_hrd->cpb_size_value_minus1[SchedSelIdx]    = (unsigned int) iVui->nal_cpb_size_value_minus1;
    nal_hrd->cbr_flag[SchedSelIdx]                 = (unsigned int) iVui->nal_vbr_cbr_flag;
  }
  nal_hrd->initial_cpb_removal_delay_length_minus1 = (unsigned int) iVui->nal_initial_cpb_removal_delay_length_minus1;
  nal_hrd->cpb_removal_delay_length_minus1         = (unsigned int) iVui->nal_cpb_removal_delay_length_minus1;
  nal_hrd->dpb_output_delay_length_minus1          = (unsigned int) iVui->nal_dpb_output_delay_length_minus1;
  nal_hrd->time_offset_length                      = (unsigned int) iVui->nal_time_offset_length;
  
  // VCL HRD parameters
  vui->vcl_hrd_parameters_present_flag             = (Boolean) iVui->vcl_hrd_parameters_present_flag;  
  vcl_hrd->cpb_cnt_minus1                          = (unsigned int) iVui->vcl_cpb_cnt_minus1;
  vcl_hrd->bit_rate_scale                          = (unsigned int) iVui->vcl_bit_rate_scale;
  vcl_hrd->cpb_size_scale                          = (unsigned int) iVui->vcl_cpb_size_scale;
  for ( SchedSelIdx = 0; SchedSelIdx <= vcl_hrd->cpb_cnt_minus1; SchedSelIdx++ )
  {
    vcl_hrd->bit_rate_value_minus1[SchedSelIdx]    = (unsigned int) iVui->vcl_bit_rate_value_minus1;
    vcl_hrd->cpb_size_value_minus1[SchedSelIdx]    = (unsigned int) iVui->vcl_cpb_size_value_minus1;
    vcl_hrd->cbr_flag[SchedSelIdx]                 = (unsigned int) iVui->vcl_vbr_cbr_flag;
  }
  vcl_hrd->initial_cpb_removal_delay_length_minus1 = (unsigned int) iVui->vcl_initial_cpb_removal_delay_length_minus1;
  vcl_hrd->cpb_removal_delay_length_minus1         = (unsigned int) iVui->vcl_cpb_removal_delay_length_minus1;
  vcl_hrd->dpb_output_delay_length_minus1          = (unsigned int) iVui->vcl_dpb_output_delay_length_minus1;
  vcl_hrd->time_offset_length                      = (unsigned int) iVui->vcl_time_offset_length;

  vui->low_delay_hrd_flag                      = (Boolean) iVui->low_delay_hrd_flag;
  vui->pic_struct_present_flag                 = (Boolean) iVui->pic_struct_present_flag;
  vui->bitstream_restriction_flag              = (Boolean) iVui->bitstream_restriction_flag;
  vui->motion_vectors_over_pic_boundaries_flag = (Boolean) iVui->motion_vectors_over_pic_boundaries_flag;
  vui->max_bytes_per_pic_denom                 = (unsigned int) iVui->max_bytes_per_pic_denom;
  vui->max_bits_per_mb_denom                   = (unsigned int) iVui->max_bits_per_mb_denom;
  vui->log2_max_mv_length_horizontal           = (unsigned int) iVui->log2_max_mv_length_horizontal;
  vui->log2_max_mv_length_vertical             = (unsigned int) iVui->log2_max_mv_length_vertical;
  vui->num_reorder_frames                      = (unsigned int) iVui->num_reorder_frames;
  vui->max_dec_frame_buffering                 = (unsigned int) iVui->max_dec_frame_buffering;
  
  // special case to signal the RGB format
  if(params->rgb_input_flag && params->yuv_format==3)
  {
    printf   ("VUI: writing Sequence Parameter VUI to signal RGB format\n");

    vui->aspect_ratio_info_present_flag = FALSE;
    vui->overscan_info_present_flag = FALSE;
    vui->video_signal_type_present_flag = TRUE;
    vui->video_format = 2;
    vui->video_full_range_flag = TRUE;
    vui->colour_description_present_flag = TRUE;
    vui->colour_primaries = 2;
    vui->transfer_characteristics = 2;
    vui->matrix_coefficients = 0;
    vui->chroma_location_info_present_flag = FALSE;
    vui->timing_info_present_flag = FALSE;
    vui->nal_hrd_parameters_present_flag = FALSE;
    vui->vcl_hrd_parameters_present_flag = FALSE;
    vui->pic_struct_present_flag = FALSE;
    vui->bitstream_restriction_flag = FALSE;
  } 
}
