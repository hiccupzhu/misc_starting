
/*!
 ************************************************************************
 *  \file
 *     global.h
 *  \brief
 *     global definitions for H.264 decoder.
 *  \author
 *     Copyright (C) 1999  Telenor Satellite Services,Norway
 *                         Ericsson Radio Systems, Sweden
 *
 *     Inge Lille-Langoy               <inge.lille-langoy@telenor.com>
 *
 *     Telenor Satellite Services
 *     Keysers gt.13                       tel.:   +47 23 13 86 98
 *     N-0130 Oslo,Norway                  fax.:   +47 22 77 79 80
 *
 *     Rickard Sjoberg                 <rickard.sjoberg@era.ericsson.se>
 *
 *     Ericsson Radio Systems
 *     KI/ERA/T/VV
 *     164 80 Stockholm, Sweden
 *
 ************************************************************************
 */
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include <time.h>
#include <sys/timeb.h>
#include "win32.h"
#include "defines.h"
#include "ifunctions.h"
#include "parsetcommon.h"

#define FILE_NAME_SIZE 255

typedef unsigned char  byte;                     //!<  8 bit unsigned
#if (IMGTYPE == 1)
  typedef unsigned short imgpel;                 //!<  Pixel type definition (16 bit for FRExt)
#else
  typedef unsigned char  imgpel;                 //!<  Pixel type definition (8 bit without FRExt)
#endif

pic_parameter_set_rbsp_t *active_pps;
seq_parameter_set_rbsp_t *active_sps;

// global picture format dependent buffers, memory allocation in decod.c
imgpel **imgY_ref;                              //!< reference frame find snr
imgpel ***imgUV_ref;

int **PicPos;
int  ReMapRef[20];
// B pictures
int  Bframe_ctr;
int  frame_no;

int  g_nFrame;

// For MB level frame/field coding
int  TopFieldForSkip_Y[16][16];
int  TopFieldForSkip_UV[2][16][16];

#define ET_SIZE 300      //!< size of error text buffer
char errortext[ET_SIZE]; //!< buffer for error message for exit with error()

/***********************************************************************
 * T y p e    d e f i n i t i o n s    f o r    T M L
 ***********************************************************************
 */

//! Data Partitioning Modes
typedef enum
{
  PAR_DP_1,    //!< no data partitioning is supported
  PAR_DP_3     //!< data partitioning with 3 partitions
} PAR_DP_TYPE;


//! Output File Types
typedef enum
{
  PAR_OF_ANNEXB,   //!< Current TML description
  PAR_OF_RTP       //!< RTP Packet Output format
} PAR_OF_TYPE;


//! definition of H.264 syntax elements
typedef enum 
{
  SE_HEADER,
  SE_PTYPE,
  SE_MBTYPE,
  SE_REFFRAME,
  SE_INTRAPREDMODE,
  SE_MVD,
  SE_CBP_INTRA,
  SE_LUM_DC_INTRA,
  SE_CHR_DC_INTRA,
  SE_LUM_AC_INTRA,
  SE_CHR_AC_INTRA,
  SE_CBP_INTER,
  SE_LUM_DC_INTER,
  SE_CHR_DC_INTER,
  SE_LUM_AC_INTER,
  SE_CHR_AC_INTER,
  SE_DELTA_QUANT_INTER,
  SE_DELTA_QUANT_INTRA,
  SE_BFRAME,
  SE_EOS,
  SE_MAX_ELEMENTS //!< number of maximum syntax elements, this MUST be the last one!
} SE_type;        // substituting the definitions in element.h


typedef enum 
{
  INTER_MB,
  INTRA_MB_4x4,
  INTRA_MB_16x16
} IntraInterDecision;

typedef enum
{
  BITS_HEADER         = 0,
  BITS_TOTAL_MB       = 1,
  BITS_MB_MODE        = 2,
  BITS_INTER_MB       = 3,
  BITS_CBP_MB         = 4,
  BITS_COEFF_Y_MB     = 5,
  BITS_COEFF_CB_MB    = 6,  
  BITS_COEFF_CR_MB    = 7,
  BITS_DELTA_QUANT_MB = 8,
  BITS_STUFFING       = 9,
  MAX_BITCOUNTER_MB   = 10
} BitCountType;

typedef enum 
{
  NO_SLICES,
  FIXED_MB,
  FIXED_RATE,
  CALL_BACK
} SliceMode;


typedef enum 
{
  UVLC,
  CABAC
} SymbolMode;

typedef enum 
{
  LIST_0  = 0,
  LIST_1  = 1,
  BI_PRED = 2
} Lists;


typedef enum 
{
  FRAME,
  TOP_FIELD,
  BOTTOM_FIELD
} PictureStructure;           //!< New enum for field processing

typedef enum
{
  // YUV
  PLANE_Y = 0,  // PLANE_Y
  PLANE_U = 1,  // PLANE_Cb
  PLANE_V = 2,  // PLANE_Cr
  // RGB
  PLANE_G = 0,
  PLANE_B = 1,
  PLANE_R = 2
} ColorPlane;


typedef enum 
{
  P_SLICE = 0,
  B_SLICE,
  I_SLICE,
  SP_SLICE,
  SI_SLICE
} SliceType;

typedef enum
{
  IS_LUMA = 0,
  IS_CHROMA = 1
} Component_Type;

typedef enum
{
  LumaComp = 0,
  CrComp = 1,
  CbComp = 2
} Color_Component;

/***********************************************************************
 * D a t a    t y p e s   f o r  C A B A C
 ***********************************************************************
 */

//! struct to characterize the state of the arithmetic coding engine
typedef struct
{
  unsigned int    Drange;
  unsigned int    Dvalue;
  int             DbitsLeft;
  byte            *Dcodestrm;
  int             *Dcodestrm_len;
} DecodingEnvironment;

typedef DecodingEnvironment *DecodingEnvironmentPtr;

//! struct for context management
typedef struct
{
  unsigned short state;         // index into state-table CP
  unsigned char  MPS;           // Least Probable Symbol 0/1 CP
  unsigned char dummy;          // for alignment
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
  BiContextType mb_type_contexts [4][NUM_MB_TYPE_CTX];
  BiContextType b8_type_contexts [2][NUM_B8_TYPE_CTX];
  BiContextType mv_res_contexts  [2][NUM_MV_RES_CTX];
  BiContextType ref_no_contexts  [2][NUM_REF_NO_CTX];
  BiContextType delta_qp_contexts[NUM_DELTA_QP_CTX];
  BiContextType mb_aff_contexts  [NUM_MB_AFF_CTX];
  BiContextType transform_size_contexts [NUM_TRANSFORM_SIZE_CTX];
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
  BiContextType  map_contexts [2][NUM_BLOCK_TYPES][NUM_MAP_CTX];
  BiContextType  last_contexts[2][NUM_BLOCK_TYPES][NUM_LAST_CTX];
  BiContextType  one_contexts [NUM_BLOCK_TYPES][NUM_ONE_CTX];
  BiContextType  abs_contexts [NUM_BLOCK_TYPES][NUM_ABS_CTX];
} TextureInfoContexts;


//*********************** end of data type definition for CABAC *******************

/***********************************************************************
 * N e w   D a t a    t y p e s   f o r    T M L
 ***********************************************************************
 */

struct img_par;
struct inp_par;
struct stat_par;

/*! Buffer structure for decoded referenc picture marking commands */
typedef struct DecRefPicMarking_s
{
  int memory_management_control_operation;
  int difference_of_pic_nums_minus1;
  int long_term_pic_num;
  int long_term_frame_idx;
  int max_long_term_frame_idx_plus1;
  struct DecRefPicMarking_s *Next;
} DecRefPicMarking_t;

//! Syntaxelement
typedef struct syntaxelement
{
  int           type;                  //!< type of syntax element for data part.
  int           value1;                //!< numerical value of syntax element
  int           value2;                //!< for blocked symbols, e.g. run/level
  int           len;                   //!< length of code
  int           inf;                   //!< info part of UVLC code
  unsigned int  bitpattern;            //!< UVLC bitpattern
  int           context;               //!< CABAC context
  int           k;                     //!< CABAC context for coeff_count,uv

#if TRACE
  #define       TRACESTRING_SIZE 100           //!< size of trace string
  char          tracestring[TRACESTRING_SIZE]; //!< trace string
#endif

  //! for mapping of UVLC to syntaxElement
  void    (*mapping)(int len, int info, int *value1, int *value2);
  //! used for CABAC: refers to actual coding method of each individual syntax element type
  void  (*reading)(struct syntaxelement *, struct img_par *img, DecodingEnvironmentPtr);

} SyntaxElement;

//! Macroblock
typedef struct macroblock
{
  int           qp;                    //!< QP luma
  int           qpc[2];                //!< QP chroma
  int           qp_scaled[MAX_PLANE];  //!< QP scaled for all comps.

  short         slice_nr;
  short         delta_quant;          //!< for rate control

  struct macroblock   *mb_available_up;   //!< pointer to neighboring MB (CABAC)
  struct macroblock   *mb_available_left; //!< pointer to neighboring MB (CABAC)

  // some storage of macroblock syntax elements for global access
  int           mb_type;
  short         mvd[2][BLOCK_MULTIPLE][BLOCK_MULTIPLE][2];      //!< indices correspond to [forw,backw][block_y][block_x][x,y]
  int           cbp;
  int64         cbp_blk ;
  int64         cbp_blk_CbCr[2]; 
  int64         cbp_bits    [3];
  int64         cbp_bits_8x8[3];

  int           i16mode;
  char          b8mode[4];
  char          b8pdir[4];
  char          ei_flag;             //!< error indicator flag that enables concealment
  char          dpl_flag;            //!< error indicator flag that signals a missing data partition

  int           DFDisableIdc;
  int           DFAlphaC0Offset;
  int           DFBetaOffset;

  int           c_ipred_mode;       //!< chroma intra prediction mode
  byte          mb_field;

  int           skip_flag;

  int mbAddrX;
  int mbAddrA, mbAddrB, mbAddrC, mbAddrD;
  int mbAvailA, mbAvailB, mbAvailC, mbAvailD;

  int           luma_transform_size_8x8_flag;
  int           NoMbPartLessThan8x8Flag;
} Macroblock;

//! Bitstream
typedef struct
{
  // CABAC Decoding
  int           read_len;           //!< actual position in the codebuffer, CABAC only
  int           code_len;           //!< overall codebuffer length, CABAC only
  // UVLC Decoding
  int           frame_bitoffset;    //!< actual position in the codebuffer, bit-oriented, UVLC only
  int           bitstream_length;   //!< over codebuffer lnegth, byte oriented, UVLC only
  // ErrorConcealment
  byte          *streamBuffer;      //!< actual codebuffer for read bytes
  int           ei_flag;            //!< error indication, 0: no error, else unspecified error
} Bitstream;

//! DataPartition
typedef struct datapartition
{

  Bitstream           *bitstream;
  DecodingEnvironment de_cabac;

  int     (*readSyntaxElement)(SyntaxElement *, struct img_par *, struct datapartition *);
          /*!< virtual function;
               actual method depends on chosen data partition and
               entropy coding method  */
} DataPartition;

//! Slice
typedef struct
{
  int                 ei_flag;       //!< 0 if the partArr[0] contains valid information
  int                 qp;
  int                 slice_qp_delta;
  int                 qs;
  int                 slice_qs_delta;
  int                 picture_type;  //!< picture type
  PictureStructure    structure;     //!< Identify picture structure type
  int                 start_mb_nr;   //!< MUST be set by NAL even in case of ei_flag == 1
  int                 max_part_nr;
  int                 dp_mode;       //!< data partitioning mode
  int                 next_header;
//  int                 last_mb_nr;    //!< only valid when entropy coding == CABAC
  DataPartition       *partArr;      //!< array of partitions
  MotionInfoContexts  *mot_ctx;      //!< pointer to struct of context models for use in CABAC
  TextureInfoContexts *tex_ctx;      //!< pointer to struct of context models for use in CABAC

  int                 ref_pic_list_reordering_flag_l0;
  int                 *reordering_of_pic_nums_idc_l0;
  int                 *abs_diff_pic_num_minus1_l0;
  int                 *long_term_pic_idx_l0;
  int                 ref_pic_list_reordering_flag_l1;
  int                 *reordering_of_pic_nums_idc_l1;
  int                 *abs_diff_pic_num_minus1_l1;
  int                 *long_term_pic_idx_l1;

  int     (*readSlice)(struct img_par *, struct inp_par *);

  int                 DFDisableIdc;     //!< Disable deblocking filter on slice
  int                 DFAlphaC0Offset;  //!< Alpha and C0 offset for filtering slice
  int                 DFBetaOffset;     //!< Beta offset for filtering slice

  int                 pic_parameter_set_id;   //!<the ID of the picture parameter set the slice is reffering to

  int                 dpB_NotPresent;    //!< non-zero, if data partition B is lost
  int                 dpC_NotPresent;    //!< non-zero, if data partition C is lost
} Slice;

//****************************** ~DM ***********************************

// image parameters
typedef struct img_par
{
  int number;                                 //!< frame number
  unsigned int current_mb_nr; // bitstream order
  unsigned int num_dec_mb;
  int current_slice_nr;
  int *intra_block;
  int tr;                                     //!< temporal reference, 8 bit, wrapps at 255
  int qp;                                     //!< quant for the current frame
  int qpsp;                                   //!< quant for SP-picture predicted frame
  int sp_switch;                              //!< 1 for switching sp, 0 for normal sp
  int direct_spatial_mv_pred_flag;            //!< 1 for Spatial Direct, 0 for Temporal
  int type;                                   //!< image type INTER/INTRA
  int width;
  int height;
  int width_cr;                               //!< width chroma
  int width_cr_m1;                               //!< width chroma
  int height_cr;                              //!< height chroma
  int mb_x;
  int mb_y;
  int block_x;
  int block_y;
  int block_y_aff;
  int pix_x;
  int pix_y;
  int pix_c_x;
  int pix_c_y;

  int allrefzero;
  
  int mvscale[6][MAX_REFERENCE_PICTURES];

  imgpel mb_pred[MAX_PLANE][16][16];     //!< predicted block
  int    mb_rres[MAX_PLANE][16][16];     //!< residual macroblock
  int    cof[MAX_PLANE][16][16];     //!< transformed coefficients 
  int    fcf[MAX_PLANE][16][16];     //!< transformed coefficients 
  
  int cofu[16];
  byte **ipredmode;                  //!< prediction type [90][74]
  int ****nz_coeff;
  int **siblock;
  int cod_counter;                   //!< Current count of number of skipped macroblocks in a row

  int newframe;

  int structure;                     //!< Identify picture structure type

  // B pictures
  Slice      *currentSlice;          //!< pointer to current Slice data struct
  Macroblock *mb_data;               //!< array containing all MBs of a whole frame
  Macroblock *mb_data_JV[MAX_PLANE]; //!< mb_data to be used for 4:4:4 independent mode
  int colour_plane_id;               //!< colour_plane_id of the current coded slice
  int ChromaArrayType;

  int subblock_x;
  int subblock_y;
  int is_intra_block;
  int is_v_block;

  // For MB level frame/field coding
  int MbaffFrameFlag;

  // for signalling to the neighbour logic that this is a deblocker call
  int DeblockCall;

  DecRefPicMarking_t *dec_ref_pic_marking_buffer;                    //!< stores the memory management control operations

  int num_ref_idx_l0_active;             //!< number of forward reference
  int num_ref_idx_l1_active;             //!< number of backward reference

  int slice_group_change_cycle;

  int redundant_pic_cnt;

  int explicit_B_prediction;

  unsigned int pre_frame_num;           //!< store the frame_num in the last decoded slice. For detecting gap in frame_num.

  // End JVT-D101
  // POC200301: from unsigned int to int
           int toppoc;      //poc for this top field // POC200301
           int bottompoc;   //poc of bottom field of frame
           int framepoc;    //poc of this frame // POC200301
  unsigned int frame_num;   //frame_num for this frame
  unsigned int field_pic_flag;
  byte         bottom_field_flag;

  //the following is for slice header syntax elements of poc
  // for poc mode 0.
  unsigned int pic_order_cnt_lsb;
           int delta_pic_order_cnt_bottom;
  // for poc mode 1.
           int delta_pic_order_cnt[3];

  // ////////////////////////
  // for POC mode 0:
    signed int PrevPicOrderCntMsb;
  unsigned int PrevPicOrderCntLsb;
    signed int PicOrderCntMsb;

  // for POC mode 1:
  unsigned int AbsFrameNum;
    signed int ExpectedPicOrderCnt, PicOrderCntCycleCnt, FrameNumInPicOrderCntCycle;
  unsigned int PreviousFrameNum, FrameNumOffset;
           int ExpectedDeltaPerPicOrderCntCycle;
           int PreviousPOC, ThisPOC;
           int PreviousFrameNumOffset;
  // /////////////////////////

  //weighted prediction
  unsigned int luma_log2_weight_denom;
  unsigned int chroma_log2_weight_denom;
  int ***wp_weight;  // weight in [list][index][component] order
  int ***wp_offset;  // offset in [list][index][component] order
  int ****wbp_weight; //weight in [list][fw_index][bw_index][component] order
  int wp_round_luma;
  int wp_round_chroma;
  unsigned int apply_weights;

  int idr_flag;
  int nal_reference_idc;                       //!< nal_reference_idc from NAL unit

  int idr_pic_id;

  int MaxFrameNum;

  unsigned int PicWidthInMbs;
  unsigned int PicHeightInMapUnits;
  unsigned int FrameHeightInMbs;
  unsigned int PicHeightInMbs;
  unsigned int PicSizeInMbs;
  unsigned int FrameSizeInMbs;
  unsigned int oldFrameSizeInMbs;

  int no_output_of_prior_pics_flag;
  int long_term_reference_flag;
  int adaptive_ref_pic_buffering_flag;

  int last_has_mmco_5;
  int last_pic_bottom_field;

  int model_number;

  // Fidelity Range Extensions Stuff
  int pic_unit_bitsize_on_disk;
  int bitdepth_luma;
  int bitdepth_chroma;
  int bitdepth_scale[2];
  int bitdepth_luma_qp_scale;
  int bitdepth_chroma_qp_scale;
  unsigned int dc_pred_value_comp[MAX_PLANE]; //!< component value for DC prediction (depends on component pel bit depth)
  int max_imgpel_value_comp[MAX_PLANE];       //!< max value that one picture element (pixel) can take (depends on pic_unit_bitdepth)
  int Transform8x8Mode;
  int profile_idc;
  int yuv_format;
  int lossless_qpprime_flag;
  int num_blk8x8_uv;
  int num_uv_blocks;
  int num_cdc_coeff;
  int mb_cr_size_x;
  int mb_cr_size_y;
  int mb_cr_size_x_blk;
  int mb_cr_size_y_blk;
  int mb_size[3][2];                         //!< component macroblock dimensions
  int mb_size_blk[3][2];                     //!< component macroblock dimensions 
  int mb_size_shift[3][2];
  
  int max_vmv_r;                             //!< maximum vertical motion vector range in luma quarter frame pixel units for the current level_idc
  int max_mb_vmv_r;                          //!< maximum vertical motion vector range in luma quarter pixel units for the current level_idc

  int idr_psnr_number;
  int psnr_number;

  // Timing related variables
  TIME_T start_time;
  TIME_T end_time;

  // picture error concealment
  int last_ref_pic_poc;
  int ref_poc_gap;
  int poc_gap;
  int conceal_mode;
  int earlier_missing_poc;
  unsigned int frame_to_conceal;
  int IDR_concealment_flag;
  int conceal_slice_type;

  // random access point decoding
  int recovery_point;
  int recovery_point_found;
  int recovery_frame_cnt;
  int recovery_frame_num;
  int recovery_poc;

  int  separate_colour_plane_flag;
} ImageParameters;

extern ImageParameters *img;
extern struct snr_par  *snr;

// signal to noise ratio parameters
struct snr_par
{
  int   frame_ctr;
  float snr[3];                                //!< current SNR (component)
  float snr1[3];                               //!< SNR (dB) first frame (component)
  float snra[3];                               //!< Average component SNR (dB) remaining frames
  float sse[3];                                //!< component SSE 
  float msse[3];                                //!< Average component SSE 
};

time_t tot_time;

// input parameters from configuration file
struct inp_par
{
  char infile[FILE_NAME_SIZE];                       //!< H.264 inputfile
  char outfile[FILE_NAME_SIZE];                      //!< Decoded YUV 4:2:0 output
  char reffile[FILE_NAME_SIZE];                      //!< Optional YUV 4:2:0 reference file for SNR measurement
  int FileFormat;                         //!< File format of the Input file, PAR_OF_ANNEXB or PAR_OF_RTP
  int ref_offset;
  int poc_scale;
  int write_uv;
  int silent;
  int intra_profile_deblocking;               //!< Loop filter usage determined by flags and parameters in bitstream 

#ifdef _LEAKYBUCKET_
  unsigned long R_decoder;                //!< Decoder Rate in HRD Model
  unsigned long B_decoder;                //!< Decoder Buffer size in HRD model
  unsigned long F_decoder;                //!< Decoder Initial buffer fullness in HRD model
  char LeakyBucketParamFile[100];         //!< LeakyBucketParamFile
#endif

  // picture error concealment
  int conceal_mode;
  int ref_poc_gap;
  int poc_gap;
};

extern struct inp_par *params;

typedef struct pix_pos
{
  int  available;
  int  mb_addr;
  int  x;
  int  y;
  int  pos_x;
  int  pos_y;
} PixelPos;

typedef struct old_slice_par
{
   unsigned field_pic_flag;   
   unsigned frame_num;
   int nal_ref_idc;
   unsigned pic_oder_cnt_lsb;
   int delta_pic_oder_cnt_bottom;
   int delta_pic_order_cnt[2];
   byte idr_flag;
   int idr_pic_id;
   int pps_id;
   byte bottom_field_flag;
} OldSliceParams;

extern OldSliceParams old_slice;

// files
int p_out;                    //!< file descriptor to output YUV file
//FILE *p_out2;                    //!< pointer to debug output YUV file
int p_ref;                    //!< pointer to input original reference YUV file file

FILE *p_log;                    //!< SNR file

#if TRACE
FILE *p_trace;
#endif

// Redundant slices
unsigned int previous_frame_num; //!< frame number of previous slice
int ref_flag[17];                //!< 0: i-th previous frame is incorrect
                                 //!< non-zero: i-th previous frame is correct
int Is_primary_correct;          //!< if primary frame is correct, 0: incorrect
int Is_redundant_correct;        //!< if redundant frame is correct, 0:incorrect
int redundant_slice_ref_idx;     //!< reference index of redundant slice
void Error_tracking(void);

// prototypes
void init_conf(struct inp_par *inp, char *config_filename);
void report(struct inp_par *inp, ImageParameters *img, struct snr_par *snr);
void init(ImageParameters *img);

void malloc_slice(struct inp_par *inp, ImageParameters *img);
void free_slice(ImageParameters *img);

int  decode_one_frame(ImageParameters *img,struct inp_par *inp, struct snr_par *snr);
void init_picture(ImageParameters *img, struct inp_par *inp);
void exit_picture(void);

int  read_new_slice(void);
void decode_one_slice(ImageParameters *img, struct inp_par *inp);

void start_macroblock     (ImageParameters *img, Macroblock **currMB);
void read_one_macroblock  (ImageParameters *img, Macroblock *currMB);
Boolean  exit_macroblock  (ImageParameters *img, int eos_bit);
void concealIPCMcoeffs    (ImageParameters *img);

void SetMotionVectorPredictor (ImageParameters  *img, Macroblock *currMB, short pmv[2], char ref_frame, byte list, 
                               char ***refPic, short ****tmp_mv, 
                               int mb_x, int mb_y, int blockshape_x, int blockshape_y);


int  intrapred_luma_16x16(ImageParameters *img, Macroblock *currMB, ColorPlane pl, int predmode);
void intrapred_chroma    (ImageParameters *img, Macroblock *currMB, int uv);

// SLICE function pointers
int  (*nal_startcode_follows) (Slice*, int );

// NAL functions TML/CABAC bitstream
int  uvlc_startcode_follows(Slice *currSlice, int dummy);
int  cabac_startcode_follows(Slice *currSlice, int eos_bit);
void free_Partition(Bitstream *currStream);

// ErrorConcealment
void reset_ec_flags(void);

void error(char *text, int code);
int  is_new_picture(void);
void init_old_slice(void);

// dynamic mem allocation
int  init_global_buffers(void);
void free_global_buffers(void);

void frame_postprocessing(ImageParameters *img);
void field_postprocessing(ImageParameters *img);
void decode_slice(ImageParameters *img,struct inp_par *inp, int current_header);

int RBSPtoSODB(byte *streamBuffer, int last_byte_pos);
int EBSPtoRBSP(byte *streamBuffer, int end_bytepos, int begin_bytepos);

int peekSyntaxElement_UVLC(SyntaxElement *sym, ImageParameters *img, struct datapartition *dP);

void fill_wp_params(ImageParameters *img);

void reset_wp_params(ImageParameters *img);

void FreePartition (DataPartition *dp, int n);
DataPartition *AllocPartition(int n);

void tracebits2(const char *trace_str, int len, int info);

void init_decoding_engine_IPCM(ImageParameters *img);
void readIPCM_CABAC(struct datapartition *dP);

unsigned CeilLog2( unsigned uiVal);
unsigned CeilLog2_sf( unsigned uiVal);

//For residual DPCM
int ipmode_DPCM;

// For 4:4:4 independent mode
void change_plane_JV( int nplane );
void make_frame_picture_JV(void);

#endif


