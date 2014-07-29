
/*!
*************************************************************************************
* \file weighted_prediction.c
*
* \brief
*    Estimate weights for WP using DC method
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*     - Alexis Michael Tourapis         <alexismt@ieee.org>
*     - Athanasios Leontaris            <aleon@dolby.com>
*************************************************************************************
*/
#include "contributors.h"

#include "global.h"
#include "image.h"
#include "wp.h"

/*!
************************************************************************
* \brief
*    Initialize weighting parameter functions
************************************************************************
*/
void InitWP(InputParameters *params)
{
  switch ( params->WPMethod )
  {
  default:
  case 0:
    EstimateWPPSlice = EstimateWPPSliceAlg0;
    EstimateWPBSlice = EstimateWPBSliceAlg0;
    TestWPPSlice = TestWPPSliceAlg0;
    TestWPBSlice = TestWPBSliceAlg0;
    break;
  case 1:
    EstimateWPPSlice = EstimateWPPSliceAlg1;
    EstimateWPBSlice = EstimateWPBSliceAlg1;
    TestWPPSlice = TestWPPSliceAlg1;
    TestWPBSlice = TestWPBSliceAlg1;
    break;
  }
}

/*!
************************************************************************
* \brief
*    Compute sum of samples in a picture
************************************************************************
*/
double ComputeImgSum(imgpel **CurrentImage, int height, int width)
{
  int i, j;
  double sum_value = 0.0;

  for (i = 0; i < height; i++)
  {
    for (j = 0; j < width; j++)
    {
      sum_value += (double) CurrentImage[i][j];
    }
  }
  return sum_value;
}

/*!
************************************************************************
* \brief
*    Estimates reference picture weighting factors for P slices
************************************************************************
*/

void EstimateWPPSliceAlg0(ImageParameters *img, InputParameters *params, int select_offset)
{
  double dc_org = 0.0;
  double dc_org_UV[2] = {0.0};
  double dc_ref[MAX_REFERENCE_PICTURES] = { 0.0 };
  double dc_ref_UV[MAX_REFERENCE_PICTURES][2] = { {0.0}};

  int i, n, k;
  int default_weight[3];
  int list_offset   = ((img->MbaffFrameFlag)&&(img->mb_data[img->current_mb_nr].mb_field))? (img->current_mb_nr & 0x01) ? 4 : 2 : 0;
  int weight[2][MAX_REFERENCE_PICTURES][3];
  int offset[2][MAX_REFERENCE_PICTURES][3];
  int clist;

  imgpel **tmpPtr;

  luma_log_weight_denom   = 5;
  chroma_log_weight_denom = 5;

  wp_luma_round           = 1 << (luma_log_weight_denom - 1);
  wp_chroma_round         = 1 << (chroma_log_weight_denom - 1);
  default_weight[0]       = 1 << luma_log_weight_denom;
  default_weight[1]       = default_weight[2] = 1 << chroma_log_weight_denom;
  
  dc_org = ComputeImgSum(pCurImg, img->height, img->width);

  if (params->ChromaWeightSupport == 1)
  {
    for (k = 0; k < 2; k++)
    {
      dc_org_UV[k] = ComputeImgSum(pImgOrg[k + 1], img->height_cr, img->width_cr);
    } 
  }

  for (clist = 0; clist < 2 + list_offset; clist++)
  {
    for (n = 0; n < listXsize[clist]; n++)
    {
      if ( wpxDetermineWP( params, img, clist, n ) )
      {
        /* set all values to defaults */
        for (i = 0; i < 3; i++)
        {
          weight[clist][n][i]    = default_weight[i];
          wp_weight[clist][n][i] = default_weight[i];
          wp_offset[clist][n][i] = 0;
          offset[clist][n][i]    = 0;
        }

        // Y
        tmpPtr = listX[clist][n]->p_curr_img;      
        dc_ref[n] = ComputeImgSum(tmpPtr, img->height, img->width);

        if (params->ChromaWeightSupport == 1)
        {
          for (k = 0; k < 2; k++)
          {
            // UV
            tmpPtr = listX[clist][n]->imgUV[k];
            dc_ref_UV[n][k] = ComputeImgSum(tmpPtr, img->height_cr, img->width_cr);
          }        
        }

        if (select_offset == 0)
        {
          if (dc_ref[n] != 0.0)
            weight[clist][n][0] = (int) (default_weight[0] * dc_org / dc_ref[n] + 0.5);
          else
            weight[clist][n][0] = default_weight[0];  // only used when reference picture is black
          weight[clist][n][0] = iClip3(-128, 127, weight[clist][n][0]);
          if (params->ChromaWeightSupport == 1)
          {
            if (dc_ref_UV[n][0] != 0)
              weight[clist][n][1] = (int) (default_weight[1] * dc_org_UV[0] / dc_ref_UV[n][0] + 0.5);
            else
              weight[clist][n][1] = default_weight[1];  // only used when reference picture is black
            weight[clist][n][1] = iClip3(-128, 127, weight[clist][n][1]);

            if (dc_ref_UV[n][1] != 0)
              weight[clist][n][2] = (int) (default_weight[2] * dc_org_UV[1] / dc_ref_UV[n][1] + 0.5);
            else
              weight[clist][n][2] = default_weight[2];  // only used when reference picture is black
            weight[clist][n][2] = iClip3(-64, 128, weight[clist][n][2]);
          }
        }
        else
        {
          offset[clist][n][0] = (int) ((dc_org - dc_ref[n])/(img->size)+0.5);
          offset[clist][n][0] = (offset[clist][n][0]+((img->bitdepth_luma-8)>>1))>>(img->bitdepth_luma-8);
          offset[clist][n][0] = iClip3( -128, 127, offset[clist][n][0]);
          offset[clist][n][0] = offset[clist][n][0]<<(img->bitdepth_luma-8);
          weight[clist][n][0] = default_weight[0];

          if (params->ChromaWeightSupport == 1)
          {
            offset[clist][n][1] = (int) ((dc_org_UV[0] - dc_ref_UV[n][0])/(img->size_cr)+0.5);
            offset[clist][n][1] = (offset[clist][n][1] + ((img->bitdepth_chroma - 8)>>1))>>(img->bitdepth_chroma-8);
            offset[clist][n][1] = iClip3( -128, 127, offset[clist][n][1]);
            offset[clist][n][1] = offset[clist][n][1]<<(img->bitdepth_chroma - 8);
            
            weight[clist][n][1] = default_weight[1];

            offset[clist][n][2] = (int) ((dc_org_UV[1] - dc_ref_UV[n][1])/(img->size_cr)+0.5);
            offset[clist][n][2] = (offset[clist][n][2] + ((img->bitdepth_chroma - 8)>>1))>>(img->bitdepth_chroma-8);
            offset[clist][n][2] = iClip3( -128, 127, offset[clist][n][2]);
            offset[clist][n][2] = offset[clist][n][2]<<(img->bitdepth_chroma - 8);

            weight[clist][n][2] = default_weight[2];
          }
        }

        for (i=0; i < 3; i ++)
        {
          wp_weight[clist][n][i] = weight[clist][n][i];
          wp_offset[clist][n][i] = offset[clist][n][i];
#if DEBUG_WP
          printf("index %d component %d weight %d offset %d\n",n,i,weight[0][n][i],offset[0][n][i]);
#endif
        }
      }
    }
  }
}

/*!
************************************************************************
* \brief
*    Estimates reference picture weighting factors for B slices
************************************************************************
*/
void EstimateWPBSliceAlg0(ImageParameters *img, InputParameters *params)
{
  int i, j, k, n;

  int tx,DistScaleFactor;

  int index;
  int comp;
  double dc_org = 0.0;
  double dc_org_UV[2] = { 0.0 };
  double dc_ref[6][MAX_REFERENCE_PICTURES] = { {0.0} };
  double dc_ref_UV[6][MAX_REFERENCE_PICTURES][2] = { {{0.0}} };

  int default_weight[3];
  int list_offset   = ((img->MbaffFrameFlag)&&(img->mb_data[img->current_mb_nr].mb_field))? (img->current_mb_nr & 0x01) ? 4 : 2 : 0;
  int weight[6][MAX_REFERENCE_PICTURES][3];
  int offset[6][MAX_REFERENCE_PICTURES][3];
  int im_weight[6][MAX_REFERENCE_PICTURES][MAX_REFERENCE_PICTURES][3];
  int clist;
  int wf_weight, wf_offset;
  imgpel **tmpPtr;

  if (active_pps->weighted_bipred_idc == 2) //! implicit mode. Values are fixed and it is important to show it here
  {
    luma_log_weight_denom = 5;
    chroma_log_weight_denom = 5;
  }
  else                                     //! explicit mode. Values can be changed for higher precision.
  {
    luma_log_weight_denom = 5;
    chroma_log_weight_denom = 5;
  }

  wp_luma_round     = 1 << (luma_log_weight_denom - 1);
  wp_chroma_round   = 1 << (chroma_log_weight_denom - 1);
  default_weight[0] = 1 << luma_log_weight_denom;
  default_weight[1] = 1 << chroma_log_weight_denom;
  default_weight[2] = 1 << chroma_log_weight_denom;

  if (active_pps->weighted_bipred_idc == 2) //! implicit mode
  {
    for (i = 0; i < listXsize[LIST_0]; i++)
    {
      for (j = 0; j < listXsize[LIST_1]; j++)
      {
        int td, tb;
        td = iClip3(-128, 127,(listX[LIST_1][j]->poc - listX[LIST_0][i]->poc));
        tb = iClip3(-128, 127,(enc_picture->poc - listX[LIST_0][i]->poc));
        for (comp = 0; comp < 3; comp++)
        {
          // implicit weights
          if (td == 0)
          {
            im_weight[0][i][j][comp] = default_weight[comp];
            im_weight[1][i][j][comp] = default_weight[comp];
          }
          else
          {
            tx = (16384 + iabs(td/2))/td;
            DistScaleFactor = iClip3(-1024, 1023, (tx*tb + 32 )>>6);
            im_weight[1][i][j][comp] = DistScaleFactor>>2;
            if (im_weight[1][i][j][comp] < -64 || im_weight[1][i][j][comp] >128)
              im_weight[1][i][j][comp] = default_weight[comp];
            im_weight[0][i][j][comp] = 64 - im_weight[1][i][j][comp];
          }
        }
#if DEBUG_WP
        printf ("%d imp weight[%d][%d] = %d  , %d (%d %d %d) (%d %d) (%d %d)\n",enc_picture->poc, i, j,  im_weight[0][i][j][0], im_weight[1][i][j][0],
          enc_picture->poc,listX[LIST_0][i]->poc, listX[LIST_1][j]->poc,
          DistScaleFactor ,tx,td,tb);
#endif
      }
    }

    for (k = 0; k < 2; k++)
    {
      for (i = 0; i < listXsize[LIST_0]; i++)
      {
        for (j = 0; j < listXsize[LIST_1]; j++)
        {
          for (comp = 0; comp < 3; comp++)
          {
            wbp_weight[k][i][j][comp] = im_weight[k][i][j][comp];
          }
        }
      }
    }

    for (clist=0; clist<2 + list_offset; clist++)
    {
      for (index = 0; index < listXsize[clist]; index++)
      {
        for (comp = 0; comp < 3; comp++)
        {
          wp_weight[clist][index][comp] = default_weight[comp];
          wp_offset[clist][index][comp] = 0;
        }
      }
    }
  }
  else
  {
    dc_org = ComputeImgSum(pCurImg, img->height, img->width);

    if (params->ChromaWeightSupport == 1)
    {
      for (k = 0; k < 2; k++)
      {
        dc_org_UV[k] = ComputeImgSum(pImgOrg[k + 1], img->height_cr, img->width_cr);
      } 
    }

    for (clist=0; clist<2 + list_offset; clist++)
    {
      for (n = 0; n < listXsize[clist]; n++)
      {
        if ( wpxDetermineWP( params, img, clist, n ) )
        {
          /* set all values to defaults */
          for (i = 0; i < 3; i++)
          {
            wp_weight[clist][n][i] = default_weight[i];
            wp_offset[clist][n][i] = 0;
            offset   [clist][n][i] = 0;
            weight   [clist][n][i] = default_weight[i];
          }
          // To simplify these computations we may wish to perform these after a reference is 
          // stored in the reference buffer and attach them to the storedimage structure!!!
          // Y
          tmpPtr = listX[clist][n]->p_curr_img;
          dc_ref[clist][n] = ComputeImgSum(tmpPtr, img->height, img->width);

          if (dc_ref[clist][n] != 0.0)
            wf_weight = (int) (default_weight[0] * dc_org / dc_ref[clist][n] + 0.5);
          else
            wf_weight = default_weight[0];  // only used when reference picture is black
          wf_weight = iClip3(-128, 127, wf_weight);
          wf_offset = 0;

          //    printf("dc_org = %d, dc_ref = %d, weight[%d] = %d\n",dc_org, dc_ref[n],n,weight[n][0]);

          weight[clist][n][0] = wf_weight;
          offset[clist][n][0] = wf_offset;

          // UV
          if (params->ChromaWeightSupport == 1)
          {          
            for (k = 0; k < 2; k++)
            {        	
              tmpPtr = listX[clist][n]->imgUV[k];
              dc_ref_UV[clist][n][k] = ComputeImgSum(tmpPtr, img->height_cr, img->width_cr);

              if (dc_ref_UV[clist][n][k] != 0.0)
                wf_weight = (int) (default_weight[k + 1] * dc_org_UV[k] / dc_ref_UV[clist][n][k] + 0.5);
              else
                wf_weight = default_weight[k + 1];  // only used when reference picture is black
              wf_weight = iClip3(-128, 127, wf_weight);
              wf_offset = 0;

              weight[clist][n][k + 1] = wf_weight;
              offset[clist][n][k + 1] = wf_offset;
            }
          }
          else
          {
            weight[clist][n][1] = default_weight[1];
            weight[clist][n][2] = default_weight[2];        
            offset[clist][n][1] = 0;
            offset[clist][n][2] = 0;
          }

          for (i = 0; i < 3; i++)
          {
            wp_weight[clist][n][i] = weight[clist][n][i];
            wp_offset[clist][n][i] = offset[clist][n][i];
#if DEBUG_WP
            printf("%d %d\n",wp_weight[clist][index][comp],wp_offset[clist][index][comp]);
#endif
          }
        }
      }
    }

    if (active_pps->weighted_bipred_idc != 1)
    {
      for (clist=0; clist<2 + list_offset; clist++)
      {
        for (index = 0; index < listXsize[clist]; index++)
        {
          memcpy(wp_weight[clist][index], default_weight, 3 * sizeof(int));
          memset(wp_offset[clist][index], 0, 3 * sizeof(int));
        }
      }
    }


    for (i = 0; i < listXsize[LIST_0]; i++)
    {
      for (j = 0; j < listXsize[LIST_1]; j++)
      {
        for (comp = 0; comp < 3; comp++)
        {
          wbp_weight[0][i][j][comp] = wp_weight[0][i][comp];
          wbp_weight[1][i][j][comp] = wp_weight[1][j][comp];
        }
#if DEBUG_WP
        printf ("bpw weight[%d][%d] = %d  , %d (%d %d %d) (%d %d) (%d %d)\n", i, j, wbp_weight[0][i][j][0], wbp_weight[1][i][j][0],
          enc_picture->poc,listX[LIST_0][i]->poc, listX[LIST_1][j]->poc,
          DistScaleFactor ,tx,tx,tx);
#endif
      }
    }
  }
}


/*!
************************************************************************
* \brief
*    Tests P slice weighting factors to perform or not WP RD decision
************************************************************************
*/

int TestWPPSliceAlg0(ImageParameters *img, InputParameters *params, int select_offset)
{
  int i, j, k, n;

  int index;
  int comp;
  double dc_org = 0.0;
  double dc_org_UV[2] = {0.0};  
  double dc_ref[MAX_REFERENCE_PICTURES] = { 0.0 };
  double dc_ref_UV[MAX_REFERENCE_PICTURES][2] = { {0.0}};

  int default_weight[3];

  int list_offset   = ((img->MbaffFrameFlag)&&(img->mb_data[img->current_mb_nr].mb_field))? (img->current_mb_nr & 0x01) ? 4 : 2 : 0;
  int weight[2][MAX_REFERENCE_PICTURES][3];
  int offset[2][MAX_REFERENCE_PICTURES][3];
  int clist;
  int perform_wp = 0;
  imgpel **tmpPtr;

  luma_log_weight_denom = 5;
  chroma_log_weight_denom = 5;
  wp_luma_round = 1 << (luma_log_weight_denom - 1);
  wp_chroma_round = 1 << (chroma_log_weight_denom - 1);
  default_weight[0] = 1 << luma_log_weight_denom;
  default_weight[1] = default_weight[2] = 1 << chroma_log_weight_denom;

  /* set all values to defaults */
  for (i = 0; i < 2 + list_offset; i++)
  {
    for (j = 0; j < listXsize[i]; j++)
    {
      for (n = 0; n < 3; n++)
      {
        weight[i][j][n] = default_weight[n];
        wp_weight[i][j][n] = default_weight[n];
        wp_offset[i][j][n] = 0;
        offset[i][j][n] = 0;
      }
    }
  }

  dc_org = ComputeImgSum(pCurImg, img->height, img->width);

  if (params->ChromaWeightSupport == 1)
  {
    for (k = 0; k < 2; k++)
    {
      dc_org_UV[k] = ComputeImgSum(pImgOrg[k + 1], img->height_cr, img->width_cr);
    } 
  }

  for (clist = 0; clist < 2 + list_offset; clist++)
  {
    for (n = 0; n < listXsize[clist]; n++)
    {
      tmpPtr = listX[clist][n]->p_curr_img;
      dc_ref[n] = ComputeImgSum(tmpPtr, img->height, img->width);

      if (params->ChromaWeightSupport == 1)
      {
        for (k = 0; k < 2; k++)
        {
          tmpPtr = listX[clist][n]->imgUV[k];
          dc_ref_UV[n][k] = ComputeImgSum(tmpPtr, img->height_cr, img->width_cr);
        }        
      }

      if (select_offset == 0)
      {
        if (dc_ref[n] != 0.0)
          weight[clist][n][0] = (int) (default_weight[0] * dc_org / dc_ref[n] + 0.5);
        else
          weight[clist][n][0] = default_weight[0];  // only used when reference picture is black
        weight[clist][n][0] = iClip3(-128, 127, weight[clist][n][0]);
        if (params->ChromaWeightSupport == 1)
        {
          if (dc_ref_UV[n][0] != 0)
            weight[clist][n][1] = (int) (default_weight[1] * dc_org_UV[0] / dc_ref_UV[n][0] + 0.5);
          else
            weight[clist][n][1] = default_weight[1];  // only used when reference picture is black
          weight[clist][n][1] = iClip3(-128, 127, weight[clist][n][1]);

          if (dc_ref_UV[n][1] != 0)
            weight[clist][n][2] = (int) (default_weight[2] * dc_org_UV[1] / dc_ref_UV[n][1] + 0.5);
          else
            weight[clist][n][2] = default_weight[2];  // only used when reference picture is black
          weight[clist][n][2] = iClip3(-64, 128, weight[clist][n][2]);
        }
      }
      else
      {
        offset[clist][n][0] = (int) ((dc_org - dc_ref[n])/(img->size)+0.5);
        offset[clist][n][0] = (offset[clist][n][0]+((img->bitdepth_luma-8)>>1))>>(img->bitdepth_luma-8);
        offset[clist][n][0] = iClip3( -128, 127, offset[clist][n][0]);
        offset[clist][n][0] = offset[clist][n][0]<<(img->bitdepth_luma-8);
        weight[clist][n][0] = default_weight[0];

        if (params->ChromaWeightSupport == 1)
        {
            offset[clist][n][1] = (int) ((dc_org_UV[0] - dc_ref_UV[n][0])/(img->size_cr)+0.5);
            offset[clist][n][1] = (offset[clist][n][1] + ((img->bitdepth_chroma - 8)>>1))>>(img->bitdepth_chroma-8);
            offset[clist][n][1] = iClip3( -128, 127, offset[clist][n][1]);
            offset[clist][n][1] = offset[clist][n][1]<<(img->bitdepth_chroma - 8);
            
            weight[clist][n][1] = default_weight[1];

            offset[clist][n][2] = (int) ((dc_org_UV[1] - dc_ref_UV[n][1])/(img->size_cr)+0.5);
            offset[clist][n][2] = (offset[clist][n][2] + ((img->bitdepth_chroma - 8)>>1))>>(img->bitdepth_chroma-8);
            offset[clist][n][2] = iClip3( -128, 127, offset[clist][n][2]);
            offset[clist][n][2] = offset[clist][n][2]<<(img->bitdepth_chroma - 8);

            weight[clist][n][2] = default_weight[2];
        }
      }
    }
  }

  for (clist=0; clist<2 + list_offset; clist++)
  {
    for (index = 0; index < listXsize[clist]; index++)
    {
      for (comp=0; comp < 3; comp ++)
      {
        int offset_test = params->RDPSliceBTest && active_sps->profile_idc != 66
          ? iabs(offset[clist][index][comp]) > 2
          : offset[clist][index][comp] != 0;

        if (weight[clist][index][comp] != default_weight[0] || offset_test)
        {
          perform_wp = 1;
          break;
        }
      }
      if (perform_wp == 1) break;
    }
    if (perform_wp == 1) break;
  }

  return perform_wp;
}

/*!
************************************************************************
* \brief
*    TestWPBSliceAlg0:
*    Tests B slice weighting prediction
************************************************************************
*/
int TestWPBSliceAlg0(ImageParameters *img, InputParameters *params, int select_method)
{
  int i, j, k, n;

  int tx,DistScaleFactor;

  int index;
  int comp;
  double dc_org = 0.0;
  double dc_org_UV[2] = { 0.0 };    
  double dc_ref[6][MAX_REFERENCE_PICTURES] = { {0.0} };  
  double dc_ref_UV[6][MAX_REFERENCE_PICTURES][2] = { {{0.0}} };

  int default_weight[3];
  // this needs to be fixed.
  int list_offset   = ((img->MbaffFrameFlag)&&(img->mb_data[img->current_mb_nr].mb_field))? (img->current_mb_nr & 0x01) ? 4 : 2 : 0;
  int weight[6][MAX_REFERENCE_PICTURES][3];
  int offset[6][MAX_REFERENCE_PICTURES][3];
  int im_weight[6][MAX_REFERENCE_PICTURES][MAX_REFERENCE_PICTURES][3];
  int clist;
  int wf_weight, wf_offset;
  int perform_wp = 0;
  imgpel **tmpPtr;

  if (select_method == 1) //! implicit mode
  {
    luma_log_weight_denom = 5;
    chroma_log_weight_denom = 5;
  }
  else
  {
    luma_log_weight_denom = 5;
    chroma_log_weight_denom = 5;
  }

  wp_luma_round     = 1 << (luma_log_weight_denom - 1);
  wp_chroma_round   = 1 << (chroma_log_weight_denom - 1);
  default_weight[0] = 1 << luma_log_weight_denom;
  default_weight[1] = 1 << chroma_log_weight_denom;
  default_weight[2] = 1 << chroma_log_weight_denom;

  /* set all values to defaults */
  for (i = 0; i < 2 + list_offset; i++)
  {
    for (j = 0; j < listXsize[i]; j++)
    {
      for (n = 0; n < 3; n++)
      {
        wp_weight[i][j][n] = default_weight[n];
        wp_offset[i][j][n] = 0;
        offset   [i][j][n] = 0;
        weight   [i][j][n] = default_weight[n];
      }
    }
  }

  for (i = 0; i < listXsize[LIST_0]; i++)
  {
    for (j = 0; j < listXsize[LIST_1]; j++)
    {
      int td, tb;
      td = iClip3(-128, 127,(listX[LIST_1][j]->poc - listX[LIST_0][i]->poc));
      tb = iClip3(-128, 127,(enc_picture->poc - listX[LIST_0][i]->poc));
      for (comp = 0; comp < 3; comp++)
      {
        // implicit weights
        if (td == 0)
        {
          im_weight[0][i][j][comp] = default_weight[comp];
          im_weight[1][i][j][comp] = default_weight[comp];
        }
        else
        {
          tx = (16384 + iabs(td/2))/td;
          DistScaleFactor = iClip3(-1024, 1023, (tx*tb + 32 )>>6);
          im_weight[1][i][j][comp] = DistScaleFactor >> 2;
          if (im_weight[1][i][j][comp] < -64 || im_weight[1][i][j][comp] >128)
            im_weight[1][i][j][comp] = default_weight[comp];
          im_weight[0][i][j][comp] = 64 - im_weight[1][i][j][comp];
        }
      }
    }
  }


  if (select_method == 1) //! implicit mode
  {
    for (i = 0; i < listXsize[LIST_0]; i++)
    {
      for (j = 0; j < listXsize[LIST_1]; j++)
      {
        for (comp = 0; comp < 3; comp++)
        {
          wbp_weight[1][i][j][comp] = im_weight[1][i][j][comp] ;
          wbp_weight[0][i][j][comp] = im_weight[0][i][j][comp];
        }
      }
    }

    for (clist=0; clist<2 + list_offset; clist++)
    {
      for (index = 0; index < listXsize[clist]; index++)
      {
        for (comp = 0; comp < 3; comp++)
        {
          wp_weight[clist][index][comp] = default_weight[comp];
          wp_offset[clist][index][comp] = 0;
        }
      }
    }
  }
  else
  {
    dc_org = ComputeImgSum(pCurImg, img->height, img->width);

    if (params->ChromaWeightSupport == 1)
    {
      for (k = 0; k < 2; k++)
      {
        dc_org_UV[k] = ComputeImgSum(pImgOrg[k + 1], img->height_cr, img->width_cr);
      } 
    }

    for (clist=0; clist<2 + list_offset; clist++)
    {
      for (n = 0; n < listXsize[clist]; n++)
      {
        // To simplify these computations we may wish to perform these after a reference is 
        // stored in the reference buffer and attach them to the storedimage structure!!!
        // Y
        tmpPtr = listX[clist][n]->p_curr_img;
        dc_ref[clist][n] = ComputeImgSum(tmpPtr, img->height, img->width);

        if (dc_ref[clist][n] != 0.0)
          wf_weight = (int) (default_weight[0] * dc_org / dc_ref[clist][n] + 0.5);
        else
          wf_weight = default_weight[0];  // only used when reference picture is black
        wf_weight = iClip3(-128, 127, wf_weight);
        wf_offset = 0;

        weight[clist][n][0] = wf_weight;
        offset[clist][n][0] = wf_offset;

        // UV
        if (params->ChromaWeightSupport == 1)
        {          
          for (k = 0; k < 2; k++)
          {
            tmpPtr = listX[clist][n]->imgUV[k];
            dc_ref_UV[clist][n][k] = ComputeImgSum(tmpPtr, img->height_cr, img->width_cr);

            if (dc_ref_UV[clist][n][k] != 0.0)
              wf_weight = (int) (default_weight[k + 1] * dc_org_UV[k] / dc_ref_UV[clist][n][k] + 0.5);
            else
              wf_weight = default_weight[k + 1];  // only used when reference picture is black
            wf_weight = iClip3(-128, 127, wf_weight);
            wf_offset = 0;

            weight[clist][n][k + 1] = wf_weight;
            offset[clist][n][k + 1] = wf_offset;
          }
        }
        else
        {
          weight[clist][n][1] = default_weight[1];
          weight[clist][n][2] = default_weight[2];        
          offset[clist][n][1] = 0;
          offset[clist][n][2] = 0;
        }
      }
    }

    if (select_method == 0) //! explicit mode
    {
      for (clist=0; clist<2 + list_offset; clist++)
      {
        for (index = 0; index < listXsize[clist]; index++)
        {
          memcpy(wp_weight[clist][index], weight[clist][index], 3 * sizeof(int));
          memcpy(wp_offset[clist][index], offset[clist][index], 3 * sizeof(int));
        }
      }
    }
    else
    {
      for (clist=0; clist<2 + list_offset; clist++)
      {
        for (index = 0; index < listXsize[clist]; index++)
        {
          memcpy(wp_weight[clist][index], default_weight, 3 * sizeof(int));
          memset(wp_offset[clist][index], 0, 3 * sizeof(int));
        }
      }
    }

    for (i = 0; i < listXsize[LIST_0]; i++)
    {
      for (j = 0; j < listXsize[LIST_1]; j++)
      {
        for (comp = 0; comp < 3; comp++)
        {
          wbp_weight[0][i][j][comp] = wp_weight[0][i][comp];
          wbp_weight[1][i][j][comp] = wp_weight[1][j][comp];
        }
#if DEBUG_WP
        printf ("bpw weight[%d][%d] = %d  , %d (%d %d %d) (%d %d) (%d %d)\n", i, j, wbp_weight[0][i][j][0], wbp_weight[1][i][j][0],
        enc_picture->poc,listX[LIST_0][i]->poc, listX[LIST_1][j]->poc,
        DistScaleFactor ,tx,tx,tx);
#endif
      }
    }
  }

  if (select_method == 0) //! implicit mode
  {
    int active_refs[2];

    active_refs[0] = (params->B_List0_refs == 0 ? listXsize[0] : imin(params->B_List0_refs, listXsize[0]));
    active_refs[1] = (params->B_List1_refs == 0 ? listXsize[1] : imin(params->B_List1_refs, listXsize[1]));

    for (clist=0; clist<2 + list_offset; clist++)
    {
      for (index = 0; index < active_refs[clist]; index++)
      {
        for (comp=0; comp < 3; comp ++)
        {
          if (wp_weight[clist][index][comp] != default_weight[comp])
          {
            perform_wp = 1;
            break;
          }
        }
        if (perform_wp == 1) break;
      }
      if (perform_wp == 1) break;
    }
  }
  return perform_wp;
}
