
/*!
 ***************************************************************************
 * \file
 *    ratectl.h
 *
 * \author
 *    Zhengguo LI
 *
 * \date
 *    14 Jan 2003
 *
 * \brief
 *    Headerfile for rate control
 **************************************************************************
 */

#ifndef _RATE_CTL_H_
#define _RATE_CTL_H_

#include "rc_quadratic.h"

/* generic rate control variables */
typedef struct {
  // RC flags
  int   TopFieldFlag;
  int   FieldControl;
  int   FieldFrame;
  int   NoGranularFieldRC;
  // bits stats
  int   NumberofHeaderBits;
  int   NumberofTextureBits;
  int   NumberofBasicUnitHeaderBits;
  int   NumberofBasicUnitTextureBits;
  // frame stats
  int   NumberofGOP;
  int   NumberofCodedBFrame;  
  // MAD stats
  int64 TotalMADBasicUnit;
  int   *MADofMB;
  // buffer and budget
  int64 CurrentBufferFullness; //LIZG 25/10/2002
  int64 RemainingBits;
  // bit allocations for RC_MODE_3
  int   RCPSliceBits;
  int   RCISliceBits;
  int   RCBSliceBits[RC_MAX_TEMPORAL_LEVELS];
  int   temporal_levels;
  int   hierNb[RC_MAX_TEMPORAL_LEVELS];
  int   NPSlice;
  int   NISlice;
} rc_generic;

// macroblock activity
int    diffy[16][16];

// generic functions
int    Qstep2QP          ( double Qstep );
double QP2Qstep          ( int QP );
int    ComputeMBMAD      ( void );
double ComputeFrameMAD   ( void );
void   rc_store_mad      (Macroblock *currMB);
void   update_qp_cbp     (Macroblock *currMB, short best_mode);
void   update_qp_cbp_tmp (Macroblock *currMB, int cbp, int best_mode);

// rate control functions
// init/copy
void  rc_alloc_generic           ( rc_generic **prc );
void  rc_free_generic            ( rc_generic **prc );
void  rc_copy_generic            ( rc_generic *dst, rc_generic *src );
void  rc_init_gop_params         (void);
void  rc_init_frame              (int FrameNumberInFile);
void  rc_init_sequence           (void);
void  rc_store_slice_header_bits (int len);


// rate control CURRENT pointers
rc_generic   *generic_RC;
// rate control object pointers for RDPictureDecision buffering...
rc_generic   *generic_RC_init, *generic_RC_best;


#endif

