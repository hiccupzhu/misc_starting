
/*!
 ***********************************************************************
 *  \file
 *      quant.c
 *
 *  \brief
 *      Quantization functions
 *
 *  \author
 *      Main contributors (see contributors.h for copyright, address and affiliation details)
 *
 ***********************************************************************
 */

#include "contributors.h"

#include "global.h"
#include "memalloc.h"
#include "block.h"
#include "image.h"
#include "mb_access.h"
#include "transform.h"
#include "quant.h"

const int dequant_coef8[6][8][8] =
{
  {
    {20,  19, 25, 19, 20, 19, 25, 19},
    {19,  18, 24, 18, 19, 18, 24, 18},
    {25,  24, 32, 24, 25, 24, 32, 24},
    {19,  18, 24, 18, 19, 18, 24, 18},
    {20,  19, 25, 19, 20, 19, 25, 19},
    {19,  18, 24, 18, 19, 18, 24, 18},
    {25,  24, 32, 24, 25, 24, 32, 24},
    {19,  18, 24, 18, 19, 18, 24, 18}
  },
  {
    {22,  21, 28, 21, 22, 21, 28, 21},
    {21,  19, 26, 19, 21, 19, 26, 19},
    {28,  26, 35, 26, 28, 26, 35, 26},
    {21,  19, 26, 19, 21, 19, 26, 19},
    {22,  21, 28, 21, 22, 21, 28, 21},
    {21,  19, 26, 19, 21, 19, 26, 19},
    {28,  26, 35, 26, 28, 26, 35, 26},
    {21,  19, 26, 19, 21, 19, 26, 19}
  },
  {
    {26,  24, 33, 24, 26, 24, 33, 24},
    {24,  23, 31, 23, 24, 23, 31, 23},
    {33,  31, 42, 31, 33, 31, 42, 31},
    {24,  23, 31, 23, 24, 23, 31, 23},
    {26,  24, 33, 24, 26, 24, 33, 24},
    {24,  23, 31, 23, 24, 23, 31, 23},
    {33,  31, 42, 31, 33, 31, 42, 31},
    {24,  23, 31, 23, 24, 23, 31, 23}
  },
  {
    {28,  26, 35, 26, 28, 26, 35, 26},
    {26,  25, 33, 25, 26, 25, 33, 25},
    {35,  33, 45, 33, 35, 33, 45, 33},
    {26,  25, 33, 25, 26, 25, 33, 25},
    {28,  26, 35, 26, 28, 26, 35, 26},
    {26,  25, 33, 25, 26, 25, 33, 25},
    {35,  33, 45, 33, 35, 33, 45, 33},
    {26,  25, 33, 25, 26, 25, 33, 25}
  },
  {
    {32,  30, 40, 30, 32, 30, 40, 30},
    {30,  28, 38, 28, 30, 28, 38, 28},
    {40,  38, 51, 38, 40, 38, 51, 38},
    {30,  28, 38, 28, 30, 28, 38, 28},
    {32,  30, 40, 30, 32, 30, 40, 30},
    {30,  28, 38, 28, 30, 28, 38, 28},
    {40,  38, 51, 38, 40, 38, 51, 38},
    {30,  28, 38, 28, 30, 28, 38, 28}
  },
  {
    {36,  34, 46, 34, 36, 34, 46, 34},
    {34,  32, 43, 32, 34, 32, 43, 32},
    {46,  43, 58, 43, 46, 43, 58, 43},
    {34,  32, 43, 32, 34, 32, 43, 32},
    {36,  34, 46, 34, 36, 34, 46, 34},
    {34,  32, 43, 32, 34, 32, 43, 32},
    {46,  43, 58, 43, 46, 43, 58, 43},
    {34,  32, 43, 32, 34, 32, 43, 32}
  }
};

//! Dequantization coefficients
const int dequant_coef[6][4][4] = {
  {
    { 10, 13, 10, 13},
    { 13, 16, 13, 16},
    { 10, 13, 10, 13},
    { 13, 16, 13, 16}},
  {
    { 11, 14, 11, 14},
    { 14, 18, 14, 18},
    { 11, 14, 11, 14},
    { 14, 18, 14, 18}},
  {
    { 13, 16, 13, 16},
    { 16, 20, 16, 20},
    { 13, 16, 13, 16},
    { 16, 20, 16, 20}},
  {
    { 14, 18, 14, 18},
    { 18, 23, 18, 23},
    { 14, 18, 14, 18},
    { 18, 23, 18, 23}},
  {
    { 16, 20, 16, 20},
    { 20, 25, 20, 25},
    { 16, 20, 16, 20},
    { 20, 25, 20, 25}},
  {
    { 18, 23, 18, 23},
    { 23, 29, 23, 29},
    { 18, 23, 18, 23},
    { 23, 29, 23, 29}}
};

const int quant_coef[6][4][4] = {
  {
    { 13107,  8066, 13107,  8066},
    {  8066,  5243,  8066,  5243},
    { 13107,  8066, 13107,  8066},
    {  8066,  5243,  8066,  5243}},
  {
    { 11916,  7490, 11916,  7490},
    {  7490,  4660,  7490,  4660},
    { 11916,  7490, 11916,  7490},
    {  7490,  4660,  7490,  4660}},
  {
    { 10082,  6554, 10082,  6554},
    {  6554,  4194,  6554,  4194},
    { 10082,  6554, 10082,  6554},
    {  6554,  4194,  6554,  4194}},
  {
    {  9362,  5825,  9362,  5825},
    {  5825,  3647,  5825,  3647},
    {  9362,  5825,  9362,  5825},
    {  5825,  3647,  5825,  3647}},
  {
    {  8192,  5243,  8192,  5243},
    {  5243,  3355,  5243,  3355},
    {  8192,  5243,  8192,  5243},
    {  5243,  3355,  5243,  3355}},
  {
    {  7282,  4559,  7282,  4559},
    {  4559,  2893,  4559,  2893},
    {  7282,  4559,  7282,  4559},
    {  4559,  2893,  4559,  2893}}
};

const int A[4][4] = {
  { 16, 20, 16, 20},
  { 20, 25, 20, 25},
  { 16, 20, 16, 20},
  { 20, 25, 20, 25}
};

int quant_intra_default[16] = {
   6,13,20,28,
  13,20,28,32,
  20,28,32,37,
  28,32,37,42
};

int quant_inter_default[16] = {
  10,14,20,24,
  14,20,24,27,
  20,24,27,30,
  24,27,30,34
};

int quant8_intra_default[64] = {
 6,10,13,16,18,23,25,27,
10,11,16,18,23,25,27,29,
13,16,18,23,25,27,29,31,
16,18,23,25,27,29,31,33,
18,23,25,27,29,31,33,36,
23,25,27,29,31,33,36,38,
25,27,29,31,33,36,38,40,
27,29,31,33,36,38,40,42
};

int quant8_inter_default[64] = {
 9,13,15,17,19,21,22,24,
13,13,17,19,21,22,24,25,
15,17,19,21,22,24,25,27,
17,19,21,22,24,25,27,28,
19,21,22,24,25,27,28,30,
21,22,24,25,27,28,30,32,
22,24,25,27,28,30,32,33,
24,25,27,28,30,32,33,35
};

int quant_org[16] = { //to be use if no q matrix is chosen
16,16,16,16,
16,16,16,16,
16,16,16,16,
16,16,16,16
};

int quant8_org[64] = { //to be use if no q matrix is chosen
16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16,
16,16,16,16,16,16,16,16
};

int *qp_per_matrix;
int *qp_rem_matrix;

int  InvLevelScale4x4_Intra[3][6][4][4];
int  InvLevelScale4x4_Inter[3][6][4][4];
int  InvLevelScale8x8_Intra[3][6][8][8];
int  InvLevelScale8x8_Inter[3][6][8][8];

int  *qmatrix[12];

/*!
 ***********************************************************************
 * \brief
 *    Initiate quantization process arrays
 ***********************************************************************
 */
void init_qp_process(ImageParameters *img)
{
  int bitdepth_qp_scale = imax(img->bitdepth_luma_qp_scale,img->bitdepth_chroma_qp_scale);
  int i;

  // We should allocate memory outside of this process since maybe we will have a change of SPS 
  // and we may need to recreate these. Currently should only support same bitdepth
  if (qp_per_matrix == NULL)
    if ((qp_per_matrix = (int*)malloc((MAX_QP + 1 +  bitdepth_qp_scale)*sizeof(int))) == NULL)
      no_mem_exit("init_global_buffers: qp_per_matrix");
  if (qp_rem_matrix == NULL)
    if ((qp_rem_matrix = (int*)malloc((MAX_QP + 1 +  bitdepth_qp_scale)*sizeof(int))) == NULL)
      no_mem_exit("init_global_buffers: qp_per_matrix");

  for (i = 0; i < MAX_QP + bitdepth_qp_scale + 1; i++)
  {
    qp_per_matrix[i] = i / 6;
    qp_rem_matrix[i] = i % 6;
  }
}

/*!
 ************************************************************************
 * \brief
 *    For mapping the q-matrix to the active id and calculate quantisation values
 *
 * \param pps
 *    Picture parameter set
 * \param sps
 *    Sequence parameter set
 *
 ************************************************************************
 */
void AssignQuantParam(pic_parameter_set_rbsp_t* pps, seq_parameter_set_rbsp_t* sps)
{
  int i;
  int n_ScalingList;

  if(!pps->pic_scaling_matrix_present_flag && !sps->seq_scaling_matrix_present_flag)
  {
    for(i=0; i<12; i++)
      qmatrix[i] = (i < 6) ? quant_org : quant8_org;
  }
  else
  {
    n_ScalingList = (sps->chroma_format_idc != YUV444) ? 8 : 12;
    if(sps->seq_scaling_matrix_present_flag) // check sps first
    {
      for(i=0; i<n_ScalingList; i++)
      {
        if(i<6)
        {
          if(!sps->seq_scaling_list_present_flag[i]) // fall-back rule A
          {
            if(i==0)
              qmatrix[i] = quant_intra_default;
            else if(i==3)
              qmatrix[i] = quant_inter_default;
            else
              qmatrix[i] = qmatrix[i-1];
          }
          else
          {
            if(sps->UseDefaultScalingMatrix4x4Flag[i])
              qmatrix[i] = (i<3) ? quant_intra_default : quant_inter_default;
            else
              qmatrix[i] = sps->ScalingList4x4[i];
          }
        }
        else
        {
          if(!sps->seq_scaling_list_present_flag[i]) // fall-back rule A
          {
            if(i==6)
              qmatrix[i] = quant8_intra_default;
            else if(i==7)
              qmatrix[i] = quant8_inter_default;
            else
              qmatrix[i] = qmatrix[i-2];
          }
          else
          {
            if(sps->UseDefaultScalingMatrix8x8Flag[i-6])
              qmatrix[i] = (i==6 || i==8 || i==10) ? quant8_intra_default:quant8_inter_default;
            else
              qmatrix[i] = sps->ScalingList8x8[i-6];
          }
        }
      }
    }

    if(pps->pic_scaling_matrix_present_flag) // then check pps
    {
      for(i=0; i<n_ScalingList; i++)
      {
        if(i<6)
        {
          if(!pps->pic_scaling_list_present_flag[i]) // fall-back rule B
          {
            if (i==0)
            {
              if(!sps->seq_scaling_matrix_present_flag)
                qmatrix[i] = quant_intra_default;
            }
            else if (i==3)
            {
              if(!sps->seq_scaling_matrix_present_flag)
                qmatrix[i] = quant_inter_default;
            }
            else
              qmatrix[i] = qmatrix[i-1];
          }
          else
          {
            if(pps->UseDefaultScalingMatrix4x4Flag[i])
              qmatrix[i] = (i<3) ? quant_intra_default:quant_inter_default;
            else
              qmatrix[i] = pps->ScalingList4x4[i];
          }
        }
        else
        {
          if(!pps->pic_scaling_list_present_flag[i]) // fall-back rule B
          {
            if (i==6)
            {
              if(!sps->seq_scaling_matrix_present_flag)
                qmatrix[i] = quant8_intra_default;
            }
            else if(i==7)
            {
              if(!sps->seq_scaling_matrix_present_flag)
                qmatrix[i] = quant8_inter_default;
            }
            else  
              qmatrix[i] = qmatrix[i-2];
          }
          else
          {
            if(pps->UseDefaultScalingMatrix8x8Flag[i-6])
              qmatrix[i] = (i==6 || i==8 || i==10) ? quant8_intra_default:quant8_inter_default;
            else
              qmatrix[i] = pps->ScalingList8x8[i-6];
          }
        }
      }
    }
  }

  CalculateQuantParam();
  if(pps->transform_8x8_mode_flag)
    CalculateQuant8Param();
}

/*!
 ************************************************************************
 * \brief
 *    For calculating the quantisation values at frame level
 *
 ************************************************************************
 */
void CalculateQuantParam()
{
  int i, j, k, temp;

  for(k=0; k<6; k++)
  {
    for(i=0; i<4; i++)
    {
      for(j=0; j<4; j++)
      {
        temp = (i<<2)+j;
        InvLevelScale4x4_Intra[0][k][i][j] = dequant_coef[k][i][j] * qmatrix[0][temp];
        InvLevelScale4x4_Intra[1][k][i][j] = dequant_coef[k][i][j] * qmatrix[1][temp];
        InvLevelScale4x4_Intra[2][k][i][j] = dequant_coef[k][i][j] * qmatrix[2][temp];

        InvLevelScale4x4_Inter[0][k][i][j] = dequant_coef[k][i][j] * qmatrix[3][temp];
        InvLevelScale4x4_Inter[1][k][i][j] = dequant_coef[k][i][j] * qmatrix[4][temp];
        InvLevelScale4x4_Inter[2][k][i][j] = dequant_coef[k][i][j] * qmatrix[5][temp];
      }
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Calculate the quantisation and inverse quantisation parameters
 *
 ************************************************************************
 */
void CalculateQuant8Param()
{
  int i, j, k, temp;

  for(k=0; k<6; k++)
  {
    for(i=0; i<8; i++)
    {
      for(j=0; j<8; j++)
      {
        temp = (i<<3)+j;
        InvLevelScale8x8_Intra[0][k][i][j] = dequant_coef8[k][i][j] * qmatrix[6][temp];
        InvLevelScale8x8_Inter[0][k][i][j] = dequant_coef8[k][i][j] * qmatrix[7][temp];
      }
    }
  }

  if( active_sps->chroma_format_idc == 3 )  // 4:4:4
  {
    for(k=0; k<6; k++)
    {
      for(i=0; i<8; i++)
      {
        for(j=0; j<8; j++)
        {
          temp = (i<<3)+j;
          InvLevelScale8x8_Intra[1][k][i][j] = dequant_coef8[k][i][j] * qmatrix[8][temp];
          InvLevelScale8x8_Inter[1][k][i][j] = dequant_coef8[k][i][j] * qmatrix[9][temp];
          InvLevelScale8x8_Intra[2][k][i][j] = dequant_coef8[k][i][j] * qmatrix[10][temp];
          InvLevelScale8x8_Inter[2][k][i][j] = dequant_coef8[k][i][j] * qmatrix[11][temp];
        }
      }
    }
  }
}
