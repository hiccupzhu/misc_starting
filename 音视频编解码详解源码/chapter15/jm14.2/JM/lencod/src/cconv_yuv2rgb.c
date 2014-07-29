
/*!
 *************************************************************************************
 * \file img_distortion.c
 *
 * \brief
 *    YUV to RGB color conversion
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Woo-Shik Kim                    <wooshik.kim@usc.edu>
 *************************************************************************************
 */
#include "contributors.h"
#include "global.h"
#include "memalloc.h"
#include "img_distortion.h"

#define YUV2RGB_YOFFSET

//YUV to RGB conversion
#ifdef YUV2RGB_YOFFSET
#define OFFSET_Y 16
static const float K0 = 1.164f;
static const float K1 = 1.596f;
static const float K2 = 0.391f;
static const float K3 = 0.813f;
static const float K4 = 2.018f;
static int offset_y, offset_cr;
#else
static const float K0 = 1.000f;
static const float K1 = 1.402f;
static const float K2 = 0.34414f;
static const float K3 = 0.71414f;
static const float K4 = 1.772f;
#endif
static int wk0, wk1, wk2, wk3, wk4;

ImageStructure imgRGB_src, imgRGB_ref;

int create_RGB_memory(ImageParameters *img)
{
  int memory_size = 0;
  int j;
  for( j = 0; j < 3; j++ )
  {
    memory_size += get_mem2Dpel (&imgRGB_src.data[j], img->height, img->width);
  }
  for( j = 0; j < 3; j++ )
  {
    memory_size += get_mem2Dpel (&imgRGB_ref.data[j], img->height, img->width);
  }
  
  return memory_size;
}

void delete_RGB_memory(void)
{
  int i;
  for( i = 0; i < 3; i++ )
  {
    free_mem2Dpel(imgRGB_src.data[i]);
  }
  for( i = 0; i < 3; i++ )
  {
    free_mem2Dpel(imgRGB_ref.data[i]);
  }
}

void init_YUVtoRGB(void)
{ 
  float conv_scale = (float) (65536.0f);

  wk0 = float2int(  conv_scale * K0);
  wk1 = float2int(  conv_scale * K1);
  wk2 = float2int( -conv_scale * K2);
  wk3 = float2int( -conv_scale * K3);
  wk4 = float2int(  conv_scale * K4);

#ifdef YUV2RGB_YOFFSET
  offset_y = OFFSET_Y << (params->output.bit_depth[0] - 8);
  offset_cr = 1 << (params->output.bit_depth[0] - 1);
#endif
}

/*! 
*************************************************************************************
* \brief
*    YUV to RGB conversion
*    ITU 601 with and without offset consideration
*    Upsampling by repetition of chroma samples in case of 4:2:0 and 4:2:2
*    Method not support for 4:0:0 content
*************************************************************************************
*/
void YUVtoRGB(ImageStructure *YUV, ImageStructure *RGB)
{
  static int i, j, j_cr, i_cr;
  static int sy, su, sv;
  static int wbuv, wguv, wruv;
  static imgpel *Y, *U, *V, *R, *G, *B;
  FrameFormat format = YUV->format;
  int width = format.width;
  int height = format.height;
  int max_value = format.max_value[0];

  // Color conversion
  for (j = 0; j < height; j++)
  {
    j_cr = j >> shift_cr_y;
    Y = YUV->data[0][j];
    U = YUV->data[1][j_cr];
    V = YUV->data[2][j_cr];
    R = RGB->data[0][j];
    G = RGB->data[1][j];
    B = RGB->data[2][j];

    for (i = 0; i < width; i++)
    {
      i_cr = i >> shift_cr_x;

      su = U[i_cr] - offset_cr;
      sv = V[i_cr] - offset_cr;

      wruv = wk1 * sv;
      wguv = wk2 * su + wk3 * sv;
      wbuv = wk4 * su;

#ifdef YUV2RGB_YOFFSET // Y offset value of 16 is considered
      sy = wk0 * (Y[i] - offset_y);
#else
      sy = wk0 * Y[i];
#endif

      R[i] = iClip1( max_value, rshift_rnd(sy + wruv, 16));
      G[i] = iClip1( max_value, rshift_rnd(sy + wguv, 16));
      B[i] = iClip1( max_value, rshift_rnd(sy + wbuv, 16));
    }
  }
  // Setting RGB FrameFormat
  RGB->format = format;  // copy format information from YUV to RGB
  RGB->format.yuv_format = 3;
  RGB->format.rgb_format = 1;
  RGB->format.height_cr = format.height;
  RGB->format.width_cr = format.width;
  for (i = 1; i < 3; i++)
  {
    RGB->format.size_cmp[i] = format.size_cmp[0];
    RGB->format.bit_depth[i] = format.bit_depth[0];
    RGB->format.max_value[i] = max_value;
    RGB->format.max_value_sq[i] = format.max_value_sq[0];
  }
}
