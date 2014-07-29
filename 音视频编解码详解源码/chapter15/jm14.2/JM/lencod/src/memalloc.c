
/*!
 ************************************************************************
 * \file  memalloc.c
 *
 * \brief
 *    Memory allocation and free helper functions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 ************************************************************************
 */

#include "global.h"

 /*!
 ************************************************************************
 * \brief
 *    Initialize 2-dimensional top and bottom field to point to the proper
 *    lines in frame
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int init_top_bot_planes(imgpel **imgFrame, int rows, int columns, imgpel ***imgTopField, imgpel ***imgBotField)
{
  int i;

  if((*imgTopField   = (imgpel**)calloc((rows>>1) , sizeof(imgpel*))) == NULL)
    no_mem_exit("init_top_bot_planes: imgTopField");

  if((*imgBotField   = (imgpel**)calloc((rows>>1) , sizeof(imgpel*))) == NULL)
    no_mem_exit("init_top_bot_planes: imgBotField");

  for(i = 0; i < (rows>>1); i++)
  {
    (*imgTopField)[i] =  imgFrame[2*i  ];
    (*imgBotField)[i] =  imgFrame[2*i+1];
  }

  return rows*sizeof(imgpel*);
}

 /*!
 ************************************************************************
 * \brief
 *    free 2-dimensional top and bottom fields without freeing target memory
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
void free_top_bot_planes(imgpel **imgTopField, imgpel **imgBotField)
{
  free (imgTopField);
  free (imgBotField);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> LambdaParams array2D[rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem2Dlm(LambdaParams ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (LambdaParams**)calloc(dim0,       sizeof(LambdaParams*))) == NULL)
    no_mem_exit("get_mem2Dlm: array2D");
  if((*(*array2D) = (LambdaParams* )calloc(dim0 * dim1,sizeof(LambdaParams ))) == NULL)
    no_mem_exit("get_mem2Dlm: array2D");

  for(i = 1 ; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * dim1 * sizeof(LambdaParams);
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dlm()
 ************************************************************************
 */
void free_mem2Dlm(LambdaParams **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2Dlm: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
    error ("free_mem2Dlm: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> PicMotionParams2 array2D[rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem2Dmp(PicMotionParams2 ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (PicMotionParams2**)calloc(dim0,       sizeof(PicMotionParams2*))) == NULL)
    no_mem_exit("get_mem2Dmv: array2D");
  if((*(*array2D) = (PicMotionParams2* )calloc(dim0 * dim1,sizeof(PicMotionParams2 ))) == NULL)
    no_mem_exit("get_mem2Dmp: array2D");

  for(i = 1 ; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * dim1 * sizeof(PicMotionParams2);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> PicMotionParams2 array3D[frames][rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dmp(PicMotionParams2 ****array3D, int dim0, int dim1, int dim2)
{
  int i;

  if(((*array3D) = (PicMotionParams2***)calloc(dim0,sizeof(PicMotionParams2**))) == NULL)
    no_mem_exit("get_mem3Dmp: array3D");

  get_mem2Dmp(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] = (*array3D)[i - 1] + dim1;
  
  return dim0 * dim1 * dim2 * sizeof(PicMotionParams2);
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dmp()
 ************************************************************************
 */
void free_mem2Dmp(PicMotionParams2 **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2Dmp: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
    error ("free_mem2Dmp: trying to free unused memory",100);
  }
}


/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3Dmp()
 ************************************************************************
 */
void free_mem3Dmp(PicMotionParams2 ***array3D)
{
  if (array3D)
  {
    free_mem2Dmp(*array3D);
    free (array3D);
  }
  else
  {
    error ("free_mem3Dmp: trying to free unused memory",100);
  }
}



/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> MotionVector array2D[rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem2Dmv(MotionVector ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (MotionVector**)calloc(dim0,       sizeof(MotionVector*))) == NULL)
    no_mem_exit("get_mem2Dmv: array2D");
  if((*(*array2D) = (MotionVector* )calloc(dim0 * dim1,sizeof(MotionVector ))) == NULL)
    no_mem_exit("get_mem2Dmv: array2D");

  for(i = 1 ; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * dim1 * sizeof(MotionVector);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> MotionVector array3D[frames][rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dmv(MotionVector ****array3D, int dim0, int dim1, int dim2)
{
  int i;

  if(((*array3D) = (MotionVector***)calloc(dim0,sizeof(MotionVector**))) == NULL)
    no_mem_exit("get_mem3Dmv: array3D");

  get_mem2Dmv(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] = (*array3D)[i - 1] + dim1;
  
  return dim0 * dim1 * dim2 * sizeof(MotionVector);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 4D memory array -> MotionVector array3D[dim0][dim1][dim2][dim3]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem4Dmv(MotionVector *****array4D, int dim0, int dim1, int dim2, int dim3)
{
  int i;

  if(((*array4D) = (MotionVector****)calloc(dim0,sizeof(MotionVector***))) == NULL)
    no_mem_exit("get_mem4Dpel: array4D");

  get_mem3Dmv(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] = (*array4D)[i - 1] + dim1;
  
  return dim0 * dim1 * dim2 * dim3 * sizeof(MotionVector);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 5D memory array -> MotionVector array3D[dim0][dim1][dim2][dim3][dim4]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem5Dmv(MotionVector ******array5D, int dim0, int dim1, int dim2, int dim3, int dim4)
{
  int i;

  if(((*array5D) = (MotionVector*****)calloc(dim0,sizeof(MotionVector****))) == NULL)
    no_mem_exit("get_mem5Dpel: array5D");

  get_mem4Dmv(*array5D, dim0 * dim1, dim2, dim3, dim4);

  for(i = 1; i < dim0; i++)
    (*array5D)[i] = (*array5D)[i - 1] + dim1;
  
  return dim0 * dim1 * dim2 * dim3 * dim4 * sizeof(MotionVector);
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dmv()
 ************************************************************************
 */
void free_mem2Dmv(MotionVector **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2Dmv: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
    error ("free_mem2Dmv: trying to free unused memory",100);
  }
}


/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3Dmv()
 ************************************************************************
 */
void free_mem3Dmv(MotionVector ***array3D)
{
  if (array3D)
  {
    free_mem2Dmv(*array3D);
    free (array3D);
  }
  else
  {
    error ("free_mem3Dmv: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 4D memory array
 *    which was allocated with get_mem4Dmv()
 ************************************************************************
 */
void free_mem4Dmv(MotionVector ****array4D)
{
  if (array4D)
  {
    free_mem3Dmv(*array4D);
    free (array4D);
  }
  else
  {
    error ("free_mem4Dmv: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 5D memory array
 *    which was allocated with get_mem5Dmv()
 ************************************************************************
 */
void free_mem5Dmv(MotionVector *****array5D)
{
  if (array5D)
  {
    free_mem4Dmv(*array5D);
    free (array5D);
  }
  else
  {
    error ("free_mem5Dmv: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> imgpel array2D[rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem2Dpel(imgpel ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (imgpel**)calloc(dim0,       sizeof(imgpel*))) == NULL)
    no_mem_exit("get_mem2Dpel: array2D");
  if((*(*array2D) = (imgpel* )calloc(dim0 * dim1,sizeof(imgpel ))) == NULL)
    no_mem_exit("get_mem2Dpel: array2D");

  for(i = 1 ; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * dim1 * sizeof(imgpel);
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> imgpel array3D[frames][rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dpel(imgpel ****array3D, int dim0, int dim1, int dim2)
{
  int i;

  if(((*array3D) = (imgpel***)calloc(dim0,sizeof(imgpel**))) == NULL)
    no_mem_exit("get_mem3Dpel: array3D");

  get_mem2Dpel(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] = (*array3D)[i - 1] + dim1;
  
  return dim0 * dim1 * dim2 * sizeof(imgpel);
}
/*!
 ************************************************************************
 * \brief
 *    Allocate 4D memory array -> imgpel array4D[sub_x][sub_y][rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem4Dpel(imgpel *****array4D, int dim0, int dim1, int dim2, int dim3)
{  
  int  i;

  if(((*array4D) = (imgpel****)calloc(dim0, sizeof(imgpel***))) == NULL)
    no_mem_exit("get_mem4Dpel: array4D");

  get_mem3Dpel(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] = (*array4D)[i - 1] + dim1;

  return dim0 * dim1 * dim2 * dim3 * sizeof(imgpel);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 5D memory array -> imgpel array5D[dims][sub_x][sub_y][rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem5Dpel(imgpel ******array5D, int dim0, int dim1, int dim2, int dim3, int dim4)
{
  int  i;

  if(((*array5D) = (imgpel*****)calloc(dim0, sizeof(imgpel****))) == NULL)
    no_mem_exit("get_mem5Dpel: array5D");

  get_mem4Dpel(*array5D, dim0 * dim1, dim2, dim3, dim4);

  for(i = 1; i < dim0; i++)
    (*array5D)[i] = (*array5D)[i - 1] + dim1;

  return dim0 * dim1 * dim2 * dim3 * dim4 * sizeof(imgpel);
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dpel()
 ************************************************************************
 */
void free_mem2Dpel(imgpel **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2Dpel: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
    error ("free_mem2Dpel: trying to free unused memory",100);
  }
}


/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3Dpel()
 ************************************************************************
 */
void free_mem3Dpel(imgpel ***array3D)
{
  if (array3D)
  {
    free_mem2Dpel(*array3D);
    free (array3D);
  }
  else
  {
    error ("free_mem3Dpel: trying to free unused memory",100);
  }
}
/*!
 ************************************************************************
 * \brief
 *    free 4D memory array
 *    which was allocated with get_mem4Dpel()
 ************************************************************************
 */
void free_mem4Dpel(imgpel ****array4D)
{
  if (array4D)
  {
    free_mem3Dpel(*array4D);
    free (array4D);
  }
  else
  {
    error ("free_mem4Dpel: trying to free unused memory",100);
  }
}
/*!
 ************************************************************************
 * \brief
 *    free 5D memory array
 *    which was allocated with get_mem5Dpel()
 ************************************************************************
 */
void free_mem5Dpel(imgpel *****array5D)
{
  if (array5D)
  {
    free_mem4Dpel(*array5D);
    free (array5D);
  }
  else
  {
    error ("free_mem5Dpel: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> unsigned char array2D[rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem2D(byte ***array2D, int dim0, int dim1)
{
  int i;

  if((  *array2D  = (byte**)calloc(dim0,       sizeof(byte*))) == NULL)
    no_mem_exit("get_mem2D: array2D");
  if((*(*array2D) = (byte* )calloc(dim0 * dim1,sizeof(byte ))) == NULL)
    no_mem_exit("get_mem2D: array2D");

  for(i = 1; i < dim0; i++)
    (*array2D)[i] = (*array2D)[i-1] + dim1;

  return dim0 * dim1;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> int array2D[rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Dint(int ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (int**)calloc(dim0,        sizeof(int*))) == NULL)
    no_mem_exit("get_mem2Dint: array2D");
  if((*(*array2D) = (int* )calloc(dim0 * dim1, sizeof(int ))) == NULL)
    no_mem_exit("get_mem2Dint: array2D");

  for(i = 1 ; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * dim1 * sizeof(int);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> int64 array2D[rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Dint64(int64 ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (int64**)calloc(dim0,       sizeof(int64*))) == NULL)
    no_mem_exit("get_mem2Dint64: array2D");
  if((*(*array2D) = (int64* )calloc(dim0 * dim1,sizeof(int64 ))) == NULL)
    no_mem_exit("get_mem2Dint64: array2D");

  for(i = 1; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * dim1 * sizeof(int64);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> unsigned char array3D[frames][rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3D(byte ****array3D, int dim0, int dim1, int dim2)
{
  int  i;

  if(((*array3D) = (byte***)calloc(dim0,sizeof(byte**))) == NULL)
    no_mem_exit("get_mem3D: array3D");

  get_mem2D(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1;

  return dim0 * dim1 * dim2;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> int array3D[frames][rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dint(int ****array3D, int dim0, int dim1, int dim2)
{
  int  i;

  if(((*array3D) = (int***)calloc(dim0, sizeof(int**))) == NULL)
    no_mem_exit("get_mem3Dint: array3D");

  get_mem2Dint(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1;

  return dim0 * dim1 * dim2 * sizeof(int);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> int64 array3D[frames][rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dint64(int64 ****array3D, int dim0, int dim1, int dim2)
{
  int  i;

  if(((*array3D) = (int64***)calloc(dim0, sizeof(int64**))) == NULL)
    no_mem_exit("get_mem3Dint64: array3D");

  get_mem2Dint64(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1;

  return dim0 * dim1 * dim2 * sizeof(int64);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 4D memory array -> int array4D[frames][rows][columns][component]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem4Dint(int *****array4D, int dim0, int dim1, int dim2, int dim3)
{
  int  i;

  if(((*array4D) = (int****)calloc(dim0, sizeof(int***))) == NULL)
    no_mem_exit("get_mem4Dint: array4D");

  get_mem3Dint(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] =  (*array4D)[i-1] + dim1;

  return dim0 * dim1 * dim2 * dim3 * sizeof(int);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 5D memory array -> int array5D[refs][blocktype][rows][columns][component]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem5Dint(int ******array5D, int dim0, int dim1, int dim2, int dim3, int dim4)
{
  int  i;

  if(((*array5D) = (int*****)calloc(dim0, sizeof(int****))) == NULL)
    no_mem_exit("get_mem5Dint: array5D");

  get_mem4Dint(*array5D, dim0 * dim1, dim2, dim3, dim4);

  for(i = 1; i < dim0; i++)
    (*array5D)[i] =  (*array5D)[i-1] + dim1;

  return dim0 * dim1 * dim2 * dim3 * dim4 * sizeof(int);
}


/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2D()
 ************************************************************************
 */
void free_mem2D(byte **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2D: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
    error ("free_mem2D: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dint()
 ************************************************************************
 */
void free_mem2Dint(int **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2Dint: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
    error ("free_mem2Dint: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dint64()
 ************************************************************************
 */
void free_mem2Dint64(int64 **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2Dint64: trying to free unused memory",100);
    free (array2D);
  } 
  else
  {
    error ("free_mem2Dint64: trying to free unused memory",100);
  }
}


/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3D()
 ************************************************************************
 */
void free_mem3D(byte ***array3D)
{
  if (array3D)
  {
   free_mem2D(*array3D);
   free (array3D);
  } 
  else
  {
    error ("free_mem3D: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3Dint()
 ************************************************************************
 */
void free_mem3Dint(int ***array3D)
{
  if (array3D)
  {
   free_mem2Dint(*array3D);
   free (array3D);
  } 
  else
  {
    error ("free_mem3Dint: trying to free unused memory",100);
  }
}


/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3Dint64()
 ************************************************************************
 */
void free_mem3Dint64(int64 ***array3D)
{
  if (array3D)
  {
   free_mem2Dint64(*array3D);
   free (array3D);
  } 
  else
  {
    error ("free_mem3Dint64: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 4D memory array
 *    which was allocated with get_mem4Dint()
 ************************************************************************
 */
void free_mem4Dint(int ****array4D)
{
  if (array4D)
  {
    free_mem3Dint( *array4D);
    free (array4D);
  } else
  {
    error ("free_mem4Dint: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 5D int memory array
 *    which was allocated with get_mem5Dint()
 ************************************************************************
 */
void free_mem5Dint(int *****array5D)
{
  if (array5D)
  {
    free_mem4Dint( *array5D);
    free (array5D);
  } else
  {
    error ("free_mem5Dint: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    Exit program if memory allocation failed (using error())
 * \param where
 *    string indicating which memory allocation failed
 ************************************************************************
 */
void no_mem_exit(char *where)
{
   snprintf(errortext, ET_SIZE, "Could not allocate memory: %s",where);
   error (errortext, 100);
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 2D short memory array -> short array2D[rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Dshort(short ***array2D, int dim0, int dim1)
{
  int i;

  if((  *array2D  = (short**)calloc(dim0,       sizeof(short*))) == NULL)
    no_mem_exit("get_mem2Dshort: array2D");
  if((*(*array2D) = (short* )calloc(dim0 * dim1,sizeof(short ))) == NULL)
    no_mem_exit("get_mem2Dshort: array2D");

  for(i = 1; i < dim0; i++)
    (*array2D)[i] = (*array2D)[i-1] + dim1;

  return dim0 * dim1 * sizeof(short);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory short array -> short array3D[frames][rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dshort(short ****array3D,int dim0, int dim1, int dim2)
{
  int  i;

  if(((*array3D) = (short***)calloc(dim0, sizeof(short**))) == NULL)
    no_mem_exit("get_mem3Dshort: array3D");

  get_mem2Dshort(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1;

  return dim0 * dim1 * dim2 * sizeof(short);
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 4D memory short array -> short array3D[frames][rows][columns][component]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem4Dshort(short *****array4D, int dim0, int dim1, int dim2, int dim3)
{
  int  i;

  if(((*array4D) = (short****)calloc(dim0, sizeof(short***))) == NULL)
    no_mem_exit("get_mem4Dshort: array4D");

  get_mem3Dshort(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] =  (*array4D)[i-1] + dim1;

  return dim0 * dim1 * dim2 * dim3 * sizeof(short);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 5D memory array -> short array5D[refs][blocktype][rows][columns][component]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem5Dshort(short ******array5D, int dim0, int dim1, int dim2, int dim3, int dim4)
{
  int  i;

  if(((*array5D) = (short*****)calloc(dim0, sizeof(short****))) == NULL)
    no_mem_exit("get_mem5Dshort: array5D");

  get_mem4Dshort(*array5D, dim0 * dim1, dim2, dim3, dim4);

  for(i = 1; i < dim0; i++)
    (*array5D)[i] =  (*array5D)[i-1] + dim1;

  return dim0 * dim1 * dim2 * dim3 * dim4 * sizeof(short);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 6D memory array -> short array6D[list][refs][blocktype][rows][columns][component]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem6Dshort(short *******array6D, int dim0, int dim1, int dim2, int dim3, int dim4, int dim5)
{
  int  i;

  if(((*array6D) = (short******)calloc(dim0, sizeof(short*****))) == NULL)
    no_mem_exit("get_mem6Dshort: array6D");

  get_mem5Dshort(*array6D, dim0 * dim1, dim2, dim3, dim4, dim5);

  for(i = 1; i < dim0; i++)
    (*array6D)[i] =  (*array6D)[i-1] + dim1;

  return dim0 * dim1 * dim2 * dim3 * dim4 * dim5 * sizeof(short);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 7D memory array -> short array7D[list][refs][blocktype][rows][columns][component]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem7Dshort(short ********array7D, int dim0, int dim1, int dim2, int dim3, int dim4, int dim5, int dim6)
{
  int  i;

  if(((*array7D) = (short*******)calloc(dim0, sizeof(short******))) == NULL)
    no_mem_exit("get_mem7Dshort: array7D");

  get_mem6Dshort(*array7D, dim0 * dim1, dim2, dim3, dim4, dim5, dim6);

  for(i = 1; i < dim0; i++)
    (*array7D)[i] =  (*array7D)[i-1] + dim1;

  return dim0 * dim1 * dim2 * dim3 * dim4 * dim5 * dim6 * sizeof(short);
}

/*!
 ************************************************************************
 * \brief
 *    free 2D short memory array
 *    which was allocated with get_mem2Dshort()
 ************************************************************************
 */
void free_mem2Dshort(short **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else error ("free_mem2Dshort: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
    error ("free_mem2Dshort: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 3D short memory array
 *    which was allocated with get_mem3Dshort()
 ************************************************************************
 */
void free_mem3Dshort(short ***array3D)
{
  if (array3D)
  {
   free_mem2Dshort(*array3D);
   free (array3D);
  } 
  else
  {
    error ("free_mem3Dshort: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 4D short memory array
 *    which was allocated with get_mem4Dshort()
 ************************************************************************
 */
void free_mem4Dshort(short ****array4D)
{  
  if (array4D)
  {
    free_mem3Dshort( *array4D);
    free (array4D);
  } 
  else
  {
    error ("free_mem4Dshort: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 5D short memory array
 *    which was allocated with get_mem5Dshort()
 ************************************************************************
 */
void free_mem5Dshort(short *****array5D)
{
  if (array5D)
  {
    free_mem4Dshort( *array5D) ;
    free (array5D);
  }
  else
  {
    error ("free_mem5Dshort: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 6D short memory array
 *    which was allocated with get_mem6Dshort()
 ************************************************************************
 */
void free_mem6Dshort(short ******array6D)
{
  if (array6D)
  {
    free_mem5Dshort( *array6D);
    free (array6D);
  }
  else
  {
    error ("free_mem6Dshort: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 7D short memory array
 *    which was allocated with get_mem7Dshort()
 ************************************************************************
 */
void free_mem7Dshort(short *******array7D)
{
  if (array7D)
  {
    free_mem6Dshort( *array7D);
    free (array7D);
  }
  else
  {
    error ("free_mem7Dshort: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> double array2D[rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Ddouble(double ***array2D, int rows, int columns)
{
  int i;

  if((*array2D      = (double**)calloc(rows,        sizeof(double*))) == NULL)
    no_mem_exit("get_mem2Ddouble: array2D");
  
  if(((*array2D)[0] = (double* )calloc(rows*columns,sizeof(double ))) == NULL)
    no_mem_exit("get_mem2Ddouble: array2D");

  for(i=1 ; i<rows ; i++)
    (*array2D)[i] =  (*array2D)[i-1] + columns  ;

  return rows*columns*sizeof(double);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> double array2D[rows][columns]
 *    Note that array is shifted towards offset allowing negative values
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Dodouble(double ***array2D, int rows, int columns, int offset)
{
  int i;

  if((*array2D      = (double**)calloc(rows,        sizeof(double*))) == NULL)
    no_mem_exit("get_mem2Dodouble: array2D");
  if(((*array2D)[0] = (double* )calloc(rows*columns,sizeof(double ))) == NULL)
    no_mem_exit("get_mem2Dodouble: array2D");

  (*array2D)[0] += offset;

  for(i=1 ; i<rows ; i++)
    (*array2D)[i] =  (*array2D)[i-1] + columns  ;

  return rows*columns*sizeof(double);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory double array -> double array3D[pels][rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dodouble(double ****array3D, int rows, int columns, int pels, int offset)
{
  int  i,j;

  if(((*array3D) = (double***)calloc(rows,sizeof(double**))) == NULL)
    no_mem_exit("get_mem3Dodouble: array3D");

  if(((*array3D)[0] = (double** )calloc(rows*columns,sizeof(double*))) == NULL)
    no_mem_exit("get_mem3Dodouble: array3D");

  (*array3D) [0] += offset;

  for(i=1 ; i<rows ; i++)
    (*array3D)[i] =  (*array3D)[i-1] + columns  ;

  for (i = 0; i < rows; i++)
    for (j = -offset; j < columns - offset; j++)
      if(((*array3D)[i][j] = (double* )calloc(pels,sizeof(double))) == NULL)
        no_mem_exit("get_mem3Dodouble: array3D");

  return rows*columns*pels*sizeof(double);
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> int array2D[rows][columns]
 *    Note that array is shifted towards offset allowing negative values
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_offset_mem2Dshort(short ***array2D, int rows, int columns, int offset_y, int offset_x)
{
  int i;

  if((*array2D      = (short**)calloc(rows, sizeof(short*))) == NULL)
    no_mem_exit("get_offset_mem2Dshort: array2D");

  if(((*array2D)[0] = (short* )calloc(rows * columns,sizeof(short))) == NULL)
    no_mem_exit("get_offset_mem2Dshort: array2D");
  (*array2D)[0] += offset_x + offset_y * columns;

  for(i=-1 ; i > -offset_y - 1; i--)
  {
    (*array2D)[i] =  (*array2D)[i+1] - columns;
  }

  for(i=1 ; i < columns - offset_y; i++)
    (*array2D)[i] =  (*array2D)[i-1] + columns;

  return rows * columns * sizeof(short);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory int array -> int array3D[rows][columns][pels]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Doint(int ****array3D, int rows, int columns, int pels, int offset)
{
  int  i,j;

  if(((*array3D) = (int***)calloc(rows,sizeof(int**))) == NULL)
    no_mem_exit("get_mem3Doint: array3D");

  if(((*array3D)[0] = (int** )calloc(rows*columns,sizeof(int*))) == NULL)
    no_mem_exit("get_mem3Doint: array3D");

  (*array3D) [0] += offset;

  for(i=1 ; i<rows ; i++)
    (*array3D)[i] =  (*array3D)[i-1] + columns  ;

  for (i = 0; i < rows; i++)
    for (j = -offset; j < columns - offset; j++)
      if(((*array3D)[i][j] = (int* )calloc(pels,sizeof(int))) == NULL)
        no_mem_exit("get_mem3Doint: array3D");

  return rows*columns*pels*sizeof(int);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> int array2D[rows][columns]
 *    Note that array is shifted towards offset allowing negative values
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Doint(int ***array2D, int rows, int columns, int offset)
{
  int i;

  if((*array2D      = (int**)calloc(rows, sizeof(int*))) == NULL)
    no_mem_exit("get_mem2Dint: array2D");
  if(((*array2D)[0] = (int* )calloc(rows*columns,sizeof(int))) == NULL)
    no_mem_exit("get_mem2Dint: array2D");

  (*array2D)[0] += offset;

  for(i=1 ; i<rows ; i++)
    (*array2D)[i] =  (*array2D)[i-1] + columns  ;

  return rows*columns*sizeof(int);
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> int array3D[frames][rows][columns]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
// same change as in get_mem3Dint
int get_mem3Ddouble(double ****array3D, int frames, int rows, int columns)
{
	int  j;

  double **array2D;

  get_mem2Ddouble(&array2D, frames * rows, columns);

  if(((*array3D) = (double***)calloc(frames,sizeof(double**))) == NULL)
    no_mem_exit("get_mem3Ddouble: array3D");

  for(j=0;j<frames;j++)
  {    
    (*array3D)[j] = &array2D[j * rows];
  }

  return frames*rows*columns*sizeof(double);
}

/*!
 ************************************************************************
 * \brief
 *    free 2D double memory array
 *    which was allocated with get_mem2Ddouble()
 ************************************************************************
 */
void free_mem2Ddouble(double **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2Ddouble: trying to free unused memory",100);

    free (array2D);

  }
  else
  {
    error ("free_mem2Ddouble: trying to free unused memory",100);
  }
}


/*!
************************************************************************
* \brief
*    free 2D double memory array (with offset)
*    which was allocated with get_mem2Ddouble()
************************************************************************
*/
void free_mem2Dodouble(double **array2D, int offset)
{
  if (array2D)
  {
    array2D[0] -= offset;
    if (array2D[0])
      free (array2D[0]);
    else error ("free_mem2Dodouble: trying to free unused memory",100);

    free (array2D);

  } else
  {
    error ("free_mem2Dodouble: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 3D memory array with offset
 ************************************************************************
 */
void free_mem3Dodouble(double ***array3D, int rows, int columns, int offset)
{
  int i, j;

  if (array3D)
  {
    for (i = 0; i < rows; i++)
    {
      for (j = -offset; j < columns - offset; j++)
      {
        if (array3D[i][j])
          free(array3D[i][j]);
        else
          error ("free_mem3Dodouble: trying to free unused memory",100);
      }
    }
    array3D[0] -= offset;
    if (array3D[0])
      free(array3D[0]);
    else
      error ("free_mem3Dodouble: trying to free unused memory",100);
    free (array3D);
  }
  else
  {
    error ("free_mem3Dodouble: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 3D memory array with offset
 ************************************************************************
 */
void free_mem3Doint(int ***array3D, int rows, int columns, int offset)
{
  int i, j;

  if (array3D)
  {
    for (i = 0; i < rows; i++)
    {
      for (j = -offset; j < columns - offset; j++)
      {
        if (array3D[i][j])
          free(array3D[i][j]);
        else
          error ("free_mem3Doint: trying to free unused memory",100);
      }
    }
    array3D[0] -= offset;
    if (array3D[0])
      free(array3D[0]);
    else
      error ("free_mem3Doint: trying to free unused memory",100);
    free (array3D);
  }
  else
  {
    error ("free_mem3Doint: trying to free unused memory",100);
  }
}


/*!
************************************************************************
* \brief
*    free 2D double memory array (with offset)
*    which was allocated with get_mem2Ddouble()
************************************************************************
*/
void free_mem2Doint(int **array2D, int offset)
{
  if (array2D)
  {
    array2D[0] -= offset;
    if (array2D[0])
      free (array2D[0]);
    else 
      error ("free_mem2Doint: trying to free unused memory",100);

    free (array2D);

  } 
  else
  {
    error ("free_mem2Doint: trying to free unused memory",100);
  }
}

/*!
************************************************************************
* \brief
*    free 2D double memory array (with offset)
*    which was allocated with get_mem2Ddouble()
************************************************************************
*/
void free_offset_mem2Dshort(short **array2D, int columns, int offset_y, int offset_x)
{
  if (array2D)
  {
    array2D[0] -= offset_x + offset_y * columns;
    if (array2D[0])
      free (array2D[0]);
    else 
      error ("free_offset_mem2Dshort: trying to free unused memory",100);

    free (array2D);

  } 
  else
  {
    error ("free_offset_mem2Dshort: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was alocated with get_mem3Dint()
 ************************************************************************
 */
void free_mem3Ddouble(double ***array3D)
{
	if (array3D)
	{
  	free_mem2Ddouble(*array3D);
		free (array3D);
	} 
	else
	{
		error ("free_mem3D: trying to free unused memory",100);
	}
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> LambdaParams array2D[rows][columns]
 *    Note that array is shifted towards offset allowing negative values
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Dolm(LambdaParams ***array2D, int rows, int columns, int offset)
{
  int i;

  if((*array2D      = (LambdaParams**) calloc(rows, sizeof(LambdaParams*))) == NULL)
    no_mem_exit("get_mem2Dolm: array2D");
  if(((*array2D)[0] = (LambdaParams* ) calloc(rows*columns,sizeof(LambdaParams))) == NULL)
    no_mem_exit("get_mem2Dolm: array2D");

  (*array2D)[0] += offset;

  for(i=1 ; i<rows ; i++)
    (*array2D)[i] =  (*array2D)[i-1] + columns  ;

  return rows*columns*sizeof(LambdaParams);
}


/*!
************************************************************************
* \brief
*    free 2D LambdaParams memory array (with offset)
*    which was allocated with get_mem2Dlm()
************************************************************************
*/
void free_mem2Dolm(LambdaParams **array2D, int offset)
{
  if (array2D)
  {
    array2D[0] -= offset;
    if (array2D[0])
      free (array2D[0]);
    else 
      error ("free_mem2Dolm: trying to free unused memory",100);

    free (array2D);

  } 
  else
  {
    error ("free_mem2Dolm: trying to free unused memory",100);
  }
}

