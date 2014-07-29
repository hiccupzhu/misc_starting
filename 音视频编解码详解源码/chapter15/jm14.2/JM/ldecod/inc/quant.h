
/*!
 ************************************************************************
 * \file quant.h
 *
 * \brief
 *    definitions for quantization functions
 *
 * \author
 *
 ************************************************************************
 */

#ifndef _QUANT_H_
#define _QUANT_H_

// Macro defines
#define Q_BITS          15
#define DQ_BITS          6
#define Q_BITS_8        16
#define DQ_BITS_8        6 

// exported variables
extern const int dequant_coef[6][4][4];
extern const int quant_coef[6][4][4];

extern int InvLevelScale4x4_Intra[3][6][4][4];
extern int InvLevelScale4x4_Inter[3][6][4][4];
extern int InvLevelScale8x8_Intra[3][6][8][8];
extern int InvLevelScale8x8_Inter[3][6][8][8];

extern int *qmatrix[12];

// SP decoding parameter (EQ. 8-425)
extern const int A[4][4];

extern int *qp_per_matrix;
extern int *qp_rem_matrix;

// exported functions
// quantization initialization
void init_qp_process(ImageParameters *img);

// For Q-matrix
void AssignQuantParam(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps);
void CalculateQuantParam(void);
void CalculateQuant8Param(void);

#endif

