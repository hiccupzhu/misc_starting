
/*!
*************************************************************************************
* \file img_chroma.c
*
* \brief
*    Chroma interpolation functions
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*      - Athanasios Leontaris    <aleon@dolby.com>
*      - Alexis Michael Tourapis <alexis.tourapis@dolby.com>
*
*************************************************************************************
*/

#include "contributors.h"

#include <limits.h>

#include "global.h"
#include "image.h"
#include "img_chroma.h"


/*!
 ************************************************************************
 * \brief
 *    Integer sample position image generation routine
 ************************************************************************
 */
static void generateChroma00( int size_x_minus1, int size_y_minus1, imgpel **wImgDst, imgpel **imgUV)
{
  int i, j;
  int jpad;
  static imgpel *wBufDst;
  static imgpel *wBufSrc0;


  for (j = -img_pad_size_uv_y, jpad = 0; j < 0; j++, jpad++)
  {

    wBufDst = &( wImgDst[jpad][0]);
    wBufSrc0 = imgUV[0];

    for (i = -img_pad_size_uv_x; i < 0; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }

    memcpy(wBufDst, wBufSrc0, size_x_minus1 * sizeof(imgpel));
    wBufDst  += size_x_minus1;
    wBufSrc0 += size_x_minus1;

    for (i = 0; i < img_pad_size_uv_x; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }
  }

  for (j = 0; j < size_y_minus1; j++, jpad++)
  {
    wBufDst = &( wImgDst[jpad][0] );
    wBufSrc0 = imgUV[j    ];

    for (i = -img_pad_size_uv_x; i < 0; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }

    memcpy(wBufDst, wBufSrc0, size_x_minus1 * sizeof(imgpel));
    wBufDst  += size_x_minus1;
    wBufSrc0 += size_x_minus1;

    for (i = 0; i < img_pad_size_uv_x; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }
  }

  for (j = 0; j < img_pad_size_uv_y; j++, jpad++)
  {
    wBufDst = &( wImgDst[jpad][0] );
    wBufSrc0 = imgUV[size_y_minus1];

    for (i = -img_pad_size_uv_x; i < 0; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }

    memcpy(wBufDst, wBufSrc0, size_x_minus1 * sizeof(imgpel));
    wBufDst  += size_x_minus1;
    wBufSrc0 += size_x_minus1;

    for (i = 0; i < img_pad_size_uv_x; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Horizontal chroma interpolation image generation routine
 ************************************************************************
 */
static void generateChroma01( int size_x_minus1, int size_y_minus1, int weight00, int weight01, imgpel **wImgDst, imgpel **imgUV)
{
  int i, j;
  int jpad;
  int cur_value;
  static imgpel *wBufDst;
  static imgpel *wBufSrc0;


  for (j = -img_pad_size_uv_y, jpad = 0; j < 0; j++, jpad++)
  {
    wBufDst  = wImgDst[jpad];
    wBufSrc0 = imgUV[0];

    for (i = -img_pad_size_uv_x; i < 0; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }

    for (i = 0; i < size_x_minus1; i++)
    {
      cur_value  = weight00 * (*wBufSrc0++);
      cur_value += weight01 * (*wBufSrc0  );
      *(wBufDst++) = (imgpel) rshift_rnd_sf(cur_value, 6);
    }

    for (i = 0; i < img_pad_size_uv_x; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }
  }

  for (j = 0; j < size_y_minus1; j++, jpad++)
  {
    wBufDst  = wImgDst[jpad];
    wBufSrc0 = imgUV[j    ];

    for (i = -img_pad_size_uv_x; i < 0; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }

    for (i = 0; i < size_x_minus1; i++)
    {
      cur_value  = weight00 * (*wBufSrc0++);
      cur_value += weight01 * (*wBufSrc0  );
      *(wBufDst++) = (imgpel) rshift_rnd_sf(cur_value, 6);
    }

    for (i = 0; i < img_pad_size_uv_x; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }
  }

  for (j = 0; j < img_pad_size_uv_y; j++, jpad++)
  {
    wBufDst  = wImgDst[jpad];
    wBufSrc0 = imgUV[size_y_minus1];

    for (i = -img_pad_size_uv_x; i < 0; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }

    for (i = 0; i < size_x_minus1; i++)
    {
      cur_value  = weight00 * (*wBufSrc0++);
      cur_value += weight01 * (*wBufSrc0  );
      *(wBufDst++) = (imgpel) rshift_rnd_sf(cur_value, 6);
    }

    for (i = 0; i < img_pad_size_uv_x; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Vertical chroma interpolation image generation routine
 ************************************************************************
 */
static void generateChroma10( int size_x_minus1, int size_y_minus1, int weight00, int weight10, imgpel **wImgDst, imgpel **imgUV)
{
  int i, j;
  int jpad;
  int cur_value;
  static imgpel *wBufDst;
  static imgpel *wBufSrc0, *wBufSrc1;


  for (j = -img_pad_size_uv_y, jpad = 0; j < 0; j++, jpad++)
  {
    wBufDst = &( wImgDst[jpad][0] );
    wBufSrc0 = imgUV[0];

    for (i = -img_pad_size_uv_x; i < 0; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }

    memcpy(wBufDst, wBufSrc0, size_x_minus1 * sizeof(imgpel));
    wBufDst  += size_x_minus1;
    wBufSrc0 += size_x_minus1;

    for (i = 0; i < img_pad_size_uv_x; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }
  }

  for (j = 0; j < size_y_minus1; j++, jpad++)
  {
    wBufDst = &( wImgDst[jpad][0] );
    wBufSrc0 = imgUV[j    ];
    wBufSrc1 = imgUV[j + 1];

    cur_value = rshift_rnd_sf(weight00 * (*wBufSrc0) + weight10 * (*wBufSrc1), 6 );
    for (i = -img_pad_size_uv_x; i < 0; i++)
    {
      *(wBufDst++) = (imgpel) cur_value;
    }

    for (i = 0; i < size_x_minus1; i++)
    {
      cur_value  = weight00 * (*wBufSrc0++) + weight10 * (*wBufSrc1++);
      *(wBufDst++) = (imgpel) rshift_rnd_sf(cur_value, 6);
    }

    cur_value = rshift_rnd_sf(weight00 * (*wBufSrc0) + weight10 * (*wBufSrc1), 6 );
    for (i = 0; i < img_pad_size_uv_x; i++)
    {
      *(wBufDst++) = (imgpel) cur_value;
    }
  }

  for (j = 0; j < img_pad_size_uv_y; j++, jpad++)
  {
    wBufDst = &( wImgDst[jpad][0] );
    wBufSrc0 = imgUV[size_y_minus1];

    for (i = -img_pad_size_uv_x; i < 0; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }

    memcpy(wBufDst, wBufSrc0, size_x_minus1 * sizeof(imgpel));
    wBufDst  += size_x_minus1;
    wBufSrc0 += size_x_minus1;

    for (i = 0; i < img_pad_size_uv_x; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Generic/Diagonal chroma interpolation image generation routine
 ************************************************************************
 */
static void generateChromaXX( int size_x_minus1, int size_y_minus1, int weight00, int weight01, int weight10, int weight11, imgpel **wImgDst, imgpel **imgUV)
{
  int i, j;
  int jpad;
  int cur_value;
  static imgpel *wBufDst;
  static imgpel *wBufSrc0, *wBufSrc1;
  int weight0001 = weight00 + weight01;
  int weight1011 = weight10 + weight11;
  int weight0010 = weight00 + weight10;
  int weight0111 = weight01 + weight11;


  for (j = -img_pad_size_uv_y, jpad = 0; j < 0; j++, jpad++)
  {
    wBufDst = &( wImgDst[jpad][0] );
    wBufSrc0 = imgUV[0];

    for (i = -img_pad_size_uv_x; i < 0; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }

    for (i = 0; i < size_x_minus1; i++)
    {
      cur_value  = weight0010 * (*wBufSrc0++);
      cur_value += weight0111 * (*wBufSrc0  );
      *(wBufDst++) = (imgpel) rshift_rnd_sf(cur_value, 6);
    }

    for (i = 0; i < img_pad_size_uv_x; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }
  }

  for (j = 0; j < size_y_minus1; j++, jpad++)
  {
    wBufDst = &( wImgDst[jpad][0] );
    wBufSrc0 = imgUV[j    ];
    wBufSrc1 = imgUV[j + 1];

    cur_value = rshift_rnd_sf(weight0001 * (*wBufSrc0) + weight1011 * (*wBufSrc1), 6 );
    for (i = -img_pad_size_uv_x; i < 0; i++)
    {
      *(wBufDst++) = (imgpel) cur_value;
    }

    for (i = 0; i < size_x_minus1; i++)
    {
      cur_value  = weight00 * (*wBufSrc0++) + weight10 * (*wBufSrc1++);
      cur_value += weight01 * (*wBufSrc0  ) + weight11 * (*wBufSrc1  );
      *(wBufDst++) = (imgpel) rshift_rnd_sf(cur_value, 6);
    }

    cur_value = rshift_rnd_sf(weight0001 * (*wBufSrc0) + weight1011 * (*wBufSrc1), 6 );
    for (i = 0; i < img_pad_size_uv_x; i++)
    {
      *(wBufDst++) = (imgpel) cur_value;
    }
  }

  for (j = 0; j < img_pad_size_uv_y; j++, jpad++)
  {
    wBufDst = &( wImgDst[jpad][0] );
    wBufSrc0 = imgUV[size_y_minus1];

    for (i = -img_pad_size_uv_x; i < 0; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }

    for (i = 0; i < size_x_minus1; i++)
    {
      cur_value  = weight0010 * (*wBufSrc0++);
      cur_value += weight0111 * (*wBufSrc0  );
      *(wBufDst++) = (imgpel) rshift_rnd_sf(cur_value, 6);
    }

    for (i = 0; i < img_pad_size_uv_x; i++)
    {
      *(wBufDst++) = *wBufSrc0;
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Creates the 16 (YUV444), 32 (YUV422), or 64 (YUV420) sub-images that
 *    contain quarter-pel samples sub-sampled at different
 *    spatial orientationss;
 *      enables more efficient implementation
 *
 * \param s
 *    pointer to StorablePicture structure
 s************************************************************************
 */
void getSubImagesChroma( StorablePicture *s )
{
  int uv, k, l, m;
  int weight00, weight01, weight10, weight11;
  int subimages_y, subimages_x, subx, suby;
  int size_x_minus1, size_y_minus1;

  // multiplier factor for index to account for UV sampling ratios
  int mul_x, mul_y;
  int mm, kk;

  imgpel **wImgDst;  

  size_x_minus1 = s->size_x_cr - 1;
  size_y_minus1 = s->size_y_cr - 1;

  if ( img->yuv_format == YUV420 ) 
  {
    subimages_x = 8;
    subimages_y = 8;
    mul_x = mul_y = 1;
  }
  else if ( img->yuv_format == YUV422 ) 
  {
    subimages_x = 8;
    subimages_y = 4;
    mul_y = 2;
    mul_x = 1;
  }
  else 
  { // YUV444
    subimages_x = 4;
    subimages_y = 4;
    mul_x = mul_y = 2;
  }

  // U or V
  for ( uv = 0; uv < 2; uv++ )
  {
    for ( suby = 0, k = 0; suby < subimages_y; suby++, k += mul_y )
    {
      m = (8 - k);
      mm = m << 3;
      kk = k << 3;
      for ( subx = 0, l = 0; subx < subimages_x; subx++, l += mul_x )
      {
        weight01 = m * l;
        weight00 = mm - weight01;
        weight11 = k * l;
        weight10 = kk - weight11; 
        wImgDst = s->imgUV_sub[uv][suby][subx];

        // Lets break things into cases
        if (weight01 == 0 && weight10 == 0 && weight11 == 0) // integer
        {
          generateChroma00( size_x_minus1, size_y_minus1, wImgDst, s->imgUV[uv]);
        }
        else if (weight10 == 0 && weight11 == 0) // horizontal
        {
          generateChroma01( size_x_minus1, size_y_minus1, weight00, weight01, wImgDst, s->imgUV[uv]);
        }
        else if (weight01 == 0 && weight11 == 0) // vertical
        {
          generateChroma10( size_x_minus1, size_y_minus1, weight00, weight10, wImgDst, s->imgUV[uv]);
        }
        else //diagonal
        {
          generateChromaXX( size_x_minus1, size_y_minus1, weight00, weight01, weight10, weight11, wImgDst, s->imgUV[uv]);
        }          
      }
    }
  }
}

