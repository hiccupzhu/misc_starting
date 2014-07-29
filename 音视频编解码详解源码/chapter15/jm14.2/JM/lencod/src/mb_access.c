
/*!
 *************************************************************************************
 * \file mb_access.c
 *
 * \brief
 *    Functions for macroblock neighborhoods
 *
 *  \author
 *      Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Karsten Sühring          <suehring@hhi.de>
 *************************************************************************************
 */

#include "global.h"
#include "mb_access.h"

/*!
 ************************************************************************
 * \brief
 *    returns 1 if the macroblock at the given address is available
 ************************************************************************
 */
int mb_is_available(int mbAddr, Macroblock *currMB)
{
  if ((mbAddr < 0) || (mbAddr > ((int)img->PicSizeInMbs - 1)))
    return 0;

  // the following line checks both: slice number and if the mb has been decoded
  if (!img->DeblockCall)
  {
    if (img->mb_data[mbAddr].slice_nr != currMB->slice_nr)
      return 0;
  }

  return 1;
}


/*!
 ************************************************************************
 * \brief
 *    Checks the availability of neighboring macroblocks of
 *    the current macroblock for prediction and context determination;
 ************************************************************************
 */
void CheckAvailabilityOfNeighbors(Macroblock *currMB)
{
  const int mb_nr = currMB->mbAddrX;

  // mark all neighbors as unavailable
  currMB->mb_available_up   = NULL;
  currMB->mb_available_left = NULL;

  if (img->MbaffFrameFlag)
  {
    int cur_mb_pair = mb_nr >> 1;
    currMB->mbAddrA = 2 * (cur_mb_pair - 1);
    currMB->mbAddrB = 2 * (cur_mb_pair - img->PicWidthInMbs);
    currMB->mbAddrC = 2 * (cur_mb_pair - img->PicWidthInMbs + 1);
    currMB->mbAddrD = 2 * (cur_mb_pair - img->PicWidthInMbs - 1);

    currMB->mbAvailA = mb_is_available(currMB->mbAddrA, currMB) && ((PicPos[cur_mb_pair    ][0])!=0);
    currMB->mbAvailB = mb_is_available(currMB->mbAddrB, currMB);
    currMB->mbAvailC = mb_is_available(currMB->mbAddrC, currMB) && ((PicPos[cur_mb_pair + 1][0])!=0);
    currMB->mbAvailD = mb_is_available(currMB->mbAddrD, currMB) && ((PicPos[cur_mb_pair    ][0])!=0);
  }
  else
  {
    currMB->mbAddrA = mb_nr - 1;
    currMB->mbAddrB = mb_nr - img->PicWidthInMbs;
    currMB->mbAddrC = mb_nr - img->PicWidthInMbs + 1;
    currMB->mbAddrD = mb_nr - img->PicWidthInMbs - 1;

    currMB->mbAvailA = mb_is_available(currMB->mbAddrA, currMB) && ((PicPos[mb_nr    ][0])!=0);
    currMB->mbAvailB = mb_is_available(currMB->mbAddrB, currMB);
    currMB->mbAvailC = mb_is_available(currMB->mbAddrC, currMB) && ((PicPos[mb_nr + 1][0])!=0);
    currMB->mbAvailD = mb_is_available(currMB->mbAddrD, currMB) && ((PicPos[mb_nr    ][0])!=0);
  }

  if (currMB->mbAvailA) currMB->mb_available_left = &(img->mb_data[currMB->mbAddrA]);
  if (currMB->mbAvailB) currMB->mb_available_up   = &(img->mb_data[currMB->mbAddrB]);
}


/*!
 ************************************************************************
 * \brief
 *    returns the x and y macroblock coordinates for a given MbAddress
 ************************************************************************
 */
void get_mb_block_pos_normal (int mb_addr, int *x, int*y)
{
  *x = PicPos[ mb_addr ][0];
  *y = PicPos[ mb_addr ][1];
}

/*!
 ************************************************************************
 * \brief
 *    returns the x and y macroblock coordinates for a given MbAddress
 *    for mbaff type slices
 ************************************************************************
 */
void get_mb_block_pos_mbaff (int mb_addr, int *x, int*y)
{
  *x =  PicPos[mb_addr>>1][0];
  *y = (PicPos[mb_addr>>1][1] << 1) + (mb_addr & 0x01);
}

/*!
 ************************************************************************
 * \brief
 *    returns the x and y sample coordinates for a given MbAddress
 ************************************************************************
 */
void get_mb_pos (int mb_addr, int mb_size[2], int *x, int*y)
{
  get_mb_block_pos(mb_addr, x, y);

  (*x) *= mb_size[0];
  (*y) *= mb_size[1];
}


/*!
 ************************************************************************
 * \brief
 *    get neighbouring positions for non-aff coding
 * \param currMb
 *   current macroblock
 * \param xN
 *    input x position
 * \param yN
 *    input y position
 * \param mb_size
 *    Macroblock size in pixel (according to luma or chroma MB access)
 * \param pix
 *    returns position informations
 ************************************************************************
 */
void getNonAffNeighbour(Macroblock *currMb, int xN, int yN, int mb_size[2], PixelPos *pix)
{
  int maxW = mb_size[0], maxH = mb_size[1];
  static int *CurPos;

  if ((xN<0)&&(yN<0))
  {
    pix->mb_addr   = currMb->mbAddrD;
    pix->available = currMb->mbAvailD;
  }
  else if ((xN<0)&&((yN>=0)&&(yN<maxH)))
  {
    pix->mb_addr  = currMb->mbAddrA;
    pix->available = currMb->mbAvailA;
  }
  else if (((xN>=0)&&(xN<maxW))&&(yN<0))
  {
    pix->mb_addr  = currMb->mbAddrB;
    pix->available = currMb->mbAvailB;
  }
  else if (((xN>=0)&&(xN<maxW))&&((yN>=0)&&(yN<maxH)))
  {
    pix->mb_addr  = currMb->mbAddrX;
    pix->available = TRUE;
  }
  else if ((xN>=maxW)&&(yN<0))
  {
    pix->mb_addr  = currMb->mbAddrC;
    pix->available = currMb->mbAvailC;
  }
  else
  {
    pix->available = FALSE;
  }

  if (pix->available || img->DeblockCall)
  {
    pix->x = xN & (maxW - 1);
    pix->y = yN & (maxH - 1);
    CurPos = PicPos[ pix->mb_addr ];
    pix->pos_x = pix->x + CurPos[0] * maxW;
    pix->pos_y = pix->y + CurPos[1] * maxH;
  }
}

/*!
 ************************************************************************
 * \brief
 *    get neighboring positions for aff coding
 * \param currMb
 *   current macroblock
 * \param xN
 *    input x position
 * \param yN
 *    input y position
 * \param mb_size
 *    Macroblock size in pixel (according to luma or chroma MB access)
 * \param pix
 *    returns position informations
 ************************************************************************
 */
void getAffNeighbour(Macroblock *currMb, int xN, int yN, int mb_size[2], PixelPos *pix)
{
  int maxW, maxH;
  int yM = -1;

  maxW = mb_size[0];
  maxH = mb_size[1];

  // initialize to "not available"
  pix->available = FALSE;

  if(yN > (maxH - 1))
  {
    return;
  }
  if (xN > (maxW - 1) && yN >= 0 && yN < maxH)
  {
    return;
  }

  if (xN < 0)
  {
    if (yN < 0)
    {
      if(!currMb->mb_field)
      {
        // frame
        if ((currMb->mbAddrX & 0x01) == 0)
        {
          // top
          pix->mb_addr   = currMb->mbAddrD  + 1;
          pix->available = currMb->mbAvailD;
          yM = yN;
        }
        else
        {
          // bottom
          pix->mb_addr   = currMb->mbAddrA;
          pix->available = currMb->mbAvailA;
          if (currMb->mbAvailA)
          {
            if(!img->mb_data[currMb->mbAddrA].mb_field)
            {
               yM = yN;
            }
            else
            {
              (pix->mb_addr)++;
               yM = (yN + maxH) >> 1;
            }
          }
        }
      }
      else
      {
        // field
        if ((currMb->mbAddrX & 0x01) == 0)
        {
          // top
          pix->mb_addr   = currMb->mbAddrD;
          pix->available = currMb->mbAvailD;
          if (currMb->mbAvailD)
          {
            if(!img->mb_data[currMb->mbAddrD].mb_field)
            {
              (pix->mb_addr)++;
               yM = 2 * yN;
            }
            else
            {
               yM = yN;
            }
          }
        }
        else
        {
          // bottom
          pix->mb_addr   = currMb->mbAddrD+1;
          pix->available = currMb->mbAvailD;
          yM = yN;
        }
      }
    }
    else
    { // xN < 0 && yN >= 0
      if (yN >= 0 && yN <maxH)
      {
        if (!currMb->mb_field)
        {
          // frame
          if ((currMb->mbAddrX & 0x01) == 0)
          {
            // top
            pix->mb_addr   = currMb->mbAddrA;
            pix->available = currMb->mbAvailA;
            if (currMb->mbAvailA)
            {
              if(!img->mb_data[currMb->mbAddrA].mb_field)
              {
                 yM = yN;
              }
              else
              {
                (pix->mb_addr)+= ((yN & 0x01) != 0);
                yM = yN >> 1;
              }
            }
          }
          else
          {
            // bottom
            pix->mb_addr   = currMb->mbAddrA;
            pix->available = currMb->mbAvailA;
            if (currMb->mbAvailA)
            {
              if(!img->mb_data[currMb->mbAddrA].mb_field)
              {
                (pix->mb_addr)++;
                 yM = yN;
              }
              else
              {
                (pix->mb_addr)+= ((yN & 0x01) != 0);
                yM = (yN + maxH) >> 1;
              }
            }
          }
        }
        else
        {
          // field
          if ((currMb->mbAddrX & 0x01) == 0)
          {
            // top
            pix->mb_addr  = currMb->mbAddrA;
            pix->available = currMb->mbAvailA;
            if (currMb->mbAvailA)
            {
              if(!img->mb_data[currMb->mbAddrA].mb_field)
              {
                if (yN < (maxH >> 1))
                {
                   yM = yN << 1;
                }
                else
                {
                  (pix->mb_addr)++;
                   yM = (yN << 1 ) - maxH;
                }
              }
              else
              {
                 yM = yN;
              }
            }
          }
          else
          {
            // bottom
            pix->mb_addr  = currMb->mbAddrA;
            pix->available = currMb->mbAvailA;
            if (currMb->mbAvailA)
            {
              if(!img->mb_data[currMb->mbAddrA].mb_field)
              {
                if (yN < (maxH >> 1))
                {
                  yM = (yN << 1) + 1;
                }
                else
                {
                  (pix->mb_addr)++;
                   yM = (yN << 1 ) + 1 - maxH;
                }
              }
              else
              {
                (pix->mb_addr)++;
                 yM = yN;
              }
            }
          }
        }
      }
    }
  }
  else
  { // xN >= 0
    if (xN >= 0 && xN < maxW)
    {
      if (yN<0)
      {
        if (!currMb->mb_field)
        {
          //frame
          if ((currMb->mbAddrX & 0x01) == 0)
          {
            //top
            pix->mb_addr  = currMb->mbAddrB;
            // for the deblocker if the current MB is a frame and the one above is a field
            // then the neighbor is the top MB of the pair
            if (currMb->mbAvailB)
            {
              if (!(img->DeblockCall == 1 && (img->mb_data[currMb->mbAddrB]).mb_field))
                pix->mb_addr  += 1;
            }

            pix->available = currMb->mbAvailB;
            yM = yN;
          }
          else
          {
            // bottom
            pix->mb_addr   = currMb->mbAddrX - 1;
            pix->available = TRUE;
            yM = yN;
          }
        }
        else
        {
          // field
          if ((currMb->mbAddrX & 0x01) == 0)
          {
            // top
            pix->mb_addr   = currMb->mbAddrB;
            pix->available = currMb->mbAvailB;
            if (currMb->mbAvailB)
            {
              if(!img->mb_data[currMb->mbAddrB].mb_field)
              {
                (pix->mb_addr)++;
                 yM = 2* yN;
              }
              else
              {
                 yM = yN;
              }
            }
          }
          else
          {
            // bottom
            pix->mb_addr   = currMb->mbAddrB + 1;
            pix->available = currMb->mbAvailB;
            yM = yN;
          }
        }
      }
      else
      {
        // yN >=0
        // for the deblocker if this is the extra edge then do this special stuff
        if (yN == 0 && img->DeblockCall == 2)
        {
          pix->mb_addr  = currMb->mbAddrB + 1;
          pix->available = TRUE;
          yM = yN - 1;
        }

        else if ((yN >= 0) && (yN <maxH))
        {
          pix->mb_addr   = currMb->mbAddrX;
          pix->available = TRUE;
          yM = yN;
        }
      }
    }
    else
    { // xN >= maxW
      if(yN < 0)
      {
        if (!currMb->mb_field)
        {
          // frame
          if ((currMb->mbAddrX & 0x01) == 0)
          {
            // top
            pix->mb_addr  = currMb->mbAddrC + 1;
            pix->available = currMb->mbAvailC;
            yM = yN;
          }
          else
          {
            // bottom
            pix->available = FALSE;
          }
        }
        else
        {
          // field
          if ((currMb->mbAddrX & 0x01) == 0)
          {
            // top
            pix->mb_addr   = currMb->mbAddrC;
            pix->available = currMb->mbAvailC;
            if (currMb->mbAvailC)
            {
              if(!img->mb_data[currMb->mbAddrC].mb_field)
              {
                (pix->mb_addr)++;
                 yM = 2* yN;
              }
              else
              {
                yM = yN;
              }
            }
          }
          else
          {
            // bottom
            pix->mb_addr   = currMb->mbAddrC + 1;
            pix->available = currMb->mbAvailC;
            yM = yN;
          }
        }
      }
    }
  }
  if (pix->available || img->DeblockCall)
  {
    pix->x = xN & (maxW - 1);
    pix->y = yM & (maxH - 1);
    get_mb_pos(pix->mb_addr, mb_size, &(pix->pos_x), &(pix->pos_y));
    pix->pos_x += pix->x;
    pix->pos_y += pix->y;
  }
}


/*!
 ************************************************************************
 * \brief
 *    get neighboring 4x4 chroma block
 * \param currMb
 *   current macroblock
 * \param block_x
 *    input x block position
 * \param block_y
 *    input y block position
 * \param mb_size
 *    Macroblock size in pixel (according to luma or chroma MB access)
 * \param pix
 *    returns position informations
 ************************************************************************
 */
void get4x4Neighbour (Macroblock *currMb, int block_x, int block_y, int mb_size[2], PixelPos *pix)
{
  getNeighbour(currMb, block_x, block_y, mb_size, pix);

  if (pix->available)
  {
    pix->x >>= 2;
    pix->y >>= 2;
    pix->pos_x >>= 2;
    pix->pos_y >>= 2;
  }
}
