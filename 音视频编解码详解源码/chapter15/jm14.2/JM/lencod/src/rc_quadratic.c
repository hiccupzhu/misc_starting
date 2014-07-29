
/*!
 ***************************************************************************
 * \file rc_quadratic.c
 *
 * \brief
 *    Rate Control algorithm
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Siwei Ma             <swma@jdl.ac.cn>
 *     - Zhengguo LI          <ezgli@lit.a-star.edu.sg>
 *     - Athanasios Leontaris <aleon@dolby.com>
 *
 * \date
 *   16 Jan. 2003
 **************************************************************************
 */

#include <math.h>
#include <limits.h>

#include "global.h"
#include "ratectl.h"


static const float THETA = 1.3636F;
static const float OMEGA = 0.9F;
static const float MINVALUE = 4.0F;
/*!
 *************************************************************************************
 * \brief
 *    Dynamically allocate memory needed for rate control
 *
 *************************************************************************************
 */
void rc_alloc_quadratic( rc_quadratic **prc )
{
  int rcBufSize = img->FrameSizeInMbs / params->basicunit;
  rc_quadratic *lprc;

  (*prc) = (rc_quadratic *) malloc ( sizeof( rc_quadratic ) );
  if (NULL==(*prc))
  {
    no_mem_exit("init_global_buffers: (*prc)");
  }
  lprc = *prc;

  lprc->PreviousFrameMAD = 1.0;
  lprc->CurrentFrameMAD = 1.0;
  lprc->Pprev_bits = 0;
  lprc->Target = 0;
  lprc->TargetField = 0;
  lprc->LowerBound = 0;
  lprc->UpperBound1 = INT_MAX;
  lprc->UpperBound2 = INT_MAX;
  lprc->Wp = 0.0;
  lprc->Wb = 0.0;
  lprc->AveWb = 0.0;
  lprc->PAveFrameQP   = params->qp0;
  lprc->m_Qc          = lprc->PAveFrameQP;
  lprc->FieldQPBuffer = lprc->PAveFrameQP;
  lprc->FrameQPBuffer = lprc->PAveFrameQP;
  lprc->PAverageQp    = lprc->PAveFrameQP;
  lprc->MyInitialQp   = lprc->PAveFrameQP;
  lprc->AveWb         = 0.0;

  lprc->BUPFMAD = (double*) calloc ((rcBufSize), sizeof (double));
  if (NULL==lprc->BUPFMAD)
  {
    no_mem_exit("rc_alloc_quadratic: lprc->BUPFMAD");
  }

  lprc->BUCFMAD = (double*) calloc ((rcBufSize), sizeof (double));
  if (NULL==lprc->BUCFMAD)
  {
    no_mem_exit("rc_alloc_quadratic: lprc->BUCFMAD");
  }

  lprc->FCBUCFMAD = (double*) calloc ((rcBufSize), sizeof (double));
  if (NULL==lprc->FCBUCFMAD)
  {
    no_mem_exit("rc_alloc_quadratic: lprc->FCBUCFMAD");
  }

  lprc->FCBUPFMAD = (double*) calloc ((rcBufSize), sizeof (double));
  if (NULL==lprc->FCBUPFMAD)
  {
    no_mem_exit("rc_alloc_quadratic: lprc->FCBUPFMAD");
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Copy JVT rate control objects
 *
 *************************************************************************************
 */
void rc_copy_quadratic( rc_quadratic *dst, rc_quadratic *src )
{
  int rcBufSize = img->FrameSizeInMbs / params->basicunit;
  /* buffer original addresses for which memory has been allocated */
  double   *tmpBUPFMAD = dst->BUPFMAD;
  double   *tmpBUCFMAD = dst->BUCFMAD;
  double *tmpFCBUPFMAD = dst->FCBUPFMAD;
  double *tmpFCBUCFMAD = dst->FCBUCFMAD;

  /* copy object */
  memcpy( (void *)dst, (void *)src, sizeof(rc_quadratic) );

  /* restore original addresses */
  dst->BUPFMAD   = tmpBUPFMAD;
  dst->BUCFMAD   = tmpBUCFMAD;
  dst->FCBUPFMAD = tmpFCBUPFMAD;
  dst->FCBUCFMAD = tmpFCBUCFMAD;

  /* copy MADs */
  memcpy( (void *)dst->BUPFMAD,   (void *)src->BUPFMAD,   (rcBufSize) * sizeof (double) );
  memcpy( (void *)dst->BUCFMAD,   (void *)src->BUCFMAD,   (rcBufSize) * sizeof (double) );
  memcpy( (void *)dst->FCBUPFMAD, (void *)src->FCBUPFMAD, (rcBufSize) * sizeof (double) );
  memcpy( (void *)dst->FCBUCFMAD, (void *)src->FCBUCFMAD, (rcBufSize) * sizeof (double) );
}

/*!
 *************************************************************************************
 * \brief
 *    Free memory needed for rate control
 *
 *************************************************************************************
*/
void rc_free_quadratic(rc_quadratic **prc)
{
  if (NULL!=(*prc)->BUPFMAD)
  {
    free ((*prc)->BUPFMAD);
    (*prc)->BUPFMAD = NULL;
  }
  if (NULL!=(*prc)->BUCFMAD)
  {
    free ((*prc)->BUCFMAD);
    (*prc)->BUCFMAD = NULL;
  }
  if (NULL!=(*prc)->FCBUCFMAD)
  {
    free ((*prc)->FCBUCFMAD);
    (*prc)->FCBUCFMAD = NULL;
  }
  if (NULL!=(*prc)->FCBUPFMAD)
  {
    free ((*prc)->FCBUPFMAD);
    (*prc)->FCBUPFMAD = NULL;
  }
  if (NULL!=(*prc))
  {
    free ((*prc));
    (*prc) = NULL;
  }
}


/*!
 *************************************************************************************
 * \brief
 *    Initialize rate control parameters
 *
 *************************************************************************************
*/
void rc_init_seq(rc_quadratic *prc)
{
  double L1,L2,L3,bpp;
  int qp, i;

  switch ( params->RCUpdateMode )
  {
  case RC_MODE_0:
    updateQP = updateQPRC0;
    break;
  case RC_MODE_1:
    updateQP = updateQPRC1;
    break;
  case RC_MODE_2:
    updateQP = updateQPRC2;
    break;
  case RC_MODE_3:
    updateQP = updateQPRC3;
    break;
  default:
    updateQP = updateQPRC0;
    break;
  }
  // set function pointers
  rc_update_pict_frame_ptr = rc_update_pict_frame;
  rc_update_picture_ptr    = rc_update_picture;
  rc_init_pict_ptr         = rc_init_pict;

  prc->Xp=0;
  prc->Xb=0;

  prc->bit_rate = (float) params->bit_rate;
  prc->frame_rate = (img->framerate *(float)(params->successive_Bframe + 1)) / (float) (params->jumpd + 1);
  prc->PrevBitRate = prc->bit_rate;

  /*compute the total number of MBs in a frame*/
  if(params->basicunit > img->FrameSizeInMbs)
    params->basicunit = img->FrameSizeInMbs;
  if(params->basicunit < img->FrameSizeInMbs)
    prc->TotalNumberofBasicUnit = img->FrameSizeInMbs/params->basicunit;
  else
    prc->TotalNumberofBasicUnit = 1;

  /*initialize the parameters of fluid flow traffic model*/
  generic_RC->CurrentBufferFullness = 0;
  prc->GOPTargetBufferLevel = (double) generic_RC->CurrentBufferFullness;

  /*initialize the previous window size*/
  prc->m_windowSize    = 0;
  prc->MADm_windowSize = 0;
  generic_RC->NumberofCodedBFrame = 0;
  prc->NumberofCodedPFrame = 0;
  generic_RC->NumberofGOP         = 0;
  /*remaining # of bits in GOP */
  generic_RC->RemainingBits = 0;
  /*control parameter */
  if(params->successive_Bframe>0)
  {
    prc->GAMMAP=0.25;
    prc->BETAP=0.9;
  }
  else
  {
    prc->GAMMAP=0.5;
    prc->BETAP=0.5;
  }

  /*quadratic rate-distortion model*/
  prc->PPreHeader=0;

  prc->Pm_X1 = prc->bit_rate * 1.0;
  prc->Pm_X2 = 0.0;
  /* linear prediction model for P picture*/
  prc->PMADPictureC1 = 1.0;
  prc->PMADPictureC2 = 0.0;

  // Initialize values
  for(i=0;i<21;i++)
  {
    prc->Pm_rgQp[i] = 0;
    prc->Pm_rgRp[i] = 0.0;
    prc->PPictureMAD[i] = 0.0;
  }

  //Define the largest variation of quantization parameters
  prc->PMaxQpChange = params->RCMaxQPChange;

  /*basic unit layer rate control*/
  prc->PAveHeaderBits1 = 0;
  prc->PAveHeaderBits3 = 0;
  prc->DDquant = (prc->TotalNumberofBasicUnit>=9 ? 1 : 2);

  prc->MBPerRow = img->PicWidthInMbs;

  /*adaptive field/frame coding*/
  generic_RC->FieldControl=0;  

  if (params->SeinitialQP==0)
  {
    /*compute the initial QP*/
    bpp = 1.0*prc->bit_rate /(prc->frame_rate*img->size);

    if (img->width == 176)
    {
      L1 = 0.1;
      L2 = 0.3;
      L3 = 0.6;
    }
    else if (img->width == 352)
    {
      L1 = 0.2;
      L2 = 0.6;
      L3 = 1.2;
    }
    else
    {
      L1 = 0.6;
      L2 = 1.4;
      L3 = 2.4;
    }
    if (bpp<= L1)
      qp = 35;
    else if(bpp<=L2)
      qp = 25;
    else if(bpp<=L3)
      qp = 20;
    else
      qp = 10;
    params->SeinitialQP = qp;
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Initialize one GOP
 *
 *************************************************************************************
*/
void rc_init_GOP(rc_quadratic *prc, int np, int nb)
{
  Boolean Overum=FALSE;
  int OverBits, OverDuantQp;
  int GOPDquant;
  int64 AllocatedBits;

  // bit allocation for RC_MODE_3
  switch( params->RCUpdateMode )
  {
  case RC_MODE_3:
    {
      int sum = 0, tmp, level, levels = 0, num_frames[RC_MAX_TEMPORAL_LEVELS];
      float numer, denom;
      int gop = params->successive_Bframe + 1;
      memset( num_frames, 0, RC_MAX_TEMPORAL_LEVELS * sizeof(int) );
      // are there any B frames?
      if ( params->successive_Bframe )
      {
        if ( params->HierarchicalCoding == 1 ) // two layers: even/odd
        {
          levels = 2;
          num_frames[0] = params->successive_Bframe >> 1;
          num_frames[1] = (params->successive_Bframe - num_frames[0]) >= 0 ? (params->successive_Bframe - num_frames[0]) : 0;
        }
        else if ( params->HierarchicalCoding == 2 ) // binary hierarchical structure
        {
          // check if gop is power of two
          tmp = gop;
          while ( tmp )
          {
            sum += tmp & 1;
            tmp >>= 1;
          }
          assert( sum == 1 );

          // determine number of levels
          levels = 0;
          tmp = gop;
          while ( tmp > 1 )
          {
            tmp >>= 1; // divide by 2          
            num_frames[levels] = 1 << levels;
            levels++;          
          }
          assert( levels >= 1 && levels <= RC_MAX_TEMPORAL_LEVELS );        
        }
        else if ( params->HierarchicalCoding == 3 )
        {
          fprintf(stderr, "\n RCUpdateMode=3 and HierarchicalCoding == 3 are currently not supported"); // This error message should be moved elsewhere and have proper memory deallocation
          exit(1);
        }
        else // all frames of the same priority - level
        {
          levels = 1;
          num_frames[0] = params->successive_Bframe;
        }
        generic_RC->temporal_levels = levels;      
      }
      else
      {
        for ( level = 0; level < RC_MAX_TEMPORAL_LEVELS; level++ )
        {
          params->RCBSliceBitRatio[level] = 0.0F;
        }
        generic_RC->temporal_levels = 0;
      }
      // calculate allocated bits for each type of frame
      numer = (float)(( (!params->intra_period ? 1 : params->intra_period) * gop) * ((double)params->bit_rate / params->FrameRate));
      denom = 0.0F;

      for ( level = 0; level < levels; level++ )
      {
        denom += (float)(num_frames[level] * params->RCBSliceBitRatio[level]);
        generic_RC->hierNb[level] = num_frames[level] * np;
      }
      denom += 1.0F;
      if ( params->intra_period >= 1 )
      {
        denom *= (float)params->intra_period;
        denom += (float)params->RCISliceBitRatio - 1.0F;
      }

      // set bit targets for each type of frame
      generic_RC->RCPSliceBits = (int) floor( numer / denom + 0.5F );
      generic_RC->RCISliceBits = (params->intra_period) ? (int)(params->RCISliceBitRatio * generic_RC->RCPSliceBits + 0.5) : 0;

      for ( level = 0; level < levels; level++ )
      {
        generic_RC->RCBSliceBits[level] = (int)floor(params->RCBSliceBitRatio[level] * generic_RC->RCPSliceBits + 0.5);
      }

      generic_RC->NISlice = (params->intra_period) ? ((params->no_frames - 1) / params->intra_period) : 0;
      generic_RC->NPSlice = params->no_frames - 1 - generic_RC->NISlice;
    }
    break;
  default:
    break;
  }

  /* check if the last GOP over uses its budget. If yes, the initial QP of the I frame in
  the coming  GOP will be increased.*/

  if(generic_RC->RemainingBits<0)
    Overum=TRUE;
  OverBits=-(int)(generic_RC->RemainingBits);

  /*initialize the lower bound and the upper bound for the target bits of each frame, HRD consideration*/
  prc->LowerBound  = (int)(generic_RC->RemainingBits + prc->bit_rate / prc->frame_rate);
  prc->UpperBound1 = (int)(generic_RC->RemainingBits + (prc->bit_rate * 2.048));

  /*compute the total number of bits for the current GOP*/
  AllocatedBits = (int64) floor((1 + np + nb) * prc->bit_rate / prc->frame_rate + 0.5);
  generic_RC->RemainingBits += AllocatedBits;
  prc->Np = np;
  prc->Nb = nb;

  OverDuantQp=(int)(8 * OverBits/AllocatedBits+0.5);
  prc->GOPOverdue=FALSE;

  /*field coding*/
  //generic_RC->NoGranularFieldRC = ( params->PicInterlace || !params->MbInterlace || params->basicunit != img->FrameSizeInMbs );
  if ( !params->PicInterlace && params->MbInterlace && params->basicunit == img->FrameSizeInMbs )
    generic_RC->NoGranularFieldRC = 0;
  else
    generic_RC->NoGranularFieldRC = 1;

  /*Compute InitialQp for each GOP*/
  prc->TotalPFrame=np;
  generic_RC->NumberofGOP++;
  if(generic_RC->NumberofGOP==1)
  {
    prc->MyInitialQp = params->SeinitialQP;
    prc->CurrLastQP = prc->MyInitialQp - 1; //recent change -0;
    prc->QPLastGOP   = prc->MyInitialQp;

    prc->PAveFrameQP   = prc->MyInitialQp;
    prc->m_Qc          = prc->PAveFrameQP;
    prc->FieldQPBuffer = prc->PAveFrameQP;
    prc->FrameQPBuffer = prc->PAveFrameQP;
    prc->PAverageQp    = prc->PAveFrameQP;
  }
  else
  {
    /*adaptive field/frame coding*/
    if( params->PicInterlace == ADAPTIVE_CODING || params->MbInterlace )
    {
      if (generic_RC->FieldFrame == 1)
      {
        prc->TotalQpforPPicture += prc->FrameQPBuffer;
        prc->QPLastPFrame = prc->FrameQPBuffer;
      }
      else
      {
        prc->TotalQpforPPicture += prc->FieldQPBuffer;
        prc->QPLastPFrame = prc->FieldQPBuffer;
      }
    }
    /*compute the average QP of P frames in the previous GOP*/
    prc->PAverageQp=(int)(1.0 * prc->TotalQpforPPicture / prc->NumberofPPicture+0.5);

    GOPDquant=(int)((1.0*(np+nb+1)/15.0) + 0.5);
    if(GOPDquant>2)
      GOPDquant=2;

    prc->PAverageQp -= GOPDquant;

    if (prc->PAverageQp > (prc->QPLastPFrame - 2))
      prc->PAverageQp--;

    // QP is constrained by QP of previous QP
    prc->PAverageQp = iClip3(prc->QPLastGOP - 2, prc->QPLastGOP + 2, prc->PAverageQp);
    // Also clipped within range.
    prc->PAverageQp = iClip3(img->RCMinQP,  img->RCMaxQP,  prc->PAverageQp);

    prc->MyInitialQp = prc->PAverageQp;
    prc->Pm_Qp       = prc->PAverageQp;
    prc->PAveFrameQP = prc->PAverageQp;
    prc->QPLastGOP   = prc->MyInitialQp;
    prc->PrevLastQP = prc->CurrLastQP;
    prc->CurrLastQP = prc->MyInitialQp - 1;
  }

  prc->TotalQpforPPicture=0;
  prc->NumberofPPicture=0;
  prc->NumberofBFrames=0;
}


/*!
 *************************************************************************************
 * \brief
 *    Initialize one picture
 *
 *************************************************************************************
*/
void rc_init_pict(rc_quadratic *prc, int fieldpic,int topfield,int targetcomputation, float mult)
{
  int tmp_T;

  /* compute the total number of basic units in a frame */
  if(params->MbInterlace)
    prc->TotalNumberofBasicUnit = img->FrameSizeInMbs / img->BasicUnit;
  else
    prc->TotalNumberofBasicUnit = img->FrameSizeInMbs / params->basicunit;

  img->NumberofCodedMacroBlocks = 0;

  /* Normally, the bandwidth for the VBR case is estimated by
     a congestion control algorithm. A bandwidth curve can be predefined if we only want to
     test the proposed algorithm */
  if(params->channel_type==1)
  {
    if(prc->NumberofCodedPFrame==58)
      prc->bit_rate *= 1.5;
    else if(prc->NumberofCodedPFrame==59)
      prc->PrevBitRate = prc->bit_rate;
  }

  /* predefine a target buffer level for each frame */
  if((fieldpic||topfield) && targetcomputation)
  {
    if ( img->type == P_SLICE || (params->RCUpdateMode == RC_MODE_1 && (img->number !=0)) )
    {
      /* Since the available bandwidth may vary at any time, the total number of
      bits is updated picture by picture*/
      if(prc->PrevBitRate!=prc->bit_rate)
        generic_RC->RemainingBits +=(int) floor((prc->bit_rate-prc->PrevBitRate)*(prc->Np + prc->Nb)/prc->frame_rate+0.5);

      /* predefine the  target buffer level for each picture.
      frame layer rate control */
      if(img->BasicUnit == img->FrameSizeInMbs)
      {
        if(prc->NumberofPPicture==1)
        {
          prc->TargetBufferLevel = (double) generic_RC->CurrentBufferFullness;
          prc->DeltaP = (generic_RC->CurrentBufferFullness - prc->GOPTargetBufferLevel) / (prc->TotalPFrame-1);
          prc->TargetBufferLevel -= prc->DeltaP;
        }
        else if(prc->NumberofPPicture>1)
          prc->TargetBufferLevel -= prc->DeltaP;
      }
      /* basic unit layer rate control */
      else
      {
        if(prc->NumberofCodedPFrame>0)
        {
          /* adaptive frame/field coding */
          if(((params->PicInterlace==ADAPTIVE_CODING)||(params->MbInterlace))&&(generic_RC->FieldControl==1))
            memcpy((void *)prc->FCBUPFMAD,(void *)prc->FCBUCFMAD, prc->TotalNumberofBasicUnit * sizeof(double));
          else
            memcpy((void *)prc->BUPFMAD,(void *)prc->BUCFMAD, prc->TotalNumberofBasicUnit * sizeof(double));
        }

        if(generic_RC->NumberofGOP==1)
        {
          if(prc->NumberofPPicture==1)
          {
            prc->TargetBufferLevel = (double) generic_RC->CurrentBufferFullness;
            prc->DeltaP = (generic_RC->CurrentBufferFullness - prc->GOPTargetBufferLevel)/(prc->TotalPFrame - 1);
            prc->TargetBufferLevel -= prc->DeltaP;
          }
          else if(prc->NumberofPPicture>1)
            prc->TargetBufferLevel -= prc->DeltaP;
        }
        else if(generic_RC->NumberofGOP>1)
        {
          if(prc->NumberofPPicture==0)
          {
            prc->TargetBufferLevel = (double) generic_RC->CurrentBufferFullness;
            prc->DeltaP = (generic_RC->CurrentBufferFullness - prc->GOPTargetBufferLevel) / prc->TotalPFrame;
            prc->TargetBufferLevel -= prc->DeltaP;
          }
          else if(prc->NumberofPPicture>0)
            prc->TargetBufferLevel -= prc->DeltaP;
        }
      }

      if(prc->NumberofCodedPFrame==1)
        prc->AveWp = prc->Wp;

      if((prc->NumberofCodedPFrame<8)&&(prc->NumberofCodedPFrame>1))
        prc->AveWp = (prc->AveWp + prc->Wp * (prc->NumberofCodedPFrame-1))/prc->NumberofCodedPFrame;
      else if(prc->NumberofCodedPFrame>1)
        prc->AveWp = (prc->Wp + 7 * prc->AveWp) / 8;

      // compute the average complexity of B frames
      if(params->successive_Bframe>0)
      {
        // compute the target buffer level
        prc->TargetBufferLevel += (prc->AveWp * (params->successive_Bframe + 1)*prc->bit_rate\
          /(prc->frame_rate*(prc->AveWp+prc->AveWb*params->successive_Bframe))-prc->bit_rate/prc->frame_rate);
      }
    }
    else if ( img->type == B_SLICE )
    {
      /* update the total number of bits if the bandwidth is changed*/
      if(prc->PrevBitRate != prc->bit_rate)
        generic_RC->RemainingBits +=(int) floor((prc->bit_rate-prc->PrevBitRate) * (prc->Np + prc->Nb) / prc->frame_rate+0.5);
      if(generic_RC->NumberofCodedBFrame == 1)
      {
        if(prc->NumberofCodedPFrame == 1)
        {
          prc->AveWp = prc->Wp;
        }
        prc->AveWb = prc->Wb;
      }
      else if(generic_RC->NumberofCodedBFrame > 1)
      {
        //compute the average weight
        if(generic_RC->NumberofCodedBFrame<8)
          prc->AveWb = (prc->AveWb + prc->Wb*(generic_RC->NumberofCodedBFrame-1)) / generic_RC->NumberofCodedBFrame;
        else
          prc->AveWb = (prc->Wb + 7 * prc->AveWb) / 8;
      }
    }
    /* Compute the target bit for each frame */
    if( img->type == P_SLICE || ( (img->number != 0) && (params->RCUpdateMode == RC_MODE_1 || params->RCUpdateMode == RC_MODE_3 ) ) )
    {
      /* frame layer rate control */
      if(img->BasicUnit == img->FrameSizeInMbs || (params->RCUpdateMode == RC_MODE_3) )
      {
        if(prc->NumberofCodedPFrame>0)
        {
          if (params->RCUpdateMode == RC_MODE_3)
          {
            int level_idx = (img->type == B_SLICE && params->HierarchicalCoding) ? (generic_RC->temporal_levels - 1 - gop_structure[img->b_frame_to_code-1].hierarchy_layer) : 0;
            int bitrate = (img->type == B_SLICE) ? generic_RC->RCBSliceBits[ level_idx ]
            : ( img->type == P_SLICE ? generic_RC->RCPSliceBits : generic_RC->RCISliceBits );
            int level, denom = generic_RC->NISlice * generic_RC->RCISliceBits + generic_RC->NPSlice * generic_RC->RCPSliceBits;
            if ( params->HierarchicalCoding )
            {
              for ( level = 0; level < generic_RC->temporal_levels; level++ )
                denom += generic_RC->hierNb[ level ] * generic_RC->RCBSliceBits[ level ];
            }
            else
            {
              denom += generic_RC->hierNb[0] * generic_RC->RCBSliceBits[0];
            }
            // target due to remaining bits
            prc->Target = (int) floor( (float)(1.0 * bitrate * generic_RC->RemainingBits) / (float)denom + 0.5F );
            // target given original taget rate and buffer considerations
            tmp_T  = imax(0, (int) floor( (double)bitrate - prc->GAMMAP * (generic_RC->CurrentBufferFullness-prc->TargetBufferLevel) + 0.5) );
            // translate Target rate from B or I "domain" to P domain since the P RC model is going to be used to select the QP
            // for hierarchical coding adjust the target QP to account for different temporal levels
            switch( img->type )
            {
            case B_SLICE:
              prc->Target = (int) floor( (float)prc->Target / params->RCBoverPRatio + 0.5F);
              break;
            case I_SLICE:
              prc->Target = (int) floor( (float)prc->Target / (params->RCIoverPRatio * 4.0) + 0.5F); // 4x accounts for the fact that header bits reduce the percentage of texture
              break;
            case P_SLICE:
            default:
              break;
            }
          }
          else
          {
            prc->Target = (int) floor( prc->Wp * generic_RC->RemainingBits / (prc->Np * prc->Wp + prc->Nb * prc->Wb) + 0.5);
            tmp_T  = imax(0, (int) floor(prc->bit_rate / prc->frame_rate - prc->GAMMAP * (generic_RC->CurrentBufferFullness-prc->TargetBufferLevel) + 0.5));
            prc->Target = (int) floor(prc->BETAP * (prc->Target - tmp_T) + tmp_T + 0.5);
          }
        }
      }
      /* basic unit layer rate control */
      else
      {
        if(((generic_RC->NumberofGOP == 1)&&(prc->NumberofCodedPFrame>0))
          || (generic_RC->NumberofGOP > 1))
        {
          prc->Target = (int) (floor( prc->Wp * generic_RC->RemainingBits / (prc->Np * prc->Wp + prc->Nb * prc->Wb) + 0.5));
          tmp_T  = imax(0, (int) (floor(prc->bit_rate / prc->frame_rate - prc->GAMMAP * (generic_RC->CurrentBufferFullness-prc->TargetBufferLevel) + 0.5)));
          prc->Target = (int) (floor(prc->BETAP * (prc->Target - tmp_T) + tmp_T + 0.5));
        }
      }
      prc->Target = (int)(mult * prc->Target);

      /* HRD consideration */
      if ( params->RCUpdateMode != RC_MODE_3 || img->type == P_SLICE )
        prc->Target = iClip3(prc->LowerBound, prc->UpperBound2, prc->Target);
      if((topfield) || (fieldpic && ((params->PicInterlace==ADAPTIVE_CODING)||(params->MbInterlace))))
        prc->TargetField=prc->Target;
    }
  }

  if(fieldpic || topfield)
  {
    /* frame layer rate control */
    generic_RC->NumberofHeaderBits  = 0;
    generic_RC->NumberofTextureBits = 0;

    /* basic unit layer rate control */
    if(img->BasicUnit < img->FrameSizeInMbs)
    {
      prc->TotalFrameQP = 0;
      generic_RC->NumberofBasicUnitHeaderBits  = 0;
      generic_RC->NumberofBasicUnitTextureBits = 0;
      generic_RC->TotalMADBasicUnit = 0;
      if(generic_RC->FieldControl==0)
        prc->NumberofBasicUnit = prc->TotalNumberofBasicUnit;
      else
        prc->NumberofBasicUnit = prc->TotalNumberofBasicUnit >> 1;
    }
  }

  if( ( img->type==P_SLICE || (params->RCUpdateMode == RC_MODE_1 && (img->number != 0)) ) && img->BasicUnit < img->FrameSizeInMbs && generic_RC->FieldControl == 1 )
  {
    /* top field at basic unit layer rate control */
    if(topfield)
    {
      prc->bits_topfield=0;
      prc->Target=(int)(prc->TargetField*0.6);
    }
    /* bottom field at basic unit layer rate control */
    else
    {
      prc->Target=prc->TargetField-prc->bits_topfield;
      generic_RC->NumberofBasicUnitHeaderBits=0;
      generic_RC->NumberofBasicUnitTextureBits=0;
      generic_RC->TotalMADBasicUnit=0;
      prc->NumberofBasicUnit=prc->TotalNumberofBasicUnit >> 1;
    }
  }
}

/*!
 *************************************************************************************
 * \brief
 *    update one picture after frame/field encoding
 *
 * \param nbits
 *    number of bits used for picture
 *
 *************************************************************************************
*/
void rc_update_pict(rc_quadratic *prc, int nbits)
{
  int delta_bits = (nbits - (int)floor(prc->bit_rate / prc->frame_rate + 0.5F) );
  // remaining # of bits in GOP
  generic_RC->RemainingBits -= nbits; 
  generic_RC->CurrentBufferFullness += delta_bits;

  // update the lower bound and the upper bound for the target bits of each frame, HRD consideration
  prc->LowerBound  -= (int) delta_bits;
  prc->UpperBound1 -= (int) delta_bits;
  prc->UpperBound2  = (int)(OMEGA * prc->UpperBound1);

  // update the parameters of quadratic R-D model
  if( img->type==P_SLICE || (params->RCUpdateMode == RC_MODE_1 && img->frm_number) )
  {
    updateRCModel(prc);
    if ( params->RCUpdateMode == RC_MODE_3 )
      prc->PreviousWholeFrameMAD = ComputeFrameMAD();
  }
}

/*!
 *************************************************************************************
 * \brief
 *    update one picture after coding
 *
 * \param bits
 *    number of bits used for picture
 *
 *************************************************************************************
*/

void rc_update_picture( int bits )
{
  rc_update_pict(quadratic_RC, bits);
}

int updateComplexity( rc_quadratic *prc, Boolean is_updated, int nbits )
{
  double Avem_Qc;

  /* frame layer rate control */
  if(img->BasicUnit == img->FrameSizeInMbs)
    return ((int) floor(nbits * prc->m_Qc + 0.5));
  /* basic unit layer rate control */
  else
  {
    if( is_updated )
    {
      if( generic_RC->NoGranularFieldRC == 0 || generic_RC->FieldControl == 0 )
      {
        Avem_Qc = (double)prc->TotalFrameQP / (double)prc->TotalNumberofBasicUnit;
        return ((int)floor(nbits * Avem_Qc + 0.5));
      }
    }
    else if( img->type == B_SLICE )
      return ((int) floor(nbits * prc->m_Qc + 0.5));
  }
  return 0;
}

void updatePparams( rc_quadratic *prc, int complexity )
{
  prc->Xp = complexity;
  prc->Np--;
  prc->Wp = prc->Xp;
  prc->Pm_Hp = generic_RC->NumberofHeaderBits;
  prc->NumberofCodedPFrame++;
  prc->NumberofPPicture++;
}

void updateBparams( rc_quadratic *prc, int complexity )
{
  prc->Xb = complexity;
  prc->Nb--;
  prc->Wb = prc->Xb / THETA;     
  prc->NumberofBFrames++;
  generic_RC->NumberofCodedBFrame++;
}

/*! 
 *************************************************************************************
 * \brief
 *    update after frame encoding
 *
 * \param nbits
 *    number of bits used for frame
 *
 *************************************************************************************
*/
void rc_update_pict_frame(rc_quadratic *prc, int nbits)
{
  /* update the complexity weight of I, P, B frame */  
  int complexity = 0;

  switch( params->RCUpdateMode )
  {
  case RC_MODE_0:
  case RC_MODE_2:
  default:
    complexity = updateComplexity( prc, (Boolean) (img->type == P_SLICE), nbits );
    if ( img->type == P_SLICE )
    {
      if( generic_RC->NoGranularFieldRC == 0 || generic_RC->FieldControl == 0 )
        updatePparams( prc, complexity );
      else
        generic_RC->NoGranularFieldRC = 0;
    }
    else if ( img->type == B_SLICE )
      updateBparams( prc, complexity );
    break;
  case RC_MODE_1:
    complexity = updateComplexity( prc, (Boolean) (img->number != 0), nbits );
    if ( img->number != 0 )
    {
      if( generic_RC->NoGranularFieldRC == 0 || generic_RC->FieldControl == 0 )
        updatePparams( prc, complexity );
      else
        generic_RC->NoGranularFieldRC = 0;
    }
    break;
  case RC_MODE_3:
    complexity = updateComplexity( prc, (Boolean) (img->type == P_SLICE), nbits );
    if (img->type == I_SLICE && (img->number != 0))
      generic_RC->NISlice--;

    if ( img->type == P_SLICE )
    {
      if( generic_RC->NoGranularFieldRC == 0 || generic_RC->FieldControl == 0 )
      {
        updatePparams( prc, complexity );
        generic_RC->NPSlice--;
      }
      else
        generic_RC->NoGranularFieldRC = 0;
    }
    else if ( img->type == B_SLICE )
    {
      updateBparams( prc, complexity );
      generic_RC->hierNb[ params->HierarchicalCoding ? (generic_RC->temporal_levels - 1 - gop_structure[img->b_frame_to_code-1].hierarchy_layer) : 0 ]--;
    }
    break;
  }   
}


/*!
 *************************************************************************************
 * \brief
 *    update the parameters of quadratic R-D model
 *
 *************************************************************************************
*/
void updateRCModel (rc_quadratic *prc)
{
  int n_windowSize;
  int i;
  double std = 0.0, threshold;
  int m_Nc = prc->NumberofCodedPFrame;
  Boolean MADModelFlag = FALSE;
  static Boolean m_rgRejected[RC_MODEL_HISTORY];
  static double  error       [RC_MODEL_HISTORY];

  if( img->type == P_SLICE || (params->RCUpdateMode == RC_MODE_1 && (img->number != 0)) )
  {
    /*frame layer rate control*/
    if(img->BasicUnit == img->FrameSizeInMbs)
    {
      prc->CurrentFrameMAD = ComputeFrameMAD();
      m_Nc=prc->NumberofCodedPFrame;
    }
    /*basic unit layer rate control*/
    else
    {
      /*compute the MAD of the current basic unit*/
      prc->CurrentFrameMAD = (double) ((generic_RC->TotalMADBasicUnit >> 8)/img->BasicUnit);
      generic_RC->TotalMADBasicUnit=0;

      /* compute the average number of header bits*/
      prc->CodedBasicUnit=prc->TotalNumberofBasicUnit-prc->NumberofBasicUnit;
      if(prc->CodedBasicUnit > 0)
      {
        prc->PAveHeaderBits1=(int)((double)(prc->PAveHeaderBits1*(prc->CodedBasicUnit-1)+
          generic_RC->NumberofBasicUnitHeaderBits)/prc->CodedBasicUnit+0.5);
        if(prc->PAveHeaderBits3 == 0)
          prc->PAveHeaderBits2 = prc->PAveHeaderBits1;
        else
        {
          prc->PAveHeaderBits2 = (int)((double)(prc->PAveHeaderBits1 * prc->CodedBasicUnit+
            prc->PAveHeaderBits3 * prc->NumberofBasicUnit)/prc->TotalNumberofBasicUnit+0.5);
        }
      }
      /*update the record of MADs for reference*/
      if(((params->PicInterlace == ADAPTIVE_CODING) || (params->MbInterlace)) && (generic_RC->FieldControl == 1))
        prc->FCBUCFMAD[prc->TotalNumberofBasicUnit-1-prc->NumberofBasicUnit]=prc->CurrentFrameMAD;
      else
        prc->BUCFMAD[prc->TotalNumberofBasicUnit-1-prc->NumberofBasicUnit]=prc->CurrentFrameMAD;

      if(prc->NumberofBasicUnit != 0)
        m_Nc = prc->NumberofCodedPFrame * prc->TotalNumberofBasicUnit + prc->CodedBasicUnit;
      else
        m_Nc = (prc->NumberofCodedPFrame-1) * prc->TotalNumberofBasicUnit + prc->CodedBasicUnit;
    }

    if(m_Nc > 1)
      MADModelFlag=TRUE;

    prc->PPreHeader = generic_RC->NumberofHeaderBits;
    for (i = (RC_MODEL_HISTORY-2); i > 0; i--)
    {// update the history
      prc->Pm_rgQp[i] = prc->Pm_rgQp[i - 1];
      prc->m_rgQp[i]  = prc->Pm_rgQp[i];
      prc->Pm_rgRp[i] = prc->Pm_rgRp[i - 1];
      prc->m_rgRp[i]  = prc->Pm_rgRp[i];
    }
    prc->Pm_rgQp[0] = QP2Qstep(prc->m_Qc); //*1.0/prc->CurrentFrameMAD;
    /*frame layer rate control*/
    if(img->BasicUnit == img->FrameSizeInMbs)
      prc->Pm_rgRp[0] = generic_RC->NumberofTextureBits*1.0/prc->CurrentFrameMAD;
    /*basic unit layer rate control*/
    else
      prc->Pm_rgRp[0] = generic_RC->NumberofBasicUnitTextureBits*1.0/prc->CurrentFrameMAD;

    prc->m_rgQp[0] = prc->Pm_rgQp[0];
    prc->m_rgRp[0] = prc->Pm_rgRp[0];
    prc->m_X1 = prc->Pm_X1;
    prc->m_X2 = prc->Pm_X2;

    /*compute the size of window*/
    n_windowSize = (prc->CurrentFrameMAD>prc->PreviousFrameMAD)
      ? (int)(prc->PreviousFrameMAD/prc->CurrentFrameMAD * (RC_MODEL_HISTORY-1) )
      : (int)(prc->CurrentFrameMAD/prc->PreviousFrameMAD *(RC_MODEL_HISTORY-1));
    n_windowSize=iClip3(1, m_Nc, n_windowSize);
    n_windowSize=imin(n_windowSize,prc->m_windowSize+1);
    n_windowSize=imin(n_windowSize,(RC_MODEL_HISTORY-1));

    /*update the previous window size*/
    prc->m_windowSize=n_windowSize;

    for (i = 0; i < (RC_MODEL_HISTORY-1); i++)
    {
      m_rgRejected[i] = FALSE;
    }

    // initial RD model estimator
    RCModelEstimator (prc, n_windowSize, m_rgRejected);

    n_windowSize = prc->m_windowSize;
    // remove outlier

    for (i = 0; i < (int) n_windowSize; i++)
    {
      error[i] = prc->m_X1 / prc->m_rgQp[i] + prc->m_X2 / (prc->m_rgQp[i] * prc->m_rgQp[i]) - prc->m_rgRp[i];
      std += error[i] * error[i];
    }
    threshold = (n_windowSize == 2) ? 0 : sqrt (std / n_windowSize);
    for (i = 0; i < (int) n_windowSize; i++)
    {
      if (fabs(error[i]) > threshold)
        m_rgRejected[i] = TRUE;
    }
    // always include the last data point
    m_rgRejected[0] = FALSE;

    // second RD model estimator
    RCModelEstimator (prc, n_windowSize, m_rgRejected);

    if( MADModelFlag )
      updateMADModel(prc);
    else if( img->type == P_SLICE || (params->RCUpdateMode == RC_MODE_1 && (img->number != 0)) )
      prc->PPictureMAD[0] = prc->CurrentFrameMAD;
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Model Estimator
 *
 *************************************************************************************
*/
void RCModelEstimator (rc_quadratic *prc, int n_windowSize, Boolean *m_rgRejected)
{
  int n_realSize = n_windowSize;
  int i;
  double oneSampleQ = 0;
  double a00 = 0.0, a01 = 0.0, a10 = 0.0, a11 = 0.0, b0 = 0.0, b1 = 0.0;
  double MatrixValue;
  Boolean estimateX2 = FALSE;

  for (i = 0; i < n_windowSize; i++)
  {// find the number of samples which are not rejected
    if (m_rgRejected[i])
      n_realSize--;
  }

  // default RD model estimation results
  prc->m_X1 = prc->m_X2 = 0.0;

  for (i = 0; i < n_windowSize; i++)
  {
    if (!m_rgRejected[i])
      oneSampleQ = prc->m_rgQp[i];
  }
  for (i = 0; i < n_windowSize; i++)
  {// if all non-rejected Q are the same, take 1st order model
    if ((prc->m_rgQp[i] != oneSampleQ) && !m_rgRejected[i])
      estimateX2 = TRUE;
    if (!m_rgRejected[i])
      prc->m_X1 += (prc->m_rgQp[i] * prc->m_rgRp[i]) / n_realSize;
  }

  // take 2nd order model to estimate X1 and X2
  if ((n_realSize >= 1) && estimateX2)
  {
    for (i = 0; i < n_windowSize; i++)
    {
      if (!m_rgRejected[i])
      {
        a00  = a00 + 1.0;
        a01 += 1.0 / prc->m_rgQp[i];
        a10  = a01;
        a11 += 1.0 / (prc->m_rgQp[i] * prc->m_rgQp[i]);
        b0  += prc->m_rgQp[i] * prc->m_rgRp[i];
        b1  += prc->m_rgRp[i];
      }
    }
    // solve the equation of AX = B
    MatrixValue=a00*a11-a01*a10;
    if(fabs(MatrixValue) > 0.000001)
    {
      prc->m_X1 = (b0 * a11 - b1 * a01) / MatrixValue;
      prc->m_X2 = (b1 * a00 - b0 * a10) / MatrixValue;
    }
    else
    {
      prc->m_X1 = b0 / a00;
      prc->m_X2 = 0.0;
    }
  }
  if( img->type == P_SLICE || (params->RCUpdateMode == RC_MODE_1 && (img->number != 0)) )
  {
    prc->Pm_X1 = prc->m_X1;
    prc->Pm_X2 = prc->m_X2;
  }
}

/*!
 *************************************************************************************
 * \brief
 *    update the parameters of linear prediction model
 *
 *************************************************************************************
*/
void updateMADModel (rc_quadratic *prc)
{
  int    n_windowSize;
  int    i;
  double std = 0.0, threshold;
  int    m_Nc = prc->NumberofCodedPFrame;
  static Boolean PictureRejected[RC_MODEL_HISTORY];
  static double  error          [RC_MODEL_HISTORY];

  if(prc->NumberofCodedPFrame>0)
  {
    //assert (img->type!=P_SLICE);
    /*frame layer rate control*/
    if(img->BasicUnit == img->FrameSizeInMbs)
      m_Nc = prc->NumberofCodedPFrame;
    else // basic unit layer rate control
      m_Nc=prc->NumberofCodedPFrame*prc->TotalNumberofBasicUnit+prc->CodedBasicUnit;

    for (i = (RC_MODEL_HISTORY-2); i > 0; i--)
    {// update the history
      prc->PPictureMAD[i]  = prc->PPictureMAD[i - 1];
      prc->PictureMAD[i]   = prc->PPictureMAD[i];
      prc->ReferenceMAD[i] = prc->ReferenceMAD[i-1];
    }
    prc->PPictureMAD[0] = prc->CurrentFrameMAD;
    prc->PictureMAD[0]  = prc->PPictureMAD[0];

    if(img->BasicUnit == img->FrameSizeInMbs)
      prc->ReferenceMAD[0]=prc->PictureMAD[1];
    else
    {
      if(((params->PicInterlace==ADAPTIVE_CODING)||(params->MbInterlace)) &&(generic_RC->FieldControl==1))
        prc->ReferenceMAD[0]=prc->FCBUPFMAD[prc->TotalNumberofBasicUnit-1-prc->NumberofBasicUnit];
      else
        prc->ReferenceMAD[0]=prc->BUPFMAD[prc->TotalNumberofBasicUnit-1-prc->NumberofBasicUnit];
    }
    prc->MADPictureC1 = prc->PMADPictureC1;
    prc->MADPictureC2 = prc->PMADPictureC2;

    /*compute the size of window*/
    n_windowSize = (prc->CurrentFrameMAD > prc->PreviousFrameMAD)
      ? (int) ((float)(RC_MODEL_HISTORY-1) * prc->PreviousFrameMAD / prc->CurrentFrameMAD)
      : (int) ((float)(RC_MODEL_HISTORY-1) * prc->CurrentFrameMAD / prc->PreviousFrameMAD);
    n_windowSize = iClip3(1, (m_Nc-1), n_windowSize);
    n_windowSize=imin(n_windowSize, imin(20, prc->MADm_windowSize + 1));

    /*update the previous window size*/
    prc->MADm_windowSize=n_windowSize;

    for (i = 0; i < (RC_MODEL_HISTORY-1); i++)
    {
      PictureRejected[i] = FALSE;
    }

    //update the MAD for the previous frame
    if( img->type == P_SLICE || (params->RCUpdateMode == RC_MODE_1 && (img->number != 0)) )
      prc->PreviousFrameMAD=prc->CurrentFrameMAD;

    // initial MAD model estimator
    MADModelEstimator (prc, n_windowSize, PictureRejected);

    // remove outlier
    for (i = 0; i < n_windowSize; i++)
    {
      error[i] = prc->MADPictureC1 * prc->ReferenceMAD[i] + prc->MADPictureC2 - prc->PictureMAD[i];
      std += (error[i] * error[i]);
    }

    threshold = (n_windowSize == 2) ? 0 : sqrt (std / n_windowSize);
    for (i = 0; i < n_windowSize; i++)
    {
      if (fabs(error[i]) > threshold)
        PictureRejected[i] = TRUE;
    }
    // always include the last data point
    PictureRejected[0] = FALSE;

    // second MAD model estimator
    MADModelEstimator (prc, n_windowSize, PictureRejected);
  }
}


/*!
 *************************************************************************************
 * \brief
 *    MAD mode estimator
 *
 *************************************************************************************
*/
void MADModelEstimator (rc_quadratic *prc, int n_windowSize, Boolean *PictureRejected)
{
  int    n_realSize = n_windowSize;
  int    i;
  double oneSampleQ = 0.0;
  double a00 = 0.0, a01 = 0.0, a10 = 0.0, a11 = 0.0, b0 = 0.0, b1 = 0.0;
  double MatrixValue;
  Boolean estimateX2 = FALSE;

  for (i = 0; i < n_windowSize; i++)
  {// find the number of samples which are not rejected
    if (PictureRejected[i])
      n_realSize--;
  }

  // default MAD model estimation results
  prc->MADPictureC1 = prc->MADPictureC2 = 0.0;

  for (i = 0; i < n_windowSize; i++)
  {
    if (!PictureRejected[i])
      oneSampleQ = prc->PictureMAD[i];
  }

  for (i = 0; i < n_windowSize; i++)
  {// if all non-rejected MAD are the same, take 1st order model
    if ((prc->PictureMAD[i] != oneSampleQ) && !PictureRejected[i])
      estimateX2 = TRUE;
    if (!PictureRejected[i])
      prc->MADPictureC1 += prc->PictureMAD[i] / (prc->ReferenceMAD[i]*n_realSize);
  }

  // take 2nd order model to estimate X1 and X2
  if ((n_realSize >= 1) && estimateX2)
  {
    for (i = 0; i < n_windowSize; i++)
    {
      if (!PictureRejected[i])
      {
        a00  = a00 + 1.0;
        a01 += prc->ReferenceMAD[i];
        a10  = a01;
        a11 += prc->ReferenceMAD[i] * prc->ReferenceMAD[i];
        b0  += prc->PictureMAD[i];
        b1  += prc->PictureMAD[i]   * prc->ReferenceMAD[i];
      }
    }
    // solve the equation of AX = B
    MatrixValue = a00 * a11 - a01 * a10;
    if(fabs(MatrixValue) > 0.000001)
    {
      prc->MADPictureC2 = (b0 * a11 - b1 * a01) / MatrixValue;
      prc->MADPictureC1 = (b1 * a00 - b0 * a10) / MatrixValue;
    }
    else
    {
      prc->MADPictureC1 = b0/a01;
      prc->MADPictureC2 = 0.0;
    }
  }
  if( img->type == P_SLICE || (params->RCUpdateMode == RC_MODE_1 && (img->number != 0)) )
  {
    prc->PMADPictureC1 = prc->MADPictureC1;
    prc->PMADPictureC2 = prc->MADPictureC2;
  }
}

/*!
 *************************************************************************************
 * \brief
 *    compute a  quantization parameter for each frame (RC_MODE_0)
 *
 *************************************************************************************
*/
int updateQPRC0(rc_quadratic *prc, int topfield)
{
  int m_Bits;
  int BFrameNumber;
  int StepSize;
  int SumofBasicUnit;
  int MaxQpChange, m_Qp, m_Hp;

  /* frame layer rate control */
  if( img->BasicUnit == img->FrameSizeInMbs )
  {
    /* fixed quantization parameter is used to coded I frame, the first P frame and the first B frame
    the quantization parameter is adjusted according the available channel bandwidth and
    the type of video */
    /*top field*/
    if((topfield) || (generic_RC->FieldControl==0))
    {
      if (img->type==I_SLICE)
      {
        prc->m_Qc = prc->MyInitialQp;
        return prc->m_Qc;
      }
      else if(img->type == B_SLICE)
      {
        if(params->successive_Bframe==1)
        {
          if((params->PicInterlace==ADAPTIVE_CODING)||(params->MbInterlace))
            updateQPInterlace( prc );

          prc->m_Qc = imin(prc->PrevLastQP, prc->CurrLastQP) + 2;
          prc->m_Qc = imax(prc->m_Qc, imax(prc->PrevLastQP, prc->CurrLastQP));
          prc->m_Qc = imax(prc->m_Qc, prc->CurrLastQP + 1);
          prc->m_Qc = iClip3(img->RCMinQP, img->RCMaxQP, prc->m_Qc); // Clipping
        }
        else
        {
          BFrameNumber = (prc->NumberofBFrames + 1) % params->successive_Bframe;
          if(BFrameNumber==0)
            BFrameNumber = params->successive_Bframe;

          /*adaptive field/frame coding*/
          if(BFrameNumber==1)
          {
            if((params->PicInterlace==ADAPTIVE_CODING)||(params->MbInterlace))
              updateQPInterlace( prc );
          }

          if((prc->CurrLastQP-prc->PrevLastQP)<=(-2*params->successive_Bframe-3))
            StepSize=-3;
          else  if((prc->CurrLastQP-prc->PrevLastQP)==(-2*params->successive_Bframe-2))
            StepSize=-2;
          else if((prc->CurrLastQP-prc->PrevLastQP)==(-2*params->successive_Bframe-1))
            StepSize=-1;
          else if((prc->CurrLastQP-prc->PrevLastQP)==(-2*params->successive_Bframe))
            StepSize=0;
          else if((prc->CurrLastQP-prc->PrevLastQP)==(-2*params->successive_Bframe+1))
            StepSize=1;
          else
            StepSize=2;

          prc->m_Qc  = prc->PrevLastQP + StepSize;
          prc->m_Qc += iClip3( -2 * (BFrameNumber - 1), 2*(BFrameNumber-1),
            (BFrameNumber-1)*(prc->CurrLastQP-prc->PrevLastQP)/(params->successive_Bframe-1));
          prc->m_Qc  = iClip3(img->RCMinQP, img->RCMaxQP, prc->m_Qc); // Clipping
        }
        return prc->m_Qc;
      }
      else if( img->type == P_SLICE && prc->NumberofPPicture == 0 )
      {
        prc->m_Qc=prc->MyInitialQp;

        if(generic_RC->FieldControl==0)
          updateQPNonPicAFF( prc );
        return prc->m_Qc;
      }
      else
      {
        /*adaptive field/frame coding*/
        if( ( params->PicInterlace == ADAPTIVE_CODING || params->MbInterlace ) && generic_RC->FieldControl == 0 )
          updateQPInterlaceBU( prc );

        prc->m_X1 = prc->Pm_X1;
        prc->m_X2 = prc->Pm_X2;
        prc->MADPictureC1 = prc->PMADPictureC1;
        prc->MADPictureC2 = prc->PMADPictureC2;
        prc->PreviousPictureMAD = prc->PPictureMAD[0];

        MaxQpChange = prc->PMaxQpChange;
        m_Qp = prc->Pm_Qp;
        m_Hp = prc->PPreHeader;

        /* predict the MAD of current picture*/
        prc->CurrentFrameMAD = prc->MADPictureC1*prc->PreviousPictureMAD + prc->MADPictureC2;

        /*compute the number of bits for the texture*/
        if(prc->Target < 0)
        {
          prc->m_Qc=m_Qp+MaxQpChange;
          prc->m_Qc = iClip3(img->RCMinQP, img->RCMaxQP, prc->m_Qc); // Clipping
        }
        else
        {
          m_Bits = prc->Target-m_Hp;
          m_Bits = imax(m_Bits, (int)(prc->bit_rate/(MINVALUE*prc->frame_rate)));

          updateModelQPFrame( prc, m_Bits );

          prc->m_Qc = iClip3(img->RCMinQP, img->RCMaxQP, prc->m_Qc); // clipping
          prc->m_Qc = iClip3(m_Qp-MaxQpChange, m_Qp+MaxQpChange, prc->m_Qc); // control variation
        }

        if( generic_RC->FieldControl == 0 )
          updateQPNonPicAFF( prc );

        return prc->m_Qc;
      }
    }
    /*bottom field*/
    else
    {
      if( img->type == P_SLICE && generic_RC->NoGranularFieldRC == 0 )
        updateBottomField( prc );
      return prc->m_Qc;
    }
  }
  /*basic unit layer rate control*/
  else
  {
    /*top field of I frame*/
    if (img->type == I_SLICE)
    {
      prc->m_Qc = prc->MyInitialQp;
      return prc->m_Qc;
    }
    else if( img->type == B_SLICE )
    {
      /*top field of B frame*/
      if((topfield)||(generic_RC->FieldControl==0))
      {
        if(params->successive_Bframe==1)
        {
          /*adaptive field/frame coding*/
          if((params->PicInterlace==ADAPTIVE_CODING)||(params->MbInterlace))
            updateQPInterlace( prc );

          if(prc->PrevLastQP==prc->CurrLastQP)
            prc->m_Qc=prc->PrevLastQP+2;
          else
            prc->m_Qc=(prc->PrevLastQP+prc->CurrLastQP)/2+1;
          prc->m_Qc = iClip3(img->RCMinQP, img->RCMaxQP, prc->m_Qc); // Clipping
        }
        else
        {
          BFrameNumber=(prc->NumberofBFrames+1)%params->successive_Bframe;
          if(BFrameNumber==0)
            BFrameNumber=params->successive_Bframe;

          /*adaptive field/frame coding*/
          if(BFrameNumber==1)
          {
            if((params->PicInterlace==ADAPTIVE_CODING)||(params->MbInterlace))
              updateQPInterlace( prc );
          }

          if((prc->CurrLastQP-prc->PrevLastQP)<=(-2*params->successive_Bframe-3))
            StepSize=-3;
          else  if((prc->CurrLastQP-prc->PrevLastQP)==(-2*params->successive_Bframe-2))
            StepSize=-2;
          else if((prc->CurrLastQP-prc->PrevLastQP)==(-2*params->successive_Bframe-1))
            StepSize=-1;
          else if((prc->CurrLastQP-prc->PrevLastQP)==(-2*params->successive_Bframe))
            StepSize=0;//0
          else if((prc->CurrLastQP-prc->PrevLastQP)==(-2*params->successive_Bframe+1))
            StepSize=1;//1
          else
            StepSize=2;//2
          prc->m_Qc=prc->PrevLastQP+StepSize;
          prc->m_Qc +=
            iClip3( -2*(BFrameNumber-1), 2*(BFrameNumber-1), (BFrameNumber-1)*(prc->CurrLastQP-prc->PrevLastQP)/(params->successive_Bframe-1) );
          prc->m_Qc = iClip3(img->RCMinQP, img->RCMaxQP, prc->m_Qc); // Clipping
        }
        return prc->m_Qc;
      }
      /*bottom field of B frame*/
      else
      {
        return prc->m_Qc;
      }
    }
    else if( img->type == P_SLICE )
    {
      if( (generic_RC->NumberofGOP == 1) && (prc->NumberofPPicture == 0) )
      {
        if((generic_RC->FieldControl==0)||((generic_RC->FieldControl==1) && (generic_RC->NoGranularFieldRC==0)))
          return updateFirstP( prc, topfield );
      }
      else
      {
        prc->m_X1=prc->Pm_X1;
        prc->m_X2=prc->Pm_X2;
        prc->MADPictureC1=prc->PMADPictureC1;
        prc->MADPictureC2=prc->PMADPictureC2;

        m_Qp=prc->Pm_Qp;

        if(generic_RC->FieldControl==0)
          SumofBasicUnit=prc->TotalNumberofBasicUnit;
        else
          SumofBasicUnit=prc->TotalNumberofBasicUnit>>1;

        /*the average QP of the previous frame is used to coded the first basic unit of the current frame or field*/
        if(prc->NumberofBasicUnit==SumofBasicUnit)
          return updateFirstBU( prc, topfield );
        else
        {
          /*compute the number of remaining bits*/
          prc->Target -= (generic_RC->NumberofBasicUnitHeaderBits + generic_RC->NumberofBasicUnitTextureBits);
          generic_RC->NumberofBasicUnitHeaderBits  = 0;
          generic_RC->NumberofBasicUnitTextureBits = 0;
          if(prc->Target<0)
            return updateNegativeTarget( prc, topfield, m_Qp );
          else
          {
            /*predict the MAD of current picture*/
            predictCurrPicMAD( prc );

            /*compute the total number of bits for the current basic unit*/
            updateModelQPBU( prc, topfield, m_Qp );

            prc->TotalFrameQP +=prc->m_Qc;
            prc->Pm_Qp=prc->m_Qc;
            prc->NumberofBasicUnit--;
            if( prc->NumberofBasicUnit == 0 && img->type == P_SLICE )
              updateLastBU( prc, topfield );

            return prc->m_Qc;
          }
        }
      }
    }
  }
  return prc->m_Qc;
}

/*!
 *************************************************************************************
 * \brief
 *    compute a  quantization parameter for each frame
 *
 *************************************************************************************
*/
int updateQPRC1(rc_quadratic *prc, int topfield)
{
  int m_Bits;
  int SumofBasicUnit;
  int MaxQpChange, m_Qp, m_Hp;

  /* frame layer rate control */
  if( img->BasicUnit == img->FrameSizeInMbs )
  {
    /* fixed quantization parameter is used to coded I frame, the first P frame and the first B frame
    the quantization parameter is adjusted according the available channel bandwidth and
    the type of vide */
    /*top field*/
    if((topfield) || (generic_RC->FieldControl==0))
    {
      if (img->number == 0)
      {
        prc->m_Qc = prc->MyInitialQp;
        return prc->m_Qc;
      }
      else if( prc->NumberofPPicture == 0 && (img->number != 0))
      {
        prc->m_Qc=prc->MyInitialQp;

        if(generic_RC->FieldControl==0)
          updateQPNonPicAFF( prc );
        return prc->m_Qc;
      }
      else
      {
        /*adaptive field/frame coding*/
        if( ( params->PicInterlace == ADAPTIVE_CODING || params->MbInterlace ) && generic_RC->FieldControl == 0 )
          updateQPInterlaceBU( prc );

        prc->m_X1 = prc->Pm_X1;
        prc->m_X2 = prc->Pm_X2;
        prc->MADPictureC1 = prc->PMADPictureC1;
        prc->MADPictureC2 = prc->PMADPictureC2;
        prc->PreviousPictureMAD = prc->PPictureMAD[0];

        MaxQpChange = prc->PMaxQpChange;
        m_Qp = prc->Pm_Qp;
        m_Hp = prc->PPreHeader;

        /* predict the MAD of current picture*/
        prc->CurrentFrameMAD=prc->MADPictureC1*prc->PreviousPictureMAD + prc->MADPictureC2;

        /*compute the number of bits for the texture*/
        if(prc->Target < 0)
        {
          prc->m_Qc=m_Qp+MaxQpChange;
          prc->m_Qc = iClip3(img->RCMinQP, img->RCMaxQP, prc->m_Qc); // Clipping
        }
        else
        {
          m_Bits = prc->Target-m_Hp;
          m_Bits = imax(m_Bits, (int)(prc->bit_rate/(MINVALUE*prc->frame_rate)));

          updateModelQPFrame( prc, m_Bits );

          prc->m_Qc = iClip3(img->RCMinQP, img->RCMaxQP, prc->m_Qc); // clipping
          prc->m_Qc = iClip3(m_Qp-MaxQpChange, m_Qp+MaxQpChange, prc->m_Qc); // control variation
        }

        if( generic_RC->FieldControl == 0 )
          updateQPNonPicAFF( prc );

        return prc->m_Qc;
      }
    }
    /*bottom field*/
    else
    {
      if( generic_RC->NoGranularFieldRC == 0 )
        updateBottomField( prc );
      return prc->m_Qc;
    }
  }
  /*basic unit layer rate control*/
  else
  {
    /*top field of I frame*/
    if (img->number == 0)
    {
      prc->m_Qc = prc->MyInitialQp;
      return prc->m_Qc;
    }
    else
    {
      if((generic_RC->NumberofGOP==1)&&(prc->NumberofPPicture==0))
      {
        if((generic_RC->FieldControl==0)||((generic_RC->FieldControl==1) && (generic_RC->NoGranularFieldRC==0)))
          return updateFirstP( prc, topfield );
      }
      else
      {
        prc->m_X1=prc->Pm_X1;
        prc->m_X2=prc->Pm_X2;
        prc->MADPictureC1=prc->PMADPictureC1;
        prc->MADPictureC2=prc->PMADPictureC2;

        m_Qp=prc->Pm_Qp;

        if(generic_RC->FieldControl==0)
          SumofBasicUnit=prc->TotalNumberofBasicUnit;
        else
          SumofBasicUnit=prc->TotalNumberofBasicUnit>>1;

        /*the average QP of the previous frame is used to coded the first basic unit of the current frame or field*/
        if(prc->NumberofBasicUnit==SumofBasicUnit)
          return updateFirstBU( prc, topfield );
        else
        {
          /*compute the number of remaining bits*/
          prc->Target -= (generic_RC->NumberofBasicUnitHeaderBits + generic_RC->NumberofBasicUnitTextureBits);
          generic_RC->NumberofBasicUnitHeaderBits  = 0;
          generic_RC->NumberofBasicUnitTextureBits = 0;
          if(prc->Target<0)
            return updateNegativeTarget( prc, topfield, m_Qp );
          else
          {
            /*predict the MAD of current picture*/
            predictCurrPicMAD( prc );

            /*compute the total number of bits for the current basic unit*/
            updateModelQPBU( prc, topfield, m_Qp );

            prc->TotalFrameQP +=prc->m_Qc;
            prc->Pm_Qp=prc->m_Qc;
            prc->NumberofBasicUnit--;
            if((prc->NumberofBasicUnit==0) && (img->number != 0))
              updateLastBU( prc, topfield );

            return prc->m_Qc;
          }
        }
      }
    }
  }
  return prc->m_Qc;
}

/*!
 *************************************************************************************
 * \brief
 *    compute a  quantization parameter for each frame
 *
 *************************************************************************************
*/
int updateQPRC2(rc_quadratic *prc, int topfield)
{
  int m_Bits;
  int SumofBasicUnit;
  int MaxQpChange, m_Qp, m_Hp;

  /* frame layer rate control */
  if( img->BasicUnit == img->FrameSizeInMbs )
  {
    /* fixed quantization parameter is used to coded I frame, the first P frame and the first B frame
    the quantization parameter is adjusted according the available channel bandwidth and
    the type of vide */
    /*top field*/
    if((topfield) || (generic_RC->FieldControl==0))
    {
      if (img->number == 0)
      {
        prc->m_Qc = prc->MyInitialQp;
        return prc->m_Qc;
      }
      else if (img->type==I_SLICE)
      {
        if((params->PicInterlace==ADAPTIVE_CODING)||(params->MbInterlace))
          updateQPInterlace( prc );

        prc->m_Qc = prc->CurrLastQP; // Set QP to average qp of last P frame
        return prc->m_Qc;
      }
      else if(img->type == B_SLICE)
      {
        int prevQP = imax(prc->PrevLastQP, prc->CurrLastQP);
        // for more than one consecutive B frames the below call will overwrite the old anchor frame QP with the current value
        // it should be called once in the B-frame sequence....this should be modified for the BU < frame as well
        if((params->PicInterlace==ADAPTIVE_CODING)||(params->MbInterlace))
          updateQPInterlace( prc );

        if (params->HierarchicalCoding)
        {
          if (img->b_frame_to_code == 0)
            prc->m_Qc = prevQP;
          else
            prc->m_Qc = prevQP + img->GopLevels - gop_structure[img->b_frame_to_code-1].hierarchy_layer;
        }
        else
          prc->m_Qc = prevQP + 2 - img->nal_reference_idc;
        prc->m_Qc = iClip3(img->RCMinQP, img->RCMaxQP, prc->m_Qc); // Clipping

        return prc->m_Qc;
      }
      else if( img->type == P_SLICE && prc->NumberofPPicture == 0 )
      {
        prc->m_Qc=prc->MyInitialQp;

        if(generic_RC->FieldControl==0)
          updateQPNonPicAFF( prc );
        return prc->m_Qc;
      }
      else
      {
        /*adaptive field/frame coding*/
        if( ( params->PicInterlace == ADAPTIVE_CODING || params->MbInterlace ) && generic_RC->FieldControl == 0 )
          updateQPInterlaceBU( prc );

        prc->m_X1 = prc->Pm_X1;
        prc->m_X2 = prc->Pm_X2;
        prc->MADPictureC1 = prc->PMADPictureC1;
        prc->MADPictureC2 = prc->PMADPictureC2;
        prc->PreviousPictureMAD = prc->PPictureMAD[0];

        MaxQpChange = prc->PMaxQpChange;
        m_Qp = prc->Pm_Qp;
        m_Hp = prc->PPreHeader;

        /* predict the MAD of current picture*/
        prc->CurrentFrameMAD=prc->MADPictureC1*prc->PreviousPictureMAD + prc->MADPictureC2;

        /*compute the number of bits for the texture*/
        if(prc->Target < 0)
        {
          prc->m_Qc=m_Qp+MaxQpChange;
          prc->m_Qc = iClip3(img->RCMinQP, img->RCMaxQP, prc->m_Qc); // Clipping
        }
        else
        {
          m_Bits = prc->Target-m_Hp;
          m_Bits = imax(m_Bits, (int)(prc->bit_rate/(MINVALUE*prc->frame_rate)));

          updateModelQPFrame( prc, m_Bits );

          prc->m_Qc = iClip3(img->RCMinQP, img->RCMaxQP, prc->m_Qc); // clipping
          prc->m_Qc = iClip3(m_Qp-MaxQpChange, m_Qp+MaxQpChange, prc->m_Qc); // control variation
        }

        if( generic_RC->FieldControl == 0 )
          updateQPNonPicAFF( prc );

        return prc->m_Qc;
      }
    }
    /*bottom field*/
    else
    {
      if( img->type==P_SLICE && generic_RC->NoGranularFieldRC == 0 )
        updateBottomField( prc );
      return prc->m_Qc;
    }
  }
  /*basic unit layer rate control*/
  else
  {
    /*top field of I frame*/
    if (img->number == 0)
    {
      prc->m_Qc = prc->MyInitialQp;
      return prc->m_Qc;
    }
    else if (img->type==I_SLICE)
    {
      /*adaptive field/frame coding*/
      if((params->PicInterlace==ADAPTIVE_CODING)||(params->MbInterlace))
        updateQPInterlace( prc );

      prc->m_Qc = prc->PrevLastQP; // Set QP to average qp of last P frame
      prc->PrevLastQP = prc->CurrLastQP;
      prc->CurrLastQP = prc->PrevLastQP;
      prc->PAveFrameQP = prc->CurrLastQP;

      return prc->m_Qc;
    }
    else if(img->type == B_SLICE)
    {
      int prevQP = imax(prc->PrevLastQP, prc->CurrLastQP);
      if((params->PicInterlace==ADAPTIVE_CODING)||(params->MbInterlace))
        updateQPInterlace( prc );

      if (params->HierarchicalCoding)
      {

        if (img->b_frame_to_code == 0)
          prc->m_Qc = prevQP;
        else
          prc->m_Qc = prevQP + img->GopLevels - gop_structure[img->b_frame_to_code-1].hierarchy_layer;
      }
      else
        prc->m_Qc = prevQP + 2 - img->nal_reference_idc;
      prc->m_Qc = iClip3(img->RCMinQP, img->RCMaxQP, prc->m_Qc); // Clipping

      return prc->m_Qc;

    }
    else if( img->type == P_SLICE )
    {
      if((generic_RC->NumberofGOP==1)&&(prc->NumberofPPicture==0))
      {
        if((generic_RC->FieldControl==0)||((generic_RC->FieldControl==1) && (generic_RC->NoGranularFieldRC==0)))
          return updateFirstP( prc, topfield );
      }
      else
      {
        prc->m_X1=prc->Pm_X1;
        prc->m_X2=prc->Pm_X2;
        prc->MADPictureC1=prc->PMADPictureC1;
        prc->MADPictureC2=prc->PMADPictureC2;

        m_Qp=prc->Pm_Qp;

        if(generic_RC->FieldControl==0)
          SumofBasicUnit=prc->TotalNumberofBasicUnit;
        else
          SumofBasicUnit=prc->TotalNumberofBasicUnit>>1;

        /*the average QP of the previous frame is used to coded the first basic unit of the current frame or field*/
        if(prc->NumberofBasicUnit==SumofBasicUnit)
          return updateFirstBU( prc, topfield );
        else
        {
          /*compute the number of remaining bits*/
          prc->Target -= (generic_RC->NumberofBasicUnitHeaderBits + generic_RC->NumberofBasicUnitTextureBits);
          generic_RC->NumberofBasicUnitHeaderBits  = 0;
          generic_RC->NumberofBasicUnitTextureBits = 0;
          if(prc->Target<0)
            return updateNegativeTarget( prc, topfield, m_Qp );
          else
          {
            /*predict the MAD of current picture*/
            predictCurrPicMAD( prc );

            /*compute the total number of bits for the current basic unit*/
            updateModelQPBU( prc, topfield, m_Qp );

            prc->TotalFrameQP +=prc->m_Qc;
            prc->Pm_Qp=prc->m_Qc;
            prc->NumberofBasicUnit--;
            if((prc->NumberofBasicUnit==0) && img->type == P_SLICE )
              updateLastBU( prc, topfield );

            return prc->m_Qc;
          }
        }
      }
    }
  }
  return prc->m_Qc;
}

/*!
 *************************************************************************************
 * \brief
 *    compute a  quantization parameter for each frame
 *
 *************************************************************************************
*/
int updateQPRC3(rc_quadratic *prc, int topfield)
{
  int m_Bits;
  int SumofBasicUnit;
  int MaxQpChange, m_Qp, m_Hp;

  /* frame layer rate control */
  if( img->BasicUnit == img->FrameSizeInMbs || img->type != P_SLICE )
  {
    /* fixed quantization parameter is used to coded I frame, the first P frame and the first B frame
    the quantization parameter is adjusted according the available channel bandwidth and
    the type of video */
    /*top field*/
    if((topfield) || (generic_RC->FieldControl==0))
    {
      if (img->number == 0)
      {
        if((params->PicInterlace == ADAPTIVE_CODING) || (params->MbInterlace))
          updateQPInterlace( prc );
        prc->m_Qc = prc->MyInitialQp;
        return prc->m_Qc;
      }
      else if( img->type == P_SLICE && prc->NumberofPPicture == 0 )
      {
        prc->m_Qc=prc->MyInitialQp;

        if(generic_RC->FieldControl==0)
          updateQPNonPicAFF( prc );
        return prc->m_Qc;
      }
      else
      {
        if( ( (img->type == B_SLICE && img->b_frame_to_code == 1) || img->type == I_SLICE) && ((params->PicInterlace == ADAPTIVE_CODING) || (params->MbInterlace)) )
          updateQPInterlace( prc );
        /*adaptive field/frame coding*/
        if( img->type == P_SLICE && ( params->PicInterlace == ADAPTIVE_CODING || params->MbInterlace ) && generic_RC->FieldControl == 0 )
          updateQPInterlaceBU( prc );

        prc->m_X1 = prc->Pm_X1;
        prc->m_X2 = prc->Pm_X2;
        prc->MADPictureC1 = prc->PMADPictureC1;
        prc->MADPictureC2 = prc->PMADPictureC2;
        prc->PreviousPictureMAD = prc->PPictureMAD[0];

        MaxQpChange = prc->PMaxQpChange;
        m_Qp = prc->Pm_Qp;
        m_Hp = prc->PPreHeader;

        if ( img->BasicUnit < img->FrameSizeInMbs && img->type != P_SLICE )
        {
          // when RC_MODE_3 is set and basic unit is smaller than a frame, note that:
          // the linear MAD model and the quadratic QP model operate on small units and not on a whole frame;
          // we therefore have to account for this
          prc->PreviousPictureMAD = prc->PreviousWholeFrameMAD;
        }
        if ( img->type == I_SLICE )
          m_Hp = 0; // it is usually a very small portion of the total I_SLICE bit budget

        /* predict the MAD of current picture*/
        prc->CurrentFrameMAD=prc->MADPictureC1*prc->PreviousPictureMAD + prc->MADPictureC2;

        /*compute the number of bits for the texture*/
        if(prc->Target < 0)
        {
          prc->m_Qc=m_Qp+MaxQpChange;
          prc->m_Qc = iClip3(img->RCMinQP, img->RCMaxQP, prc->m_Qc); // Clipping
        }
        else
        {
          if ( img->type != P_SLICE )
          {
            if ( img->BasicUnit < img->FrameSizeInMbs )
              m_Bits =(prc->Target-m_Hp)/prc->TotalNumberofBasicUnit;
            else
              m_Bits =prc->Target-m_Hp;
          }
          else {
            m_Bits = prc->Target-m_Hp;
            m_Bits = imax(m_Bits, (int)(prc->bit_rate/(MINVALUE*prc->frame_rate)));
          }          
          updateModelQPFrame( prc, m_Bits );

          prc->m_Qc = iClip3(img->RCMinQP, img->RCMaxQP, prc->m_Qc); // clipping
          if ( img->type == P_SLICE )
            prc->m_Qc = iClip3(m_Qp-MaxQpChange, m_Qp+MaxQpChange, prc->m_Qc); // control variation
        }

        if( img->type == P_SLICE && generic_RC->FieldControl == 0 )
          updateQPNonPicAFF( prc );

        if ( img->type == B_SLICE )
        {
          // hierarchical adjustment
          int prevqp = ((prc->PrevLastQP+prc->CurrLastQP) >> 1) + 1;
          if ( params->HierarchicalCoding && img->b_frame_to_code)
            prc->m_Qc -= gop_structure[img->b_frame_to_code-1].hierarchy_layer;
          // check bounds
          prc->m_Qc = iClip3(prevqp - (params->HierarchicalCoding ? 0 : 5), prevqp + 5, prc->m_Qc); // control variation
          prc->m_Qc = iClip3(img->RCMinQP, img->RCMaxQP, prc->m_Qc); // clipping
        }
        return prc->m_Qc;
      }
    }
    /*bottom field*/
    else
    {
      if( img->type==P_SLICE && generic_RC->NoGranularFieldRC == 0 )
        updateBottomField( prc );
      return prc->m_Qc;
    }
  }
  /*basic unit layer rate control*/
  else
  {
    /*top field of I frame*/
    if (img->number == 0)
    {
      if((params->PicInterlace == ADAPTIVE_CODING) || (params->MbInterlace))
        updateQPInterlace( prc );
      prc->m_Qc = prc->MyInitialQp;
      return prc->m_Qc;
    }
    else if( img->type == P_SLICE )
    {
      if((generic_RC->NumberofGOP==1)&&(prc->NumberofPPicture==0))
      {
        if((generic_RC->FieldControl==0)||((generic_RC->FieldControl==1) && (generic_RC->NoGranularFieldRC==0)))
          return updateFirstP( prc, topfield );
      }
      else
      {
        if( ( (img->type == B_SLICE && img->b_frame_to_code == 1) || img->type == I_SLICE) && ((params->PicInterlace == ADAPTIVE_CODING) || (params->MbInterlace)) )
          updateQPInterlace( prc );
        prc->m_X1=prc->Pm_X1;
        prc->m_X2=prc->Pm_X2;
        prc->MADPictureC1=prc->PMADPictureC1;
        prc->MADPictureC2=prc->PMADPictureC2;

        m_Qp=prc->Pm_Qp;

        if(generic_RC->FieldControl==0)
          SumofBasicUnit=prc->TotalNumberofBasicUnit;
        else
          SumofBasicUnit=prc->TotalNumberofBasicUnit>>1;

        /*the average QP of the previous frame is used to coded the first basic unit of the current frame or field*/
        if(prc->NumberofBasicUnit==SumofBasicUnit)
          return updateFirstBU( prc, topfield );
        else
        {
          /*compute the number of remaining bits*/
          prc->Target -= (generic_RC->NumberofBasicUnitHeaderBits + generic_RC->NumberofBasicUnitTextureBits);
          generic_RC->NumberofBasicUnitHeaderBits  = 0;
          generic_RC->NumberofBasicUnitTextureBits = 0;
          if(prc->Target<0)
            return updateNegativeTarget( prc, topfield, m_Qp );
          else
          {
            /*predict the MAD of current picture*/
            predictCurrPicMAD( prc );

            /*compute the total number of bits for the current basic unit*/
            updateModelQPBU( prc, topfield, m_Qp );

            prc->TotalFrameQP +=prc->m_Qc;
            prc->Pm_Qp=prc->m_Qc;
            prc->NumberofBasicUnit--;
            if((prc->NumberofBasicUnit==0) && img->type == P_SLICE )
              updateLastBU( prc, topfield );

            return prc->m_Qc;
          }
        }
      }
    }
  }
  return prc->m_Qc;
}

/*!
 *************************************************************************************
 * \brief
 *    Save previous QP values for interlaced coding
 *
 *************************************************************************************
*/
void updateQPInterlace( rc_quadratic *prc )
{
  if(generic_RC->FieldControl==0)
  {
    /*previous choice is frame coding*/
    if(generic_RC->FieldFrame==1)
    {
      prc->PrevLastQP=prc->CurrLastQP;
      prc->CurrLastQP=prc->FrameQPBuffer;
    }
    /*previous choice is field coding*/
    else
    {
      prc->PrevLastQP=prc->CurrLastQP;
      prc->CurrLastQP=prc->FieldQPBuffer;
    }
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Save previous QP values for the case of non-PicAFF
 *
 *************************************************************************************
*/
void updateQPNonPicAFF( rc_quadratic *prc )
{
  if(active_sps->frame_mbs_only_flag)
  {
    prc->TotalQpforPPicture +=prc->m_Qc;
    prc->PrevLastQP=prc->CurrLastQP;
    prc->CurrLastQP=prc->m_Qc;
    prc->Pm_Qp=prc->m_Qc;
  }
  /*adaptive field/frame coding*/
  else
    prc->FrameQPBuffer=prc->m_Qc;
}

/*!
 *************************************************************************************
 * \brief
 *    Update QP values for bottom filed in field coding
 *************************************************************************************
*/
void updateBottomField( rc_quadratic *prc )
{
  /*field coding*/
  if(params->PicInterlace==FIELD_CODING)
  {
    prc->TotalQpforPPicture +=prc->m_Qc;
    prc->PrevLastQP=prc->CurrLastQP+1;
    prc->CurrLastQP=prc->m_Qc;//+0 Recent change 13/1/2003
    prc->Pm_Qp=prc->m_Qc;
  }
  /*adaptive field/frame coding*/
  else
    prc->FieldQPBuffer=prc->m_Qc;
}

/*!
 *************************************************************************************
 * \brief
 *    update QP variables for P frames
 *************************************************************************************
*/
int updateFirstP( rc_quadratic *prc, int topfield )
{
  /*top field of the first P frame*/
  prc->m_Qc=prc->MyInitialQp;
  generic_RC->NumberofBasicUnitHeaderBits=0;
  generic_RC->NumberofBasicUnitTextureBits=0;
  prc->NumberofBasicUnit--;
  /*bottom field of the first P frame*/
  if((!topfield)&&(prc->NumberofBasicUnit==0))
  {
    /*frame coding or field coding*/
    if((active_sps->frame_mbs_only_flag)||(params->PicInterlace==FIELD_CODING))
    {
      prc->TotalQpforPPicture +=prc->m_Qc;
      prc->PrevLastQP=prc->CurrLastQP;
      prc->CurrLastQP=prc->m_Qc;
      prc->PAveFrameQP=prc->m_Qc;
      prc->PAveHeaderBits3=prc->PAveHeaderBits2;
    }
    /*adaptive frame/field coding*/
    else if((params->PicInterlace==ADAPTIVE_CODING)||(params->MbInterlace))
    {
      if(generic_RC->FieldControl==0)
      {
        prc->FrameQPBuffer=prc->m_Qc;
        prc->FrameAveHeaderBits=prc->PAveHeaderBits2;
      }
      else
      {
        prc->FieldQPBuffer=prc->m_Qc;
        prc->FieldAveHeaderBits=prc->PAveHeaderBits2;
      }
    }
  }
  prc->Pm_Qp=prc->m_Qc;
  prc->TotalFrameQP +=prc->m_Qc;
  return prc->m_Qc;
}

/*!
 *************************************************************************************
 * \brief
 *    update QP when bit target is negative
 *************************************************************************************
*/
int updateNegativeTarget( rc_quadratic *prc, int topfield, int m_Qp )
{
  int PAverageQP;

  if(prc->GOPOverdue==TRUE)
    prc->m_Qc=m_Qp+2;
  else
    prc->m_Qc=m_Qp+prc->DDquant;//2

  prc->m_Qc = imin(prc->m_Qc, img->RCMaxQP);  // clipping
  if(params->basicunit>=prc->MBPerRow)
    prc->m_Qc = imin(prc->m_Qc, prc->PAveFrameQP + 6);
  else
    prc->m_Qc = imin(prc->m_Qc, prc->PAveFrameQP + 3);

  prc->TotalFrameQP +=prc->m_Qc;
  prc->NumberofBasicUnit--;
  if(prc->NumberofBasicUnit==0)
  {
    if((!topfield)||(generic_RC->FieldControl==0))
    {
      /*frame coding or field coding*/
      if((active_sps->frame_mbs_only_flag)||(params->PicInterlace==FIELD_CODING))
      {
        PAverageQP=(int)((double)prc->TotalFrameQP/(double)prc->TotalNumberofBasicUnit+0.5);
        if (prc->NumberofPPicture == (params->intra_period - 2))
          prc->QPLastPFrame = PAverageQP;

        prc->TotalQpforPPicture +=PAverageQP;
        if(prc->GOPOverdue==TRUE)
        {
          prc->PrevLastQP=prc->CurrLastQP+1;
          prc->CurrLastQP=PAverageQP;
        }
        else
        {
          if((prc->NumberofPPicture==0)&&(generic_RC->NumberofGOP>1))
          {
            prc->PrevLastQP=prc->CurrLastQP;
            prc->CurrLastQP=PAverageQP;
          }
          else if(prc->NumberofPPicture>0)
          {
            prc->PrevLastQP=prc->CurrLastQP+1;
            prc->CurrLastQP=PAverageQP;
          }
        }
        prc->PAveFrameQP=PAverageQP;
        prc->PAveHeaderBits3=prc->PAveHeaderBits2;
      }
      /*adaptive field/frame coding*/
      else if((params->PicInterlace==ADAPTIVE_CODING)||(params->MbInterlace))
      {
        if(generic_RC->FieldControl==0)
        {
          PAverageQP=(int)((double)prc->TotalFrameQP/(double)prc->TotalNumberofBasicUnit+0.5);
          prc->FrameQPBuffer=PAverageQP;
          prc->FrameAveHeaderBits=prc->PAveHeaderBits2;
        }
        else
        {
          PAverageQP=(int)((double)prc->TotalFrameQP/(double)prc->TotalNumberofBasicUnit+0.5);
          prc->FieldQPBuffer=PAverageQP;
          prc->FieldAveHeaderBits=prc->PAveHeaderBits2;
        }
      }
    }
  }
  if(prc->GOPOverdue==TRUE)
    prc->Pm_Qp=prc->PAveFrameQP;
  else
    prc->Pm_Qp=prc->m_Qc;

  return prc->m_Qc;
}

/*!
 *************************************************************************************
 * \brief
 *    update QP for the first Basic Unit in the picture
 *************************************************************************************
*/
int updateFirstBU( rc_quadratic *prc, int topfield )
{
  /*adaptive field/frame coding*/
  if(((params->PicInterlace==ADAPTIVE_CODING)||(params->MbInterlace))&&(generic_RC->FieldControl==0))
  {
    /*previous choice is frame coding*/
    if(generic_RC->FieldFrame==1)
    {
      if(prc->NumberofPPicture>0)
        prc->TotalQpforPPicture +=prc->FrameQPBuffer;
      prc->PAveFrameQP=prc->FrameQPBuffer;
      prc->PAveHeaderBits3=prc->FrameAveHeaderBits;
    }
    /*previous choice is field coding*/
    else
    {
      if(prc->NumberofPPicture>0)
        prc->TotalQpforPPicture +=prc->FieldQPBuffer;
      prc->PAveFrameQP=prc->FieldQPBuffer;
      prc->PAveHeaderBits3=prc->FieldAveHeaderBits;
    }
  }

  if(prc->Target<=0)
  {
    prc->m_Qc = prc->PAveFrameQP + 2;
    if(prc->m_Qc > img->RCMaxQP)
      prc->m_Qc = img->RCMaxQP;

    if(topfield||(generic_RC->FieldControl==0))
      prc->GOPOverdue=TRUE;
  }
  else
  {
    prc->m_Qc=prc->PAveFrameQP;
  }
  prc->TotalFrameQP +=prc->m_Qc;
  prc->NumberofBasicUnit--;
  prc->Pm_Qp = prc->PAveFrameQP;

  return prc->m_Qc;
}

/*!
 *************************************************************************************
 * \brief
 *    update QP for the last Basic Unit in the picture
 *************************************************************************************
*/
void updateLastBU( rc_quadratic *prc, int topfield )
{
  int PAverageQP;

  if((!topfield)||(generic_RC->FieldControl==0))
  {
    /*frame coding or field coding*/
    if((active_sps->frame_mbs_only_flag)||(params->PicInterlace==FIELD_CODING))
    {
      PAverageQP=(int)((double)prc->TotalFrameQP/(double) prc->TotalNumberofBasicUnit+0.5);
      if (prc->NumberofPPicture == (params->intra_period - 2))
        prc->QPLastPFrame = PAverageQP;

      prc->TotalQpforPPicture +=PAverageQP;
      prc->PrevLastQP=prc->CurrLastQP;
      prc->CurrLastQP=PAverageQP;
      prc->PAveFrameQP=PAverageQP;
      prc->PAveHeaderBits3=prc->PAveHeaderBits2;
    }
    else if((params->PicInterlace==ADAPTIVE_CODING)||(params->MbInterlace))
    {
      if(generic_RC->FieldControl==0)
      {
        PAverageQP=(int)((double) prc->TotalFrameQP/(double)prc->TotalNumberofBasicUnit+0.5);
        prc->FrameQPBuffer=PAverageQP;
        prc->FrameAveHeaderBits=prc->PAveHeaderBits2;
      }
      else
      {
        PAverageQP=(int)((double) prc->TotalFrameQP/(double) prc->TotalNumberofBasicUnit+0.5);
        prc->FieldQPBuffer=PAverageQP;
        prc->FieldAveHeaderBits=prc->PAveHeaderBits2;
      }
    }
  }
}

/*!
 *************************************************************************************
 * \brief
 *    update current picture MAD
 *************************************************************************************
*/
void predictCurrPicMAD( rc_quadratic *prc )
{
  int i;
  if(((params->PicInterlace==ADAPTIVE_CODING)||(params->MbInterlace))&&(generic_RC->FieldControl==1))
  {
    prc->CurrentFrameMAD=prc->MADPictureC1*prc->FCBUPFMAD[prc->TotalNumberofBasicUnit-prc->NumberofBasicUnit]+prc->MADPictureC2;
    prc->TotalBUMAD=0;
    for(i=prc->TotalNumberofBasicUnit-1; i>=(prc->TotalNumberofBasicUnit-prc->NumberofBasicUnit);i--)
    {
      prc->CurrentBUMAD=prc->MADPictureC1*prc->FCBUPFMAD[i]+prc->MADPictureC2;
      prc->TotalBUMAD +=prc->CurrentBUMAD*prc->CurrentBUMAD;
    }
  }
  else
  {
    prc->CurrentFrameMAD=prc->MADPictureC1*prc->BUPFMAD[prc->TotalNumberofBasicUnit-prc->NumberofBasicUnit]+prc->MADPictureC2;
    prc->TotalBUMAD=0;
    for(i=prc->TotalNumberofBasicUnit-1; i>=(prc->TotalNumberofBasicUnit-prc->NumberofBasicUnit);i--)
    {
      prc->CurrentBUMAD=prc->MADPictureC1*prc->BUPFMAD[i]+prc->MADPictureC2;
      prc->TotalBUMAD +=prc->CurrentBUMAD*prc->CurrentBUMAD;
    }
  }
}

/*!
 *************************************************************************************
 * \brief
 *    update QP using the quadratic model for basic unit coding
 *************************************************************************************
*/
void updateModelQPBU( rc_quadratic *prc, int topfield, int m_Qp )
{
  double dtmp, m_Qstep;
  int m_Bits;
  /*compute the total number of bits for the current basic unit*/
  m_Bits =(int)(prc->Target * prc->CurrentFrameMAD * prc->CurrentFrameMAD / prc->TotalBUMAD);
  /*compute the number of texture bits*/
  m_Bits -=prc->PAveHeaderBits2;

  m_Bits=imax(m_Bits,(int)(prc->bit_rate/(MINVALUE*prc->frame_rate*prc->TotalNumberofBasicUnit)));

  dtmp = prc->CurrentFrameMAD * prc->CurrentFrameMAD * prc->m_X1 * prc->m_X1 \
    + 4 * prc->m_X2 * prc->CurrentFrameMAD * m_Bits;
  if ((prc->m_X2 == 0.0) || (dtmp < 0) || ((sqrt (dtmp) - prc->m_X1 * prc->CurrentFrameMAD) <= 0.0))  // fall back 1st order mode
    m_Qstep = (float)(prc->m_X1 * prc->CurrentFrameMAD / (double) m_Bits);
  else // 2nd order mode
    m_Qstep = (float) ((2 * prc->m_X2 * prc->CurrentFrameMAD) / (sqrt (dtmp) - prc->m_X1 * prc->CurrentFrameMAD));

  prc->m_Qc = Qstep2QP(m_Qstep);
  prc->m_Qc = imin(m_Qp+prc->DDquant,  prc->m_Qc); // control variation

  if(params->basicunit>=prc->MBPerRow)
    prc->m_Qc = imin(prc->PAveFrameQP+6, prc->m_Qc);
  else
    prc->m_Qc = imin(prc->PAveFrameQP+3, prc->m_Qc);

  prc->m_Qc = iClip3(m_Qp-prc->DDquant, img->RCMaxQP, prc->m_Qc); // clipping
  if(params->basicunit>=prc->MBPerRow)
    prc->m_Qc = imax(prc->PAveFrameQP-6, prc->m_Qc);
  else
    prc->m_Qc = imax(prc->PAveFrameQP-3, prc->m_Qc);

  prc->m_Qc = imax(img->RCMinQP, prc->m_Qc);
}

/*!
 *************************************************************************************
 * \brief
 *    update QP variables for interlaced pictures and basic unit coding
 *************************************************************************************
*/
void updateQPInterlaceBU( rc_quadratic *prc )
{
  /*previous choice is frame coding*/
  if(generic_RC->FieldFrame==1)
  {
    prc->TotalQpforPPicture +=prc->FrameQPBuffer;
    prc->Pm_Qp=prc->FrameQPBuffer;
  }
  /*previous choice is field coding*/
  else
  {
    prc->TotalQpforPPicture +=prc->FieldQPBuffer;
    prc->Pm_Qp=prc->FieldQPBuffer;
  }
}

/*!
 *************************************************************************************
 * \brief
 *    update QP with quadratic model
 *************************************************************************************
*/
void updateModelQPFrame( rc_quadratic *prc, int m_Bits )
{
  double dtmp, m_Qstep;

  dtmp = prc->CurrentFrameMAD * prc->m_X1 * prc->CurrentFrameMAD * prc->m_X1
    + 4 * prc->m_X2 * prc->CurrentFrameMAD * m_Bits;
  if ((prc->m_X2 == 0.0) || (dtmp < 0) || ((sqrt (dtmp) - prc->m_X1 * prc->CurrentFrameMAD) <= 0.0)) // fall back 1st order mode
    m_Qstep = (float) (prc->m_X1 * prc->CurrentFrameMAD / (double) m_Bits);
  else // 2nd order mode
    m_Qstep = (float) ((2 * prc->m_X2 * prc->CurrentFrameMAD) / (sqrt (dtmp) - prc->m_X1 * prc->CurrentFrameMAD));

  prc->m_Qc = Qstep2QP(m_Qstep);
}

/*!
 *************************************************************************************
 * \brief
 *    rate control at the MB level
 *************************************************************************************
*/
int rc_handle_mb( int prev_mb, Macroblock *currMB, Slice *curr_slice )
{
  int  mb_qp = img->qp;
  Macroblock *prevMB = NULL;

  if (prev_mb > -1)
  {
    prevMB = &img->mb_data[prev_mb];

    if ( params->MbInterlace == ADAPTIVE_CODING && !img->bot_MB && currMB->mb_field )
      mb_qp = prevMB->qp;
  }

  // frame layer rate control
  if (params->basicunit != img->FrameSizeInMbs)
  {
    // each I or B frame has only one QP
    if ( ((img->type == I_SLICE || img->type == B_SLICE) && params->RCUpdateMode != RC_MODE_1 ) || !(img->number) )
    {
      return mb_qp;
    }
    else if ( img->type == P_SLICE || params->RCUpdateMode == RC_MODE_1 )
    {
      if (!img->write_macroblock) //write macroblock
      {
        if (prev_mb > -1) 
        {      
          if (!((params->MbInterlace) && img->bot_MB)) //top macroblock
          {
            if (prevMB->prev_cbp != 1)
            {
              mb_qp = prevMB->prev_qp;
            }
          }
        }
      }

      // compute the quantization parameter for each basic unit of P frame

      if (!img->write_macroblock)
      {
        if(!((params->MbInterlace) && img->bot_MB))
        {
          if(params->RCUpdateMode <= MAX_RC_MODE && (img->NumberofCodedMacroBlocks > 0) && (img->NumberofCodedMacroBlocks % img->BasicUnit == 0))
          {
            updateRCModel(quadratic_RC);
            // frame coding
            if(active_sps->frame_mbs_only_flag)
            {
              img->BasicUnitQP = updateQP(quadratic_RC, generic_RC->TopFieldFlag);

            }
            // picture adaptive field/frame coding
            else if(params->MbInterlace || ((params->PicInterlace!=FRAME_CODING) && (generic_RC->NoGranularFieldRC==0)))
            {
              img->BasicUnitQP = updateQP(quadratic_RC, generic_RC->TopFieldFlag);
            }
          }

          if(img->current_mb_nr==0)
            img->BasicUnitQP = mb_qp;


          mb_qp = img->BasicUnitQP;
          mb_qp = iClip3(-img->bitdepth_luma_qp_scale, 51, mb_qp);
        }
      }
    }
  }
  return mb_qp;
}

/*!
 *************************************************************************************
 * \brief
 *    initialize rate control model for the top field
 *************************************************************************************
*/
void rc_init_top_field ( void )
{
  img->BasicUnit = params->basicunit;
  generic_RC->TopFieldFlag = 1;
  rc_init_pict_ptr(quadratic_RC, 0, 1, (params->PicInterlace == FIELD_CODING), 1.0F); 
  img->qp = updateQP(quadratic_RC, 1);
}

/*!
 *************************************************************************************
 * \brief
 *    initialize rate control model for the bottom field
 *************************************************************************************
*/
void rc_init_bottom_field ( int TopFieldBits )
{
  quadratic_RC->bits_topfield = TopFieldBits;
  generic_RC->TopFieldFlag = 0;
  rc_init_pict_ptr(quadratic_RC, 0,0,0, 1.0F); 
  img->qp  = updateQP(quadratic_RC, 0); 
}

/*!
 *************************************************************************************
 * \brief
 *    initialize rate control for RDPictureDecision
 *************************************************************************************
*/
void rc_init_frame_rdpic( float rateRatio )
{
  switch (params->RCUpdateMode)
  {
  case RC_MODE_0:  case RC_MODE_1:  case RC_MODE_2:  case RC_MODE_3:
    // re-store the initial RC model
    rc_copy_quadratic( quadratic_RC, quadratic_RC_init );
    rc_copy_generic( generic_RC, generic_RC_init );
    rc_init_pict_ptr(quadratic_RC, 1,0,1, rateRatio );
    img->qp  = updateQP(quadratic_RC, 0);
    break;
  default:
    break;
  }
}

/*!
 *************************************************************************************
 * \brief
 *    allocate rate control memory
 *************************************************************************************
*/
void rc_allocate_memory( void )
{
  switch (params->RCUpdateMode)
  {
  case RC_MODE_0:  case RC_MODE_1:  case RC_MODE_2:  case RC_MODE_3:
    rc_alloc_generic( &generic_RC );
    rc_alloc_quadratic( &quadratic_RC );
    if ( params->RDPictureDecision || params->MbInterlace == ADAPTIVE_CODING )
    {
      rc_alloc_generic( &generic_RC_init );
      rc_alloc_quadratic( &quadratic_RC_init );
      rc_alloc_generic( &generic_RC_best );
      rc_alloc_quadratic( &quadratic_RC_best );
    }
    break;
  default:
    break;
  }
}

/*!
 *************************************************************************************
 * \brief
 *    free rate control memory
 *************************************************************************************
*/
void rc_free_memory( void )
{
  switch (params->RCUpdateMode)
  {
  case RC_MODE_0:  case RC_MODE_1:  case RC_MODE_2:  case RC_MODE_3:
    rc_free_generic( &generic_RC );
    rc_free_quadratic( &quadratic_RC );

    if ( params->RDPictureDecision || params->MbInterlace == ADAPTIVE_CODING )
    {
      rc_free_generic( &generic_RC_init );
      rc_free_quadratic( &quadratic_RC_init );
      rc_free_generic( &generic_RC_best );
      rc_free_quadratic( &quadratic_RC_best );
    }
    break;
  default:
    break;
  }
}

/*!
 *************************************************************************************
 * \brief
 *    update coding statistics after MB coding
 *************************************************************************************
*/
void rc_update_mb_stats( Macroblock *currMB, int *bitCount )
{
  // Rate control
  img->NumberofMBHeaderBits = bitCount[BITS_MB_MODE] + bitCount[BITS_INTER_MB]
                            + bitCount[BITS_CBP_MB ] + bitCount[BITS_DELTA_QUANT_MB];
  img->NumberofMBTextureBits = bitCount[BITS_COEFF_Y_MB]+ bitCount[BITS_COEFF_UV_MB];

  switch (params->RCUpdateMode)
  {
  case RC_MODE_0:  case RC_MODE_1:  case RC_MODE_2:  case RC_MODE_3:
    generic_RC->NumberofTextureBits += img->NumberofMBTextureBits;
    generic_RC->NumberofHeaderBits  += img->NumberofMBHeaderBits;
    // basic unit layer rate control
    if(img->BasicUnit < img->FrameSizeInMbs)
    {
      generic_RC->NumberofBasicUnitHeaderBits  += img->NumberofMBHeaderBits;
      generic_RC->NumberofBasicUnitTextureBits += img->NumberofMBTextureBits;
    }
    break;
  default:
    break;
  }
}

/*!
 *************************************************************************************
 * \brief
 *    save state of rate control model
 *************************************************************************************
*/
void rc_save_state( void )
{
  switch (params->RCUpdateMode)
  {
  case RC_MODE_0:  case RC_MODE_1:  case RC_MODE_2:  case RC_MODE_3:
    rc_copy_quadratic( quadratic_RC_best, quadratic_RC );
    rc_copy_generic( generic_RC_best, generic_RC );
    break;
  default:
    break;
  }
}

/*!
 *************************************************************************************
 * \brief
 *    restore state of rate control model
 *************************************************************************************
*/
void rc_restore_state( void )
{
  switch (params->RCUpdateMode)
  {
  case RC_MODE_0:  case RC_MODE_1:  case RC_MODE_2:  case RC_MODE_3:
    rc_copy_quadratic( quadratic_RC, quadratic_RC_best );
    rc_copy_generic( generic_RC, generic_RC_best );
    break;
  default:
    break;
  }
}
