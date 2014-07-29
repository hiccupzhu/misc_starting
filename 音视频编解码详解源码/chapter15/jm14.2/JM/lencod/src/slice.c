
/*!
 **************************************************************************************
 * \file
 *    slice.c
 * \brief
 *    generate the slice header, setup the bit buffer for slices,
 *    and generates the slice NALU(s)

 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Thomas Stockhammer            <stockhammer@ei.tum.de>
 *      - Detlev Marpe                  <marpe@hhi.de>
 *      - Stephan Wenger                <stewe@cs.tu-berlin.de>
 *      - Alexis Michael Tourapis       <alexismt@ieee.org>
 ***************************************************************************************
 */

#include "contributors.h"

#include <math.h>
#include <float.h>

#include "global.h"
#include "header.h"
#include "nal.h"
#include "rtp.h"
#include "fmo.h"
#include "vlc.h"
#include "image.h"
#include "cabac.h"
#include "elements.h"
#include "macroblock.h"
#include "symbol.h"
#include "context_ini.h"
#include "enc_statistics.h"
#include "ratectl.h"
#include "me_epzs.h"
#include "wp.h"
#include "slice.h"
#include "rdoq.h"
#include "wp_mcprec.h"
#include "q_offsets.h"
#include "conformance.h"
#include "quant4x4.h"
#include "quant8x8.h"
#include "quantChroma.h"

// Local declarations
static Slice *malloc_slice();
static void  free_slice(Slice *slice);
static void  set_ref_pic_num();

extern ColocatedParams *Co_located;
extern StorablePicture **listX[6];

//! convert from H.263 QP to H.264 quant given by: quant=pow(2,QP/6)
const int QP2QUANT[40]=
{
   1, 1, 1, 1, 2, 2, 2, 2,
   3, 3, 3, 4, 4, 4, 5, 6,
   6, 7, 8, 9,10,11,13,14,
  16,18,20,23,25,29,32,36,
  40,45,51,57,64,72,81,91
};

/*!
 ************************************************************************
 * \brief
 *    init_ref_pic_list_reordering initializations should go here
 ************************************************************************
 */
void init_ref_pic_list_reordering(Slice* currSlice)
{
  currSlice->ref_pic_list_reordering_flag_l0 = 0;
  currSlice->ref_pic_list_reordering_flag_l1 = 0;
}


/*!
 ************************************************************************
 *  \brief
 *     This function generates the slice (and partition) header(s)
 *
 *  \return number of bits used for the slice (and partition) header(s)
 *
 *  \par Side effects:
 *      Adds slice/partition header symbols to the symbol buffer
 *      increments Picture->no_slices, allocates memory for the
 *      slice, sets img->currSlice
 ************************************************************************
*/
static int start_slice(Slice *currSlice, StatParameters *cur_stats)
{
  EncodingEnvironmentPtr eep;
  Bitstream *currStream;
  int header_len = 0;
  int i;
  int NumberOfPartitions = (params->partition_mode == PAR_DP_1?1:3);

  //one  partition for IDR img
  if(img->currentPicture->idr_flag)
  {
     NumberOfPartitions = 1;
  }

  RTPUpdateTimestamp (img->tr);   // this has no side effects, just leave it for all NALs

  for (i=0; i<NumberOfPartitions; i++)
  {
    currStream = (currSlice->partArr[i]).bitstream;

    currStream->write_flag = 0;
    if (i==0)     // First partition
      header_len += SliceHeader ();
    else          // Second/Third partition
      header_len += Partition_BC_Header(i);

    //! Initialize CABAC
    if (currSlice->symbol_mode == CABAC)
    {
      eep = &((currSlice->partArr[i]).ee_cabac);
      if (currStream->bits_to_go != 8)
        header_len+=currStream->bits_to_go;
      writeVlcByteAlign(currStream, cur_stats);
      arienco_start_encoding(eep, currStream->streamBuffer, &(currStream->byte_pos));
      arienco_reset_EC(eep);
    }
    else
    {
      // Initialize CA-VLC
      CAVLC_init();
    }
  }

  if(currSlice->symbol_mode == CABAC)
  {
    init_contexts();
  }
  return header_len;
}

/*!
************************************************************************
* \brief
*    This creates a NAL unit structures for all data partition of the slice
*
************************************************************************
*/
void create_slice_nalus(Slice *currSlice)
{
  // KS: this is approx. max. allowed code picture size
  const int buffer_size = 500 + img->FrameSizeInMbs * (128 + 256 * img->bitdepth_luma + 512 * img->bitdepth_chroma);
  
  int part;
  NALU_t *nalu;

  for (part=0; part< currSlice->max_part_nr; part++)
  {
    if (currSlice->partArr[part].bitstream->write_flag)
    {
      nalu = AllocNALU(buffer_size);
      currSlice->partArr[part].nal_unit = nalu;
      nalu->startcodeprefix_len = 1+ (currSlice->start_mb_nr == 0 && part == 0 ?ZEROBYTES_SHORTSTARTCODE+1:ZEROBYTES_SHORTSTARTCODE);
      nalu->forbidden_bit = 0;

      if (img->currentPicture->idr_flag)
      {
        nalu->nal_unit_type = NALU_TYPE_IDR;
        nalu->nal_reference_idc = NALU_PRIORITY_HIGHEST;
      }
      else
      {
        //different nal header for different partitions
        if(params->partition_mode == 0)
        {
          nalu->nal_unit_type = NALU_TYPE_SLICE;
        }
        else
        {
          nalu->nal_unit_type = (NaluType) (NALU_TYPE_DPA +  part);
        }

        if (img->nal_reference_idc !=0)
        {
          nalu->nal_reference_idc = NALU_PRIORITY_HIGH;
        }
        else
        {
          nalu->nal_reference_idc = NALU_PRIORITY_DISPOSABLE;
        }
      }
    }
    else
    {
      currSlice->partArr[part].nal_unit = NULL;
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    This function terminates a slice (but doesn't write it out),
 *    the old terminate_slice (0)
 * \return
 *    0 if OK,                                                         \n
 *    1 in case of error
 *
 ************************************************************************
 */
static int terminate_slice(Slice *currSlice, int lastslice, StatParameters *cur_stats )
{
  Bitstream *currStream;
  NALU_t    *currNalu;
  EncodingEnvironmentPtr eep;
  int part;
  int tmp_stuffingbits = img->mb_data[img->current_mb_nr].bitcounter[BITS_STUFFING];

  if (currSlice->symbol_mode == CABAC)
    write_terminating_bit (1);      // only once, not for all partitions

  create_slice_nalus(currSlice);

  for (part=0; part<currSlice->max_part_nr; part++)
  {
    currStream = (currSlice->partArr[part]).bitstream;
    currNalu   = (currSlice->partArr[part]).nal_unit;
    if (currStream->write_flag)
    {
      if (currSlice->symbol_mode == CAVLC)
      {
        SODBtoRBSP(currStream);
        currNalu->len = RBSPtoEBSP(currNalu->buf, currStream->streamBuffer, currStream->byte_pos);
      }
      else     // CABAC
      {
        eep = &((currSlice->partArr[part]).ee_cabac);
        // terminate the arithmetic code
        arienco_done_encoding(eep);
        set_pic_bin_count(eep);

        currStream->bits_to_go = eep->Ebits_to_go;
        currStream->byte_buf = 0;

        currNalu->len = RBSPtoEBSP(currNalu->buf, currStream->streamBuffer, currStream->byte_pos);

        // NumBytesInNALunit is: payload length + 1 byte header
        img->bytes_in_picture += currNalu->len + 1;

        if (lastslice && (part==(currSlice->max_part_nr-1)))
        {
          addCabacZeroWords(currNalu, cur_stats);
        }
      }           // CABAC
    }
  }           // partition loop
  if( currSlice->symbol_mode == CABAC )
  {
    store_contexts();
  }

  cur_stats->bit_use_stuffingBits[img->type] += img->mb_data[img->current_mb_nr].bitcounter[BITS_STUFFING] - tmp_stuffingbits;

  if (img->type != I_SLICE && img->type != SI_SLICE)
    free_ref_pic_list_reordering_buffer (currSlice);

  return 0;
}

/*!
************************************************************************
* \brief
*    Encodes one slice
* \par
*   returns the number of coded MBs in the SLice
************************************************************************
*/
int encode_one_slice (int SliceGroupId, Picture *pic, int TotalCodedMBs)
{
  Boolean end_of_slice = FALSE;
  Boolean recode_macroblock;
  Boolean prev_recode_mb = FALSE;
  int len;
  int NumberOfCodedMBs = 0;
  Macroblock* currMB      = NULL;
  int CurrentMbAddr;
  double FrameRDCost = DBL_MAX, FieldRDCost = DBL_MAX;
  StatParameters *cur_stats = &enc_picture->stats;
  Slice *currSlice = NULL;

  Motion_Selected = 0;

  if( IS_INDEPENDENT(params) )
  {
    change_plane_JV( img->colour_plane_id );
  }

  img->cod_counter = 0;

  CurrentMbAddr = FmoGetFirstMacroblockInSlice (SliceGroupId);
  // printf ("\n\nEncode_one_slice: PictureID %d SliceGroupId %d  SliceID %d  FirstMB %d \n", img->tr, SliceGroupId, img->current_slice_nr, CurrentMbInScanOrder);

  init_slice (&currSlice, CurrentMbAddr);
  // Initialize quantization functions based on rounding/quantization method
  // Done here since we may wish to disable adaptive rounding on occasional intervals (even at a frame or gop level).
  init_quant_4x4(params, img, currSlice);
  init_quant_8x8(params, img, currSlice);
  init_quant_Chroma(params, img, currSlice);


  SetLagrangianMultipliers();

  if (currSlice->symbol_mode == CABAC)
  {
    SetCtxModelNumber ();
  }

  img->checkref = (params->rdopt && params->RestrictRef && (img->type==P_SLICE || img->type==SP_SLICE));

  len = start_slice (currSlice, cur_stats);

  // Rate control
  if (params->RCEnable)
    rc_store_slice_header_bits( len );


  // Update statistics
  stats->bit_slice += len;
  cur_stats->bit_use_header[img->type] += len;

  while (end_of_slice == FALSE) // loop over macroblocks
  {
    if (img->AdaptiveRounding && params->AdaptRndPeriod && (img->current_mb_nr % params->AdaptRndPeriod == 0))
    {
      CalculateOffsetParam();

      if(params->Transform8x8Mode)
      {
        CalculateOffset8Param();
      }
    }

    //sw paff
    if (!img->MbaffFrameFlag)
    {
      recode_macroblock = FALSE;
      if(params->UseRDOQuant) // This needs revisit
        rdopt = &rddata_trellis_curr;
      else
        rdopt = &rddata_top_frame_mb;   // store data in top frame MB

      start_macroblock (currSlice,  &currMB, CurrentMbAddr, FALSE);

      if(params->UseRDOQuant)
      {
        trellis_coding(currMB, CurrentMbAddr, prev_recode_mb);   
      }
      else
      {
        img->masterQP = img->qp;
        encode_one_macroblock (currMB);
        write_one_macroblock (currMB, 1, prev_recode_mb);    
      }

      terminate_macroblock (currSlice, currMB, &end_of_slice, &recode_macroblock);
      prev_recode_mb = recode_macroblock;
      //       printf ("encode_one_slice: mb %d,  slice %d,   bitbuf bytepos %d EOS %d\n",
      //       img->current_mb_nr, img->current_slice_nr,
      //       currSlice->partArr[0].bitstream->byte_pos, end_of_slice);

      if (recode_macroblock == FALSE)       // The final processing of the macroblock has been done
      {
        img->SumFrameQP += currMB->qp;
        CurrentMbAddr = FmoGetNextMBNr (CurrentMbAddr);
        if (CurrentMbAddr == -1)   // end of slice
        {
          // printf ("FMO End of Slice Group detected, current MBs %d, force end of slice\n", NumberOfCodedMBs+1);
          end_of_slice = TRUE;
        }
        NumberOfCodedMBs++;       // only here we are sure that the coded MB is actually included in the slice
        proceed2nextMacroblock (currMB);
      }
      else
      {
        //!Go back to the previous MB to recode it
        img->current_mb_nr = FmoGetPreviousMBNr(img->current_mb_nr);
        img->NumberofCodedMacroBlocks--;
        if(img->current_mb_nr == -1 )   // The first MB of the slice group  is too big,
          // which means it's impossible to encode picture using current slice bits restriction
        {
          snprintf (errortext, ET_SIZE, "Error encoding first MB with specified parameter, bits of current MB may be too big");
          error (errortext, 300);
        }
      }
    }
    else                      // TBD -- Addition of FMO
    {

      //! This following ugly code breaks slices, at least for a slice mode that accumulates a certain
      //! number of bits into one slice.
      //! The suggested algorithm is as follows:
      //!
      //! SaveState (Bitstream, stats,  etc. etc.);
      //! BitsForThisMBPairInFrameMode = CodeMB (Upper, FRAME_MODE) + CodeMB (Lower, FRAME_MODE);
      //! DistortionForThisMBPairInFrameMode = CalculateDistortion(Upper) + CalculateDistortion (Lower);
      //! RestoreState();
      //! BitsForThisMBPairInFieldMode = CodeMB (Upper, FIELD_MODE) + CodeMB (Lower, FIELD_MODE);
      //! DistortionForThisMBPairInFrameMode = CalculateDistortion(Upper) + CalculateDistortion (Lower);
      //! FrameFieldMode = Decision (...)
      //! RestoreState()
      //! if (FrameFieldMode == FRAME) {
      //!   CodeMB (Upper, FRAME); CodeMB (Lower, FRAME);
      //! } else {
      //!   CodeMB (Upper FIELD); CodeMB (Lower, FIELD);
      //! }
      //!
      //! Open questions/issues:
      //!   1. CABAC/CA-VLC state:  It seems that the CABAC/CA_VLC states are changed during the
      //!      dummy encoding processes (for the R-D based selection), but that they are never
      //!      reset, once the selection is made.  I believe that this breaks the MB-adaptive
      //!      frame/field coding.  The necessary code for the state saves is readily available
      //!      in macroblock.c, start_macroblock() and terminate_macroblock() (this code needs
      //!      to be double checked that it works with CA-VLC as well
      //!   2. would it be an option to allocate Bitstreams with zero data in them (or copy the
      //!      already generated bitstream) for the "test coding"?

      img->write_macroblock = FALSE;
      if (params->MbInterlace == ADAPTIVE_CODING || params->MbInterlace == FRAME_MB_PAIR_CODING)
      {
        //================ code MB pair as frame MB ================
        //----------------------------------------------------------
        recode_macroblock = FALSE;
        //set mv limits to frame type
        update_mv_limits(img, FALSE);

        img->field_mode = FALSE;  // MB coded as frame
        img->top_field  = FALSE;   // Set top field to 0

        //Rate control
        img->write_macroblock = FALSE;
        img->bot_MB = FALSE;

        // save RC state only when it is going to change
        if ( params->RCEnable && params->RCUpdateMode <= MAX_RC_MODE )
        {
          if ( params->MbInterlace == ADAPTIVE_CODING
            && img->NumberofCodedMacroBlocks > 0 && (img->NumberofCodedMacroBlocks % img->BasicUnit) == 0 )
            rc_copy_quadratic( quadratic_RC_init, quadratic_RC ); // save initial RC status
          if ( params->MbInterlace == ADAPTIVE_CODING )
            rc_copy_generic( generic_RC_init, generic_RC ); // save initial RC status
        }

        start_macroblock (currSlice, &currMB, CurrentMbAddr, FALSE);

        rdopt = &rddata_top_frame_mb; // store data in top frame MB
        img->masterQP = img->qp;
        encode_one_macroblock (currMB);   // code the MB as frame

        FrameRDCost = rdopt->min_rdcost;
        //***   Top MB coded as frame MB ***//

        //Rate control
        img->bot_MB = TRUE; //for Rate control

        // go to the bottom MB in the MB pair
        img->field_mode = FALSE;  // MB coded as frame  //GB

        start_macroblock (currSlice, &currMB, CurrentMbAddr + 1, FALSE);
        rdopt = &rddata_bot_frame_mb; // store data in top frame MB
        img->masterQP = img->qp;
        encode_one_macroblock (currMB);         // code the MB as frame

        if ( params->RCEnable && params->RCUpdateMode <= MAX_RC_MODE )
        {
          if ( params->MbInterlace == ADAPTIVE_CODING
            && img->NumberofCodedMacroBlocks > 0 && (img->NumberofCodedMacroBlocks % img->BasicUnit) == 0 )
            rc_copy_quadratic( quadratic_RC_best, quadratic_RC ); // restore initial RC status

          if ( params->MbInterlace == ADAPTIVE_CODING )
            rc_copy_generic( generic_RC_best, generic_RC ); // save frame RC stats
        }

        FrameRDCost += rdopt->min_rdcost;
        //***   Bottom MB coded as frame MB ***//
      }

      if ((params->MbInterlace == ADAPTIVE_CODING) || (params->MbInterlace == FIELD_CODING))
      {
        //Rate control
        img->bot_MB = FALSE;
        //set mv limits to field type
        update_mv_limits(img, TRUE);

        //=========== start coding the MB pair as a field MB pair =============
        //---------------------------------------------------------------------
        img->field_mode = TRUE;  // MB coded as field
        img->top_field = TRUE;   // Set top field to 1
        img->buf_cycle <<= 1;
        params->num_ref_frames <<= 1;
        img->num_ref_idx_l0_active <<= 1;
        img->num_ref_idx_l0_active += 1;

        if ( params->RCEnable && params->RCUpdateMode <= MAX_RC_MODE )
        {
          if ( params->MbInterlace == ADAPTIVE_CODING
            && img->NumberofCodedMacroBlocks > 0 && (img->NumberofCodedMacroBlocks % img->BasicUnit) == 0 )
            rc_copy_quadratic( quadratic_RC, quadratic_RC_init ); // restore initial RC status

          if ( params->MbInterlace == ADAPTIVE_CODING )
            rc_copy_generic( generic_RC, generic_RC_init ); // reset RC stats
        }

        start_macroblock (currSlice, &currMB, CurrentMbAddr, TRUE);

        rdopt = &rddata_top_field_mb; // store data in top frame MB
        //        TopFieldIsSkipped = 0;        // set the top field MB skipped flag to 0
        img->masterQP = img->qp;
        encode_one_macroblock (currMB);         // code the MB as field

        FieldRDCost = rdopt->min_rdcost;
        //***   Top MB coded as field MB ***//
        //Rate control
        img->bot_MB = TRUE;//for Rate control

        img->top_field = FALSE;   // Set top field to 0
        start_macroblock (currSlice, &currMB, CurrentMbAddr+1, TRUE);
        rdopt = &rddata_bot_field_mb; // store data in top frame MB
        img->masterQP = img->qp;
        encode_one_macroblock (currMB);         // code the MB as field

        FieldRDCost += rdopt->min_rdcost;
        //***   Bottom MB coded as field MB ***//
      }

      //Rate control
      img->write_mbaff_frame = 0;  //Rate control

      //=========== decide between frame/field MB pair ============
      //-----------------------------------------------------------
      if ( ((params->MbInterlace == ADAPTIVE_CODING) && (FrameRDCost < FieldRDCost)) || params->MbInterlace == FRAME_MB_PAIR_CODING )
      {
        img->field_mode = FALSE;
        MBPairIsField = FALSE;
        if ( params->MbInterlace != FRAME_MB_PAIR_CODING )
        {
          img->buf_cycle >>= 1;
          params->num_ref_frames >>= 1;
          img->num_ref_idx_l0_active -= 1;
          img->num_ref_idx_l0_active >>= 1;
        }

        if ( params->RCEnable && params->RCUpdateMode <= MAX_RC_MODE )
        {
          if ( params->MbInterlace == ADAPTIVE_CODING
            && img->NumberofCodedMacroBlocks > 0 && (img->NumberofCodedMacroBlocks % img->BasicUnit) == 0 )
            rc_copy_quadratic( quadratic_RC, quadratic_RC_best ); // restore initial RC status

          if ( params->MbInterlace == ADAPTIVE_CODING )
            rc_copy_generic( generic_RC, generic_RC_best ); // restore frame RC stats
        }

        //Rate control
        img->write_mbaff_frame = 1;  //for Rate control
      }
      else
      {
        img->field_mode = TRUE;
        MBPairIsField = TRUE;
      }

      //Rate control
      img->write_macroblock = TRUE;//Rate control

      if (MBPairIsField)
        img->top_field = TRUE;
      else
        img->top_field = FALSE;

      //Rate control
      img->bot_MB = FALSE;// for Rate control

      // go back to the Top MB in the MB pair
      start_macroblock (currSlice, &currMB, CurrentMbAddr, img->field_mode);

      rdopt =  img->field_mode ? &rddata_top_field_mb : &rddata_top_frame_mb;
      copy_rdopt_data (currMB, 0);  // copy the MB data for Top MB from the temp buffers
      write_one_macroblock (currMB, 1, prev_recode_mb);     // write the Top MB data to the bitstream
      terminate_macroblock (currSlice, currMB, &end_of_slice, &recode_macroblock);     // done coding the Top MB
      prev_recode_mb = recode_macroblock;

      if (recode_macroblock == FALSE)       // The final processing of the macroblock has been done
      {
        img->SumFrameQP += currMB->qp;

        CurrentMbAddr = FmoGetNextMBNr (CurrentMbAddr);
        if (CurrentMbAddr == -1)   // end of slice
        {
          end_of_slice = TRUE;
        }
        NumberOfCodedMBs++;       // only here we are sure that the coded MB is actually included in the slice
        proceed2nextMacroblock (currMB);

        //Rate control
        img->bot_MB = TRUE;//for Rate control
        // go to the Bottom MB in the MB pair
        img->top_field = FALSE;
        start_macroblock (currSlice, &currMB, CurrentMbAddr, img->field_mode);

        rdopt = img->field_mode ? &rddata_bot_field_mb : &rddata_bot_frame_mb;
        copy_rdopt_data (currMB, 1);  // copy the MB data for Bottom MB from the temp buffers

        write_one_macroblock (currMB, 0, prev_recode_mb);     // write the Bottom MB data to the bitstream
        terminate_macroblock (currSlice, currMB, &end_of_slice, &recode_macroblock);     // done coding the Top MB
        prev_recode_mb = recode_macroblock;
        if (recode_macroblock == FALSE)       // The final processing of the macroblock has been done
        {
          img->SumFrameQP += currMB->qp;

          CurrentMbAddr = FmoGetNextMBNr (CurrentMbAddr);
          if (CurrentMbAddr == -1)   // end of slice
          {
            end_of_slice = TRUE;
          }
          NumberOfCodedMBs++;       // only here we are sure that the coded MB is actually included in the slice
          proceed2nextMacroblock (currMB);
        }
        else
        {
          //Go back to the beginning of the macroblock pair to recode it
          img->current_mb_nr = FmoGetPreviousMBNr(img->current_mb_nr);
          img->current_mb_nr = FmoGetPreviousMBNr(img->current_mb_nr);
          img->NumberofCodedMacroBlocks -= 2;
          if(img->current_mb_nr == -1 )   // The first MB of the slice group  is too big,
            // which means it's impossible to encode picture using current slice bits restriction
          {
            snprintf (errortext, ET_SIZE, "Error encoding first MB with specified parameter, bits of current MB may be too big");
            error (errortext, 300);
          }
        }
      }
      else
      {
        //!Go back to the previous MB to recode it
        img->current_mb_nr = FmoGetPreviousMBNr(img->current_mb_nr);
        img->NumberofCodedMacroBlocks--;
        if(img->current_mb_nr == -1 )   // The first MB of the slice group  is too big,
          // which means it's impossible to encode picture using current slice bits restriction
        {
          snprintf (errortext, ET_SIZE, "Error encoding first MB with specified parameter, bits of current MB may be too big");
          error (errortext, 300);
        }
      }

      if (MBPairIsField)    // if MB Pair was coded as field the buffer size variables back to frame mode
      {
        img->buf_cycle >>= 1;
        params->num_ref_frames >>= 1;
        img->num_ref_idx_l0_active -= 1;
        img->num_ref_idx_l0_active >>= 1;
      }

      img->field_mode = img->top_field = FALSE; // reset to frame mode

      if ( !end_of_slice )
      {
        assert( CurrentMbAddr < (int)img->PicSizeInMbs );
        assert( CurrentMbAddr >= 0 );
        if (CurrentMbAddr == FmoGetLastCodedMBOfSliceGroup (FmoMB2SliceGroup (CurrentMbAddr)))
          end_of_slice = TRUE;        // just in case it doesn't get set in terminate_macroblock
      }
    }
  }

  terminate_slice (currSlice, (NumberOfCodedMBs + TotalCodedMBs >= (int)img->PicSizeInMbs), cur_stats );
  return NumberOfCodedMBs;
}



/*!
 ************************************************************************
 * \brief
 *    Initializes the parameters for a new slice and
 *     allocates the memory for the coded slice in the Picture structure
 *  \par Side effects:
 *      Adds slice/partition header symbols to the symbol buffer
 *      increments Picture->no_slices, allocates memory for the
 *      slice, sets img->currSlice
 ************************************************************************
 */
void init_slice (Slice **currSlice, int start_mb_addr)
{
  int i,j;
  Picture *currPic = img->currentPicture;
  DataPartition *dataPart;
  Bitstream *currStream;
  int active_ref_lists = (img->MbaffFrameFlag) ? 6 : 2;

  img->current_mb_nr = start_mb_addr;

  // Allocate new Slice in the current Picture, and set img->currentSlice
  assert (currPic != NULL);
  currPic->no_slices++;

  if (currPic->no_slices >= MAXSLICEPERPICTURE)
    error ("Too many slices per picture, increase MAXSLICEPERPICTURE in global.h.", -1);

  currPic->slices[currPic->no_slices-1] = malloc_slice();
  *currSlice = currPic->slices[currPic->no_slices-1];

  img->currentSlice = *currSlice;

  (*currSlice)->picture_id    = img->tr % 256;
  (*currSlice)->qp            = img->qp;
  (*currSlice)->start_mb_nr   = start_mb_addr;
  (*currSlice)->slice_too_big = dummy_slice_too_big;

  for (i = 0; i < (*currSlice)->max_part_nr; i++)
  {
    dataPart = &(*currSlice)->partArr[i];

    currStream = dataPart->bitstream;
    currStream->bits_to_go = 8;
    currStream->byte_pos = 0;
    currStream->byte_buf = 0;
  }

  img->num_ref_idx_l0_active = active_pps->num_ref_idx_l0_active_minus1 + 1;
  img->num_ref_idx_l1_active = active_pps->num_ref_idx_l1_active_minus1 + 1;

  // primary and redundant slices: number of references overriding.
  if(params->redundant_pic_flag)
  {
    if(!redundant_coding)
    {
      img->num_ref_idx_l0_active = imin(img->number,params->NumRefPrimary);
    }
    else
    {
      // 1 reference picture for redundant slices
      img->num_ref_idx_l0_active = 1;
    }
  }

  // code now also considers fields. Issue whether we should account this within the appropriate input params directly
  if ((img->type == P_SLICE || img->type == SP_SLICE) && params->P_List0_refs)
  {
    img->num_ref_idx_l0_active = imin(img->num_ref_idx_l0_active, params->P_List0_refs * ((img->structure !=0) + 1));
  }
  if (img->type == B_SLICE )
  {
    if (params->B_List0_refs)
    {
      img->num_ref_idx_l0_active = imin(img->num_ref_idx_l0_active, params->B_List0_refs * ((img->structure !=0) + 1));
    }
    if (params->B_List1_refs)
    {
      img->num_ref_idx_l1_active = imin(img->num_ref_idx_l1_active, params->B_List1_refs * ((img->structure !=0) + 1));
    }
  }
  // generate reference picture lists
  init_lists(img->type, (PictureStructure) img->structure);

  // assign list 0 size from list size
  img->num_ref_idx_l0_active = listXsize[0];
  img->num_ref_idx_l1_active = listXsize[1];

  if ( params->WPMCPrecision && params->WPMCPrecFullRef )
  {
    wpxAdaptRefNum(img);
  }

  //Perform memory management based on poc distances  

  if (img->nal_reference_idc && params->PocMemoryManagement)
  {
    if (img->structure == FRAME && dpb.ref_frames_in_buffer==active_sps->num_ref_frames)
    {
      poc_based_ref_management_frame_pic(img->frame_num);
    }
    else if (img->structure == TOP_FIELD && dpb.ref_frames_in_buffer==active_sps->num_ref_frames)
    {
      poc_based_ref_management_field_pic((img->frame_num << 1) + 1);      
    }
    else if (img->structure == BOTTOM_FIELD)
      poc_based_ref_management_field_pic((img->frame_num << 1) + 1);
  }

  if (params->EnableOpenGOP)
  {
    for (i = 0; i<listXsize[0]; i++)
    {
      if (listX[0][i]->poc < img->last_valid_reference && img->ThisPOC > img->last_valid_reference)
      {
        listXsize[0] = img->num_ref_idx_l0_active = imax(1, i);
        break;
      }
    }

    for (i = 0; i<listXsize[1]; i++)
    {
      if (listX[1][i]->poc < img->last_valid_reference && img->ThisPOC > img->last_valid_reference)
      {
        listXsize[1] = img->num_ref_idx_l1_active = imax(1,i);
        break;
      }
    }
  }

  init_ref_pic_list_reordering(*currSlice);
 
  // reference list reordering 
  if ( (img->type == P_SLICE || img->type == B_SLICE) && 
    params->WPMCPrecision && pWPX->curr_wp_rd_pass->algorithm != WP_REGULAR )
  {
    wpxReorderLists( img, *currSlice );
  }
  else
  {
    // Perform reordering based on poc distances for HierarchicalCoding
    if ( img->type == P_SLICE && params->ReferenceReorder)
    {
      int i, num_ref;

      alloc_ref_pic_list_reordering_buffer(*currSlice);

      for (i = 0; i < img->num_ref_idx_l0_active + 1; i++)
      {
        (*currSlice)->reordering_of_pic_nums_idc_l0[i] = 3;
        (*currSlice)->abs_diff_pic_num_minus1_l0[i] = 0;
        (*currSlice)->long_term_pic_idx_l0[i] = 0;
      }

      num_ref = img->num_ref_idx_l0_active;
      if ( img->structure == FRAME )
        poc_ref_pic_reorder_frame(listX[LIST_0], num_ref,
        (*currSlice)->reordering_of_pic_nums_idc_l0,
        (*currSlice)->abs_diff_pic_num_minus1_l0,
        (*currSlice)->long_term_pic_idx_l0, LIST_0);
      else
      {
        poc_ref_pic_reorder_field(listX[LIST_0], num_ref,
          (*currSlice)->reordering_of_pic_nums_idc_l0,
          (*currSlice)->abs_diff_pic_num_minus1_l0,
          (*currSlice)->long_term_pic_idx_l0, LIST_0);
      }
      //reference picture reordering
      reorder_ref_pic_list(listX[LIST_0], &listXsize[LIST_0],
        img->num_ref_idx_l0_active - 1,
        (*currSlice)->reordering_of_pic_nums_idc_l0,
        (*currSlice)->abs_diff_pic_num_minus1_l0,
        (*currSlice)->long_term_pic_idx_l0);
    }
  }

  //if (img->MbaffFrameFlag)
  if (img->structure==FRAME)
    init_mbaff_lists();

  InitWP(params);

  if (img->type != I_SLICE && (active_pps->weighted_pred_flag == 1 || (active_pps->weighted_bipred_idc > 0 && (img->type == B_SLICE))))
  {
    if (img->type==P_SLICE || img->type==SP_SLICE)
    {
      int wp_type = (params->GenerateMultiplePPS && params->RDPictureDecision) && (enc_picture != enc_frame_picture[1]);
      EstimateWPPSlice (img, params, wp_type);
    }
    else
      EstimateWPBSlice (img, params);
  }

  set_ref_pic_num();

  if (img->type == B_SLICE)
  {
    if( IS_INDEPENDENT(params) )
    {
      compute_colocated_JV(Co_located, listX);
    }
    else
    {
      compute_colocated(Co_located, listX);
    }
  }

  if (img->type != I_SLICE && params->SearchMode == EPZS)
    EPZSSliceInit(params, img, EPZSCo_located, listX);

  if ((*currSlice)->symbol_mode == CAVLC)
  {
    writeMB_typeInfo       = writeSE_UVLC;
    writeIntraPredMode     = writeIntraPredMode_CAVLC;
    writeB8_typeInfo       = writeSE_UVLC;
    for (i=0; i<6; i++)
    {
      switch (listXsize[i])
      {
      case 0:
        writeRefFrame[i]   = NULL;
        break;
      case 1:
        writeRefFrame[i]   = writeSE_Dummy;
        break;
      case 2:
        writeRefFrame[i]   = writeSE_invFlag;
        break;
      default:
        writeRefFrame[i]   = writeSE_UVLC;
        break;
      }
    }

    writeMVD               = writeSE_SVLC;
    writeCBP               = writeCBP_VLC;
    writeDquant            = writeSE_SVLC;
    writeCIPredMode        = writeSE_UVLC;
    writeFieldModeInfo     = writeSE_Flag;
    writeMB_transform_size = writeSE_Flag;
  }
  else
  {
    writeMB_typeInfo       = writeMB_typeInfo_CABAC;
    writeIntraPredMode     = writeIntraPredMode_CABAC;
    writeB8_typeInfo       = writeB8_typeInfo_CABAC;
    for (i=0; i<6; i++)
    {
      switch (listXsize[i])
      {
      case 0:
        writeRefFrame[i]   = NULL;
      case 1:
        writeRefFrame[i]   = writeSE_Dummy;
        break;
      default:
        writeRefFrame[i]   = writeRefFrame_CABAC;
      }
    }
    writeMVD               = writeMVD_CABAC;
    writeCBP               = writeCBP_CABAC;
    writeDquant            = writeDquant_CABAC;
    writeCIPredMode        = writeCIPredMode_CABAC;
    writeFieldModeInfo     = writeFieldModeInfo_CABAC;
    writeMB_transform_size = writeMB_transform_size_CABAC;
  }

  // assign luma common reference picture pointers to be used for ME/sub-pel interpolation

  for(i = 0; i < active_ref_lists; i++)
  {
    for(j = 0; j < listXsize[i]; j++)
    {
      if( listX[i][j] )
      {
        listX[i][j]->p_curr_img     = listX[i][j]->p_img    [img->colour_plane_id];
        listX[i][j]->p_curr_img_sub = listX[i][j]->p_img_sub[img->colour_plane_id];
      }
    }
  }
  if (params->UseRDOQuant)
    init_rdoq_slice(img->type, (*currSlice)->symbol_mode);
}


/*!
 ************************************************************************
 * \brief
 *    Allocates a slice structure along with its dependent data structures
 * \return
 *    Pointer to a Slice
 ************************************************************************
 */
static Slice *malloc_slice()
{
  int i;
  DataPartition *dataPart;
  Slice *slice;
  int cr_size = IS_INDEPENDENT( params ) ? 0 : 512;

//  const int buffer_size = (img->size * 4); // AH 190202: There can be data expansion with
                                                          // low QP values. So, we make sure that buffer
                                                          // does not overflow. 4 is probably safe multiplier.
  int buffer_size;
  switch (params->slice_mode)
  {
  case 2:
    //buffer_size = imax(2 * params->slice_argument, 500 + (128 + 256 * img->bitdepth_luma + 512 * img->bitdepth_chroma));
    buffer_size = imax(2 * params->slice_argument, 764);
    break;
  case 1:
    buffer_size = 500 + params->slice_argument * (128 + 256 * img->bitdepth_luma + cr_size * img->bitdepth_chroma);
    break;
  default:
    buffer_size = 500 + img->FrameSizeInMbs * (128 + 256 * img->bitdepth_luma + cr_size * img->bitdepth_chroma);
    break;
  }

  // KS: this is approx. max. allowed code picture size
  if ((slice = (Slice *) calloc(1, sizeof(Slice))) == NULL) no_mem_exit ("malloc_slice: slice structure");

  slice->symbol_mode  = params->symbol_mode;
  cs_mb->symbol_mode  = params->symbol_mode;
  cs_b8->symbol_mode  = params->symbol_mode;
  cs_cm->symbol_mode  = params->symbol_mode;
  cs_ib8->symbol_mode = params->symbol_mode;
  cs_ib4->symbol_mode = params->symbol_mode;

  if (slice->symbol_mode == CABAC)
  {
    // create all context models
    slice->mot_ctx = create_contexts_MotionInfo();
    slice->tex_ctx = create_contexts_TextureInfo();
  }

  slice->max_part_nr = params->partition_mode==0?1:3;

  //for IDR img there should be only one partition
  if(img->currentPicture->idr_flag)
    slice->max_part_nr = 1;

  assignSE2partition[0] = assignSE2partition_NoDP;
  //ZL
  //for IDR img all the syntax element should be mapped to one partition
  if(!img->currentPicture->idr_flag && params->partition_mode == 1)
    assignSE2partition[1] =  assignSE2partition_DP;
  else
    assignSE2partition[1] =  assignSE2partition_NoDP;

  slice->num_mb = 0;          // no coded MBs so far

  if ((slice->partArr = (DataPartition *) calloc(slice->max_part_nr, sizeof(DataPartition))) == NULL) 
    no_mem_exit ("malloc_slice: partArr");
  for (i=0; i<slice->max_part_nr; i++) // loop over all data partitions
  {
    dataPart = &(slice->partArr[i]);
    if ((dataPart->bitstream = (Bitstream *) calloc(1, sizeof(Bitstream))) == NULL) 
      no_mem_exit ("malloc_slice: Bitstream");
    if ((dataPart->bitstream->streamBuffer = (byte *) calloc(buffer_size, sizeof(byte))) == NULL) 
      no_mem_exit ("malloc_slice: StreamBuffer");
    // Initialize storage of bitstream parameters
  }

  return slice;
}


/*!
 ************************************************************************
 * \brief
 *    This function frees nal units
 *
 ************************************************************************
 */
static void free_nal_unit(Picture *pic)
{
  int partition, slice;
  Slice  *currSlice;

  // loop over all slices of the picture
  for (slice=0; slice < pic->no_slices; slice++)
  {
    currSlice = pic->slices[slice];

    // loop over the partitions
    if (currSlice != NULL)
    {
      for (partition=0; partition < currSlice->max_part_nr; partition++)
      {
        // free only if the partition has content
        if (currSlice->partArr[partition].bitstream->write_flag )
        {
          if (currSlice->partArr[partition].nal_unit != NULL)
          {
            FreeNALU(currSlice->partArr[partition].nal_unit);
            currSlice->partArr[partition].nal_unit = NULL;
          }
        }
      }
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Memory frees of all Slice structures and of its dependent
 *    data structures
 * \par Input:
 *    Image Parameters struct struct img_par *img
 ************************************************************************
 */
void free_slice_list(Picture *currPic)
{
  int i;

  free_nal_unit(currPic);

  for (i = 0; i < currPic->no_slices; i++)
  {
    free_slice (currPic->slices[i]);
    currPic->slices[i]=NULL;
  }
}


/*!
 ************************************************************************
 * \brief
 *    Memory frees of the Slice structure and of its dependent
 *    data structures
 * \param slice:
 *    Slice to be freed
 ************************************************************************
 */
static void free_slice(Slice *slice)
{
  int i;
  DataPartition *dataPart;

  if (slice != NULL)
  {
    for (i=0; i<slice->max_part_nr; i++) // loop over all data partitions
    {
      dataPart = &(slice->partArr[i]);
      if (dataPart != NULL)
      {
         if (dataPart->bitstream != NULL)
         {
           if (dataPart->bitstream->streamBuffer != NULL)
             free(dataPart->bitstream->streamBuffer);       
           free(dataPart->bitstream);
         }
      }
    }
    if (slice->partArr != NULL)
      free(slice->partArr);
    
    if (slice->symbol_mode == CABAC)
    {
      delete_contexts_MotionInfo(slice->mot_ctx);
      delete_contexts_TextureInfo(slice->tex_ctx);
    }

    free(slice);
  }
}

static void set_ref_pic_num()
{
  int i,j;
  StorablePicture *this_ref;

  //! need to add field ref_pic_num that handles field pair.

  for (i=0;i<listXsize[LIST_0];i++)
  {
    this_ref = listX[LIST_0][i];
    enc_picture->ref_pic_num        [LIST_0][i] = this_ref->poc * 2 + ((this_ref->structure==BOTTOM_FIELD)?1:0) ;
    enc_picture->frm_ref_pic_num    [LIST_0][i] = this_ref->frame_poc * 2;
    enc_picture->top_ref_pic_num    [LIST_0][i] = this_ref->top_poc * 2;
    enc_picture->bottom_ref_pic_num [LIST_0][i] = this_ref->bottom_poc * 2 + 1;
  }

  for (i=0;i<listXsize[LIST_1];i++)
  {
    this_ref = listX[LIST_1][i];
    enc_picture->ref_pic_num        [LIST_1][i] = this_ref->poc * 2 + ((this_ref->structure==BOTTOM_FIELD)?1:0);
    enc_picture->frm_ref_pic_num    [LIST_1][i] = this_ref->frame_poc * 2;
    enc_picture->top_ref_pic_num    [LIST_1][i] = this_ref->top_poc * 2;
    enc_picture->bottom_ref_pic_num [LIST_1][i] = this_ref->bottom_poc * 2 + 1;
  }

  if (!active_sps->frame_mbs_only_flag && img->structure==FRAME)
  {
    for (j=2;j<6;j++)
      for (i=0;i<listXsize[j];i++)
      {
        this_ref = listX[j][i];
        enc_picture->ref_pic_num[j][i] = this_ref->poc * 2 + ((this_ref->structure==BOTTOM_FIELD)?1:0);
        enc_picture->frm_ref_pic_num[j][i] = this_ref->frame_poc * 2 ;
        enc_picture->top_ref_pic_num[j][i] = this_ref->top_poc * 2 ;
        enc_picture->bottom_ref_pic_num[j][i] = this_ref->bottom_poc * 2 + 1;
      }
  }
}

/*!
************************************************************************
* \brief
*    decide reference picture reordering, Frame only
************************************************************************
*/
void poc_ref_pic_reorder_frame(StorablePicture **list, unsigned num_ref_idx_lX_active, int *reordering_of_pic_nums_idc, int *abs_diff_pic_num_minus1, int *long_term_pic_idx, int list_no)
{
  unsigned int i,j,k;

  int currPicNum, picNumLXPred;

  int default_order[32];
  int re_order[32];
  int tmp_reorder[32];
  int list_sign[32];
  int reorder_stop, no_reorder;
  int poc_diff[32];
  int tmp_value, diff;

  int abs_poc_dist;
  int maxPicNum;
  unsigned int numRefs;

  maxPicNum  = max_frame_num;
  currPicNum = img->frame_num;

  picNumLXPred = currPicNum;

  // First assign default list order.
  for (i=0; i<num_ref_idx_lX_active; i++)
  {
    default_order[i] = list[i]->pic_num;
  }

  // Now access all references in buffer and assign them
  // to a potential reordering list. For each one of these
  // references compute the poc distance compared to current
  // frame.
  numRefs = dpb.ref_frames_in_buffer;
  for (i=0; i<dpb.ref_frames_in_buffer; i++)
  {
    poc_diff[i] = 0xFFFF;
    re_order[i] = dpb.fs_ref[i]->frame->pic_num;

    if (dpb.fs_ref[i]->is_used==3 && (dpb.fs_ref[i]->frame->used_for_reference)&&(!dpb.fs_ref[i]->frame->is_long_term))
    {
      abs_poc_dist = iabs(dpb.fs_ref[i]->frame->poc - enc_picture->poc) ;
      poc_diff[i] = abs_poc_dist;
      if (list_no == LIST_0)
      {
        list_sign[i] = (enc_picture->poc < dpb.fs_ref[i]->frame->poc) ? +1 : -1;
      }
      else
      {
        list_sign[i] = (enc_picture->poc > dpb.fs_ref[i]->frame->poc) ? +1 : -1;
      }
    }
  }

  // now sort these references based on poc (temporal) distance
  for (i = 0; i< numRefs - 1; i++)
  {
    for (j = i + 1; j < numRefs; j++)
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
  // populate list with selections from the pre-analysis stage
  if ( params->WPMCPrecision 
    && pWPX->curr_wp_rd_pass->algorithm != WP_REGULAR
    && pWPX->num_wp_ref_list[list_no] )
  {
    for (i=0; i<num_ref_idx_lX_active; i++)
    {
      re_order[i] = pWPX->wp_ref_list[list_no][i].PicNum;
    }
  }

  // Check versus default list to see if any
  // change has happened
  no_reorder = 1;
  for (i=0; i<num_ref_idx_lX_active; i++)
  {
    if (default_order[i] != re_order[i])
    {
      no_reorder = 0;
    }
  }

  // If different, then signal reordering
  if (no_reorder==0)
  {
    for (i=0; i<num_ref_idx_lX_active; i++)
    {
      diff = re_order[i]-picNumLXPred;
      if (diff <= 0)
      {
        reordering_of_pic_nums_idc[i] = 0;
        abs_diff_pic_num_minus1[i] = iabs(diff)-1;
        if (abs_diff_pic_num_minus1[i] < 0)
          abs_diff_pic_num_minus1[i] = maxPicNum -1;
      }
      else
      {
        reordering_of_pic_nums_idc[i] = 1;
        abs_diff_pic_num_minus1[i] = iabs(diff)-1;
      }
      picNumLXPred = re_order[i];

      tmp_reorder[i] = re_order[i];

      k = i;
      for (j=i; j<num_ref_idx_lX_active; j++)
      {
        if (default_order[j] != re_order[i])
        {
          ++k;
          tmp_reorder[k] = default_order[j];
        }
      }
      reorder_stop = 1;
      for(j=i+1; j<num_ref_idx_lX_active; j++)
      {
        if (tmp_reorder[j] != re_order[j])
        {
          reorder_stop = 0;
          break;
        }
      }

      if (reorder_stop==1)
      {
        ++i;
        break;
      }

      memcpy ( default_order, tmp_reorder, num_ref_idx_lX_active * sizeof(int));

    }
    reordering_of_pic_nums_idc[i] = 3;

    memcpy ( default_order, tmp_reorder, num_ref_idx_lX_active * sizeof(int));


    if (list_no==0)
    {
      img->currentSlice->ref_pic_list_reordering_flag_l0=1;
    }
    else
    {
      img->currentSlice->ref_pic_list_reordering_flag_l1=1;
    }
  }
}

/*!
************************************************************************
* \brief
*    decide reference picture reordering, Field only
************************************************************************
*/
void poc_ref_pic_reorder_field(StorablePicture **list, unsigned num_ref_idx_lX_active, int *reordering_of_pic_nums_idc, int *abs_diff_pic_num_minus1, int *long_term_pic_idx, int list_no)
{
  unsigned int i,j,k;

  int currPicNum, picNumLXPred;

  int default_order[32];
  int re_order[32];
  int tmp_reorder[32];
  int list_sign[32];  
  int poc_diff[32];
  int fld_type[32];

  int reorder_stop, no_reorder;
  int tmp_value, diff;

  int abs_poc_dist;
  int maxPicNum;
  unsigned int numRefs;

  int field_used[2] = {1, 2};
  int fld, idx, num_flds;

  unsigned int top_idx = 0;
  unsigned int bot_idx = 0;
  unsigned int list_size = 0;

  StorablePicture *pField[2]; // 0: TOP_FIELD, 1: BOTTOM_FIELD
  FrameStore      *pFrameStore; 

  maxPicNum  = 2 * max_frame_num;
  currPicNum = 2 * img->frame_num + 1;

  picNumLXPred = currPicNum;

  // First assign default list order.
  for (i=0; i<num_ref_idx_lX_active; i++)
  {
    default_order[i] = list[i]->pic_num;
  }

  // Now access all references in buffer and assign them
  // to a potential reordering list. For each one of these
  // references compute the poc distance compared to current
  // frame.  
  // look for eligible fields
  idx = 0;

  for (i=0; i<dpb.ref_frames_in_buffer; i++)
  {
    pFrameStore = dpb.fs_ref[i];
    pField[0]   = pFrameStore->top_field;
    pField[1]   = pFrameStore->bottom_field;
    num_flds    = (img->structure == BOTTOM_FIELD && (enc_picture->poc == (pField[0]->poc + 1) ) ) ? 1 : 2;

    poc_diff[2*i    ] = 0xFFFF;
    poc_diff[2*i + 1] = 0xFFFF;

    for ( fld = 0; fld < num_flds; fld++ )
    {
      if ( (pFrameStore->is_used & field_used[fld]) && pField[fld]->used_for_reference && !(pField[fld]->is_long_term) )
      {
        abs_poc_dist = iabs(pField[fld]->poc - enc_picture->poc) ;
        poc_diff[idx] = abs_poc_dist;
        re_order[idx] = pField[fld]->pic_num;
        fld_type[idx] = fld + 1;

        if (list_no == LIST_0)
        {
          list_sign[idx] = (enc_picture->poc < pField[fld]->poc) ? +1 : -1;
        }
        else
        {
          list_sign[idx] = (enc_picture->poc > pField[fld]->poc) ? +1 : -1;
        }
        idx++;
      }
    }
  }
  numRefs = idx;

  // now sort these references based on poc (temporal) distance
  for (i=0; i < numRefs-1; i++)
  {
    for (j = (i + 1); j < numRefs; j++)
    {
      if (poc_diff[i] > poc_diff[j] || (poc_diff[i] == poc_diff[j] && list_sign[j] > list_sign[i]))
      {
        // poc_diff
        tmp_value   = poc_diff[i];
        poc_diff[i] = poc_diff[j];
        poc_diff[j] = tmp_value;
        // re_order (PicNum)
        tmp_value   = re_order[i];
        re_order[i] = re_order[j];
        re_order[j] = tmp_value;
        // list_sign
        tmp_value    = list_sign[i];
        list_sign[i] = list_sign[j];
        list_sign[j] = tmp_value;
        // fld_type
        tmp_value   = fld_type[i];
        fld_type[i] = fld_type[j];
        fld_type[j] = tmp_value ;
      }
    }
  }

  if (img->structure == TOP_FIELD)
  {
    while ((top_idx < numRefs)||(bot_idx < numRefs))
    {
      for ( ; top_idx < numRefs; top_idx++)
      {
        if ( fld_type[top_idx] == TOP_FIELD )
        {
          tmp_reorder[list_size] = re_order[top_idx];
          list_size++;
          top_idx++;
          break;
        }
      }
      for ( ; bot_idx < numRefs; bot_idx++)
      {
        if ( fld_type[bot_idx] == BOTTOM_FIELD )
        {
          tmp_reorder[list_size] = re_order[bot_idx];
          list_size++;
          bot_idx++;
          break;
        }
      }
    }
  }
  if (img->structure == BOTTOM_FIELD)
  {
    while ((top_idx < numRefs)||(bot_idx < numRefs))
    {
      for ( ; bot_idx < numRefs; bot_idx++)
      {
        if ( fld_type[bot_idx] == BOTTOM_FIELD )
        {
          tmp_reorder[list_size] = re_order[bot_idx];
          list_size++;
          bot_idx++;
          break;
        }
      }
      for ( ; top_idx < numRefs; top_idx++)
      {
        if ( fld_type[top_idx] == TOP_FIELD )
        {
          tmp_reorder[list_size] = re_order[top_idx];
          list_size++;
          top_idx++;
          break;
        }
      }
    }
  }

  // copy to final matrix
  list_size = imin( list_size, 32 );
  for ( i = 0; i < list_size; i++ )
  {
    re_order[i] = tmp_reorder[i];
  }

  // Check versus default list to see if any
  // change has happened
  no_reorder = 1;
  for (i=0; i<num_ref_idx_lX_active; i++)
  {
    if (default_order[i] != re_order[i])
    {
      no_reorder = 0;
    }
  }

  // If different, then signal reordering
  if (no_reorder == 0)
  {
    for (i=0; i<num_ref_idx_lX_active; i++)
    {
      diff = re_order[i] - picNumLXPred;
      if (diff <= 0)
      {
        reordering_of_pic_nums_idc[i] = 0;
        abs_diff_pic_num_minus1[i] = iabs(diff)-1;
        if (abs_diff_pic_num_minus1[i] < 0)
          abs_diff_pic_num_minus1[i] = maxPicNum -1;
      }
      else
      {
        reordering_of_pic_nums_idc[i] = 1;
        abs_diff_pic_num_minus1[i] = iabs(diff)-1;
      }
      picNumLXPred = re_order[i];

      tmp_reorder[i] = re_order[i];

      k = i;
      for (j = i; j < num_ref_idx_lX_active; j++)
      {
        if (default_order[j] != re_order[i])
        {
          ++k;
          tmp_reorder[k] = default_order[j];
        }
      }
      reorder_stop = 1;
      for(j=i+1; j<num_ref_idx_lX_active; j++)
      {
        if (tmp_reorder[j] != re_order[j])
        {
          reorder_stop = 0;
          break;
        }
      }

      if (reorder_stop==1)
      {
        ++i;
        break;
      }

      memcpy ( default_order, tmp_reorder, num_ref_idx_lX_active * sizeof(int));
    }
    reordering_of_pic_nums_idc[i] = 3;

    memcpy ( default_order, tmp_reorder, num_ref_idx_lX_active * sizeof(int));

    if (list_no==0)
    {
      img->currentSlice->ref_pic_list_reordering_flag_l0 = 1;
    }
    else
    {
      img->currentSlice->ref_pic_list_reordering_flag_l1 = 1;
    }
  }
}

void UpdateMELambda(ImageParameters *img)
{
  int j, k, qp;
  if (params->UpdateLambdaChromaME)
  {
    for (j = 0; j < 6; j++)
    {
      for (qp = -img->bitdepth_luma_qp_scale; qp < 52; qp++)
      { 
        for (k = 0; k < 3; k++)
        {
          if ((params->MEErrorMetric[k] == ERROR_SAD) && (params->ChromaMEEnable))
          {
            switch(params->yuv_format)
            {
            case YUV420:
              img->lambda_mf[j][qp][k] = (3 * img->lambda_mf[j][qp][k] + 1) >> 1;
              img->lambda_me[j][qp][k] *= 1.5;
              break;
            case YUV422:
              img->lambda_mf[j][qp][k] *= 2;
              img->lambda_me[j][qp][k] *= 2.0;
              break;
            case YUV444:
              img->lambda_mf[j][qp][k] *= 3;
              img->lambda_me[j][qp][k] *= 3.0;
              break;
            default:
              break;
            }
          }
        }
      }
    }
  }
}

void SetLambda(int j, int qp, double lambda_scale)
{
  int k;
  img->lambda_md[j][qp] *= lambda_scale;

  for (k = F_PEL; k <= Q_PEL; k++)
  {
    img->lambda_me[j][qp][k] =  (params->MEErrorMetric[k] == ERROR_SSE) ? img->lambda_md[j][qp] : sqrt(img->lambda_md[j][qp]);
    img->lambda_mf[j][qp][k] = LAMBDA_FACTOR (img->lambda_me[j][qp][k]);
  }
}

void SetLagrangianMultipliersOn()
{
  int qp, j;
  double qp_temp;
  double lambda_scale = 1.0 - dClip3(0.0,0.5,0.05 * (double) params->jumpd);;

  if (params->UseExplicitLambdaParams == 1) // consideration of explicit lambda weights.
  {
    for (j = 0; j < 6; j++)
    {
      for (qp = -img->bitdepth_luma_qp_scale; qp < 52; qp++)
      {
        qp_temp = (double)qp + img->bitdepth_luma_qp_scale - SHIFT_QP;

        img->lambda_md[j][qp] = params->LambdaWeight[j] * pow (2, qp_temp/3.0);
        SetLambda(j, qp, ((params->MEErrorMetric[H_PEL] == ERROR_SATD && params->MEErrorMetric[Q_PEL] == ERROR_SATD) ? 1.00 : 0.95));
      }
    }
  }
  else if (params->UseExplicitLambdaParams == 2) // consideration of fixed lambda values.
  {
    for (j = 0; j < 6; j++)
    {
      for (qp = -img->bitdepth_luma_qp_scale; qp < 52; qp++)
      {
        qp_temp = (double)qp + img->bitdepth_luma_qp_scale - SHIFT_QP;

        img->lambda_md[j][qp] = params->FixedLambda[j];
        SetLambda(j, qp, ((params->MEErrorMetric[H_PEL] == ERROR_SATD && params->MEErrorMetric[Q_PEL] == ERROR_SATD) ? 1.00 : 0.95));
      }
    }
  }
  else
  {
    for (j = 0; j < 5; j++)
    {
      for (qp = -img->bitdepth_luma_qp_scale; qp < 52; qp++)
      {
        qp_temp = (double)qp + img->bitdepth_luma_qp_scale - SHIFT_QP;

        if (params->successive_Bframe > 0)
          img->lambda_md[j][qp] = 0.68 * pow (2, qp_temp/3.0)
          * (j == B_SLICE && img->b_frame_to_code != 0 ? dClip3(2.00, 4.00, (qp_temp / 6.0)) : (j == SP_SLICE) ? dClip3(1.4,3.0,(qp_temp / 12.0)) : 1.0);
        else
          img->lambda_md[j][qp] = 0.85 * pow (2, qp_temp/3.0) * ((j == SP_SLICE) ? dClip3(1.4, 3.0,(qp_temp / 12.0)) : 1.0);

        // Scale lambda due to hadamard qpel only consideration
        img->lambda_md[j][qp] = ((params->MEErrorMetric[H_PEL] == ERROR_SATD && params->MEErrorMetric[Q_PEL] == ERROR_SATD) ? 1.00 : 0.95) * img->lambda_md[j][qp];
        //img->lambda_md[j][qp] = (j == B_SLICE && params->BRefPictures == 2 && img->b_frame_to_code == 0 ? 0.50 : 1.00) * img->lambda_md[j][qp];

        if (j == B_SLICE)
        {
          img->lambda_md[5][qp] = img->lambda_md[j][qp];

          if (img->b_frame_to_code != 0)
          {
            if (params->HierarchicalCoding == 2)
              img->lambda_md[5][qp] *= (1.0 - dmin(0.4, 0.2 * (double) gop_structure[img->b_frame_to_code-1].hierarchy_layer));
            else
              img->lambda_md[5][qp] *= 0.80;

          }
          SetLambda(5, qp, lambda_scale);
        }
        else
          img->lambda_md[j][qp] *= lambda_scale;

        SetLambda(j, qp, 1.0);

        if (params->CtxAdptLagrangeMult == 1)
        {
          int lambda_qp = (qp >= 32 && !params->RCEnable) ? imax(0, qp - 4) : imax(0, qp - 6);
          img->lambda_mf_factor[j][qp] = log (img->lambda_me[j][lambda_qp][Q_PEL] + 1.0) / log (2.0);
        }
      }
    }
  }

  UpdateMELambda(img);
}


void SetLagrangianMultipliersOff()
{
  int qp, j, k;
  double qp_temp;

  for (j = 0; j < 6; j++)
  {
    for (qp = -img->bitdepth_luma_qp_scale; qp < 52; qp++)
    {
      qp_temp = (double)qp + img->bitdepth_luma_qp_scale - SHIFT_QP;

      switch (params->UseExplicitLambdaParams)
      {
      case 1:  // explicit lambda weights
        img->lambda_md[j][qp] = sqrt(params->LambdaWeight[j] * pow (2, qp_temp/3.0));
        break;
      case 2: // explicit lambda
        img->lambda_md[j][qp] = sqrt(params->FixedLambda[j]);
        break;
      default:
        img->lambda_md[j][qp] = QP2QUANT[imax(0,qp - SHIFT_QP)];
        break;
      }

      for (k = F_PEL; k <= Q_PEL; k++)
      {
        img->lambda_me[j][qp][k]  = (params->MEErrorMetric[k] == ERROR_SSE) ? (img->lambda_md[j][qp] * img->lambda_md[j][qp]) : img->lambda_md[j][qp];
        img->lambda_mf[j][qp][k]  = LAMBDA_FACTOR (img->lambda_me[j][qp][k]);
      }

      if (params->CtxAdptLagrangeMult == 1)
      {
        int lambda_qp = (qp >= 32 && !params->RCEnable) ? imax(0, qp-4) : imax(0, qp-6);
        img->lambda_mf_factor[j][qp] = log (img->lambda_me[j][lambda_qp][Q_PEL] + 1.0) / log (2.0);
      }
    }
  }
  UpdateMELambda(img);
}
