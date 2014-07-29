
/*!
 **************************************************************************
 * \file defines.h
 *
 * \brief
 *    Header file containing some useful global definitions.
 *
 * \author
 *    Detlev Marpe
 *    Copyright (C) 2000 HEINRICH HERTZ INSTITUTE All Rights Reserved.
 *
 * \date
 *    21. March 2001
 **************************************************************************
 */

#ifndef _DEFINES_H_
#define _DEFINES_H_

#if defined _DEBUG
# define TRACE           1     //!< 0:Trace off 1:Trace on 2:detailed CABAC context information
#else
# define TRACE           0      //!< 0:Trace off 1:Trace on 2:detailed CABAC context information
#endif


#define DUMP_DPB                  0    //!< Dump DPB info for debug purposes
#define PAIR_FIELDS_IN_OUTPUT     0    //!< Pair field pictures for output purposes
#define IMGTYPE                   1    //!< Define imgpel size type. 0 implies byte (cannot handle >8 bit depths) and 1 implies unsigned short
#define ENABLE_HIGH444_CTX        1    //!< Enables field context types for CABAC. If disabled, results in speedup for progressive content.
#define ZEROSNR                   0    //!< PSNR computation method
#define ENABLE_OUTPUT_TONEMAPPING 1    //!< enable tone map the output if tone mapping SEI present

//#define MAX_NUM_SLICES 150
#define MAX_NUM_SLICES 50
#define MAX_REFERENCE_PICTURES 32               //!< H.264 allows 32 fields
#define MAX_CODED_FRAME_SIZE 8000000         //!< bytes for one frame

//FREXT Profile IDC definitions
#define FREXT_HP        100      //!< YUV 4:2:0/8 "High"
#define FREXT_Hi10P     110      //!< YUV 4:2:0/10 "High 10"
#define FREXT_Hi422     122      //!< YUV 4:2:2/10 "High 4:2:2"
#define FREXT_Hi444     244      //!< YUV 4:4:4/14 "High 4:4:4"
#define FREXT_CAVLC444   44      //!< YUV 4:4:4/14 "CAVLC 4:4:4"


enum {
  YUV400 = 0,
  YUV420 = 1,
  YUV422 = 2,
  YUV444 = 3
} color_formats;

// CAVLC
enum {
  LUMA              =  0,
  LUMA_INTRA16x16DC =  1,
  LUMA_INTRA16x16AC =  2,
  CB                =  3,
  CB_INTRA16x16DC   =  4,
  CB_INTRA16x16AC   =  5,
  CR                =  8,
  CR_INTRA16x16DC   =  9,
  CR_INTRA16x16AC   = 10
} CAVLCBlockTypes;


//--- block types for CABAC ----
enum {
  LUMA_16DC     =   0,
  LUMA_16AC     =   1,
  LUMA_8x8      =   2,
  LUMA_8x4      =   3,
  LUMA_4x8      =   4,
  LUMA_4x4      =   5,
  CHROMA_DC     =   6,
  CHROMA_AC     =   7,
  CHROMA_DC_2x4 =   8,
  CHROMA_DC_4x4 =   9,
  CB_16DC       =  10,
  CB_16AC       =  11,
  CB_8x8        =  12,
  CB_8x4        =  13,
  CB_4x8        =  14,
  CB_4x4        =  15,
  CR_16DC       =  16,
  CR_16AC       =  17,
  CR_8x8        =  18,
  CR_8x4        =  19,
  CR_4x8        =  20,
  CR_4x4        =  21
} CABACBlockTypes;

#if (ENABLE_HIGH444_CTX == 1)
# define NUM_BLOCK_TYPES 22  
#else
# define NUM_BLOCK_TYPES 10
#endif

//#define _LEAKYBUCKET_
enum {
  PSKIP        =  0,
  BSKIP_DIRECT =  0,
  P16x16       =  1,
  P16x8        =  2,
  P8x16        =  3,
  SMB8x8       =  4,
  SMB8x4       =  5,
  SMB4x8       =  6,
  SMB4x4       =  7,
  P8x8         =  8,
  I4MB         =  9,
  I16MB        = 10,
  IBLOCK       = 11,
  SI4MB        = 12,
  I8MB         = 13,
  IPCM         = 14,
  MAXMODE      = 15
} MBModeTypes;

#define IS_INTRA(MB)    ((MB)->mb_type==I4MB  || (MB)->mb_type==I16MB ||(MB)->mb_type==IPCM || (MB)->mb_type==I8MB || (MB)->mb_type==SI4MB)
#define IS_NEWINTRA(MB) ((MB)->mb_type==I16MB  || (MB)->mb_type==IPCM)
#define IS_OLDINTRA(MB) ((MB)->mb_type==I4MB)

#define IS_INTER(MB)    ((MB)->mb_type!=I4MB  && (MB)->mb_type!=I16MB && (MB)->mb_type!=I8MB  && (MB)->mb_type!=IPCM)
#define IS_INTERMV(MB)  ((MB)->mb_type!=I4MB  && (MB)->mb_type!=I16MB && (MB)->mb_type!=I8MB  && (MB)->mb_type!=0 && (MB)->mb_type!=IPCM)
#define IS_DIRECT(MB)   ((MB)->mb_type==0     && (img->type==B_SLICE ))
#define IS_COPY(MB)     ((MB)->mb_type==0     && (img->type==P_SLICE || img->type==SP_SLICE))
#define IS_P8x8(MB)     ((MB)->mb_type==P8x8)

#define TOTRUN_NUM       15
#define RUNBEFORE_NUM     7
#define RUNBEFORE_NUM_M1  6

// Quantization parameter range
#define MIN_QP          0
#define MAX_QP          51

#define BLOCK_SIZE      4
#define BLOCK_SIZE_8x8  8
#define SMB_BLOCK_SIZE  8
#define BLOCK_PIXELS    16
#define BLOCK_SHIFT     2
#define MB_BLOCK_SIZE   16
#define MB_PIXELS       256    // MB_BLOCK_SIZE * MB_BLOCK_SIZE
#define BLOCK_MULTIPLE  4      // (MB_BLOCK_SIZE/BLOCK_SIZE)

#define NO_INTRA_PMODE  9      //!< number of intra prediction modes
// 4x4 intra prediction modes 
enum {
  VERT_PRED            = 0,
  HOR_PRED             = 1,
  DC_PRED              = 2,
  DIAG_DOWN_LEFT_PRED  = 3,
  DIAG_DOWN_RIGHT_PRED = 4,
  VERT_RIGHT_PRED      = 5,
  HOR_DOWN_PRED        = 6,
  VERT_LEFT_PRED       = 7,
  HOR_UP_PRED          = 8
} I4x4PredModes;

// 16x16 intra prediction modes
enum {
  VERT_PRED_16 = 0,
  HOR_PRED_16  = 1,
  DC_PRED_16   = 2,
  PLANE_16     = 3
} I16x16PredModes;

// 8x8 chroma intra prediction modes
enum {
  DC_PRED_8   = 0,
  HOR_PRED_8  = 1,
  VERT_PRED_8 = 2,
  PLANE_8     = 3
} I8x8PredModes;

// MV Prediction types
enum {
  MVPRED_MEDIAN   = 0,
  MVPRED_L        = 1,
  MVPRED_U        = 2,
  MVPRED_UR       = 3
};

enum {
  EOS = 1,    //!< End Of Sequence
  SOP = 2,    //!< Start Of Picture
  SOS = 3     //!< Start Of Slice
};

enum {
  DECODING_OK     = 0,
  SEARCH_SYNC     = 1,
  PICTURE_DECODED = 2
};

#define INVALIDINDEX  (-135792468)


//Start code and Emulation Prevention need this to be defined in identical manner at encoder and decoder
#define ZEROBYTES_SHORTSTARTCODE 2 //indicates the number of zero bytes in the short start-code prefix

#define MAX_PLANE   3
#define IS_INDEPENDENT(IMG) ((IMG)->separate_colour_plane_flag)
#define IS_FREXT_PROFILE(profile_idc) ( profile_idc>=FREXT_HP || profile_idc == FREXT_CAVLC444 )
#define HI_INTRA_ONLY_PROFILE (((active_sps->profile_idc>=FREXT_Hi10P)&&(active_sps->constrained_set3_flag))||(active_sps->profile_idc==FREXT_CAVLC444)) 
#endif

