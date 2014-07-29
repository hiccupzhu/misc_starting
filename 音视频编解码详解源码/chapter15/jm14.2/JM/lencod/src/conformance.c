
/*!
 **************************************************************************************
 * \file
 *    conformance.c
 * \brief
 *    Level & Profile related conformance functions
 * \author
 *  Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Karsten Suehring  
 *    - Alexis Michael Tourapis
 * \note
 *
 **************************************************************************************
 */

#include "global.h"

// Level Limit             -  -  -  -  -  -  -  -  -  1b  10  11   12   13   -  -  -  -  -  -  20   21   22    -  -  -  -  -  -  -
unsigned int  MaxFs [] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 99, 99, 396, 396, 396, 0, 0, 0, 0, 0, 0, 396, 792, 1620, 0, 0, 0, 0, 0, 0, 0,  
//                        30    31    32    -  -  -  -  -  -  -  40    41    42    -  -  -  -  -  -  -  50     51
                          1620, 3600, 5120, 0, 0, 0, 0, 0, 0, 0, 8192, 8192, 8704, 0, 0, 0, 0, 0, 0, 0, 22080, 36864 };

// Vertical MV Limits (integer/halfpel/quarterpel)
// Currently only Integer Pel restrictions are used,
// since the way values are specified
// (i.e. mvlowbound = (levelmvlowbound + 1) and the way
// Subpel ME is performed, subpel will always be within range.
const int LEVELMVLIMIT[17][6] =
{
  {  -63,  63,  -128,  127,  -256,  255},
  {  -63,  63,  -128,  127,  -256,  255},
  { -127, 127,  -256,  255,  -512,  511},
  { -127, 127,  -256,  255,  -512,  511},
  { -127, 127,  -256,  255,  -512,  511},
  { -127, 127,  -256,  255,  -512,  511},
  { -255, 255,  -512,  511, -1024, 1023},
  { -255, 255,  -512,  511, -1024, 1023},
  { -255, 255,  -512,  511, -1024, 1023},
  { -511, 511, -1024, 1023, -2048, 2047},
  { -511, 511, -1024, 1023, -2048, 2047},
  { -511, 511, -1024, 1023, -2048, 2047},
  { -511, 511, -1024, 1023, -2048, 2047},
  { -511, 511, -1024, 1023, -2048, 2047},
  { -511, 511, -1024, 1023, -2048, 2047},
  { -511, 511, -1024, 1023, -2048, 2047},
  { -511, 511, -1024, 1023, -2048, 2047}

  /*
  {  -64,  63,  -128,  127,  -256,  255},
  {  -64,  63,  -128,  127,  -256,  255},
  { -128, 127,  -256,  255,  -512,  511},
  { -128, 127,  -256,  255,  -512,  511},
  { -128, 127,  -256,  255,  -512,  511},
  { -128, 127,  -256,  255,  -512,  511},
  { -256, 255,  -512,  511, -1024, 1023},
  { -256, 255,  -512,  511, -1024, 1023},
  { -256, 255,  -512,  511, -1024, 1023},
  { -512, 511, -1024, 1023, -2048, 2047},
  { -512, 511, -1024, 1023, -2048, 2047},
  { -512, 511, -1024, 1023, -2048, 2047},
  { -512, 511, -1024, 1023, -2048, 2047},
  { -512, 511, -1024, 1023, -2048, 2047},
  { -512, 511, -1024, 1023, -2048, 2047},
  { -512, 511, -1024, 1023, -2048, 2047},
  { -512, 511, -1024, 1023, -2048, 2047}
  */
};

/*!
 ***********************************************************************
 * \brief
 *    Get maximum frame size (in MBs) supported given a level
 ***********************************************************************
 */
unsigned getMaxFs (unsigned int levelIdc)
{
  unsigned int ret;

  if ( (levelIdc < 9) || (levelIdc > 51))
    error ("getMaxFs: Unknown LevelIdc", 500);

  // in Baseline, Main and Extended: Level 1b is specified with LevelIdc==11 and constrained_set3_flag == 1

  ret = MaxFs[levelIdc];

  if ( 0 == ret )
    error ("getMaxFs: Unknown LevelIdc", 500);

  return ret;
}

/*!
 ***********************************************************************
 * \brief
 *    Check Profile conformance
 ***********************************************************************
 */
void ProfileCheck(void)
{
  if((params->ProfileIDC != 66 ) &&
     (params->ProfileIDC != 77 ) &&
     (params->ProfileIDC != 88 ) &&
     (params->ProfileIDC != FREXT_HP    ) &&
     (params->ProfileIDC != FREXT_Hi10P ) &&
     (params->ProfileIDC != FREXT_Hi422 ) &&
     (params->ProfileIDC != FREXT_Hi444 ) &&
     (params->ProfileIDC != FREXT_CAVLC444 ))
  {
    snprintf(errortext, ET_SIZE, "Profile must be in\n\n  66 (Baseline),\n  77 (Main),\n  88 (Extended),\n 100 (High),\n 110 (High 10 or High 10 Intra)\n"
      " 122 (High 4:2:2 or High 4:2:2 Intra),\n 244 (High 4:4:4 predictive or High 4:4:4 Intra),\n  44 (CAVLC 4:4:4 Intra)\n");
    error (errortext, 500);
  }

  if ((params->partition_mode) && (params->symbol_mode==CABAC))
  {
    snprintf(errortext, ET_SIZE, "Data partitioning and CABAC is not supported in any profile.");
    error (errortext, 500);
  }

  if (params->redundant_pic_flag)
  {
    if (params->ProfileIDC != 66)
    {
      snprintf(errortext, ET_SIZE, "Redundant pictures are only allowed in Baseline profile (ProfileIDC = 66).");
      error (errortext, 500);
    }
  }

  if ((params->partition_mode) && (params->ProfileIDC!=88))
  {
    snprintf(errortext, ET_SIZE, "Data partitioning is only allowed in Extended profile (ProfileIDC = 88).");
    error (errortext, 500);
  }

  if (params->ChromaIntraDisable && params->FastCrIntraDecision)
  {
    fprintf( stderr, "\n Warning: ChromaIntraDisable and FastCrIntraDecision cannot be combined together.\n Using only Chroma Intra DC mode.\n");
    params->FastCrIntraDecision=0;
  }
  
  if ((params->sp_periodicity) && (params->ProfileIDC != 88 ))
  {
    snprintf(errortext, ET_SIZE, "SP pictures are only allowed in Extended profile (ProfileIDC = 88).");
    error (errortext, 500);
  }

  // baseline
  if (params->ProfileIDC == 66 )
  {
    if ((params->successive_Bframe || params->BRefPictures==2) && params->PReplaceBSlice == 0)
    {
      snprintf(errortext, ET_SIZE, "B slices are not allowed in Baseline profile (ProfileIDC = 66).");
      error (errortext, 500);
    }
    if (params->WeightedPrediction)
    {
      snprintf(errortext, ET_SIZE, "Weighted prediction is not allowed in Baseline profile (ProfileIDC = 66).");
      error (errortext, 500);
    }
    if (params->WeightedBiprediction)
    {
      snprintf(errortext, ET_SIZE, "Weighted prediction is not allowed in Baseline profile (ProfileIDC = 66).");
      error (errortext, 500);
    }
    if (params->symbol_mode == CABAC)
    {
      snprintf(errortext, ET_SIZE, "CABAC is not allowed in Baseline profile (ProfileIDC = 66).");
      error (errortext, 500);
    }
    if ((params->PicInterlace) ||(params->MbInterlace))
    {
      snprintf(errortext, ET_SIZE, "Interlace tools are not allowed in Baseline profile (ProfileIDC = 66).");
      error (errortext, 500);
    }
  }

  // main
  if (params->ProfileIDC == 77 )
  {
    if (params->num_slice_groups_minus1)
    {
      snprintf(errortext, ET_SIZE, "num_slice_groups_minus1>0 (FMO) is not allowed in Main profile (ProfileIDC = 77).");
      error (errortext, 500);
    }
  }

  // extended
  if (params->ProfileIDC == 88 )
  {
    if (!params->directInferenceFlag)
    {
      snprintf(errortext, ET_SIZE, "direct_8x8_inference flag must be equal to 1 in Extended profile (ProfileIDC = 88).");
      error (errortext, 500);
    }

    if (params->symbol_mode == CABAC)
    {
      snprintf(errortext, ET_SIZE, "CABAC is not allowed in Extended profile (ProfileIDC = 88).");
      error (errortext, 500);
    }
  }

  //FRExt
  if ( params->separate_colour_plane_flag )
  {
    if( params->yuv_format!=3 )
    {
      fprintf( stderr, "\nWarning: SeparateColourPlane has only effect in 4:4:4 chroma mode (YUVFormat=3),\n         disabling SeparateColourPlane.");
      params->separate_colour_plane_flag = 0;
    }

    if ( params->ChromaMEEnable )
    {
      snprintf(errortext, ET_SIZE, "\nChromaMEEnable is not allowed when SeparateColourPlane is enabled.");
      error (errortext, 500);
    }
  }

  // CAVLC 4:4:4 Intra
  if ( params->ProfileIDC == FREXT_CAVLC444 )
  {
    if ( params->symbol_mode != CAVLC )
    {
      snprintf(errortext, ET_SIZE, "\nCABAC is not allowed in CAVLC 4:4:4 Intra profile (ProfileIDC = 44).");
      error (errortext, 500);
    }
    if ( !params->IntraProfile )
    {
      fprintf (stderr, "\nWarning: ProfileIDC equal to 44 implies an Intra only profile, setting IntraProfile = 1.");
      params->IntraProfile = 1;
    }
  }

  // Intra only profiles
  if (params->IntraProfile && ( params->ProfileIDC<FREXT_HP && params->ProfileIDC!=FREXT_CAVLC444 ))
  {
    snprintf(errortext, ET_SIZE, "\nIntraProfile is allowed only with ProfileIDC %d to %d.", FREXT_HP, FREXT_Hi444);
    error (errortext, 500);
  }

  if (params->IntraProfile && !params->idr_period) 
  {
    snprintf(errortext, ET_SIZE, "\nIntraProfile requires IDRPeriod >= 1.");
    error (errortext, 500);
  }

  if (params->IntraProfile && params->intra_period != 1) 
  {
    snprintf(errortext, ET_SIZE, "\nIntraProfile requires IntraPeriod equal 1.");
    error (errortext, 500);
  }

  if (params->IntraProfile && params->num_ref_frames) 
  {
    fprintf( stderr, "\nWarning: Setting NumberReferenceFrames to 0 in IntraProfile.");
    params->num_ref_frames = 0;
  }

  if (params->IntraProfile == 0 && params->num_ref_frames == 0) 
  {
    snprintf(errortext, ET_SIZE, "\nProfiles other than IntraProfile require NumberReferenceFrames > 0.");
    error (errortext, 500);
  }
}

/*!
 ***********************************************************************
 * \brief
 *    Check if Level constraints are satisfied
 ***********************************************************************
 */
void LevelCheck(void)
{
  unsigned int PicSizeInMbs = ( (params->output.width + img->auto_crop_right) * (params->output.height + img->auto_crop_bottom) ) >> 8;

  if ( (params->LevelIDC>=30) && (params->directInferenceFlag==0))
  {
    fprintf( stderr, "\nWarning: LevelIDC 3.0 and above require direct_8x8_inference to be set to 1. Please check your settings.\n");
    params->directInferenceFlag=1;
  }
  if ( ((params->LevelIDC<21) || (params->LevelIDC>41)) && (params->PicInterlace > 0 || params->MbInterlace > 0) )
  {
    snprintf(errortext, ET_SIZE, "\nInterlace modes only supported for LevelIDC in the range of 21 and 41. Please check your settings.\n");
    error (errortext, 500);
  }

  if ( PicSizeInMbs > getMaxFs(params->LevelIDC) )
  {
    snprintf(errortext, ET_SIZE, "\nPicSizeInMbs exceeds maximum allowed size at specified LevelIdc %d\n", params->LevelIDC);
    error (errortext, 500);
  }
  
  if (params->IntraProfile && (PicSizeInMbs > 1620) && params->slice_mode != 1) 
  {
    error ("\nIntraProfile with PicSizeInMbs > 1620 requires SliceMode equal 1.", 500);
  }

  if (params->IntraProfile && (PicSizeInMbs > 1620) && ((unsigned int)params->slice_argument > (  getMaxFs(params->LevelIDC) >> 2 ) ) )
  {
    //when PicSizeInMbs is greater than 1620, the number of macroblocks in any coded slice shall not exceed MaxFS / 4
    snprintf(errortext, ET_SIZE, "\nIntraProfile requires SliceArgument smaller or equal to 1/4 MaxFs at specified LevelIdc %d.", params->LevelIDC);
    error (errortext, 500);
  }
}

/*!
 ***********************************************************************
 * \brief
 *    Update Motion Vector Limits
 ***********************************************************************
 */
void update_mv_limits(ImageParameters *img, byte is_field)
{
  memcpy(img->MaxVmvR, LEVELMVLIMIT[img->LevelIndex], 6 * sizeof(int));
  if (is_field)
  {
    int i;
    for (i = 0; i < 6; i++)
      img->MaxVmvR[i] >>= 1;
  }
}


/*!
 ***********************************************************************
 * \brief
 *    Clip motion vector range given encoding level
 ***********************************************************************
 */
void clip_mv_range(ImageParameters *img, int search_range, short mv[2], int res)
{
  int max_hor = 2047 << res;
  search_range <<= res;

  mv[0] = iClip3( -max_hor + search_range, max_hor - search_range, mv[0]);
  mv[1] = iClip3( img->MaxVmvR[0 + res] + search_range, img->MaxVmvR[1 + res] - search_range, mv[1]);
}

/*!
 ***********************************************************************
 * \brief
 *    Clip motion vector range given encoding level
 ***********************************************************************
 */
int out_of_bounds_mvs(ImageParameters *img, short mv[2], int res)
{
  return (mv[0] < -8192 || mv[0] > 8191|| mv[1] < img->MaxVmvR[4] || mv[1] > img->MaxVmvR[5]);
}

