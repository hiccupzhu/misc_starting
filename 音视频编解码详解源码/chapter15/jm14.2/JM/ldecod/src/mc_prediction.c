
/*!
 *************************************************************************************
 * \file mc_prediction.c
 *
 * \brief
 *    Functions for motion compensated prediction
 *
 * \author
 *      Main contributors (see contributors.h for copyright, 
 *                         address and affiliation details)
 *      - Alexis Michael Tourapis  <alexismt@ieee.org>
 *
 *************************************************************************************
 */
#include "global.h"
#include "block.h"
#include "mc_prediction.h"
#include "mbuffer.h"
#include "mb_access.h"
#include "macroblock.h"

extern StorablePicture *no_reference_picture;

imgpel tmp_block_l0[MB_BLOCK_SIZE][MB_BLOCK_SIZE];     //!< l0 prediction
imgpel tmp_block_l1[MB_BLOCK_SIZE][MB_BLOCK_SIZE];     //!< l1 prediction

static const int COEF[6] = { 1, -5, 20, 20, -5, 1 };
/*!
 ************************************************************************
 * \brief
 *    block single list prediction
 ************************************************************************
 */
static inline void mc_prediction(imgpel mb_pred[MB_BLOCK_SIZE][MB_BLOCK_SIZE],
                    int ver_block_size, 
                    int hor_block_size,
                    int ioff,
                    imgpel block[MB_BLOCK_SIZE][MB_BLOCK_SIZE])
{
  static int jj;

  if (hor_block_size == MB_BLOCK_SIZE)
  {
    memcpy(&(mb_pred[0][ioff]), &(block[0][0]), hor_block_size * ver_block_size * sizeof(imgpel));
  }
  else
  {
    for(jj = 0; jj < ver_block_size; jj++)
    {
      memcpy(&(mb_pred[jj][ioff]), &(block[jj][0]), hor_block_size * sizeof(imgpel));
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    block single list weighted prediction
 ************************************************************************
 */
static inline void weighted_mc_prediction(imgpel mb_pred[MB_BLOCK_SIZE][MB_BLOCK_SIZE],
                            int ver_block_size, 
                            int hor_block_size,
                            int ioff,
                            imgpel block[MB_BLOCK_SIZE][MB_BLOCK_SIZE], 
                            int wp_scale,
                            int wp_offset,
                            int weight_denom,
                            int color_clip)
{
  static int ii, jj;
  static imgpel *mpr, *b0;
  
  for(jj=0;jj<ver_block_size;jj++)
  {
    mpr = &mb_pred[jj][ioff];
    b0 = block[jj];
    for(ii=0;ii<hor_block_size;ii++)
      *(mpr++) = (imgpel) iClip1(color_clip, (rshift_rnd((wp_scale * *(b0++)), weight_denom)  + wp_offset ));
  }
}


/*!
 ************************************************************************
 * \brief
 *    block biprediction
 ************************************************************************
 */
static inline void bi_prediction(imgpel mb_pred[MB_BLOCK_SIZE][MB_BLOCK_SIZE],  
                                 imgpel block_l0[MB_BLOCK_SIZE][MB_BLOCK_SIZE], 
                                 imgpel block_l1[MB_BLOCK_SIZE][MB_BLOCK_SIZE],
                                 int ver_block_size, 
                                 int hor_block_size,
                                 int ioff)
{
  static int ii, jj;
  static imgpel *mpr, *b0, *b1;

  for(jj = 0;jj < ver_block_size;jj++)
  {
    mpr = &mb_pred[jj][ioff];
    b0 = block_l0[jj];
    b1 = block_l1[jj];
    for(ii = 0; ii < hor_block_size;ii++)
      *(mpr++) = (imgpel) rshift_rnd_sf((*(b0++) + *(b1++)), 1);
  }
}

/*!
 ************************************************************************
 * \brief
 *    block weighted biprediction
 ************************************************************************
 */
static inline void weighted_bi_prediction(imgpel mb_pred[MB_BLOCK_SIZE][MB_BLOCK_SIZE], 
                                          imgpel block_l0[MB_BLOCK_SIZE][MB_BLOCK_SIZE], 
                                          imgpel block_l1[MB_BLOCK_SIZE][MB_BLOCK_SIZE],
                                          int ver_block_size, 
                                          int hor_block_size,
                                          int ioff,
                                          int wp_scale_l0,
                                          int wp_scale_l1,
                                          int wp_offset,
                                          int weight_denom,
                                          int color_clip)
{
  static int ii, jj;
  static imgpel *mpr, *b0, *b1;
  
  for(jj = 0; jj < ver_block_size; jj++)
  {
    mpr = &mb_pred[jj][ioff];    
    b0  = block_l0[jj];
    b1  = block_l1[jj];

    for(ii=0;ii<hor_block_size;ii++)
      *(mpr++) = (imgpel) iClip1(color_clip, (rshift_rnd((wp_scale_l0 * *(b0++) + wp_scale_l1 * *(b1++)), weight_denom) + wp_offset));
  }
}

/*!
 ************************************************************************
 * \brief
 *    Interpolation of 1/4 subpixel
 ************************************************************************
 */ 
void get_block_luma(ColorPlane pl, StorablePicture *curr_ref, int x_pos, int y_pos, int hor_block_size, int ver_block_size, ImageParameters *img, imgpel block[MB_BLOCK_SIZE][MB_BLOCK_SIZE])
{
  static int tmp_res[21][21];
  static int *tmp_line;
  static imgpel *p0, *p1, *p2, *p3, *p4, *p5;
  static int    *x0, *x1, *x2, *x3, *x4, *x5;  
  
  static imgpel **cur_imgY, *cur_lineY;
  static int ipos_m2, ipos_m1, ipos, ipos_p1, ipos_p2, ipos_p3;
  static imgpel *orig_line;
  int tmp_pos;

  int dx = (x_pos & 3), dy = (y_pos & 3);
  int i, j, jj;
  int shift_x  = dec_picture->size_x;
  int maxold_x = dec_picture->size_x_m1;
  int maxold_y = (dec_picture->motion.mb_field[img->current_mb_nr]) ? (dec_picture->size_y >> 1) - 1 : dec_picture->size_y_m1;
  int result;
  int pres_x;
  int max_imgpel_value = img->max_imgpel_value_comp[pl];


  if (curr_ref == no_reference_picture && img->framepoc < img->recovery_poc)
  {
    printf("list[ref_frame] is equal to 'no reference picture' before RAP\n");

    /* fill the block with sample value 128 */
    for (j = 0; j < ver_block_size; j++)
      for (i = 0; i < hor_block_size; i++)
        block[j][i] = 128;
    return;
  }

  if( IS_INDEPENDENT(img) )
  {
    switch( img->colour_plane_id )
    {
    case    0:
      cur_imgY = curr_ref->imgY;
      break;
    case    1:
      cur_imgY = curr_ref->imgUV[0];
      break;
    case    2:
      cur_imgY = curr_ref->imgUV[1];
      break;
    }
  }
  else if (pl==PLANE_Y)
  {
    cur_imgY = curr_ref->imgY;
  }
  else
  {
    cur_imgY = curr_ref->imgUV[pl-1]; 
  }

  x_pos = x_pos >> 2;
  y_pos = y_pos >> 2;

  if ( (y_pos > 1) && (y_pos < maxold_y - 2 - ver_block_size) && (x_pos > 1) && (x_pos < maxold_x - 2 - hor_block_size))
  {
    cur_imgY = &cur_imgY[ y_pos];
    if (dx == 0 && dy == 0)
    {  /* fullpel position */
      for (j = 0; j < ver_block_size; j++)
      {        
        memcpy(&(block[j][0]), &(cur_imgY[j][x_pos]), hor_block_size * sizeof(imgpel));
      }
    }
    else
    { /* other positions */

      if (dy == 0)
      { /* No vertical interpolation */
        for (j = 0; j < ver_block_size; j++)
        {
          p0 = &cur_imgY[j][x_pos - 2];
          p1 = p0 + 1;
          p2 = p1 + 1;
          p3 = p2 + 1;
          p4 = p3 + 1;
          p5 = p4 + 1;
          orig_line = block[j];

          for (i = 0; i < hor_block_size; i++)
          {        
            result  = (*(p0++) + *(p5++)) * COEF[0]
                    + (*(p1++) + *(p4++)) * COEF[1]
                    + (*(p2++) + *(p3++)) * COEF[2];

            *orig_line++ = (imgpel) iClip1(max_imgpel_value, ((result + 16)>>5));
          }
        }

        if ((dx&1) == 1)
        {          
          for (j = 0; j < ver_block_size; j++)
          {
            cur_lineY = &(cur_imgY[j][x_pos + (dx >> 1)]);
            orig_line = block[j];
            for (i = 0; i < hor_block_size; i++)
            {
              *orig_line = (imgpel) ((*orig_line + *(cur_lineY++) + 1 ) >> 1);
              orig_line++;
            }
          }
        }
      }
      else if (dx == 0)
      {  /* No horizontal interpolation */        
        p0 = &(cur_imgY[ - 2][x_pos]);
        for (j = 0; j < ver_block_size; j++)
        {                  
          p1 = p0 + shift_x;          
          p2 = p1 + shift_x;
          p3 = p2 + shift_x;
          p4 = p3 + shift_x;
          p5 = p4 + shift_x;
          orig_line = block[j];

          for (i = 0; i < hor_block_size; i++)
          {
            result  = (*(p0++) + *(p5++)) * COEF[0]
                    + (*(p1++) + *(p4++)) * COEF[1]
                    + (*(p2++) + *(p3++)) * COEF[2];

            *orig_line++ = (imgpel) iClip1(max_imgpel_value, ((result + 16)>>5));
          }
          p0 = p1 - hor_block_size;
        }

        if ((dy&1) == 1)
        {
          jj = (dy >> 1);
          for (j = 0; j < ver_block_size; j++)
          {
            cur_lineY = &(cur_imgY[jj++][x_pos]);
            orig_line = block[j];
            for (i = 0; i < hor_block_size; i++)
            {
              *orig_line = (imgpel) ((*orig_line + *(cur_lineY++) + 1 ) >> 1);
              orig_line++;
            }
          }
        }
      }
      else if (dx == 2)
      {  /* Vertical & horizontal interpolation */
        jj = - 2;
        for (j = 0; j < ver_block_size + 5; j++)
        {
          p0 = &cur_imgY[jj++][x_pos - 2];
          p1 = p0 + 1;
          p2 = p1 + 1;
          p3 = p2 + 1;
          p4 = p3 + 1;
          p5 = p4 + 1;
          orig_line = block[j];
          tmp_line  = tmp_res[j];

          for (i = 0; i < hor_block_size; i++)
          {        
            *(tmp_line++) = (*(p0++) + *(p5++)) * COEF[0]
                          + (*(p1++) + *(p4++)) * COEF[1]
                          + (*(p2++) + *(p3++)) * COEF[2];
          }
        }

        for (j = 0; j < ver_block_size; j++)
        {
          x0 = tmp_res[j    ];
          x1 = tmp_res[j + 1];
          x2 = tmp_res[j + 2];
          x3 = tmp_res[j + 3];
          x4 = tmp_res[j + 4];
          x5 = tmp_res[j + 5];
          orig_line = block[j];

          for (i = 0; i < hor_block_size; i++)
          {
            result  = (*x0++ + *x5++) * COEF[0]
                    + (*x1++ + *x4++) * COEF[1]
                    + (*x2++ + *x3++) * COEF[2];

            *(orig_line++) = (imgpel) iClip1(max_imgpel_value, ((result+512)>>10));
          }
        }

        if ((dy&1) == 1)
        {
          jj = 2 + (dy>>1);
          for (j = 0; j < ver_block_size; j++)
          {            
            tmp_line  = tmp_res[jj++];
            orig_line = block[j];
            for (i = 0; i < hor_block_size; i++)
            {
              *orig_line = (imgpel) ((*orig_line + iClip1(max_imgpel_value, ((*(tmp_line++) + 16) >> 5)) + 1 )>> 1);
              orig_line++;
            }
          }
        }
      }
      else if (dy == 2)
      {  /* Horizontal & vertical interpolation */
        p0 = &(cur_imgY[ -2][x_pos - 2]);
        for (j = 0; j < ver_block_size; j++)
        {                    
          p1 = p0 + shift_x;
          p2 = p1 + shift_x;
          p3 = p2 + shift_x;
          p4 = p3 + shift_x;
          p5 = p4 + shift_x;
          tmp_line  = tmp_res[j];

          for (i = 0; i < hor_block_size + 5; i++)
          {
            *(tmp_line++)  = (*(p0++) + *(p5++)) * COEF[0]
                           + (*(p1++) + *(p4++)) * COEF[1]
                           + (*(p2++) + *(p3++)) * COEF[2];
          }
          p0 = p1 - (hor_block_size + 5);
        }

        for (j = 0; j < ver_block_size; j++)
        {
          orig_line = block[j];
          x0 = tmp_res[j];
          x1 = x0 + 1;
          x2 = x1 + 1;
          x3 = x2 + 1;
          x4 = x3 + 1;
          x5 = x4 + 1;

          for (i = 0; i < hor_block_size; i++)
          {
            result  = (*(x0++) + *(x5++)) * COEF[0]
                    + (*(x1++) + *(x4++)) * COEF[1]
                    + (*(x2++) + *(x3++)) * COEF[2];

            *(orig_line++) = (imgpel) iClip1(max_imgpel_value, ((result + 512)>>10));
          }
        }

        if ((dx&1) == 1)
        {
          for (j = 0; j < ver_block_size; j++)
          {
            tmp_line  = &tmp_res[j][2 + (dx>>1)];
            orig_line = block[j];
            for (i = 0; i < hor_block_size; i++)
            {
              *orig_line = (imgpel) ((*orig_line + iClip1(max_imgpel_value, ((*(tmp_line++) + 16)>>5))+1)>>1);
              orig_line ++;
            }
          }
        }
      }
      else
      {  /* Diagonal interpolation */
        jj = (dy == 1 ? 0 : 1);

        for (j = 0; j < ver_block_size; j++)
        {
          p0 = &cur_imgY[jj++][x_pos - 2];
          p1 = p0 + 1;
          p2 = p1 + 1;
          p3 = p2 + 1;
          p4 = p3 + 1;
          p5 = p4 + 1;

          orig_line = block[j];

          for (i = 0; i < hor_block_size; i++)
          {        
            result  = (*(p0++) + *(p5++)) * COEF[0]
                    + (*(p1++) + *(p4++)) * COEF[1]
                    + (*(p2++) + *(p3++)) * COEF[2];

            *(orig_line++) = (imgpel) iClip1(max_imgpel_value, ((result + 16)>>5));
          }
        }

        p0 = &(cur_imgY[-2][(dx == 1 ? x_pos : x_pos + 1)]);
        for (j = 0; j < ver_block_size; j++)
        {        
          p1 = p0 + shift_x;
          p2 = p1 + shift_x;
          p3 = p2 + shift_x;
          p4 = p3 + shift_x;
          p5 = p4 + shift_x;
          orig_line = block[j];

          for (i = 0; i < hor_block_size; i++)
          {
            result  = (*(p0++) + *(p5++)) * COEF[0]
                    + (*(p1++) + *(p4++)) * COEF[1]
                    + (*(p2++) + *(p3++)) * COEF[2];

            *orig_line = (imgpel) ((*orig_line + iClip1(max_imgpel_value, ((result + 16) >> 5)) + 1) >> 1);
            orig_line++;
          }
          p0 = p1 - hor_block_size ;
        }      
      }
    }
  }
  else // unsafe positions
  {
    if (dx == 0 && dy == 0)
    {  /* fullpel position */
      for (j = 0; j < ver_block_size; j++)
      {
        cur_lineY = cur_imgY[iClip3(0, maxold_y, y_pos + j)];
        orig_line = block[j];
        for (i = 0; i < hor_block_size; i++)
        {
          *(orig_line++) = cur_lineY[iClip3(0, maxold_x, x_pos + i )];
        }
      }
    }
    else
    { /* other positions */

      if (dy == 0)
      { /* No vertical interpolation */
        tmp_pos = x_pos - 2;
        for (i = 0; i < hor_block_size; i++)
        {        
          ipos_m2 = iClip3(0, maxold_x, tmp_pos++);
          ipos_m1 = iClip3(0, maxold_x, tmp_pos++);
          ipos    = iClip3(0, maxold_x, tmp_pos++);
          ipos_p1 = iClip3(0, maxold_x, tmp_pos++);
          ipos_p2 = iClip3(0, maxold_x, tmp_pos++);
          ipos_p3 = iClip3(0, maxold_x, tmp_pos  );
          tmp_pos -= 4;          

          for (j = 0; j < ver_block_size; j++)
          {
            cur_lineY = cur_imgY[iClip3(0,maxold_y,y_pos+j)];

            result  = (cur_lineY[ipos_m2] + cur_lineY[ipos_p3]) * COEF[0];
            result += (cur_lineY[ipos_m1] + cur_lineY[ipos_p2]) * COEF[1];
            result += (cur_lineY[ipos   ] + cur_lineY[ipos_p1]) * COEF[2];

            block[j][i] = (imgpel) iClip1(max_imgpel_value, ((result + 16)>>5));
          }
        }

        if ((dx&1) == 1)
        {
          for (j = 0; j < ver_block_size; j++)
          {
            cur_lineY = cur_imgY[iClip3(0,maxold_y,y_pos+j)];
            orig_line = block[j];
            for (i = 0; i < hor_block_size; i++)
            {
              *orig_line = (imgpel) ((*orig_line + cur_lineY[iClip3(0, maxold_x, x_pos + i + (dx >> 1))] + 1 ) >> 1);
              orig_line++;
            }
          }
        }
      }
      else if (dx == 0)
      {  /* No horizontal interpolation */
        tmp_pos = y_pos - 2;
        for (j = 0; j < ver_block_size; j++)
        { 
          p0 = cur_imgY[iClip3(0, maxold_y, tmp_pos++)];
          p1 = cur_imgY[iClip3(0, maxold_y, tmp_pos++)];
          p2 = cur_imgY[iClip3(0, maxold_y, tmp_pos++)];
          p3 = cur_imgY[iClip3(0, maxold_y, tmp_pos++)];
          p4 = cur_imgY[iClip3(0, maxold_y, tmp_pos++)];
          p5 = cur_imgY[iClip3(0, maxold_y, tmp_pos  )];
          
          tmp_pos -= 4;
          orig_line = block[j];

          for (i = 0; i < hor_block_size; i++)
          {
            pres_x = iClip3(0,maxold_x,x_pos+i);

            result  = (p0[pres_x] + p5[pres_x]) * COEF[0];
            result += (p1[pres_x] + p4[pres_x]) * COEF[1];
            result += (p2[pres_x] + p3[pres_x]) * COEF[2];
            *(orig_line++) = (imgpel) iClip1(max_imgpel_value, ((result+16)>>5));
          }
        }

        if ((dy&1) == 1)
        {
          for (j = 0; j < ver_block_size; j++)
          {
            cur_lineY = cur_imgY[iClip3(0,maxold_y,y_pos+j+(dy>>1))];
            orig_line = block[j];
            for (i = 0; i < hor_block_size; i++)
            {
              *orig_line = (imgpel) ((*orig_line + cur_lineY[iClip3(0, maxold_x, x_pos + i)] + 1 ) >> 1);
              orig_line++;
            }
          }
        }
      }
      else if (dx == 2)
      {  /* Vertical & horizontal interpolation */
        tmp_pos = x_pos - 2;
        for (i = 0; i < hor_block_size; i++)
        {        
          ipos_m2 = iClip3(0, maxold_x, tmp_pos++);
          ipos_m1 = iClip3(0, maxold_x, tmp_pos++);
          ipos    = iClip3(0, maxold_x, tmp_pos++);
          ipos_p1 = iClip3(0, maxold_x, tmp_pos++);
          ipos_p2 = iClip3(0, maxold_x, tmp_pos++);
          ipos_p3 = iClip3(0, maxold_x, tmp_pos  );
          tmp_pos -= 4;

          for (j = 0; j < ver_block_size + 5; j++)
          {
            cur_lineY = cur_imgY[iClip3(0,maxold_y,y_pos + j - 2)];

            tmp_res[j][i]  = (cur_lineY[ipos_m2] + cur_lineY[ipos_p3]) * COEF[0];
            tmp_res[j][i] += (cur_lineY[ipos_m1] + cur_lineY[ipos_p2]) * COEF[1];
            tmp_res[j][i] += (cur_lineY[ipos   ] + cur_lineY[ipos_p1]) * COEF[2];
          }
        }

        for (j = 0; j < ver_block_size; j++)
        {
          x0 = tmp_res[j    ];
          x1 = tmp_res[j + 1];
          x2 = tmp_res[j + 2];
          x3 = tmp_res[j + 3];
          x4 = tmp_res[j + 4];
          x5 = tmp_res[j + 5];
          orig_line = block[j];

          for (i = 0; i < hor_block_size; i++)
          {
            result  = (*x0++ + *x5++) * COEF[0]
                    + (*x1++ + *x4++) * COEF[1]
                    + (*x2++ + *x3++) * COEF[2];

            *(orig_line++) = (imgpel) iClip1(max_imgpel_value, ((result+512)>>10));
          }
        }

        if ((dy&1) == 1)
        {
          for (j = 0; j < ver_block_size; j++)
          {
            tmp_line  = tmp_res[j + 2 + (dy>>1)];            
            orig_line = block[j];
            for (i = 0; i < hor_block_size; i++)
            {
              *orig_line = (imgpel) ((*orig_line + iClip1(max_imgpel_value, ((*(tmp_line++) + 16) >> 5)) + 1 )>>1);
              orig_line++;
            }
          }
        }
      }
      else if (dy == 2)
      {  /* Horizontal & vertical interpolation */

        tmp_pos = y_pos - 2;
        for (j = 0; j < ver_block_size; j++)
        {
          p0 = cur_imgY[iClip3(0, maxold_y, tmp_pos++)];
          p1 = cur_imgY[iClip3(0, maxold_y, tmp_pos++)];
          p2 = cur_imgY[iClip3(0, maxold_y, tmp_pos++)];
          p3 = cur_imgY[iClip3(0, maxold_y, tmp_pos++)];
          p4 = cur_imgY[iClip3(0, maxold_y, tmp_pos++)];
          p5 = cur_imgY[iClip3(0, maxold_y, tmp_pos  )];

          tmp_pos -= 4;

          for (i = 0; i < hor_block_size + 5; i++)
          {
            pres_x = iClip3(0,maxold_x, x_pos + i - 2);
            result  = (p0[pres_x] + p5[pres_x])*COEF[0]
                    + (p1[pres_x] + p4[pres_x])*COEF[1]
                    + (p2[pres_x] + p3[pres_x])*COEF[2];
            tmp_res[j][i] = result;
          }
        }

        for (j = 0; j < ver_block_size; j++)
        {
          orig_line = block[j];
          tmp_line  = tmp_res[j];
          x0 = tmp_res[j];
          x1 = x0 + 1;
          x2 = x1 + 1;
          x3 = x2 + 1;
          x4 = x3 + 1;
          x5 = x4 + 1;

          for (i = 0; i < hor_block_size; i++)
          {
            result  = (*(x0++) + *(x5++)) * COEF[0]
                    + (*(x1++) + *(x4++)) * COEF[1]
                    + (*(x2++) + *(x3++)) * COEF[2];

            *(orig_line++) = (imgpel) iClip1(max_imgpel_value, ((result + 512)>>10));
          }
        }

        if ((dx&1) == 1)
        {
          for (j = 0; j < ver_block_size; j++)
          {
            tmp_line  = &tmp_res[j][2 + (dx>>1)];
            orig_line = block[j];            
            for (i = 0; i < hor_block_size; i++)
            {
              *orig_line = (imgpel) ((*orig_line + iClip1(max_imgpel_value, ((*(tmp_line++) + 16)>>5)) + 1)>> 1);
              orig_line++;
            }
          }
        }
      }
      else
      {  /* Diagonal interpolation */
        tmp_pos = x_pos - 2;
        for (i = 0; i < hor_block_size; i++)
        {
          ipos_m2 = iClip3(0, maxold_x, tmp_pos++);
          ipos_m1 = iClip3(0, maxold_x, tmp_pos++);
          ipos    = iClip3(0, maxold_x, tmp_pos++);
          ipos_p1 = iClip3(0, maxold_x, tmp_pos++);
          ipos_p2 = iClip3(0, maxold_x, tmp_pos++);
          ipos_p3 = iClip3(0, maxold_x, tmp_pos  );
          tmp_pos -= 4;

          for (j = 0; j < ver_block_size; j++)
          {
            cur_lineY = cur_imgY[iClip3(0,maxold_y,(dy == 1 ? y_pos+j : y_pos+j+1))];

            result  = (cur_lineY[ipos_m2] + cur_lineY[ipos_p3]) * COEF[0];
            result += (cur_lineY[ipos_m1] + cur_lineY[ipos_p2]) * COEF[1];
            result += (cur_lineY[ipos   ] + cur_lineY[ipos_p1]) * COEF[2];

            block[j][i] = (imgpel) iClip1(max_imgpel_value, ((result+16)>>5));
          }
        }

        tmp_pos = y_pos - 2;
        for (j = 0; j < ver_block_size; j++)
        {      
          p0 = cur_imgY[iClip3(0, maxold_y, tmp_pos++)];
          p1 = cur_imgY[iClip3(0, maxold_y, tmp_pos++)];
          p2 = cur_imgY[iClip3(0, maxold_y, tmp_pos++)];
          p3 = cur_imgY[iClip3(0, maxold_y, tmp_pos++)];
          p4 = cur_imgY[iClip3(0, maxold_y, tmp_pos++)];
          p5 = cur_imgY[iClip3(0, maxold_y, tmp_pos  )];

          tmp_pos -= 4;
          orig_line = block[j];

          for (i = 0; i < hor_block_size; i++)
          {
            pres_x = dx == 1 ? x_pos+i : x_pos+i+1;
            pres_x = iClip3(0, maxold_x, pres_x);

            result  = (p0[pres_x] + p5[pres_x]) * COEF[0];
            result += (p1[pres_x] + p4[pres_x]) * COEF[1];
            result += (p2[pres_x] + p3[pres_x]) * COEF[2];

            *orig_line = (imgpel) ((*orig_line + iClip1(max_imgpel_value, ((result+16)>>5)) + 1 ) >> 1);
            orig_line++;
          }
        }      
      }
    }
  }
}

void get_block_chroma(int uv, StorablePicture *curr_ref, int x_pos, int y_pos, int hor_block_size, int ver_block_size, ImageParameters *img, imgpel block[MB_BLOCK_SIZE][MB_BLOCK_SIZE])
{
  int subpel_x    = img->mb_cr_size_x == 8 ? 7 : 3;
  int subpel_y    = img->mb_cr_size_y == 8 ? 7 : 3;
  int shiftpel_x  = img->mb_cr_size_x == 8 ? 3 : 2;
  int shiftpel_y  = img->mb_cr_size_y == 8 ? 3 : 2;
  int total_scale = shiftpel_x + shiftpel_y;

  int dx = (x_pos & subpel_x);
  int dy = (y_pos & subpel_y);
  int dxcur = (subpel_x + 1 - dx);
  int dycur = (subpel_y + 1 - dy);

  int w00 = dxcur * dycur;
  int w01 = dxcur * dy;
  int w10 = dx * dycur;
  int w11 = dx * dy;

  int i, j;
  int maxold_x = dec_picture->size_x_cr_m1;
  int maxold_y = (dec_picture->motion.mb_field[img->current_mb_nr]) ? (dec_picture->size_y_cr >> 1) - 1 : dec_picture->size_y_cr_m1;
  int result;
  
  static imgpel **cur_img, *blk_line;
  static imgpel *cur_line, *cur_line_p1;
  int tmp_pos;
  static int ipos, ipos_p1;
  int    max_imgpel_value = img->max_imgpel_value_comp[uv + 1];


  if (curr_ref == no_reference_picture && img->framepoc < img->recovery_poc)
  {
    printf("list[ref_frame] is equal to 'no reference picture' before RAP\n");

    /* fill the block with sample value 128 */
    for (j = 0; j < ver_block_size; j++)
      for (i = 0; i < hor_block_size; i++)
        block[j][i] = 128;
    return;
  }

  cur_img = curr_ref->imgUV[uv];

  x_pos = x_pos >> shiftpel_x;
  y_pos = y_pos >> shiftpel_y;

  if ((y_pos >= 0) && (y_pos < maxold_y - ver_block_size) && (x_pos >= 0) && (x_pos < maxold_x - hor_block_size))
  {
    if (dx == 0 && dy == 0)
    {  /* fullpel position */
      for (j = 0; j < ver_block_size; j++)
      {        
        memcpy(&(block[j][0]), &(cur_img[ y_pos + j ][x_pos]), hor_block_size * sizeof(imgpel));
      }
    }
    else if (dx == 0)
    { 
      for (j = 0; j < ver_block_size; j++)
      {
        cur_line    = &cur_img[y_pos + j    ][x_pos];
        cur_line_p1 = &cur_img[y_pos + j + 1][x_pos];
        blk_line = block[j];

        for (i = 0; i < hor_block_size; i++)
        {
          result = (w00 * *cur_line++ + w01 * *cur_line_p1++);
          *(blk_line++) = (imgpel) iClip1(max_imgpel_value, rshift_rnd_sf(result, total_scale));
        }
      }
    }
    else if (dy == 0)
    { 
      for (j = 0; j < ver_block_size; j++)
      {
        cur_line    = &cur_img[y_pos + j][x_pos];
        cur_line_p1 = cur_line + 1;
        blk_line = block[j];

        for (i = 0; i < hor_block_size; i++)
        {
          result = (w00 * *cur_line++ + w10 * *cur_line_p1++);
          *(blk_line++) = (imgpel) iClip1(max_imgpel_value, rshift_rnd_sf(result, total_scale));
        }
      }
    }
    else
    { /* other positions */
      for (j = 0; j < ver_block_size; j++)
      {
        cur_line    = &cur_img[y_pos + j    ][x_pos];
        cur_line_p1 = &cur_img[y_pos + j + 1][x_pos];
        blk_line = block[j];

        for (i = 0; i < hor_block_size; i++)
        {
          result  = (w00 * *(cur_line++) + w01 * *(cur_line_p1++));
          result += (w10 * *(cur_line  ) + w11 * *(cur_line_p1  ));
          *(blk_line++) = (imgpel) iClip1(max_imgpel_value, rshift_rnd_sf(result, total_scale));
        }
      }
    }
  }
  else // unsafe positions
  {
    if (dx == 0 && dy == 0)
    {  /* fullpel position */
      for (j = 0; j < ver_block_size; j++)
      {
        cur_line = cur_img[iClip3(0, maxold_y, y_pos + j)];
        blk_line = block[j];
        for (i = 0; i < hor_block_size; i++)
        {
          *(blk_line++) = cur_line[iClip3(0, maxold_x, x_pos + i )];
        }
      }
    }
    else if (dx == 0)
    { 
      for (j = 0; j < ver_block_size; j++)
      {
        cur_line    = cur_img[iClip3(0, maxold_y, y_pos + j)];
        cur_line_p1 = cur_img[iClip3(0, maxold_y, y_pos + j + 1)];
        tmp_pos = x_pos;
        blk_line = block[j];

        for (i = 0; i < hor_block_size; i++)
        {
          ipos    = iClip3(0, maxold_x, tmp_pos++);

          result = (w00 * cur_line[ipos] + w01 * cur_line_p1[ipos]);
          *(blk_line++) = (imgpel) iClip1(max_imgpel_value, rshift_rnd_sf(result, total_scale));
        }
      }      
    }
    else if (dy == 0)
    { 
      for (j = 0; j < ver_block_size; j++)
      {
        cur_line    = cur_img[iClip3(0, maxold_y, y_pos + j)];
        tmp_pos = x_pos;
        blk_line = block[j];

        for (i = 0; i < hor_block_size; i++)
        {
          ipos    = iClip3(0, maxold_x, tmp_pos++);
          ipos_p1 = iClip3(0, maxold_x, tmp_pos  );

          result = (w00 * cur_line[ipos   ] + w10 * cur_line[ipos_p1]);
          *(blk_line++) = (imgpel)iClip1(max_imgpel_value, rshift_rnd_sf(result, total_scale));
        }
      }      
    }
    else
    { /* other positions */ 
      for (j = 0; j < ver_block_size; j++)
      {
        cur_line    = cur_img[iClip3(0, maxold_y, y_pos + j)];
        cur_line_p1 = cur_img[iClip3(0, maxold_y, y_pos + j + 1)];
        tmp_pos = x_pos;
        blk_line = block[j];

        for (i = 0; i < hor_block_size; i++)
        {
          ipos    = iClip3(0, maxold_x, tmp_pos++);
          ipos_p1 = iClip3(0, maxold_x, tmp_pos  );

          result = (
            w00 * cur_line   [ipos   ] + 
            w10 * cur_line   [ipos_p1] +
            w01 * cur_line_p1[ipos   ] +
            w11 * cur_line_p1[ipos_p1]);
          *(blk_line++) = (imgpel) iClip1(max_imgpel_value, rshift_rnd_sf(result, total_scale));
        }
      }      
    }
  }
}


void intra_cr_decoding(Macroblock *currMB, int yuv, ImageParameters *img, int smb)
{
  imgpel **curUV, *cur_img;
  int (*m7UV)[16], *m7;
  int uv;
  int b8,b4;
  int ioff, joff, ii, jj;

  for(uv = 0; uv < 2; uv++)
  {
    Boolean lossless_qpprime = (Boolean) ((img->lossless_qpprime_flag == 1) &&((img->qp + dec_picture->chroma_qp_offset[uv] + img->bitdepth_chroma_qp_scale) == 0));
    itrans_4x4 = (!lossless_qpprime) ? itrans4x4 : itrans4x4_ls;

    curUV = dec_picture->imgUV[uv];
    m7UV  = img->mb_rres[uv+1];
    intrapred_chroma(img, currMB, uv);

    if (!smb && (currMB->cbp >> 4))
    {
      for (b8 = 0; b8 < (img->num_uv_blocks); b8++)
      {
        for(b4 = 0; b4 < 4; b4++)
        {
          joff = subblk_offset_y[yuv][b8][b4];          
          ioff = subblk_offset_x[yuv][b8][b4];          

          itrans_4x4(img, (ColorPlane) (uv + 1), ioff, joff);

          for(jj=joff; jj<joff + 4;jj++)
          {
            cur_img = &curUV[img->pix_c_y+jj][img->pix_c_x + ioff];
            m7 = &m7UV[jj][ioff];

            for(ii=0; ii<4;ii++)
            {
              *cur_img++ = (imgpel) *m7++;
            }
          }
        }
      }
    }
    else if ((currMB->cbp >> 4) == 0)
    {
      for (b8 = 0; b8 < (img->num_uv_blocks); b8++)
      {
        for(b4 = 0; b4 < 4; b4++)
        {
          joff = subblk_offset_y[yuv][b8][b4];
          ioff = subblk_offset_x[yuv][b8][b4];          

          for(jj = joff; jj < 4 + joff;jj++)
            memcpy(&(curUV[img->pix_c_y + jj][img->pix_c_x + ioff]), &(img->mb_pred[uv + 1][jj][ioff]), BLOCK_SIZE * sizeof(imgpel));
        }
      }
    }
    else
    {
      itrans_sp_cr(img, uv);

      for (joff  = 0; joff < 8; joff += 4)
      {
        for(ioff = 0; ioff < 8;ioff+=4)
        {          
          itrans_4x4(img, (ColorPlane) (uv + 1), ioff, joff);

          for(jj = joff; jj < joff + 4; jj++)
            for(ii = ioff; ii < ioff + 4; ii++)
            {
              curUV[img->pix_c_y+jj][ii + img->pix_c_x] = (imgpel) img->mb_rres[uv+1][jj][ii];
            }
        }
      }
    }
  }
}

void prepare_direct_params(Macroblock *currMB, StorablePicture *dec_picture, ImageParameters *img, short pmvl0[2], short pmvl1[2],char *l0_rFrame, char *l1_rFrame)
{
  char l0_rFrameL, l0_rFrameU, l0_rFrameUL, l0_rFrameUR;
  char l1_rFrameL, l1_rFrameU, l1_rFrameUL, l1_rFrameUR;
  
  PixelPos mb_left, mb_up, mb_upleft, mb_upright;

  get4x4Neighbour(currMB, -1,  0, img->mb_size[IS_LUMA], &mb_left);
  get4x4Neighbour(currMB,  0, -1, img->mb_size[IS_LUMA], &mb_up);
  get4x4Neighbour(currMB, 16, -1, img->mb_size[IS_LUMA], &mb_upright);
  get4x4Neighbour(currMB, -1, -1, img->mb_size[IS_LUMA], &mb_upleft);

  if (!img->MbaffFrameFlag)
  {
    l0_rFrameL  = (char) (mb_left.available    ? dec_picture->motion.ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x]       : -1);
    l0_rFrameU  = (char) (mb_up.available      ? dec_picture->motion.ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x]           : -1);
    l0_rFrameUL = (char) (mb_upleft.available  ? dec_picture->motion.ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x]   : -1);
    l0_rFrameUR = (char) (mb_upright.available ? dec_picture->motion.ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] : l0_rFrameUL);

    l1_rFrameL  = (char) (mb_left.available     ? dec_picture->motion.ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x]      : -1);
    l1_rFrameU  = (char) (mb_up.available       ? dec_picture->motion.ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x]          : -1);
    l1_rFrameUL = (char) (mb_upleft.available  ? dec_picture->motion.ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x]   : -1);
    l1_rFrameUR = (char) (mb_upright.available ? dec_picture->motion.ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] : l1_rFrameUL);
  }
  else
  {
    if (currMB->mb_field)
    {
      l0_rFrameL = (char) (mb_left.available 
        ? img->mb_data[mb_left.mb_addr].mb_field  || dec_picture->motion.ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] < 0
        ? dec_picture->motion.ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] 
        : dec_picture->motion.ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] * 2: -1);

      l0_rFrameU = (char) (mb_up.available 
        ? img->mb_data[mb_up.mb_addr].mb_field || dec_picture->motion.ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] < 0
        ? dec_picture->motion.ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] 
        : dec_picture->motion.ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] * 2: -1);

       l0_rFrameUL = (char) (mb_upleft.available 
         ? img->mb_data[mb_upleft.mb_addr].mb_field || dec_picture->motion.ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] < 0
         ? dec_picture->motion.ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] 
         : dec_picture->motion.ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] *2: -1);

       l0_rFrameUR = (char) (mb_upright.available 
         ? img->mb_data[mb_upright.mb_addr].mb_field || dec_picture->motion.ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] < 0 
         ? dec_picture->motion.ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] 
         : dec_picture->motion.ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] * 2: l0_rFrameUL);

       l1_rFrameL = (char) (mb_left.available 
         ? img->mb_data[mb_left.mb_addr].mb_field || dec_picture->motion.ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x]  < 0 
         ? dec_picture->motion.ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] 
         : dec_picture->motion.ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] * 2: -1);

       l1_rFrameU = (char) (mb_up.available 
         ? img->mb_data[mb_up.mb_addr].mb_field || dec_picture->motion.ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x]  < 0 
         ? dec_picture->motion.ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] 
         : dec_picture->motion.ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] * 2: -1);

       l1_rFrameUL = (char) (mb_upleft.available 
         ? img->mb_data[mb_upleft.mb_addr].mb_field || dec_picture->motion.ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x]  < 0 
         ? dec_picture->motion.ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] 
         : dec_picture->motion.ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] *2 : -1);

       l1_rFrameUR = (char) (mb_upright.available 
         ? img->mb_data[mb_upright.mb_addr].mb_field || dec_picture->motion.ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] < 0
         ? dec_picture->motion.ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] 
         : dec_picture->motion.ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] * 2: l1_rFrameUL);
    }
    else
    {
      l0_rFrameL = (char) (mb_left.available 
        ? img->mb_data[mb_left.mb_addr].mb_field || dec_picture->motion.ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x]  < 0 
        ? dec_picture->motion.ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] >> 1 
        : dec_picture->motion.ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x]: -1);

      l0_rFrameU = (char) (mb_up.available 
        ? img->mb_data[mb_up.mb_addr].mb_field || dec_picture->motion.ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] < 0 
        ? dec_picture->motion.ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] >> 1 
        : dec_picture->motion.ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] : -1);

      l0_rFrameUL = (char) (mb_upleft.available 
        ? img->mb_data[mb_upleft.mb_addr].mb_field || dec_picture->motion.ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] < 0 
        ? dec_picture->motion.ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x]>> 1 
        : dec_picture->motion.ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] : -1);

      l0_rFrameUR = (char) (mb_upright.available 
        ? img->mb_data[mb_upright.mb_addr].mb_field || dec_picture->motion.ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] < 0 
        ? dec_picture->motion.ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] >> 1 
        : dec_picture->motion.ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] : l0_rFrameUL);

      l1_rFrameL = (char) (mb_left.available 
        ? img->mb_data[mb_left.mb_addr].mb_field || dec_picture->motion.ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] < 0 
        ? dec_picture->motion.ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] >> 1 
        : dec_picture->motion.ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] : -1);

      l1_rFrameU = (char) (mb_up.available 
        ? img->mb_data[mb_up.mb_addr].mb_field || dec_picture->motion.ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] < 0 
        ? dec_picture->motion.ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] >> 1 
        : dec_picture->motion.ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] : -1);

      l1_rFrameUL = (char) (mb_upleft.available 
        ? img->mb_data[mb_upleft.mb_addr].mb_field || dec_picture->motion.ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] < 0 
        ? dec_picture->motion.ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] >> 1 
        : dec_picture->motion.ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] : -1);

      l1_rFrameUR = (char) (mb_upright.available 
        ? img->mb_data[mb_upright.mb_addr].mb_field || dec_picture->motion.ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] < 0 
        ? dec_picture->motion.ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] >> 1
        : dec_picture->motion.ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] : l1_rFrameUL);
    }
  }

  *l0_rFrame = (char) ((l0_rFrameL >= 0 && l0_rFrameU >= 0)  ? imin(l0_rFrameL,l0_rFrameU) : imax(l0_rFrameL,l0_rFrameU));
  *l0_rFrame = (char) ((*l0_rFrame >= 0 && l0_rFrameUR >= 0) ? imin(*l0_rFrame,l0_rFrameUR): imax(*l0_rFrame,l0_rFrameUR));

  *l1_rFrame = (char) ((l1_rFrameL >= 0 && l1_rFrameU >= 0)  ? imin(l1_rFrameL,l1_rFrameU) : imax(l1_rFrameL,l1_rFrameU));
  *l1_rFrame = (char) ((*l1_rFrame >= 0 && l1_rFrameUR >= 0) ? imin(*l1_rFrame,l1_rFrameUR): imax(*l1_rFrame,l1_rFrameUR));

  if (*l0_rFrame >=0)
    SetMotionVectorPredictor (img, currMB, pmvl0, *l0_rFrame, LIST_0, dec_picture->motion.ref_idx, dec_picture->motion.mv, 0, 0, 16, 16);

  if (*l1_rFrame >=0)
    SetMotionVectorPredictor (img, currMB, pmvl1, *l1_rFrame, LIST_1, dec_picture->motion.ref_idx, dec_picture->motion.mv, 0, 0, 16, 16);
}

void check_motion_vector_range(ImageParameters *img, short mv_x, short mv_y)
{
  if (mv_x > 8191 || mv_x < -8192)
  {
    fprintf(stderr,"ERROR! Horizontal motion vector %d is out of allowed range {-8192, 8191} in picture %d, macroblock %d\n", mv_x, img->number, img->current_mb_nr);
    error("invalid stream: too big horizontal motion vector", 500);
  }

  if (mv_y > (img->max_mb_vmv_r - 1) || mv_y < (-img->max_mb_vmv_r))
  {
    fprintf(stderr,"ERROR! Vertical motion vector %d is out of allowed range {%d, %d} in picture %d, macroblock %d\n", mv_y, (-img->max_mb_vmv_r), (img->max_mb_vmv_r - 1), img->number, img->current_mb_nr);
    error("invalid stream: too big vertical motion vector", 500);
  }
}

void perform_mc(ColorPlane pl, StorablePicture *dec_picture, ImageParameters *img, int pred_dir, int i, int j, int list_offset, int block_size_x, int block_size_y, int curr_mb_field)
{
  static int vec1_x=0, vec1_y=0, vec2_x=0, vec2_y=0;
  static int vec1_y_cr = 0, vec2_y_cr = 0;
  static int alpha_l0, alpha_l1, wp_offset;
  static const int mv_mul = 16; // 4 * 4
  int        max_imgpel_value = img->max_imgpel_value_comp[pl];
  
  int i4   = img->block_x + i;
  int j4   = img->block_y + j;
  int ioff = (i << 2);
  int joff = (j << 2);         
  
  assert (pred_dir<=2);

  if (pred_dir != 2)
  {
    //===== Single List Prediction =====
    short       ref_idx = dec_picture->motion.ref_idx[pred_dir][j4][i4];
    short       ref_idx_wp = ref_idx;
    short      ***mv_array = dec_picture->motion.mv[pred_dir];
    StorablePicture **list = listX[list_offset + pred_dir];

    check_motion_vector_range(img, mv_array[j4][i4][0], mv_array[j4][i4][1]);

    vec1_x = i4 * mv_mul + mv_array[j4][i4][0];
    vec1_y = (img->block_y_aff + j) * mv_mul + mv_array[j4][i4][1];

    get_block_luma (pl, list[ref_idx], vec1_x, vec1_y, block_size_x, block_size_y, img, tmp_block_l0); 

    if (img->apply_weights)
    {
      if (curr_mb_field && ((active_pps->weighted_pred_flag&&(img->type==P_SLICE|| img->type == SP_SLICE))||
         (active_pps->weighted_bipred_idc==1 && (img->type==B_SLICE))))
      {
        ref_idx_wp >>=1;
      }

      alpha_l0  = img->wp_weight[pred_dir][ref_idx_wp][0];
      wp_offset = img->wp_offset[pred_dir][ref_idx_wp][0];

      weighted_mc_prediction(&img->mb_pred[pl][joff], block_size_y, block_size_x, ioff, tmp_block_l0, alpha_l0, wp_offset, img->luma_log2_weight_denom, max_imgpel_value);
    }
    else
    {
      mc_prediction(&img->mb_pred[pl][joff], block_size_y, block_size_x, ioff, tmp_block_l0); 
    }

    if ((dec_picture->chroma_format_idc != YUV400) && (dec_picture->chroma_format_idc != YUV444) ) 
    {
      int uv;

      int ioff_cr = (img->mb_cr_size_x == MB_BLOCK_SIZE) ? ioff : ioff >> 1;
      int joff_cr = (img->mb_cr_size_y == MB_BLOCK_SIZE) ? joff : joff >> 1;
      int block_size_x_cr = img->mb_cr_size_x == MB_BLOCK_SIZE ? block_size_x : block_size_x >> 1;
      int block_size_y_cr = img->mb_cr_size_y == MB_BLOCK_SIZE ? block_size_y : block_size_y >> 1;

      vec1_y_cr = vec1_y + ((active_sps->chroma_format_idc == 1)? list[ref_idx]->chroma_vector_adjustment : 0);

      for(uv=0;uv<2;uv++)
      {
        get_block_chroma (uv, list[ref_idx], vec1_x, vec1_y_cr, block_size_x_cr, block_size_y_cr, img, tmp_block_l0);

        if (img->apply_weights)
        {
          alpha_l0  = img->wp_weight[pred_dir][ref_idx_wp][uv + 1];
          wp_offset = img->wp_offset[pred_dir][ref_idx_wp][uv + 1];

          weighted_mc_prediction(&img->mb_pred[uv + 1][joff_cr], block_size_y_cr, block_size_x_cr, ioff_cr, tmp_block_l0, alpha_l0, wp_offset, img->chroma_log2_weight_denom, img->max_imgpel_value_comp[uv + 1]);
        }
        else
        {
          mc_prediction(&img->mb_pred[uv + 1][joff_cr], block_size_y_cr, block_size_x_cr, ioff_cr, tmp_block_l0);
        }
      }
    }
  }
  else
  {
    //===== BI-PREDICTION =====
    short ***l0_mv_array = dec_picture->motion.mv[LIST_0];
    short ***l1_mv_array = dec_picture->motion.mv[LIST_1];

    short l0_refframe = dec_picture->motion.ref_idx[LIST_0][j4][i4];
    short l1_refframe = dec_picture->motion.ref_idx[LIST_1][j4][i4];
    short l0_ref_idx  = l0_refframe;
    short l1_ref_idx  = l1_refframe;

    check_motion_vector_range(img, l0_mv_array[j4][i4][0], l0_mv_array[j4][i4][1]);
    check_motion_vector_range(img, l1_mv_array[j4][i4][0], l1_mv_array[j4][i4][1]);
    vec1_x = i4 * mv_mul + l0_mv_array[j4][i4][0];
    vec2_x = i4 * mv_mul + l1_mv_array[j4][i4][0];

    vec1_y = (img->block_y_aff + j) * mv_mul + l0_mv_array[j4][i4][1];
    vec2_y = (img->block_y_aff + j) * mv_mul + l1_mv_array[j4][i4][1];

    get_block_luma(pl, listX[LIST_0 + list_offset][l0_refframe], vec1_x, vec1_y, block_size_x, block_size_y, img, tmp_block_l0);  
    get_block_luma(pl, listX[LIST_1 + list_offset][l1_refframe], vec2_x, vec2_y, block_size_x, block_size_y, img, tmp_block_l1);  

    if(img->apply_weights)
    {
      int wt_list_offset = (active_pps->weighted_bipred_idc==2)? list_offset : 0;

      // This code existed in the original. Seems pointless but copying it here for reference and in case temporal direct breaks.
      // if (mv_mode==0 && img->direct_spatial_mv_pred_flag==0 ) l1_ref_idx=0;    
      if (((active_pps->weighted_pred_flag&&(img->type==P_SLICE|| img->type == SP_SLICE))||
        (active_pps->weighted_bipred_idc==1 && (img->type==B_SLICE))) && curr_mb_field)
      {
        l0_ref_idx >>=1;
        l1_ref_idx >>=1;
      }

      alpha_l0  =   img->wbp_weight[LIST_0 + wt_list_offset][l0_ref_idx][l1_ref_idx][0];
      alpha_l1  =   img->wbp_weight[LIST_1 + wt_list_offset][l0_ref_idx][l1_ref_idx][0];
      wp_offset = ((img->wp_offset [LIST_0 + wt_list_offset][l0_ref_idx][0] + img->wp_offset[LIST_1 + wt_list_offset][l1_ref_idx][0] + 1) >>1);

      weighted_bi_prediction(&img->mb_pred[pl][joff], tmp_block_l0, tmp_block_l1, block_size_y, block_size_x, ioff, alpha_l0, alpha_l1, wp_offset, (img->luma_log2_weight_denom + 1), max_imgpel_value);
    }
    else
    { 
      bi_prediction(&img->mb_pred[pl][joff], tmp_block_l0, tmp_block_l1, block_size_y, block_size_x, ioff); 
    }

    if ((dec_picture->chroma_format_idc != YUV400) && (dec_picture->chroma_format_idc != YUV444) ) 
    {
      int uv;

      int ioff_cr = img->mb_cr_size_x == MB_BLOCK_SIZE ? ioff : ioff >> 1;
      int joff_cr = img->mb_cr_size_y == MB_BLOCK_SIZE ? joff : joff >> 1;
      int block_size_x_cr = img->mb_cr_size_x == MB_BLOCK_SIZE ? block_size_x : block_size_x >> 1;
      int block_size_y_cr = img->mb_cr_size_y == MB_BLOCK_SIZE ? block_size_y : block_size_y >> 1;

      vec1_y_cr = vec1_y + ((active_sps->chroma_format_idc == 1)? listX[LIST_0 + list_offset][l0_refframe]->chroma_vector_adjustment : 0);
      vec2_y_cr = vec2_y + ((active_sps->chroma_format_idc == 1)? listX[LIST_1 + list_offset][l1_refframe]->chroma_vector_adjustment : 0);

      for(uv=0;uv<2;uv++)
      {
        get_block_chroma (uv, listX[LIST_0 + list_offset][l0_refframe], vec1_x, vec1_y_cr, block_size_x_cr, block_size_y_cr, img, tmp_block_l0);
        get_block_chroma (uv, listX[LIST_1 + list_offset][l1_refframe], vec2_x, vec2_y_cr, block_size_x_cr, block_size_y_cr, img, tmp_block_l1);

        if(img->apply_weights)
        {
          int wt_list_offset = (active_pps->weighted_bipred_idc==2)? list_offset : 0;

          alpha_l0  =   img->wbp_weight[LIST_0 + wt_list_offset][l0_ref_idx][l1_ref_idx][uv + 1];
          alpha_l1  =   img->wbp_weight[LIST_1 + wt_list_offset][l0_ref_idx][l1_ref_idx][uv + 1];
          wp_offset = ((img->wp_offset [LIST_0 + wt_list_offset][l0_ref_idx][uv + 1] + img->wp_offset[LIST_1 + wt_list_offset][l1_ref_idx][uv + 1] + 1) >>1);

          weighted_bi_prediction(&img->mb_pred[uv+1][joff_cr], tmp_block_l0, tmp_block_l1, block_size_y_cr, block_size_x_cr, ioff_cr, alpha_l0, alpha_l1, wp_offset, (img->chroma_log2_weight_denom + 1), img->max_imgpel_value_comp[uv + 1]);
        }
        else
        {
          bi_prediction(&img->mb_pred[uv + 1][joff_cr], tmp_block_l0, tmp_block_l1, block_size_y_cr, block_size_x_cr, ioff_cr);
        }
      }
    }      
  }
}

