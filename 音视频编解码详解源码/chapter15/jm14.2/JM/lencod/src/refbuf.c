
/*!
 ************************************************************************
 * \file refbuf.c
 *
 * \brief
 *    Declarations of the reference frame buffer types and functions
 ************************************************************************
 */

#include "global.h"
#include "refbuf.h"


/*!
 ************************************************************************
 * \brief
 *    Yields a pel line _pointer_ from one of the 16 sub-images
 *    Input does not require subpixel image indices
 ************************************************************************
 */
imgpel *FastLine4X (imgpel ****Pic, int y, int x)
{
  return &(Pic[(y & 0x03)][(x & 0x03)][y >> 2][x >> 2]);
}

/*!
 ************************************************************************
 * \brief
 *    Yields a pel line _pointer_ from one of the 16 sub-images
 *    Input does not require subpixel image indices
 ************************************************************************
 */
imgpel *UMVLine4X (imgpel ****Pic, int y, int x)
{
  return &(Pic[(y & 0x03)][(x & 0x03)][iClip3( 0, height_pad, y >> 2)][iClip3( 0, width_pad , x >> 2)]);
}

/*!
 ************************************************************************
 * \brief
 *    Yields a pel line _pointer_ from one of the 16 (4:4:4), 32 (4:2:2),
 *    or 64 (4:2:0) sub-images
 *    Input does not require subpixel image indices
 ************************************************************************
 */
imgpel *UMVLine8X_chroma (imgpel ****Pic, int y, int x)
{
  return &(Pic[y & chroma_mask_mv_y][x & chroma_mask_mv_x][iClip3 (0, height_pad_cr, y >> chroma_shift_y)][iClip3 (0, width_pad_cr , x >> chroma_shift_x)]);
}

/*!
 ************************************************************************
 * \brief
 *    Yields a pel line _pointer_ from one of the 16 (4:4:4), 32 (4:2:2),
 *    or 64 (4:2:0) sub-images
 *    Input does not require subpixel image indices
 ************************************************************************
 */
imgpel *FastLine8X_chroma (imgpel ****Pic, int y, int x)
{
  return &(Pic[y & chroma_mask_mv_y][x & chroma_mask_mv_x][y >> chroma_shift_y][x >> chroma_shift_x]);
}



