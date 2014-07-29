
/*!
 ***************************************************************************
 *
 * \file transform8x8.h
 *
 * \brief
*    prototypes of 8x8 transform functions
  *
 * \date
 *    9. October 2003
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    Yuri Vatis
 **************************************************************************/

#ifndef _TRANSFORM8X8_H_
#define _TRANSFORM8X8_H_

int    Mode_Decision_for_new_Intra8x8Macroblock (Macroblock *currMB, double lambda, double *min_cost);
int    (*Mode_Decision_for_new_8x8IntraBlocks)(Macroblock *currMB, int b8, double lambda, double *min_cost, int cr_cbp[3]);
int    Mode_Decision_for_new_8x8IntraBlocks_JM_Low(Macroblock *currMB, int b8, double lambda, double *min_cost, int cr_cbp[3]);
int    Mode_Decision_for_new_8x8IntraBlocks_JM_High(Macroblock *currMB, int b8, double lambda, double *min_cost, int cr_cbp[3]);

double RDCost_for_8x8IntraBlocks(Macroblock *currMB, int *c_nz, int b8, int ipmode, double lambda, double min_rdcost, int mostProbableMode, int c_nzCbCr[3]);
void   compute_comp_cost8x8(imgpel **cur_img, imgpel mpr8x8[16][16], int pic_opix_x, double *cost);

void   intrapred_8x8(Macroblock *currMB, ColorPlane pl, int img_x,int img_y, int *left_available, int *up_available, int *all_available);
void   LowPassForIntra8x8Pred(imgpel *PredPel, int block_up_left, int block_up, int block_left);

#endif //_TRANSFORM8X8_H_
