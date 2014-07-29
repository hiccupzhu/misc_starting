
/*!
 ************************************************************************
 * \file frame.h
 *
 * \brief
 *    headers for frame format related information
 *
 * \author
 *
 ************************************************************************
 */
#ifndef _FRAME_H_
#define _FRAME_H_

typedef struct
{
  int yuv_format;                    //!< YUV format (0=4:0:0, 1=4:2:0, 2=4:2:2, 3=4:4:4)
  int rgb_format;                    //!< 4:4:4 format (0: YUV, 1: RGB)
  int width;                         //!< luma component frame width
  int height;                        //!< luma component frame height    
  int height_cr;                     //!< chroma component frame width
  int width_cr;                      //!< chroma component frame height
  int mb_width;                      //!< luma component frame width
  int mb_height;                     //!< luma component frame height    
  int size_cmp[3];                   //!< component sizes  
  int size;                          //!< total image size
  int bit_depth[3];                  //!< component bit depth  
  int max_value[3];                  //!< component max value
  int max_value_sq[3];                  //!< component max value squared
} FrameFormat;

#endif

