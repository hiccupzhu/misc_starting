
/*!
 ************************************************************************
 * \file
 *     me_epzs.h
 *
 * \author
 *    Alexis Michael Tourapis        <alexis.tourapis@dolby.com>
 *
 * \date
 *    11. August 2006
 *
 * \brief
 *    Headerfile for EPZS motion estimation
 **************************************************************************
 */


#ifndef _ME_EPZS_H_
#define _ME_EPZS_H_


#define CHECK_RANGE  ((cand.mv_x >= 0) && (cand.mv_x < img_width  - blocksize_x) &&(cand.mv_y >= 0) && (cand.mv_y < img_height - blocksize_y))


typedef struct
{
  int         mb_adaptive_frame_field_flag;
  int         size_x, size_y;
  
  // Frame
  MotionVector ***frame;  //!< motion vector       [list][subblock_x][subblock_y]
  // Top field
  MotionVector ***top;    //!< motion vector       [list][subblock_x][subblock_y]
  // Bottom field
  MotionVector ***bot;    //!< motion vector       [list][subblock_x][subblock_y]
} EPZSColocParams;

typedef struct
{
  MotionVector motion;
  int start_nmbr;
  int next_points;
}
SPoint;

typedef struct MEPatternNode
{
  int    searchPoints;
  SPoint *point;
  int    stopSearch;
  int    nextLast;
  struct MEPatternNode *nextpattern;
}
EPZSStructure;

typedef enum
{
  SDIAMOND  = 0,
  SQUARE    = 1,
  EDIAMOND  = 2,
  LDIAMOND  = 3,
  SBDIAMOND = 4
} EPZSPatterns;

extern EPZSColocParams *EPZSCo_located;
extern int ***EPZSDistortion;  //!< Array for storing SAD Values

extern int  EPZSInit        (InputParameters *params, ImageParameters *img);
extern void EPZSDelete      (InputParameters *params);
extern void EPZSOutputStats (InputParameters *params, FILE * stat, short stats_file);
extern void EPZSSliceInit   (InputParameters *params, ImageParameters *img, EPZSColocParams* p, StorablePicture **listX[6]);
extern int  EPZSPelBlockMotionSearch (Macroblock *, imgpel *, short, int, int, char ***, short ****,
                                     int, int, int, short[2], short[2], int, int, int, int);

extern int  EPZSBiPredBlockMotionSearch (Macroblock *, imgpel *, short, int, int, char  ***, short  ****,
                                        int, int, int, short[2], short[2],
                                        short[2], short[2], int, int, int, int, int);

extern int EPZSSubPelBlockMotionSearch (imgpel *, short, int, int, int, int, int, short[2],
                                        short[2], int, int, int, int*, int);

extern int EPZSSubPelBlockSearchBiPred  (imgpel* orig_pic, short ref, int list, int pic_pix_x, int pic_pix_y,
                                         int blocktype, short pred_mv1[2], short pred_mv2[2], short mv1[2], short mv2[2],
                                         int search_pos2, int search_pos4, int min_mcost, int *lambda_factor, int apply_weights);

#endif

