/*!
 **************************************************************************
 * \file defines.h
 *
 * \brief
 *    Header file containing some useful global definitions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Detlev Marpe
 *     - Karsten Sühring                 <suehring@hhi.de> 
 *     - Alexis Michael Tourapis         <alexismt@ieee.org> 
 *   
 *
 * \date
 *    21. March 2001
 **************************************************************************
 */


#ifndef _DEFINES_H_
#define _DEFINES_H_

#if defined _DEBUG
#define TRACE               0       //!< 0:Trace off 1:Trace on 2:detailed CABAC context information
#else
#define TRACE               0       //!< 0:Trace off 1:Trace on 2:detailed CABAC context information
#endif

#define JM                  "14 (FRExt)"
#define VERSION             "14.2"
#define EXT_VERSION         "(FRExt)"

#define GET_METIME          1       //!< Enables or disables ME computation time
#define DUMP_DPB            0       //!< Dump DPB for debug purposes
#define IMGTYPE             1       //!< Define imgpel size type. 0 implies byte (cannot handle >8 bit depths) and 1 implies unsigned short
#define ENABLE_FIELD_CTX    1       //!< Enables field context types for CABAC. If disabled, results in speedup for progressive content.
#define ENABLE_HIGH444_CTX  1       //!< Enables field context types for CABAC. If disabled, results in speedup for progressive content.
#define DEBUG_BITDEPTH      0       //!< Ensures that > 8 bit content have no values that would result in out of range results


#define MAX_RC_MODE              3
#define RC_MAX_TEMPORAL_LEVELS   5

//#define BEST_NZ_COEFF 1   // yuwen 2005.11.03 => for high complexity mode decision (CAVLC, #TotalCoeff)

//AVC Profile IDC definitions
#define BASELINE         66      //!< YUV 4:2:0/8  "Baseline"
#define MAIN             77      //!< YUV 4:2:0/8  "Main"
#define EXTENDED         88      //!< YUV 4:2:0/8  "Extended"
#define FREXT_HP        100      //!< YUV 4:2:0/8 "High"
#define FREXT_Hi10P     110      //!< YUV 4:2:0/10 "High 10"
#define FREXT_Hi422     122      //!< YUV 4:2:2/10 "High 4:2:2"
#define FREXT_Hi444     244      //!< YUV 4:4:4/14 "High 4:4:4"
#define FREXT_CAVLC444   44      //!< YUV 4:4:4/14 "CAVLC 4:4:4"

#define ZEROSNR           1

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

#define LEVEL_NUM         6
#define TOTRUN_NUM       15
#define RUNBEFORE_NUM     7
#define RUNBEFORE_NUM_M1  6

#define CAVLC_LEVEL_LIMIT 2063

//--- block types for CABAC
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

#define _FULL_SEARCH_RANGE_
#define _ADAPT_LAST_GROUP_
#define _CHANGE_QP_
#define _LEAKYBUCKET_

// ---------------------------------------------------------------------------------
// FLAGS and DEFINES for new chroma intra prediction, Dzung Hoang
// Threshold values to zero out quantized transform coefficients.
// Recommend that _CHROMA_COEFF_COST_ be low to improve chroma quality
#define _LUMA_COEFF_COST_       4 //!< threshold for luma coeffs
#define _CHROMA_COEFF_COST_     4 //!< threshold for chroma coeffs, used to be 7
#define _LUMA_MB_COEFF_COST_    5 //!< threshold for luma coeffs of inter Macroblocks
#define _LUMA_8x8_COEFF_COST_   5 //!< threshold for luma coeffs of 8x8 Inter Partition

#define IMG_PAD_SIZE           20 //!< Number of pixels padded around the reference frame (>=4)
#define IMG_PAD_SIZE_TIMES4    80 //!< Number of pixels padded around the reference frame in subpel units(>=16)


#define MAX_VALUE       999999   //!< used for start value for some variables
#define INVALIDINDEX  (-135792468)

#define P8x8    8
#define I4MB    9
#define I16MB   10
#define IBLOCK  11
#define SI4MB   12
#define I8MB    13
#define IPCM    14
#define MAXMODE 15


#define  LAMBDA_ACCURACY_BITS         16
#define  LAMBDA_FACTOR(lambda)        ((int)((double)(1 << LAMBDA_ACCURACY_BITS) * lambda + 0.5))
#define  WEIGHTED_COST(factor,bits)   (((factor) * (bits)) >> LAMBDA_ACCURACY_BITS)

#define  MV_COST(f,s,cx,cy,px,py)    (WEIGHTED_COST(f,mvbits[((cx) << (s)) - px] + mvbits[((cy) << (s)) - py]))
#define  MV_COST_SMP(f,cx,cy,px,py)  (WEIGHTED_COST(f,mvbits[cx - px] + mvbits[cy - py]))
#define  REF_COST(f,ref,list_offset) (WEIGHTED_COST(f,((listXsize[list_offset]<=1)? 0:refbits[(ref)])))

#define IS_INTRA(MB)    ((MB)->mb_type==I4MB  || (MB)->mb_type==I16MB || (MB)->mb_type==I8MB || (MB)->mb_type==IPCM)
#define IS_NEWINTRA(MB) ((MB)->mb_type==I16MB)
#define IS_OLDINTRA(MB) ((MB)->mb_type==I4MB)
#define IS_IPCM(MB)     ((MB)->mb_type==IPCM)

#define IS_INTER(MB)    ((MB)->mb_type!=I4MB  && (MB)->mb_type!=I16MB && (MB)->mb_type!=I8MB)
#define IS_INTERMV(MB)  ((MB)->mb_type!=I4MB  && (MB)->mb_type!=I16MB && (MB)->mb_type!=I8MB  && (MB)->mb_type!=0)
#define IS_DIRECT(MB)   ((MB)->mb_type==0     && (img->type==B_SLICE))
#define IS_COPY(MB)     ((MB)->mb_type==0     && (img->type==P_SLICE||img ->type==SP_SLICE))
#define IS_P8x8(MB)     ((MB)->mb_type==P8x8)

// Quantization parameter range

#define MIN_QP          0
#define MAX_QP          51
#define SHIFT_QP        12

// Direct Mode types
enum {
  DIR_TEMPORAL = 0, //!< Temporal Direct Mode
  DIR_SPATIAL  = 1 //!< Spatial Direct Mode
} DirectModes;

#define MAX_REFERENCE_PICTURES 32

#define BLOCK_SHIFT            2
#define BLOCK_SIZE             4
#define BLOCK_SIZE_8x8         8
#define MB_BLOCK_SIZE         16
#define MB_PIXELS            256 //(MB_BLOCK_SIZE * MB_BLOCK_SIZE)
#define MB_PIXELS_SHIFT        8 // log2(MB_BLOCK_SIZE * MB_BLOCK_SIZE)
#define MB_BLOCK_SHIFT         4
#define BLOCK_MULTIPLE         4  //(MB_BLOCK_SIZE/BLOCK_SIZE)
#define MB_BLOCK_PARTITIONS   16  //(BLOCK_MULTIPLE * BLOCK_MULTIPLE)
#define BLOCK_CONTEXT         64  //(4 * MB_BLOCK_PARTITIONS)

// These variables relate to the subpel accuracy supported by the software (1/4)
#define BLOCK_SIZE_SP      16  // BLOCK_SIZE << 2
#define BLOCK_SIZE_8x8_SP  32  // BLOCK_SIZE8x8 << 2

// number of intra prediction modes
#define NO_INTRA_PMODE  9

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
  VERT_PRED_16   = 0,
  HOR_PRED_16    = 1,
  DC_PRED_16     = 2,
  PLANE_16       = 3
} I16x16PredModes;

// 8x8 chroma intra prediction modes
enum {
  DC_PRED_8     =  0,
  HOR_PRED_8    =  1,
  VERT_PRED_8   =  2,
  PLANE_8       =  3
} I8x8PredModes;

#define INIT_FRAME_RATE 30
enum {
  EOS = 1,    //!< End Of Sequence
  SOP = 2,    //!< Start Of Picture
  SOS = 3     //!< Start Of Slice
};


// MV Prediction types
enum {
  MVPRED_MEDIAN   = 0,
  MVPRED_L        = 1,
  MVPRED_U        = 2,
  MVPRED_UR       = 3
} MVPredTypes;

#define MAX_SYMBOLS_PER_MB  1200  //!< Maximum number of different syntax elements for one MB
                                  // CAVLC needs more symbols per MB


#define MAX_PART_NR     3 /*!< Maximum number of different data partitions.
                               Some reasonable number which should reflect
                               what is currently defined in the SE2Partition map (elements.h) */

//Start code and Emulation Prevention need this to be defined in identical manner at encoder and decoder
#define ZEROBYTES_SHORTSTARTCODE 2 //indicates the number of zero bytes in the short start-code prefix

#define Q_BITS          15
#define DQ_BITS         6

#define Q_BITS_8        16
#define DQ_BITS_8       6

// Context Adaptive Lagrange Multiplier (CALM)
#define CALM_MF_FACTOR_THRESHOLD 512.0

#define MAX_PLANE       3
#define IS_INDEPENDENT(INP) (INP->separate_colour_plane_flag)
#define IS_FREXT_PROFILE(profile_idc) ( profile_idc>=FREXT_HP || profile_idc == FREXT_CAVLC444 )

typedef unsigned char byte;     //!< byte type definition

#endif

