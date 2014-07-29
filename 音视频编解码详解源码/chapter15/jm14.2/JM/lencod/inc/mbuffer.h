
/*!
 ***********************************************************************
 *  \file
 *      mbuffer.h
 *
 *  \brief
 *      Frame buffer functions
 *
 *  \author
 *      Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Karsten Sühring          <suehring@hhi.de>
 ***********************************************************************
 */
#ifndef _MBUFFER_H_
#define _MBUFFER_H_

#include "global.h"
#include "enc_statistics.h"

#define MAX_LIST_SIZE 33

typedef struct picture_stats
{
  double dsum[3];
  double dvar[3];
} PictureStats;


//! definition of pic motion parameters
typedef struct pic_motion_params
{
  int64 ***   ref_pic_id;    //!< reference picture identifier [list][subblock_y][subblock_x]
  int64 ***   ref_id;        //!< reference picture identifier [list][subblock_y][subblock_x]
  short ****  mv;            //!< motion vector       [list][subblock_x][subblock_y][component]
  char  ***   ref_idx;       //!< reference picture   [list][subblock_y][subblock_x]
  byte *      mb_field;      //!< field macroblock indicator
  byte **     field_frame;   //!< indicates if co_located is field or frame.

} PicMotionParams;

//! definition a picture (field or frame)
typedef struct storable_picture
{
  PictureStructure structure;

  int         poc;
  int         top_poc;
  int         bottom_poc;
  int         frame_poc;
  int         order_num;
  int64       ref_pic_num[6][MAX_LIST_SIZE];
  int64       frm_ref_pic_num[6][MAX_LIST_SIZE];
  int64       top_ref_pic_num[6][MAX_LIST_SIZE];
  int64       bottom_ref_pic_num[6][MAX_LIST_SIZE];
  unsigned    frame_num;
  int         pic_num;
  int         long_term_pic_num;
  int         long_term_frame_idx;

  int         is_long_term;
  int         used_for_reference;
  int         is_output;
  int         non_existing;

  int         size_x, size_y, size_x_cr, size_y_cr;
  int         size_x_padded, size_y_padded;
  int         size_x_pad, size_y_pad;
  int         size_x_cr_pad, size_y_cr_pad;
  int         chroma_vector_adjustment;
  int         coded_frame;
  int         MbaffFrameFlag;

  imgpel **   imgY;          //!< Y picture component
  imgpel **** imgY_sub;      //!< Y picture component upsampled (Quarter pel)
  imgpel ***  imgUV;         //!< U and V picture components
  imgpel *****imgUV_sub;     //!< UV picture component upsampled (Quarter/One-Eighth pel)

  //Multiple Decoder Buffers (used if rdopt==3)
  imgpel ***  dec_imgY;       //!< Decoded Y component in multiple hypothetical decoders
  imgpel **** dec_imgUV;      //!< Decoded U and V components in multiple hypothetical decoders
  imgpel ***  p_dec_img[MAX_PLANE];      //!< pointer array for accessing decoded pictures in hypothetical decoders

  byte   ***  mb_error_map;    //!< Map of macroblock errors in hypothetical decoders.

  imgpel **   p_img[MAX_PLANE];          //!< pointer array for accessing imgY/imgUV[]
  imgpel **** p_img_sub[MAX_PLANE];      //!< pointer array for storing top address of imgY_sub/imgUV_sub[]
  imgpel **   p_curr_img;                //!< current int-pel ref. picture area to be used for motion estimation
  imgpel **** p_curr_img_sub;            //!< current sub-pel ref. picture area to be used for motion estimation

  PicMotionParams2 ***mv_info;    //!< Motion info
  PicMotionParams  motion;    //!< Motion info
  PicMotionParams JVmotion[MAX_PLANE];    //!< Motion info for 4:4:4 independent coding

  int colour_plane_id;                     //!< colour_plane_id to be used for 4:4:4 independent mode encoding

  struct storable_picture *top_field;     // for mb aff, if frame for referencing the top field
  struct storable_picture *bottom_field;  // for mb aff, if frame for referencing the bottom field
  struct storable_picture *frame;         // for mb aff, if field for referencing the combined frame

  int         chroma_format_idc;
  int         frame_mbs_only_flag;
  int         frame_cropping_flag;
  int         frame_cropping_rect_left_offset;
  int         frame_cropping_rect_right_offset;
  int         frame_cropping_rect_top_offset;
  int         frame_cropping_rect_bottom_offset;

  PictureStats p_stats;
  StatParameters stats;

} StorablePicture;

//! definition of motion parameters
typedef struct motion_params
{
  int64 ***   ref_pic_id;    //!< reference picture identifier [list][subblock_y][subblock_x]
  short ****  mv;            //!< motion vector       [list][subblock_x][subblock_y][component]
  char  ***   ref_idx;       //!< reference picture   [list][subblock_y][subblock_x]
  byte **     moving_block;
} MotionParams;

//! definition a picture (field or frame)
typedef struct colocated_params
{
  int         mb_adaptive_frame_field_flag;
  int         size_x, size_y;
  byte        is_long_term;

  MotionParams frame;
  MotionParams top;
  MotionParams bottom;

} ColocatedParams;

//! Frame Stores for Decoded Picture Buffer
typedef struct frame_store
{
  int       is_used;                //!< 0=empty; 1=top; 2=bottom; 3=both fields (or frame)
  int       is_reference;           //!< 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used
  int       is_long_term;           //!< 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used
  int       is_orig_reference;      //!< original marking by nal_ref_idc: 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used

  int       is_non_existent;

  unsigned  frame_num;
  int       frame_num_wrap;
  int       long_term_frame_idx;
  int       is_output;
  int       poc;

  StorablePicture *frame;
  StorablePicture *top_field;
  StorablePicture *bottom_field;
} FrameStore;


//! Decoded Picture Buffer
typedef struct decoded_picture_buffer
{
  FrameStore  **fs;
  FrameStore  **fs_ref;
  FrameStore  **fs_ltref;
  unsigned      size;
  unsigned      used_size;
  unsigned      ref_frames_in_buffer;
  unsigned      ltref_frames_in_buffer;
  int           last_output_poc;
  int           max_long_term_pic_idx;

  int           init_done;

  FrameStore   *last_picture;
} DecodedPictureBuffer;


extern DecodedPictureBuffer dpb;
extern StorablePicture **listX[6];
extern int listXsize[6];

void             init_dpb(void);
void             free_dpb(void);
FrameStore*      alloc_frame_store(void);
void             free_frame_store(FrameStore* f);
StorablePicture* alloc_storable_picture(PictureStructure type, int size_x, int size_y, int size_x_cr, int size_y_cr);
void             free_storable_picture(StorablePicture* p);
void             store_picture_in_dpb(StorablePicture* p);
void             replace_top_pic_with_frame(StorablePicture* p);
void             flush_dpb(void);

void             dpb_split_field(FrameStore *fs);
void             dpb_combine_field(FrameStore *fs);
void             dpb_combine_field_yuv(FrameStore *fs);

void             init_lists(int currSliceType, PictureStructure currPicStructure);
void             reorder_ref_pic_list(StorablePicture **list, int *list_size,
                                      int num_ref_idx_lX_active_minus1, int *reordering_of_pic_nums_idc,
                                      int *abs_diff_pic_num_minus1, int *long_term_pic_idx);

void             init_mbaff_lists(void);
void             alloc_ref_pic_list_reordering_buffer(Slice *currSlice);
void             free_ref_pic_list_reordering_buffer(Slice *currSlice);

void             fill_frame_num_gap(ImageParameters *img);

ColocatedParams* alloc_colocated(int size_x, int size_y,int mb_adaptive_frame_field_flag);
void free_colocated(ColocatedParams* p);
void compute_colocated(ColocatedParams* p, StorablePicture **listX[6]);

// For 4:4:4 independent mode
void compute_colocated_JV(ColocatedParams* p, StorablePicture **listX[6]);
void copy_storable_param_JV( int nplane, StorablePicture *d, StorablePicture *s );

#endif

