/*!
 ***************************************************************************
 * \file
 *    img_luma.h
 *
 * \author
 *    Athanasios Leontaris           <aleon@dolby.com>
 *    Alexis Michael Tourapis        <alexis.tourapis@dolby.com>
 *
 * \date
 *    4. October 2006
 *
 * \brief
 *    Headerfile for luma interpolation functions
 **************************************************************************
 */

#ifndef _IMG_LUMA_H_
#define _IMG_LUMA_H_

void getSubImagesLuma       ( StorablePicture *s );
void getSubImageInteger     ( StorablePicture *s, imgpel **dstImg, imgpel **srcImg);
void getHorSubImageSixTap   ( StorablePicture *s, imgpel **dst_imgY, imgpel **ref_imgY);
void getVerSubImageSixTap   ( StorablePicture *s, imgpel **dst_imgY, imgpel **ref_imgY);
void getVerSubImageSixTapTmp( StorablePicture *s, imgpel **dst_imgY, imgpel **ref_imgY);
void getSubImageBiLinear    ( StorablePicture *s, imgpel **dstImg, imgpel **srcImgL, imgpel **srcImgR);
void getHorSubImageBiLinear ( StorablePicture *s, imgpel **dstImg, imgpel **srcImgL, imgpel **srcImgR);
void getVerSubImageBiLinear ( StorablePicture *s, imgpel **dstImg, imgpel **srcImgT, imgpel **srcImgB);
void getDiagSubImageBiLinear( StorablePicture *s, imgpel **dstImg, imgpel **srcImgT, imgpel **srcImgB);
#endif // _IMG_LUMA_H_
