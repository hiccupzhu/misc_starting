
/*!
*************************************************************************************
* \file me_epzs.c
*
* \brief
*    Motion Estimation using EPZS
*
* \author
*    Main contributors (see contributors.h for copyright, address and affiliation details)
*      - Alexis Michael Tourapis <alexismt@ieee.org>
*      - Athanasios Leontaris    <aleon@dolby.com>
*
*************************************************************************************
*/

#include "contributors.h"

#include <limits.h>

#include "global.h"
#include "image.h"
#include "memalloc.h"
#include "mb_access.h"
#include "refbuf.h"

#include "me_distortion.h"
#include "me_epzs.h"
#include "mv-search.h"

#define EPZSREF 1

// Define Global Parameters
static const short blk_parent[8] = {1, 1, 1, 1, 2, 4, 4, 5}; //!< {skip, 16x16, 16x8, 8x16, 8x8, 8x4, 4x8, 4x4}
//static const short blk_child[8]  = {1, 2, 4, 4, 5, 7, 7, 7}; //!< {skip, 16x16, 16x8, 8x16, 8x8, 8x4, 4x8, 4x4}
static const int   minthres_base[8] = {0,  64,  32,  32,  16,  8,  8,  4};
static const int   medthres_base[8] = {0, 256, 128, 128,  64, 32, 32, 16};
static const int   maxthres_base[8] = {0, 768, 384, 384, 192, 96, 96, 48};
static const short search_point_hp[10][2] = {{0,0},{-2,0}, {0,2}, {2,0},  {0,-2}, {-2,2},  {2,2},  {2,-2}, {-2,-2}, {-2,2}};
static const short search_point_qp[10][2] = {{0,0},{-1,0}, {0,1}, {1,0},  {0,-1}, {-1,1},  {1,1},  {1,-1}, {-1,-1}, {-1,1}};
//static const int   next_subpel_pos_start[5][5] = {};
//static const int   next_subpel_pos_end  [5][5] = {};
static int (*computePred)(imgpel* , int , int , int , int , int );

static short EPZSBlkCount;
static int   searcharray;
static int   mv_rescale;

//! Define EPZS Refinement patterns
static int pattern_data[5][12][4] =
{
  { // Small Diamond pattern
    {  0,  4,  3, 3 }, {  4,  0,  0, 3 }, {  0, -4,  1, 3 }, { -4,  0, 2, 3 }
  },
  { // Square pattern
    {  0,  4,  7, 3 }, {  4,  4,  7, 5 }, {  4,  0,  1, 3 }, {  4, -4, 1, 5 },
    {  0, -4,  3, 3 }, { -4, -4,  3, 5 }, { -4,  0,  5, 3 }, { -4,  4, 5, 5 }
  },
  { // Enhanced Diamond pattern
    { -4,  4, 10, 5 }, {  0,  8, 10, 8 }, {  0,  4, 10, 7 }, {  4,  4, 1, 5 },
    {  8,  0, 1,  8 }, {  4,  0,  1, 7 }, {  4, -4,  4, 5 }, {  0, -8, 4, 8 },
    {  0, -4, 4,  7 }, { -4, -4, 7,  5 }, { -8,  0,  7, 8 }, { -4,  0, 7, 7 }

  },
  { // Large Diamond pattern
    {  0,  8, 6,  5 }, {  4,  4, 0,  3 }, {  8,  0, 0,  5 }, {  4, -4, 2, 3 },
    {  0, -8, 2,  5 }, { -4, -4, 4,  3 }, { -8,  0, 4,  5 }, { -4,  4, 6, 3 }
  },
  { // Extended Subpixel pattern
    {  0,  8, 6, 12 }, {  4,  4, 0, 12 }, {  8,  0, 0, 12 }, {  4, -4, 2, 12 },
    {  0, -8, 2, 12 }, { -4, -4, 4, 12 }, { -8,  0, 4, 12 }, { -4,  4, 6, 12 },
    {  0,  2, 6, 12 }, {  2,  0, 0, 12 }, {  0, -2, 2, 12 }, { -2,  0, 4, 12 }
  }
};

// Other definitions
static const char c_EPZSPattern[6][20]     = { "Diamond", "Square", "Extended Diamond", "Large Diamond", "SBP Large Diamond", "PMVFAST"};
static const char c_EPZSDualPattern[7][20] = { "Disabled","Diamond", "Square", "Extended Diamond", "Large Diamond", "SBP Large Diamond", "PMVFAST"};
static const char c_EPZSFixed[3][20]       = { "Disabled","All P", "All P + B"};
static const char c_EPZSOther[2][20]       = { "Disabled","Enabled"};

static int medthres[8];
static int maxthres[8];
static int minthres[8];
static int subthres[8];
static int mv_scale[6][MAX_REFERENCE_PICTURES][MAX_REFERENCE_PICTURES];

static short **EPZSMap;  //!< Memory Map definition
int ***EPZSDistortion;  //!< Array for storing SAD Values
int ***EPZSBiDistortion;  //!< Array for storing SAD Values
#if EPZSREF
MotionVector *****EPZS_Motion;  //!< Array for storing Motion Vectors
short ******EPZSMotion;  //!< Array for storing Motion Vectors
#else
MotionVector ****EPZS_Motion;  //!< Array for storing Motion Vectors
short *****EPZSMotion;  //!< Array for storing Motion Vectors
#endif

//
EPZSStructure *searchPattern,*searchPatternD, *predictor;
EPZSStructure *window_predictor, *window_predictor_extended;
EPZSStructure *sdiamond,*square,*ediamond,*ldiamond, *sbdiamond, *pmvfast;
EPZSColocParams *EPZSCo_located;

// Functions

/*!
*************************************************************************************
* \brief
*    Determine stop criterion for EPZS
*************************************************************************************
*/
static int EPZSDetermineStopCriterion(int* prevSad, PixelPos *block_a, PixelPos *block_b, PixelPos *block_c, int pic_pix_x2, int blocktype, int blockshape_x)
{
  int sadA, sadB, sadC, stopCriterion;

  sadA = block_a->available ? prevSad[pic_pix_x2 - blockshape_x] : INT_MAX;
  sadB = block_b->available ? prevSad[pic_pix_x2] : INT_MAX;
  sadC = block_c->available ? prevSad[pic_pix_x2 + blockshape_x] : INT_MAX;

  stopCriterion = imin(sadA,imin(sadB,sadC));
  stopCriterion = imax(stopCriterion,minthres[blocktype]);
  stopCriterion = imin(stopCriterion,maxthres[blocktype]);

  stopCriterion = (9 * imax (medthres[blocktype], stopCriterion) + 2 * medthres[blocktype]) >> 3;
  return stopCriterion;
}

/*!
************************************************************************
* \brief
*    Allocate co-located memory
*
* \param size_x
*    horizontal luma size
* \param size_y
*    vertical luma size
* \param mb_adaptive_frame_field_flag
*    flag that indicates macroblock adaptive frame/field coding
*
* \return
*    the allocated EPZSColocParams structure
************************************************************************
*/
static EPZSColocParams* allocEPZScolocated(int size_x, int size_y, int mb_adaptive_frame_field_flag)
{
  EPZSColocParams *s;

  s = calloc(1, sizeof(EPZSColocParams));
  if (NULL == s)
    no_mem_exit("alloc_EPZScolocated: s");

  s->size_x = size_x;
  s->size_y = size_y;
  get_mem3Dmv(&(s->frame), 2, size_y / BLOCK_SIZE, size_x / BLOCK_SIZE);

  if (mb_adaptive_frame_field_flag)
  {
    get_mem3Dmv(&(s->top), 2, size_y / (BLOCK_SIZE * 2), size_x / BLOCK_SIZE);
    get_mem3Dmv(&(s->bot), 2, size_y / (BLOCK_SIZE * 2), size_x / BLOCK_SIZE);
  }

  s->mb_adaptive_frame_field_flag  = mb_adaptive_frame_field_flag;

  return s;
}

/*!
************************************************************************
* \brief
*    Free co-located memory.
*
* \param p
*    structure to be freed
*
************************************************************************
*/
static void freeEPZScolocated(EPZSColocParams* p)
{
  if (p)
  {
    free_mem3Dmv (p->frame);

    if (p->mb_adaptive_frame_field_flag)
    {
      free_mem3Dmv (p->top);
      free_mem3Dmv (p->bot);
    }

    free(p);

    p=NULL;
  }
}

/*!
************************************************************************
* \brief
*    Allocate EPZS pattern memory
*
* \param searchpoints
*    number of searchpoints to allocate
*
* \return
*    the allocated EPZSStructure structure
************************************************************************
*/
static EPZSStructure* allocEPZSpattern(int searchpoints)
{
  EPZSStructure *s;

  s = calloc(1, sizeof(EPZSStructure));
  if (NULL == s)
    no_mem_exit("alloc_EPZSpattern: s");

  s->searchPoints = searchpoints;
  s->point = (SPoint*) calloc(searchpoints, sizeof(SPoint));

  return s;
}

/*!
************************************************************************
* \brief
*    Free EPZS pattern memory.
*
* \param p
*    structure to be freed
*
************************************************************************
*/
static void freeEPZSpattern(EPZSStructure* p)
{
  if (p)
  {
    free ( (SPoint*) p->point);
    free(p);
    p=NULL;
  }
}

static void assignEPZSpattern(EPZSStructure *pattern,int type, int stopSearch, int nextLast, EPZSStructure *nextpattern)
{
  int i;

  for (i = 0; i < pattern->searchPoints; i++)
  {
    pattern->point[i].motion.mv_x = pattern_data[type][i][0] >> mv_rescale;
    pattern->point[i].motion.mv_y = pattern_data[type][i][1] >> mv_rescale;
    pattern->point[i].start_nmbr  = pattern_data[type][i][2];
    pattern->point[i].next_points = pattern_data[type][i][3];
  }
  pattern->stopSearch = stopSearch;
  pattern->nextLast = nextLast;
  pattern->nextpattern = nextpattern;
}

/*!
************************************************************************
* \brief
*    calculate RoundLog2(uiVal)
************************************************************************
*/
static int RoundLog2 (int iValue)
{
  int iRet = 0;
  int iValue_square = iValue * iValue;

  while ((1 << (iRet + 1)) <= iValue_square)
    iRet++;

  iRet = (iRet + 1) >> 1;

  return iRet;
}

/*!
***********************************************************************
* \brief
*    Add Predictor function
*    
***********************************************************************
*/
static inline int add_predictor(MotionVector *cur_mv, MotionVector prd_mv, int mvScale, int shift_mv)
{
  *cur_mv = prd_mv;
  cur_mv->mv_x = rshift_rnd_sf((mvScale * cur_mv->mv_x), shift_mv);
  cur_mv->mv_y = rshift_rnd_sf((mvScale * cur_mv->mv_y), shift_mv);
  return (*((int*) cur_mv) != 0);
}

/*!
************************************************************************
* \brief
*    EPZS Search Window Predictor Initialization
************************************************************************
*/
static void EPZSWindowPredictorInit (short search_range, EPZSStructure * predictor, short mode)
{
  int pos;
  int searchpos, fieldsearchpos;
  int prednum = 0;
  int i;
  int search_range_qpel = params->EPZSSubPelGrid ? 2 : 0;
  if (mode == 0)
  {
    for (pos = RoundLog2 (search_range) - 2; pos > -1; pos--)
    {
      searchpos = ((search_range << search_range_qpel)>> pos);

      for (i=1; i>=-1; i-=2)
      {
        predictor->point[prednum  ].motion.mv_x =  i * searchpos;
        predictor->point[prednum++].motion.mv_y =  0;
        predictor->point[prednum  ].motion.mv_x =  i * searchpos;
        predictor->point[prednum++].motion.mv_y =  i * searchpos;
        predictor->point[prednum  ].motion.mv_x =  0;
        predictor->point[prednum++].motion.mv_y =  i * searchpos;
        predictor->point[prednum  ].motion.mv_x = -i * searchpos;
        predictor->point[prednum++].motion.mv_y =  i * searchpos;
      }
    }
  }
  else // if (mode == 0)
  {
    for (pos = RoundLog2 (search_range) - 2; pos > -1; pos--)
    {
      searchpos = ((search_range << search_range_qpel) >> pos);
      fieldsearchpos = ((3 * searchpos + 1) << search_range_qpel) >> 1;

      for (i=1; i>=-1; i-=2)
      {
        predictor->point[prednum  ].motion.mv_x =  i * searchpos;
        predictor->point[prednum++].motion.mv_y =  0;
        predictor->point[prednum  ].motion.mv_x =  i * searchpos;
        predictor->point[prednum++].motion.mv_y =  i * searchpos;
        predictor->point[prednum  ].motion.mv_x =  0;
        predictor->point[prednum++].motion.mv_y =  i * searchpos;
        predictor->point[prednum  ].motion.mv_x = -i * searchpos;
        predictor->point[prednum++].motion.mv_y =  i * searchpos;
      }

      for (i=1; i>=-1; i-=2)
      {
        predictor->point[prednum  ].motion.mv_x =  i * fieldsearchpos;
        predictor->point[prednum++].motion.mv_y = -i * searchpos;
        predictor->point[prednum  ].motion.mv_x =  i * fieldsearchpos;
        predictor->point[prednum++].motion.mv_y =  0;
        predictor->point[prednum  ].motion.mv_x =  i * fieldsearchpos;
        predictor->point[prednum++].motion.mv_y =  i * searchpos;
        predictor->point[prednum  ].motion.mv_x =  i * searchpos;
        predictor->point[prednum++].motion.mv_y =  i * fieldsearchpos;
        predictor->point[prednum  ].motion.mv_x =  0;
        predictor->point[prednum++].motion.mv_y =  i * fieldsearchpos;
        predictor->point[prednum  ].motion.mv_x = -i * searchpos;
        predictor->point[prednum++].motion.mv_y =  i * fieldsearchpos;
      }
    }
  }
  predictor->searchPoints = prednum;
}

/*!
************************************************************************
* \brief
*    EPZS Global Initialization
************************************************************************
*/
int EPZSInit (InputParameters *params, ImageParameters *img)
{
  int pel_error_me = 1 << (img->bitdepth_luma - 8);
  int pel_error_me_cr = 1 << (img->bitdepth_chroma - 8);
  int i, memory_size = 0;
  double chroma_weight = params->ChromaMEEnable ? pel_error_me_cr * params->ChromaMEWeight * (double) (img->width_cr * img->height_cr) / (double) (img->width * img->height) : 0;
  int searchlevels = RoundLog2 (params->search_range) - 1;
  
  searcharray = params->BiPredMotionEstimation? (2 * imax (params->search_range, params->BiPredMESearchRange) + 1) << (params->EPZSGrid) : (2 * params->search_range + 1)<< (params->EPZSGrid);

  mv_rescale = params->EPZSSubPelGrid ? 0 : 2;
  
  //! In this implementation we keep threshold limits fixed.
  //! However one could adapt these limits based on lagrangian
  //! optimization considerations (i.e. qp), while also allow
  //! adaptation of the limits themselves based on content or complexity.
  for (i=0;i<8;i++)
  {
    medthres[i] = params->EPZSMedThresScale * (medthres_base[i] * pel_error_me + (int) (medthres_base[i] * chroma_weight + 0.5));
    maxthres[i] = params->EPZSMaxThresScale * (maxthres_base[i] * pel_error_me + (int) (maxthres_base[i] * chroma_weight + 0.5));
    minthres[i] = params->EPZSMinThresScale * (minthres_base[i] * pel_error_me + (int) (minthres_base[i] * chroma_weight + 0.5));
    subthres[i] = params->EPZSSubPelThresScale * (medthres_base[i] * pel_error_me + (int) (medthres_base[i] * chroma_weight + 0.5));
  }

  //! Definition of pottential EPZS patterns.
  //! It is possible to also define other patterns, or even use
  //! resizing patterns (such as the PMVFAST scheme. These patterns
  //! are only shown here as reference, while the same also holds
  //! for this implementation (i.e. new conditions could be added
  //! on adapting predictors, or thresholds etc. Note that search
  //! could also be performed on subpel positions directly while
  //! pattern needs not be restricted on integer positions only.

  //! Allocate memory and assign search patterns
  sdiamond = allocEPZSpattern(4);
  assignEPZSpattern(sdiamond, SDIAMOND, TRUE, TRUE, sdiamond);
  square = allocEPZSpattern(8);
  assignEPZSpattern(square, SQUARE, TRUE, TRUE, square);
  ediamond = allocEPZSpattern(12);
  assignEPZSpattern(ediamond, EDIAMOND, TRUE, TRUE, ediamond);
  ldiamond = allocEPZSpattern(8);
  assignEPZSpattern(ldiamond, LDIAMOND, TRUE, TRUE, ldiamond);
  sbdiamond = allocEPZSpattern(12);
  assignEPZSpattern(sbdiamond, SBDIAMOND, FALSE, TRUE, sdiamond);
  pmvfast = allocEPZSpattern(8);
  assignEPZSpattern(pmvfast, LDIAMOND, FALSE, TRUE, sdiamond);

  //! Allocate and assign window based predictors.
  //! Other window types could also be used, while method could be
  //! made a bit more adaptive (i.e. patterns could be assigned
  //! based on neighborhood
  window_predictor = allocEPZSpattern(searchlevels * 8);
  window_predictor_extended = allocEPZSpattern(searchlevels * 20);
  EPZSWindowPredictorInit ((short) params->search_range, window_predictor, 0);
  EPZSWindowPredictorInit ((short) params->search_range, window_predictor_extended, 1);
  //! Also assing search predictor memory
  // maxwindow + spatial + blocktype + temporal + memspatial
  predictor = allocEPZSpattern(searchlevels * 20 + 5 + 5 + 9 * (params->EPZSTemporal) + 3 * (params->EPZSSpatialMem));

  //! Finally assign memory for all other elements
  //! (distortion, EPZSMap, and temporal predictors)

  //memory_size += get_offset_mem2Dshort(&EPZSMap, searcharray, searcharray, (searcharray>>1), (searcharray>>1));
  memory_size += get_mem3Dint (&EPZSDistortion, 6, 7, img->width/BLOCK_SIZE);
  if (params->BiPredMotionEstimation)
    memory_size += get_mem3Dint (&EPZSBiDistortion, 6, 7, img->width/BLOCK_SIZE);
  memory_size += get_mem2Dshort (&EPZSMap, searcharray, searcharray );

  if (params->EPZSSpatialMem)
  {
#if EPZSREF
    memory_size += get_mem5Dmv (&EPZS_Motion, 6, img->max_num_references, 7, 4, img->width/BLOCK_SIZE);
    memory_size += get_mem6Dshort (&EPZSMotion, 6, img->max_num_references, 7, 4, img->width/BLOCK_SIZE, 2);
#else
    memory_size += get_mem4Dmv (&EPZS_Motion, 6, 7, 4, img->width/BLOCK_SIZE);
    memory_size += get_mem5Dshort (&EPZSMotion, 6, 7, 4, img->width/BLOCK_SIZE, 2);
#endif
  }

  if (params->EPZSTemporal)
    EPZSCo_located = allocEPZScolocated (img->width, img->height, active_sps->mb_adaptive_frame_field_flag);

  switch (params->EPZSPattern)
  {
  case 5:
    searchPattern = pmvfast;
    break;
  case 4:
    searchPattern = sbdiamond;
    break;
  case 3:
    searchPattern = ldiamond;
    break;
  case 2:
    searchPattern = ediamond;
    break;
  case 1:
    searchPattern = square;
    break;
  case 0:
  default:
    searchPattern = sdiamond;
    break;
  }

  switch (params->EPZSDual)
  {
  case 6:
    searchPatternD = pmvfast;
    break;
  case 5:
    searchPatternD = sbdiamond;
    break;
  case 4:
    searchPatternD = ldiamond;
    break;
  case 3:
    searchPatternD = ediamond;
    break;
  case 2:
    searchPatternD = square;
    break;
  case 1:
  default:
    searchPatternD = sdiamond;
    break;
  }

  return memory_size;
}

/*!
************************************************************************
* \brief
*    Delete EPZS Alocated memory
************************************************************************
*/
void EPZSDelete (InputParameters *params)
{
  if (params->EPZSTemporal)
    freeEPZScolocated (EPZSCo_located);

  //free_offset_mem2Dshort(EPZSMap, searcharray, (searcharray>>1), (searcharray>>1));
  free_mem2Dshort(EPZSMap);
  free_mem3Dint  (EPZSDistortion);
  if (params->BiPredMotionEstimation)
    free_mem3Dint  (EPZSBiDistortion);
  freeEPZSpattern(window_predictor_extended);
  freeEPZSpattern(window_predictor);
  freeEPZSpattern(predictor);
  // Free search patterns
  freeEPZSpattern(pmvfast);
  freeEPZSpattern(sbdiamond);
  freeEPZSpattern(ldiamond);
  freeEPZSpattern(ediamond);
  freeEPZSpattern(sdiamond);
  freeEPZSpattern(square);
  if (params->EPZSSpatialMem)
  {
#if EPZSREF
    free_mem5Dmv (EPZS_Motion);
    free_mem6Dshort (EPZSMotion);
#else
    free_mem4Dmv (EPZS_Motion);
    free_mem5Dshort (EPZSMotion);
#endif
  }

}

//! For ME purposes restricting the co-located partition is not necessary.
/*!
************************************************************************
* \brief
*    EPZS Slice Level Initialization
************************************************************************
*/
void EPZSSliceInit (InputParameters *params, ImageParameters *img, EPZSColocParams * p, StorablePicture ** listX[6])
{
  StorablePicture *fs, *fs_top, *fs_bottom;
  StorablePicture *fs1, *fs_top1, *fs_bottom1, *fsx;
  int i, j, k, jj, jdiv, loffset;
  int prescale, iTRb, iTRp;
  int list = img->type == B_SLICE ? LIST_1 : LIST_0;
  int tempmv_scale[2];
  int epzs_scale[2][6][MAX_LIST_SIZE];
  int iref;
  int invmv_precision = 8;

  // Lets compute scaling factoes between all references in lists.
  // Needed to scale spatial predictors.
  for (j = LIST_0; j < 2 + (img->MbaffFrameFlag * 4); j ++)
  {
    for (k = 0; k < listXsize[j]; k++)
    {
      for (i = 0; i < listXsize[j]; i++)
      {
        if ((j >> 1) == 0)
        {
          iTRb = iClip3 (-128, 127, enc_picture->poc - listX[j][i]->poc);
          iTRp = iClip3 (-128, 127, enc_picture->poc - listX[j][k]->poc);
        }
        else if ((j >> 1) == 1)
        {
          iTRb = iClip3 (-128, 127, enc_picture->top_poc - listX[j][i]->poc);
          iTRp = iClip3 (-128, 127, enc_picture->top_poc - listX[j][k]->poc);
        }
        else
        {
          iTRb = iClip3 (-128, 127, enc_picture->bottom_poc - listX[j][i]->poc);
          iTRp = iClip3 (-128, 127, enc_picture->bottom_poc - listX[j][k]->poc);
        }

        if (iTRp != 0)
        {
          prescale = (16384 + iabs (iTRp / 2)) / iTRp;
          mv_scale[j][i][k] = iClip3 (-2048, 2047, rshift_rnd_sf((iTRb * prescale), 6));
        }
        else
          mv_scale[j][i][k] = 256;
      }
    }
  }

  if (params->EPZSTemporal)
  {
    fs_top = fs_bottom = fs = listX[list][0];
    if (listXsize[list]> 1)
      fs_top1 = fs_bottom1 = fs1 = listX[list][1];
    else
      fs_top1 = fs_bottom1 = fs1 = listX[list][0];

    for (j = 0; j < 6; j++)
    {
      for (i = 0; i < 6; i++)
      {
        epzs_scale[0][j][i] = 256;
        epzs_scale[1][j][i] = 256;
      }
    }

    for (j = 0; j < 2 + (img->MbaffFrameFlag * 4); j += 2)
    {
      for (i = 0; i < listXsize[j]; i++)
      {
        if (j == 0)
          iTRb = iClip3 (-128, 127, enc_picture->poc - listX[LIST_0 + j][i]->poc);
        else if (j == 2)
          iTRb = iClip3 (-128, 127, enc_picture->top_poc - listX[LIST_0 + j][i]->poc);
        else
          iTRb = iClip3 (-128, 127, enc_picture->bottom_poc - listX[LIST_0 + j][i]->poc);
        iTRp = iClip3 (-128, 127, listX[list + j][0]->poc - listX[LIST_0 + j][i]->poc);
        if (iTRp != 0)
        {
          prescale = (16384 + iabs (iTRp / 2)) / iTRp;
          prescale = iClip3 (-2048, 2047, rshift_rnd_sf((iTRb * prescale), 6));
          //prescale = (iTRb * prescale + 32) >> 6;
        }
        else      // This could not happen but lets use it in case that reference is removed.
          prescale = 256;
        epzs_scale[0][j][i] = rshift_rnd_sf((mv_scale[j][0][i] * prescale), 8);
        epzs_scale[0][j + 1][i] = prescale - 256;
        
        if (listXsize[list + j]>1)
        {
          iTRp = iClip3 (-128, 127, listX[list + j][1]->poc - listX[LIST_0 + j][i]->poc);
          if (iTRp != 0)
          {
            prescale = (16384 + iabs (iTRp / 2)) / iTRp;
            prescale = iClip3 (-2048, 2047, rshift_rnd_sf((iTRb * prescale), 6));
            //prescale = (iTRb * prescale + 32) >> 6;
          }
          else      // This could not happen but lets use it for case that reference is removed.
            prescale = 256;
          epzs_scale[1][j][i] = rshift_rnd_sf((mv_scale[j][1][i] * prescale), 8);
          epzs_scale[1][j + 1][i] = prescale - 256;
        }
        else
        {
          epzs_scale[1][j][i] = epzs_scale[0][j][i];
          epzs_scale[1][j + 1][i] = epzs_scale[0][j + 1][i];
        }
      }
    }
    if (img->MbaffFrameFlag)
    {
      fs_top = listX[list + 2][0];
      fs_bottom = listX[list + 4][0];
      if (listXsize[0]> 1)
      {
        fs_top1 = listX[list + 2][1];
        fs_bottom = listX[list + 4][1];
      }
    }
    else
    {
      if (img->structure != FRAME)
      {
        if ((img->structure != fs->structure) && (fs->coded_frame))
        {
          if (img->structure == TOP_FIELD)
          {
            fs_top = fs_bottom = fs = listX[list][0]->top_field;
            fs_top1 = fs_bottom1 = fs1 = listX[list][0]->bottom_field;
          }
          else
          {
            fs_top = fs_bottom = fs = listX[list][0]->bottom_field;
            fs_top1 = fs_bottom1 = fs1 = listX[list][0]->top_field;
          }
        }
      }
    }

    if (!active_sps->frame_mbs_only_flag)
    {
      if (img->MbaffFrameFlag)
      {
        for (j = 0; j < fs->size_y >> 2; j++)
        {
          jj = j >> 1;
          jdiv = jj + 4 * (j >> 3);

          for (i = 0; i < fs->size_x >> 2; i++)
          {
            if (fs->motion.field_frame[j][i])
            {
              //! Assign frame buffers for field MBs
              //! Check whether we should use top or bottom field mvs.
              //! Depending on the assigned poc values.
              if (iabs (enc_picture->poc - fs_bottom->poc) > iabs (enc_picture->poc - fs_top->poc))
              {
                tempmv_scale[LIST_0] = 256;
                tempmv_scale[LIST_1] = 0;

                if (fs->motion.ref_id [LIST_0][jdiv][i] < 0 && listXsize[LIST_0] > 1)
                {
                  fsx = fs_top1;
                  loffset = 1;
                }
                else
                {
                  fsx = fs_top;
                  loffset = 0;
                }

                if (fs->motion.ref_id [LIST_0][jdiv][i] != -1)
                {
                  for (iref = 0; iref < imin(img->num_ref_idx_l0_active,listXsize[LIST_0]); iref++)
                  {
                    if (enc_picture->ref_pic_num[LIST_0][iref]==fs->motion.ref_id [LIST_0][jdiv][i])
                    {
                      tempmv_scale[LIST_0] = epzs_scale[loffset][LIST_0][iref];
                      tempmv_scale[LIST_1] = epzs_scale[loffset][LIST_1][iref];
                      break;
                    }
                  }
                  p->frame[LIST_0][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->motion.mv[LIST_0][jj][i][0]), invmv_precision));
                  p->frame[LIST_0][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->motion.mv[LIST_0][jj][i][1]), invmv_precision));
                  p->frame[LIST_1][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->motion.mv[LIST_0][jj][i][0]), invmv_precision));
                  p->frame[LIST_1][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->motion.mv[LIST_0][jj][i][1]), invmv_precision));
                }
                else
                {
                  p->frame[LIST_0][j][i].mv_x = 0;
                  p->frame[LIST_0][j][i].mv_y = 0;
                  p->frame[LIST_1][j][i].mv_x = 0;
                  p->frame[LIST_1][j][i].mv_y = 0;
                }
              }
              else
              {
                tempmv_scale[LIST_0] = 256;
                tempmv_scale[LIST_1] = 0;

                if (fs->motion.ref_id [LIST_0][jdiv + 4][i] < 0 && listXsize[LIST_0] > 1)
                {
                  fsx = fs_bottom1;
                  loffset = 1;
                }
                else
                {
                  fsx = fs_bottom;
                  loffset = 0;
                }

                if (fs->motion.ref_id [LIST_0][jdiv + 4][i] != -1)
                {
                  for (iref = 0; iref < imin(img->num_ref_idx_l0_active,listXsize[LIST_0]); iref++)
                  {
                    if (enc_picture->ref_pic_num[LIST_0][iref]==fs->motion.ref_id [LIST_0][jdiv + 4][i])
                    {
                      tempmv_scale[LIST_0] = epzs_scale[loffset][LIST_0][iref];
                      tempmv_scale[LIST_1] = epzs_scale[loffset][LIST_1][iref];
                      break;
                    }
                  }
                  p->frame[LIST_0][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->motion.mv[LIST_0][jj][i][0]), invmv_precision));
                  p->frame[LIST_0][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->motion.mv[LIST_0][jj][i][1]), invmv_precision));
                  p->frame[LIST_1][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->motion.mv[LIST_0][jj][i][0]), invmv_precision));
                  p->frame[LIST_1][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->motion.mv[LIST_0][jj][i][1]), invmv_precision));
                }
                else
                {
                  p->frame[LIST_0][j][i].mv_x = 0;
                  p->frame[LIST_0][j][i].mv_y = 0;
                  p->frame[LIST_1][j][i].mv_x = 0;
                  p->frame[LIST_1][j][i].mv_y = 0;
                }
              }
            }
            else
            {
              tempmv_scale[LIST_0] = 256;
              tempmv_scale[LIST_1] = 0;
              if (fs->motion.ref_id [LIST_0][j][i] < 0 && listXsize[LIST_0] > 1)
              {
                fsx = fs1;
                loffset = 1;
              }
              else
              {
                fsx = fs;
                loffset = 0;
              }

              if (fsx->motion.ref_id [LIST_0][j][i] != -1)
              {
                for (iref = 0; iref < imin(img->num_ref_idx_l0_active,listXsize[LIST_0]); iref++)
                {
                  if (enc_picture->ref_pic_num[LIST_0][iref]==fsx->motion.ref_id [LIST_0][j][i])
                  {
                    tempmv_scale[LIST_0] = epzs_scale[loffset][LIST_0][iref];
                    tempmv_scale[LIST_1] = epzs_scale[loffset][LIST_1][iref];
                    break;
                  }
                }
                p->frame[LIST_0][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->motion.mv[LIST_0][j][i][0]), invmv_precision));
                p->frame[LIST_0][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->motion.mv[LIST_0][j][i][1]), invmv_precision));
                p->frame[LIST_1][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->motion.mv[LIST_0][j][i][0]), invmv_precision));
                p->frame[LIST_1][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->motion.mv[LIST_0][j][i][1]), invmv_precision));
              }
              else
              {
                p->frame[LIST_0][j][i].mv_x = 0;
                p->frame[LIST_0][j][i].mv_y = 0;
                p->frame[LIST_1][j][i].mv_x = 0;
                p->frame[LIST_1][j][i].mv_y = 0;
              }
            }
          }
        }
      }
      else
      {
        for (j = 0; j < fs->size_y >> 2; j++)
        {
          jj = j >> 1;
          jdiv = jj + 4 * (j >> 3);

          for (i = 0; i < fs->size_x >> 2; i++)
          {
            tempmv_scale[LIST_0] = 256;
            tempmv_scale[LIST_1] = 0;
            if (fs->motion.ref_id [LIST_0][j][i] < 0 && listXsize[LIST_0] > 1)
            {
              fsx = fs1;
              loffset = 1;
            }
            else
            {
              fsx = fs;
              loffset = 0;
            }

            if (fsx->motion.ref_id [LIST_0][j][i] != -1)
            {
              for (iref = 0; iref < imin(img->num_ref_idx_l0_active,listXsize[LIST_0]); iref++)
              {
                if (enc_picture->ref_pic_num[LIST_0][iref]==fsx->motion.ref_id [LIST_0][j][i])
                {
                  tempmv_scale[LIST_0] = epzs_scale[loffset][LIST_0][iref];
                  tempmv_scale[LIST_1] = epzs_scale[loffset][LIST_1][iref];
                  break;
                }
              }
              p->frame[LIST_0][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->motion.mv[LIST_0][j][i][0]), invmv_precision));
              p->frame[LIST_0][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->motion.mv[LIST_0][j][i][1]), invmv_precision));
              p->frame[LIST_1][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->motion.mv[LIST_0][j][i][0]), invmv_precision));
              p->frame[LIST_1][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->motion.mv[LIST_0][j][i][1]), invmv_precision));
            }
            else
            {
              p->frame[LIST_0][j][i].mv_x = 0;
              p->frame[LIST_0][j][i].mv_y = 0;
              p->frame[LIST_1][j][i].mv_x = 0;
              p->frame[LIST_1][j][i].mv_y = 0;
            }
          }
        }
      }
      
      //! Generate field MVs from Frame MVs
      if (img->structure || img->MbaffFrameFlag)
      {
        for (j = 0; j < fs->size_y / 8; j++)
        {
          for (i = 0; i < fs->size_x / 4; i++)
          {
            if (!img->MbaffFrameFlag)
            {
              tempmv_scale[LIST_0] = 256;
              tempmv_scale[LIST_1] = 0;

              if (fs->motion.ref_id [LIST_0][j][i] < 0 && listXsize[LIST_0] > 1)
              {
                fsx = fs1;
                loffset = 1;
              }
              else
              {
                fsx = fs;
                loffset = 0;
              }

              if (fsx->motion.ref_id [LIST_0][j][i] != -1)
              {
                for (iref = 0; iref < imin(img->num_ref_idx_l0_active,listXsize[LIST_0]); iref++)
                {
                  if (enc_picture->ref_pic_num[LIST_0][iref]==fsx->motion.ref_id [LIST_0][j][i])
                  {
                    tempmv_scale[LIST_0] = epzs_scale[loffset][LIST_0][iref];
                    tempmv_scale[LIST_1] = epzs_scale[loffset][LIST_1][iref];
                    break;
                  }
                }
                p->frame[LIST_0][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->motion.mv[LIST_0][j][i][0]), invmv_precision));
                p->frame[LIST_0][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->motion.mv[LIST_0][j][i][1]), invmv_precision));
                p->frame[LIST_1][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->motion.mv[LIST_0][j][i][0]), invmv_precision));
                p->frame[LIST_1][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->motion.mv[LIST_0][j][i][1]), invmv_precision));
              }
              else
              {
                p->frame[LIST_0][j][i].mv_x = 0;
                p->frame[LIST_0][j][i].mv_y = 0;
                p->frame[LIST_1][j][i].mv_x = 0;
                p->frame[LIST_1][j][i].mv_y = 0;
              }
            }
            else
            {
              tempmv_scale[LIST_0] = 256;
              tempmv_scale[LIST_1] = 0;

              if (fs_bottom->motion.ref_id [LIST_0][j][i] < 0 && listXsize[LIST_0] > 1)
              {
                fsx = fs_bottom1;
                loffset = 1;
              }
              else
              {
                fsx = fs_bottom;
                loffset = 0;
              }

              if (fsx->motion.ref_id [LIST_0][j][i] != -1)
              {
                for (iref = 0; iref < imin(2*img->num_ref_idx_l0_active,listXsize[LIST_0 + 4]); iref++)
                {
                  if (enc_picture->ref_pic_num[LIST_0 + 4][iref]==fsx->motion.ref_id [LIST_0][j][i])
                  {
                    tempmv_scale[LIST_0] = epzs_scale[loffset][LIST_0 + 4][iref];
                    tempmv_scale[LIST_1] = epzs_scale[loffset][LIST_1 + 4][iref];
                    break;
                  }
                }
                p->bot[LIST_0][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->motion.mv[LIST_0][j][i][0]), invmv_precision));
                p->bot[LIST_0][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->motion.mv[LIST_0][j][i][1]), invmv_precision));
                p->bot[LIST_1][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->motion.mv[LIST_0][j][i][0]), invmv_precision));
                p->bot[LIST_1][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->motion.mv[LIST_0][j][i][1]), invmv_precision));
              }
              else
              {
                p->bot[LIST_0][j][i].mv_x = 0;
                p->bot[LIST_0][j][i].mv_y = 0;
                p->bot[LIST_1][j][i].mv_x = 0;
                p->bot[LIST_1][j][i].mv_y = 0;
              }

              if (!fs->motion.field_frame[2 * j][i])
              {
                p->bot[LIST_0][j][i].mv_y  = (p->bot[LIST_0][j][i].mv_y + 1) >> 1;
                p->bot[LIST_1][j][i].mv_y  = (p->bot[LIST_1][j][i].mv_y + 1) >> 1;
              }

              tempmv_scale[LIST_0] = 256;
              tempmv_scale[LIST_1] = 0;

              if (fs_top->motion.ref_id [LIST_0][j][i] < 0 && listXsize[LIST_0] > 1)
              {
                fsx = fs_top1;
                loffset = 1;
              }
              else
              {
                fsx = fs_top;
                loffset = 0;
              }

              if (fsx->motion.ref_id [LIST_0][j][i] != -1)
              {
                for (iref = 0; iref < imin(2*img->num_ref_idx_l0_active,listXsize[LIST_0 + 2]); iref++)
                {
                  if (enc_picture->ref_pic_num[LIST_0 + 2][iref]==fsx->motion.ref_id [LIST_0][j][i])
                  {
                    tempmv_scale[LIST_0] = epzs_scale[loffset][LIST_0 + 2][iref];
                    tempmv_scale[LIST_1] = epzs_scale[loffset][LIST_1 + 2][iref];
                    break;
                  }
                }
                p->top[LIST_0][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->motion.mv[LIST_0][j][i][0]), invmv_precision));
                p->top[LIST_0][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->motion.mv[LIST_0][j][i][1]), invmv_precision));
                p->top[LIST_1][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->motion.mv[LIST_0][j][i][0]), invmv_precision));
                p->top[LIST_1][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->motion.mv[LIST_0][j][i][1]), invmv_precision));
              }
              else
              {
                p->top[LIST_0][j][i].mv_x = 0;
                p->top[LIST_0][j][i].mv_y = 0;
                p->top[LIST_1][j][i].mv_x = 0;
                p->top[LIST_1][j][i].mv_y = 0;
              }

              if (!fs->motion.field_frame[2 * j][i])
              {
                p->top[LIST_0][j][i].mv_y  = (p->top[LIST_0][j][i].mv_y + 1) >> 1;
                p->top[LIST_1][j][i].mv_y  = (p->top[LIST_1][j][i].mv_y + 1) >> 1;
              }
            }
          }
        }
      }
    }

    //if (!active_sps->frame_mbs_only_flag || active_sps->direct_8x8_inference_flag)
    if (!active_sps->frame_mbs_only_flag )
    {
      //! Use inference flag to remap mvs/references
      //! Frame with field co-located
      if (!img->structure)
      {
        for (j = 0; j < fs->size_y >> 2; j++)
        {
          jj = j>>1;
          jdiv = (j>>1) + ((j>>3) << 2);
          for (i = 0; i < fs->size_x >> 2; i++)
          {
            if (fs->motion.field_frame[j][i])
            {
              tempmv_scale[LIST_0] = 256;
              tempmv_scale[LIST_1] = 0;

              if (fs->motion.ref_id [LIST_0][jdiv][i] < 0 && listXsize[LIST_0] > 1)
              {
                fsx = fs1;
                loffset = 1;
              }
              else
              {
                fsx = fs;
                loffset = 0;
              }
              if (fsx->motion.ref_id [LIST_0][jdiv][i] != -1)
              {
                for (iref = 0; iref < imin(img->num_ref_idx_l0_active,listXsize[LIST_0]); iref++)
                {
                  if (enc_picture->ref_pic_num[LIST_0][iref]==fsx->motion.ref_id [LIST_0][jdiv][i])
                  {
                    tempmv_scale[LIST_0] = epzs_scale[loffset][LIST_0][iref];
                    tempmv_scale[LIST_1] = epzs_scale[loffset][LIST_1][iref];
                    break;
                  }
                }

                if (iabs (enc_picture->poc - fsx->bottom_field->poc) > iabs (enc_picture->poc - fsx->top_field->poc))
                {
                  p->frame[LIST_0][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->top_field->motion.mv[LIST_0][jj][i][0]), invmv_precision));
                  p->frame[LIST_0][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->top_field->motion.mv[LIST_0][jj][i][1]), invmv_precision));
                  p->frame[LIST_1][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->top_field->motion.mv[LIST_0][jj][i][0]), invmv_precision));
                  p->frame[LIST_1][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->top_field->motion.mv[LIST_0][jj][i][1]), invmv_precision));
                }
                else
                {
                  p->frame[LIST_0][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->bottom_field->motion.mv[LIST_0][jj][i][0]), invmv_precision));
                  p->frame[LIST_0][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->bottom_field->motion.mv[LIST_0][jj][i][1]), invmv_precision));
                  p->frame[LIST_1][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->bottom_field->motion.mv[LIST_0][jj][i][0]), invmv_precision));
                  p->frame[LIST_1][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->bottom_field->motion.mv[LIST_0][jj][i][1]), invmv_precision));
                }
              }
              else
              {
                p->frame[LIST_0][j][i].mv_x = 0;
                p->frame[LIST_0][j][i].mv_y = 0;
                p->frame[LIST_1][j][i].mv_x = 0;
                p->frame[LIST_1][j][i].mv_y = 0;
              }
            }
          }
        }
      }
    }

    if (active_sps->frame_mbs_only_flag)
    {
      for (j = 0; j < fs->size_y >> 2; j++)
      {
        for (i = 0; i < fs->size_x >> 2; i++)
        {
          tempmv_scale[LIST_0] = 256;
          tempmv_scale[LIST_1] = 0;
          if (fs->motion.ref_id [LIST_0][j][i] < 0 && listXsize[LIST_0] > 1)
          {
            fsx = fs1;
            loffset = 1;
          }
          else
          {
            fsx = fs;
            loffset = 0;
          }
         
          if (fsx->motion.ref_id [LIST_0][j][i] != -1)
          {
            for (iref = 0; iref < imin(img->num_ref_idx_l0_active,listXsize[LIST_0]); iref++)
            {
              if (enc_picture->ref_pic_num[LIST_0][iref]==fsx->motion.ref_id [LIST_0][j][i])
              {
                tempmv_scale[LIST_0] = epzs_scale[loffset][LIST_0][iref];
                tempmv_scale[LIST_1] = epzs_scale[loffset][LIST_1][iref];
                break;
              }
            }

            p->frame[LIST_0][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->motion.mv[LIST_0][j][i][0]), invmv_precision));
            p->frame[LIST_0][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_0] * fsx->motion.mv[LIST_0][j][i][1]), invmv_precision));
            p->frame[LIST_1][j][i].mv_x = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->motion.mv[LIST_0][j][i][0]), invmv_precision));
            p->frame[LIST_1][j][i].mv_y = iClip3 (-32768, 32767, rshift_rnd_sf((tempmv_scale[LIST_1] * fsx->motion.mv[LIST_0][j][i][1]), invmv_precision));
          }
          else
          {
            p->frame[LIST_0][j][i].mv_x = 0;
            p->frame[LIST_0][j][i].mv_y = 0;
            p->frame[LIST_1][j][i].mv_x = 0;
            p->frame[LIST_1][j][i].mv_y = 0;
          }
        }
      }
    }

    if (!active_sps->frame_mbs_only_flag)
    {
      for (j = 0; j < fs->size_y >> 2; j++)
      {
        for (i = 0; i < fs->size_x >> 2; i++)
        {
          if ((!img->MbaffFrameFlag && !img->structure && fs->motion.field_frame[j][i]) || (img->MbaffFrameFlag && fs->motion.field_frame[j][i]))
          {
            p->frame[LIST_0][j][i].mv_y *= 2; 
            p->frame[LIST_1][j][i].mv_y *= 2;
          }
          else if (img->structure && !fs->motion.field_frame[j][i])
          {
            p->frame[LIST_0][j][i].mv_y = rshift_rnd_sf(p->frame[LIST_0][j][i].mv_y, 1);
            p->frame[LIST_1][j][i].mv_y = rshift_rnd_sf(p->frame[LIST_1][j][i].mv_y, 1);
          }
        }
      }
    }
  }
}

/*!
***********************************************************************
* \brief
*    Spatial Predictors
*    AMT/HYC
***********************************************************************
*/
static short EPZSSpatialPredictors (PixelPos block_a,
                                    PixelPos block_b,
                                    PixelPos block_c,
                                    PixelPos block_d,
                                    int list,
                                    int list_offset,
                                    short ref,
                                    char **refPic,
                                    short ***tmp_mv,
                                    EPZSStructure * predictor)
{
  int refA, refB, refC, refD;
  int *mot_scale = mv_scale[list + list_offset][ref];
  short sp_shift_mv = 8 + mv_rescale;
  int   fixed_mv = (12 >> mv_rescale);

  // zero predictor
  predictor->point[0].motion.mv_x = 0;
  predictor->point[0].motion.mv_y = 0;

  // Non MB-AFF mode
  if (!img->MbaffFrameFlag)
  {
    refA = block_a.available ? (int) refPic[block_a.pos_y][block_a.pos_x] : -1;
    refB = block_b.available ? (int) refPic[block_b.pos_y][block_b.pos_x] : -1;
    refC = block_c.available ? (int) refPic[block_c.pos_y][block_c.pos_x] : -1;
    refD = block_d.available ? (int) refPic[block_d.pos_y][block_d.pos_x] : -1;

    // Left Predictor
    if (block_a.available)
    {
      predictor->point[1].motion.mv_x = rshift_rnd_sf((mot_scale[refA] * tmp_mv[block_a.pos_y][block_a.pos_x][0]), sp_shift_mv);
      predictor->point[1].motion.mv_y = rshift_rnd_sf((mot_scale[refA] * tmp_mv[block_a.pos_y][block_a.pos_x][1]), sp_shift_mv);
    }
    else
    {
      predictor->point[1].motion.mv_x = fixed_mv;
      predictor->point[1].motion.mv_y = 0;
    }
    // Up predictor
    if (block_b.available)
    {
      predictor->point[2].motion.mv_x = rshift_rnd_sf((mot_scale[refB] * tmp_mv[block_b.pos_y][block_b.pos_x][0]), sp_shift_mv);
      predictor->point[2].motion.mv_y = rshift_rnd_sf((mot_scale[refB] * tmp_mv[block_b.pos_y][block_b.pos_x][1]), sp_shift_mv);
    }
    else
    {
      predictor->point[2].motion.mv_x = 0;
      predictor->point[2].motion.mv_y = fixed_mv;
    }

    // Up-Right predictor
    if (block_c.available)
    {
      predictor->point[3].motion.mv_x = rshift_rnd_sf((mot_scale[refC] * tmp_mv[block_c.pos_y][block_c.pos_x][0]), sp_shift_mv);
      predictor->point[3].motion.mv_y = rshift_rnd_sf((mot_scale[refC] * tmp_mv[block_c.pos_y][block_c.pos_x][1]), sp_shift_mv);
    }
    else
    {
      predictor->point[3].motion.mv_x = -fixed_mv;
      predictor->point[3].motion.mv_y = 0;
    }

    //Up-Left predictor
    if (block_d.available)
    {
      predictor->point[4].motion.mv_x = rshift_rnd_sf((mot_scale[refD] * tmp_mv[block_d.pos_y][block_d.pos_x][0]), sp_shift_mv);
      predictor->point[4].motion.mv_y = rshift_rnd_sf((mot_scale[refD] * tmp_mv[block_d.pos_y][block_d.pos_x][1]), sp_shift_mv);
    }
    else
    {
      predictor->point[4].motion.mv_x = 0;
      predictor->point[4].motion.mv_y = -fixed_mv;
    }
  }
  else  // MB-AFF mode
  {
    // Field Macroblock
    if (list_offset)
    {
      refA = block_a.available
        ? img->mb_data[block_a.mb_addr].mb_field
        ? (int) refPic[block_a.pos_y][block_a.pos_x]
        : (int) refPic[block_a.pos_y][block_a.pos_x] * 2 : -1;
      refB =block_b.available
        ? img->mb_data[block_b.mb_addr].mb_field
        ? (int) refPic[block_b.pos_y][block_b.pos_x]
        : (int) refPic[block_b.pos_y][block_b.pos_x] * 2 : -1;
      refC = block_c.available
        ? img->mb_data[block_c.mb_addr].mb_field
        ? (int) refPic[block_c.pos_y][block_c.pos_x]
        : (int) refPic[block_c.pos_y][block_c.pos_x] * 2 : -1;
      refD = block_d.available
        ? img->mb_data[block_d.mb_addr].mb_field
        ? (int) refPic[block_d.pos_y][block_d.pos_x]
        : (int) refPic[block_d.pos_y][block_d.pos_x] * 2 : -1;

      // Left Predictor
      predictor->point[1].motion.mv_x = (block_a.available)
        ? rshift_rnd_sf((mot_scale[refA] * tmp_mv[block_a.pos_y][block_a.pos_x][0]), sp_shift_mv) :  fixed_mv;
      predictor->point[1].motion.mv_y = (block_a.available)
        ? img->mb_data[block_a.mb_addr].mb_field
        ? rshift_rnd_sf((mot_scale[refA] * tmp_mv[block_a.pos_y][block_a.pos_x][1]), sp_shift_mv)
        : rshift_rnd_sf((mot_scale[refA] * tmp_mv[block_a.pos_y][block_a.pos_x][1]), sp_shift_mv + 1) :  0;

      // Up predictor
      predictor->point[2].motion.mv_x = (block_b.available)
        ? rshift_rnd_sf((mot_scale[refB] * tmp_mv[block_b.pos_y][block_b.pos_x][0]), sp_shift_mv) : 0;
      predictor->point[2].motion.mv_y = (block_b.available)
        ? img->mb_data[block_b.mb_addr].mb_field
        ? rshift_rnd_sf((mot_scale[refB] * tmp_mv[block_b.pos_y][block_b.pos_x][1]), sp_shift_mv)
        : rshift_rnd_sf((mot_scale[refB] * tmp_mv[block_b.pos_y][block_b.pos_x][1]), sp_shift_mv + 1) : fixed_mv;

      // Up-Right predictor
      predictor->point[3].motion.mv_x = (block_c.available)
        ? rshift_rnd_sf((mot_scale[refC] * tmp_mv[block_c.pos_y][block_c.pos_x][0]), sp_shift_mv) : -fixed_mv;
      predictor->point[3].motion.mv_y = (block_c.available)
        ? img->mb_data[block_c.mb_addr].mb_field
        ? rshift_rnd_sf((mot_scale[refC] * tmp_mv[block_c.pos_y][block_c.pos_x][1]), sp_shift_mv)
        : rshift_rnd_sf((mot_scale[refC] * tmp_mv[block_c.pos_y][block_c.pos_x][1]), sp_shift_mv + 1) : 0;

      //Up-Left predictor
      predictor->point[4].motion.mv_x = (block_d.available)
        ? rshift_rnd_sf((mot_scale[refD] * tmp_mv[block_d.pos_y][block_d.pos_x][0]), sp_shift_mv) : 0;
      predictor->point[4].motion.mv_y = (block_d.available)
        ? img->mb_data[block_d.mb_addr].mb_field
        ? rshift_rnd_sf((mot_scale[refD] * tmp_mv[block_d.pos_y][block_d.pos_x][1]), sp_shift_mv)
        : rshift_rnd_sf((mot_scale[refD] * tmp_mv[block_d.pos_y][block_d.pos_x][1]), sp_shift_mv + 1) : -fixed_mv;
    }
    else // Frame macroblock
    {
      refA = block_a.available
        ? img->mb_data[block_a.mb_addr].mb_field
        ? (int) refPic[block_a.pos_y][block_a.pos_x] >> 1
        : (int) refPic[block_a.pos_y][block_a.pos_x] : -1;
      refB = block_b.available
        ? img->mb_data[block_b.mb_addr].mb_field
        ? (int) refPic[block_b.pos_y][block_b.pos_x] >> 1
        : (int) refPic[block_b.pos_y][block_b.pos_x] : -1;
      refC = block_c.available
        ? img->mb_data[block_c.mb_addr].mb_field
        ? (int) refPic[block_c.pos_y][block_c.pos_x] >> 1
        : (int) refPic[block_c.pos_y][block_c.pos_x] : -1;
      refD = block_d.available
        ? img->mb_data[block_d.mb_addr].mb_field
        ? (int) refPic[block_d.pos_y][block_d.pos_x] >> 1
        : (int) refPic[block_d.pos_y][block_d.pos_x] : -1;

      // Left Predictor
      predictor->point[1].motion.mv_x = (block_a.available)
        ? rshift_rnd_sf((mot_scale[refA] * tmp_mv[block_a.pos_y][block_a.pos_x][0]), sp_shift_mv) : fixed_mv;
      predictor->point[1].motion.mv_y = (block_a.available)
        ? img->mb_data[block_a.mb_addr].mb_field
        ? rshift_rnd_sf((mot_scale[refA] * tmp_mv[block_a.pos_y][block_a.pos_x][1]), sp_shift_mv - 1)
        : rshift_rnd_sf((mot_scale[refA] * tmp_mv[block_a.pos_y][block_a.pos_x][1]), sp_shift_mv) : 0;

      // Up predictor
      predictor->point[2].motion.mv_x = (block_b.available)
        ? rshift_rnd_sf((mot_scale[refB] * tmp_mv[block_b.pos_y][block_b.pos_x][0]), sp_shift_mv) : 0;
      predictor->point[2].motion.mv_y = (block_b.available)
        ? img->mb_data[block_b.mb_addr].mb_field
        ? rshift_rnd_sf((mot_scale[refB] * tmp_mv[block_b.pos_y][block_b.pos_x][1]), sp_shift_mv - 1)
        : rshift_rnd_sf((mot_scale[refB] * tmp_mv[block_b.pos_y][block_b.pos_x][1]), sp_shift_mv) :  fixed_mv;

      // Up-Right predictor
      predictor->point[3].motion.mv_x = (block_c.available)
        ? rshift_rnd_sf((mot_scale[refC] * tmp_mv[block_c.pos_y][block_c.pos_x][0]), sp_shift_mv) : -fixed_mv;
      predictor->point[3].motion.mv_y = (block_c.available)
        ? img->mb_data[block_c.mb_addr].mb_field
        ? rshift_rnd_sf((mot_scale[refC] * tmp_mv[block_c.pos_y][block_c.pos_x][1]), sp_shift_mv - 1)
        : rshift_rnd_sf((mot_scale[refC] * tmp_mv[block_c.pos_y][block_c.pos_x][1]), sp_shift_mv) : 0;

      //Up-Left predictor
      predictor->point[4].motion.mv_x = (block_d.available)
        ? rshift_rnd_sf((mot_scale[refD] * tmp_mv[block_d.pos_y][block_d.pos_x][0]), sp_shift_mv) : 0;
      predictor->point[4].motion.mv_y = (block_d.available)
        ? img->mb_data[block_d.mb_addr].mb_field
        ? rshift_rnd_sf((mot_scale[refD] * tmp_mv[block_d.pos_y][block_d.pos_x][1]), sp_shift_mv - 1)
        : rshift_rnd_sf((mot_scale[refD] * tmp_mv[block_d.pos_y][block_d.pos_x][1]), sp_shift_mv) : -fixed_mv;
    }
  }

  return ((refA == -1) + (refB == -1) + (refC == -1 && refD == -1));
}

/*!
***********************************************************************
* \brief
*    Spatial Predictors
*    AMT/HYC
***********************************************************************
*/

static void EPZSSpatialMemPredictors (int list,
                                      short ref,
                                      int blocktype,
                                      int pic_x,
                                      int bs_x,
                                      int bs_y,
                                      int by,
                                      int *prednum,
                                      int img_width,
                                      EPZSStructure * predictor)
{
#if EPZSREF
  short ***mv = EPZSMotion[list][ref][blocktype];
  //MotionVector ***prd_mv = &EPZS_Motion[list][ref][blocktype];
  MotionVector *cur_mv = &predictor->point[*prednum].motion;

  // Left Predictor
  if (pic_x > 0)
  {
    cur_mv->mv_x = mv[by][pic_x - bs_x][0];
    cur_mv->mv_y = mv[by][pic_x - bs_x][1];
    *prednum += (*((int*) cur_mv) != 0);
    cur_mv = &predictor->point[*prednum].motion;
  }
  
  by = (by > 0) ? by - bs_y: 4 - bs_y;
  // Up predictor  
  cur_mv->mv_x = mv[by][pic_x][0];
  cur_mv->mv_y = mv[by][pic_x][1];
  *prednum += (*((int*) cur_mv) != 0);

  // Up-Right predictor
  if (pic_x + bs_x < img_width)
  {
    cur_mv = &predictor->point[*prednum].motion;
    cur_mv->mv_x = mv[by][pic_x + bs_x][0];
    cur_mv->mv_y = mv[by][pic_x + bs_x][1];
    *prednum += (*((int*) cur_mv) != 0);
  }

#else
  int mot_scale = mv_scale[list][ref][0];
  short **mv = EPZSMotion[list][blocktype];
  MotionVector **predictor = &EPZS_Motion[list][blocktype];

  // Left Predictor
  predictor->point[*prednum].motion.mv_x = (pic_x > 0)
    ? rshift_rnd_sf((mot_scale * mv[by][pic_x - bs_x][0]), 8)
    : 0;
  predictor->point[*prednum].motion.mv_y = (pic_x > 0)
    ? rshift_rnd_sf((mot_scale * mv[by][pic_x - bs_x][1]), 8)
    : 0;
  *prednum += ((predictor->point[*prednum].motion.mv_x != 0) || (predictor->point[*prednum].motion.mv_y != 0));

  // Up predictor
  predictor->point[*prednum].motion.mv_x = (by > 0)
    ? rshift_rnd_sf((mot_scale * mv[by - bs_y][pic_x][0]), 8)
    : rshift_rnd_sf((mot_scale * mv[4  - bs_y][pic_x][0]), 8);
  predictor->point[*prednum].motion.mv_y = (by > 0)
    ? rshift_rnd_sf((mot_scale * mv[by - bs_y][pic_x][1]), 8)
    : rshift_rnd_sf((mot_scale * mv[4  - bs_y][pic_x][1]), 8);
  *prednum += ((predictor->point[*prednum].motion.mv_x != 0) || (predictor->point[*prednum].motion.mv_y != 0));

  // Up-Right predictor
  predictor->point[*prednum].motion.mv_x = (pic_x + bs_x < img_width)
    ? (by > 0)
    ? rshift_rnd_sf((mot_scale * mv[by - bs_y][pic_x + bs_x][0]), 8)
    : rshift_rnd_sf((mot_scale * mv[4  - bs_y][pic_x + bs_x][0]), 8)
    : 0;
  predictor->point[*prednum].motion.mv_y = (pic_x + bs_x < img_width)
    ? (by > 0)
    ? rshift_rnd_sf((mot_scale * mv[by - bs_y][pic_x + bs_x][1]), 8)
    : rshift_rnd_sf((mot_scale * mv[4  - bs_y][pic_x + bs_x][1]), 8)
    : 0;
  *prednum += ((predictor->point[*prednum].motion.mv_x != 0) || (predictor->point[*prednum].motion.mv_y != 0));
#endif
}


/*!
***********************************************************************
* \brief
*    Temporal Predictors
*    AMT/HYC
***********************************************************************
*/
static void
EPZSTemporalPredictors (int list,         // <--  current list
                        int list_offset,  // <--  list offset for MBAFF
                        short ref,        // <--  current reference frame
                        int o_block_x,    // <--  absolute x-coordinate of regarded AxB block
                        int o_block_y,    // <--  absolute y-coordinate of regarded AxB block
                        EPZSStructure * predictor,
                        int *prednum,
                        int block_available_left,
                        int block_available_up,
                        int block_available_right,
                        int block_available_below,
                        int blockshape_x,
                        int blockshape_y,
                        int stopCriterion,
                        int min_mcost)
{
  int mvScale = mv_scale[list + list_offset][ref][0];
  MotionVector **col_mv = (list_offset == 0) ? EPZSCo_located->frame[list]  
    : (list_offset == 2) ? EPZSCo_located->top[list] : EPZSCo_located->bot[list];
  short temp_shift_mv = 8 + mv_rescale; // 16 - invmv_precision + mv_rescale
  MotionVector *cur_mv = &predictor->point[*prednum].motion;

  *prednum += add_predictor(cur_mv, col_mv[o_block_y][o_block_x], mvScale, temp_shift_mv);

  if (min_mcost > stopCriterion && ref < 2)
  {
    if (block_available_left)
    {
      *prednum += add_predictor( &predictor->point[*prednum].motion, col_mv[o_block_y][o_block_x - 1], mvScale, temp_shift_mv);

      //Up_Left
      if (block_available_up)
      {
        *prednum += add_predictor( &predictor->point[*prednum].motion, col_mv[o_block_y - 1][o_block_x - 1], mvScale, temp_shift_mv);
      }
      //Down_Left
      if (block_available_below)
      {
        *prednum += add_predictor( &predictor->point[*prednum].motion, col_mv[o_block_y + blockshape_y][o_block_x - 1], mvScale, temp_shift_mv);
      }
    }
    // Up
    if (block_available_up)
    {
      *prednum += add_predictor( &predictor->point[*prednum].motion, col_mv[o_block_y - 1][o_block_x], mvScale, temp_shift_mv);
    }

    // Up - Right
    if (block_available_right)
    {
      *prednum += add_predictor( &predictor->point[*prednum].motion, col_mv[o_block_y][o_block_x + blockshape_x], mvScale, temp_shift_mv);

      if (block_available_up)
      {
        *prednum += add_predictor( &predictor->point[*prednum].motion, col_mv[o_block_y - 1][o_block_x + blockshape_x], mvScale, temp_shift_mv);
      }
      if (block_available_below)
      {
        *prednum += add_predictor( &predictor->point[*prednum].motion, col_mv[o_block_y + blockshape_y][o_block_x + blockshape_x], mvScale, temp_shift_mv);
      }
    }

    if (block_available_below)
    {
      *prednum += add_predictor( &predictor->point[*prednum].motion, col_mv[o_block_y + blockshape_y][o_block_x], mvScale, temp_shift_mv);
    }
  }
}

/*!
************************************************************************
* \brief
*    EPZS Block Type Predictors
************************************************************************
*/
static void EPZSBlockTypePredictors (int block_x, int block_y, int blocktype, int ref, int list,
                                     EPZSStructure * predictor, int *prednum)
{
  short *****all_mv = img->all_mv[list];
  short block_shift_mv = 8 + mv_rescale;
  MotionVector *cur_mv = &predictor->point[*prednum].motion;

  cur_mv->mv_x = rshift_rnd((all_mv[ref][blk_parent[blocktype]][block_y][block_x][0]), mv_rescale);
  cur_mv->mv_y = rshift_rnd((all_mv[ref][blk_parent[blocktype]][block_y][block_x][1]), mv_rescale);
  //*prednum += ((cur_mv->mv_x | cur_mv->mv_y) != 0);
  *prednum += (*((int*) cur_mv) != 0);

  if ((ref > 0) && (blocktype < 5 || img->structure != FRAME))
  {
    cur_mv = &predictor->point[*prednum].motion;
    cur_mv->mv_x = rshift_rnd_sf((mv_scale[list][ref][ref-1] * all_mv[ref-1][blocktype][block_y][block_x][0]), block_shift_mv );
    cur_mv->mv_y = rshift_rnd_sf((mv_scale[list][ref][ref-1] * all_mv[ref-1][blocktype][block_y][block_x][1]), block_shift_mv );
    //*prednum += ((cur_mv->mv_x | cur_mv->mv_y) != 0);
    *prednum += (*((int*) cur_mv) != 0);


    cur_mv = &predictor->point[*prednum].motion;
    cur_mv->mv_x = rshift_rnd_sf((mv_scale[list][ref][0] * all_mv[0][blocktype][block_y][block_x][0]), block_shift_mv );
    cur_mv->mv_y = rshift_rnd_sf((mv_scale[list][ref][0] * all_mv[0][blocktype][block_y][block_x][1]), block_shift_mv );
    //*prednum += ((cur_mv->mv_x | cur_mv->mv_y) != 0);
    *prednum += (*((int*) cur_mv) != 0);
  }

  if (blocktype != 1)
  {
    cur_mv = &predictor->point[*prednum].motion;
    cur_mv->mv_x = rshift_rnd((all_mv[ref][1][block_y][block_x][0]), mv_rescale);
    cur_mv->mv_y = rshift_rnd((all_mv[ref][1][block_y][block_x][1]), mv_rescale);
    //*prednum += ((cur_mv->mv_x | cur_mv->mv_y) != 0);
    *prednum += (*((int*) cur_mv) != 0);
  }

  if (blocktype != 4)
  {
    cur_mv = &predictor->point[*prednum].motion;
    cur_mv->mv_x = rshift_rnd((all_mv[ref][4][block_y][block_x][0]), mv_rescale);
    cur_mv->mv_y = rshift_rnd((all_mv[ref][4][block_y][block_x][1]), mv_rescale);
    //*prednum += ((cur_mv->mv_x | cur_mv->mv_y) != 0);
    *prednum += (*((int*) cur_mv) != 0);
  }
}

/*!
************************************************************************
* \brief
*    EPZS Window Based Predictors
************************************************************************
*/
static void EPZSWindowPredictors (short mv[2], EPZSStructure *predictor, int *prednum, int extended)
{
  int pos;
  EPZSStructure *windowPred = (extended) ? window_predictor_extended : window_predictor;
  SPoint *wPoint = &windowPred->point[0];
  SPoint *pPoint = &predictor->point[(*prednum)];

  for (pos = 0; pos < windowPred->searchPoints; pos++)
  {
    (pPoint  )->motion.mv_x = mv[0] + (wPoint  )->motion.mv_x;
    (pPoint++)->motion.mv_y = mv[1] + (wPoint++)->motion.mv_y;    
  }
  *prednum += windowPred->searchPoints;
}

/*!
***********************************************************************
* \brief
*    FAST Motion Estimation using EPZS
*    AMT/HYC
***********************************************************************
*/
int                                           //  ==> minimum motion cost after search
EPZSPelBlockMotionSearch (Macroblock *currMB, // <--  current Macroblock
                          imgpel * cur_pic,   // <--  original pixel values for the AxB block
                          short ref,          // <--  reference picture
                          int list,           // <--  reference list
                          int list_offset,    // <--  offset for Mbaff
                          char ***refPic,     // <--  reference array
                          short ****tmp_mv,   // <--  mv array
                          int pic_pix_x,      // <--  absolute x-coordinate of regarded AxB block
                          int pic_pix_y,      // <--  absolute y-coordinate of regarded AxB block
                          int blocktype,      // <--  block type (1-16x16 ... 7-4x4)
                          short pred_mv[2],   // <--  motion vector predictor in sub-pel units
                          short mv[2],        // <--> in: search center (x|y) / out: motion vector (x|y) - in pel units
                          int search_range,   // <--  1-d search range in pel units
                          int min_mcost,      // <--  minimum motion cost (cost for center or huge value)
                          int lambda_factor,  // <--  lagrangian parameter for determining motion cost
                          int apply_weights)  // <--  perform weight based ME
{
  StorablePicture *ref_picture = listX[list+list_offset][ref];
  short blocksize_y = params->blc_size[blocktype][1];  // vertical block size
  short blocksize_x = params->blc_size[blocktype][0];  // horizontal block size
  short blockshape_x = (blocksize_x >> 2);  // horizontal block size in 4-pel units
  short blockshape_y = (blocksize_y >> 2);  // vertical block size in 4-pel units

  short mb_x = pic_pix_x - img->opix_x;
  short mb_y = pic_pix_y - img->opix_y;
  short pic_pix_x2 = pic_pix_x >> 2;
  short pic_pix_y2 = pic_pix_y >> 2;
  short block_x = (mb_x >> 2);
  short block_y = (mb_y >> 2);

  int   stopCriterion = medthres[blocktype];
  int   mapCenter_x = search_range - mv[0];
  int   mapCenter_y = search_range - mv[1];
  int   second_mcost = INT_MAX;  
  int   *prevSad = EPZSDistortion[list + list_offset][blocktype - 1];
  short *motion=NULL;  
  MotionVector *p_motion = NULL;

  short invalid_refs = 0;
  byte  checkMedian = FALSE;
  EPZSStructure *searchPatternF = searchPattern;
  MotionVector center, tmp, tmp2, cand, pred;
  pred.mv_x = (pic_pix_x << 2) + pred_mv[0];  // predicted position x (in sub-pel units)
  pred.mv_y = (pic_pix_y << 2) + pred_mv[1];  // predicted position x (in sub-pel units)
  center.mv_x = (pic_pix_x << (params->EPZSGrid))+ mv[0];
  center.mv_y = (pic_pix_y << (params->EPZSGrid))+ mv[1];
  cand.mv_x = center.mv_x << mv_rescale;
  cand.mv_y = center.mv_y << mv_rescale;
  tmp.mv_x = mv[0];
  tmp.mv_y = mv[1];
  tmp2.mv_x = 0;
  tmp2.mv_y = 0;

  computePred = computeUniPred[F_PEL + 3 * apply_weights];

  EPZSBlkCount ++;
  if (EPZSBlkCount == 0)
    EPZSBlkCount ++;

  ref_pic_sub.luma = ref_picture->p_curr_img_sub;

  img_width  = ref_picture->size_x;
  img_height = ref_picture->size_y;
  width_pad  = ref_picture->size_x_pad;
  height_pad = ref_picture->size_y_pad;

  if (ChromaMEEnable)
  {
    ref_pic_sub.crcb[0] = ref_picture->imgUV_sub[0];
    ref_pic_sub.crcb[1] = ref_picture->imgUV_sub[1];
    width_pad_cr  = ref_picture->size_x_cr_pad;
    height_pad_cr = ref_picture->size_y_cr_pad;
  }

  pic_pix_x = (pic_pix_x << (params->EPZSGrid));
  pic_pix_y = (pic_pix_y << (params->EPZSGrid));

  if (params->EPZSSpatialMem)
  {
#if EPZSREF
    motion   =  EPZSMotion [list + list_offset][ref][blocktype - 1][block_y][pic_pix_x2];
    p_motion = &EPZS_Motion[list + list_offset][ref][blocktype - 1][block_y][pic_pix_x2];
#else
    motion   =  EPZSMotion [list + list_offset][blocktype - 1][block_y][pic_pix_x2];
    p_motion = &EPZS_Motion[list + list_offset][blocktype - 1][block_y][pic_pix_x2];
#endif
  }

  //===== set function for getting reference picture lines =====
  ref_access_method = CHECK_RANGE ? FAST_ACCESS : UMV_ACCESS;

  // Clear EPZSMap
  // memset(EPZSMap[0],FALSE,searcharray*searcharray);
  // Check median candidate;
  //EPZSMap[0][0] = EPZSBlkCount;
  EPZSMap[search_range][search_range] = EPZSBlkCount;

  //--- initialize motion cost (cost for motion vector) and check ---
  min_mcost = MV_COST_SMP (lambda_factor, cand.mv_x, cand.mv_y, pred.mv_x, pred.mv_y);

  //--- add residual cost to motion cost ---
  min_mcost += computePred(cur_pic, blocksize_y, blocksize_x,
    INT_MAX, cand.mv_x + IMG_PAD_SIZE_TIMES4, cand.mv_y + IMG_PAD_SIZE_TIMES4);

  // Additional threshold for ref>0
  if ((ref>0 && img->structure == FRAME)
    && (prevSad[pic_pix_x2] < imin(medthres[blocktype],min_mcost)))
  {
#if EPZSREF
    if (params->EPZSSpatialMem)
#else
    if (params->EPZSSpatialMem && ref == 0)
#endif
    {
      motion[0]  = tmp.mv_x;
      motion[1]  = tmp.mv_y;
      *p_motion = tmp;
    }
    return min_mcost;
  }

  //  if ((center.mv_x > search_range) && (center.mv_x < img_width  - search_range - blocksize_x) &&
  //(center.mv_y > search_range) && (center.mv_y < img_height - search_range - blocksize_y)   )
  if ( (center.mv_x > search_range) && (center.mv_x < ((img_width  - blocksize_x) << (params->EPZSGrid)) - search_range)
    && (center.mv_y > search_range) && (center.mv_y < ((img_height - blocksize_y) << (params->EPZSGrid)) - search_range))
  {
    ref_access_method = FAST_ACCESS;
  }
  else
  {
    ref_access_method = UMV_ACCESS;
  }

  //! If medthres satisfied, then terminate, otherwise generate Predictors
  //! Condition could be strengthened by consideration distortion of adjacent partitions.
  if (min_mcost > stopCriterion)
  {
    int mb_available_right   = (img->mb_x < (img_width  >> 4) - 1);
    int mb_available_below   = (img->mb_y < (img_height >> 4) - 1);

    int block_available_right;
    int block_available_below;
    int prednum = 5;
    int patternStop = 0, pointNumber = 0, checkPts, nextLast = 0;
    int totalCheckPts = 0, motionDirection = 0;
    int conditionEPZS;
    MotionVector tmv;
    int pos, mcost;
    PixelPos block_a, block_b, block_c, block_d;
    int *mb_size = img->mb_size[IS_LUMA];

    get4x4Neighbour (currMB, mb_x - 1          , mb_y    , mb_size, &block_a);
    get4x4Neighbour (currMB, mb_x              , mb_y - 1, mb_size, &block_b);
    get4x4Neighbour (currMB, mb_x + blocksize_x, mb_y - 1, mb_size, &block_c);
    get4x4Neighbour (currMB, mb_x - 1          , mb_y - 1, mb_size, &block_d);

    if (mb_y > 0)
    {
      if (mb_x < 8)   // first column of 8x8 blocks
      {
        if (mb_y == 8)
        {
          block_available_right = (blocksize_x != MB_BLOCK_SIZE) || mb_available_right;
          if (blocksize_x == MB_BLOCK_SIZE)
            block_c.available = 0;
        }
        else
        {
          block_available_right = (mb_x + blocksize_x != 8) || mb_available_right;
          if (mb_x + blocksize_x == 8)
            block_c.available = 0;
        }
      }
      else
      {
        block_available_right = (mb_x + blocksize_x != MB_BLOCK_SIZE) || mb_available_right;
        if (mb_x + blocksize_x == MB_BLOCK_SIZE)
          block_c.available = 0;
      }
    }
    else
    {
      block_available_right = (mb_x + blocksize_x != MB_BLOCK_SIZE) || mb_available_right;
    }
    block_available_below = (mb_y + blocksize_y != MB_BLOCK_SIZE) || (mb_available_below);

    stopCriterion = EPZSDetermineStopCriterion(prevSad, &block_a, &block_b, &block_c, pic_pix_x2, blocktype, blockshape_x);

    //! Add Spatial Predictors in predictor list.
    //! Scheme adds zero, left, top-left, top, top-right. Note that top-left adds very little
    //! in terms of performance and could be removed with little penalty if any.
    invalid_refs = EPZSSpatialPredictors (block_a, block_b, block_c, block_d,
      list, list_offset, ref, refPic[list], tmp_mv[list], predictor);
    if (params->EPZSSpatialMem)
      EPZSSpatialMemPredictors (list + list_offset, ref, blocktype - 1, pic_pix_x2,
      blockshape_x, blockshape_y, block_y, &prednum, img_width>>2, predictor);

    // Temporal predictors
    if (params->EPZSTemporal)
      EPZSTemporalPredictors (list, list_offset, ref, pic_pix_x2, pic_pix_y2, predictor, &prednum,
      block_a.available, block_b.available, block_available_right,
      block_available_below, blockshape_x, blockshape_y, stopCriterion, min_mcost);

    //! Window Size Based Predictors
    //! Basically replaces a Hierarchical ME concept and helps escaping local minima, or
    //! determining large motion variations.
    //! Following predictors can be adjusted further (i.e. removed, conditioned etc)
    //! based on distortion, correlation of adjacent MVs, complexity etc. These predictors
    //! and their conditioning could also be moved after all other predictors have been
    //! tested. Adaptation could also be based on type of material and coding mode (i.e.
    //! field/frame coding,MBAFF etc considering the higher dependency with opposite parity field
    //conditionEPZS = ((min_mcost > stopCriterion)
    // && (params->EPZSFixed > 1 || (params->EPZSFixed && img->type == P_SLICE)));
    //conditionEPZS = ((ref == 0) && (blocktype < 5) && (min_mcost > stopCriterion)
    //&& (params->EPZSFixed > 1 || (params->EPZSFixed && img->type == P_SLICE)));
    conditionEPZS = ((min_mcost > stopCriterion) && ((ref < 2 && blocktype < 5)
      || ((img->structure!=FRAME || list_offset) && ref < 3))
      && (params->EPZSFixed > 1 || (params->EPZSFixed && img->type == P_SLICE)));

    if (conditionEPZS)
      EPZSWindowPredictors (mv, predictor, &prednum,
      (blocktype < 5) && (invalid_refs > 2) && (ref < 1 + (img->structure!=FRAME || list_offset)));

    //! Blocktype/Reference dependent predictors.
    //! Since already mvs for other blocktypes/references have been computed, we can reuse
    //! them in order to easier determine the optimal point. Use of predictors could depend
    //! on cost,
    //conditionEPZS = (ref == 0 || (ref > 0 && min_mcost > stopCriterion) || img->structure != FRAME || list_offset);
    conditionEPZS = (ref == 0 || (ref > 0 && min_mcost > stopCriterion));
    // above seems to result in memory leak issues which need to be resolved

    if (conditionEPZS && img->current_mb_nr != 0 && params->EPZSBlockType)
      EPZSBlockTypePredictors (block_x, block_y, blocktype, ref, list, predictor, &prednum);

    //! Check all predictors
    for (pos = 0; pos < prednum; pos++)
    {
      tmv = predictor->point[pos].motion;
      //if (((iabs (tmv.mv_x - mv[0]) > search_range || iabs (tmv.mv_y - mv[1]) > search_range)) && (tmv.mv_x || tmv.mv_y))
      if (iabs (tmv.mv_x - mv[0]) > search_range || iabs (tmv.mv_y - mv[1]) > search_range)
        continue;

      if ((iabs (tmv.mv_x - mv[0]) <= search_range) && (iabs (tmv.mv_y - mv[1]) <= search_range))
      {
        if (EPZSMap[mapCenter_y + tmv.mv_y][mapCenter_x + tmv.mv_x] == EPZSBlkCount)
          continue;
        else
          EPZSMap[mapCenter_y + tmv.mv_y][mapCenter_x + tmv.mv_x] = EPZSBlkCount;
      }

      cand.mv_x = (pic_pix_x + tmv.mv_x)<<mv_rescale;
      cand.mv_y = (pic_pix_y + tmv.mv_y)<<mv_rescale;

      //--- set motion cost (cost for motion vector) and check ---
      mcost = MV_COST_SMP (lambda_factor, cand.mv_x, cand.mv_y, pred.mv_x, pred.mv_y);

      if (mcost >= second_mcost) continue;

      //ref_access_method = CHECK_RANGE ? FAST_ACCESS : UMV_ACCESS;

      mcost += computePred(cur_pic, blocksize_y,blocksize_x,
        second_mcost - mcost, cand.mv_x + IMG_PAD_SIZE_TIMES4,cand.mv_y + IMG_PAD_SIZE_TIMES4);

      //--- check if motion cost is less than minimum cost ---
      if (mcost < min_mcost)
      {
        tmp2 = tmp;
        tmp  = tmv;
        second_mcost = min_mcost;
        min_mcost = mcost;
        checkMedian = TRUE;
      }
      //else if (mcost < second_mcost && (tmp.mv_x != tmv.mv_x || tmp.mv_y != tmv.mv_y))
      else if (mcost < second_mcost)
      {
        tmp2 = tmv;
        second_mcost = mcost;
        checkMedian = TRUE;
      }
    }

    //! Refine using EPZS pattern if needed
    //! Note that we are using a conservative threshold method. Threshold
    //! could be tested after checking only a certain number of predictors
    //! instead of the full set. Code could be easily modified for this task.
    if (min_mcost > stopCriterion)
    {
      //! Adapt pattern based on different conditions.
      if (params->EPZSPattern != 0)
      {
        if ((min_mcost < stopCriterion + ((3 * medthres[blocktype]) >> 1)))
        {
          if ((tmp.mv_x == 0 && tmp.mv_y == 0)
            || (iabs (tmp.mv_x - mv[0]) < (2<<(2-mv_rescale)) && iabs (tmp.mv_y - mv[1]) < (2<<(2-mv_rescale))))
            searchPatternF = sdiamond;
          else
            searchPatternF = square;
        }
        else if (blocktype > 5 || (ref > 0 && blocktype != 1))
          searchPatternF = square;
        else
          searchPatternF = searchPattern;
      }

      totalCheckPts = searchPatternF->searchPoints;

      //! center on best predictor
      center = tmp;
      while(1)
      {
        do
        {
          checkPts = totalCheckPts;
          do
          {
            tmv.mv_x = center.mv_x + searchPatternF->point[pointNumber].motion.mv_x;
            tmv.mv_y = center.mv_y + searchPatternF->point[pointNumber].motion.mv_y;
            cand.mv_x = (pic_pix_x + tmv.mv_x)<<mv_rescale;
            cand.mv_y = (pic_pix_y + tmv.mv_y)<<mv_rescale;

            if ((iabs (tmv.mv_x - mv[0]) <= search_range) && (iabs (tmv.mv_y - mv[1]) <= search_range))
            {
              if (EPZSMap[mapCenter_y + tmv.mv_y][mapCenter_x + tmv.mv_x] != EPZSBlkCount)
                EPZSMap[mapCenter_y + tmv.mv_y][mapCenter_x + tmv.mv_x] = EPZSBlkCount;
              else
              {
                pointNumber += 1;
                if (pointNumber >= searchPatternF->searchPoints)
                  pointNumber -= searchPatternF->searchPoints;
                checkPts -= 1;
                continue;
              }
              mcost = MV_COST_SMP (lambda_factor, cand.mv_x, cand.mv_y, pred.mv_x, pred.mv_y);

              if (mcost < min_mcost)
              {
                ref_access_method = CHECK_RANGE ? FAST_ACCESS : UMV_ACCESS;

                mcost += computePred(cur_pic, blocksize_y,blocksize_x,
                  min_mcost - mcost, cand.mv_x + IMG_PAD_SIZE_TIMES4, cand.mv_y + IMG_PAD_SIZE_TIMES4);

                if (mcost < min_mcost)
                {
                  tmp = tmv;
                  min_mcost = mcost;
                  motionDirection = pointNumber;
                }
              }
            }
            pointNumber += 1;
            if (pointNumber >= searchPatternF->searchPoints)
              pointNumber -= searchPatternF->searchPoints;
            checkPts -= 1;
          }
          while (checkPts > 0);

          if (nextLast || ((tmp.mv_x == center.mv_x) && (tmp.mv_y == center.mv_y)))
          {
            patternStop     = searchPatternF->stopSearch;
            searchPatternF  = searchPatternF->nextpattern;
            totalCheckPts   = searchPatternF->searchPoints;
            nextLast        = searchPatternF->nextLast;
            motionDirection = 0;
            pointNumber = 0;
          }
          else
          {
            totalCheckPts = searchPatternF->point[motionDirection].next_points;
            pointNumber = searchPatternF->point[motionDirection].start_nmbr;
            center = tmp;
          }
        }
        while (patternStop != 1);

        if ((ref>0) && (img->structure == FRAME) && (( 4 * prevSad[pic_pix_x2] < min_mcost) ||
          ((3 * prevSad[pic_pix_x2] < min_mcost) && (prevSad[pic_pix_x2] <= stopCriterion))))
        {
          mv[0] = tmp.mv_x;
          mv[1] = tmp.mv_y;
#if EPZSREF
          if (params->EPZSSpatialMem)
#else
          if (params->EPZSSpatialMem && ref == 0)
#endif
          {
            motion[0] = tmp.mv_x;
            motion[1] = tmp.mv_y;
            *p_motion  = tmp;
          }

          return min_mcost;
        }

        //! Check Second best predictor with EPZS pattern
        conditionEPZS = (checkMedian == TRUE)
          && ((img->type == P_SLICE) || (blocktype < 5))
          && (min_mcost > stopCriterion) && (params->EPZSDual > 0);

        if (!conditionEPZS) break;

        pointNumber = 0;
        patternStop = 0;
        motionDirection = 0;
        nextLast = 0;

        if ((tmp.mv_x == 0 && tmp.mv_y == 0)  || (tmp.mv_x == mv[0] && tmp.mv_y == mv[1]))
        {
          if (iabs (tmp.mv_x - mv[0]) < (2<<(2-mv_rescale)) && iabs (tmp.mv_y - mv[1]) < (2<<(2-mv_rescale)))
            searchPatternF = sdiamond;
          else
            searchPatternF = square;
        }
        else
          searchPatternF = searchPatternD;
        totalCheckPts = searchPatternF->searchPoints;

        //! Second best. Note that following code is identical as for best predictor.
        center = tmp2;
        checkMedian = FALSE;
      }
    }
  }

  if ((ref==0) || (prevSad[pic_pix_x2] > min_mcost))
    prevSad[pic_pix_x2] = min_mcost;
#if EPZSREF
  if (params->EPZSSpatialMem)
#else
  if (params->EPZSSpatialMem && ref == 0)
#endif
  {
    motion[0] = tmp.mv_x;
    motion[1] = tmp.mv_y;
    *p_motion = tmp;
    //printf("value %d %d %d %d\n", p_motion->mv_x, p_motion->mv_y, EPZS_Motion[list + list_offset][ref][0][0][0].mv_x, EPZS_Motion[list + list_offset][ref][0][0][0].mv_y);
    //printf("xxxxx %d %d %d %d\n", p_motion->mv_x, p_motion->mv_y, EPZS_Motion[list + list_offset][ref][blocktype - 1][block_y][pic_pix_x2].mv_x, EPZS_Motion[list + list_offset][ref][blocktype - 1][block_y][pic_pix_x2].mv_y);
  }

  mv[0] = tmp.mv_x;
  mv[1] = tmp.mv_y;
  return min_mcost;
}


/*!
***********************************************************************
* \brief
*    FAST Motion Estimation using EPZS
*    AMT/HYC
***********************************************************************
*/
int                                                //  ==> minimum motion cost after search
EPZSBiPredBlockMotionSearch (Macroblock *currMB,   // <--  Current Macroblock
                             imgpel * cur_pic,     // <--  original pixel values for the AxB block
                             short  ref,           // <--  reference picture
                             int    list,          // <--  reference list
                             int    list_offset,   // <--  offset for Mbaff
                             char  ***refPic,      // <--  reference array
                             short  ****tmp_mv,    // <--  mv array
                             int    pic_pix_x,     // <--  absolute x-coordinate of regarded AxB block
                             int    pic_pix_y,     // <--  absolute y-coordinate of regarded AxB block
                             int    blocktype,     // <--  block type (1-16x16 ... 7-4x4)
                             short  pred_mv1[2],   // <--  motion vector predictor in sub-pel units
                             short  pred_mv2[2],   // <--  motion vector predictor in sub-pel units
                             short  mv[2],         // <--> in: search center (x|y) / out: motion vector (x|y) - in pel units
                             short  static_mv[2],       // <--> in: search center (x|y) 
                             int    search_range,  // <--  1-d search range in pel units
                             int    min_mcost,     // <--  minimum motion cost (cost for center or huge value)
                             int    iteration_no,  // <--  bi pred iteration number
                             int    lambda_factor, // <--  lagrangian parameter for determining motion cost
                             int    apply_weights  // <--  perform weight based ME
                             )
{
  StorablePicture *ref_picture1 = listX[list       + list_offset][ref];
  StorablePicture *ref_picture2 = listX[(list ^ 1) + list_offset][0];
  short blocksize_y  = params->blc_size[blocktype][1];        // vertical block size
  short blocksize_x  = params->blc_size[blocktype][0];        // horizontal block size
  short blockshape_x = (blocksize_x >> 2);  // horizontal block size in 4-pel units
//  short blockshape_y = (blocksize_y >> 2);  // vertical block size in 4-pel units

  short mb_x = pic_pix_x - img->opix_x;
  short mb_y = pic_pix_y - img->opix_y;
  short pic_pix_x2 = pic_pix_x >> 2;
  //short pic_pix_y2 = pic_pix_y >> 2;

  int   stopCriterion = medthres[blocktype];
  int   mapCenter_x = search_range - mv[0];
  int   mapCenter_y = search_range - mv[1];
  int   second_mcost = INT_MAX;
  int   *prevSad = EPZSBiDistortion[list + list_offset][blocktype - 1];
  short invalid_refs = 0;
  byte  checkMedian = FALSE;
  EPZSStructure *searchPatternF = searchPattern;

  static MotionVector center1, center2, tmp, tmp2, cand, cand1, cand2, pred1, pred2;
  pred1.mv_x   = (pic_pix_x << 2) + pred_mv1[0]; // predicted position x (in sub-pel units)
  pred1.mv_y   = (pic_pix_y << 2) + pred_mv1[1]; // predicted position y (in sub-pel units)
  pred2.mv_x   = (pic_pix_x << 2) + pred_mv2[0]; // predicted position x (in sub-pel units)
  pred2.mv_y   = (pic_pix_y << 2) + pred_mv2[1]; // predicted position y (in sub-pel units)
  center1.mv_x = (pic_pix_x << (params->EPZSGrid))+ mv[0];
  center1.mv_y = (pic_pix_y << (params->EPZSGrid))+ mv[1];
  center2.mv_x = (pic_pix_x << (params->EPZSGrid))+ static_mv[0];
  center2.mv_y = (pic_pix_y << (params->EPZSGrid))+ static_mv[1];
  cand1.mv_x   = center1.mv_x << mv_rescale;
  cand1.mv_y   = center1.mv_y << mv_rescale;
  cand2.mv_x   = center2.mv_x << mv_rescale;
  cand2.mv_y   = center2.mv_y << mv_rescale;


  tmp.mv_x = mv[0];
  tmp.mv_y = mv[1];
  tmp2.mv_x = 0;
  tmp2.mv_y = 0;
  EPZSBlkCount ++;
  if (EPZSBlkCount == 0)
    EPZSBlkCount ++;

  ref_pic1_sub.luma = ref_picture1->p_curr_img_sub;
  ref_pic2_sub.luma = ref_picture2->p_curr_img_sub;

  img_width  = ref_picture1->size_x;
  img_height = ref_picture1->size_y;
  width_pad  = ref_picture1->size_x_pad;
  height_pad = ref_picture1->size_y_pad;

  if (apply_weights)
  {
    computeBiPred = computeBiPred2[F_PEL];
  }
  else
  {
    computeBiPred = computeBiPred1[F_PEL];
  }

  if ( ChromaMEEnable )
  {
    ref_pic1_sub.crcb[0] = ref_picture1->imgUV_sub[0];
    ref_pic1_sub.crcb[1] = ref_picture1->imgUV_sub[1];
    ref_pic2_sub.crcb[0] = ref_picture2->imgUV_sub[0];
    ref_pic2_sub.crcb[1] = ref_picture2->imgUV_sub[1];
    width_pad_cr  = ref_picture1->size_x_cr_pad;
    height_pad_cr = ref_picture1->size_y_cr_pad;
  }

  pic_pix_x = (pic_pix_x << (params->EPZSGrid));
  pic_pix_y = (pic_pix_y << (params->EPZSGrid));

  //===== set function for getting reference picture lines from reference 1 =====
  if ((cand1.mv_x >= 0) && (cand1.mv_x < img_width  - blocksize_x) &&(cand.mv_y >= 0) && (cand1.mv_y < img_height - blocksize_y))
  {
    bipred1_access_method = FAST_ACCESS;
  }
  else
  {
    bipred1_access_method = UMV_ACCESS;
  }

  //===== set function for getting reference picture lines from reference 2 =====
  if ((cand2.mv_x >= 0) && (cand2.mv_x < img_width  - blocksize_x) &&(cand.mv_y >= 0) && (cand2.mv_y < img_height - blocksize_y))
  {
    bipred2_access_method = FAST_ACCESS;
  }
  else
  {
    bipred2_access_method = UMV_ACCESS;
  }

  // Clear EPZSMap
  //memset(EPZSMap[0],FALSE,searcharray*searcharray);
  // Check median candidate;
  //EPZSMap[0][0] = EPZSBlkCount;
  EPZSMap[search_range][search_range] = EPZSBlkCount;

  //--- initialize motion cost (cost for motion vector) and check ---
  min_mcost  = MV_COST_SMP (lambda_factor, cand1.mv_x, cand1.mv_y, pred1.mv_x, pred1.mv_y);
  min_mcost += MV_COST_SMP (lambda_factor, cand2.mv_x, cand2.mv_y, pred2.mv_x, pred2.mv_y);

  //--- add residual cost to motion cost ---
  min_mcost += computeBiPred(cur_pic, blocksize_y, blocksize_x, INT_MAX,
    cand1.mv_x + IMG_PAD_SIZE_TIMES4,
    cand1.mv_y + IMG_PAD_SIZE_TIMES4,
    cand2.mv_x + IMG_PAD_SIZE_TIMES4,
    cand2.mv_y + IMG_PAD_SIZE_TIMES4);

  //! If medthres satisfied, then terminate, otherwise generate Predictors
  if (min_mcost > stopCriterion)
  {
    int mb_available_right   = (img->mb_x < (img_width  >> 4) - 1);
    int mb_available_below   = (img->mb_y < (img_height >> 4) - 1);

    int block_available_right;
    int block_available_below;
    int prednum = 5;
    int patternStop = 0, pointNumber = 0, checkPts, nextLast = 0;
    int totalCheckPts = 0, motionDirection = 0;
    int conditionEPZS;
    MotionVector tmv;
    int pos, mcost;
    PixelPos block_a, block_b, block_c, block_d;
    int *mb_size = img->mb_size[IS_LUMA];

    get4x4Neighbour (currMB, mb_x - 1          , mb_y    , mb_size, &block_a);
    get4x4Neighbour (currMB, mb_x              , mb_y - 1, mb_size, &block_b);
    get4x4Neighbour (currMB, mb_x + blocksize_x, mb_y - 1, mb_size, &block_c);
    get4x4Neighbour (currMB, mb_x - 1          , mb_y - 1, mb_size, &block_d);

    if (mb_y > 0)
    {
      if (mb_x < 8)   // first column of 8x8 blocks
      {
        if (mb_y == 8)
        {
          block_available_right = (blocksize_x != MB_BLOCK_SIZE) || mb_available_right;
          if (blocksize_x == MB_BLOCK_SIZE)
            block_c.available = 0;
        }
        else
        {
          block_available_right = (mb_x + blocksize_x != 8) || mb_available_right;
          if (mb_x + blocksize_x == 8)
            block_c.available = 0;
        }
      }
      else
      {
        block_available_right = (mb_x + blocksize_x != MB_BLOCK_SIZE) || mb_available_right;
        if (mb_x + blocksize_x == MB_BLOCK_SIZE)
          block_c.available = 0;
      }
    }
    else
    {
      block_available_right = (mb_x + blocksize_x != MB_BLOCK_SIZE) || mb_available_right;
    }
    block_available_below = (mb_y + blocksize_y != MB_BLOCK_SIZE) || (mb_available_below);

    stopCriterion = EPZSDetermineStopCriterion(prevSad, &block_a, &block_b, &block_c, pic_pix_x2, blocktype, blockshape_x);
    // stopCriterion = (11 * medthres[blocktype]) >> 3;


    //! Add Spatial Predictors in predictor list.
    //! Scheme adds zero, left, top-left, top, top-right. Note that top-left adds very little
    //! in terms of performance and could be removed with little penalty if any.
    invalid_refs = EPZSSpatialPredictors (block_a, block_b, block_c, block_d,
      list, list_offset, ref, refPic[list], tmp_mv[list], predictor);

    //! Check all predictors
    for (pos = 0; pos < prednum; pos++)
    {
      tmv = predictor->point[pos].motion;
      //if ((iabs (tmv.mv_x - mv[0]) > search_range || iabs (tmv.mv_y - mv[1]) > search_range) && (tmv.mv_x || tmv.mv_y))
      if (iabs (tmv.mv_x - mv[0]) > search_range || iabs (tmv.mv_y - mv[1]) > search_range)
        continue;

      if ((iabs (tmv.mv_x - mv[0]) <= search_range) && (iabs (tmv.mv_y - mv[1]) <= search_range))
      {
        if (EPZSMap[mapCenter_y + tmv.mv_y][mapCenter_x + tmv.mv_x] == EPZSBlkCount)
          continue;
        else
          EPZSMap[mapCenter_y + tmv.mv_y][mapCenter_x + tmv.mv_x] = EPZSBlkCount;
      }

      cand.mv_x = (pic_pix_x + tmv.mv_x)<<mv_rescale;
      cand.mv_y = (pic_pix_y + tmv.mv_y)<<mv_rescale;

      //--- set motion cost (cost for motion vector) and check ---
      mcost  = MV_COST_SMP (lambda_factor, cand.mv_x, cand.mv_y, pred1.mv_x, pred1.mv_y);
      mcost += MV_COST_SMP (lambda_factor, cand2.mv_x, cand2.mv_y, pred2.mv_x, pred2.mv_y);

      if (mcost >= second_mcost) continue;

      mcost += computeBiPred(cur_pic, blocksize_y, blocksize_x, 
        second_mcost - mcost,
        cand.mv_x + IMG_PAD_SIZE_TIMES4 , cand.mv_y + IMG_PAD_SIZE_TIMES4,
        cand2.mv_x + IMG_PAD_SIZE_TIMES4, cand2.mv_y + IMG_PAD_SIZE_TIMES4);

      //--- check if motion cost is less than minimum cost ---
      if (mcost < min_mcost)
      {
        tmp2 = tmp;
        tmp  = tmv;
        second_mcost = min_mcost;
        min_mcost = mcost;
        checkMedian = TRUE;
      }
      //else if (mcost < second_mcost && (tmp.mv_x != tmv.mv_x || tmp.mv_y != tmv.mv_y))
      else if (mcost < second_mcost)
      {
        tmp2 = tmv;
        second_mcost = mcost;
        checkMedian = TRUE;
      }
    }

    //! Refine using EPZS pattern if needed
    //! Note that we are using a conservative threshold method. Threshold
    //! could be tested after checking only a certain number of predictors
    //! instead of the full set. Code could be easily modified for this task.
    if (min_mcost > stopCriterion)
    {
      //! Adapt pattern based on different conditions.
      if (params->EPZSPattern != 0)
      {
        if ((min_mcost < stopCriterion + ((3 * medthres[blocktype]) >> 1)))
        {
          if ((tmp.mv_x == 0 && tmp.mv_y == 0)
            || (iabs (tmp.mv_x - mv[0]) < (2<<(2-mv_rescale)) && iabs (tmp.mv_y - mv[1]) < (2<<(2-mv_rescale))))
            searchPatternF = sdiamond;
          else
            searchPatternF = square;
        }
        else if (blocktype > 5 || (ref > 0 && blocktype != 1))
          searchPatternF = square;
        else
          searchPatternF = searchPattern;
      }

      totalCheckPts = searchPatternF->searchPoints;

      //! center on best predictor
      center1 = tmp;
      while (1)
      {
        do
        {
          checkPts = totalCheckPts;
          do
          {
            tmv.mv_x = center1.mv_x + searchPatternF->point[pointNumber].motion.mv_x;
            tmv.mv_y = center1.mv_y + searchPatternF->point[pointNumber].motion.mv_y;
            cand.mv_x = (pic_pix_x + tmv.mv_x)<<mv_rescale;
            cand.mv_y = (pic_pix_y + tmv.mv_y)<<mv_rescale;

            if ((iabs (tmv.mv_x - mv[0]) <= search_range) && (iabs (tmv.mv_y - mv[1]) <= search_range))
            {
              if (EPZSMap[mapCenter_y + tmv.mv_y][mapCenter_x + tmv.mv_x] != EPZSBlkCount)
                EPZSMap[mapCenter_y + tmv.mv_y][mapCenter_x + tmv.mv_x] = EPZSBlkCount;
              else
              {
                pointNumber += 1;
                if (pointNumber >= searchPatternF->searchPoints)
                  pointNumber -= searchPatternF->searchPoints;
                checkPts -= 1;
                continue;
              }

              mcost  = MV_COST_SMP (lambda_factor, cand.mv_x, cand.mv_y, pred2.mv_x, pred2.mv_y);
              mcost += MV_COST_SMP (lambda_factor, cand2.mv_x, cand2.mv_y, pred2.mv_x, pred2.mv_y);              

              if (mcost < min_mcost)
              {
                mcost += computeBiPred(cur_pic,
                  blocksize_y, blocksize_x, min_mcost - mcost,
                  cand.mv_x + IMG_PAD_SIZE_TIMES4, cand.mv_y + IMG_PAD_SIZE_TIMES4,
                  cand2.mv_x + IMG_PAD_SIZE_TIMES4,
                  cand2.mv_y + IMG_PAD_SIZE_TIMES4);

                if (mcost < min_mcost)
                {
                  tmp = tmv;
                  min_mcost = mcost;
                  motionDirection = pointNumber;
                }
              }
            }
            pointNumber += 1;
            if (pointNumber >= searchPatternF->searchPoints)
              pointNumber -= searchPatternF->searchPoints;
            checkPts -= 1;
          }
          while (checkPts > 0);

          if (nextLast || ((tmp.mv_x == center1.mv_x) && (tmp.mv_y == center1.mv_y)))
          {
            patternStop     = searchPatternF->stopSearch;
            searchPatternF  = searchPatternF->nextpattern;
            totalCheckPts   = searchPatternF->searchPoints;
            nextLast        = searchPatternF->nextLast;
            motionDirection = 0;
            pointNumber = 0;
          }
          else
          {
            totalCheckPts = searchPatternF->point[motionDirection].next_points;
            pointNumber = searchPatternF->point[motionDirection].start_nmbr;
            center1 = tmp;
          }
        }
        while (patternStop != 1);

        //! Check Second best predictor with EPZS pattern
        conditionEPZS = (checkMedian == TRUE)
          && (blocktype < 5) 
          && (min_mcost > stopCriterion) && (params->EPZSDual > 0);

        if (!conditionEPZS) break;

        pointNumber = 0;
        patternStop = 0;
        motionDirection = 0;
        nextLast = 0;

        if ((tmp.mv_x == 0 && tmp.mv_y == 0) || (tmp.mv_x == mv[0] && tmp.mv_y == mv[1]))
        {
          if (iabs (tmp.mv_x - mv[0]) < (2<<(2-mv_rescale)) && iabs (tmp.mv_y - mv[1]) < (2<<(2-mv_rescale)))
            searchPatternF = sdiamond;
          else
            searchPatternF = square;
        }
        else
          searchPatternF = searchPatternD;
        totalCheckPts = searchPatternF->searchPoints;

        //! Second best. Note that following code is identical as for best predictor.
        center1 = tmp2;
        checkMedian = FALSE;
      }
    }
  }
  if (iteration_no == 0)
  {
    prevSad[pic_pix_x2] = min_mcost;
  }

  mv[0] = tmp.mv_x;
  mv[1] = tmp.mv_y;

  return min_mcost;
}


/*!
***********************************************************************
* \brief
*    Report function for EPZS Fast ME
*    AMT/HYC
***********************************************************************
*/
void EPZSOutputStats (InputParameters *params, FILE * stat, short stats_file)
{
  if (stats_file == 1)
  {
    fprintf (stat, " EPZS Pattern                 : %s\n",c_EPZSPattern[params->EPZSPattern]);
    fprintf (stat, " EPZS Dual Pattern            : %s\n",c_EPZSDualPattern[params->EPZSDual]);
    fprintf (stat, " EPZS Fixed Predictors        : %s\n",c_EPZSFixed[params->EPZSFixed]);
    fprintf (stat, " EPZS Temporal Predictors     : %s\n",c_EPZSOther[params->EPZSTemporal]);
    fprintf (stat, " EPZS Spatial Predictors      : %s\n",c_EPZSOther[params->EPZSSpatialMem]);
    fprintf (stat, " EPZS Thresholds (16x16)      : (%d %d %d)\n",medthres[1], minthres[1], maxthres[1]);
    fprintf (stat, " EPZS Subpel ME               : %s\n",c_EPZSOther[params->EPZSSubPelME]);
    fprintf (stat, " EPZS Subpel ME BiPred        : %s\n",c_EPZSOther[params->EPZSSubPelMEBiPred]);
  }
  else
  {
    fprintf (stat, " EPZS Pattern                      : %s\n",c_EPZSPattern[params->EPZSPattern]);
    fprintf (stat, " EPZS Dual Pattern                 : %s\n",c_EPZSDualPattern[params->EPZSDual]);
    fprintf (stat, " EPZS Fixed Predictors             : %s\n",c_EPZSFixed[params->EPZSFixed]);
    fprintf (stat, " EPZS Temporal Predictors          : %s\n",c_EPZSOther[params->EPZSTemporal]);
    fprintf (stat, " EPZS Spatial Predictors           : %s\n",c_EPZSOther[params->EPZSSpatialMem]);
    fprintf (stat, " EPZS Thresholds (16x16)           : (%d %d %d)\n",medthres[1], minthres[1], maxthres[1]);
    fprintf (stat, " EPZS Subpel ME                    : %s\n",c_EPZSOther[params->EPZSSubPelME]);
    fprintf (stat, " EPZS Subpel ME BiPred             : %s\n",c_EPZSOther[params->EPZSSubPelMEBiPred]);
  }
}


/*!
 ***********************************************************************
 * \brief
 *    Fast sub pixel block motion search to support EPZS
 ***********************************************************************
 */
int                                                   //  ==> minimum motion cost after search
EPZSSubPelBlockMotionSearch (imgpel*   orig_pic,      // <--  original pixel values for the AxB block
                             short     ref,           // <--  reference frame (0... or -1 (backward))
                             int       list,          // <--  reference picture list
                             int       list_offset,   // <--  MBAFF list offset
                             int       pic_pix_x,     // <--  absolute x-coordinate of regarded AxB block
                             int       pic_pix_y,     // <--  absolute y-coordinate of regarded AxB block
                             int       blocktype,     // <--  block type (1-16x16 ... 7-4x4)
                             short     pred_mv[2],    // <--  motion vector predictor in sub-pel units
                             short     mv[2],         // <--> in: search center / out: motion vector - in pel units
                             int       search_pos2,   // <--  search positions for    half-pel search  (default: 9)
                             int       search_pos4,   // <--  search positions for quarter-pel search  (default: 9)
                             int       min_mcost,     // <--  minimum motion cost (cost for center or huge value)
                             int*      lambda,        // <--  lagrangian parameter for determining motion cost
                             int       apply_weights  // <--  use weight based ME
                             )
{

  int   pos, best_pos = 0, second_pos = 0, mcost;
  int   second_mcost = INT_MAX;

  int   cand_mv_x, cand_mv_y;

  int   blocksize_x     = params->blc_size[blocktype][0];
  int   blocksize_y     = params->blc_size[blocktype][1];
  int   pic4_pix_x      = ((pic_pix_x + IMG_PAD_SIZE)<< 2);
  int   pic4_pix_y      = ((pic_pix_y + IMG_PAD_SIZE)<< 2);

  int   max_pos2        = ( (!start_me_refinement_hp || !start_me_refinement_qp) ? imax(1,search_pos2) : search_pos2);

  StorablePicture *ref_picture = listX[list+list_offset][ref];

  int max_pos_x4 = ((ref_picture->size_x - blocksize_x + 2*IMG_PAD_SIZE)<<2);
  int max_pos_y4 = ((ref_picture->size_y - blocksize_y + 2*IMG_PAD_SIZE)<<2);
  int start_pos = 5, end_pos = max_pos2;  
  int lambda_factor = lambda[H_PEL];
  computePred = computeUniPred[H_PEL + 3 * apply_weights];

  ref_pic_sub.luma = ref_picture->p_curr_img_sub;
  width_pad  = ref_picture->size_x_pad;
  height_pad = ref_picture->size_y_pad;

  if ( ChromaMEEnable )
  {
    ref_pic_sub.crcb[0] = ref_picture->imgUV_sub[0];
    ref_pic_sub.crcb[1] = ref_picture->imgUV_sub[1];
    width_pad_cr  = ref_picture->size_x_cr_pad;
    height_pad_cr = ref_picture->size_y_cr_pad;
  }

  /*********************************
   *****                       *****
   *****  HALF-PEL REFINEMENT  *****
   *****                       *****
   *********************************/

  //===== set function for getting pixel values =====
  if ((pic4_pix_x + mv[0] > 1) && (pic4_pix_x + mv[0] < max_pos_x4 - 1) &&
    (pic4_pix_y + mv[1] > 1) && (pic4_pix_y + mv[1] < max_pos_y4 - 1))
  {
    ref_access_method = FAST_ACCESS;
  }
  else
  {
    ref_access_method = UMV_ACCESS;
  }

  //===== loop over search positions =====
  for (best_pos = 0, pos = start_me_refinement_hp; pos < 5; pos++)
  {
    cand_mv_x = mv[0] + search_point_hp[pos][0];    // quarter-pel units
    cand_mv_y = mv[1] + search_point_hp[pos][1];    // quarter-pel units

    //----- set motion vector cost -----
    mcost = MV_COST_SMP (lambda_factor, cand_mv_x, cand_mv_y, pred_mv[0], pred_mv[1]);
    mcost += computePred( orig_pic, blocksize_y, blocksize_x,
      INT_MAX, cand_mv_x + pic4_pix_x, cand_mv_y + pic4_pix_y);

    if (mcost < min_mcost)
    {
      second_mcost = min_mcost;
      second_pos  = best_pos;
      min_mcost = mcost;
      best_pos  = pos;
    }
    else if (mcost < second_mcost)
    {
      second_mcost = mcost;
      second_pos  = pos;
    }
  }

  if (best_pos ==0 && (pred_mv[0] == mv[0]) && (pred_mv[1] - mv[1])== 0 && min_mcost < subthres[blocktype])
      return min_mcost;

  if (best_pos != 0 && second_pos != 0)
  {
    switch (best_pos ^ second_pos)
    {
    case 1:
      start_pos = 6;
      end_pos   = 7;
      break;
    case 3:
      start_pos = 5;
      end_pos   = 6;
      break;
    case 5:
      start_pos = 8;
      end_pos   = 9;
      break;
    case 7:
      start_pos = 7;
      end_pos   = 8;
      break;
    default:
      break;
    }
  }
  else
  {
    switch (best_pos + second_pos)
    {
    case 0:
      start_pos = 5;
      end_pos   = 5;
      break;
    case 1:
      start_pos = 8;
      end_pos   = 10;
      break;
    case 2:
      start_pos = 5;
      end_pos   = 7;
      break;
    case 5:
      start_pos = 6;
      end_pos   = 8;
      break;
    case 7:
      start_pos = 7;
      end_pos   = 9;
      break;
    default:
      break;
    }
  }

  if (best_pos !=0 || (iabs(pred_mv[0] - mv[0]) + iabs(pred_mv[1] - mv[1])))
  {
    for (pos = start_pos; pos < end_pos; pos++)
    {
      cand_mv_x = mv[0] + search_point_hp[pos][0];    // quarter-pel units
      cand_mv_y = mv[1] + search_point_hp[pos][1];    // quarter-pel units

      //----- set motion vector cost -----
      mcost = MV_COST_SMP (lambda_factor, cand_mv_x, cand_mv_y, pred_mv[0], pred_mv[1]);

      if (mcost >= min_mcost) continue;

      mcost += computePred( orig_pic, blocksize_y, blocksize_x,
        min_mcost - mcost, cand_mv_x + pic4_pix_x, cand_mv_y + pic4_pix_y);

      if (mcost < min_mcost)
      {
        min_mcost = mcost;
        best_pos  = pos;
      }
    }
  }

  if (best_pos)
  {
    mv[0] += search_point_hp[best_pos][0];
    mv[1] += search_point_hp[best_pos][1];
  }

  if ( !start_me_refinement_qp )
    min_mcost = INT_MAX;

  /************************************
  *****                          *****
  *****  QUARTER-PEL REFINEMENT  *****
  *****                          *****
  ************************************/

  //===== set function for getting pixel values =====
  if ((pic4_pix_x + mv[0] > 0) && (pic4_pix_x + mv[0] < max_pos_x4) &&
    (pic4_pix_y + mv[1] > 0) && (pic4_pix_y + mv[1] < max_pos_y4)   )
  {
    ref_access_method = FAST_ACCESS;
  }
  else
  {
    ref_access_method = UMV_ACCESS;
  }

  computePred = computeUniPred[Q_PEL + 3 * apply_weights];
  lambda_factor = lambda[Q_PEL];
  second_pos = 0;
  second_mcost = INT_MAX;
  //===== loop over search positions =====
  for (best_pos = 0, pos = start_me_refinement_qp; pos < 5; pos++)
  {
    cand_mv_x = mv[0] + search_point_qp[pos][0];    // quarter-pel units
    cand_mv_y = mv[1] + search_point_qp[pos][1];    // quarter-pel units

    //----- set motion vector cost -----
    mcost = MV_COST_SMP (lambda_factor, cand_mv_x, cand_mv_y, pred_mv[0], pred_mv[1]);
    mcost += computePred( orig_pic, blocksize_y, blocksize_x,
      INT_MAX, cand_mv_x + pic4_pix_x, cand_mv_y + pic4_pix_y);

    if (mcost < min_mcost)
    {
      second_mcost = min_mcost;
      second_pos  = best_pos;
      min_mcost = mcost;
      best_pos  = pos;
    }
    else if (mcost < second_mcost)
    {
      second_mcost = mcost;
      second_pos  = pos;
    }
  }

  if (best_pos ==0 && (pred_mv[0] == mv[0]) && (pred_mv[1] - mv[1])== 0 && min_mcost < subthres[blocktype])
  {
    return min_mcost;
  }

  start_pos = 5;
  end_pos = search_pos4;

  if (best_pos != 0 && second_pos != 0)
  {
    switch (best_pos ^ second_pos)
    {
    case 1:
      start_pos = 6;
      end_pos   = 7;
      break;
    case 3:
      start_pos = 5;
      end_pos   = 6;
      break;
    case 5:
      start_pos = 8;
      end_pos   = 9;
      break;
    case 7:
      start_pos = 7;
      end_pos   = 8;
      break;
    default:
      break;
    }
  }
  else
  {
    switch (best_pos + second_pos)
    {
      //case 0:
      //start_pos = 5;
      //end_pos   = 5;
      //break;
    case 1:
      start_pos = 8;
      end_pos   = 10;
      break;
    case 2:
      start_pos = 5;
      end_pos   = 7;
      break;
    case 5:
      start_pos = 6;
      end_pos   = 8;
      break;
    case 7:
      start_pos = 7;
      end_pos   = 9;
      break;
    default:
      break;
    }
  }

  if (best_pos !=0 || (iabs(pred_mv[0] - mv[0]) + iabs(pred_mv[1] - mv[1])))
  {
    for (pos = start_pos; pos < end_pos; pos++)
    {
      cand_mv_x = mv[0] + search_point_qp[pos][0];    // quarter-pel units
      cand_mv_y = mv[1] + search_point_qp[pos][1];    // quarter-pel units

      //----- set motion vector cost -----
      mcost = MV_COST_SMP (lambda_factor, cand_mv_x, cand_mv_y, pred_mv[0], pred_mv[1]);

      if (mcost >= min_mcost) continue;
      mcost += computePred( orig_pic, blocksize_y, blocksize_x,
        min_mcost - mcost, cand_mv_x + pic4_pix_x, cand_mv_y + pic4_pix_y);

      if (mcost < min_mcost)
      {
        min_mcost = mcost;
        best_pos  = pos;
      }
    }
  }
  if (best_pos)
  {
    mv[0] += search_point_qp [best_pos][0];
    mv[1] += search_point_qp [best_pos][1];
  }

  //===== return minimum motion cost =====
  return min_mcost;
}

/*!
 ***********************************************************************
 * \brief
 *    Fast bipred sub pixel block motion search to support EPZS
 ***********************************************************************
 */
int                                                   //  ==> minimum motion cost after search
EPZSSubPelBlockSearchBiPred (imgpel*   orig_pic,      // <--  original pixel values for the AxB block
                             short     ref,           // <--  reference frame (0... or -1 (backward))
                             int       list,          // <--  reference picture list
                             int       pic_pix_x,     // <--  absolute x-coordinate of regarded AxB block
                             int       pic_pix_y,     // <--  absolute y-coordinate of regarded AxB block
                             int       blocktype,     // <--  block type (1-16x16 ... 7-4x4)
                             short     pred_mv1[2],   // <--  motion vector predictor (x) in sub-pel units
                             short     pred_mv2[2],   // <--  motion vector predictor (x) in sub-pel units
                             short     mv[2],         // <--> in: search center (x) / out: motion vector (x) - in pel units
                             short     static_mv[2],  // <--> in: search center (x) / out: motion vector (x) - in pel units
                             int       search_pos2,   // <--  search positions for    half-pel search  (default: 9)
                             int       search_pos4,   // <--  search positions for quarter-pel search  (default: 9)
                             int       min_mcost,     // <--  minimum motion cost (cost for center or huge value)
                             int       *lambda,       // <--  lagrangian parameter for determining motion cost
                             int       apply_weights  // <--  perform weight based ME
                             )
{
  int   list_offset   = img->mb_data[img->current_mb_nr].list_offset;

  int   pos, best_pos = 0, second_pos = 0, mcost;
  int   second_mcost = INT_MAX;

  int   cand_mv_x, cand_mv_y;

  int   blocksize_x     = params->blc_size[blocktype][0];
  int   blocksize_y     = params->blc_size[blocktype][1];

  int   pic4_pix_x      = ((pic_pix_x + IMG_PAD_SIZE)<< 2);
  int   pic4_pix_y      = ((pic_pix_y + IMG_PAD_SIZE)<< 2);

  int   start_hp        = (min_mcost == INT_MAX) ? 0 : start_me_refinement_hp;
  int   max_pos2        = ( (!start_me_refinement_hp || !start_me_refinement_qp) ? imax(1,search_pos2) : search_pos2);

  int   smv_x = static_mv[0] + pic4_pix_x;
  int   smv_y = static_mv[1] + pic4_pix_y;

  StorablePicture *ref_picture1 = listX[list       + list_offset][ref];
  StorablePicture *ref_picture2 = listX[(list ^ 1) + list_offset][0];

  int max_pos_x4 = ((ref_picture1->size_x - blocksize_x + 2*IMG_PAD_SIZE)<<2);
  int max_pos_y4 = ((ref_picture1->size_y - blocksize_y + 2*IMG_PAD_SIZE)<<2);
  int start_pos = 5, end_pos = max_pos2;
  int lambda_factor = lambda[H_PEL];

  ref_pic1_sub.luma = ref_picture1->p_curr_img_sub;
  ref_pic2_sub.luma = ref_picture2->p_curr_img_sub;
  img_width     = ref_picture1->size_x;
  img_height    = ref_picture1->size_y;
  width_pad    = ref_picture1->size_x_pad;
  height_pad   = ref_picture1->size_y_pad;

  if (apply_weights)
  {    
    computeBiPred = computeBiPred2[H_PEL];
  }
  else
  {
    computeBiPred = computeBiPred1[H_PEL];
  }


  if ( ChromaMEEnable )
  {
    ref_pic1_sub.crcb[0] = ref_picture1->imgUV_sub[0];
    ref_pic1_sub.crcb[1] = ref_picture1->imgUV_sub[1];
    ref_pic2_sub.crcb[0] = ref_picture2->imgUV_sub[0];
    ref_pic2_sub.crcb[1] = ref_picture2->imgUV_sub[1];
    width_pad_cr  = ref_picture1->size_x_cr_pad;
    height_pad_cr = ref_picture1->size_y_cr_pad;
  }

  /*********************************
   *****                       *****
   *****  HALF-PEL REFINEMENT  *****
   *****                       *****
   *********************************/

  //===== set function for getting pixel values =====
  if ((pic4_pix_x + mv[0] > 1) && (pic4_pix_x + mv[0] < max_pos_x4 - 1) &&
    (pic4_pix_y + mv[1] > 1) && (pic4_pix_y + mv[1] < max_pos_y4 - 1))
  {
    bipred1_access_method = FAST_ACCESS;
  }
  else
  {
    bipred1_access_method = UMV_ACCESS;
  }

  if ((pic4_pix_x + static_mv[0] > 1) && (pic4_pix_x + static_mv[0] < max_pos_x4 - 1) &&
    (pic4_pix_y + static_mv[1] > 1) && (pic4_pix_y + static_mv[1] < max_pos_y4 - 1))
  {
    bipred2_access_method = FAST_ACCESS;
  }
  else
  {
    bipred2_access_method = UMV_ACCESS;
  }

  //===== loop over search positions =====
  for (best_pos = 0, pos = start_hp; pos < 5; pos++)
  {
    cand_mv_x = mv[0] + search_point_hp[pos][0];    // quarter-pel units
    cand_mv_y = mv[1] + search_point_hp[pos][1];    // quarter-pel units

    //----- set motion vector cost -----
    mcost  = MV_COST_SMP (lambda_factor, cand_mv_x, cand_mv_y, pred_mv1[0], pred_mv1[1]);
    mcost += MV_COST_SMP (lambda_factor, static_mv[0], static_mv[1], pred_mv2[0], pred_mv2[1]);
    mcost += computeBiPred(orig_pic, blocksize_y, blocksize_x, INT_MAX,
      cand_mv_x + pic4_pix_x, cand_mv_y + pic4_pix_y, smv_x, smv_y);

    if (mcost < min_mcost)
    {
      second_mcost = min_mcost;
      second_pos  = best_pos;
      min_mcost = mcost;
      best_pos  = pos;
    }
    else if (mcost < second_mcost)
    {
      second_mcost = mcost;
      second_pos  = pos;
    }
  }

//  if (best_pos ==0 && (pred_mv1[0] == mv[0]) && (pred_mv1[1] - mv[1])== 0 && min_mcost < subthres[blocktype])
      //return min_mcost;

  if (best_pos != 0 && second_pos != 0)
  {
    switch (best_pos ^ second_pos)
    {
    case 1:
      start_pos = 6;
      end_pos   = 7;
      break;
    case 3:
      start_pos = 5;
      end_pos   = 6;
      break;
    case 5:
      start_pos = 8;
      end_pos   = 9;
      break;
    case 7:
      start_pos = 7;
      end_pos   = 8;
      break;
    default:
      break;
    }
  }
  else
  {
    switch (best_pos + second_pos)
    {
    case 0:
      start_pos = 5;
      end_pos   = 5;
      break;
    case 1:
      start_pos = 8;
      end_pos   = 10;
      break;
    case 2:
      start_pos = 5;
      end_pos   = 7;
      break;
    case 5:
      start_pos = 6;
      end_pos   = 8;
      break;
    case 7:
      start_pos = 7;
      end_pos   = 9;
      break;
    default:
      break;
    }
  }

  if (best_pos !=0 || (iabs(pred_mv1[0] - mv[0]) + iabs(pred_mv1[1] - mv[1])))
  {
    for (pos = start_pos; pos < end_pos; pos++)
    {
      cand_mv_x = mv[0] + search_point_hp[pos][0];    // quarter-pel units
      cand_mv_y = mv[1] + search_point_hp[pos][1];    // quarter-pel units

      //----- set motion vector cost -----
      mcost  = MV_COST_SMP (lambda_factor, cand_mv_x, cand_mv_y, pred_mv1[0], pred_mv1[1]);
      mcost += MV_COST_SMP (lambda_factor, static_mv[0], static_mv[1], pred_mv2[0], pred_mv2[1]);
      if (mcost >= min_mcost) continue;

      mcost += computeBiPred(orig_pic, blocksize_y, blocksize_x, min_mcost - mcost,
        cand_mv_x + pic4_pix_x, cand_mv_y + pic4_pix_y, smv_x, smv_y);

      if (mcost < min_mcost)
      {
        min_mcost = mcost;
        best_pos  = pos;
      }
    }
  }

  if (best_pos)
  {
    mv[0] += search_point_hp [best_pos][0];
    mv[1] += search_point_hp [best_pos][1];
  }

  computeBiPred = apply_weights? computeBiPred2[Q_PEL] : computeBiPred1[Q_PEL];

  /************************************
  *****                          *****
  *****  QUARTER-PEL REFINEMENT  *****
  *****                          *****
  ************************************/
  //===== set function for getting pixel values =====
  if ((pic4_pix_x + mv[0] > 0) && (pic4_pix_x + mv[0] < max_pos_x4) &&
    (pic4_pix_y + mv[1] > 0) && (pic4_pix_y + mv[1] < max_pos_y4))
  {
    bipred1_access_method = FAST_ACCESS;
  }
  else
  {
    bipred1_access_method = UMV_ACCESS;
  }

  if ((pic4_pix_x + static_mv[0] > 0) && (pic4_pix_x + static_mv[0] < max_pos_x4) &&
    (pic4_pix_y + static_mv[1] > 0) && (pic4_pix_y + static_mv[1] < max_pos_y4))
  {
    bipred2_access_method = FAST_ACCESS;
  }
  else
  {
    bipred2_access_method = UMV_ACCESS;
  }

  if ( !start_me_refinement_qp )
    min_mcost = INT_MAX;

  lambda_factor = lambda[Q_PEL];
  second_pos = 0;
  second_mcost = INT_MAX;
  //===== loop over search positions =====
  for (best_pos = 0, pos = start_me_refinement_qp; pos < 5; pos++)
  {
    cand_mv_x = mv[0] + search_point_qp[pos][0];    // quarter-pel units
    cand_mv_y = mv[1] + search_point_qp[pos][1];    // quarter-pel units

    //----- set motion vector cost -----
    mcost  = MV_COST_SMP (lambda_factor, cand_mv_x, cand_mv_y, pred_mv1[0], pred_mv1[1]);
    mcost += MV_COST_SMP (lambda_factor, static_mv[0], static_mv[1], pred_mv2[0], pred_mv2[1]);

    mcost += computeBiPred(orig_pic, blocksize_y, blocksize_x, INT_MAX,
      cand_mv_x + pic4_pix_x, cand_mv_y + pic4_pix_y, smv_x, smv_y);


    if (mcost < min_mcost)
    {
      second_mcost = min_mcost;
      second_pos  = best_pos;
      min_mcost = mcost;
      best_pos  = pos;
    }
    else if (mcost < second_mcost)
    {
      second_mcost = mcost;
      second_pos  = pos;
    }
  }

  start_pos = 5;
  end_pos = search_pos4;

  if (best_pos != 0 && second_pos != 0)
  {
    switch (best_pos ^ second_pos)
    {
    case 1:
      start_pos = 6;
      end_pos   = 7;
      break;
    case 3:
      start_pos = 5;
      end_pos   = 6;
      break;
    case 5:
      start_pos = 8;
      end_pos   = 9;
      break;
    case 7:
      start_pos = 7;
      end_pos   = 8;
      break;
    default:
      break;
    }
  }
  else
  {
    switch (best_pos + second_pos)
    {
      //case 0:
      //start_pos = 5;
      //end_pos   = 5;
      //break;
    case 1:
      start_pos = 8;
      end_pos   = 10;
      break;
    case 2:
      start_pos = 5;
      end_pos   = 7;
      break;
    case 5:
      start_pos = 6;
      end_pos   = 8;
      break;
    case 7:
      start_pos = 7;
      end_pos   = 9;
      break;
    default:
      break;
    }
  }

  if (best_pos !=0 || (iabs(pred_mv1[0] - mv[0]) + iabs(pred_mv1[1] - mv[1])))
  {
    for (pos = start_pos; pos < end_pos; pos++)
    {
      cand_mv_x = mv[0] + search_point_qp[pos][0];    // quarter-pel units
      cand_mv_y = mv[1] + search_point_qp[pos][1];    // quarter-pel units

      //----- set motion vector cost -----
      mcost  = MV_COST_SMP (lambda_factor, cand_mv_x, cand_mv_y, pred_mv1[0], pred_mv1[1]);
      mcost += MV_COST_SMP (lambda_factor, static_mv[0], static_mv[1], pred_mv2[0], pred_mv2[1]);
      if (mcost >= min_mcost) continue;

      mcost += computeBiPred(orig_pic, blocksize_y, blocksize_x, min_mcost - mcost,
        cand_mv_x + pic4_pix_x, cand_mv_y + pic4_pix_y, smv_x, smv_y);


      if (mcost < min_mcost)
      {
        min_mcost = mcost;
        best_pos  = pos;
      }
    }
  }
  if (best_pos)
  {
    mv[0] += search_point_qp[best_pos][0];
    mv[1] += search_point_qp[best_pos][1];
  }

  //===== return minimum motion cost =====
  return min_mcost;
}

