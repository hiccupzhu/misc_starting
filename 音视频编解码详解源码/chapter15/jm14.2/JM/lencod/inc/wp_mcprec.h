/*!
 ***************************************************************************
 * \file
 *    wp_mcprec.h
 *
 * \brief
 *    Headerfile for Improved Motion Compensatation Precision Scheme using Weighted Prediction
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Athanasios Leontaris            <aleon@dolby.com>
 *
 * \date
 *    16 July 2008
 *
 **************************************************************************
 */

#ifndef _WP_MCPREC_H_
#define _WP_MCPREC_H_


#define WP_MCPREC_PLUS0       4
#define WP_MCPREC_PLUS1       5
#define WP_MCPREC_MINUS0      6
#define WP_MCPREC_MINUS1      7
#define WP_MCPREC_MINUS_PLUS0 8
#define WP_REGULAR            9

typedef struct
{
  int PicNum;              // PicNum/FrameNum
  int POCNum;              // POC
}
WeightedPredRefX;

typedef struct
{
  int algorithm;
}
WPXPass;

typedef struct
{
  int               num_wp_ref_list[2];         // num of elements in each of the above matrices [LIST]
  WeightedPredRefX *wp_ref_list[2];             // structure with reordering and WP information for ref frames [LIST]
  WPXPass          *curr_wp_rd_pass;
  WPXPass           wp_rd_passes[3];            // frame_picture [0...4] (MultiRefWeightedPred == 2)
}
WPXObject;

WPXObject *pWPX;

void   wpxInitWPXObject( void );
void   wpxFreeWPXObject( void );
void   wpxInitWPXPasses( InputParameters *params );
void   wpxModifyRefPicList( ImageParameters *img );
// Note that at some point, InputParameters params contents should be copied into ImageParameters *img. 
// This would eliminate need of having to use both structures
int    wpxDetermineWP( InputParameters *params, ImageParameters *img, int clist, int n );
void   wpxAdaptRefNum( ImageParameters *img );
void   wpxReorderLists( ImageParameters *img, Slice *currSlice );

#endif

