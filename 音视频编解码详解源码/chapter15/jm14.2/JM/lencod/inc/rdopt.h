/*!
 ***************************************************************************
 * \file
 *    rdopt.h
 *
 * \author
 *    Alexis Michael Tourapis
 *
 * \date
 *    2 January 2008
 *
 * \brief
 *    Headerfile for RDO
 **************************************************************************
 */

#ifndef _RDO_H_
#define _RDO_H_

extern int   ****cofAC, ****cofAC8x8;        // [8x8block][4x4block][level/run][scan_pos]
extern int   ***cofDC;                       // [yuv][level/run][scan_pos]
extern int   **cofAC4x4, ****cofAC4x4intern; // [level/run][scan_pos]
extern int   cbp, cbp8x8, cnt_nonz_8x8;
extern int   cbp_blk8x8;
extern char  l0_refframe[4][4], l1_refframe[4][4];
extern short b8mode[4], b8pdir[4];

//CSptr cs_mb, cs_b8, cs_cm, cs_ib8, cs_ib4;
extern int   best_c_imode;
extern int   best_i16offset;
extern short best_mode;

//mixed transform sizes definitions
extern int   luma_transform_size_8x8_flag;

extern short all_mv8x8[2][2][4][4][2];       //[8x8_data/temp_data][LIST][block_x][block_y][MVx/MVy]
extern short pred_mv8x8[2][2][4][4][2];

extern int   ****cofAC8x8ts[3];        // [plane][8x8block][4x4block][level/run][scan_pos]
extern int   ****cofAC8x8CbCr[2];
extern int   **cofAC4x4CbCr[2];
extern int   ****cofAC4x4CbCrintern[2];

extern int64    cbp_blk8_8x8ts;
extern int      cbp8_8x8ts;
extern int      cost8_8x8ts;
extern int      cnt_nonz8_8x8ts;

// adaptive langrangian parameters
extern double mb16x16_cost;
extern double lambda_mf_factor;

int (*Mode_Decision_for_4x4IntraBlocks) (Macroblock *currMB, int  b8,  int  b4,  double  lambda,  double*  min_cost, int cr_cbp[3], int is_cavlc);
double RDCost_for_4x4IntraBlocks (Macroblock *currMB, int* nonzero, int b8, int b4, int ipmode, double lambda, int mostProbableMode, int c_nzCbCr[3], int is_cavlc);
int valid_intra_mode(int ipmode);
void compute_comp_cost(imgpel **cur_img, imgpel prd_img[16][16], int pic_opix_x, int *cost);
void generate_pred_error(imgpel **cur_img, imgpel prd_img[16][16], imgpel cur_prd[16][16], 
                         int m7[16][16], int pic_opix_x, int block_x);
extern void SetMotionVectorPredictor (Macroblock *currMB, short  pmv[2], char   **refPic,
                         short  ***tmp_mv, short  ref_frame,
                         int    list,      int mb_x, int mb_y, 
                         int    blockshape_x, int blockshape_y);
extern void UpdateMotionVectorPredictor(Macroblock* currMB, int mb_type) ;
                         
//============= rate-distortion optimization ===================
void  clear_rdopt (InputParameters *params);
void  init_rdopt  (InputParameters *params);

#endif

