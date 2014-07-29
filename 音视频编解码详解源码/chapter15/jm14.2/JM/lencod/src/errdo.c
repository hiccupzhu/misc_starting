
/*!
 *************************************************************************************
 * \file errdo.c
 *
 * \brief
 *    Contains functions that implement the "decoders in the encoder" concept for the
 *    rate-distortion optimization with losses.
 * \date
 *    October 22nd, 2001
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and
 *    affiliation details)
 *    - Dimitrios Kontopodis                    <dkonto@eikon.tum.de>
 *    Code revamped July 2008 by:
 *    - Peshala Pahalawatta (ppaha@dolby.com)
 *************************************************************************************
 */

#include "global.h"
#include "refbuf.h"
#include "image.h"
#include "errdo.h"
#include "errdo_mc_prediction.h"

static StorablePicture* find_nearest_ref_picture(int poc);
static void copy_conceal_mb (StorablePicture *enc_pic, ImageParameters* image, int decoder, int mb_error, Macroblock* currMB, StorablePicture* refPic);
static void get_predicted_mb(StorablePicture *enc_pic, ImageParameters* image, int decoder, Macroblock* currMB);
static void add_residue     (StorablePicture *enc_pic, int decoder, int pl, int block8x8, int x_size, int y_size);
static void Build_Status_Map(byte **s_map);

extern void DeblockFrame(ImageParameters *img, imgpel **, imgpel ***);

/*!
**************************************************************************************
* \brief 
*      Decodes one macroblock for error resilient RDO.  
*    Currently does not support:
*    1) B coded pictures
*    2) Chroma components
*    3) Potential error propagation due to intra prediction
*    4) Field coding
**************************************************************************************
*/
void decode_one_mb (ImageParameters *image, StorablePicture *enc_pic, int decoder, Macroblock* currMB)
{
  int i0, j;
  static imgpel** curComp;
  static imgpel** oldComp;

  if (currMB->mb_type > P8x8) //Intra MB
  {
    curComp = &enc_pic->p_dec_img[0][decoder][image->pix_y];
    oldComp = &enc_pic->p_curr_img[image->pix_y];
    i0 = image->pix_x;
    for (j = 0; j < MB_BLOCK_SIZE; j++)
    {
      memcpy(&(curComp[j][i0]), &(oldComp[j][i0]), MB_BLOCK_SIZE*sizeof(imgpel));
    }
  }
  else if (currMB->mb_type == 0)
  {
    get_predicted_mb(enc_pic, image, decoder, currMB);
    curComp = &enc_pic->p_dec_img[0][decoder][image->pix_y];
    for(j = 0; j < image->mb_size[0][1]; j++)
    {                
      memcpy(&(curComp[j][image->pix_x]), &(image->mb_pred[0][j][0]), image->mb_size[0][0] * sizeof(imgpel));
    }
  }
  else 
  {
    get_predicted_mb(enc_pic, image, decoder, currMB);
    add_residue(enc_pic, decoder, PLANE_Y, 0, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
  }
}

/*!
**************************************************************************************
* \brief 
*      Finds predicted macroblock values 
*   and copies them to img->mb_pred[0][][]
*   Requires img->all_mv, enc_picture->motion.ref_idx to be correct for current
*   macroblock.
**************************************************************************************
*/
static void get_predicted_mb(StorablePicture *enc_pic, ImageParameters* image, int decoder, Macroblock* currMB)
{
  int i,j,k;
  int block_size_x, block_size_y;
  int mv_mode, pred_dir;
  int list_offset   = 0; //For now
  int curr_mb_field = 0; //For now
  static const byte decode_block_scan[16] = {0,1,4,5,2,3,6,7,8,9,12,13,10,11,14,15};
  int block8x8;
  int k_start, k_end, k_inc;

  if (!currMB->mb_type)
  {
    block_size_x = MB_BLOCK_SIZE;
    block_size_y = MB_BLOCK_SIZE;
    pred_dir = LIST_0;

    perform_mc(decoder, PLANE_Y, enc_pic, image, pred_dir, 0, 0, 0, 0, list_offset, block_size_x, block_size_y, curr_mb_field);
  }
  else if (currMB->mb_type == 1)
  {
    block_size_x = MB_BLOCK_SIZE;
    block_size_y = MB_BLOCK_SIZE;
    pred_dir = currMB->b8pdir[0];   

    perform_mc(decoder, PLANE_Y, enc_pic, image, pred_dir, 1, 0, 0, 0, list_offset, block_size_x, block_size_y, curr_mb_field);
  }
  else if (currMB->mb_type == 2)
  {   
    block_size_x = MB_BLOCK_SIZE;
    block_size_y = 8;    

    for (block8x8 = 0; block8x8 < 4; block8x8 += 2)
    {
      pred_dir = currMB->b8pdir[block8x8];
      perform_mc(decoder, PLANE_Y, enc_pic, image, pred_dir, 2, 0, 0, block8x8, list_offset, block_size_x, block_size_y, curr_mb_field);
    }
  }
  else if (currMB->mb_type == 3)
  {   
    block_size_x = 8;
    block_size_y = 16;

    for (block8x8 = 0; block8x8 < 2; block8x8++)
    {
      i = block8x8<<1;
      j = 0;      
      pred_dir = currMB->b8pdir[block8x8];
      assert (pred_dir<=2);
      perform_mc(decoder, PLANE_Y, enc_pic, image, pred_dir, 3, 0, i, j, list_offset, block_size_x, block_size_y, curr_mb_field);
    }
  }
  else //Need to change to support B slices.
  {
    for (block8x8 = 0; block8x8 < 4; block8x8++)
    {
      mv_mode  = currMB->b8mode[block8x8];
      pred_dir = currMB->b8pdir[block8x8];

      if ( mv_mode != 0 )
      {
        k_start = (block8x8 << 2);
        k_inc = (mv_mode == 5) ? 2 : 1;
        k_end = (mv_mode == 4) ? k_start + 1 : ((mv_mode == 7) ? k_start + 4 : k_start + k_inc + 1);

        block_size_x = ( mv_mode == 5 || mv_mode == 4 ) ? 8 : 4;
        block_size_y = ( mv_mode == 6 || mv_mode == 4 ) ? 8 : 4;

        for (k = k_start; k < k_end; k += k_inc)
        {
          i =  (decode_block_scan[k] & 3);
          j = ((decode_block_scan[k] >> 2) & 3);
          perform_mc(decoder, PLANE_Y, enc_pic, image, pred_dir, mv_mode, 0, i, j, list_offset, block_size_x, block_size_y, curr_mb_field);
        }        
      }
    }
  }
}


/*!
**************************************************************************************
* \brief 
*      Decodes one 8x8 partition for error resilient RDO.  
*    Currently does not support:
*    1) B coded pictures
*    2) Chroma components
*    3) Potential error propagation due to intra prediction
*    4) Field coding
**************************************************************************************
*/
void decode_one_b8block (ImageParameters *image, StorablePicture *enc_pic, int decoder, int mbmode, int block8x8, short mv_mode, short b8ref) //b8ref may not be necessary any more.
{
  int i,j,k;
  int block_size_x, block_size_y;
  int i0 = (block8x8 & 0x01)<<3;
  int j0 = (block8x8 >> 1)<<3,   j1 = j0+8;
  int list_offset = 0;
  int curr_mb_field = 0;
  imgpel **curComp;
  imgpel **oldComp;
  int pred_dir;
  int k_start, k_end, k_inc;
  static const byte decode_block_scan[16] = {0,1,4,5,2,3,6,7,8,9,12,13,10,11,14,15};

  if (mv_mode > 8)  //Intra
  {
    for(j = j0; j < j1; j++)
    {
      curComp = &enc_pic->p_dec_img[0][decoder][image->pix_y];
      oldComp = &enc_pic->p_curr_img[image->pix_y];
      memcpy(&(curComp[j][i0]), &(oldComp[j][i0]), sizeof(imgpel)*8);
    }
  }
  else
  {
    pred_dir = 0;

    k_start = (block8x8 << 2);
    k_inc = (mv_mode == 5) ? 2 : 1;
    k_end = (mv_mode == 4) ? k_start + 1 : ((mv_mode == 7) ? k_start + 4 : k_start + k_inc + 1);

    block_size_x = ( mv_mode == 5 || mv_mode == 4 ) ? 8 : 4;
    block_size_y = ( mv_mode == 6 || mv_mode == 4 ) ? 8 : 4;

    for (k = k_start; k < k_end; k += k_inc)
    {
      i =  (decode_block_scan[k] & 3);
      j = ((decode_block_scan[k] >> 2) & 3);
      perform_mc(decoder, PLANE_Y, enc_pic, image, pred_dir, mv_mode, 0, i, j, list_offset, block_size_x, block_size_y, curr_mb_field);
    }        

    add_residue(enc_pic, decoder, PLANE_Y, block8x8, 8, 8);
  }
}

/*!
**************************************************************************************
* \brief 
*      Add residual to motion predicted block
**************************************************************************************
*/
static void add_residue (StorablePicture *enc_pic, int decoder, int pl, int block8x8, int x_size, int y_size) 
{
  int i,j;
  int i0 = (block8x8 & 0x01)<<3, i1 = i0 + x_size;
  int j0 = (block8x8 >> 1)<<3,   j1 = j0 + y_size;

  imgpel **p_dec_img  = &enc_pic->p_dec_img[pl][decoder][img->pix_y];
  int (*res_img)[16]  = decs->res_img[0];
  imgpel (*mpr)[16]   = img->mb_pred[pl];


  for (j = j0; j < j1; j++)
  {
    for (i = i0; i < i1; i++)
    {
      p_dec_img[j][img->pix_x+i] = iClip3(0, img->max_imgpel_value_comp[pl], (mpr[j][i] + res_img[j][i])); 
    } 
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Performs the simulation of the packet losses, calls the error concealment funcs
 *    and copies the decoded images to the reference frame buffers of the decoders
 *
 *************************************************************************************
 */
void UpdateDecoders(InputParameters *params, ImageParameters *image, StorablePicture *enc_pic)
{
  int k;
  for (k = 0; k < params->NoOfDecoders; k++)
  {
    Build_Status_Map(enc_pic->mb_error_map[k]); // simulates the packet losses
    error_conceal_picture(image, enc_pic, k); 
    DeblockFrame (image, enc_pic->p_dec_img[0][k], NULL);
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Initialize error concealment function
 *    (Currently only copy concealment is implemented. Can extend to other concealment
 *    types when available.)
 *
 *************************************************************************************
 */
void init_error_conceal(int concealment_type)
{
  error_conceal_picture = copy_conceal_picture;
}

/*!
**************************************************************************************
* \brief 
*      Finds predicted macroblock values for error concealment
*   and copies them to img->mb_pred[0][][]
*   Requires enc_picture->motion.mv and enc_picture->motion.ref_idx to be correct for 
*   current picture.
**************************************************************************************
*/
static void get_predicted_concealment_mb(StorablePicture* enc_pic, ImageParameters* image, int decoder, Macroblock* currMB)
{
  int i,j,k;
  int block_size_x, block_size_y;
  int mv_mode, pred_dir;
  static const byte decode_block_scan[16] = {0,1,4,5,2,3,6,7,8,9,12,13,10,11,14,15};
  int block8x8;
  int k_start, k_end, k_inc;

  if (!currMB->mb_type)
  {
    block_size_x = MB_BLOCK_SIZE;
    block_size_y = MB_BLOCK_SIZE;
    pred_dir = LIST_0;

    perform_mc_concealment(decoder, PLANE_Y, enc_pic, image, pred_dir, 0, 0, block_size_x, block_size_y);
  }
  else if (currMB->mb_type == 1)
  {
    block_size_x = MB_BLOCK_SIZE;
    block_size_y = MB_BLOCK_SIZE;
    pred_dir = currMB->b8pdir[0];   

    perform_mc_concealment(decoder, PLANE_Y, enc_pic, image, pred_dir, 0, 0, block_size_x, block_size_y);
  }
  else if (currMB->mb_type == 2)
  {   
    block_size_x = MB_BLOCK_SIZE;
    block_size_y = 8;    

    for (block8x8 = 0; block8x8 < 4; block8x8 += 2)
    {
      pred_dir = currMB->b8pdir[block8x8];
      perform_mc_concealment(decoder, PLANE_Y, enc_pic, image, pred_dir, 0, block8x8, block_size_x, block_size_y);
    }
  }
  else if (currMB->mb_type == 3)
  {   
    block_size_x = 8;
    block_size_y = 16;

    for (block8x8 = 0; block8x8 < 2; block8x8++)
    {
      i = block8x8<<1;
      j = 0;      
      pred_dir = currMB->b8pdir[block8x8];
      assert (pred_dir<=2);
      perform_mc_concealment(decoder, PLANE_Y, enc_pic, image, pred_dir, i, j, block_size_x, block_size_y);
    }
  }
  else //Need to change to support B slices.
  {
    for (block8x8 = 0; block8x8 < 4; block8x8++)
    {
      mv_mode  = currMB->b8mode[block8x8];
      pred_dir = currMB->b8pdir[block8x8];

      if ( mv_mode != 0 )
      {
        k_start = (block8x8 << 2);
        k_inc = (mv_mode == 5) ? 2 : 1;
        k_end = (mv_mode == 4) ? k_start + 1 : ((mv_mode == 7) ? k_start + 4 : k_start + k_inc + 1);

        block_size_x = ( mv_mode == 5 || mv_mode == 4 ) ? 8 : 4;
        block_size_y = ( mv_mode == 6 || mv_mode == 4 ) ? 8 : 4;

        for (k = k_start; k < k_end; k += k_inc)
        {
          i =  (decode_block_scan[k] & 3);
          j = ((decode_block_scan[k] >> 2) & 3);
          perform_mc_concealment(decoder, PLANE_Y, enc_pic, image, pred_dir, i, j, block_size_x, block_size_y);
        }        
      }
    }
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Performs copy error concealment for macroblocks with errors.
 *  Note: Currently assumes that the reference picture lists remain the same for all 
 *        slices of a picture. 
 *  
 *************************************************************************************
 */
void copy_conceal_picture(ImageParameters *image, StorablePicture *enc_pic, int decoder)
{
  unsigned int mb;
  Macroblock* currMB;
  int mb_error;
  byte** mb_error_map = enc_pic->mb_error_map[decoder];
  StorablePicture* refPic;

  refPic = find_nearest_ref_picture(enc_pic->poc); //Used for concealment if actual reference pic is not known.

  for (mb = 0; mb < image->PicSizeInMbs; mb++)
  {
    image->mb_x = PicPos[mb][0];
    image->mb_y = PicPos[mb][1];
    mb_error = mb_error_map[image->mb_y][image->mb_x];
    if (mb_error)
    {
      currMB = &image->mb_data[mb];
      image->block_x = image->mb_x << 2;
      image->block_y = image->mb_y << 2;
      image->pix_x   = image->block_x << 2;
      image->pix_y   = image->block_y << 2;
      copy_conceal_mb(enc_pic, image, decoder, mb_error, currMB, refPic);
    }
  }
}

/******************************************************************************************
*
* Perform copy error concealment for macroblock.
*   
*******************************************************************************************
*/
static void copy_conceal_mb(StorablePicture *enc_pic, ImageParameters* image, int decoder, int mb_error, Macroblock* currMB, StorablePicture* refPic)
{
  int j, i0 = img->pix_x;
  imgpel** concealed_img = &(enc_pic->p_dec_img[0][decoder][image->pix_y]);
  imgpel** ref_img;

  if (mb_error == 1 || (mb_error != 3 && currMB->mb_type > P8x8)) //All partitions lost, or intra mb lost
  {
    if (refPic != NULL) //Use nearest reference picture for concealment
    {
      ref_img = &(refPic->p_dec_img[0][decoder][img->pix_y]);
      for (j = 0; j < MB_BLOCK_SIZE; j++)
      {
        memcpy(&(concealed_img[j][i0]), &(ref_img[j][i0]), sizeof(imgpel)*MB_BLOCK_SIZE);
      }
    }
    else //No ref picture available
    {
      for (j = 0; j < MB_BLOCK_SIZE; j++)
      {
        memset(&(concealed_img[j][i0]), 128, sizeof(imgpel)*MB_BLOCK_SIZE); //Only reliable if sizeof(imgpel) = 1
      }
    }
  }
  else if (mb_error != 2 && image->type == P_SLICE && currMB->mb_type && currMB->mb_type < P8x8) //Only partition 3 lost, and P macroblock and not skip
  {
    get_predicted_concealment_mb(enc_pic, image, decoder, currMB);
    for(j = 0; j < MB_BLOCK_SIZE; j++)
    {                
      memcpy(&(concealed_img[j][i0]), &(image->mb_pred[j][0]), MB_BLOCK_SIZE * sizeof(imgpel));
    }
  }
}

/******************************************************************************************
*
*  Finds reference picture with nearest POC to current picture to use for error concealment
*   
*******************************************************************************************
*/
static StorablePicture* find_nearest_ref_picture(int poc)
{
  unsigned int i;
  int min_poc_diff = 1000;
  int poc_diff;
  StorablePicture* refPic = NULL;

  for (i = 0; i < dpb.ref_frames_in_buffer; i++)
  {
    if (dpb.fs_ref[i]->is_used==3)
    {
      if ((dpb.fs_ref[i]->frame->used_for_reference)&&(!dpb.fs_ref[i]->frame->is_long_term))
      {
        poc_diff = iabs(dpb.fs_ref[i]->frame->poc - poc);
        if (poc_diff < min_poc_diff)
        {
          refPic = dpb.fs_ref[i]->frame;
          min_poc_diff = poc_diff;
        }
      }
    }
  }
  return refPic;
}

/*!
 *************************************************************************************
 * \brief
 *    Gives the prediction residue for a block
 *************************************************************************************
 */
void compute_residue_block (ImageParameters *image, imgpel **imgY, int res_img[16][16], imgpel mb_pred[16][16], int b8block, int block_size) 
{
  int i,j;
  int i0 = (b8block & 0x01)<<3,   i1 = i0+block_size;
  int j0 = (b8block >> 1)<<3,     j1 = j0+block_size;
  //imgpel  (*mb_pred)[16]        = (i16mode >= 0) ? image->mpr_16x16[0][i16mode] : image->mb_pred[0];;

  for (i = i0; i < i1; i++)
  {
    for (j = j0; j < j1; j++)
    {
      res_img[j][i] = (int)imgY[j][image->pix_x + i] - mb_pred[j][i];
    } 
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Stores the pel values for the current best mode.
 *************************************************************************************
 */
void errdo_store_best_block(ImageParameters* image, imgpel*** mbY, imgpel*** dec_img, int i0, int j0, int block_size)
{
  int j, k;
  int i = image->pix_x + i0;
  int j1 = j0 + block_size;
  
  for (k = 0; k < params->NoOfDecoders; k++)
  {
    for (j = j0; j < j1; j++)
    {
      memcpy(&mbY[k][j][i0], &dec_img[k][image->pix_y + j][i], block_size * sizeof(imgpel));
    }
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Restores the pel values from the current best 8x8 mode.
 *************************************************************************************
 */
void errdo_get_best_block(ImageParameters* image, imgpel*** dec_img, imgpel*** mbY, int j0, int block_size)
{
  int j, k;
  int j1 = j0 + block_size;

  for (k = 0; k < params->NoOfDecoders; k++)
  {
    for (j = j0; j < j1; j++)
    {
      memcpy(&dec_img[k][image->pix_y + j][image->pix_x], mbY[k][j], block_size * sizeof(imgpel));
    }
  }
}

/*!
 *************************************************************************************
 * \brief
 *    Builds a random status map showing whether each MB is received or lost, based
 *    on the packet loss rate and the slice structure.
 *
 * \param s_map
 *    The status map to be filled
 *************************************************************************************
 */
static void Build_Status_Map(byte **s_map)
{
  int i,j,slice=-1,mb=0,jj,ii,packet_lost=0;

  jj = img->height/MB_BLOCK_SIZE;
  ii = img->width/MB_BLOCK_SIZE;

  for (j = 0; j < jj; j++)
  {
    for (i = 0; i < ii; i++)
    {
      if (!params->slice_mode || img->mb_data[mb].slice_nr != slice) /* new slice */
      {
        packet_lost=0;
        if ((double)rand()/(double)RAND_MAX*100 < params->LossRateC)   packet_lost += 3;
        if ((double)rand()/(double)RAND_MAX*100 < params->LossRateB)   packet_lost += 2;
        if ((double)rand()/(double)RAND_MAX*100 < params->LossRateA)   packet_lost  = 1;
        slice++;
      }
      if (!packet_lost)
      {
        s_map[j][i]=0;  //! Packet OK
      }
      else
      {
        s_map[j][i]=packet_lost;
        if(params->partition_mode == 0)  s_map[j][i]=1;
      }
      mb++;
    }
  }
}

