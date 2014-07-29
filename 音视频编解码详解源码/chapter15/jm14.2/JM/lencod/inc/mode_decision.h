/*!
 ***************************************************************************
 * \file
 *    mode_decision.h
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Alexis Michael Tourapis         <alexismt@ieee.org>
 *
 * \date
 *    21. February 2005
 *
 * \brief
 *    Headerfile for mode decision
 **************************************************************************
 */

#ifndef _MODE_DECISION_H_
#define _MODE_DECISION_H_

extern CSptr cs_mb, cs_b8, cs_cm, cs_ib8, cs_ib4;
extern RD_8x8DATA tr4x4, tr8x8;

// Adaptive Lagrangian variables
extern double mb16x16_cost;
extern double lambda_mf_factor;


extern int    ****cofAC8x8ts[3];        // [8x8block][4x4block][level/run][scan_pos]
extern int    ****cofAC, ****cofAC8x8;        // [8x8block][4x4block][level/run][scan_pos]
extern int   ****cofAC8x8CbCr[2];
extern int    QP2QUANT[40];
extern int    cbp_blk8x8;
extern int    cbp, cbp8x8, cnt_nonz_8x8;
extern int64  cbp_blk8_8x8ts;
extern int    cbp8_8x8ts;
extern int    cnt_nonz8_8x8ts;

// Residue Color Transform
extern char   b4_ipredmode[16], b4_intra_pred_modes[16];

extern short  best_mode;
extern imgpel pred[16][16];

extern void   set_stored_macroblock_parameters (Macroblock *currMB);
extern void   StoreMV8x8(int);
extern void   RestoreMV8x8(int);
extern void   store_macroblock_parameters (Macroblock *currMB, int);
extern void   SetModesAndRefframeForBlocks (Macroblock *currMB, int);
extern void   SetRefAndMotionVectors (Macroblock *currMB, int, int, int, int, int, short);
extern void   StoreNewMotionVectorsBlock8x8(int, int, int, int, int, int, int, int);
extern void   assign_enc_picture_params(int, char, int, int, int, int, int, short);
extern void   set_subblock8x8_info(Block8x8Info*, int, int, RD_8x8DATA*);
extern void   set_block8x8_info(Block8x8Info*, int, int, char[2], char, short);
extern void   update_prediction_for_mode16x16(Block8x8Info*, int, int*);
extern void   update_refresh_map(int intra, int intra1, Macroblock *currMB);
extern void   SetMotionVectorsMB (Macroblock*, int);
extern void   SetCoeffAndReconstruction8x8 (Macroblock*);

extern int    GetBestTransformP8x8(void);
extern int    I16Offset (int, int);
extern int    CheckReliabilityOfRef (int, int, int, int);
extern int    Mode_Decision_for_Intra4x4Macroblock (Macroblock *currMB, double, double*, int is_cavlc);
extern int    RDCost_for_macroblocks (Macroblock  *currMB, double, int, double*, double*, double*, int, int);
extern double RDCost_for_8x8blocks (Macroblock  *currMB, int*, int64*, double, int, int, short, short, short, short, int);
extern double *mb16x16_cost_frame;

extern const int  b8_mode_table[6];
extern const int  mb_mode_table[9];

void rc_store_diff(int cpix_x, int cpix_y, imgpel prediction[16][16]);
void submacroblock_mode_decision(RD_PARAMS *, RD_8x8DATA *, Macroblock *,int ***, int ***, int ***, int *, short, int, int *, int *, int *, int, int);
void init_enc_mb_params(Macroblock* currMB, RD_PARAMS *enc_mb, int intra, int bslice);
void list_prediction_cost(Macroblock *currMB, int list, int block, int mode, RD_PARAMS *enc_mb, int bmcost[5], char best_ref[2]);
void determine_prediction_list(int, int [5], char [2], char *, int *, short *);
void compute_mode_RD_cost(int mode, Macroblock *currMB, RD_PARAMS *enc_mb,
                               double *min_rdcost, double *min_dcost, double *min_rate,
                               int i16mode, short bslice, short *inter_skip, int is_cavlc);

int iminarray ( int arr[], int size, int *minind ); 

extern void update_lambda_costs(RD_PARAMS *enc_mb, int lambda_mf[3]);


void get_initial_mb16x16_cost(Macroblock* currMB);
void adjust_mb16x16_cost(int);

int64 (*getDistortion)( Macroblock *currMB );
int64 distortionSSE   ( Macroblock *currMB );
#endif

