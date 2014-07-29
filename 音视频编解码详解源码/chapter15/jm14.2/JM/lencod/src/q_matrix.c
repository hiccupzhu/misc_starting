
/*!
 *************************************************************************************
 * \file q_matrix.c
 *
 * \brief
 *    read q_matrix parameters from input file: q_matrix.cfg
 *
 *************************************************************************************
 */

#include "global.h"
#include "memalloc.h"

extern char *GetConfigFileContent (char *Filename, int error_type);

#define MAX_ITEMS_TO_PARSE  1000

const int quant_coef[6][4][4] = {
  {{13107, 8066,13107, 8066},{ 8066, 5243, 8066, 5243},{13107, 8066,13107, 8066},{ 8066, 5243, 8066, 5243}},
  {{11916, 7490,11916, 7490},{ 7490, 4660, 7490, 4660},{11916, 7490,11916, 7490},{ 7490, 4660, 7490, 4660}},
  {{10082, 6554,10082, 6554},{ 6554, 4194, 6554, 4194},{10082, 6554,10082, 6554},{ 6554, 4194, 6554, 4194}},
  {{ 9362, 5825, 9362, 5825},{ 5825, 3647, 5825, 3647},{ 9362, 5825, 9362, 5825},{ 5825, 3647, 5825, 3647}},
  {{ 8192, 5243, 8192, 5243},{ 5243, 3355, 5243, 3355},{ 8192, 5243, 8192, 5243},{ 5243, 3355, 5243, 3355}},
  {{ 7282, 4559, 7282, 4559},{ 4559, 2893, 4559, 2893},{ 7282, 4559, 7282, 4559},{ 4559, 2893, 4559, 2893}}
};

const int dequant_coef[6][4][4] = {
  {{10, 13, 10, 13},{ 13, 16, 13, 16},{10, 13, 10, 13},{ 13, 16, 13, 16}},
  {{11, 14, 11, 14},{ 14, 18, 14, 18},{11, 14, 11, 14},{ 14, 18, 14, 18}},
  {{13, 16, 13, 16},{ 16, 20, 16, 20},{13, 16, 13, 16},{ 16, 20, 16, 20}},
  {{14, 18, 14, 18},{ 18, 23, 18, 23},{14, 18, 14, 18},{ 18, 23, 18, 23}},
  {{16, 20, 16, 20},{ 20, 25, 20, 25},{16, 20, 16, 20},{ 20, 25, 20, 25}},
  {{18, 23, 18, 23},{ 23, 29, 23, 29},{18, 23, 18, 23},{ 23, 29, 23, 29}}
};

const int quant_coef8[6][8][8] =
{
  {
    {13107, 12222,  16777,  12222,  13107,  12222,  16777,  12222},
    {12222, 11428,  15481,  11428,  12222,  11428,  15481,  11428},
    {16777, 15481,  20972,  15481,  16777,  15481,  20972,  15481},
    {12222, 11428,  15481,  11428,  12222,  11428,  15481,  11428},
    {13107, 12222,  16777,  12222,  13107,  12222,  16777,  12222},
    {12222, 11428,  15481,  11428,  12222,  11428,  15481,  11428},
    {16777, 15481,  20972,  15481,  16777,  15481,  20972,  15481},
    {12222, 11428,  15481,  11428,  12222,  11428,  15481,  11428}
  },
  {
    {11916, 11058,  14980,  11058,  11916,  11058,  14980,  11058},
    {11058, 10826,  14290,  10826,  11058,  10826,  14290,  10826},
    {14980, 14290,  19174,  14290,  14980,  14290,  19174,  14290},
    {11058, 10826,  14290,  10826,  11058,  10826,  14290,  10826},
    {11916, 11058,  14980,  11058,  11916,  11058,  14980,  11058},
    {11058, 10826,  14290,  10826,  11058,  10826,  14290,  10826},
    {14980, 14290,  19174,  14290,  14980,  14290,  19174,  14290},
    {11058, 10826,  14290,  10826,  11058,  10826,  14290,  10826}
  },
  {
    {10082, 9675, 12710,  9675, 10082,  9675, 12710,  9675},
    {9675,  8943, 11985,  8943, 9675, 8943, 11985,  8943},
    {12710, 11985,  15978,  11985,  12710,  11985,  15978,  11985},
    {9675,  8943, 11985,  8943, 9675, 8943, 11985,  8943},
    {10082, 9675, 12710,  9675, 10082,  9675, 12710,  9675},
    {9675,  8943, 11985,  8943, 9675, 8943, 11985,  8943},
    {12710, 11985,  15978,  11985,  12710,  11985,  15978,  11985},
    {9675,  8943, 11985,  8943, 9675, 8943, 11985,  8943}
  },
  {
    {9362,  8931, 11984,  8931, 9362, 8931, 11984,  8931},
    {8931,  8228, 11259,  8228, 8931, 8228, 11259,  8228},
    {11984, 11259,  14913,  11259,  11984,  11259,  14913,  11259},
    {8931,  8228, 11259,  8228, 8931, 8228, 11259,  8228},
    {9362,  8931, 11984,  8931, 9362, 8931, 11984,  8931},
    {8931,  8228, 11259,  8228, 8931, 8228, 11259,  8228},
    {11984, 11259,  14913,  11259,  11984,  11259,  14913,  11259},
    {8931,  8228, 11259,  8228, 8931, 8228, 11259,  8228}
  },
  {
    {8192,  7740, 10486,  7740, 8192, 7740, 10486,  7740},
    {7740,  7346, 9777, 7346, 7740, 7346, 9777, 7346},
    {10486, 9777, 13159,  9777, 10486,  9777, 13159,  9777},
    {7740,  7346, 9777, 7346, 7740, 7346, 9777, 7346},
    {8192,  7740, 10486,  7740, 8192, 7740, 10486,  7740},
    {7740,  7346, 9777, 7346, 7740, 7346, 9777, 7346},
    {10486, 9777, 13159,  9777, 10486,  9777, 13159,  9777},
    {7740,  7346, 9777, 7346, 7740, 7346, 9777, 7346}
  },
  {
    {7282,  6830, 9118, 6830, 7282, 6830, 9118, 6830},
    {6830,  6428, 8640, 6428, 6830, 6428, 8640, 6428},
    {9118,  8640, 11570,  8640, 9118, 8640, 11570,  8640},
    {6830,  6428, 8640, 6428, 6830, 6428, 8640, 6428},
    {7282,  6830, 9118, 6830, 7282, 6830, 9118, 6830},
    {6830,  6428, 8640, 6428, 6830, 6428, 8640, 6428},
    {9118,  8640, 11570,  8640, 9118, 8640, 11570,  8640},
    {6830,  6428, 8640, 6428, 6830, 6428, 8640, 6428}
  }
};



const int dequant_coef8[6][8][8] =
{
  {
    {20,  19, 25, 19, 20, 19, 25, 19},
    {19,  18, 24, 18, 19, 18, 24, 18},
    {25,  24, 32, 24, 25, 24, 32, 24},
    {19,  18, 24, 18, 19, 18, 24, 18},
    {20,  19, 25, 19, 20, 19, 25, 19},
    {19,  18, 24, 18, 19, 18, 24, 18},
    {25,  24, 32, 24, 25, 24, 32, 24},
    {19,  18, 24, 18, 19, 18, 24, 18}
  },
  {
    {22,  21, 28, 21, 22, 21, 28, 21},
    {21,  19, 26, 19, 21, 19, 26, 19},
    {28,  26, 35, 26, 28, 26, 35, 26},
    {21,  19, 26, 19, 21, 19, 26, 19},
    {22,  21, 28, 21, 22, 21, 28, 21},
    {21,  19, 26, 19, 21, 19, 26, 19},
    {28,  26, 35, 26, 28, 26, 35, 26},
    {21,  19, 26, 19, 21, 19, 26, 19}
  },
  {
    {26,  24, 33, 24, 26, 24, 33, 24},
    {24,  23, 31, 23, 24, 23, 31, 23},
    {33,  31, 42, 31, 33, 31, 42, 31},
    {24,  23, 31, 23, 24, 23, 31, 23},
    {26,  24, 33, 24, 26, 24, 33, 24},
    {24,  23, 31, 23, 24, 23, 31, 23},
    {33,  31, 42, 31, 33, 31, 42, 31},
    {24,  23, 31, 23, 24, 23, 31, 23}
  },
  {
    {28,  26, 35, 26, 28, 26, 35, 26},
    {26,  25, 33, 25, 26, 25, 33, 25},
    {35,  33, 45, 33, 35, 33, 45, 33},
    {26,  25, 33, 25, 26, 25, 33, 25},
    {28,  26, 35, 26, 28, 26, 35, 26},
    {26,  25, 33, 25, 26, 25, 33, 25},
    {35,  33, 45, 33, 35, 33, 45, 33},
    {26,  25, 33, 25, 26, 25, 33, 25}
  },
  {
    {32,  30, 40, 30, 32, 30, 40, 30},
    {30,  28, 38, 28, 30, 28, 38, 28},
    {40,  38, 51, 38, 40, 38, 51, 38},
    {30,  28, 38, 28, 30, 28, 38, 28},
    {32,  30, 40, 30, 32, 30, 40, 30},
    {30,  28, 38, 28, 30, 28, 38, 28},
    {40,  38, 51, 38, 40, 38, 51, 38},
    {30,  28, 38, 28, 30, 28, 38, 28}
  },
  {
    {36,  34, 46, 34, 36, 34, 46, 34},
    {34,  32, 43, 32, 34, 32, 43, 32},
    {46,  43, 58, 43, 46, 43, 58, 43},
    {34,  32, 43, 32, 34, 32, 43, 32},
    {36,  34, 46, 34, 36, 34, 46, 34},
    {34,  32, 43, 32, 34, 32, 43, 32},
    {46,  43, 58, 43, 46, 43, 58, 43},
    {34,  32, 43, 32, 34, 32, 43, 32}
  }
};


int matrix4x4_check[6] = {0, 0, 0, 0, 0, 0};
int matrix8x8_check[6] = {0, 0, 0, 0, 0, 0};

static const char MatrixType4x4[6][20] =
{
  "INTRA4X4_LUMA",
  "INTRA4X4_CHROMAU",
  "INTRA4X4_CHROMAV",
  "INTER4X4_LUMA",
  "INTER4X4_CHROMAU",
  "INTER4X4_CHROMAV"
};

static const char MatrixType8x8[6][20] =
{
  "INTRA8X8_LUMA",
  "INTER8X8_LUMA",
  "INTRA8X8_CHROMAU",  // only for 4:4:4
  "INTER8X8_CHROMAU",  // only for 4:4:4
  "INTRA8X8_CHROMAV",  // only for 4:4:4
  "INTER8X8_CHROMAV"   // only for 4:4:4
};

int *****LevelScale4x4Comp;
int *****LevelScale8x8Comp;

int *****InvLevelScale4x4Comp;
int *****InvLevelScale8x8Comp;

short ScalingList4x4input[6][16];
short ScalingList8x8input[6][64];
short ScalingList4x4[6][16];
short ScalingList8x8[6][64];

short UseDefaultScalingMatrix4x4Flag[6];
short UseDefaultScalingMatrix8x8Flag[6];

int *qp_per_matrix;
int *qp_rem_matrix;

static const short Quant_intra_default[16] =
{
 6,13,20,28,
13,20,28,32,
20,28,32,37,
28,32,37,42
};

static const short Quant_inter_default[16] =
{
10,14,20,24,
14,20,24,27,
20,24,27,30,
24,27,30,34
};

static const short Quant8_intra_default[64] =
{
 6,10,13,16,18,23,25,27,
10,11,16,18,23,25,27,29,
13,16,18,23,25,27,29,31,
16,18,23,25,27,29,31,33,
18,23,25,27,29,31,33,36,
23,25,27,29,31,33,36,38,
25,27,29,31,33,36,38,40,
27,29,31,33,36,38,40,42
};

static const short Quant8_inter_default[64] =
{
 9,13,15,17,19,21,22,24,
13,13,17,19,21,22,24,25,
15,17,19,21,22,24,25,27,
17,19,21,22,24,25,27,28,
19,21,22,24,25,27,28,30,
21,22,24,25,27,28,30,32,
22,24,25,27,28,30,32,33,
24,25,27,28,30,32,33,35
};


/*!
 ***********************************************************************
 * \brief
 *    Check the parameter name.
 * \param s
 *    parameter name string
 * \param type
 *    4x4 or 8x8 matrix type
 * \return
 *    the index number if the string is a valid parameter name,         \n
 *    -1 for error
 ***********************************************************************
 */
int CheckParameterName (char *s, int *type)
{
  int i = 0;

  *type = 0;
  while ((MatrixType4x4[i] != NULL) && (i<6))
  {
    if (0==strcmp (MatrixType4x4[i], s))
      return i;
    else
      i++;
  }

  i = 0;
  *type = 1;
  while ((MatrixType8x8[i] != NULL) && (i<6))
  {
    if (0==strcmp (MatrixType8x8[i], s))
      return i;
    else
      i++;
  }

  return -1;
}

/*!
 ***********************************************************************
 * \brief
 *    Parse the Q matrix values read from cfg file.
 * \param buf
 *    buffer to be parsed
 * \param bufsize
 *    buffer size of buffer
 ***********************************************************************
 */
void ParseMatrix (char *buf, int bufsize)
{
  char *items[MAX_ITEMS_TO_PARSE];
  int MapIdx;
  int item = 0;
  int InString = 0, InItem = 0;
  char *p = buf;
  char *bufend = &buf[bufsize];
  int IntContent;
  int i, j, range, type, cnt;
  short *ScalingList;

  while (p < bufend)
  {
    switch (*p)
    {
      case 13:
        p++;
        break;
      case '#':                 // Found comment
        *p = '\0';              // Replace '#' with '\0' in case of comment immediately following integer or string
        while (*p != '\n' && p < bufend)  // Skip till EOL or EOF, whichever comes first
          p++;
        InString = 0;
        InItem = 0;
        break;
      case '\n':
        InItem = 0;
        InString = 0;
        *p++='\0';
        break;
      case ' ':
      case '\t':              // Skip whitespace, leave state unchanged
        if (InString)
          p++;
        else
        {                     // Terminate non-strings once whitespace is found
          *p++ = '\0';
          InItem = 0;
        }
        break;

      case '"':               // Begin/End of String
        *p++ = '\0';
        if (!InString)
        {
          items[item++] = p;
          InItem = ~InItem;
        }
        else
          InItem = 0;
        InString = ~InString; // Toggle
        break;

      case ',':
        p++;
        InItem = 0;
        break;

      default:
        if (!InItem)
        {
          items[item++] = p;
          InItem = ~InItem;
        }
        p++;
    }
  }

  item--;

  for (i=0; i<item; i+=cnt)
  {
    cnt=0;
    if (0 > (MapIdx = CheckParameterName (items[i+cnt], &type)))
    {
      snprintf (errortext, ET_SIZE, " Parsing error in config file: Parameter Name '%s' not recognized.", items[i+cnt]);
      error (errortext, 300);
    }
    cnt++;
    if (strcmp ("=", items[i+cnt]))
    {
      snprintf (errortext, ET_SIZE, " Parsing error in config file: '=' expected as the second token in each item.");
      error (errortext, 300);
    }
    cnt++;

    if (!type) //4x4 Matrix
    {
      range = 16;
      ScalingList = ScalingList4x4input[MapIdx];
      matrix4x4_check[MapIdx] = 1; //to indicate matrix found in cfg file
    }
    else //8x8 matrix
    {
      range = 64;
      ScalingList = ScalingList8x8input[MapIdx];
      matrix8x8_check[MapIdx] = 1; //to indicate matrix found in cfg file
    }

    for(j=0; j<range; j++)
    {
      if (1 != sscanf (items[i+cnt+j], "%d", &IntContent))
      {
        snprintf (errortext, ET_SIZE, " Parsing error: Expected numerical value for Parameter of %s, found '%s'.", items[i], items[i+cnt+j]);
        error (errortext, 300);
      }

      ScalingList[j] = (short)IntContent; //save value in matrix
    }
    cnt+=j;
    printf (".");
  }
}

/*!
 ***********************************************************************
 * \brief
 *    Check Q Matrix values. If invalid values found in matrix,
 *    whole matrix will be patch with default value 16.
 ***********************************************************************
 */
void PatchMatrix(void)
{
  short *ScalingList;
  int i, cnt, fail;

  for(i=0; i<6; i++)
  {
    if(params->ScalingListPresentFlag[i])
    {
      ScalingList=ScalingList4x4input[i];
      if(matrix4x4_check[i])
      {
        fail=0;
        for(cnt=0; cnt<16; cnt++)
        {
          if(ScalingList[cnt]<0 || ScalingList[cnt]>255) // ScalingList[0]=0 to indicate use default matrix
          {
            fail=1;
            break;
          }
        }

        if(fail) //value of matrix exceed range
        {
          printf("\n%s value exceed range. (Value must be 1 to 255)\n", MatrixType4x4[i]);
          printf("Setting default values for this matrix.");
          if(i>2)
            memcpy(ScalingList, Quant_inter_default, sizeof(short)*16);
          else
            memcpy(ScalingList, Quant_intra_default, sizeof(short)*16);
        }
      }
      else //matrix not found, pad with default value
      {
        printf("\n%s matrix definition not found. Setting default values.", MatrixType4x4[i]);
        if(i>2)
          memcpy(ScalingList, Quant_inter_default, sizeof(short)*16);
        else
          memcpy(ScalingList, Quant_intra_default, sizeof(short)*16);
      }
    }

    if(params->ScalingListPresentFlag[i+6])
    {
      ScalingList=ScalingList8x8input[i];
      if(matrix8x8_check[i])
      {
        fail=0;
        for(cnt=0; cnt<64; cnt++)
        {
          if(ScalingList[cnt]<0 || ScalingList[cnt]>255) // ScalingList[0]=0 to indicate use default matrix
          {
            fail=1;
            break;
          }
        }

        if(fail) //value of matrix exceed range
        {
          printf("\n%s value exceed range. (Value must be 1 to 255)\n", MatrixType8x8[i]);
          printf("Setting default values for this matrix.");
          if(i==1 || i==3 || i==5)
            memcpy(ScalingList, Quant8_inter_default, sizeof(short)*64);
          else
            memcpy(ScalingList, Quant8_intra_default, sizeof(short)*64);
        }
      }
      else //matrix not found, pad with default value
      {
        printf("\n%s matrix definition not found. Setting default values.", MatrixType8x8[i]);
        if(i==1 || i==3 || i==5)
          memcpy(ScalingList, Quant8_inter_default, sizeof(short)*64);
        else
          memcpy(ScalingList, Quant8_intra_default, sizeof(short)*64);
      }
    }
  }
}

/*!
 ***********************************************************************
 * \brief
 *    Allocate Q matrix arrays
 ***********************************************************************
 */
void allocate_QMatrix (void)
{
  int bitdepth_qp_scale = 6*(params->output.bit_depth[0] - 8);
  int i;

  if ((qp_per_matrix = (int*)malloc((MAX_QP + 1 +  bitdepth_qp_scale)*sizeof(int))) == NULL)
    no_mem_exit("init_global_buffers: qp_per_matrix");
  if ((qp_rem_matrix = (int*)malloc((MAX_QP + 1 +  bitdepth_qp_scale)*sizeof(int))) == NULL)
    no_mem_exit("init_global_buffers: qp_per_matrix");

  for (i = 0; i < MAX_QP + bitdepth_qp_scale + 1; i++)
  {
    qp_per_matrix[i] = i / 6;
    qp_rem_matrix[i] = i % 6;
  }

  get_mem5Dint(&LevelScale4x4Comp,    3, 2, 6, 4, 4);
  get_mem5Dint(&LevelScale8x8Comp,    3, 2, 6, 8, 8);

  get_mem5Dint(&InvLevelScale4x4Comp, 3, 2, 6, 4, 4);
  get_mem5Dint(&InvLevelScale8x8Comp, 3, 2, 6, 8, 8);
}

/*!
 ***********************************************************************
 * \brief
 *    Free Q matrix arrays
 ***********************************************************************
 */
void free_QMatrix ()
{
  free(qp_rem_matrix);
  free(qp_per_matrix);

  free_mem5Dint(LevelScale4x4Comp);
  free_mem5Dint(LevelScale8x8Comp);

  free_mem5Dint(InvLevelScale4x4Comp);
  free_mem5Dint(InvLevelScale8x8Comp);
}


/*!
 ***********************************************************************
 * \brief
 *    Initialise Q matrix values.
 ***********************************************************************
 */
void Init_QMatrix (void)
{
  char *content;


  allocate_QMatrix ();

  if(params->ScalingMatrixPresentFlag)
  {
    printf ("Parsing QMatrix file %s ", params->QmatrixFile);
    content = GetConfigFileContent(params->QmatrixFile, 0);
    if(content!='\0')
      ParseMatrix(content, strlen (content));
    else
      printf("\nError: %s\nProceeding with default values for all matrices.", errortext);

    PatchMatrix();
    printf("\n");

    memset(UseDefaultScalingMatrix4x4Flag, 0, 6 * sizeof(short));
    memset(UseDefaultScalingMatrix8x8Flag, 0, 6 * sizeof(short));

    free(content);
  }
}

/*!
 ************************************************************************
 * \brief
 *    For calculating the quantisation values at frame level
 *
 * \par Input:
 *    none
 *
 * \par Output:
 *    none
 ************************************************************************
 */
void CalculateQuantParam(void)
{
  int i, j, k, temp;
  int present[6];
  int no_q_matrix=FALSE;

  if(!active_sps->seq_scaling_matrix_present_flag && !active_pps->pic_scaling_matrix_present_flag) //set to no q-matrix
    no_q_matrix=TRUE;
  else
  {
    memset(present, 0, 6 * sizeof(int));

    if(active_sps->seq_scaling_matrix_present_flag)
      for(i=0; i<6; i++)
        present[i] = active_sps->seq_scaling_list_present_flag[i];

    if(active_pps->pic_scaling_matrix_present_flag)
      for(i=0; i<6; i++)
      {
        if((i==0) || (i==3))
          present[i] |= active_pps->pic_scaling_list_present_flag[i];
        else
          present[i] = active_pps->pic_scaling_list_present_flag[i];
      }
  }

  if(no_q_matrix==TRUE)
  {
    for(k=0; k<6; k++)
      for(j=0; j<4; j++)
        for(i=0; i<4; i++)
        {
          LevelScale4x4Comp[0][1][k][j][i]    = quant_coef[k][j][i];
          InvLevelScale4x4Comp[0][1][k][j][i] = dequant_coef[k][j][i]<<4;

          LevelScale4x4Comp[1][1][k][j][i]    = quant_coef[k][j][i];
          InvLevelScale4x4Comp[1][1][k][j][i] = dequant_coef[k][j][i]<<4;

          LevelScale4x4Comp[2][1][k][j][i]    = quant_coef[k][j][i];
          InvLevelScale4x4Comp[2][1][k][j][i] = dequant_coef[k][j][i]<<4;

          // Inter
          LevelScale4x4Comp[0][0][k][j][i]    = quant_coef[k][j][i];
          InvLevelScale4x4Comp[0][0][k][j][i] = dequant_coef[k][j][i]<<4;

          LevelScale4x4Comp[1][0][k][j][i]    = quant_coef[k][j][i];
          InvLevelScale4x4Comp[1][0][k][j][i] = dequant_coef[k][j][i]<<4;

          LevelScale4x4Comp[2][0][k][j][i]    = quant_coef[k][j][i];
          InvLevelScale4x4Comp[2][0][k][j][i] = dequant_coef[k][j][i]<<4;
        }
  }
  else
  {
    for(k=0; k<6; k++)
    {
      for(j=0; j<4; j++)
      {
        for(i=0; i<4; i++)
        {
          temp = (j<<2)+i;
          if((!present[0]) || UseDefaultScalingMatrix4x4Flag[0])
          {
            LevelScale4x4Comp[0][1][k][j][i]         = (quant_coef[k][j][i]<<4)/Quant_intra_default[temp];
            InvLevelScale4x4Comp[0][1][k][j][i]      = dequant_coef[k][j][i]*Quant_intra_default[temp];
          }
          else
          {
            LevelScale4x4Comp[0][1][k][j][i]         = (quant_coef[k][j][i]<<4)/ScalingList4x4[0][temp];
            InvLevelScale4x4Comp[0][1][k][j][i]      = dequant_coef[k][j][i]*ScalingList4x4[0][temp];
          }

          if(!present[1])
          {
            LevelScale4x4Comp[1][1][k][j][i]    = LevelScale4x4Comp[0][1][k][j][i];
            InvLevelScale4x4Comp[1][1][k][j][i] = InvLevelScale4x4Comp[0][1][k][j][i];
          }
          else
          {
            LevelScale4x4Comp[1][1][k][j][i]    = (quant_coef[k][j][i]<<4)/(UseDefaultScalingMatrix4x4Flag[1] ? Quant_intra_default[temp]:ScalingList4x4[1][temp]);
            InvLevelScale4x4Comp[1][1][k][j][i] = dequant_coef[k][j][i]*(UseDefaultScalingMatrix4x4Flag[1] ? Quant_intra_default[temp]:ScalingList4x4[1][temp]);
          }

          if(!present[2])
          {
            LevelScale4x4Comp[2][1][k][j][i]    = LevelScale4x4Comp[1][1][k][j][i];
            InvLevelScale4x4Comp[2][1][k][j][i] = InvLevelScale4x4Comp[1][1][k][j][i];
          }
          else
          {
            LevelScale4x4Comp[2][1][k][j][i]    = (quant_coef[k][j][i]<<4)/(UseDefaultScalingMatrix4x4Flag[2] ? Quant_intra_default[temp]:ScalingList4x4[2][temp]);
            InvLevelScale4x4Comp[2][1][k][j][i] = dequant_coef[k][j][i]*(UseDefaultScalingMatrix4x4Flag[2] ? Quant_intra_default[temp]:ScalingList4x4[2][temp]);
          }

          if((!present[3]) || UseDefaultScalingMatrix4x4Flag[3])
          {
            LevelScale4x4Comp[0][0][k][j][i]         = (quant_coef[k][j][i]<<4)/Quant_inter_default[temp];
            InvLevelScale4x4Comp[0][0][k][j][i]      = dequant_coef[k][j][i]*Quant_inter_default[temp];
          }
          else
          {
            LevelScale4x4Comp[0][0][k][j][i]         = (quant_coef[k][j][i]<<4)/ScalingList4x4[3][temp];
            InvLevelScale4x4Comp[0][0][k][j][i]      = dequant_coef[k][j][i]*ScalingList4x4[3][temp];
          }

          if(!present[4])
          {
            LevelScale4x4Comp[1][0][k][j][i]    = LevelScale4x4Comp[0][0][k][j][i];
            InvLevelScale4x4Comp[1][0][k][j][i] = InvLevelScale4x4Comp[0][0][k][j][i];
          }
          else
          {
            LevelScale4x4Comp[1][0][k][j][i]    = (quant_coef[k][j][i]<<4)/(UseDefaultScalingMatrix4x4Flag[4] ? Quant_inter_default[temp]:ScalingList4x4[4][temp]);
            InvLevelScale4x4Comp[1][0][k][j][i] = dequant_coef[k][j][i]*(UseDefaultScalingMatrix4x4Flag[4] ? Quant_inter_default[temp]:ScalingList4x4[4][temp]);
          }

          if(!present[5])
          {
            LevelScale4x4Comp[2][0][k][j][i]    = LevelScale4x4Comp[1][0][k][j][i];
            InvLevelScale4x4Comp[2][0][k][j][i] = InvLevelScale4x4Comp[1][0][k][j][i];
          }
          else
          {
            LevelScale4x4Comp[2][0][k][j][i]    = (quant_coef[k][j][i]<<4)/(UseDefaultScalingMatrix4x4Flag[5] ? Quant_inter_default[temp]:ScalingList4x4[5][temp]);
            InvLevelScale4x4Comp[2][0][k][j][i] = dequant_coef[k][j][i]*(UseDefaultScalingMatrix4x4Flag[5] ? Quant_inter_default[temp]:ScalingList4x4[5][temp]);
          }
        }
      }
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Calculate the quantisation and inverse quantisation parameters
 *
 ************************************************************************
 */
void CalculateQuant8Param()
{
  int i, j, k, temp;
  int n_ScalingList8x8;
  int present[6];
  int no_q_matrix=FALSE;

  // maximum number of valid 8x8 scaling lists
  n_ScalingList8x8 = ( active_sps->chroma_format_idc != 3 ) ? 2 : 6;

  if(!active_sps->seq_scaling_matrix_present_flag && !active_pps->pic_scaling_matrix_present_flag) //set to default matrix
    no_q_matrix=TRUE;
  else
  {
    memset(present, 0, sizeof(int)*n_ScalingList8x8);

    if(active_sps->seq_scaling_matrix_present_flag)
      for(i=0; i<n_ScalingList8x8; i++)
        present[i] = active_sps->seq_scaling_list_present_flag[i+6];

      if(active_pps->pic_scaling_matrix_present_flag)
        for(i=0; i<n_ScalingList8x8; i++)
        {
          if( i==0 || i==1 )
            present[i] |= active_pps->pic_scaling_list_present_flag[i+6];
          else
            present[i] = active_pps->pic_scaling_list_present_flag[i+6];
        }
  }

  if(no_q_matrix==TRUE)
  {
    for(k=0; k<6; k++)
      for(j=0; j<8; j++)
        for(i=0; i<8; i++)
        {
          // intra Y
          LevelScale8x8Comp[0][1][k][j][i]      = quant_coef8[k][j][i];
          InvLevelScale8x8Comp[0][1][k][j][i]   = dequant_coef8[k][j][i]<<4;

          // inter Y
          LevelScale8x8Comp[0][0][k][j][i]      = quant_coef8[k][j][i];
          InvLevelScale8x8Comp[0][0][k][j][i]   = dequant_coef8[k][j][i]<<4;

          if( n_ScalingList8x8 > 2 )  // 4:4:4 case only
          {
            // intra U
            LevelScale8x8Comp[1][1][k][j][i]    = quant_coef8[k][j][i];
            InvLevelScale8x8Comp[1][1][k][j][i] = dequant_coef8[k][j][i]<<4;

            // intra V
            LevelScale8x8Comp[2][1][k][j][i]    = quant_coef8[k][j][i];
            InvLevelScale8x8Comp[2][1][k][j][i] = dequant_coef8[k][j][i]<<4;

            // inter U
            LevelScale8x8Comp[1][0][k][j][i]    = quant_coef8[k][j][i];
            InvLevelScale8x8Comp[1][0][k][j][i] = dequant_coef8[k][j][i]<<4;

            // inter V
            LevelScale8x8Comp[2][0][k][j][i]    = quant_coef8[k][j][i];
            InvLevelScale8x8Comp[2][0][k][j][i] = dequant_coef8[k][j][i]<<4;

          }
        }
  }
  else
  {
    for(k=0; k<6; k++)
      for(j=0; j<8; j++)
        for(i=0; i<8; i++)
        {
          temp = (j<<3)+i;
          if((!present[0]) || UseDefaultScalingMatrix8x8Flag[0])
          {
            LevelScale8x8Comp[0][1][k][j][i]    = (quant_coef8[k][j][i]<<4)/Quant8_intra_default[temp];
            InvLevelScale8x8Comp[0][1][k][j][i] = dequant_coef8[k][j][i]*Quant8_intra_default[temp];
          }
          else
          {
            LevelScale8x8Comp[0][1][k][j][i]    = (quant_coef8[k][j][i]<<4)/ScalingList8x8[0][temp];
            InvLevelScale8x8Comp[0][1][k][j][i] = dequant_coef8[k][j][i]*ScalingList8x8[0][temp];
          }

          if((!present[1]) || UseDefaultScalingMatrix8x8Flag[1])
          {
            LevelScale8x8Comp[0][0][k][j][i]    = (quant_coef8[k][j][i]<<4)/Quant8_inter_default[temp];
            InvLevelScale8x8Comp[0][0][k][j][i] = dequant_coef8[k][j][i]*Quant8_inter_default[temp];
          }
          else
          {
            LevelScale8x8Comp[0][0][k][j][i]    = (quant_coef8[k][j][i]<<4)/ScalingList8x8[1][temp];
            InvLevelScale8x8Comp[0][0][k][j][i] = dequant_coef8[k][j][i]*ScalingList8x8[1][temp];
          }

          if( n_ScalingList8x8 > 2 )
          {

            // Intra U
            if(!present[2])
            {
              LevelScale8x8Comp[1][1][k][j][i]    = LevelScale8x8Comp[0][1][k][j][i];
              InvLevelScale8x8Comp[1][1][k][j][i] = InvLevelScale8x8Comp[0][1][k][j][i];
            }
            else
            {
              LevelScale8x8Comp[1][1][k][j][i]    = (quant_coef8[k][j][i]<<4)/(UseDefaultScalingMatrix8x8Flag[2] ? Quant8_intra_default[temp]:ScalingList8x8[2][temp]);
              InvLevelScale8x8Comp[1][1][k][j][i] = dequant_coef8[k][j][i]*(UseDefaultScalingMatrix8x8Flag[2] ? Quant8_intra_default[temp]:ScalingList8x8[2][temp]);
            }

            // Inter U
            if(!present[3])
            {
              LevelScale8x8Comp[1][0][k][j][i]    = LevelScale8x8Comp[0][0][k][j][i];
              InvLevelScale8x8Comp[1][0][k][j][i] = InvLevelScale8x8Comp[0][0][k][j][i];
            }
            else
            {
              LevelScale8x8Comp[1][0][k][j][i]    = (quant_coef8[k][j][i]<<4)/(UseDefaultScalingMatrix8x8Flag[3] ? Quant8_inter_default[temp]:ScalingList8x8[3][temp]);
              InvLevelScale8x8Comp[1][0][k][j][i] = dequant_coef8[k][j][i]*(UseDefaultScalingMatrix8x8Flag[3] ? Quant8_inter_default[temp]:ScalingList8x8[3][temp]);
            }

            // Intra V
            if(!present[4])
            {
              LevelScale8x8Comp[2][1][k][j][i]    = LevelScale8x8Comp[1][1][k][j][i];
              InvLevelScale8x8Comp[2][1][k][j][i] = InvLevelScale8x8Comp[1][1][k][j][i];
            }
            else
            {
              LevelScale8x8Comp[2][1][k][j][i]    = (quant_coef8[k][j][i]<<4)/(UseDefaultScalingMatrix8x8Flag[4] ? Quant8_intra_default[temp]:ScalingList8x8[4][temp]);
              InvLevelScale8x8Comp[2][1][k][j][i] = dequant_coef8[k][j][i]*(UseDefaultScalingMatrix8x8Flag[4] ? Quant8_intra_default[temp]:ScalingList8x8[4][temp]);
            }

            // Inter V
            if(!present[5])
            {
              LevelScale8x8Comp[2][0][k][j][i]    = LevelScale8x8Comp[1][0][k][j][i];
              InvLevelScale8x8Comp[2][0][k][j][i] = InvLevelScale8x8Comp[1][0][k][j][i];
            }
            else
            {
              LevelScale8x8Comp[2][0][k][j][i]    = (quant_coef8[k][j][i]<<4)/(UseDefaultScalingMatrix8x8Flag[5] ? Quant8_inter_default[temp]:ScalingList8x8[5][temp]);
              InvLevelScale8x8Comp[2][0][k][j][i] = dequant_coef8[k][j][i]*(UseDefaultScalingMatrix8x8Flag[5] ? Quant8_inter_default[temp]:ScalingList8x8[5][temp]);
            }

          }
        }
  }
}

