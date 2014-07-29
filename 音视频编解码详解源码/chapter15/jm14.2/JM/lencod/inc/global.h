
/*!
 ************************************************************************
 *  \file
 *     global.h
 *
 *  \brief
 *     global definitions for H.264 encoder.
 *
 *  \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *
 ************************************************************************
 */
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <memory.h>

#include "win32.h"
#include "defines.h"
#include "parsetcommon.h"
#include "ifunctions.h"
#include "frame.h"
#include "nalucommon.h"

/***********************************************************************
 * T y p e    d e f i n i t i o n s    f o r    T M L
 ***********************************************************************
 */

#if (IMGTYPE == 0)
  typedef byte imgpel;
  typedef unsigned short distpel;
#else
  typedef unsigned short imgpel;
  typedef int distpel;
#endif

enum {
  YUV400 = 0,
  YUV420 = 1,
  YUV422 = 2,
  YUV444 = 3
} color_formats;

typedef enum
{
  // YUV
  PLANE_Y = 0,  // PLANE_Y
  PLANE_U = 1,  // PLANE_Cb
  PLANE_V = 2,  // PLANE_Cr
  // RGB
  PLANE_G = 0,
  PLANE_B = 1,
  PLANE_R = 2,
} ColorPlane;


enum {
  LIST_0 = 0,
  LIST_1 = 1,
  BI_PRED = 2,
  BI_PRED_L0 = 3,
  BI_PRED_L1 = 4
};

enum {
  ERROR_SAD = 0,
  ERROR_SSE = 1,
  ERROR_SATD = 2,
  ERROR_PSATD = 3
};

enum {
  ME_Y_ONLY = 0,
  ME_YUV_FP = 1,
  ME_YUV_FP_SP = 2
};

enum {
  DISTORTION_MSE = 0
};

//! Data Partitioning Modes
typedef enum
{
  PAR_DP_1,   //!< no data partitioning is supported
  PAR_DP_3    //!< data partitioning with 3 partitions
} PAR_DP_TYPE;


//! Output File Types
typedef enum
{
  PAR_OF_ANNEXB,    //!< Annex B byte stream format
  PAR_OF_RTP       //!< RTP packets in outfile
} PAR_OF_TYPE;

//! Field Coding Types
typedef enum
{
  FRAME_CODING,
  FIELD_CODING,
  ADAPTIVE_CODING,
  FRAME_MB_PAIR_CODING
} CodingType;

//! definition of H.264 syntax elements
typedef enum
{
  SE_HEADER,
  SE_PTYPE,
  SE_MBTYPE,
  SE_REFFRAME,
  SE_INTRAPREDMODE,
  SE_MVD,
  SE_CBP,
  SE_LUM_DC_INTRA,
  SE_CHR_DC_INTRA,
  SE_LUM_AC_INTRA,
  SE_CHR_AC_INTRA,
  SE_LUM_DC_INTER,
  SE_CHR_DC_INTER,
  SE_LUM_AC_INTER,
  SE_CHR_AC_INTER,
  SE_DELTA_QUANT,
  SE_BFRAME,
  SE_EOS,
  SE_MAX_ELEMENTS  //!< number of maximum syntax elements
} SE_type;         // substituting the definitions in elements.h


typedef enum
{
  INTER_MB,
  INTRA_MB_4x4,
  INTRA_MB_16x16
} IntraInterDecision;


typedef enum
{
  BITS_HEADER,
  BITS_TOTAL_MB,
  BITS_MB_MODE,
  BITS_INTER_MB,
  BITS_CBP_MB,
  BITS_COEFF_Y_MB,
  BITS_COEFF_UV_MB,
  BITS_COEFF_CB_MB,  
  BITS_COEFF_CR_MB,
  BITS_DELTA_QUANT_MB,
  BITS_STUFFING,
  MAX_BITCOUNTER_MB
} BitCountType;


typedef enum
{
  NO_SLICES,
  FIXED_MB,
  FIXED_RATE,
  CALL_BACK,
} SliceMode;


typedef enum
{
  CAVLC,
  CABAC
} SymbolMode;

typedef enum
{
  FULL_SEARCH      = -1,
  FAST_FULL_SEARCH =  0,
  UM_HEX           =  1,
  UM_HEX_SIMPLE    =  2,
  EPZS             =  3
} SearchType;


typedef enum
{
  FRAME,
  TOP_FIELD,
  BOTTOM_FIELD
} PictureStructure;           //!< New enum for field processing

typedef enum
{
  P_SLICE = 0,
  B_SLICE = 1,
  I_SLICE = 2,
  SP_SLICE = 3,
  SI_SLICE = 4,
  NUM_SLICE_TYPES = 5
} SliceType;

//Motion Estimation levels
typedef enum
{
  F_PEL,   //!< Full Pel refinement
  H_PEL,   //!< Half Pel refinement
  Q_PEL    //!< Quarter Pel refinement
} MELevel;

typedef enum
{
  FAST_ACCESS = 0,    //!< Fast/safe reference access
  UMV_ACCESS = 1      //!< unconstrained reference access
} REF_ACCESS_TYPE;

typedef enum
{
  IS_LUMA = 0,
  IS_CHROMA = 1
} Component_Type;

typedef enum
{
  RC_MODE_0 = 0,
  RC_MODE_1 = 1,
  RC_MODE_2 = 2,
  RC_MODE_3 = 3
} RCModeType;

/***********************************************************************
 * D a t a    t y p e s   f o r  C A B A C
 ***********************************************************************
 */

//! struct to characterize the state of the arithmetic coding engine
typedef struct
{
  unsigned int  Elow, Erange;
  unsigned int  Ebuffer;
  unsigned int  Ebits_to_go;
  unsigned int  Echunks_outstanding;
  int           Epbuf;
  byte          *Ecodestrm;
  int           *Ecodestrm_len;
  int           C;
  int           E;
} EncodingEnvironment;

typedef EncodingEnvironment *EncodingEnvironmentPtr;

//! struct for context management
typedef struct
{
  unsigned long  count;
  unsigned short state;         // index into state-table CP
  unsigned char  MPS;           // Least Probable Symbol 0/1 CP  
} BiContextType;

typedef BiContextType *BiContextTypePtr;


/**********************************************************************
 * C O N T E X T S   F O R   T M L   S Y N T A X   E L E M E N T S
 **********************************************************************
 */


#define NUM_MB_TYPE_CTX  11
#define NUM_B8_TYPE_CTX  9
#define NUM_MV_RES_CTX   10
#define NUM_REF_NO_CTX   6
#define NUM_DELTA_QP_CTX 4
#define NUM_MB_AFF_CTX 4

#define NUM_TRANSFORM_SIZE_CTX 3

typedef struct
{
  BiContextType mb_type_contexts [3][NUM_MB_TYPE_CTX];
  BiContextType b8_type_contexts [2][NUM_B8_TYPE_CTX];
  BiContextType mv_res_contexts  [2][NUM_MV_RES_CTX];
  BiContextType ref_no_contexts  [2][NUM_REF_NO_CTX];
  BiContextType delta_qp_contexts   [NUM_DELTA_QP_CTX];
  BiContextType mb_aff_contexts     [NUM_MB_AFF_CTX];
  BiContextType transform_size_contexts   [NUM_TRANSFORM_SIZE_CTX];
} MotionInfoContexts;


#define NUM_IPR_CTX    2
#define NUM_CIPR_CTX   4
#define NUM_CBP_CTX    4
#define NUM_BCBP_CTX   4
#define NUM_MAP_CTX   15
#define NUM_LAST_CTX  15
#define NUM_ONE_CTX    5
#define NUM_ABS_CTX    5


typedef struct
{
  BiContextType  ipr_contexts [NUM_IPR_CTX];
  BiContextType  cipr_contexts[NUM_CIPR_CTX];
  BiContextType  cbp_contexts [3][NUM_CBP_CTX];
  BiContextType  bcbp_contexts[NUM_BLOCK_TYPES][NUM_BCBP_CTX];
#if ENABLE_FIELD_CTX
  BiContextType  map_contexts [3][NUM_BLOCK_TYPES][NUM_MAP_CTX];
  BiContextType  last_contexts[3][NUM_BLOCK_TYPES][NUM_LAST_CTX];
#else
  BiContextType  map_contexts [1][NUM_BLOCK_TYPES][NUM_MAP_CTX];
  BiContextType  last_contexts[1][NUM_BLOCK_TYPES][NUM_LAST_CTX];
#endif
  BiContextType  one_contexts [NUM_BLOCK_TYPES][NUM_ONE_CTX];
  BiContextType  abs_contexts [NUM_BLOCK_TYPES][NUM_ABS_CTX];
} TextureInfoContexts;

//*********************** end of data type definition for CABAC *******************

//! Pixel position for checking neighbors
typedef struct pix_pos
{
  int  available;
  int  mb_addr;
  int  x;
  int  y;
  int  pos_x;
  int  pos_y;
} PixelPos;

//! Buffer structure for decoded reference picture marking commands
typedef struct DecRefPicMarking_s
{
  int memory_management_control_operation;
  int difference_of_pic_nums_minus1;
  int long_term_pic_num;
  int long_term_frame_idx;
  int max_long_term_frame_idx_plus1;
  struct DecRefPicMarking_s *Next;
} DecRefPicMarking_t;

//! Syntax Element
typedef struct syntaxelement
{
  int                 type;           //!< type of syntax element for data part.
  int                 value1;         //!< numerical value of syntax element
  int                 value2;         //!< for blocked symbols, e.g. run/level
  int                 len;            //!< length of code
  int                 inf;            //!< info part of UVLC code
  unsigned int        bitpattern;     //!< UVLC bitpattern
  int                 context;        //!< CABAC context

#if TRACE
  #define             TRACESTRING_SIZE 100            //!< size of trace string
  char                tracestring[TRACESTRING_SIZE];  //!< trace string
#endif

  //!< for mapping of syntaxElement to UVLC
  void    (*mapping)(int value1, int value2, int* len_ptr, int* info_ptr);

} SyntaxElement;

//! Macroblock
typedef struct macroblock
{
  short               slice_nr;
  short               delta_qp;
  int                 qp;                         //!< QP luma  
  int                 qpc[2];                     //!< QP chroma
  int                 qp_scaled[MAX_PLANE];       //!< QP scaled for all comps.
  int                 qpsp ;
  int                 bitcounter[MAX_BITCOUNTER_MB];

  int                 mb_type;
  short               mvd[2][BLOCK_MULTIPLE][BLOCK_MULTIPLE][2];          //!< indices correspond to [list][block_y][block_x][x,y]
  int                 cbp ;
  short               b8mode[4];
  short               b8pdir[4];
  int                 c_ipred_mode;      //!< chroma intra prediction mode
  char                IntraChromaPredModeFlag;

  byte                mb_field;
  int                 is_field_mode;
  int                 list_offset;

  int                 mbAddrA, mbAddrB, mbAddrC, mbAddrD, mbAddrX;
  int                 mbAvailA, mbAvailB, mbAvailC, mbAvailD;

  int                 all_blk_8x8;
  int                 luma_transform_size_8x8_flag;
  int                 NoMbPartLessThan8x8Flag;
  int                 DFDisableIdc;
  int                 DFAlphaC0Offset;
  int                 DFBetaOffset;

  int                 skip_flag;

  char                intra_pred_modes[MB_BLOCK_PARTITIONS];
  char                intra_pred_modes8x8[MB_BLOCK_PARTITIONS];           //!< four 8x8 blocks in a macroblock

  int64               cbp_blk ;    //!< 1 bit set for every 4x4 block with coefs (not implemented for INTRA)
  int64               cbp_bits[3];
  int64               cbp_bits_8x8[3];

  short               bipred_me[4]; //!< bi prediction ME type for a 8x8 block (0 : not used, 1: type 1, 2: type 2)

  // rate control
  double              actj;               // macroblock activity measure for macroblock j
  int                 prev_qp;
  int                 prev_dqp;
  int                 prev_cbp;


  struct macroblock   *mb_available_up;   //!< pointer to neighboring MB (CABAC)
  struct macroblock   *mb_available_left; //!< pointer to neighboring MB (CABAC)

} Macroblock;



//! Bitstream
typedef struct
{
  int             byte_pos;           //!< current position in bitstream;
  int             bits_to_go;         //!< current bitcounter
  
  int             stored_byte_pos;    //!< storage for position in bitstream;
  int             stored_bits_to_go;  //!< storage for bitcounter
  int             byte_pos_skip;      //!< storage for position in bitstream;
  int             bits_to_go_skip;    //!< storage for bitcounter
  int             write_flag;         //!< Bitstream contains data and needs to be written

  byte            byte_buf;           //!< current buffer for last written byte
  byte            stored_byte_buf;    //!< storage for buffer of last written byte
  byte            byte_buf_skip;      //!< current buffer for last written byte
  byte            *streamBuffer;      //!< actual buffer for written bytes

#if TRACE
  Boolean             trace_enabled;
#endif

} Bitstream;

//! DataPartition
typedef struct datapartition
{
  Bitstream           *bitstream;
  NALU_t              *nal_unit;
  EncodingEnvironment ee_cabac;
  EncodingEnvironment ee_recode;
} DataPartition;

//! Slice
typedef struct
{
  int                 picture_id;
  int                 qp;
  int                 qs;
  int                 picture_type; //!< picture type
  int                 start_mb_nr;
  int                 max_part_nr;  //!< number of different partitions
  int                 num_mb;       //!< number of MBs in the slice

  int                 ref_pic_list_reordering_flag_l0;
  int                 *reordering_of_pic_nums_idc_l0;
  int                 *abs_diff_pic_num_minus1_l0;
  int                 *long_term_pic_idx_l0;
  int                 ref_pic_list_reordering_flag_l1;
  int                 *reordering_of_pic_nums_idc_l1;
  int                 *abs_diff_pic_num_minus1_l1;
  int                 *long_term_pic_idx_l1;
  int                 field_ctx[3][2]; //GB

  int                 symbol_mode;

  Boolean             (*slice_too_big)(int bits_slice); //!< for use of callback functions

  DataPartition       *partArr;     //!< array of partitions
  MotionInfoContexts  *mot_ctx;     //!< pointer to struct of context models for use in CABAC
  TextureInfoContexts *tex_ctx;     //!< pointer to struct of context models for use in CABAC

} Slice;



#define MAXSLICEPERPICTURE 100

enum {
  SSE              = 0,
  SSE_RGB          = 1,  
  PSNR             = 2,
  PSNR_RGB         = 3,
  SSIM             = 4,
  SSIM_RGB         = 5,
  MS_SSIM          = 6,
  MS_SSIM_RGB      = 7,
  TOTAL_DIST_TYPES = 8
} distortion_types;

//! DistortionParams
typedef struct
{
  float value[3];                   //!< current frame distortion
  float average[3];                 //!< average frame distortion
  float avslice[NUM_SLICE_TYPES][3]; //!< average frame type distortion
} DistMetric;

typedef struct
{
  int        frame_ctr;                        //!< number of coded frames
  DistMetric metric[TOTAL_DIST_TYPES];      //!< Distortion metrics
} DistortionParams;

typedef struct
{
  int   no_slices;
  int   bits_per_picture;
  Slice *slices[MAXSLICEPERPICTURE];

  DistMetric distortion;
  byte  idr_flag;
} Picture;

Picture *p_frame_pic;
Picture **frame_pic;
Picture **field_pic;
Picture *frame_pic_si;

#ifdef _LEAKYBUCKET_
long *Bit_Buffer;
#endif

// global picture format dependend buffers, mem allocation in image.c
imgpel **pImgOrg[MAX_PLANE];
imgpel **pCurImg;            //!< Reference image. Luma for other profiles, can be any component for 4:4:4
int    **imgY_sub_tmp;       //!< Y picture temporary component (Quarter pel)

int **PicPos;
unsigned int log2_max_frame_num_minus4;
unsigned int log2_max_pic_order_cnt_lsb_minus4;
unsigned int max_frame_num;
unsigned int max_pic_order_cnt_lsb;


time_t  me_tot_time,me_time;
pic_parameter_set_rbsp_t *active_pps;
seq_parameter_set_rbsp_t *active_sps;


int dsr_new_search_range; //!<Dynamic Search Range.
//////////////////////////////////////////////////////////////////////////
// B pictures
// motion vector : forward, backward, direct
byte  MBPairIsField;     //!< For MB level field/frame coding tools


//Weighted prediction
int ***wp_weight;  // weight in [list][index][component] order
int ***wp_offset;  // offset in [list][index][component] order
int ****wbp_weight;  // weight in [list][fwd_index][bwd_idx][component] order
int luma_log_weight_denom;
int chroma_log_weight_denom;
int wp_luma_round;
int wp_chroma_round;

// global picture format dependend buffers, mem allocation in image.c (field picture)
imgpel   **img_org_top[MAX_PLANE];
imgpel   **img_org_bot[MAX_PLANE];
imgpel   **img_org_frm[MAX_PLANE];

imgpel   **imgY_com;               //!< Encoded luma images
imgpel  ***imgUV_com;              //!< Encoded croma images

char    ***direct_ref_idx;           //!< direct mode reference index buffer
char    **direct_pdir;              //!< direct mode reference index buffer

// Buffers for rd optimization with packet losses, Dim. Kontopodis
byte **pixel_map;   //!< Shows the latest reference frame that is reliable for each pixel
byte **refresh_map; //!< Stores the new values for pixel_map
int intras;         //!< Counts the intra updates in each frame.

int  frame_no, nextP_tr_fld, nextP_tr_frm;

int64 tot_time;

#define ET_SIZE 300      //!< size of error text buffer
char errortext[ET_SIZE]; //!< buffer for error message for exit with error()

// Residue Color Transform
char b8_ipredmode8x8[4][4], b8_intra_pred_modes8x8[16];

//! Info for the "decoders-in-the-encoder" used for rdoptimization with packet losses
typedef struct
{
  imgpel ***dec_mbY;        //!< Best decoded macroblock pixel values in RDO
  imgpel ***dec_mbY8x8;     //!< Best decoded 8x8 mode pixel values in RDO
  int    res_img[MAX_PLANE][16][16]; //Residual values for macroblock
} Decoders;
extern Decoders *decs;


#define FILE_NAME_SIZE 200

typedef struct
{
  double md;     //!< Mode decision Lambda
  double me[3];  //!< Motion Estimation Lambda
  int    mf[3];  //!< Integer formatted Motion Estimation Lambda
} LambdaParams;

// VUI Parameters
typedef struct
{
  int aspect_ratio_info_present_flag;
  int aspect_ratio_idc;
  int sar_width;
  int sar_height;
  int overscan_info_present_flag;
  int overscan_appropriate_flag;
  int video_signal_type_present_flag;
  int video_format;
  int video_full_range_flag;
  int colour_description_present_flag;
  int colour_primaries;
  int transfer_characteristics; 
  int matrix_coefficients;
  int chroma_location_info_present_flag;
  int chroma_sample_loc_type_top_field;
  int chroma_sample_loc_type_bottom_field;
  int timing_info_present_flag;
  int num_units_in_tick;
  int time_scale;
  int fixed_frame_rate_flag;
  int nal_hrd_parameters_present_flag;
  int nal_cpb_cnt_minus1;
  int nal_bit_rate_scale;
  int nal_cpb_size_scale;
  int nal_bit_rate_value_minus1;
  int nal_cpb_size_value_minus1;
  int nal_vbr_cbr_flag;
  int nal_initial_cpb_removal_delay_length_minus1;
  int nal_cpb_removal_delay_length_minus1;
  int nal_dpb_output_delay_length_minus1;
  int nal_time_offset_length;
  int vcl_hrd_parameters_present_flag;
  int vcl_cpb_cnt_minus1;
  int vcl_bit_rate_scale;
  int vcl_cpb_size_scale;
  int vcl_bit_rate_value_minus1;
  int vcl_cpb_size_value_minus1;
  int vcl_vbr_cbr_flag;
  int vcl_initial_cpb_removal_delay_length_minus1;
  int vcl_cpb_removal_delay_length_minus1;
  int vcl_dpb_output_delay_length_minus1;
  int vcl_time_offset_length;
  int low_delay_hrd_flag;
  int pic_struct_present_flag;
  int bitstream_restriction_flag;
  int motion_vectors_over_pic_boundaries_flag;
  int max_bytes_per_pic_denom;
  int max_bits_per_mb_denom;
  int log2_max_mv_length_vertical;
  int log2_max_mv_length_horizontal;
  int num_reorder_frames;
  int max_dec_frame_buffering;
} VUIParameters;

//! block 8x8 temporary RD info
typedef struct 
{
   short  best8x8mode            [4];
   char   best8x8pdir   [MAXMODE][4];
   char   best8x8l0ref  [MAXMODE][4];
   char   best8x8l1ref  [MAXMODE][4];
   short  bipred8x8me   [MAXMODE][4];
} Block8x8Info;

                             
//! all input parameters
typedef struct
{
  int ProfileIDC;                    //!< value of syntax element profile_idc
  int LevelIDC;                      //!< value of syntax element level_idc
  int IntraProfile;                  //!< Enable Intra profiles

  int no_frames;                     //!< number of frames to be encoded
  int qp0;                           //!< QP of first frame
  int qpN;                           //!< QP of remaining frames
  int jumpd;                         //!< number of frames to skip in input sequence (e.g 2 takes frame 0,3,6,9...)
  int DisableSubpelME;               //!< Disable sub-pixel motion estimation
  int search_range;                  /*!< search range - integer pel search and 16x16 blocks.  The search window is
                                          generally around the predicted vector. Max vector is 2xmcrange.  For 8x8
                                          and 4x4 block sizes the search range is 1/2 of that for 16x16 blocks.       */
  int num_ref_frames;                //!< number of reference frames to be used
  int P_List0_refs;                  //!< number of reference picture in list 0 in P pictures
  int B_List0_refs;                  //!< number of reference picture in list 0 in B pictures
  int B_List1_refs;                  //!< number of reference picture in list 1 in B pictures
  int Log2MaxFNumMinus4;             //!< value of syntax element log2_max_frame_num
  int Log2MaxPOCLsbMinus4;           //!< value of syntax element log2_max_pic_order_cnt_lsb_minus4

  // Input/output sequence format related variables
  FrameFormat source;                   //!< source related information
  FrameFormat output;                   //!< output related information
  int rgb_input_flag;
  int src_resize;                    //!< Control if input sequence will be resized (currently only cropping is supported)
  int src_BitDepthRescale;           //!< Control if input sequence bitdepth should be adjusted
  int yuv_format;                    //!< YUV format (0=4:0:0, 1=4:2:0, 2=4:2:2, 3=4:4:4)
  int intra_upd;                     /*!< For error robustness. 0: no special action. 1: One GOB/frame is intra coded
                                          as regular 'update'. 2: One GOB every 2 frames is intra coded etc.
                                          In connection with this intra update, restrictions is put on motion vectors
                                          to prevent errors to propagate from the past                                */
  int blc_size[8][2];                //!< array for different block sizes
  int part_size[8][2];               //!< array for different partition sizes

  int slice_mode;                    //!< Indicate what algorithm to use for setting slices
  int slice_argument;                //!< Argument to the specified slice algorithm
  int UseConstrainedIntraPred;       //!< 0: Inter MB pixels are allowed for intra prediction 1: Not allowed
  int  infile_header;                //!< If input file has a header set this to the length of the header
  char infile     [FILE_NAME_SIZE];  //!< YUV 4:2:0 input format
  char outfile    [FILE_NAME_SIZE];  //!< H.264 compressed output bitstream
  char ReconFile  [FILE_NAME_SIZE];  //!< Reconstructed Pictures
  char TraceFile  [FILE_NAME_SIZE];  //!< Trace Outputs
  char StatsFile  [FILE_NAME_SIZE];  //!< Stats File
  char QmatrixFile[FILE_NAME_SIZE];  //!< Q matrix cfg file
  int  EnableOpenGOP;                 //!< support for open gops.
  int  EnableIDRGOP;                  //!< support for open gops.

  int idr_period;                    //!< IDR picture period
  int intra_period;                  //!< intra picture period
  int intra_delay;                   //!< IDR picture delay
  int adaptive_idr_period;
  int adaptive_intra_period;         //!< reinitialize start of intra period

  int start_frame;                   //!< Encode sequence starting from Frame start_frame

  int GenerateMultiplePPS;
  int GenerateSEIMessage;
  char SEIMessageText[500];

  int ResendSPS;
  int ResendPPS;

  // B pictures
  int successive_Bframe;             //!< number of B frames that will be used
  int PReplaceBSlice;
  int qpB;                           //!< QP for non-reference B slice coded pictures
  int qpBRSOffset;                   //!< QP for reference B slice coded pictures
  int direct_spatial_mv_pred_flag;   //!< Direct Mode type to be used (0: Temporal, 1: Spatial)
  int directInferenceFlag;           //!< Direct Mode Inference Flag

  int BiPredMotionEstimation;
  int BiPredSearch[4];               //!< Use Bi prediction for modes 16x16, 16x8, 8x16, and 8x8
  
  int BiPredMERefinements;           //!< Number of bipred refinements
  int BiPredMESearchRange;
  int BiPredMESubPel;

  // SP Pictures
  int sp_periodicity;                //!< The periodicity of SP-pictures
  int qpsp;                          //!< SP Picture QP for prediction error
  int qpsp_pred;                     //!< SP Picture QP for predicted block

  int si_frame_indicator;            //!< Flag indicating whether SI frames should be encoded rather than SP frames (0: not used, 1: used)
  int sp2_frame_indicator;           //!< Flag indicating whether switching SP frames should be encoded rather than SP frames (0: not used, 1: used)
  int sp_output_indicator;           //!< Flag indicating whether coefficients are output to allow future encoding of switchin SP frames (0: not used, 1: used)
  char sp_output_filename[FILE_NAME_SIZE];    //!<Filename where SP coefficients are output
  char sp2_input_filename1[FILE_NAME_SIZE];   //!<Filename of coefficients of the first bitstream when encoding SP frames to switch bitstreams
  char sp2_input_filename2[FILE_NAME_SIZE];   //!<Filenames of coefficients of the second bitstream when encoding SP frames to switch bitstreams

  int WeightedPrediction;            //!< Weighted prediction for P frames (0: not used, 1: explicit)
  int WeightedBiprediction;          //!< Weighted prediction for B frames (0: not used, 1: explicit, 2: implicit)
  int WPMethod;                      //!< WP method (0: DC, 1: LMS)
  int WPMCPrecision;
  int WPMCPrecFullRef;
  int WPMCPrecBSlice;
  int EnhancedBWeightSupport;
  int ChromaWeightSupport;           //!< Weighted prediction support for chroma (0: disabled, 1: enabled)
  int UseWeightedReferenceME;        //!< Use Weighted Reference for ME.
  int RDPictureDecision;             //!< Perform RD optimal decision between various coded versions of same picture
  int RDPictureIntra;                //!< Enabled RD pic decision for intra as well.
  int RDPSliceWeightOnly;            //!< If enabled, does not check QP variations for P slices.
  int RDPSliceBTest;                 //!< Tests B slice replacement for P.
  int RDBSliceWeightOnly;            //!< If enabled, does not check QP variations for B slices.
  int SkipIntraInInterSlices;        //!< Skip intra type checking in inter slices if best_mode is skip/direct
  int BRefPictures;                  //!< B coded reference pictures replace P pictures (0: not used, 1: used)
  int HierarchicalCoding;
  int HierarchyLevelQPEnable;
  char ExplicitHierarchyFormat[1024];//!< Explicit GOP format (HierarchicalCoding==3).
  int ReferenceReorder;              //!< Reordering based on Poc distances
  int PocMemoryManagement;           //!< Memory management based on Poc distances for hierarchical coding

  int symbol_mode;                   //!< Specifies the mode the symbols are mapped on bits
  int of_mode;                       //!< Specifies the mode of the output file
  int partition_mode;                //!< Specifies the mode of data partitioning

  int InterSearch[2][8];

  int DisableIntra4x4;
  int DisableIntra16x16;
  int FastMDEnable; 
  int FastIntraMD; 
  int FastIntra4x4;
  int FastIntra16x16;
  int FastIntra8x8;
  int FastIntraChroma;
  int DisableIntraInInter;
  int IntraDisableInterOnly;
  int Intra4x4ParDisable;
  int Intra4x4DiagDisable;
  int Intra4x4DirDisable;
  int Intra16x16ParDisable;
  int Intra16x16PlaneDisable;
  int ChromaIntraDisable;

  int EnableIPCM;

  double FrameRate;

  int EPZSPattern;
  int EPZSDual;
  int EPZSFixed;
  int EPZSTemporal;
  int EPZSSpatialMem;
  int EPZSBlockType;
  int EPZSMinThresScale;
  int EPZSMaxThresScale;
  int EPZSMedThresScale;
  int EPZSSubPelGrid;
  int EPZSGrid;
  int EPZSSubPelME;
  int EPZSSubPelMEBiPred;
  int EPZSSubPelThresScale;

  int chroma_qp_index_offset;
#ifdef _FULL_SEARCH_RANGE_
  int full_search;
#endif
#ifdef _ADAPT_LAST_GROUP_
  int last_frame;
#endif
#ifdef _CHANGE_QP_
  int qpN2, qpB2, qp2start;
  int qp02, qpBRS2Offset;
#endif
  int rdopt;
  int Distortion[TOTAL_DIST_TYPES];
  double VisualResWavPSNR;
  int SSIMOverlapSize;
  int DistortionYUVtoRGB;
  int CtxAdptLagrangeMult;    //!< context adaptive lagrangian multiplier
  int FastCrIntraDecision;
  int disthres;
  int nobskip;

#ifdef _LEAKYBUCKET_
  int NumberLeakyBuckets;
  char LeakyBucketRateFile[FILE_NAME_SIZE];
  char LeakyBucketParamFile[FILE_NAME_SIZE];
#endif

  int PicInterlace;           //!< picture adaptive frame/field
  int MbInterlace;            //!< macroblock adaptive frame/field

  int IntraBottom;            //!< Force Intra Bottom at GOP periods.

  int LossRateA;              //!< assumed loss probablility of partition A (or full slice), in per cent, used for loss-aware R/D optimization
  int LossRateB;              //!< assumed loss probablility of partition B, in per cent, used for loss-aware R/D
  int LossRateC;              //!< assumed loss probablility of partition C, in per cent, used for loss-aware R/D
  int NoOfDecoders;
  int ErrorConcealment;       //!< Error concealment method used for loss-aware RDO (0: Copy Concealment)
  int RestrictRef;
  int NumFramesInELSubSeq;

  int RandomIntraMBRefresh;     //!< Number of pseudo-random intra-MBs per picture

  int DFSendParameters;
  int DFDisableIdc[2][NUM_SLICE_TYPES];
  int DFAlpha     [2][NUM_SLICE_TYPES];
  int DFBeta      [2][NUM_SLICE_TYPES];

  int SparePictureOption;
  int SPDetectionThreshold;
  int SPPercentageThreshold;

  // FMO
  char SliceGroupConfigFileName[FILE_NAME_SIZE];    //!< Filename for config info fot type 0, 2, 6
  int num_slice_groups_minus1;           //!< "FmoNumSliceGroups" in encoder.cfg, same as FmoNumSliceGroups, which should be erased later
  int slice_group_map_type;

  int *top_left;                         //!< top_left and bottom_right store values indicating foregrounds
  int *bottom_right;
  byte *slice_group_id;                   //!< slice_group_id is for slice group type being 6
  int *run_length_minus1;                //!< run_length_minus1 is for slice group type being 0

  int slice_group_change_direction_flag;
  int slice_group_change_rate_minus1;
  int slice_group_change_cycle;

  int redundant_pic_flag;   //! encoding of redundant pictures
  int pic_order_cnt_type;   //! POC type

  int context_init_method;
  int model_number;
  int Transform8x8Mode;
  int ReportFrameStats;
  int DisplayEncParams;
  int Verbose;

  //! Rate Control on JVT standard
  int RCEnable;
  int bit_rate;
  int SeinitialQP;
  unsigned int basicunit;
  int channel_type;
  int RCUpdateMode;
  double RCIoverPRatio;
  double RCBoverPRatio;
  double RCISliceBitRatio;
  double RCBSliceBitRatio[RC_MAX_TEMPORAL_LEVELS];
  int    RCMinQP[NUM_SLICE_TYPES];
  int    RCMaxQP[NUM_SLICE_TYPES];
  int    RCMaxQPChange;

  // Search Algorithm
  SearchType SearchMode;
  int UMHexDSR;//!< Dynamic Search Range
  int UMHexScale;

//////////////////////////////////////////////////////////////////////////
  // Fidelity Range Extensions
  int ScalingMatrixPresentFlag;
  int ScalingListPresentFlag[12];

  int cb_qp_index_offset;
  int cr_qp_index_offset;
  // Lossless Coding
  int lossless_qpprime_y_zero_flag;

  // Lambda Params
  int UseExplicitLambdaParams;
  int UpdateLambdaChromaME;
  double LambdaWeight[6];
  double FixedLambda[6];

  char QOffsetMatrixFile[FILE_NAME_SIZE];        //!< Quantization Offset matrix cfg file
  int  OffsetMatrixPresentFlag;                  //!< Enable Explicit Quantization Offset Matrices

  int AdaptiveRounding;                          //!< Adaptive Rounding parameter based on JVT-N011
  int AdaptRoundingFixed;                        //!< Global rounding for all qps
  int AdaptRndPeriod;                            //!< Set period for adaptive rounding of JVT-N011 in MBs
  int AdaptRndChroma;
  int AdaptRndWFactor  [2][NUM_SLICE_TYPES];     //!< Weighting factors for luma component based on reference indicator and slice type
  int AdaptRndCrWFactor[2][NUM_SLICE_TYPES];     //!< Weighting factors for chroma components based on reference indicator and slice type
  // Fast Mode Decision
  int EarlySkipEnable;
  int SelectiveIntraEnable;
  int DisposableP;
  int DispPQPOffset;

  //Redundant picture
  int NumRedundantHierarchy;   //!< number of entries to allocate redundant pictures in a GOP
  int PrimaryGOPLength;        //!< GOP length of primary pictures
  int NumRefPrimary;           //!< number of reference frames for primary picture

  // Chroma interpolation and buffering
  int ChromaMCBuffer;
  int ChromaMEEnable;
  int ChromaMEWeight;
  int MEErrorMetric[3];
  int ModeDecisionMetric;
  int SkipDeBlockNonRef;

  // tone mapping SEI message
  int ToneMappingSEIPresentFlag;
  char ToneMappingFile[FILE_NAME_SIZE];    //!< ToneMapping SEI message cfg file

  int separate_colour_plane_flag;
  double WeightY;
  double WeightCb;
  double WeightCr;
  

  int UseRDOQuant;
  int RDOQ_DC;
  int RDOQ_CR;
  int RDOQ_QP_Num;
  int RDOQ_CP_Mode;
  int RDOQ_CP_MV;
  int RDOQ_Fast;

  int EnableVUISupport;
  // VUI parameters
  VUIParameters VUI;
  // end of VUI parameters

} InputParameters;

//! ImageParameters
typedef struct
{
  int number;                  //!< current image number to be encoded (in first layer)  
  int frm_number;
  int cur_frm_number;          //!< current image number to be encoded (in all layers)
  int idr_gop_number;          //!< current idr image number to be encoded
  int rewind_frame;                  //!< current image number to be encoded
  int pn;                      //!< picture number
  int LevelIndex;              //!< mapped level idc
  int MaxVmvR[6];              //!< maximum vertical motion vector
  int current_mb_nr;
  int current_slice_nr;
  int type;
  int structure;               //!< picture structure
  int base_dist;
  int num_ref_frames;          //!< number of reference frames to be used
  int max_num_references;      //!< maximum number of reference pictures that may occur
  int qp;                      //!< quant for the current frame
  int qpsp;                    //!< quant for the prediction frame of SP-frame

  int RCMinQP;
  int RCMaxQP;

  float framerate;
  int width;                   //!< Number of pels
  int width_padded;            //!< Width in pels of padded picture
  int width_blk;               //!< Number of columns in blocks
  int width_cr;                //!< Number of pels chroma
  int height;                  //!< Number of lines
  int height_padded;           //!< Number in lines of padded picture
  int height_blk;              //!< Number of lines in blocks
  int height_cr;               //!< Number of lines  chroma
  int height_cr_frame;         //!< Number of lines  chroma frame
  int size;                    //!< Luma Picture size in pels
  int size_cr;                 //!< Chroma Picture size in pels
  int subblock_x;              //!< current subblock horizontal
  int subblock_y;              //!< current subblock vertical
  int is_intra_block;
  int is_v_block;
  int mb_y_upd;
  int mb_y_intra;              //!< which GOB to intra code
  int block_c_x;               //!< current block chroma vertical
  char **ipredmode;             //!< intra prediction mode
  char **ipredmode8x8;          //!< help storage for 8x8 modes, inserted by YV

  int cod_counter;             //!< Current count of number of skipped macroblocks in a row
  int ***nz_coeff;             //!< number of coefficients per block (CAVLC)

  int mb_x;                    //!< current MB horizontal
  int mb_y;                    //!< current MB vertical
  int block_x;                 //!< current block horizontal
  int block_y;                 //!< current block vertical
  int pix_x;                   //!< current pixel horizontal
  int pix_y;                   //!< current pixel vertical
  int pix_c_x;                 //!< current pixel chroma horizontal
  int pix_c_y;                 //!< current pixel chroma vertical

  int opix_x;                   //!< current original picture pixel horizontal
  int opix_y;                   //!< current original picture pixel vertical
  int opix_c_x;                 //!< current original picture pixel chroma horizontal
  int opix_c_y;                 //!< current original picture pixel chroma vertical


  // prediction/residual buffers   
  imgpel mpr_4x4  [MAX_PLANE][9][16][16]; //!< prediction samples for   4x4 intra prediction modes
  imgpel mpr_8x8  [MAX_PLANE][9][16][16]; //!< prediction samples for   8x8 intra prediction modes
  imgpel mpr_16x16[MAX_PLANE][5][16][16]; //!< prediction samples for 16x16 intra prediction modes (and chroma)
  imgpel mb_pred  [MAX_PLANE][16][16];    //!< current best prediction mode
  int    mb_rres  [MAX_PLANE][16][16];    //!< the diff pixel values between the original macroblock/block and its prediction (reconstructed)
  int    mb_ores  [MAX_PLANE][16][16];    //!< the diff pixel values between the original macroblock/block and its prediction (original)

  int    (*curr_res)[16];                 //!< pointer to current residual
  imgpel (*curr_prd)[16];                 //!< pointer to current prediction

  int ****cofAC;               //!< AC coefficients [8x8block][4x4block][level/run][scan_pos]
  int *** cofDC;               //!< DC coefficients [yuv][level/run][scan_pos]

  int ***fadjust4x4;           //!< Transform coefficients for 4x4 luma. Excludes DC for I16x16
  int ***fadjust8x8;           //!< Transform coefficients for 8x8 luma
  int ****fadjust4x4Cr;        //!< Transform coefficients for 4x4 chroma. Excludes DC chroma.
  int ****fadjust8x8Cr;        //!< Transform coefficients for 4x4 chroma within 8x8 inter blocks.

  Picture       *currentPicture; //!< The coded picture currently in the works (typically p_frame_pic, field_pic[0], or field_pic[1])
  Slice         *currentSlice;                                //!< pointer to current Slice data struct
  Macroblock    *mb_data;                                   //!< array containing all MBs of a whole frame
  Block8x8Info  *b8x8info;                                  //!< block 8x8 information for RDopt
  int *quad;               //!< Array containing square values,used for snr computation  */                                         /* Values are limited to 5000 for pixel differences over 70 (sqr(5000)).
  int *intra_block;

  int tr;
  int fld_type;                        //!< top or bottom field
  byte fld_flag;
  unsigned int rd_pass;

  // B pictures
  double b_interval;
  int p_interval;
  int b_frame_to_code;
  int fw_mb_mode;
  int bw_mb_mode;

  short****** pred_mv;                 //!< motion vector predictors for all block types and all reference frames
  short****** all_mv;                  //!< replaces local all_mv
  short******* bipred_mv;             //!<Biprediction MVs  
  
  int DFDisableIdc;
  int DFAlphaC0Offset;
  int DFBetaOffset;

  int direct_spatial_mv_pred_flag;              //!< Direct Mode type to be used (0: Temporal, 1: Spatial)

  int num_ref_idx_active_list[2];
  int num_ref_idx_l0_active;
  int num_ref_idx_l1_active;

  byte field_mode;     //!< For MB level field/frame -- field mode on flag
  byte top_field;      //!< For MB level field/frame -- top field flag
  int mvscale[6][MAX_REFERENCE_PICTURES];
  int buf_cycle;
  int i16offset;

  int layer;             //!< which layer this picture belonged to

  int NoResidueDirect;
  int AdaptiveRounding;                          //!< Adaptive Rounding parameter based on JVT-N011

  int redundant_pic_cnt; // JVT-D101

  int MbaffFrameFlag;    //!< indicates frame with mb aff coding

  //the following should probably go in sequence parameters
  unsigned int pic_order_cnt_type;

  // for poc mode 1
  Boolean      delta_pic_order_always_zero_flag;
  int          offset_for_non_ref_pic;
  int          offset_for_top_to_bottom_field;
  unsigned int num_ref_frames_in_pic_order_cnt_cycle;
  int          offset_for_ref_frame[1];

  //the following is for slice header syntax elements of poc
  // for poc mode 0.
  unsigned int pic_order_cnt_lsb;
  int          delta_pic_order_cnt_bottom;
  // for poc mode 1.
  int          delta_pic_order_cnt[2];

  int          frm_iter;   // frame variations to create (useful for multiple coding passes)

  unsigned int field_picture;
    signed int toppoc;      //!< poc for this frame or field
    signed int bottompoc;   //!< for completeness - poc of bottom field of a frame (always = poc+1)
    signed int framepoc;    //!< min (toppoc, bottompoc)
    signed int ThisPOC;     //!< current picture POC
  unsigned int frame_num;   //!< frame_num for this frame

  unsigned int PicWidthInMbs;
  unsigned int PicHeightInMapUnits;
  unsigned int FrameHeightInMbs;
  unsigned int PicSizeInMbs;
  unsigned int FrameSizeInMbs;

  //the following should probably go in picture parameters
  Boolean pic_order_present_flag; // ????????

  //the following are sent in the slice header
//  int delta_pic_order_cnt[2];
  int nal_reference_idc;

  int adaptive_ref_pic_buffering_flag;
  int no_output_of_prior_pics_flag;
  int long_term_reference_flag;

  DecRefPicMarking_t *dec_ref_pic_marking_buffer;

  int model_number;

  // rate control variables
  int NumberofCodedMacroBlocks;
  int BasicUnitQP;
  int NumberofMBTextureBits;
  int NumberofMBHeaderBits;
  unsigned int BasicUnit;
  byte write_macroblock;
  byte bot_MB;
  int write_mbaff_frame;

  int DeblockCall;

  int last_pic_bottom_field;
  int last_has_mmco_5;
  int pre_frame_num;

  int slice_group_change_cycle;

  int pic_unit_size_on_disk;
  int out_unit_size_on_disk;
  int bitdepth_luma;
  int bitdepth_chroma;
  int bitdepth_scale[2];
  int bitdepth_luma_qp_scale;
  int bitdepth_chroma_qp_scale;
  int bitdepth_lambda_scale;
  int max_bitCount;
  int max_qp_delta;
  int min_qp_delta;
  // Lagrangian Parameters
  LambdaParams **lambda;
  double  **lambda_md;     //!< Mode decision Lambda
  double ***lambda_me;     //!< Motion Estimation Lambda
  int    ***lambda_mf;     //!< Integer formatted Motion Estimation Lambda

  double **lambda_mf_factor; //!< Motion Estimation Lamda Scale Factor

  unsigned int dc_pred_value_comp[MAX_PLANE]; //!< component value for DC prediction (depends on component pel bit depth)
  unsigned int dc_pred_value;                 //!< DC prediction value for current component
  int max_imgpel_value_comp      [MAX_PLANE];       //!< max value that one picture element (pixel) can take (depends on pic_unit_bitdepth)
  int max_imgpel_value_comp_sq   [MAX_PLANE];       //!< max value that one picture element (pixel) can take (depends on pic_unit_bitdepth)
  int max_imgpel_value;              //!< max value that one picture element (pixel) can take (depends on pic_unit_bitdepth)

  int num_blk8x8_uv;
  int num_cdc_coeff;
  int yuv_format;
  int P444_joined;
  int lossless_qpprime_flag;
  int mb_cr_size_x;
  int mb_cr_size_y;
  int mb_size[MAX_PLANE][2];

  int chroma_qp_offset[2];      //!< offset for qp for chroma [0-Cb, 1-Cr]

  int auto_crop_right;
  int auto_crop_bottom;

  short checkref;
  int last_valid_reference;
  int bytes_in_picture;

  int AverageFrameQP;
  int SumFrameQP;
  int GopLevels;

  int ChromaArrayType;
  Macroblock    *mb_data_JV[MAX_PLANE];  //!< mb_data to be used during 4:4:4 independent mode encoding
  int colour_plane_id;    //!< colour_plane_id of the current coded slice (valid only when separate_colour_plane_flag is 1)

  int lastIDRnumber;
  int lastIntraNumber;
  int lastINTRA;
  int last_ref_idc;
  int idr_refresh;

  int masterQP;
} ImageParameters;


//! definition of pic motion parameters
typedef struct pic_motion_params2
{
  int64    ref_pic_id;    //!< reference picture identifier [list][subblock_y][subblock_x]
  int64    ref_id;        //!< reference picture identifier [list][subblock_y][subblock_x]
  short    mv[2];            //!< motion vector       [list][subblock_x][subblock_y][component]
  char     ref_idx;       //!< reference picture   [list][subblock_y][subblock_x]
  byte     mb_field;      //!< field macroblock indicator
  byte     field_frame;   //!< indicates if co_located is field or frame.
} PicMotionParams2;

// Motion Vector structure
typedef struct
{
  short mv_x;
  short mv_y;
} MotionVector;

//! For MB level field/frame coding tools
//! temporary structure to store MB data for field/frame coding
typedef struct
{
  double min_rdcost;
  double min_dcost;

  imgpel rec_mbY[16][16];       // hold the Y component of reconstructed MB
  imgpel rec_mb_cr[2][16][16];
  int    ****cofAC;
  int    ***cofDC;
  int    mb_type;
  short  b8mode[4], b8pdir[4];
  int    cbp;
  int    mode;
  int    i16offset;
  int    c_ipred_mode;
  int    luma_transform_size_8x8_flag;
  int    NoMbPartLessThan8x8Flag;

  int    qp;
  int    prev_qp;
  int    prev_dqp;
  short  delta_qp;
  int    prev_cbp;
  
  int64  cbp_blk;
  short  ******pred_mv;        //!< predicted motion vectors
  short  ******all_mv;         //!< all modes motion vectors

  char   intra_pred_modes[16];
  char   intra_pred_modes8x8[16];
  char   **ipredmode;
  char   refar[2][4][4];       //!< reference frame array [list][y][x]
} RD_DATA;


//! Set Explicit GOP Parameters.
//! Currently only supports Enhancement GOP but could be easily extended
typedef struct
{
  int slice_type;       //! Slice type
  int display_no;       //! GOP Display order
  int reference_idc;    //! Is reference?
  int slice_qp;         //! Assigned QP
  int hierarchy_layer;    //! Hierarchy layer (used with GOP Hierarchy option 2)
  int hierarchyPocDelta;  //! Currently unused
} GOP_DATA;


typedef struct
{
  int mb_p8x8_cost;
  int smb_p8x8_cost[4];
  double smb_p8x8_rdcost[4]; 
  int lrec[16][16]; // transform and quantized coefficients will be stored here for SP frames
  int cbp8x8;
  int cbp_blk8x8;
  int cnt_nonz_8x8;
  short part8x8mode[4];
  short part8x8bipred[4];
  char  part8x8pdir[4];
  char  part8x8l0ref[4];
  char  part8x8l1ref[4];
  imgpel rec_mbY8x8[16][16];
  imgpel mpr8x8[16][16];
  imgpel mpr8x8CbCr[2][16][16];
  imgpel rec_mb8x8_cr[2][16][16];
} RD_8x8DATA;


typedef struct
{
  double lambda_md;        //!< Mode decision Lambda
  double lambda_me[3];     //!< Motion Estimation Lambda
  int    lambda_mf[3];     //!< Integer formatted Motion Estimation Lambda
  int    best_mcost[2];

  short  valid[MAXMODE];
  short  list_offset[2];
  short  curr_mb_field;
} RD_PARAMS;

GOP_DATA *gop_structure;
RD_DATA *rdopt;
RD_DATA rddata_top_frame_mb, rddata_bot_frame_mb; //!< For MB level field/frame coding tools
RD_DATA rddata_top_field_mb, rddata_bot_field_mb; //!< For MB level field/frame coding tools

RD_DATA rddata_trellis_best, rddata_trellis_curr;
short *****tmp_mv8, *****tmp_pmv8;
int   ***motion_cost8;

extern InputParameters *params;
extern ImageParameters *img;

extern DistortionParams *dist;

// files
FILE *p_log;                     //!< SNR file
FILE *p_trace;                   //!< Trace file
int  p_in;                       //!< original YUV file handle
int  p_dec;                      //!< decoded image file handle

int coeff_cost_cr[MAX_PLANE];
int cmp_cbp[3], i16x16mode, curr_cbp[2];
int64 cur_cbp_blk[MAX_PLANE];
int CbCr_predmode_8x8[4]; 

/***********************************************************************
 * P r o t o t y p e s   f o r    T M L
 ***********************************************************************
 */

void intrapred_4x4   (Macroblock *currMB, ColorPlane pl, int CurrPixX,int CurrPixY, int *left_available, int *up_available, int *all_available);
void intrapred_16x16 (Macroblock *currMB, ColorPlane pl);
// Transform function pointers
int (*pDCT_4x4)      (Macroblock *currMB, ColorPlane pl, int block_x, int block_y, int *coeff_cost, int intra, int is_cavlc);
int (*pDCT_16x16)    (Macroblock *currMB, ColorPlane pl, int, int is_cavlc);
int (*pDCT_8x8)      (Macroblock *currMB, ColorPlane pl, int b8, int *coeff_cost, int intra);
int  (*dct_cr_4x4[2])(Macroblock *currMB, int uv,int i11, int is_cavlc);

int  dct_8x8         (Macroblock *currMB, ColorPlane pl, int b8, int *coeff_cost, int intra);
int  dct_8x8_cavlc   (Macroblock *currMB, ColorPlane pl, int b8, int *coeff_cost, int intra);
int  dct_8x8_ls      (Macroblock *currMB, ColorPlane pl, int b8, int *coeff_cost, int intra);

void copyblock_sp    (Macroblock *currMB, ColorPlane pl, int pos_mb1,int pos_mb2);
int  dct_chroma_sp   (Macroblock *currMB, int uv,int i11, int is_cavlc);
int  dct_chroma_sp2  (Macroblock *currMB, int uv,int i11, int is_cavlc);


int  distortion4x4(int*);
int  distortion8x8(int*);

extern int*   refbits;
extern int**** motion_cost;
double *mb16x16_cost_frame;

void  FindSkipModeMotionVector  (Macroblock *currMB);
void  Get_Direct_Motion_Vectors (Macroblock *currMB);
void  PartitionMotionSearch     (Macroblock *currMB, int, int, int*);
int   BIDPartitionCost          (Macroblock *currMB, int, int, char[2], int);
int   BPredPartitionCost        (Macroblock *currMB, int, int, short, short, int, int);
int   GetDirectCostMB           (Macroblock *currMB, int bslice);
int   GetDirectCost8x8          (Macroblock *currMB, int, int*);


void poc_based_ref_management_frame_pic(int current_pic_num);
void poc_based_ref_management_field_pic(int current_pic_num);

int  picture_coding_decision (Picture *picture1, Picture *picture2, int qp);

unsigned CeilLog2( unsigned uiVal);


// dynamic mem allocation
int  init_global_buffers(void);
void free_global_buffers(void);
void no_mem_exit  (char *where);

void free_img    (void);

int  get_mem_ACcoeff  (int*****);
int  get_mem_DCcoeff  (int****);
void free_mem_ACcoeff (int****);
void free_mem_DCcoeff (int***);

#if TRACE
void  trace2out(SyntaxElement *se);
void  trace2out_cabac(SyntaxElement *se);
#endif

void error(char *text, int code);

byte  field_flag_inference(Macroblock  *currMB);
void set_mbaff_parameters(Macroblock  *currMB);  // For MB AFF

//============= restriction of reference frames based on the latest intra-refreshes==========
void UpdatePixelMap(void);

int64 compute_SSE(imgpel **imgRef, imgpel **imgSrc, int xRef, int xSrc, int ySize, int xSize);

// Tian Dong: for IGOPs
extern Boolean In2ndIGOP;
extern int start_frame_no_in_this_IGOP;
extern int start_tr_in_this_IGOP;
extern int FirstFrameIn2ndIGOP;
extern int FrameNumberInFile;

int CalculateFrameNumber(void);

#define IMG_NUMBER (img->number - start_frame_no_in_this_IGOP)

void encode_one_macroblock_low (Macroblock *currMB);
void encode_one_macroblock_high (Macroblock *currMB);
void encode_one_macroblock_highfast (Macroblock *currMB);
void encode_one_macroblock_highloss (Macroblock *currMB);
void (*encode_one_macroblock) (Macroblock *currMB);

int is_bipred_enabled(int mode); 

void update_qp    (ImageParameters *img, Macroblock *currMB);
void select_plane (ColorPlane color_plane);
void select_dct   (ImageParameters *img, Macroblock *currMB);

void store_coding_state_cs_cm(Macroblock *currMB);
void reset_coding_state_cs_cm(Macroblock *currMB);

void set_slice_type(int slice_type);

void free_encoder_memory(ImageParameters *img);

int check_for_SI16(void);
int **lrec ;
int ***lrec_uv;
int si_frame_indicator;

int sp2_frame_indicator;
int number_sp2_frames;

void output_SP_coefficients(void);
void read_SP_coefficients(void);

int giRDOpt_B8OnlyFlag;

#ifdef BEST_NZ_COEFF
int gaaiMBAFF_NZCoeff[4][12];
#endif

// Redundant picture
imgpel **imgY_tmp;
imgpel **imgUV_tmp[2];
int  frameNuminGOP;
int  redundant_coding;
int  key_frame;
int  redundant_ref_idx;
void Init_redundant_frame(void);
void Set_redundant_frame(void);
void encode_one_redundant_frame(void);

int img_pad_size_uv_x;
int img_pad_size_uv_y;

unsigned char chroma_mask_mv_y;
unsigned char chroma_mask_mv_x;
int chroma_shift_y, chroma_shift_x;
int shift_cr_x, shift_cr_x2, shift_cr_y;
int img_padded_size_x;
int img_padded_size_x2;
int img_padded_size_x4;
int img_padded_size_x_m8;
int img_padded_size_x_m8x8;
int img_padded_size_x_m4x4;
int img_cr_padded_size_x;
int img_cr_padded_size_x_m8;
int img_cr_padded_size_x2;
int img_cr_padded_size_x4;

// struct with pointers to the sub-images
typedef struct 
{
  imgpel ****luma;    // component 0 (usually Y, X, or R)
  imgpel ****crcb[2]; // component 2 (usually U/V, Y/Z, or G/B)
} SubImageContainer;

int start_me_refinement_hp; // if set then recheck the center position when doing half-pel motion refinement
int start_me_refinement_qp; // if set then recheck the center position when doing quarter-pel motion refinement

//For residual DPCM
int ipmode_DPCM;
int lossless_res[4][4];

// For 4:4:4 independent mode
void change_plane_JV( int nplane );
void make_frame_picture_JV(void);

int Motion_Selected;
int Intra_Selected; 

#endif

