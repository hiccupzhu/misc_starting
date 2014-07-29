
/*!
 ***********************************************************************
 *  \file
 *      block.c
 *
 *  \brief
 *      Block functions
 *
 *  \author
 *      Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Inge Lille-Langoy          <inge.lille-langoy@telenor.com>
 *      - Rickard Sjoberg            <rickard.sjoberg@era.ericsson.se>
 ***********************************************************************
 */

#include "contributors.h"

#include "global.h"
#include "block.h"
#include "image.h"
#include "mb_access.h"
#include "transform.h"
#include "quant.h"

// Notation for comments regarding prediction and predictors.
// The pels of the 4x4 block are labelled a..p. The predictor pels above
// are labelled A..H, from the left I..L, and from above left X, as follows:
//
//  X A B C D E F G H
//  I a b c d
//  J e f g h
//  K i j k l
//  L m n o p
//

// Predictor array index definitions
#define P_X (PredPel[0])
#define P_A (PredPel[1])
#define P_B (PredPel[2])
#define P_C (PredPel[3])
#define P_D (PredPel[4])
#define P_E (PredPel[5])
#define P_F (PredPel[6])
#define P_G (PredPel[7])
#define P_H (PredPel[8])
#define P_I (PredPel[9])
#define P_J (PredPel[10])
#define P_K (PredPel[11])
#define P_L (PredPel[12])

  static int M4[BLOCK_SIZE][BLOCK_SIZE];
  static int PBlock[MB_BLOCK_SIZE][MB_BLOCK_SIZE];

static void intra_chroma_DC_single(imgpel **curr_img, int up_avail, int left_avail, PixelPos up, PixelPos left[17], int blk_x, int blk_y, int *pred, int direction )
{
  static int i;
  int s0 = 0;

  if ((direction && up_avail) || (!left_avail && up_avail))
  {
    for (i = blk_x; i < (blk_x + 4);i++)  
      s0 += curr_img[up.pos_y][up.pos_x + i];
    *pred = (s0 + 2) >> 2;
  }
  else if (left_avail)  
  {
    for (i = blk_y; i < (blk_y + 4);i++)  
      s0 += curr_img[left[i].pos_y][left[i].pos_x];
    *pred = (s0 + 2) >> 2;
  }
}


static void intra_chroma_DC_all(imgpel **curr_img, int up_avail, int left_avail, PixelPos up, PixelPos left[17], int blk_x, int blk_y, int *pred )
{
  static int i;
  int s0 = 0, s1 = 0;

  if (up_avail)       
    for (i = blk_x; i < (blk_x + 4);i++)  
      s0 += curr_img[up.pos_y][up.pos_x + i];
  
  if (left_avail)  
    for (i = blk_y; i < (blk_y + 4);i++)  
      s1 += curr_img[left[i].pos_y][left[i].pos_x];
  
  if (up_avail && left_avail)
    *pred = (s0 + s1 + 4) >> 3;
  else if (up_avail)
    *pred = (s0 + 2) >> 2;
  else if (left_avail)
    *pred = (s1 + 2) >> 2;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 4x4 blocks with all 5 intra prediction modes
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was sucessfull            \n
 *    SEARCH_SYNC   search next sync element as errors while decoding occured
 ***********************************************************************
 */
int intrapred(ImageParameters *img,  //!< image parameters 
              Macroblock *currMB, 
              ColorPlane pl,               
              int ioff,             //!< pixel offset X within MB
              int joff,             //!< pixel offset Y within MB
              int img_block_x,      //!< location of block X, multiples of 4
              int img_block_y)      //!< location of block Y, multiples of 4
{
  int i,j;
  int s0;  
  imgpel PredPel[13];  // array of predictor pels
  int uv = pl-1;
  imgpel **imgY = (pl) ? dec_picture->imgUV[uv] : dec_picture->imgY;

  PixelPos pix_a[4];
  PixelPos pix_b, pix_c, pix_d;

  int block_available_up;
  int block_available_left;
  int block_available_up_left;
  int block_available_up_right;

  byte predmode = img->ipredmode[img_block_y][img_block_x];
  int jpos0 = joff, jpos1 = joff + 1, jpos2 = joff + 2, jpos3 = joff + 3;
  int ipos0 = ioff, ipos1 = ioff + 1, ipos2 = ioff + 2, ipos3 = ioff + 3;
  imgpel *predrow, prediction, (*mpr)[16] = img->mb_pred[pl];

  ipmode_DPCM = predmode; //For residual DPCM

  for (i=0;i<4;i++)
  {
    getNeighbour(currMB, ioff -1 , joff +i , img->mb_size[IS_LUMA], &pix_a[i]);
  }

  getNeighbour(currMB, ioff    , joff -1 , img->mb_size[IS_LUMA], &pix_b);
  getNeighbour(currMB, ioff +4 , joff -1 , img->mb_size[IS_LUMA], &pix_c);
  getNeighbour(currMB, ioff -1 , joff -1 , img->mb_size[IS_LUMA], &pix_d);

  pix_c.available = pix_c.available && !((ioff==4) && ((joff==4)||(joff==12)));

  if (active_pps->constrained_intra_pred_flag)
  {
    for (i=0, block_available_left=1; i<4;i++)
      block_available_left  &= pix_a[i].available ? img->intra_block[pix_a[i].mb_addr]: 0;
    block_available_up       = pix_b.available ? img->intra_block [pix_b.mb_addr] : 0;
    block_available_up_right = pix_c.available ? img->intra_block [pix_c.mb_addr] : 0;
    block_available_up_left  = pix_d.available ? img->intra_block [pix_d.mb_addr] : 0;
  }
  else
  {
    block_available_left     = pix_a[0].available;
    block_available_up       = pix_b.available;
    block_available_up_right = pix_c.available;
    block_available_up_left  = pix_d.available;
  }

  // form predictor pels
  if (block_available_up)
  {
    P_A = imgY[pix_b.pos_y][pix_b.pos_x + 0];
    P_B = imgY[pix_b.pos_y][pix_b.pos_x + 1];
    P_C = imgY[pix_b.pos_y][pix_b.pos_x + 2];
    P_D = imgY[pix_b.pos_y][pix_b.pos_x + 3];

  }
  else
  {
    P_A = P_B = P_C = P_D = (imgpel) img->dc_pred_value_comp[pl];
  }

  if (block_available_up_right)
  {
    P_E = imgY[pix_c.pos_y][pix_c.pos_x + 0];
    P_F = imgY[pix_c.pos_y][pix_c.pos_x + 1];
    P_G = imgY[pix_c.pos_y][pix_c.pos_x + 2];
    P_H = imgY[pix_c.pos_y][pix_c.pos_x + 3];
  }
  else
  {
    P_E = P_F = P_G = P_H = P_D;
  }

  if (block_available_left)
  {
    P_I = imgY[pix_a[0].pos_y][pix_a[0].pos_x];
    P_J = imgY[pix_a[1].pos_y][pix_a[1].pos_x];
    P_K = imgY[pix_a[2].pos_y][pix_a[2].pos_x];
    P_L = imgY[pix_a[3].pos_y][pix_a[3].pos_x];
  }
  else
  {
    P_I = P_J = P_K = P_L = (imgpel) img->dc_pred_value_comp[pl];
  }

  if (block_available_up_left)
  {
    P_X = imgY[pix_d.pos_y][pix_d.pos_x];
  }
  else
  {
    P_X = (imgpel) img->dc_pred_value_comp[pl];
  }


  switch (predmode)
  {
  case DC_PRED:                         /* DC prediction */

    s0 = 0;
    if (block_available_up && block_available_left)
    {
      // no edge
      s0 = (P_A + P_B + P_C + P_D + P_I + P_J + P_K + P_L + 4)>>3;
    }
    else if (!block_available_up && block_available_left)
    {
      // upper edge
      s0 = (P_I + P_J + P_K + P_L + 2)>>2;
    }
    else if (block_available_up && !block_available_left)
    {
      // left edge
      s0 = (P_A + P_B + P_C + P_D + 2)>>2;
    }
    else //if (!block_available_up && !block_available_left)
    {
      // top left corner, nothing to predict from
      s0 = img->dc_pred_value_comp[pl];
    }

    for (j=joff; j < joff + BLOCK_SIZE; j++)
    {
      for (i=ioff; i < ioff + BLOCK_SIZE; i++)
      {
        // store DC prediction
        mpr[j][i] = (imgpel) s0;
      }
    }
    break;

  case VERT_PRED:                       /* vertical prediction from block above */
    if (!block_available_up)
      printf ("warning: Intra_4x4_Vertical prediction mode not allowed at mb %d\n", (int) img->current_mb_nr);

    for(j = joff; j < joff + BLOCK_SIZE; j++) /* store predicted 4x4 block */
      memcpy(&(mpr[j][ioff]), &(imgY[pix_b.pos_y][pix_b.pos_x]), BLOCK_SIZE * sizeof(imgpel));
    break;

  case HOR_PRED:                        /* horizontal prediction from left block */
    if (!block_available_left)
      printf ("warning: Intra_4x4_Horizontal prediction mode not allowed at mb %d\n",(int) img->current_mb_nr);

    for(j=0;j<BLOCK_SIZE;j++)
    {
      predrow = mpr[j+joff];
      prediction = imgY[pix_a[j].pos_y][pix_a[j].pos_x];
      for(i = ioff;i < ioff + BLOCK_SIZE;i++)
        predrow[i]= prediction; /* store predicted 4x4 block */
    }
    break;

  case DIAG_DOWN_RIGHT_PRED:
    if ((!block_available_up)||(!block_available_left)||(!block_available_up_left))
      printf ("warning: Intra_4x4_Diagonal_Down_Right prediction mode not allowed at mb %d\n",(int) img->current_mb_nr);

    mpr[jpos3][ipos0] = (imgpel) ((P_L + 2*P_K + P_J + 2) >> 2);
    mpr[jpos2][ipos0] =
    mpr[jpos3][ipos1] = (imgpel) ((P_K + 2*P_J + P_I + 2) >> 2);
    mpr[jpos1][ipos0] =
    mpr[jpos2][ipos1] =
    mpr[jpos3][ipos2] = (imgpel) ((P_J + 2*P_I + P_X + 2) >> 2);
    mpr[jpos0][ipos0] =
    mpr[jpos1][ipos1] =
    mpr[jpos2][ipos2] =
    mpr[jpos3][ipos3] = (imgpel) ((P_I + 2*P_X + P_A + 2) >> 2);
    mpr[jpos0][ipos1] =
    mpr[jpos1][ipos2] =
    mpr[jpos2][ipos3] = (imgpel) ((P_X + 2*P_A + P_B + 2) >> 2);
    mpr[jpos0][ipos2] =
    mpr[jpos1][ipos3] = (imgpel) ((P_A + 2*P_B + P_C + 2) >> 2);
    mpr[jpos0][ipos3] = (imgpel) ((P_B + 2*P_C + P_D + 2) >> 2);
    break;

  case DIAG_DOWN_LEFT_PRED:
    if (!block_available_up)
      printf ("warning: Intra_4x4_Diagonal_Down_Left prediction mode not allowed at mb %d\n", (int) img->current_mb_nr);

    mpr[jpos0][ipos0] = (imgpel) ((P_A + P_C + 2*(P_B) + 2) >> 2);
    mpr[jpos0][ipos1] =
    mpr[jpos1][ipos0] = (imgpel) ((P_B + P_D + 2*(P_C) + 2) >> 2);
    mpr[jpos0][ipos2] =
    mpr[jpos1][ipos1] =
    mpr[jpos2][ipos0] = (imgpel) ((P_C + P_E + 2*(P_D) + 2) >> 2);
    mpr[jpos0][ipos3] =
    mpr[jpos1][ipos2] =
    mpr[jpos2][ipos1] =
    mpr[jpos3][ipos0] = (imgpel) ((P_D + P_F + 2*(P_E) + 2) >> 2);
    mpr[jpos1][ipos3] =
    mpr[jpos2][ipos2] =
    mpr[jpos3][ipos1] = (imgpel) ((P_E + P_G + 2*(P_F) + 2) >> 2);
    mpr[jpos2][ipos3] =
    mpr[jpos3][ipos2] = (imgpel) ((P_F + P_H + 2*(P_G) + 2) >> 2);
    mpr[jpos3][ipos3] = (imgpel) ((P_G + 3*(P_H) + 2) >> 2);
    break;

  case  VERT_RIGHT_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
    if ((!block_available_up)||(!block_available_left)||(!block_available_up_left))
      printf ("warning: Intra_4x4_Vertical_Right prediction mode not allowed at mb %d\n", (int) img->current_mb_nr);

    mpr[jpos0][ipos0] =
    mpr[jpos2][ipos1] = (imgpel) ((P_X + P_A + 1) >> 1);
    mpr[jpos0][ipos1] =
    mpr[jpos2][ipos2] = (imgpel) ((P_A + P_B + 1) >> 1);
    mpr[jpos0][ipos2] =
    mpr[jpos2][ipos3] = (imgpel) ((P_B + P_C + 1) >> 1);
    mpr[jpos0][ipos3] = (imgpel) ((P_C + P_D + 1) >> 1);
    mpr[jpos1][ipos0] =
    mpr[jpos3][ipos1] = (imgpel) ((P_I + 2*P_X + P_A + 2) >> 2);
    mpr[jpos1][ipos1] =
    mpr[jpos3][ipos2] = (imgpel) ((P_X + 2*P_A + P_B + 2) >> 2);
    mpr[jpos1][ipos2] =
    mpr[jpos3][ipos3] = (imgpel) ((P_A + 2*P_B + P_C + 2) >> 2);
    mpr[jpos1][ipos3] = (imgpel) ((P_B + 2*P_C + P_D + 2) >> 2);
    mpr[jpos2][ipos0] = (imgpel) ((P_X + 2*P_I + P_J + 2) >> 2);
    mpr[jpos3][ipos0] = (imgpel) ((P_I + 2*P_J + P_K + 2) >> 2);
    break;

  case  VERT_LEFT_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
    if (!block_available_up)
      printf ("warning: Intra_4x4_Vertical_Left prediction mode not allowed at mb %d\n", (int) img->current_mb_nr);

    mpr[jpos0][ipos0] = (imgpel) ((P_A + P_B + 1) >> 1);
    mpr[jpos0][ipos1] =
    mpr[jpos2][ipos0] = (imgpel) ((P_B + P_C + 1) >> 1);
    mpr[jpos0][ipos2] =
    mpr[jpos2][ipos1] = (imgpel) ((P_C + P_D + 1) >> 1);
    mpr[jpos0][ipos3] =
    mpr[jpos2][ipos2] = (imgpel) ((P_D + P_E + 1) >> 1);
    mpr[jpos2][ipos3] = (imgpel) ((P_E + P_F + 1) >> 1);
    mpr[jpos1][ipos0] = (imgpel) ((P_A + 2*P_B + P_C + 2) >> 2);
    mpr[jpos1][ipos1] =
    mpr[jpos3][ipos0] = (imgpel) ((P_B + 2*P_C + P_D + 2) >> 2);
    mpr[jpos1][ipos2] =
    mpr[jpos3][ipos1] = (imgpel) ((P_C + 2*P_D + P_E + 2) >> 2);
    mpr[jpos1][ipos3] =
    mpr[jpos3][ipos2] = (imgpel) ((P_D + 2*P_E + P_F + 2) >> 2);
    mpr[jpos3][ipos3] = (imgpel) ((P_E + 2*P_F + P_G + 2) >> 2);
    break;

  case  HOR_UP_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
    if (!block_available_left)
      printf ("warning: Intra_4x4_Horizontal_Up prediction mode not allowed at mb %d\n",(int) img->current_mb_nr);

    mpr[jpos0][ipos0] = (imgpel) ((P_I + P_J + 1) >> 1);
    mpr[jpos0][ipos1] = (imgpel) ((P_I + 2*P_J + P_K + 2) >> 2);
    mpr[jpos0][ipos2] =
    mpr[jpos1][ipos0] = (imgpel) ((P_J + P_K + 1) >> 1);
    mpr[jpos0][ipos3] =
    mpr[jpos1][ipos1] = (imgpel) ((P_J + 2*P_K + P_L + 2) >> 2);
    mpr[jpos1][ipos2] =
    mpr[jpos2][ipos0] = (imgpel) ((P_K + P_L + 1) >> 1);
    mpr[jpos1][ipos3] =
    mpr[jpos2][ipos1] = (imgpel) ((P_K + 2*P_L + P_L + 2) >> 2);
    mpr[jpos2][ipos3] =
    mpr[jpos3][ipos1] =
    mpr[jpos3][ipos0] =
    mpr[jpos2][ipos2] =
    mpr[jpos3][ipos2] =
    mpr[jpos3][ipos3] = (imgpel) P_L;
    break;

  case  HOR_DOWN_PRED:/* diagonal prediction -22.5 deg to horizontal plane */
    if ((!block_available_up)||(!block_available_left)||(!block_available_up_left))
      printf ("warning: Intra_4x4_Horizontal_Down prediction mode not allowed at mb %d\n", (int) img->current_mb_nr);

    mpr[jpos0][ipos0] =
    mpr[jpos1][ipos2] = (imgpel) ((P_X + P_I + 1) >> 1);
    mpr[jpos0][ipos1] =
    mpr[jpos1][ipos3] = (imgpel) ((P_I + 2*P_X + P_A + 2) >> 2);
    mpr[jpos0][ipos2] = (imgpel) ((P_X + 2*P_A + P_B + 2) >> 2);
    mpr[jpos0][ipos3] = (imgpel) ((P_A + 2*P_B + P_C + 2) >> 2);
    mpr[jpos1][ipos0] =
    mpr[jpos2][ipos2] = (imgpel) ((P_I + P_J + 1) >> 1);
    mpr[jpos1][ipos1] =
    mpr[jpos2][ipos3] = (imgpel) ((P_X + 2*P_I + P_J + 2) >> 2);
    mpr[jpos2][ipos0] =
    mpr[jpos3][ipos2] = (imgpel) ((P_J + P_K + 1) >> 1);
    mpr[jpos2][ipos1] =
    mpr[jpos3][ipos3] = (imgpel) ((P_I + 2*P_J + P_K + 2) >> 2);
    mpr[jpos3][ipos0] = (imgpel) ((P_K + P_L + 1) >> 1);
    mpr[jpos3][ipos1] = (imgpel) ((P_J + 2*P_K + P_L + 2) >> 2);
    break;

  default:
    printf("Error: illegal intra_4x4 prediction mode: %d\n", (int) predmode);
    return SEARCH_SYNC;
    break;
  }

  return DECODING_OK;
}


/*!
 ***********************************************************************
 * \return
 *    best SAD
 ***********************************************************************
 */ 
int intrapred_luma_16x16(ImageParameters *img, //!< image parameters
                         Macroblock *currMB,  //!< Current Macroblock
                         ColorPlane pl,       //!< Current colorplane (for 4:4:4)                         
                         int predmode)        //!< prediction mode
{
  int s0 = 0, s1, s2;

  int i,j;

  int ih,iv;
  int ib,ic,iaa;

  int uv = pl-1;
  imgpel **imgY = (pl) ? dec_picture->imgUV[uv] : dec_picture->imgY;
  imgpel (*mpr)[MB_BLOCK_SIZE] = &(img->mb_pred[pl][0]); 
  imgpel *mpr_line;
  imgpel prediction;
  int max_imgpel_value = img->max_imgpel_value_comp[pl];

  PixelPos up;          //!< pixel position p(0,-1)
  PixelPos left[17];    //!< pixel positions p(-1, -1..15)

  int up_avail, left_avail, left_up_avail;

  s1=s2=0;

  for (i=0;i<17;i++)
  {
    getNeighbour(currMB, -1 ,  i-1 , img->mb_size[IS_LUMA], &left[i]);
  }
  getNeighbour(currMB, 0     ,  -1 , img->mb_size[IS_LUMA], &up);

  if (!active_pps->constrained_intra_pred_flag)
  {
    up_avail   = up.available;
    left_avail = left[1].available;
    left_up_avail = left[0].available;
  }
  else
  {
    up_avail      = up.available ? img->intra_block[up.mb_addr] : 0;
    for (i=1, left_avail=1; i<17;i++)
      left_avail  &= left[i].available ? img->intra_block[left[i].mb_addr]: 0;
    left_up_avail = left[0].available ? img->intra_block[left[0].mb_addr]: 0;
  }

  switch (predmode)
  {
  case VERT_PRED_16:                       // vertical prediction from block above
    if (!up_avail)
      error ("invalid 16x16 intra pred Mode VERT_PRED_16",500);
    for(j=0;j<MB_BLOCK_SIZE;j++)
      memcpy(img->mb_pred[pl][j], &(imgY[up.pos_y][up.pos_x]), MB_BLOCK_SIZE * sizeof(imgpel));
    break;
  case HOR_PRED_16:                        // horizontal prediction from left block
    if (!left_avail)
      error ("invalid 16x16 intra pred Mode HOR_PRED_16",500);
    for(j = 0; j < MB_BLOCK_SIZE; j++)
    {
      prediction = imgY[left[j+1].pos_y][left[j+1].pos_x];
      for(i=0;i<MB_BLOCK_SIZE;i++)
        mpr[j][i]= prediction; // store predicted 16x16 block
    }
    break;
  case DC_PRED_16:                         // DC prediction
    s1=s2=0;
    for (i = 0; i < MB_BLOCK_SIZE; i++)
    {
      if (up_avail)
        s1 += imgY[up.pos_y][up.pos_x+i];    // sum hor pix
      if (left_avail)
        s2 += imgY[left[i + 1].pos_y][left[i + 1].pos_x];    // sum vert pix
    }
    if (up_avail && left_avail)
      s0 = (s1 + s2 + 16)>>5;       // no edge
    if (!up_avail && left_avail)
      s0 = (s2 + 8)>>4;              // upper edge
    if (up_avail && !left_avail)
      s0 = (s1 + 8)>>4;              // left edge
    if (!up_avail && !left_avail)
      s0 = img->dc_pred_value_comp[pl];                            // top left corner, nothing to predict from

    for(j = 0; j < MB_BLOCK_SIZE; j++)
      for(i = 0; i < MB_BLOCK_SIZE; i++)
      {
        mpr[j][i]=(imgpel) s0;
      }

    break;
  case PLANE_16:// 16 bit integer plan pred
    if (!up_avail || !left_up_avail  || !left_avail)
      error ("invalid 16x16 intra pred Mode PLANE_16",500);

    ih=0;
    iv=0;

    mpr_line = &imgY[up.pos_y][up.pos_x+7];
    for (i = 1; i < 8; i++)
    {
      ih += i*(mpr_line[i] - mpr_line[-i]);
      iv += i*(imgY[left[8+i].pos_y][left[8+i].pos_x] - imgY[left[8-i].pos_y][left[8-i].pos_x]);
    }

    ih += 8*(mpr_line[8] - imgY[left[0].pos_y][left[0].pos_x]);
    iv += 8*(imgY[left[16].pos_y][left[16].pos_x] - imgY[left[0].pos_y][left[0].pos_x]);

    ib=(5*ih+32)>>6;
    ic=(5*iv+32)>>6;

    iaa=16*(mpr_line[8] + imgY[left[16].pos_y][left[16].pos_x]);
    for (j=0;j< MB_BLOCK_SIZE;j++)
    {
      for (i=0;i< MB_BLOCK_SIZE;i++)
      {
        mpr[j][i]=(imgpel) iClip1(max_imgpel_value,((iaa+(i-7)*ib +(j-7)*ic + 16)>>5));
      }
    }// store plane prediction
    break;

  default:
    {                                    // indication of fault in bitstream,exit
      printf("illegal 16x16 intra prediction mode input: %d\n",predmode);
      return SEARCH_SYNC;
    }
  }

  return DECODING_OK;
}

/*!
 ************************************************************************
 * \brief
 *    Chroma Intra prediction. Note that many operations can be moved
 *    outside since they are repeated for both components for no reason.
 ************************************************************************
 */

void intrapred_chroma(ImageParameters *img, Macroblock *currMB, int uv)
{
  int i,j, ii, jj;

  imgpel **imgUV = dec_picture->imgUV[uv];
  imgpel (*mpr)[16] = img->mb_pred[uv + 1];
  int     max_imgpel_value = img->max_imgpel_value_comp[uv + 1];
  
  int ih, iv, ib, ic, iaa;

  int        b8, b4;
  int        yuv = dec_picture->chroma_format_idc - 1;
  int        blk_x, blk_y;
  int        pred;
  static int block_pos[3][4][4]= //[yuv][b8][b4]
  {
    { {0, 1, 2, 3},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}},
    { {0, 1, 2, 3},{2, 3, 2, 3},{0, 0, 0, 0},{0, 0, 0, 0}},
    { {0, 1, 2, 3},{1, 1, 3, 3},{2, 3, 2, 3},{3, 3, 3, 3}}
  };

  PixelPos up;        //!< pixel position  p(0,-1)
  PixelPos left[17];  //!< pixel positions p(-1, -1..16)

  int up_avail, left_avail[2], left_up_avail;

  int cr_MB_x = img->mb_cr_size_x;
  int cr_MB_y = img->mb_cr_size_y;
  int cr_MB_y2 = (cr_MB_y >> 1);
  int cr_MB_x2 = (cr_MB_x >> 1);

  for (i=0; i < cr_MB_y + 1 ; i++)
  {
    getNeighbour(currMB, -1, i-1, img->mb_size[IS_CHROMA], &left[i]);
  }

  getNeighbour(currMB, 0, -1, img->mb_size[IS_CHROMA], &up);

  if (!active_pps->constrained_intra_pred_flag)
  {
    up_avail      = up.available;
    left_avail[0] = left_avail[1] = left[1].available;
    left_up_avail = left[0].available;
  }
  else
  {
    up_avail = up.available ? img->intra_block[up.mb_addr] : 0;
    for (i=0, left_avail[0] = 1; i < cr_MB_y2;i++)
      left_avail[0]  &= left[i + 1].available ? img->intra_block[left[i + 1].mb_addr]: 0;
    for (i = cr_MB_y2, left_avail[1] = 1; i<cr_MB_y;i++)
      left_avail[1]  &= left[i + 1].available ? img->intra_block[left[i + 1].mb_addr]: 0;
    left_up_avail = left[0].available ? img->intra_block[left[0].mb_addr]: 0;
  }

  switch (currMB->c_ipred_mode)
  {
  case DC_PRED_8:
    // DC prediction
    // Note that unlike what is stated in many presentations and papers, this mode does not operate
    // the same way as I_16x16 DC prediction.
    for(b8 = 0; b8 < (img->num_uv_blocks) ;b8++)
    {
      for (b4 = 0; b4 < 4; b4++)
      {
        blk_y = subblk_offset_y[yuv][b8][b4];
        blk_x = subblk_offset_x[yuv][b8][b4];

        pred = img->dc_pred_value_comp[1];

        //===== get prediction value =====
        switch (block_pos[yuv][b8][b4])
        {
        case 0:  //===== TOP LEFT =====
          intra_chroma_DC_all   (imgUV, up_avail, left_avail[0], up, left, blk_x, blk_y + 1, &pred);
          break;
        case 1: //===== TOP RIGHT =====
          intra_chroma_DC_single(imgUV, up_avail, left_avail[0], up, left, blk_x, blk_y + 1, &pred, 1);
          break;
        case 2: //===== BOTTOM LEFT =====
          intra_chroma_DC_single(imgUV, up_avail, left_avail[1], up, left, blk_x, blk_y + 1, &pred, 0);
          break;
        case 3: //===== BOTTOM RIGHT =====
          intra_chroma_DC_all   (imgUV, up_avail, left_avail[1], up, left, blk_x, blk_y + 1, &pred);          
          break;
        }
        
        for (jj = blk_y; jj < blk_y + BLOCK_SIZE; jj++)
          for (ii = blk_x; ii < blk_x + BLOCK_SIZE; ii++)
          {
            mpr[jj][ii]=(imgpel) pred;
          }
      }
    }
    break;
  case HOR_PRED_8:
    // Horizontal Prediction
    if (!left_avail[0] || !left_avail[1])
      error("unexpected HOR_PRED_8 chroma intra prediction mode",-1);

    for (j = 0; j < cr_MB_y; j++)
    {
      pred = imgUV[left[1 + j].pos_y][left[1 + j].pos_x];
      for (i = 0; i < cr_MB_x; i++)
        mpr[j][i]=(imgpel) pred;
    }

    break;
  case VERT_PRED_8:
    // Vertical Prediction
    if (!up_avail)
      error("unexpected VERT_PRED_8 chroma intra prediction mode",-1);

    for (j = 0; j < cr_MB_y; j++)
    {
      memcpy(&(mpr[j][0]), &(imgUV[up.pos_y][up.pos_x]), cr_MB_x * sizeof(imgpel));
    }
    break;
  case PLANE_8:
    // plane prediction
    if (!left_up_avail || !left_avail[0] || !left_avail[1] || !up_avail)
      error("unexpected PLANE_8 chroma intra prediction mode",-1);
    else
    {
      imgpel *upPred = &imgUV[up.pos_y][up.pos_x];

      ih = cr_MB_x2 * (upPred[cr_MB_x - 1] - imgUV[left[0].pos_y][left[0].pos_x]);
      for (i = 0; i < cr_MB_x2 - 1; i++)
        ih += (i + 1) * (upPred[cr_MB_x2 + i] - upPred[cr_MB_x2 - 2 - i]);

      iv = cr_MB_y2 * (imgUV[left[cr_MB_y].pos_y][left[cr_MB_y].pos_x] - imgUV[left[0].pos_y][left[0].pos_x]);
      for (i = 0; i < cr_MB_y2 - 1; i++)
        iv += (i + 1)*(imgUV[left[cr_MB_y2 + 1 + i].pos_y][left[cr_MB_y2 + 1 + i].pos_x] -
        imgUV[left[cr_MB_y2 - 1 - i].pos_y][left[cr_MB_y2 - 1 - i].pos_x]);

      ib= ((cr_MB_x == 8 ? 17 : 5) * ih + 2 * cr_MB_x)>>(cr_MB_x == 8 ? 5 : 6);
      ic= ((cr_MB_y == 8 ? 17 : 5) * iv + 2 * cr_MB_y)>>(cr_MB_y == 8 ? 5 : 6);

      iaa=16*(imgUV[left[cr_MB_y].pos_y][left[cr_MB_y].pos_x] + upPred[cr_MB_x-1]);

      for (j = 0; j < cr_MB_y; j++)
        for (i = 0; i < cr_MB_x; i++)
          mpr[j][i]=(imgpel) iClip1(max_imgpel_value, ((iaa + (i - cr_MB_x2 + 1) * ib + (j - cr_MB_y2 + 1) * ic + 16) >> 5));  }
    break;
  default:
    error("illegal chroma intra prediction mode", 600);
    break;
  }
}

/*!
 ***********************************************************************
 * \brief
 *    Inverse 4x4 transformation, transforms cof to m7
 ***********************************************************************
 */
void itrans4x4(ImageParameters *img, //!< image parameters
               ColorPlane pl,              
               int ioff,            //!< index to 4x4 block
               int joff)            //!<
{
  static int i,j;

  int max_imgpel_value = img->max_imgpel_value_comp[pl];

  imgpel (*mpr)[MB_BLOCK_SIZE] = img->mb_pred[pl];
  int    (*m7) [MB_BLOCK_SIZE] = img->mb_rres[pl];

  inverse4x4(img->cof[pl],m7,joff,ioff);

  for (j = joff; j < joff + BLOCK_SIZE; j++)
  {
    for (i = ioff; i < ioff + BLOCK_SIZE; i++)
    {      
      m7[j][i] = iClip1(max_imgpel_value, rshift_rnd_sf((m7[j][i] + ((long)mpr[j][i] << DQ_BITS)), DQ_BITS));
    }
  }
}

/*!
 ***********************************************************************
 * \brief
 *    Inverse 4x4 lossless_qpprime transformation, transforms cof to m7
 ***********************************************************************
 */
void itrans4x4_ls(ImageParameters *img, //!< image parameters
                  ColorPlane pl,        //!< Color plane (for 4:4:4)                  
                  int ioff,             //!< index to 4x4 block
                  int joff)             //!<
{
  static int i,j;

  int max_imgpel_value = img->max_imgpel_value_comp[pl];

  imgpel (*mpr)[16] = img->mb_pred[pl];
  int    ( *m7)[16] = img->mb_rres [pl];

  inverse4x4(img->cof[pl],m7,joff,ioff);

  for (j = joff; j < joff + BLOCK_SIZE; j++)
  {
    for (i = ioff; i < ioff + BLOCK_SIZE; i++)
    {      
      m7[j][i] = iClip1(max_imgpel_value, rshift_rnd_sf((m7[j][i] + ((long)mpr[j][i] << DQ_BITS)), DQ_BITS));
    }
  }
}

/*!
************************************************************************
* \brief
*    Inverse residual DPCM for Intra lossless coding
*
* \par Input:
*    ioff_x,joff_y: Block position inside a macro block (0,4,8,12).
************************************************************************
*/  //For residual DPCM
void Inv_Residual_trans_4x4(ImageParameters *img, //!< image parameters
                            ColorPlane pl,                            
                            int ioff,            //!< index to 4x4 block
                            int joff)
{
  int i,j;
  int temp[4][4];
  imgpel (*mpr)[MB_BLOCK_SIZE] = img->mb_pred[pl];
  int    (*m7) [MB_BLOCK_SIZE] = img->mb_rres[pl];
  int    (*cof)[MB_BLOCK_SIZE] = img->cof[pl];

  if(ipmode_DPCM==VERT_PRED)
  {
    for(i=0; i<4; i++)
    {
      temp[0][i] = cof[joff + 0][ioff + i];
      temp[1][i] = cof[joff + 1][ioff + i] + temp[0][i];
      temp[2][i] = cof[joff + 2][ioff + i] + temp[1][i];
      temp[3][i] = cof[joff + 3][ioff + i] + temp[2][i];
    }
    for(i=0; i<4; i++)
    {
      m7[joff    ][ioff + i]=temp[0][i];
      m7[joff + 1][ioff + i]=temp[1][i];
      m7[joff + 2][ioff + i]=temp[2][i];
      m7[joff + 3][ioff + i]=temp[3][i];
    }
  }
  else if(ipmode_DPCM==HOR_PRED)
  {
    for(j=0; j<4; j++)
    {
      temp[j][0] = cof[joff + j][ioff    ];
      temp[j][1] = cof[joff + j][ioff + 1] + temp[j][0];
      temp[j][2] = cof[joff + j][ioff + 2] + temp[j][1];
      temp[j][3] = cof[joff + j][ioff + 3] + temp[j][2];
    }
    for(j=0; j<4; j++)
    {
      m7[joff + j][ioff    ]=temp[j][0];
      m7[joff + j][ioff + 1]=temp[j][1];
      m7[joff + j][ioff + 2]=temp[j][2];
      m7[joff + j][ioff + 3]=temp[j][3];
    }
  }
  else
  {
    for (j = 0; j < BLOCK_SIZE; j++)
      for (i = 0; i < BLOCK_SIZE; i++)
        m7[joff+j][ioff+i] = cof[joff+j][ioff+i];
  }
  for (j = 0; j < BLOCK_SIZE; j++)
    for (i = 0; i < BLOCK_SIZE; i++)
      m7[joff+j][ioff+i] +=  (long)mpr[joff+j][ioff+i];

}

/*!
************************************************************************
* \brief
*    Inverse residual DPCM for Intra lossless coding
*
* \par Input:
*    ioff_x,joff_y: Block position inside a macro block (0,8).
************************************************************************
*/
//For residual DPCM
void Inv_Residual_trans_8x8(ImageParameters *img, Macroblock *currMB, ColorPlane pl, int ioff,int joff)
{
  int i, j;
  int temp[8][8];
  imgpel (*mpr)[16] = img->mb_pred[pl];
  int    (*m7) [16] = img->mb_rres[pl];
  

  if(ipmode_DPCM==VERT_PRED)
    {
      for(i=0; i<8; i++)
      {
        temp[0][i] = m7[joff + 0][ioff + i];
        temp[1][i] = m7[joff + 1][ioff + i] + temp[0][i];
        temp[2][i] = m7[joff + 2][ioff + i] + temp[1][i];
        temp[3][i] = m7[joff + 3][ioff + i] + temp[2][i];
        temp[4][i] = m7[joff + 4][ioff + i] + temp[3][i];
        temp[5][i] = m7[joff + 5][ioff + i] + temp[4][i];
        temp[6][i] = m7[joff + 6][ioff + i] + temp[5][i];
        temp[7][i] = m7[joff + 7][ioff + i] + temp[6][i];
      }
      for(i=0; i<8; i++)
      {
        m7[joff  ][ioff+i]=temp[0][i];
        m7[joff+1][ioff+i]=temp[1][i];
        m7[joff+2][ioff+i]=temp[2][i];
        m7[joff+3][ioff+i]=temp[3][i];
        m7[joff+4][ioff+i]=temp[4][i];
        m7[joff+5][ioff+i]=temp[5][i];
        m7[joff+6][ioff+i]=temp[6][i];
        m7[joff+7][ioff+i]=temp[7][i];
      }
    }
    else if(ipmode_DPCM==HOR_PRED)//HOR_PRED
    {
      for(i=0; i<8; i++)
      {
        temp[i][0] = m7[joff + i][ioff + 0];
        temp[i][1] = m7[joff + i][ioff + 1] + temp[i][0];
        temp[i][2] = m7[joff + i][ioff + 2] + temp[i][1];
        temp[i][3] = m7[joff + i][ioff + 3] + temp[i][2];
        temp[i][4] = m7[joff + i][ioff + 4] + temp[i][3];
        temp[i][5] = m7[joff + i][ioff + 5] + temp[i][4];
        temp[i][6] = m7[joff + i][ioff + 6] + temp[i][5];
        temp[i][7] = m7[joff + i][ioff + 7] + temp[i][6];
      }
      for(i=0; i<8; i++)
      {
        m7[joff+i][ioff+0]=temp[i][0];
        m7[joff+i][ioff+1]=temp[i][1];
        m7[joff+i][ioff+2]=temp[i][2];
        m7[joff+i][ioff+3]=temp[i][3];
        m7[joff+i][ioff+4]=temp[i][4];
        m7[joff+i][ioff+5]=temp[i][5];
        m7[joff+i][ioff+6]=temp[i][6];
        m7[joff+i][ioff+7]=temp[i][7];
      }
    }
    for (j = 0; j < BLOCK_SIZE*2; j++)
      for (i = 0; i < BLOCK_SIZE*2; i++)
        m7[j+joff][i+ioff] +=  (long)mpr[j+joff][i+ioff];
}



/*!
 ***********************************************************************
 * \brief
 *    Luma DC inverse transform
 ***********************************************************************
 */ 
void itrans_2(ImageParameters *img, Macroblock *currMB, ColorPlane pl) //!< image parameters
{
  int i,j;

  int (*cof)[16] = img->cof[pl];
  int qp_scaled = currMB->qp_scaled[pl];

  int qp_per = qp_per_matrix[ qp_scaled ];
  int qp_rem = qp_rem_matrix[ qp_scaled ];    

  int invLevelScale = InvLevelScale4x4_Intra[pl][qp_rem][0][0];

  // horizontal
  for (j=0; j < 4;j++) 
  {
    for (i=0; i < 4;i++) 
    {
      M4[j][i]=cof[j<<2][i<<2];
    }
  }

  ihadamard4x4(M4, M4);

  // vertical
  for (j=0; j < 4;j++) 
  {
    for (i=0; i < 4;i++) 
    {
      cof[j<<2][i<<2] = rshift_rnd((( M4[j][i] * invLevelScale) << qp_per), 6);
    }
  }
}


void itrans_sp(ImageParameters *img,  //!< image parameters
               ColorPlane pl, 
               int ioff,             //!< index to 4x4 block
               int joff)              //!<
{
  int i,j;  
  int ilev, icof;

  int qp = (img->type == SI_SLICE) ? img->qpsp : img->qp;
  int qp_per = qp_per_matrix[ qp ];
  int qp_rem = qp_rem_matrix[ qp ];

  int qp_per_sp = qp_per_matrix[ img->qpsp ];
  int qp_rem_sp = qp_rem_matrix[ img->qpsp ];
  int q_bits_sp = Q_BITS + qp_per_sp;

  imgpel (*mpr)[16] = img->mb_pred[pl];
  int    (*m7) [16] = img->mb_rres[pl];
  int    (*cof)[16] = img->cof[pl];
  int max_imgpel_value = img->max_imgpel_value_comp[pl];
  
  //int (*InvLevelScale4x4)[4] = InvLevelScale4x4_Intra[0][qp_rem];
  const int (*InvLevelScale4x4)  [4] = dequant_coef[qp_rem];
  const int (*InvLevelScale4x4SP)[4] = dequant_coef[qp_rem_sp];


  for (j=0; j< BLOCK_SIZE; j++)
    for (i=0; i< BLOCK_SIZE; i++)
      PBlock[j][i] = mpr[j+joff][i+ioff];

  forward4x4(PBlock, PBlock, 0, 0);

  if(img->sp_switch || img->type==SI_SLICE)
  {    
    for (j=0;j<BLOCK_SIZE;j++)
    {
      for (i=0;i<BLOCK_SIZE;i++)
      {
        // recovering coefficient since they are already dequantized earlier
        icof = (cof[joff + j][ioff + i] >> qp_per) / InvLevelScale4x4[j][i];
        //icof = ((cof[joff + j][ioff + i] * quant_coef[qp_rem][j][i])>> (qp_per + 15)) ;
        // icof  = rshift_rnd_sf(cof[joff + j][ioff + i] * quant_coef[qp_rem][j][i], qp_per + 15);
        ilev  = rshift_rnd_sf(iabs(PBlock[j][i]) * quant_coef[qp_rem_sp][j][i], q_bits_sp);
        ilev  = isignab(ilev, PBlock[j][i]) + icof;
        cof[joff + j][ioff + i] = ilev * InvLevelScale4x4SP[j][i] << qp_per_sp;
      }
    }
  }
  else
  {
    for (j=0;j<BLOCK_SIZE;j++)
    {
      for (i=0;i<BLOCK_SIZE;i++)
      {
        // recovering coefficient since they are already dequantized earlier
        icof = (cof[joff + j][ioff + i] >> qp_per) / InvLevelScale4x4[j][i];
        //icof = cof[joff + j][ioff + i];
        //icof  = rshift_rnd_sf(cof[joff + j][ioff + i] * quant_coef[qp_rem][j][i], qp_per + 15);
        ilev = PBlock[j][i] + ((icof * InvLevelScale4x4[j][i] * A[j][i] <<  qp_per) >> 6);
        ilev  = isign(ilev) * rshift_rnd_sf(iabs(ilev) * quant_coef[qp_rem_sp][j][i], q_bits_sp);
        //cof[joff + j][ioff + i] = ilev * InvLevelScale4x4SP[j][i] << qp_per_sp;
        cof[joff + j][ioff + i] = ilev * dequant_coef[qp_rem_sp][j][i] << qp_per_sp;
      }
    }
  }

  inverse4x4(cof, m7, joff, ioff);

  for (j=0;j<BLOCK_SIZE;j++)
    for (i=0;i<BLOCK_SIZE;i++)
      m7[j + joff][i + ioff] = iClip1(max_imgpel_value,rshift_rnd_sf(m7[j + joff][i + ioff], DQ_BITS));
}


void itrans_sp_cr(ImageParameters *img, int uv)
{
  int i,j,ilev, icof, n2,n1;
  int mp1[BLOCK_SIZE];
  int qp_per,qp_rem;
  int qp_per_sp,qp_rem_sp,q_bits_sp;
  imgpel (*mpr)[MB_BLOCK_SIZE] = img->mb_pred[uv + 1];
  int    (*cof)[MB_BLOCK_SIZE] = img->cof[uv + 1];

  qp_per    = qp_per_matrix[ ((img->qp < 0 ? img->qp : QP_SCALE_CR[img->qp]))];
  qp_rem    = qp_rem_matrix[ ((img->qp < 0 ? img->qp : QP_SCALE_CR[img->qp]))];

  qp_per_sp = qp_per_matrix[ ((img->qpsp < 0 ? img->qpsp : QP_SCALE_CR[img->qpsp]))];
  qp_rem_sp = qp_rem_matrix[ ((img->qpsp < 0 ? img->qpsp : QP_SCALE_CR[img->qpsp]))];
  q_bits_sp = Q_BITS + qp_per_sp;  

  if (img->type == SI_SLICE)
  {
    qp_per = qp_per_sp;
    qp_rem = qp_rem_sp;
  }

  for (j=0; j < img->mb_cr_size_y; j++)
  {
    for (i=0; i < img->mb_cr_size_x; i++)
    {
      PBlock[j][i] = mpr[j][i];
      mpr[j][i] = 0;
    }
  }

  for (n2=0; n2 < img->mb_cr_size_y; n2 += BLOCK_SIZE)
  {
    for (n1=0; n1 < img->mb_cr_size_x; n1 += BLOCK_SIZE)
    {
      forward4x4(PBlock, PBlock, n2, n1);
    }
  }

  //     2X2 transform of DC coeffs.
  mp1[0] = (PBlock[0][0] + PBlock[4][0] + PBlock[0][4] + PBlock[4][4]);
  mp1[1] = (PBlock[0][0] - PBlock[4][0] + PBlock[0][4] - PBlock[4][4]);
  mp1[2] = (PBlock[0][0] + PBlock[4][0] - PBlock[0][4] - PBlock[4][4]);
  mp1[3] = (PBlock[0][0] - PBlock[4][0] - PBlock[0][4] + PBlock[4][4]);

  if (img->sp_switch || img->type==SI_SLICE)  
  {        
    for (n2=0; n2 < 2; n2 ++)
    {
      for (n1=0; n1 < 2; n1 ++)
      {
        //quantization fo predicted block
        ilev = rshift_rnd_sf(iabs (mp1[n1+n2*2]) * quant_coef[qp_rem_sp][0][0], q_bits_sp + 1);
        //addition
        ilev = isignab(ilev, mp1[n1+n2*2]) + cof[n2<<2][n1<<2];
        //dequantization
        mp1[n1+n2*2] =ilev * dequant_coef[qp_rem_sp][0][0] << qp_per_sp;
      }
    }

    for (n2 = 0; n2 < img->mb_cr_size_y; n2 += BLOCK_SIZE)
    {
      for (n1 = 0; n1 < img->mb_cr_size_x; n1 += BLOCK_SIZE)
      {
        for (j = 0; j < BLOCK_SIZE; j++)
        {
          for (i = 0; i < BLOCK_SIZE; i++)
          {
            // recovering coefficient since they are already dequantized earlier
            cof[n2 + j][n1 + i] = (cof[n2 + j][n1 + i] >> qp_per) / dequant_coef[qp_rem][j][i];

            //quantization of the predicted block
            ilev = rshift_rnd_sf(iabs(PBlock[n2 + j][n1 + i]) * quant_coef[qp_rem_sp][j][i], q_bits_sp);
            //addition of the residual
            ilev = isignab(ilev,PBlock[n2 + j][n1 + i]) + cof[n2 + j][n1 + i];
            // Inverse quantization
            cof[n2 + j][n1 + i] = ilev * dequant_coef[qp_rem_sp][j][i] << qp_per_sp;
          }
        }
      }
    }
  }
  else
  {
    for (n2=0; n2 < 2; n2 ++)
    {
      for (n1=0; n1 < 2; n1 ++)
      {
        ilev = mp1[n1+n2*2] + (((cof[n2<<2][n1<<2] * dequant_coef[qp_rem][0][0] * A[0][0]) << qp_per) >> 5);
        ilev = isign(ilev) * rshift_rnd_sf(iabs(ilev) * quant_coef[qp_rem_sp][0][0], q_bits_sp + 1);
        //ilev = isignab(rshift_rnd_sf(iabs(ilev)* quant_coef[qp_rem_sp][0][0], q_bits_sp + 1), ilev);
        mp1[n1+n2*2] = ilev * dequant_coef[qp_rem_sp][0][0] << qp_per_sp;
      }
    }

    for (n2 = 0; n2 < img->mb_cr_size_y; n2 += BLOCK_SIZE)
    {
      for (n1 = 0; n1 < img->mb_cr_size_x; n1 += BLOCK_SIZE)
      {
        for (j = 0; j< BLOCK_SIZE; j++)
        {
          for (i = 0; i< BLOCK_SIZE; i++)
          {
            // recovering coefficient since they are already dequantized earlier
            //icof = ((((cof[n2 + j][n1 + i] << 4) + qp_per/2)>> qp_per) + dequant_coef[qp_rem][j][i]/2) / dequant_coef[qp_rem][j][i];
            icof = (cof[n2 + j][n1 + i] >> qp_per) / dequant_coef[qp_rem][j][i];
            //dequantization and addition of the predicted block      
            ilev = PBlock[n2 + j][n1 + i] + ((icof * dequant_coef[qp_rem][j][i] * A[j][i] << qp_per) >> 6);
            //quantization and dequantization
            ilev = isign(ilev) * rshift_rnd_sf(iabs(ilev) * quant_coef[qp_rem_sp][j][i], q_bits_sp);
            cof[n2 + j][n1 + i] = ilev * dequant_coef[qp_rem_sp][j][i] << qp_per_sp;
            //printf( " %d %d %d\n", j, i, quant_coef[qp_rem_sp][j][i]);
          }
        }
      }
    }
  }

  cof[0][0] = (mp1[0] + mp1[1] + mp1[2] + mp1[3]) >> 1;
  cof[0][4] = (mp1[0] + mp1[1] - mp1[2] - mp1[3]) >> 1;
  cof[4][0] = (mp1[0] - mp1[1] + mp1[2] - mp1[3]) >> 1;
  cof[4][4] = (mp1[0] - mp1[1] - mp1[2] + mp1[3]) >> 1;
}

static const byte decode_block_scan[16] = {0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15};

void iMBtrans4x4(ColorPlane pl, ImageParameters *img, int smb)
{
  int i_pos;
  int jj, ii;
  int block8x8;
  int i, j, k;  
  int    (*m7) [16] = img->mb_rres[pl];
  imgpel **curr_img = pl ? &(dec_picture->imgUV[pl - 1][img->pix_y]): &dec_picture->imgY[img->pix_y];

  //For residual DPCM
  Boolean lossless_qpprime = (Boolean) (((img->qp + img->bitdepth_luma_qp_scale) == 0) && (img->lossless_qpprime_flag == 1));  

  // =============== 4x4 itrans ================
  // -------------------------------------------

  itrans_4x4 = (smb) ? itrans_sp : ((!lossless_qpprime) ? itrans4x4 : Inv_Residual_trans_4x4);

  i_pos = img->pix_x;

  for (block8x8=0; block8x8 < MB_BLOCK_SIZE; block8x8 += 4)
  { 
    for (k = block8x8; k < block8x8 + 4; k ++)
    {
      jj = ((decode_block_scan[k] >> 2) & 3) << BLOCK_SHIFT;
      ii = (decode_block_scan[k] & 3) << BLOCK_SHIFT;

      itrans_4x4(img, pl, ii, jj);   // use DCT transform and make 4x4 block m7 from prediction block mpr
      
      for(j = jj; j < jj + BLOCK_SIZE; j++)
      {
        for(i = ii; i < ii + BLOCK_SIZE; i++)
        {
          curr_img[j][i_pos + i] = (imgpel) m7[j][i]; // construct picture from 4x4 blocks
        }
      }
    }
  }
}

void iMBtrans8x8(ImageParameters *img,  Macroblock *currMB, ColorPlane pl)
{
  imgpel **curr_img = pl ? &dec_picture->imgUV[pl - 1][img->pix_y]: &dec_picture->imgY[img->pix_y];
  int    (*m7)[MB_BLOCK_SIZE] = img->mb_rres[pl];
  int block8x8;
  int i,j;
  int ioff, joff;

  for (block8x8=0; block8x8<4; block8x8++)
  {
    // =============== 8x8 itrans ================
    // -------------------------------------------
    ioff = 8 * (block8x8 & 0x01);
    joff = 8 * (block8x8 >> 1);

    itrans8x8(img, currMB, pl, ioff, joff);      // use DCT transform and make 8x8 block m7 from prediction block mpr

    for(j = joff; j < joff + 8; j++)
    {
      for(i = ioff; i < ioff + 8; i++)
      {
        curr_img[j][img->pix_x + i] = (imgpel) m7[j][i];
      }
    }
  }
}

void iTransform(ImageParameters *img,  Macroblock *currMB, ColorPlane pl, int need_4x4_transform, int smb)
{
  static imgpel (*mpr) [16];
  static imgpel **curr_img;
  int j, uv = pl-1; 

  if ((currMB->cbp & 15) != 0 || smb)
  {
    if(need_4x4_transform) // 4x4 inverse transform
    {
      iMBtrans4x4(pl, img, smb); 
    }
    else // 8x8 inverse transform
    {  
      iMBtrans8x8(img, currMB, pl);    
    }
  }
  else
  {
    mpr = img->mb_pred[pl]; 
    curr_img = pl ? &dec_picture->imgUV[uv][img->pix_y] : &dec_picture->imgY[img->pix_y];
    for(j = 0; j < MB_BLOCK_SIZE; j++)
    { 
      memcpy(&(curr_img[j][img->pix_x]), &(img->mb_pred[pl][j][0]), MB_BLOCK_SIZE * sizeof(imgpel));
    }
  }

  if ((dec_picture->chroma_format_idc != YUV400) && (dec_picture->chroma_format_idc != YUV444)) 
  {
    static imgpel **curUV, *cur_line;
    static int b4, b8;
    static int ioff, joff, ii, jj;
    static int (*m7UV)[16], *m7;

    for(uv=0;uv<2;uv++)
    {
      Boolean lossless_qpprime = (Boolean) ((img->lossless_qpprime_flag == 1) &&((img->qp + dec_picture->chroma_qp_offset[uv] + img->bitdepth_chroma_qp_scale) == 0));  
      itrans_4x4 = (!lossless_qpprime) ? itrans4x4 : itrans4x4_ls;

      // =============== 4x4 itrans ================
      // -------------------------------------------
      curUV = &dec_picture->imgUV[uv][img->pix_c_y]; 
      m7UV  = img->mb_rres[uv+1];

      if (!smb && (currMB->cbp>>4))
      {
        for (b8 = 0; b8 < (img->num_uv_blocks); b8++)
        {
          for(b4 = 0; b4 < 4; b4++)
          {
            joff = subblk_offset_y[1][b8][b4];
            ioff = subblk_offset_x[1][b8][b4];

            itrans_4x4(img, (ColorPlane) (uv + 1), ioff, joff);

            for(jj=joff;jj<joff + 4;jj++)
            {
              cur_line = &curUV[jj][img->pix_c_x + ioff];
              m7 = &m7UV[jj][ioff];
              for(ii=0;ii<4;ii++)
              {
                *(cur_line++) = (imgpel) *(m7++);
              }
            }
          }
        }
      }
      else if (smb)
      {
        itrans_sp_cr(img, uv);

        for (joff = 0; joff < img->mb_cr_size_y; joff += BLOCK_SIZE)
        {
          for(ioff = 0; ioff < img->mb_cr_size_x ;ioff += BLOCK_SIZE)
          {
            itrans_4x4(img, (ColorPlane) (uv + 1), ioff, joff);

            for(jj=joff;jj<joff + 4;jj++)
              for(ii=ioff;ii<ioff + 4;ii++)
              {
                curUV[jj][img->pix_c_x+ii]= (imgpel) m7UV[jj][ii];
              }
          }
        }
      }
      else 
      {
        mpr = img->mb_pred[uv + 1];
        for(jj = 0; jj < img->mb_size[1][1]; jj++)
          memcpy(&(curUV[jj][img->pix_c_x]), &(mpr[jj][0]), img->mb_size[1][0] * sizeof(imgpel));
      }
    }
  }
}

