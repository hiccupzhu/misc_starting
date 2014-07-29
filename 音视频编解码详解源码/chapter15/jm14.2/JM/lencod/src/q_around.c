
/*!
 *************************************************************************************
 * \file q_around.c
 *
 * \brief
 *    Quantization Adaptive Rounding (JVT-N011)
 * \author
 *    - Alexis Michael Tourapis    <alexismt@ieee.org>
 *    - Ehsan Maani                <emaan@dolby.com>
 *
 *************************************************************************************
 */

#include "global.h"
#include "memalloc.h"
#include "q_offsets.h"
#include "q_around.h"

static const int AdaptRndCrPos[2][5] =
{
  //  P,   B,   I,  SP,  SI
  {   4,   7,   1,   4,   1}, // Intra MB
  {  10,  13,  10,  10,  10}  // Inter MB
};

static const int AdaptRndPos[4][5] =
{
  //  P,   B,   I,  SP,  SI
  {   3,   6,   0,   3,   0}, // 4x4 Intra MB
  {   1,   2,   0,   1,   2}, // 8x8 Intra MB
  {   9,  12,   9,   9,   9}, // 4x4 Inter MB
  {   3,   4,   3,   3,   3}, // 8x8 Inter MB
};

ARoundOffset bestOffset;

int   **fadjust8x8 = NULL, **fadjust4x4 = NULL, ***fadjust4x4Cr = NULL, ***fadjust8x8Cr = NULL;

static void allocate_offset_memory(InputParameters *params, ARoundOffset *offset)
{
  get_mem2Dint(&offset->InterFAdjust4x4, 16, 16);
  get_mem2Dint(&offset->IntraFAdjust4x4, 16, 16);
  get_mem2Dint(&offset->InterFAdjust8x8, 16, 16);
  get_mem2Dint(&offset->IntraFAdjust8x8, 16, 16);

  if (params->yuv_format != 0 )
  {
    if  (params->yuv_format == YUV444)
    {
      get_mem3Dint(&offset->InterFAdjust8x8Cr, 2, 16, 16);
      get_mem3Dint(&offset->IntraFAdjust8x8Cr, 2, 16, 16);
    }
    get_mem3Dint(&offset->InterFAdjust4x4Cr, 2, img->mb_cr_size_y, img->mb_cr_size_x);
    get_mem3Dint(&offset->IntraFAdjust4x4Cr, 2, img->mb_cr_size_y, img->mb_cr_size_x);
  }
}

static void free_offset_memory(InputParameters *params, ARoundOffset *offset)
{
  free_mem2Dint(offset->InterFAdjust4x4);
  free_mem2Dint(offset->IntraFAdjust4x4);
  free_mem2Dint(offset->InterFAdjust8x8);
  free_mem2Dint(offset->IntraFAdjust8x8);

  if (params->yuv_format != 0 )
  {
    if  (params->yuv_format == YUV444)
    {
      free_mem3Dint(offset->InterFAdjust8x8Cr);
      free_mem3Dint(offset->IntraFAdjust8x8Cr);
    }
    free_mem3Dint(offset->InterFAdjust4x4Cr);
    free_mem3Dint(offset->IntraFAdjust4x4Cr);
  }
}


void setup_adaptive_rounding (InputParameters *params)
{
  allocate_offset_memory(params, &bestOffset);

  get_mem2Dint(&fadjust8x8, 16, 16);
  get_mem2Dint(&fadjust4x4, 16, 16);
  if (params->yuv_format != 0 )
  {
    get_mem3Dint(&fadjust4x4Cr, 2, img->mb_cr_size_y, img->mb_cr_size_x);
    get_mem3Dint(&fadjust8x8Cr, 2, img->mb_cr_size_y, img->mb_cr_size_x);
  }
}

void clear_adaptive_rounding (InputParameters *params)
{
  free_offset_memory(params, &bestOffset);

  free_mem2Dint(fadjust8x8);
  free_mem2Dint(fadjust4x4);
  if (params->yuv_format != 0)
  {
    free_mem3Dint(fadjust4x4Cr);
    free_mem3Dint(fadjust8x8Cr);
  }
}

void store_adaptive_rounding (ImageParameters *img, int block_y, int block_x)
{
  int j;

  for (j = block_y; j < block_y + 4; j++)
    memcpy(&fadjust4x4[j][block_x],&img->fadjust4x4[1][j][block_x], BLOCK_SIZE * sizeof(int));

  if (img->P444_joined)
  {
    for (j = block_y; j < block_y + 4; j++)
    {
      memcpy(&fadjust4x4Cr[0][j][block_x],&img->fadjust4x4Cr[0][1][j][block_x], BLOCK_SIZE * sizeof(int));
    }
    for (j = block_y; j < block_y + 4; j++)
    {
      memcpy(&fadjust4x4Cr[1][j][block_x],&img->fadjust4x4Cr[1][1][j][block_x], BLOCK_SIZE * sizeof(int));
    }
  }
}

void update_adaptive_rounding(ImageParameters *img, int block_y, int block_x)
{
  int j;

  for (j = block_y; j < block_y + BLOCK_SIZE; j++)
    memcpy (&img->fadjust4x4[1][j][block_x],&fadjust4x4[j][block_x], BLOCK_SIZE * sizeof(int));

  if (img->P444_joined)
  {
    for (j = 0; j < block_y + BLOCK_SIZE; j++)
    {
      memcpy (&img->fadjust4x4Cr[0][1][j][block_x],&fadjust4x4Cr[0][j][block_x], BLOCK_SIZE * sizeof(int));
    }
    for (j = 0; j < block_y + BLOCK_SIZE; j++)
    {
      memcpy (&img->fadjust4x4Cr[1][1][j][block_x],&fadjust4x4Cr[1][j][block_x], BLOCK_SIZE * sizeof(int));
    }
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Store adaptive rounding luma parameters
 *************************************************************************************
 */

void store_adaptive_rounding_parameters_luma (Macroblock *currMB, int mode)
{
   int is_inter = (mode != I4MB) && (mode != I16MB) && (mode != I8MB);

  if (currMB->luma_transform_size_8x8_flag)
  {
    if ((mode == P8x8))
      memcpy(&(bestOffset.InterFAdjust8x8[0][0]),&(img->fadjust8x8[2][0][0]), MB_PIXELS * sizeof(int));
    else if (is_inter)
      memcpy(&(bestOffset.InterFAdjust8x8[0][0]),&(img->fadjust8x8[0][0][0]), MB_PIXELS * sizeof(int));
    else
      memcpy(&(bestOffset.IntraFAdjust8x8[0][0]),&(img->fadjust8x8[1][0][0]), MB_PIXELS * sizeof(int));
  }
  else
  {
    if ((mode == P8x8))
      memcpy(&(bestOffset.InterFAdjust4x4[0][0]),&(img->fadjust4x4[3][0][0]),MB_PIXELS * sizeof(int));
    else if (is_inter)
      memcpy(&(bestOffset.InterFAdjust4x4[0][0]),&(img->fadjust4x4[0][0][0]),MB_PIXELS * sizeof(int));
    else
      memcpy(&(bestOffset.IntraFAdjust4x4[0][0]),&(img->fadjust4x4[1 + mode == I16MB][0][0]),MB_PIXELS * sizeof(int));
  }
}


/*!
 *************************************************************************************
 * \brief  
 *    Store adaptive rounding chroma parameters
 *************************************************************************************
 */
void store_adaptive_rounding_parameters_chroma (Macroblock *currMB, int mode)
{
  int j;
  int is_inter = (mode != I4MB)&&(mode != I16MB)&&(mode != I8MB);

  if (img->P444_joined)
  {
    if (currMB->luma_transform_size_8x8_flag)
    {
      if ((mode == P8x8))
      {
        memcpy(&(bestOffset.InterFAdjust8x8Cr[0][0][0]),&(img->fadjust8x8Cr[0][2][0][0]), MB_PIXELS * sizeof(int));
        memcpy(&(bestOffset.InterFAdjust8x8Cr[1][0][0]),&(img->fadjust8x8Cr[1][2][0][0]), MB_PIXELS * sizeof(int));
      }
      else if (is_inter)
      {
        memcpy(&(bestOffset.InterFAdjust8x8Cr[0][0][0]),&(img->fadjust8x8Cr[0][0][0][0]), MB_PIXELS * sizeof(int));
        memcpy(&(bestOffset.InterFAdjust8x8Cr[1][0][0]),&(img->fadjust8x8Cr[1][0][0][0]), MB_PIXELS * sizeof(int));
      }
      else
      {
        memcpy(&(bestOffset.IntraFAdjust8x8Cr[0][0][0]),&(img->fadjust8x8Cr[0][1][0][0]), MB_PIXELS * sizeof(int));
        memcpy(&(bestOffset.IntraFAdjust8x8Cr[1][0][0]),&(img->fadjust8x8Cr[1][1][0][0]), MB_PIXELS * sizeof(int));
      }
	}
    else
    {
      if ((mode == P8x8))
      {
        memcpy(&(bestOffset.InterFAdjust4x4Cr[0][0][0]),&(img->fadjust4x4Cr[0][3][0][0]), MB_PIXELS * sizeof(int));
        memcpy(&(bestOffset.InterFAdjust4x4Cr[1][0][0]),&(img->fadjust4x4Cr[1][3][0][0]), MB_PIXELS * sizeof(int));
      }
      else if (is_inter)
      {
        memcpy(&(bestOffset.InterFAdjust4x4Cr[0][0][0]),&(img->fadjust4x4Cr[0][0][0][0]), MB_PIXELS * sizeof(int));
        memcpy(&(bestOffset.InterFAdjust4x4Cr[1][0][0]),&(img->fadjust4x4Cr[1][0][0][0]), MB_PIXELS * sizeof(int));
      }
      else
      {
        memcpy(&(bestOffset.IntraFAdjust4x4Cr[0][0][0]),&(img->fadjust4x4Cr[0][1 + mode == I16MB][0][0]), MB_PIXELS * sizeof(int));
        memcpy(&(bestOffset.IntraFAdjust4x4Cr[1][0][0]),&(img->fadjust4x4Cr[1][1 + mode == I16MB][0][0]), MB_PIXELS * sizeof(int));
      }
    }
  }

  if ( (params->yuv_format == YUV420 || params->yuv_format == YUV422) && (params->AdaptRndChroma))  
  {
    if (currMB->luma_transform_size_8x8_flag && mode == P8x8)
    {
      for (j = 0; j < img->mb_cr_size_y; j++)
      {
        memcpy(bestOffset.InterFAdjust4x4Cr[0][j],img->fadjust8x8Cr[0][0][j],img->mb_cr_size_x * sizeof(int));
        memcpy(bestOffset.InterFAdjust4x4Cr[1][j],img->fadjust8x8Cr[1][0][j],img->mb_cr_size_x * sizeof(int));
      }
    }
    else if (mode == P8x8)
    {
      for (j = 0; j < img->mb_cr_size_y; j++)
      {
        memcpy(bestOffset.InterFAdjust4x4Cr[0][j],img->fadjust4x4Cr[0][2][j],img->mb_cr_size_x * sizeof(int));
        memcpy(bestOffset.InterFAdjust4x4Cr[1][j],img->fadjust4x4Cr[1][2][j],img->mb_cr_size_x * sizeof(int));
      }
    }
    else if (is_inter)
    {
      for (j = 0; j < img->mb_cr_size_y; j++)
      {
        memcpy(bestOffset.InterFAdjust4x4Cr[0][j],img->fadjust4x4Cr[0][0][j],img->mb_cr_size_x * sizeof(int));
        memcpy(bestOffset.InterFAdjust4x4Cr[1][j],img->fadjust4x4Cr[1][0][j],img->mb_cr_size_x * sizeof(int));
      }
    }
    else
    {
      for (j = 0; j < img->mb_cr_size_y; j++)
      {
        memcpy(bestOffset.IntraFAdjust4x4Cr[0][j],img->fadjust4x4Cr[0][1][j],img->mb_cr_size_x * sizeof(int));
        memcpy(bestOffset.IntraFAdjust4x4Cr[1][j],img->fadjust4x4Cr[1][1][j],img->mb_cr_size_x * sizeof(int));
      }
    }
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Store all adaptive rounding parameters
 *************************************************************************************
 */
void store_adaptive_rounding_parameters (Macroblock *currMB, int mode)
{
  store_adaptive_rounding_parameters_luma (currMB, mode);
  store_adaptive_rounding_parameters_chroma (currMB, mode);
}

/*!
************************************************************************
* \brief
*    update rounding offsets based on JVT-N011
************************************************************************
*/
void update_offset_params(Macroblock *currMB, int mode, int luma_transform_size_8x8_flag)
{
  int is_inter = (mode != I4MB)&&(mode != I16MB) && (mode != I8MB);
  int luma_pos = AdaptRndPos[(is_inter<<1) + luma_transform_size_8x8_flag][img->type];
  int i,j;
  int qp = currMB->qp + img->bitdepth_luma_qp_scale;
  int cur_qp = params->AdaptRoundingFixed ? 0 : qp;
  int temp = 0;
  int offsetRange = 1 << (OffsetBits - 1);
  int blk_mask = 0x03 + (luma_transform_size_8x8_flag<<2);
  int blk_shift = 2 + luma_transform_size_8x8_flag;
  short **offsetList = luma_transform_size_8x8_flag ? OffsetList8x8[cur_qp] : OffsetList4x4[cur_qp];

  int **fAdjust = is_inter
    ? (luma_transform_size_8x8_flag ? bestOffset.InterFAdjust8x8 : bestOffset.InterFAdjust4x4)
    : (luma_transform_size_8x8_flag ? bestOffset.IntraFAdjust8x8 : bestOffset.IntraFAdjust4x4);

  if( (active_sps->chroma_format_idc == YUV444) && IS_INDEPENDENT(params) )
  {
    if( luma_transform_size_8x8_flag )  // 8x8
      luma_pos += 5 * img->colour_plane_id;
    else  // 4x4
      luma_pos += img->colour_plane_id;
  }

  for (j=0; j < MB_BLOCK_SIZE; j++)
  {
    int j_pos = ((j & blk_mask)<<blk_shift);
    for (i=0; i < MB_BLOCK_SIZE; i++)
    {
      temp = j_pos + (i & blk_mask);
      offsetList[luma_pos][temp] += fAdjust[j][i];
      offsetList[luma_pos][temp]  = iClip3(0, offsetRange, offsetList[luma_pos][temp]);
    }
  }

  if(img->P444_joined)
  { 
    int ***fAdjustCbCr = (int ***) (is_inter
      ? (luma_transform_size_8x8_flag ? bestOffset.InterFAdjust8x8Cr : bestOffset.InterFAdjust4x4Cr)
      : (luma_transform_size_8x8_flag ? bestOffset.IntraFAdjust8x8Cr : bestOffset.IntraFAdjust4x4Cr));
    int uv;

    for(uv = 0; uv < 2; uv++)
    {
      luma_pos = AdaptRndPos[(is_inter<<1) + luma_transform_size_8x8_flag][img->type];
      if(luma_transform_size_8x8_flag )  // 8x8
        luma_pos += 5 * (uv+1);
      else  // 4x4
        luma_pos += (uv+1);
      for (j=0; j < MB_BLOCK_SIZE; j++)
      {
        int j_pos = ((j & blk_mask)<<blk_shift);
        for (i=0; i < MB_BLOCK_SIZE; i++)
        {
          temp = j_pos + (i & blk_mask);
          offsetList[luma_pos][temp] += fAdjustCbCr[uv][j][i];
          offsetList[luma_pos][temp] = iClip3(0,offsetRange,offsetList[luma_pos][temp]);
        }
      }
    }
  }

  if ((params->AdaptRndChroma) && (params->yuv_format == YUV420 || params->yuv_format == YUV422 ))
  {
    int u_pos = AdaptRndCrPos[is_inter][img->type];
    int v_pos = u_pos + 1;
    int jpos;

    int ***fAdjustCr = is_inter ? bestOffset.InterFAdjust4x4Cr : bestOffset.IntraFAdjust4x4Cr;

    for (j = 0; j < img->mb_cr_size_y; j++)
    {
      jpos = ((j & 0x03)<<2);
      for (i = 0; i < img->mb_cr_size_x; i++)
      {
        temp = jpos + (i & 0x03);
        OffsetList4x4[cur_qp][u_pos][temp] += fAdjustCr[0][j][i];
        OffsetList4x4[cur_qp][u_pos][temp]  = iClip3(0,offsetRange,OffsetList4x4[cur_qp][u_pos][temp]);
        OffsetList4x4[cur_qp][v_pos][temp] += fAdjustCr[1][j][i];
        OffsetList4x4[cur_qp][v_pos][temp]  = iClip3(0,offsetRange,OffsetList4x4[cur_qp][v_pos][temp]);
      }
    }
  }
}


