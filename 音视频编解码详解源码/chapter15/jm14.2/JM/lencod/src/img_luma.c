/*!
*************************************************************************************
* \file img_luma.c
*
* \brief
*    Luma interpolation functions
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*      - Alexis Michael Tourapis <alexis.tourapis@dolby.com>
*      - Athanasios Leontaris    <aleon@dolby.com>
*
*************************************************************************************
*/

#include "contributors.h"

#include <limits.h>

#include "global.h"
#include "image.h"
#include "img_luma.h"
#include "memalloc.h"

const int ONE_FOURTH_TAP[2][3] =
{
  {20, -5, 1},  // AVC Interpolation taps
  {20,-4, 0},   // Experimental - not valid
};

/*!
 ************************************************************************
 * \brief
 *    Creates the 4x4 = 16 images that contain quarter-pel samples
 *    sub-sampled at different spatial orientations;
 *    enables more efficient implementation
 *
 * \param s
 *    pointer to StorablePicture structure
 s************************************************************************
 */
void getSubImagesLuma( StorablePicture *s )
{
  imgpel  **p_curr_img = s->p_curr_img;
  imgpel ****cImgSub   = s->p_curr_img_sub;

  //  0  1  2  3
  //  4  5  6  7
  //  8  9 10 11
  // 12 13 14 15

  //// INTEGER PEL POSITIONS ////

  // sub-image 0 [0][0]
  // simply copy the integer pels
  getSubImageInteger( s, cImgSub[0][0], p_curr_img);

  //// HALF-PEL POSITIONS: SIX-TAP FILTER ////

  // sub-image 2 [0][2]
  // HOR interpolate (six-tap) sub-image [0][0]
  getHorSubImageSixTap( s, cImgSub[0][2], cImgSub[0][0] );

  // sub-image 8 [2][0]
  // VER interpolate (six-tap) sub-image [0][0]
  getVerSubImageSixTap( s, cImgSub[2][0], cImgSub[0][0]);

  // sub-image 10 [2][2]
  // VER interpolate (six-tap) sub-image [0][2]
  getVerSubImageSixTapTmp( s, cImgSub[2][2], cImgSub[0][2]);

  //// QUARTER-PEL POSITIONS: BI-LINEAR INTERPOLATION ////

  // sub-image 1 [0][1]
  getSubImageBiLinear    ( s, cImgSub[0][1], cImgSub[0][0], cImgSub[0][2]);
  // sub-image 4 [1][0]
  getSubImageBiLinear    ( s, cImgSub[1][0], cImgSub[0][0], cImgSub[2][0]);
  // sub-image 5 [1][1]
  getSubImageBiLinear    ( s, cImgSub[1][1], cImgSub[0][2], cImgSub[2][0]);
  // sub-image 6 [1][2]
  getSubImageBiLinear    ( s, cImgSub[1][2], cImgSub[0][2], cImgSub[2][2]);
  // sub-image 9 [2][1]
  getSubImageBiLinear    ( s, cImgSub[2][1], cImgSub[2][0], cImgSub[2][2]);

  // sub-image 3  [0][3]
  getHorSubImageBiLinear ( s, cImgSub[0][3], cImgSub[0][2], cImgSub[0][0]);
  // sub-image 7  [1][3]
  getHorSubImageBiLinear ( s, cImgSub[1][3], cImgSub[0][2], cImgSub[2][0]);
  // sub-image 11 [2][3]
  getHorSubImageBiLinear ( s, cImgSub[2][3], cImgSub[2][2], cImgSub[2][0]);

  // sub-image 12 [3][0]
  getVerSubImageBiLinear ( s, cImgSub[3][0], cImgSub[2][0], cImgSub[0][0]);
  // sub-image 13 [3][1]
  getVerSubImageBiLinear ( s, cImgSub[3][1], cImgSub[2][0], cImgSub[0][2]);
  // sub-image 14 [3][2]
  getVerSubImageBiLinear ( s, cImgSub[3][2], cImgSub[2][2], cImgSub[0][2]);

  // sub-image 15 [3][3]
  getDiagSubImageBiLinear( s, cImgSub[3][3], cImgSub[0][2], cImgSub[2][0]);
}


/*!
 ************************************************************************
 * \brief
 *    Copy Integer Samples to image [0][0]
 *
 * \param s
 *    pointer to StorablePicture structure
 * \param dstImg
 *    destination image
 * \param srcImg
 *    source image
 ************************************************************************
 */
void getSubImageInteger( StorablePicture *s, imgpel **dstImg, imgpel **srcImg)
{
  int i, j;
  int size_x_minus1 = s->size_x - 1;

  static imgpel *wBufSrc, *wBufDst;

  // Copy top line
  wBufDst = &( dstImg[0][0] ); 
  wBufSrc = srcImg[0];
  // left IMG_PAD_SIZE
  for (i = 0; i < IMG_PAD_SIZE; i++)
    *(wBufDst++) = wBufSrc[0];
  // center 0-(s->size_x)
  memcpy(wBufDst, wBufSrc, s->size_x * sizeof(imgpel));
  wBufDst += s->size_x;
  // right IMG_PAD_SIZE
  for (i = 0; i < IMG_PAD_SIZE; i++)
    *(wBufDst++) = wBufSrc[size_x_minus1];

  // Now copy remaining pad lines
  for (j = 1; j < IMG_PAD_SIZE + 1; j++)
  {
    memcpy(dstImg[j], dstImg[j - 1], s->size_x_padded * sizeof(imgpel));
  }

  for (j = 1; j < s->size_y; j++)
  {    
    wBufDst = &( dstImg[j + IMG_PAD_SIZE][0] ); // 4:4:4 independent mode
    wBufSrc = srcImg[j];
    // left IMG_PAD_SIZE
    for (i = 0; i < IMG_PAD_SIZE; i++)
      *(wBufDst++) = wBufSrc[0];
    // center 0-(s->size_x)
    memcpy(wBufDst, wBufSrc, s->size_x * sizeof(imgpel));
    wBufDst += s->size_x;
    // right IMG_PAD_SIZE
    for (i = 0; i < IMG_PAD_SIZE; i++)
      *(wBufDst++) = wBufSrc[size_x_minus1];
  }

  // Replicate bottom pad lines
  for (j = s->size_y + IMG_PAD_SIZE; j < s->size_y_padded; j++)
  {    
    memcpy(dstImg[j], dstImg[j - 1], s->size_x_padded * sizeof(imgpel));
  }
}

/*!
 ************************************************************************
 * \brief
 *    Does _horizontal_ interpolation using the SIX TAP filters
 *
 * \param s
 *    pointer to StorablePicture structure
 * \param dstImg
 *    destination image
 * \param srcImg
 *    source image
 ************************************************************************
 */
void getHorSubImageSixTap( StorablePicture *s, imgpel **dstImg, imgpel **srcImg)
{
  int is, jpad, ipad;
  int ypadded_size = s->size_y_padded;
  int xpadded_size = s->size_x_padded;

  static imgpel *wBufSrc, *wBufDst;
  static imgpel *srcImgA, *srcImgB, *srcImgC, *srcImgD, *srcImgE, *srcImgF;
  static int *iBufDst;
  const int tap0 = ONE_FOURTH_TAP[0][0];
  const int tap1 = ONE_FOURTH_TAP[0][1];
  const int tap2 = ONE_FOURTH_TAP[0][2];

  for (jpad = 0; jpad < ypadded_size; jpad++)
  {
    wBufSrc = srcImg[jpad];     // 4:4:4 independent mode
    wBufDst = dstImg[jpad];     // 4:4:4 independent mode
    iBufDst = imgY_sub_tmp[jpad];

    srcImgA = &wBufSrc[0];
    srcImgB = &wBufSrc[0];      
    srcImgC = &wBufSrc[0];
    srcImgD = &wBufSrc[1];
    srcImgE = &wBufSrc[2];
    srcImgF = &wBufSrc[3];

    // left padded area
    is =
      (tap0 * (*srcImgA++ + *srcImgD++) +
      tap1 *  (*srcImgB   + *srcImgE++) +
      tap2 *  (*srcImgC   + *srcImgF++));

    *iBufDst++ =  is;
    *wBufDst++ = (imgpel) iClip1 ( img->max_imgpel_value, rshift_rnd_sf( is, 5 ) );      

    is =
      (tap0 * (*srcImgA++ + *srcImgD++) +
      tap1 *  (*srcImgB++ + *srcImgE++) +
      tap2 *  (*srcImgC   + *srcImgF++));

    *iBufDst++ =  is;
    *wBufDst++ = (imgpel) iClip1 ( img->max_imgpel_value, rshift_rnd_sf( is, 5 ) );      

    // center
    for (ipad = 2; ipad < xpadded_size - 4; ipad++)
    {
      is =
        (tap0 * (*srcImgA++ + *srcImgD++) +
        tap1 *  (*srcImgB++ + *srcImgE++) +
        tap2 *  (*srcImgC++ + *srcImgF++));

      *iBufDst++ =  is;
      *wBufDst++ = (imgpel) iClip1 ( img->max_imgpel_value, rshift_rnd_sf( is, 5 ) );      
    }

    is = (
      tap0 * (*srcImgA++ + *srcImgD++) +
      tap1 * (*srcImgB++ + *srcImgE++) +
      tap2 * (*srcImgC++ + *srcImgF  ));

    *iBufDst++ =  is;
    *wBufDst++ = (imgpel) iClip1 ( img->max_imgpel_value, rshift_rnd_sf( is, 5 ) );      

    // right padded area
    is = (
      tap0 * (*srcImgA++ + *srcImgD++) +
      tap1 * (*srcImgB++ + *srcImgE) +
      tap2 * (*srcImgC++ + *srcImgF));

    *iBufDst++ =  is;
    *wBufDst++ = (imgpel) iClip1 ( img->max_imgpel_value, rshift_rnd_sf( is, 5 ) );      

    is = (
      tap0 * (*srcImgA++ + *srcImgD) +
      tap1 * (*srcImgB++ + *srcImgE) +
      tap2 * (*srcImgC++ + *srcImgF));

    *iBufDst++ =  is;
    *wBufDst++ = (imgpel) iClip1 ( img->max_imgpel_value, rshift_rnd_sf( is, 5 ) );      

    is = (
      tap0 * (*srcImgA + *srcImgD) +
      tap1 * (*srcImgB + *srcImgE) +
      tap2 * (*srcImgC + *srcImgF));

    *iBufDst =  is;
    *wBufDst = (imgpel) iClip1 ( img->max_imgpel_value, rshift_rnd_sf( is, 5 ) );      

  }
}


/*!
 ************************************************************************
 * \brief
 *    Does _vertical_ interpolation using the SIX TAP filters
 *
 * \param s
 *    pointer to StorablePicture structure
 * \param dstImg
 *    pointer to target image
 * \param srcImg
 *    pointer to source image
 ************************************************************************
 */
void getVerSubImageSixTap( StorablePicture *s, imgpel **dstImg, imgpel **srcImg)
{
  int is, jpad, ipad;
  int ypadded_size = s->size_y_padded;
  int xpadded_size = s->size_x_padded;
  int maxy = ypadded_size - 1;

  static imgpel *wxLineDst;
  static imgpel *srcImgA, *srcImgB, *srcImgC, *srcImgD, *srcImgE, *srcImgF;
  const int tap0 = ONE_FOURTH_TAP[0][0];
  const int tap1 = ONE_FOURTH_TAP[0][1];
  const int tap2 = ONE_FOURTH_TAP[0][2];

  // branches within the j loop
  // top
  for (jpad = 0; jpad < 2; jpad++)
  {
    wxLineDst = dstImg[jpad];
    srcImgA = srcImg[jpad ];
    srcImgB = srcImg[0];      
    srcImgC = srcImg[0];
    srcImgD = srcImg[jpad + 1];
    srcImgE = srcImg[jpad + 2];
    srcImgF = srcImg[jpad + 3];
    for (ipad = 0; ipad < xpadded_size; ipad++)
    {
      is =
        (tap0 * (*srcImgA++ + *srcImgD++) +
        tap1 *  (*srcImgB++ + *srcImgE++) +
        tap2 *  (*srcImgC++ + *srcImgF++));

      wxLineDst[ipad] = (imgpel) iClip1 (img->max_imgpel_value, rshift_rnd_sf( is, 5 ) );
    }
  }
  // center
  for (jpad = 2; jpad < ypadded_size - 3; jpad++)
  {
    wxLineDst = dstImg[jpad];
    srcImgA = srcImg[jpad ];
    srcImgB = srcImg[jpad - 1];      
    srcImgC = srcImg[jpad - 2];
    srcImgD = srcImg[jpad + 1];
    srcImgE = srcImg[jpad + 2];
    srcImgF = srcImg[jpad + 3];
    for (ipad = 0; ipad < xpadded_size; ipad++)
    {
      is =
        (tap0 * (*srcImgA++ + *srcImgD++) +
        tap1 *  (*srcImgB++ + *srcImgE++) +
        tap2 *  (*srcImgC++ + *srcImgF++));

      wxLineDst[ipad] = (imgpel) iClip1 ( img->max_imgpel_value, rshift_rnd_sf( is, 5 ) );
    }
  }

  // bottom
  for (jpad = ypadded_size - 3; jpad < ypadded_size; jpad++)
  {
    wxLineDst = dstImg[jpad];
    srcImgA = srcImg[jpad ];
    srcImgB = srcImg[jpad - 1];      
    srcImgC = srcImg[jpad - 2];
    srcImgD = srcImg[imin (maxy, jpad + 1)];
    srcImgE = srcImg[maxy];
    srcImgF = srcImg[maxy];
    for (ipad = 0; ipad < xpadded_size; ipad++)
    {
      is =
        (tap0 * (*srcImgA++ + *srcImgD++) +
        tap1 *  (*srcImgB++ + *srcImgE++) +
        tap2 *  (*srcImgC++ + *srcImgF++));

      wxLineDst[ipad] = (imgpel) iClip1 ( img->max_imgpel_value, rshift_rnd_sf( is, 5 ) );
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Does _vertical_ interpolation using the SIX TAP filters
 *
 * \param s
 *    pointer to StorablePicture structure
 * \param dstImg
 *    pointer to target image
 * \param srcImg
 *    pointer to source image
 ************************************************************************
 */
void getVerSubImageSixTapTmp( StorablePicture *s, imgpel **dstImg, imgpel **srcImg)
{
  int is, jpad, ipad;
  int ypadded_size = s->size_y_padded;
  int xpadded_size = s->size_x_padded;
  int maxy = ypadded_size - 1;

  static imgpel *wxLineDst;
  static int *srcImgA, *srcImgB, *srcImgC, *srcImgD, *srcImgE, *srcImgF;
  const int tap0 = ONE_FOURTH_TAP[0][0];
  const int tap1 = ONE_FOURTH_TAP[0][1];
  const int tap2 = ONE_FOURTH_TAP[0][2];

  // top
  for (jpad = 0; jpad < 2; jpad++)
  {
    wxLineDst = dstImg[jpad];
    srcImgA = imgY_sub_tmp[jpad ];
    srcImgB = imgY_sub_tmp[0];      
    srcImgC = imgY_sub_tmp[0];
    srcImgD = imgY_sub_tmp[jpad + 1];
    srcImgE = imgY_sub_tmp[jpad + 2];
    srcImgF = imgY_sub_tmp[jpad + 3];

    for (ipad = 0; ipad < xpadded_size; ipad++)
    {
      is =
        (tap0 * (*srcImgA++ + *srcImgD++) +
        tap1 *  (*srcImgB++ + *srcImgE++) +
        tap2 *  (*srcImgC++ + *srcImgF++));

      wxLineDst[ipad] = (imgpel) iClip1 ( img->max_imgpel_value, rshift_rnd_sf( is, 10 ) );
    }
  }

  // center
  for (jpad = 2; jpad < ypadded_size - 3; jpad++)
  {
    wxLineDst = dstImg[jpad];
    srcImgA = imgY_sub_tmp[jpad ];
    srcImgB = imgY_sub_tmp[jpad - 1];      
    srcImgC = imgY_sub_tmp[jpad - 2];
    srcImgD = imgY_sub_tmp[jpad + 1];
    srcImgE = imgY_sub_tmp[jpad + 2];
    srcImgF = imgY_sub_tmp[jpad + 3];
    for (ipad = 0; ipad < xpadded_size; ipad++)
    {
      is =
        (tap0 * (*srcImgA++ + *srcImgD++) +
        tap1 *  (*srcImgB++ + *srcImgE++) +
        tap2 *  (*srcImgC++ + *srcImgF++));

      wxLineDst[ipad] = (imgpel) iClip1 ( img->max_imgpel_value, rshift_rnd_sf( is, 10 ) );
    }
  }

  // bottom
  for (jpad = ypadded_size - 3; jpad < ypadded_size; jpad++)
  {
    wxLineDst = dstImg[jpad];
    srcImgA = imgY_sub_tmp[jpad ];
    srcImgB = imgY_sub_tmp[jpad - 1];      
    srcImgC = imgY_sub_tmp[jpad - 2];
    srcImgD = imgY_sub_tmp[imin (maxy, jpad + 1)];
    srcImgE = imgY_sub_tmp[maxy];
    srcImgF = imgY_sub_tmp[maxy];
    for (ipad = 0; ipad < xpadded_size; ipad++)
    {
      is =
        (tap0 * (*srcImgA++ + *srcImgD++) +
        tap1 *  (*srcImgB++ + *srcImgE++) +
        tap2 *  (*srcImgC++ + *srcImgF++));

      wxLineDst[ipad] = (imgpel) iClip1 ( img->max_imgpel_value, rshift_rnd_sf( is, 10 ) );
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Does _horizontal_ interpolation using the BiLinear filter
 *
 * \param s
 *    pointer to StorablePicture structure
 * \param dstImg
 *    destination Image
 * \param srcImgL
 *    source left image
 * \param srcImgR
 *    source right image 
 ************************************************************************
 */
void getSubImageBiLinear( StorablePicture *s, imgpel **dstImg, imgpel **srcImgL, imgpel **srcImgR)
{
  int jpad, ipad;
  int ypadded_size = s->size_y_padded;
  int xpadded_size = s->size_x_padded;

  static imgpel *wBufSrcL, *wBufSrcR, *wBufDst;

  for (jpad = 0; jpad < ypadded_size; jpad++)
  {
    wBufSrcL = srcImgL[jpad]; // 4:4:4 independent mode
    wBufSrcR = srcImgR[jpad]; // 4:4:4 independent mode
    wBufDst  = dstImg[jpad];  // 4:4:4 independent mode

    for (ipad = 0; ipad < xpadded_size; ipad++)
    {
      *wBufDst++ = (imgpel) rshift_rnd_sf( *wBufSrcL++ + *wBufSrcR++, 1 );
    }
  }
}


/*!
 ************************************************************************
 * \brief
 *    Does _horizontal_ interpolation using the BiLinear filter
 *
 * \param s
 *    pointer to StorablePicture structure
 * \param dstImg
 *    destination Image
 * \param srcImgL
 *    source left image
 * \param srcImgR
 *    source right image 
 ************************************************************************
 */
void getHorSubImageBiLinear( StorablePicture *s, imgpel **dstImg, imgpel **srcImgL, imgpel **srcImgR)
{
  int jpad, ipad;
  int ypadded_size = s->size_y_padded;
  int xpadded_size = s->size_x_padded - 1;

  static imgpel *wBufSrcL, *wBufSrcR, *wBufDst;

  for (jpad = 0; jpad < ypadded_size; jpad++)
  {
    wBufSrcL = srcImgL[jpad]; // 4:4:4 independent mode
    wBufSrcR = &srcImgR[jpad][1]; // 4:4:4 independent mode
    wBufDst  = dstImg[jpad];     // 4:4:4 independent mode

    // left padded area + center
    for (ipad = 0; ipad < xpadded_size; ipad++)
    {
      *wBufDst++ = (imgpel) rshift_rnd_sf( *wBufSrcL++ + *wBufSrcR++, 1 );
    }
    // right padded area
      *wBufDst++ = (imgpel) rshift_rnd_sf( *wBufSrcL++ + wBufSrcR[-1], 1 );
  }
}


/*!
 ************************************************************************
 * \brief
 *    Does _vertical_ interpolation using the BiLinear filter
 *
 * \param s
 *    pointer to StorablePicture structure
 * \param dstImg
 *    destination Image
 * \param srcImgT
 *    source top image
 * \param srcImgB
 *    source bottom image 
 ************************************************************************
 */
void getVerSubImageBiLinear( StorablePicture *s, imgpel **dstImg, imgpel **srcImgT, imgpel **srcImgB)
{
  int jpad, ipad;
  int ypadded_size = s->size_y_padded - 1;
  int xpadded_size = s->size_x_padded;  

  static imgpel *wBufSrcT, *wBufSrcB, *wBufDst;

  // top
  for (jpad = 0; jpad < ypadded_size; jpad++)
  {
    wBufSrcT = srcImgT[jpad];           // 4:4:4 independent mode
    wBufDst  = dstImg[jpad];            // 4:4:4 independent mode
    wBufSrcB = srcImgB[jpad + 1];  // 4:4:4 independent mode

    for (ipad = 0; ipad < xpadded_size; ipad++)
    {
      *wBufDst++ = (imgpel) rshift_rnd_sf(*wBufSrcT++ + *wBufSrcB++, 1);
    }
  }
  // bottom
  wBufSrcT = srcImgT[ypadded_size];           // 4:4:4 independent mode
  wBufDst  = dstImg[ypadded_size];            // 4:4:4 independent mode
  wBufSrcB = srcImgB[ypadded_size];           // 4:4:4 independent mode

  for (ipad = 0; ipad < xpadded_size; ipad++)
  {
    *wBufDst++ = (imgpel) rshift_rnd_sf(*wBufSrcT++ + *wBufSrcB++, 1);
  }
}


/*!
 ************************************************************************
 * \brief
 *    Does _diagonal_ interpolation using the BiLinear filter
 *
 * \param s
 *    pointer to StorablePicture structure
 * \param dstImg
 *    destination Image
 * \param srcImgT
 *    source top/left image
 * \param srcImgB
 *    source bottom/right image 
 ************************************************************************
 */
void getDiagSubImageBiLinear( StorablePicture *s, imgpel **dstImg, imgpel **srcImgT, imgpel **srcImgB )
{
  int jpad, ipad;
  int maxx = s->size_x_padded - 1;
  int maxy = s->size_y_padded - 1;

  static imgpel *wBufSrcL, *wBufSrcR, *wBufDst;

  for (jpad = 0; jpad < maxy; jpad++)
  {
    wBufSrcL = srcImgT[jpad + 1]; // 4:4:4 independent mode
    wBufSrcR = &srcImgB[jpad][1]; // 4:4:4 independent mode
    wBufDst  = dstImg[jpad];      // 4:4:4 independent mode

    for (ipad = 0; ipad < maxx; ipad++)
    {
      *wBufDst++ = (imgpel) rshift_rnd_sf(*wBufSrcL++ + *wBufSrcR++, 1);
    }

    *wBufDst++ = (imgpel) rshift_rnd_sf(*wBufSrcL++ +  wBufSrcR[-1], 1);
  }

  wBufSrcL = srcImgT[maxy];     // 4:4:4 independent mode
  wBufSrcR = &srcImgB[maxy][1]; // 4:4:4 independent mode
  wBufDst = dstImg[maxy];       // 4:4:4 independent mode

  for (ipad = 0; ipad < maxx; ipad++)
  {
    *wBufDst++ = (imgpel) rshift_rnd_sf(*wBufSrcL++ + *wBufSrcR++, 1);
  }

    *wBufDst++ = (imgpel) rshift_rnd_sf(*wBufSrcL++ + wBufSrcR[-1], 1);
}


