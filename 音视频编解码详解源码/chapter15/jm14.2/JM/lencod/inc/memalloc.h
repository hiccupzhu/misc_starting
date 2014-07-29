
/*!
 ************************************************************************
 * \file  memalloc.h
 *
 * \brief
 *    Memory allocation and free helper funtions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Karsten Sühring                 <suehring@hhi.de> 
 *     - Alexis Michael Tourapis         <alexismt@ieee.org> 
 *
 ************************************************************************
 */

#ifndef _MEMALLOC_H_
#define _MEMALLOC_H_

int  get_mem2Dlm (LambdaParams ***array2D, int dim0, int dim1);
int  get_mem2Dolm(LambdaParams ***array2D, int dim0, int dim1, int offset);

int  get_mem2Dmp(PicMotionParams2 ***array2D, int dim0, int dim1);
int  get_mem3Dmp(PicMotionParams2 ****array3D, int dim0, int dim1, int dim2);

int  get_mem2Dmv(MotionVector ***array2D, int dim0, int dim1);
int  get_mem3Dmv(MotionVector ****array3D, int dim0, int dim1, int dim2);
int  get_mem4Dmv(MotionVector *****array4D, int dim0, int dim1, int dim2, int dim3);
int  get_mem5Dmv(MotionVector ******array5D, int dim0, int dim1, int dim2, int dim3, int dim4);

int  get_mem2D(byte ***array2D, int rows, int columns);
int  get_mem3D(byte ****array2D, int frames, int rows, int columns);

int  get_mem2Dint(int ***array2D, int rows, int columns);
int  get_mem3Dint(int ****array3D, int frames, int rows, int columns);
int  get_mem4Dint(int *****array4D, int idx, int frames, int rows, int columns );
int  get_mem5Dint(int ******array5D, int refs, int blocktype, int rows, int columns, int component);

int  get_mem2Dint64(int64 ***array2D, int rows, int columns);
int  get_mem3Dint64(int64 ****array3D, int frames, int rows, int columns);

int  get_mem2Dshort(short ***array2D, int dim0, int dim1);
int  get_mem3Dshort(short ****array3D, int dim0, int dim1, int dim2);
int  get_mem4Dshort(short *****array4D, int dim0, int dim1, int dim2, int dim3);
int  get_mem5Dshort(short ******array5D, int dim0, int dim1, int dim2, int dim3, int dim4);
int  get_mem6Dshort(short *******array6D, int dim0, int dim1, int dim2, int dim3, int dim4, int dim5);
int  get_mem7Dshort(short ********array7D, int dim0, int dim1, int dim2, int dim3, int dim4, int dim5, int dim6);

int  get_mem2Dpel(imgpel ***array2D, int rows, int columns);
int  get_mem3Dpel(imgpel ****array3D, int frames, int rows, int columns);
int  get_mem4Dpel(imgpel *****array4D, int sub_x, int sub_y, int rows, int columns);
int  get_mem5Dpel(imgpel ******array5D, int dims, int sub_x, int sub_y, int rows, int columns);

int  get_mem2Ddouble (double ***array2D, int rows, int columns);
int  get_mem2Dodouble(double ***array2D, int rows, int columns, int offset);
int  get_mem3Dodouble(double ****array2D, int rows, int columns, int pels, int offset);

int  get_mem2Doint (int ***array2D, int rows, int columns, int offset);
int  get_mem3Doint (int ****array3D, int rows, int columns, int pels, int offset);

int  get_offset_mem2Dshort(short ***array2D, int rows, int columns, int offset_y, int offset_x);

void free_offset_mem2Dshort(short **array2D, int columns, int offset_x, int offset_y);

void free_mem2Dlm   (LambdaParams **array2D);
void free_mem2Dolm  (LambdaParams **array2D, int offset);

void free_mem2Dmp   (PicMotionParams2    **array2D);
void free_mem3Dmp   (PicMotionParams2   ***array2D);

void free_mem2Dmv   (MotionVector    **array2D);
void free_mem3Dmv   (MotionVector   ***array2D);
void free_mem4Dmv   (MotionVector  ****array2D);
void free_mem5Dmv   (MotionVector *****array2D);

void free_mem2D     (byte      **array2D);
void free_mem3D     (byte     ***array2D);

void free_mem2Dint  (int       **array2D);
void free_mem3Dint  (int      ***array3D);
void free_mem4Dint  (int     ****array4D);
void free_mem5Dint  (int    *****array5D);

void free_mem2Dint64(int64     **array2D);
void free_mem3Dint64(int64    ***array3D);

void free_mem2Dshort(short      **array2D);
void free_mem3Dshort(short     ***array3D);
void free_mem4Dshort(short    ****array4D);
void free_mem5Dshort(short   *****array5D);
void free_mem6Dshort(short  ******array6D);
void free_mem7Dshort(short *******array7D);

void free_mem2Dpel  (imgpel    **array2D);
void free_mem3Dpel  (imgpel   ***array3D);
void free_mem4Dpel  (imgpel  ****array4D);
void free_mem5Dpel  (imgpel *****array5D);

void free_mem2Ddouble(double **array2D);

void free_mem2Dodouble(double **array2D, int offset);
void free_mem3Dodouble(double ***array3D, int rows, int columns, int offset);
void free_mem2Doint   (int **array2D, int offset);
void free_mem3Doint   (int ***array3D, int rows, int columns, int offset);

int init_top_bot_planes(imgpel **imgFrame, int rows, int columns, imgpel ***imgTopField, imgpel ***imgBotField);
void free_top_bot_planes(imgpel **imgTopField, imgpel **imgBotField);


void no_mem_exit(char *where);

#endif
