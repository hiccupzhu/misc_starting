
/*!
 ***************************************************************************
 * \file
 *    rdoq.h
 *
 * \brief
 *    Headerfile for trellis based mode decision
 *
 * \author
 *    Limin Liu                       <lliu@dolby.com>
 *    Alexis Michael Tourapis         <alexismt@ieee.org>                
 ***************************************************************************
 */


#ifndef _RDOQ_H_
#define _RDOQ_H_

#include <math.h>


#define MAX_PREC_COEFF    25
#define SIGN_BITS    1

typedef struct 
{
  int  significantBits[16][2];
  int  lastBits[16][2];
  int  greaterOneBits[2][5][2]; // c1 and c2
  int  greaterOneState[5];
  int  blockCbpBits[4][2]; // c1 and c2
} estBitsCabacStruct;

typedef struct 
{
  int level[3];
  int  levelDouble;
  double  errLevel[3];
  int noLevels;
  int coeff_ctr;
  int pre_level;
  int sign;
} levelDataStruct;

extern double norm_factor_4x4;
extern double norm_factor_8x8;


void init_rdoq_slice(int slice_type, int symbol_mode);

/*----------CAVLC related functions----------*/
void est_RunLevel_CAVLC(levelDataStruct *levelData, int *levelTrellis, int block_type, 
                        int b8, int b4, int coeff_num, double lambda);
int est_CAVLC_bits(levelDataStruct *levelData, int level_to_enc[16], int sign_to_enc[16], int nnz, int block_type);

/*----------CABAC related functions----------*/
void precalculate_unary_exp_golomb_level();
int est_unary_exp_golomb_level_bits(unsigned int symbol, int bits0, int bits1);
int est_exp_golomb_encode_eq_prob(unsigned int symbol);

void estRunLevel_CABAC (Macroblock* currMB, int context);
void est_CBP_block_bit (Macroblock* currMB, EncodingEnvironmentPtr eep_dp, int type);
void est_significance_map(Macroblock* currMB, EncodingEnvironmentPtr eep_dp, int type);
void est_significant_coefficients (Macroblock* currMB, EncodingEnvironmentPtr eep_dp,  int type);
int biari_no_bits(EncodingEnvironmentPtr eep, signed short symbol, BiContextTypePtr bi_ct );
int biari_state(EncodingEnvironmentPtr eep, signed short symbol, BiContextTypePtr bi_ct );
int est_write_and_store_CBP_block_bit(Macroblock* currMB, int type);
void est_writeRunLevel_CABAC(levelDataStruct levelData[], int levelTabMin[], int type, double lambda, int kStart, 
                             int kStop, int noCoeff, int estCBP);
int est_unary_exp_golomb_level_encode(unsigned int symbol, int ctx, int type);

void init_trellis_data_4x4_CAVLC(int (*tblock)[16], int block_x, int qp_per, int qp_rem, 
                         int **levelscale, int **leveloffset, const byte *p_scan, Macroblock *currMB,  
                         levelDataStruct *dataLevel, int type);
int init_trellis_data_4x4_CABAC(int (*tblock)[16], int block_x, int qp_per, int qp_rem, 
                         int **levelscale, int **leveloffset, const byte *p_scan, Macroblock *currMB,  
                         levelDataStruct *dataLevel, int* kStart, int* kStop, int type);
void init_trellis_data_8x8_CAVLC(int (*tblock)[16], int block_x, int qp_per, int qp_rem, 
                         int **levelscale, int **leveloffset, const byte *p_scan, Macroblock *currMB,  
                         levelDataStruct levelData[4][16]);
int init_trellis_data_8x8_CABAC(int (*tblock)[16], int block_x, int qp_per, int qp_rem, 
                         int **levelscale, int **leveloffset, const byte *p_scan, Macroblock *currMB,  
                         levelDataStruct *dataLevel, int* kStart, int* kStop, int symbolmode);
void init_trellis_data_DC_CAVLC(int (*tblock)[4], int qp_per, int qp_rem, 
                         int levelscale, int leveloffset, const byte *p_scan, Macroblock *currMB,  
                         levelDataStruct *dataLevel, int type);
int init_trellis_data_DC_CABAC(int (*tblock)[4], int qp_per, int qp_rem, 
                         int levelscale, int leveloffset, const byte *p_scan, Macroblock *currMB,  
                         levelDataStruct *dataLevel, int* kStart, int* kStop);
int init_trellis_data_DC_cr(int (*tblock)[4], int qp_per, int qp_rem, 
                         int levelscale, int **leveloffset, const byte *p_scan, Macroblock *currMB,  
                         levelDataStruct *dataLevel, int* kStart, int* kStop, int type);

void RDOQ_update_mode(RD_PARAMS *enc_mb, int bslice);
void copy_rddata_trellis (RD_DATA *dest, RD_DATA *src);
void updateMV_mp(int *m_cost, short ref, int list, int h, int v, int blocktype, int *lambda_factor, int block8x8);
void trellis_coding(Macroblock *currMB, int CurrentMbAddr, Boolean prev_recode_mb);

#endif  // _RDOQ_H_

