
/*!
 ************************************************************************
 * \file
 *    macroblock.h
 *
 * \brief
 *    Arrays for macroblock processing
 *
 * \author
 *    Inge Lille-Langoy               <inge.lille-langoy@telenor.com>     \n
 *    Telenor Satellite Services                                          \n
 *    P.O.Box 6914 St.Olavs plass                                         \n
 *    N-0130 Oslo, Norway
 *
 ************************************************************************/

#ifndef _MACROBLOCK_H_
#define _MACROBLOCK_H_

void  proceed2nextMacroblock(Macroblock* currMB);
void  start_macroblock(Slice *currSlice, Macroblock** currMB, int mb_addr, byte mb_field);
void  reset_macroblock(Macroblock *currMB, int prev_mb);
void  terminate_macroblock(Slice *currSlice, Macroblock* currMB, Boolean *end_of_slice, Boolean *recode_macroblock);

void  write_one_macroblock(Macroblock* currMB, int eos_bit, Boolean prev_recode_mb);

int  LumaResidualCoding8x8 (Macroblock* currMB, int*, int64*, int, short, int, int, short, short, int);
void LumaResidualCoding (Macroblock *currMB, int is_cavlc);

void ChromaResidualCoding (Macroblock *currMB, int is_cavlc);

void IntraChromaPrediction (Macroblock *currMB, int*, int*, int*);
void IntraChromaRDDecision (Macroblock *currMB, RD_PARAMS);

int  TransformDecision(Macroblock *currMB, int, int*);

int  B8Mode2Value (short b8mode, short b8pdir);

int  writeMBLayer (Macroblock *currMB, int rdopt, int *coeff_rate);
void write_terminating_bit (short bit);

int  writeReferenceFrame  (Macroblock *currMB, int mode, int i, int j, int fwd_flag, int  ref);
int  writeMotionVector8x8 (Macroblock *currMB, int  i0, int  j0, int  i1, int  j1, int  refframe, int  list_idx, int  mv_mode, short bipred_me);

int  writeCoeff4x4_CABAC (Macroblock *currMB, ColorPlane, int, int, int);
int  writeCoeff8x8_CABAC (Macroblock* currMB, ColorPlane, int, int);
int  writeCoeff8x8       (Macroblock* currMB, ColorPlane, int, int, int);
int  writeCoeff16x16     (Macroblock* currMB, ColorPlane, int);

int  writeCoeff4x4_CAVLC (Macroblock* currMB, int block_type, int b8, int b4, int param);

int distortion_sad(imgpel **img_org, imgpel pred_img[16][16]);
int distortion_sse(imgpel **img_org, imgpel pred_img[16][16]);
int distortion_hadamard(imgpel **img_org, imgpel pred_img[16][16]);

double (*find_sad_16x16) (Macroblock *currMB, int *intra_mode);
double find_sad_16x16_JM (Macroblock *currMB, int *intra_mode);

void SetLagrangianMultipliersOn();
void SetLagrangianMultipliersOff();
void (*SetLagrangianMultipliers)();

int   predict_nnz(Macroblock *currMB, int block_type, int i,int j);
int   predict_nnz_chroma(Macroblock *currMB, int i,int j);


#endif

