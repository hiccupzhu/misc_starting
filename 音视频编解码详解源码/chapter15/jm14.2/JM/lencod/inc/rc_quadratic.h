
/*!
 ***************************************************************************
 * \file
 *    rc_quadratic.h
 *
 * \author
 *    Zhengguo LI
 *    Athanasios Leontaris
 *
 * \date
 *    14 Jan 2003
 *
 * \brief
 *    Headerfile for rate control
 **************************************************************************
 */

#ifndef _RC_QUADRATIC_H_
#define _RC_QUADRATIC_H_

#define RC_MODEL_HISTORY 21

typedef struct
{
  float  bit_rate;
  float  frame_rate;
  float  PrevBitRate;           //LIZG  25/10/2002
  double GAMMAP;                //LIZG, JVT019r1
  double BETAP;                 //LIZG, JVT019r1
  double GOPTargetBufferLevel;
  double TargetBufferLevel;     //LIZG 25/10/2002
  double AveWp;
  double AveWb;
  int    MyInitialQp;
  int    PAverageQp;
  /*LIZG JVT50V2 distortion prediction model*/
  /*coefficients of the prediction model*/
  double PreviousPictureMAD;
  double MADPictureC1;
  double MADPictureC2;
  double PMADPictureC1;
  double PMADPictureC2;
  /* LIZG JVT50V2 picture layer MAD */
  double PPictureMAD [RC_MODEL_HISTORY];
  double PictureMAD  [RC_MODEL_HISTORY];
  double ReferenceMAD[RC_MODEL_HISTORY];
  double m_rgQp      [RC_MODEL_HISTORY];
  double m_rgRp      [RC_MODEL_HISTORY];
  double Pm_rgQp     [RC_MODEL_HISTORY];
  double Pm_rgRp     [RC_MODEL_HISTORY];

  double m_X1;
  double m_X2;
  double Pm_X1;
  double Pm_X2;
  int    Pm_Qp;
  int    Pm_Hp;

  int    MADm_windowSize;
  int    m_windowSize;
  int    m_Qc;

  int    PPreHeader;
  int    PrevLastQP; // QP of the second-to-last coded frame in the primary layer
  int    CurrLastQP; // QP of the last coded frame in the primary layer
  int    NumberofBFrames;
  /*basic unit layer rate control*/
  int    TotalFrameQP;
  int    NumberofBasicUnit;
  int    PAveHeaderBits1;
  int    PAveHeaderBits2;
  int    PAveHeaderBits3;
  int    PAveFrameQP;
  int    TotalNumberofBasicUnit;
  int    CodedBasicUnit;

  int    NumberofCodedPFrame;  
  int    TotalQpforPPicture;
  int    NumberofPPicture;

  double CurrentFrameMAD;
  double CurrentBUMAD;
  double TotalBUMAD;
  double PreviousFrameMAD;
  double PreviousWholeFrameMAD;

  int    DDquant;
  unsigned int    MBPerRow;
  int    QPLastPFrame;
  int    QPLastGOP;

  /* adaptive field/frame coding*/
  int    FieldQPBuffer;
  int    FrameQPBuffer;
  int    FrameAveHeaderBits;
  int    FieldAveHeaderBits;
  double *BUPFMAD;
  double *BUCFMAD;
  double *FCBUCFMAD;
  double *FCBUPFMAD;

  Boolean GOPOverdue;
  int64   Pprev_bits;

  /* rate control variables */
  int    Xp, Xb;
  int    Target;
  int    TargetField;
  int    Np, Nb, bits_topfield;
  //HRD consideration
  int    UpperBound1, UpperBound2, LowerBound;
  double Wp, Wb; // complexity weights
  double DeltaP;
  int    TotalPFrame;
  int    PMaxQpChange;
} rc_quadratic;

// rate control functions
// init/copy
void rc_alloc_quadratic( rc_quadratic **prc );
void rc_free_quadratic ( rc_quadratic **prc );
void rc_copy_quadratic ( rc_quadratic *dst, rc_quadratic *src );

// rate control (externally visible)
void rc_init_seq          (rc_quadratic *prc);
void rc_init_GOP          (rc_quadratic *prc, int np, int nb);
void rc_update_pict_frame (rc_quadratic *prc, int nbits);
void rc_init_pict         (rc_quadratic *prc, int fieldpic, int topfield, int targetcomputation, float mult);
void rc_update_pict       (rc_quadratic *prc, int nbits);
void rc_update_picture    (int bits);

int  updateQPRC0(rc_quadratic *prc, int topfield);
int  updateQPRC1(rc_quadratic *prc, int topfield);
int  updateQPRC2(rc_quadratic *prc, int topfield);
int  updateQPRC3(rc_quadratic *prc, int topfield);

// internal functions
void updateQPInterlace   ( rc_quadratic *prc );
void updateQPNonPicAFF   ( rc_quadratic *prc );
void updateBottomField   ( rc_quadratic *prc );
int  updateFirstP        ( rc_quadratic *prc, int topfield );
int  updateNegativeTarget( rc_quadratic *prc, int topfield, int m_Qp );
int  updateFirstBU       ( rc_quadratic *prc, int topfield );
void updateLastBU        ( rc_quadratic *prc, int topfield );
void predictCurrPicMAD   ( rc_quadratic *prc );
void updateModelQPBU     ( rc_quadratic *prc, int topfield, int m_Qp );
void updateQPInterlaceBU ( rc_quadratic *prc );
void updateModelQPFrame  ( rc_quadratic *prc, int m_Bits );

void updateRCModel    (rc_quadratic *prc);
void updateMADModel   (rc_quadratic *prc);
void RCModelEstimator (rc_quadratic *prc, int n_windowSize, Boolean *m_rgRejected);
void MADModelEstimator(rc_quadratic *prc, int n_windowSize, Boolean *PictureRejected);
int  updateComplexity (rc_quadratic *prc, Boolean is_updated, int nbits );
void updatePparams    (rc_quadratic *prc, int complexity );
void updateBparams    (rc_quadratic *prc, int complexity );

// external generic functions
int  rc_handle_mb         ( int prev_mb, Macroblock *currMB, Slice *curr_slice );
void rc_init_top_field    ( void );
void rc_init_bottom_field ( int TopFieldBits );
void rc_init_frame_rdpic  ( float rateRatio );
void rc_allocate_memory   ( void );
void rc_free_memory       ( void );
void rc_update_mb_stats   ( Macroblock *currMB, int *bitCount );
void rc_save_state        ( void );
void rc_restore_state     ( void );

// rate control CURRENT pointers
rc_quadratic *quadratic_RC;
// rate control object pointers for RDPictureDecision buffering...
rc_quadratic *quadratic_RC_init, *quadratic_RC_best;

// generic function pointers
int  (*updateQP)                (rc_quadratic *prc, int topfield);
void (*rc_update_pict_frame_ptr)(rc_quadratic *prc, int nbits);
void (*rc_update_picture_ptr)   (int bits);
void (*rc_init_pict_ptr)        (rc_quadratic *prc, int fieldpic, int topfield, int targetcomputation, float mult);

#endif
