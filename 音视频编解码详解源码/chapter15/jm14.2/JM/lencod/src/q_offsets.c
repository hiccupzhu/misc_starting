
/*!
 *************************************************************************************
 * \file q_offsets.c
 *
 * \brief
 *    read Quantization Offset matrix parameters from input file: q_OffsetMatrix.cfg
 *
 *************************************************************************************
 */

#include "global.h"
#include "memalloc.h"
#include "q_matrix.h"
#include "q_offsets.h"

extern char *GetConfigFileContent (char *Filename, int error_type);

#define MAX_ITEMS_TO_PARSE  2000

int offset4x4_check[6] = { 0, 0, 0, 0, 0, 0 };
int offset8x8_check[6] = { 0, 0, 0, 0, 0, 0 };

static const char OffsetType4x4[15][24] = {
  "INTRA4X4_LUMA_INTRA",
  "INTRA4X4_CHROMAU_INTRA",
  "INTRA4X4_CHROMAV_INTRA",
  "INTRA4X4_LUMA_INTERP",
  "INTRA4X4_CHROMAU_INTERP",
  "INTRA4X4_CHROMAV_INTERP",
  "INTRA4X4_LUMA_INTERB",
  "INTRA4X4_CHROMAU_INTERB",
  "INTRA4X4_CHROMAV_INTERB",
  "INTER4X4_LUMA_INTERP",
  "INTER4X4_CHROMAU_INTERP",
  "INTER4X4_CHROMAV_INTERP",
  "INTER4X4_LUMA_INTERB",
  "INTER4X4_CHROMAU_INTERB",
  "INTER4X4_CHROMAV_INTERB"
};

static const char OffsetType8x8[15][24] = {
  "INTRA8X8_LUMA_INTRA",
  "INTRA8X8_LUMA_INTERP",
  "INTRA8X8_LUMA_INTERB",
  "INTER8X8_LUMA_INTERP",
  "INTER8X8_LUMA_INTERB",
  "INTRA8X8_CHROMAU_INTRA",
  "INTRA8X8_CHROMAU_INTERP",
  "INTRA8X8_CHROMAU_INTERB",
  "INTER8X8_CHROMAU_INTERP",
  "INTER8X8_CHROMAU_INTERB",
  "INTRA8X8_CHROMAV_INTRA",
  "INTRA8X8_CHROMAV_INTERP",
  "INTRA8X8_CHROMAV_INTERB",
  "INTER8X8_CHROMAV_INTERP",
  "INTER8X8_CHROMAV_INTERB"
};


int *****LevelOffset4x4Comp;
int *****LevelOffset8x8Comp;

// global pointers for level offset matrices
int ****ptLevelOffset4x4;
int ****ptLevelOffset8x8;

int AdaptRndWeight;
int AdaptRndCrWeight;

short **OffsetList4x4input;
short **OffsetList8x8input;
short ***OffsetList4x4;
short ***OffsetList8x8;

void InitOffsetParam ();

const int OffsetBits = 11;

static const short Offset_intra_default_intra[16] = {
  682, 682, 682, 682,
  682, 682, 682, 682,
  682, 682, 682, 682,
  682, 682, 682, 682
};

static const short Offset_intra_default_chroma[16] = {
  682, 682, 682, 682,
  682, 682, 682, 682,
  682, 682, 682, 682,
  682, 682, 682, 682
};


static const short Offset_intra_default_inter[16] = {
  342, 342, 342, 342,
  342, 342, 342, 342,
  342, 342, 342, 342,
  342, 342, 342, 342,
};

static const short Offset_inter_default[16] = {
  342, 342, 342, 342,
  342, 342, 342, 342,
  342, 342, 342, 342,
  342, 342, 342, 342,
};

static const short Offset8_intra_default_intra[64] = {
  682, 682, 682, 682, 682, 682, 682, 682,
  682, 682, 682, 682, 682, 682, 682, 682,
  682, 682, 682, 682, 682, 682, 682, 682,
  682, 682, 682, 682, 682, 682, 682, 682,
  682, 682, 682, 682, 682, 682, 682, 682,
  682, 682, 682, 682, 682, 682, 682, 682,
  682, 682, 682, 682, 682, 682, 682, 682,
  682, 682, 682, 682, 682, 682, 682, 682
};

static const short Offset8_intra_default_chroma[64] = {
  682, 682, 682, 682, 682, 682, 682, 682,
  682, 682, 682, 682, 682, 682, 682, 682,
  682, 682, 682, 682, 682, 682, 682, 682,
  682, 682, 682, 682, 682, 682, 682, 682,
  682, 682, 682, 682, 682, 682, 682, 682,
  682, 682, 682, 682, 682, 682, 682, 682,
  682, 682, 682, 682, 682, 682, 682, 682,
  682, 682, 682, 682, 682, 682, 682, 682
};

static const short Offset8_intra_default_inter[64] = {
  342, 342, 342, 342, 342, 342, 342, 342,
  342, 342, 342, 342, 342, 342, 342, 342,
  342, 342, 342, 342, 342, 342, 342, 342,
  342, 342, 342, 342, 342, 342, 342, 342,
  342, 342, 342, 342, 342, 342, 342, 342,
  342, 342, 342, 342, 342, 342, 342, 342,
  342, 342, 342, 342, 342, 342, 342, 342,
  342, 342, 342, 342, 342, 342, 342, 342
};

static const short Offset8_inter_default[64] = {
  342, 342, 342, 342, 342, 342, 342, 342,
  342, 342, 342, 342, 342, 342, 342, 342,
  342, 342, 342, 342, 342, 342, 342, 342,
  342, 342, 342, 342, 342, 342, 342, 342,
  342, 342, 342, 342, 342, 342, 342, 342,
  342, 342, 342, 342, 342, 342, 342, 342,
  342, 342, 342, 342, 342, 342, 342, 342,
  342, 342, 342, 342, 342, 342, 342, 342
};

/*!
 ***********************************************************************
 * \brief
 *    Allocate Q matrix arrays
 ***********************************************************************
 */
void allocate_QOffsets ()
{
  int max_bitdepth = imax(params->output.bit_depth[0], params->output.bit_depth[1]);
  int max_qp = (3 + 6*(max_bitdepth));

  get_mem5Dint(&LevelOffset4x4Comp, 3, 2, max_qp + 1, 4, 4);
  get_mem5Dint(&LevelOffset8x8Comp, 3, 2, max_qp + 1, 8, 8);

  if (params->AdaptRoundingFixed)
  {
    get_mem3Dshort(&OffsetList4x4, 1, 25, 16);
    get_mem3Dshort(&OffsetList8x8, 1, 15, 64);    
  }
  else
  {
    get_mem3Dshort(&OffsetList4x4, max_qp + 1, 25, 16);
    get_mem3Dshort(&OffsetList8x8, max_qp + 1, 15, 64);    
  }

  get_mem2Dshort(&OffsetList4x4input, 25, 16);
  get_mem2Dshort(&OffsetList8x8input, 15, 64);
}

/*!
 ***********************************************************************
 * \brief
 *    Free Q matrix arrays
 ***********************************************************************
 */
void free_QOffsets ()
{
  free_mem5Dint(LevelOffset4x4Comp);
  free_mem5Dint(LevelOffset8x8Comp);

  if (params->AdaptRoundingFixed)
  {
    free_mem3Dshort(OffsetList8x8);
    free_mem3Dshort(OffsetList4x4);    
  }
  else
  {
    free_mem3Dshort(OffsetList8x8);
    free_mem3Dshort(OffsetList4x4);    
  }

  free_mem2Dshort(OffsetList8x8input);
  free_mem2Dshort(OffsetList4x4input);
}


/*!
 ***********************************************************************
 * \brief
 *    Check the parameter name.
 * \param s
 *    parameter name string
 * \param type
 *    4x4 or 8x8 offset matrix type
 * \return
 *    the index number if the string is a valid parameter name,         \n
 *    -1 for error
 ***********************************************************************
 */

int CheckOffsetParameterName (char *s, int *type)
{
  int i = 0;

  *type = 0;
  while ((OffsetType4x4[i] != NULL) && (i < 15))
  {
    if (0 == strcmp (OffsetType4x4[i], s))
      return i;
    else
      i++;
  }

  i = 0;
  *type = 1;
  while ((OffsetType8x8[i] != NULL) && (i < 15))
  {
    if (0 == strcmp (OffsetType8x8[i], s))
      return i;
    else
      i++;
  }

  return -1;
}

/*!
 ***********************************************************************
 * \brief
 *    Parse the Q Offset Matrix values read from cfg file.
 * \param buf
 *    buffer to be parsed
 * \param bufsize
 *    buffer size of buffer
 ***********************************************************************
 */
void ParseQOffsetMatrix (char *buf, int bufsize)
{
  char *items[MAX_ITEMS_TO_PARSE];
  int MapIdx;
  int item = 0;
  int InString = 0, InItem = 0;
  char *p = buf;
  char *bufend = &buf[bufsize];
  int IntContent;
  int i, j, range, type, cnt;
  short *OffsetList;

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
      *p++ = '\0';
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

  for (i = 0; i < item; i += cnt)
  {
    cnt = 0;
    if (0 > (MapIdx = CheckOffsetParameterName (items[i + cnt], &type)))
    {
      snprintf (errortext, ET_SIZE,
        " Parsing error in config file: Parameter Name '%s' not recognized.",
        items[i + cnt]);
      error (errortext, 300);
    }
    cnt++;
    if (strcmp ("=", items[i + cnt]))
    {
      snprintf (errortext, ET_SIZE,
        " Parsing error in config file: '=' expected as the second token in each item.");
      error (errortext, 300);
    }
    cnt++;

    if (!type) //4x4 Matrix
    {
      range = 16;
      OffsetList = OffsetList4x4input[MapIdx];
      offset4x4_check[MapIdx] = 1; //to indicate matrix found in cfg file
    }
    else //8x8 matrix
    {
      range = 64;
      OffsetList = OffsetList8x8input[MapIdx];
      offset8x8_check[MapIdx] = 1; //to indicate matrix found in cfg file
    }

    for (j = 0; j < range; j++)
    {
      if (1 != sscanf (items[i + cnt + j], "%d", &IntContent))
      {
        snprintf (errortext, ET_SIZE,
          " Parsing error: Expected numerical value for Parameter of %s, found '%s'.",
          items[i], items[i + cnt + j]);
        error (errortext, 300);
      }

      OffsetList[j] = (short) IntContent; //save value in matrix
    }
    cnt += j;
    printf (".");
  }
}


/*!
 ***********************************************************************
 * \brief
 *    Initialise Q offset matrix values.
 ***********************************************************************
 */
void Init_QOffsetMatrix ()
{
  char *content;

  allocate_QOffsets ();

  if (params->OffsetMatrixPresentFlag)
  {
    printf ("Parsing Quantization Offset Matrix file %s ",
      params->QOffsetMatrixFile);
    content = GetConfigFileContent (params->QOffsetMatrixFile, 0);
    if (content != '\0')
      ParseQOffsetMatrix (content, strlen (content));
    else
    {
      printf
        ("\nError: %s\nProceeding with default values for all matrices.",
        errortext);
      params->OffsetMatrixPresentFlag = 0;
    }

    printf ("\n");

    free (content);
  }
  //! Now set up all offset params. This process could be reused if we wish to re-init offsets
  InitOffsetParam ();
}

/*!
 ************************************************************************
 * \brief
 *    Intit quantization offset params
 *
 * \par Input:
 *    none
 *
 * \par Output:
 *    none
 ************************************************************************
 */

void InitOffsetParam ()
{
  int i, k;
  int max_qp_luma = (4 + 6*(params->output.bit_depth[0]));
  int max_qp_cr   = (4 + 6*(params->output.bit_depth[1]));

  for (i = 0; i < (params->AdaptRoundingFixed ? 1 : imax(max_qp_luma, max_qp_cr)); i++)
  {
    if (params->OffsetMatrixPresentFlag)
    {
      memcpy(&(OffsetList4x4[i][0][0]),&(OffsetList4x4input[0][0]), 400 * sizeof(short)); // 25 * 16
      memcpy(&(OffsetList8x8[i][0][0]),&(OffsetList8x8input[0][0]), 960 * sizeof(short)); // 15 * 64
    }
    else
    {
      // 0 (INTRA4X4_LUMA_INTRA)
      memcpy(&(OffsetList4x4[i][0][0]),&(Offset_intra_default_intra[0]), 16 * sizeof(short));
      for (k = 1; k < 3; k++) // 1,2 (INTRA4X4_CHROMA_INTRA)
        memcpy(&(OffsetList4x4[i][k][0]),&(Offset_intra_default_chroma[0]),  16 * sizeof(short));
      for (k = 3; k < 9; k++) // 3,4,5,6,7,8 (INTRA4X4_LUMA/CHROMA_INTERP/INTERB)
        memcpy(&(OffsetList4x4[i][k][0]),&(Offset_intra_default_inter[0]),  16 * sizeof(short));
      for (k = 9; k < 25; k++) // 9,10,11,12,13,14 (INTER4X4)
        memcpy(&(OffsetList4x4[i][k][0]),&(Offset_inter_default[0]),  16 * sizeof(short));

      // 0 (INTRA8X8_LUMA_INTRA)
      memcpy(&(OffsetList8x8[i][0][0]),&(Offset8_intra_default_intra[0]), 64 * sizeof(short));
      for (k = 1; k < 3; k++)  // 1,2 (INTRA8X8_LUMA_INTERP/INTERB)
        memcpy(&(OffsetList8x8[i][k][0]),&(Offset8_intra_default_inter[0]),  64 * sizeof(short));
      for (k = 3; k < 5; k++)  // 3,4 (INTER8X8_LUMA_INTERP/INTERB)
        memcpy(&(OffsetList8x8[i][k][0]),&(Offset8_inter_default[0]),  64 * sizeof(short));

      // 5 (INTRA8X8_CHROMAU_INTRA)
      memcpy(&(OffsetList8x8[i][5][0]),&(Offset8_intra_default_chroma[0]), 64 * sizeof(short));
      for (k = 6; k < 8; k++)  // 6,7 (INTRA8X8_CHROMAU_INTERP/INTERB)
        memcpy(&(OffsetList8x8[i][k][0]),&(Offset8_intra_default_inter[0]),  64 * sizeof(short));
      for (k = 8; k < 10; k++)  // 8,9 (INTER8X8_CHROMAU_INTERP/INTERB)
        memcpy(&(OffsetList8x8[i][k][0]),&(Offset8_inter_default[0]),  64 * sizeof(short));

      // 10 (INTRA8X8_CHROMAV_INTRA)
      memcpy(&(OffsetList8x8[i][10][0]),&(Offset8_intra_default_chroma[0]), 64 * sizeof(short));
      for (k = 11; k < 13; k++)  // 11,12 (INTRA8X8_CHROMAV_INTERP/INTERB)
        memcpy(&(OffsetList8x8[i][k][0]),&(Offset8_intra_default_inter[0]),  64 * sizeof(short));
      for (k = 13; k < 15; k++)  // 8,9 (INTER8X8_CHROMAV_INTERP/INTERB)
        memcpy(&(OffsetList8x8[i][k][0]),&(Offset8_inter_default[0]),  64 * sizeof(short));
    }
  }  
}


/*!
 ************************************************************************
 * \brief
 *    Calculation of the quantization offset params at the frame level
 *
 * \par Input:
 *    none
 *
 * \par Output:
 *    none
 ************************************************************************
 */
void CalculateOffsetParam ()
{
  int i, j, k, temp;  
  int qp_per, qp;
  short **OffsetList;
  static int **LevelOffsetCmp0Intra, **LevelOffsetCmp1Intra, **LevelOffsetCmp2Intra;
  static int **LevelOffsetCmp0Inter, **LevelOffsetCmp1Inter, **LevelOffsetCmp2Inter;
  int img_type = (img->type == SI_SLICE ? I_SLICE : (img->type == SP_SLICE ? P_SLICE : img->type));

  int max_qp_scale = imax(img->bitdepth_luma_qp_scale, img->bitdepth_chroma_qp_scale);
  int max_qp = 51 + max_qp_scale;

  AdaptRndWeight = params->AdaptRndWFactor[img->nal_reference_idc != 0][img_type];
  AdaptRndCrWeight = params->AdaptRndCrWFactor[img->nal_reference_idc != 0][img_type];

  if (img_type == I_SLICE )
  {
    for (qp = 0; qp < max_qp + 1; qp++)
    {
      k = qp_per_matrix [qp];
      qp_per = Q_BITS + k - OffsetBits;
      OffsetList = OffsetList4x4[params->AdaptRoundingFixed ? 0 : qp];
      LevelOffsetCmp0Intra = LevelOffset4x4Comp[0][1][qp];
      LevelOffsetCmp1Intra = LevelOffset4x4Comp[1][1][qp];
      LevelOffsetCmp2Intra = LevelOffset4x4Comp[2][1][qp];

      temp = 0;
      for (j = 0; j < 4; j++)
      {
        for (i = 0; i < 4; i++, temp++)
        {
          LevelOffsetCmp0Intra[j][i] = (int) OffsetList[0][temp] << qp_per;
          LevelOffsetCmp1Intra[j][i] = (int) OffsetList[1][temp] << qp_per;
          LevelOffsetCmp2Intra[j][i] = (int) OffsetList[2][temp] << qp_per;
        }
      }
    }
  }
  else if (img_type == B_SLICE)
  {
    for (qp = 0; qp < max_qp + 1; qp++)
    {
      k = qp_per_matrix [qp];
      qp_per = Q_BITS + k - OffsetBits;
      OffsetList = OffsetList4x4[params->AdaptRoundingFixed ? 0 : qp];
      LevelOffsetCmp0Intra = LevelOffset4x4Comp[0][1][qp];
      LevelOffsetCmp1Intra = LevelOffset4x4Comp[1][1][qp];
      LevelOffsetCmp2Intra = LevelOffset4x4Comp[2][1][qp];
      LevelOffsetCmp0Inter = LevelOffset4x4Comp[0][0][qp];
      LevelOffsetCmp1Inter = LevelOffset4x4Comp[1][0][qp];
      LevelOffsetCmp2Inter = LevelOffset4x4Comp[2][0][qp];


      for (temp = 0, j = 0; j < 4; j++)
      {
        for (i = 0; i < 4; i++, temp++)
        {          
          // intra
          LevelOffsetCmp0Intra[j][i] = (int) OffsetList[6][temp] << qp_per;
          LevelOffsetCmp1Intra[j][i] = (int) OffsetList[7][temp] << qp_per;
          LevelOffsetCmp2Intra[j][i] = (int) OffsetList[8][temp] << qp_per;
        }
      }

      for (temp = 0, j = 0; j < 4; j++)
      {
        for (i = 0; i < 4; i++, temp++)
        {          
          // inter
          LevelOffsetCmp0Inter[j][i] = (int) OffsetList[12][temp] << qp_per;
          LevelOffsetCmp1Inter[j][i] = (int) OffsetList[13][temp] << qp_per;
          LevelOffsetCmp2Inter[j][i] = (int) OffsetList[14][temp] << qp_per;
        }
      }

    }
  }
  else
  {
    for (qp = 0; qp < max_qp + 1; qp++)
    {
      k = qp_per_matrix [qp];
      qp_per = Q_BITS + k - OffsetBits;
      OffsetList = OffsetList4x4[params->AdaptRoundingFixed ? 0 : qp];
      LevelOffsetCmp0Intra = LevelOffset4x4Comp[0][1][qp];
      LevelOffsetCmp1Intra = LevelOffset4x4Comp[1][1][qp];
      LevelOffsetCmp2Intra = LevelOffset4x4Comp[2][1][qp];
      LevelOffsetCmp0Inter = LevelOffset4x4Comp[0][0][qp];
      LevelOffsetCmp1Inter = LevelOffset4x4Comp[1][0][qp];
      LevelOffsetCmp2Inter = LevelOffset4x4Comp[2][0][qp];

      temp = 0;
      for (j = 0; j < 4; j++)
      {
        for (i = 0; i < 4; i++, temp++)
        {
          // intra
          LevelOffsetCmp0Intra[j][i] = (int) OffsetList[3][temp] << qp_per;
          LevelOffsetCmp1Intra[j][i] = (int) OffsetList[4][temp] << qp_per;
          LevelOffsetCmp2Intra[j][i] = (int) OffsetList[5][temp] << qp_per;
          // inter
          LevelOffsetCmp0Inter[j][i] = (int) OffsetList[9 ][temp] << qp_per;
          LevelOffsetCmp1Inter[j][i] = (int) OffsetList[10][temp] << qp_per;
          LevelOffsetCmp2Inter[j][i] = (int) OffsetList[11][temp] << qp_per;
        }
      }      
    }
  }

  // setting for 4x4 luma quantization offset
  if( IS_INDEPENDENT(params) )
  {
    if( img->colour_plane_id == 0 )
    {
      ptLevelOffset4x4 = LevelOffset4x4Comp[0];
    }
    else if( img->colour_plane_id == 1 )
    {
      ptLevelOffset4x4   = LevelOffset4x4Comp[1];
    }
    else if( img->colour_plane_id == 2 )
    {
      ptLevelOffset4x4   = LevelOffset4x4Comp[2];
    }
  }
  else
  {
    ptLevelOffset4x4 = LevelOffset4x4Comp[0];
  }
}

/*!
 ************************************************************************
 * \brief
 *    Calculate the quantisation offset parameters
 *
 ************************************************************************
*/
void CalculateOffset8Param ()
{
  int i, j, k, temp;
  int q_bits, qp;

  int max_qp_scale = imax(img->bitdepth_luma_qp_scale, img->bitdepth_chroma_qp_scale);
  int max_qp = 51 + max_qp_scale;

  if (img->type == I_SLICE || img->type == SI_SLICE )
  {
    for (qp = 0; qp < max_qp + 1; qp++)
    {
      q_bits = Q_BITS_8 + qp_per_matrix[qp] - OffsetBits;
      k = params->AdaptRoundingFixed ? 0 : qp;
      for (j = 0; j < 8; j++)
      {
        temp = (j << 3);
        for (i = 0; i < 8; i++)
        {          
          // INTRA8X8
          LevelOffset8x8Comp[0][1][qp][j][i] = (int) OffsetList8x8[k][0][temp] << q_bits;

          // INTRA8X8 CHROMAU
          LevelOffset8x8Comp[1][1][qp][j][i] = (int) OffsetList8x8[k][5][temp] << q_bits;

          // INTRA8X8 CHROMAV
          LevelOffset8x8Comp[2][1][qp][j][i] = (int) OffsetList8x8[k][10][temp++] << q_bits;
        }
      }
    }
  }
  else if ((img->type == P_SLICE) || (img->type == SP_SLICE))
  {
    for (qp = 0; qp < max_qp + 1; qp++)
    {
      q_bits = Q_BITS_8 + qp_per_matrix[qp] - OffsetBits;
      k = params->AdaptRoundingFixed ? 0 : qp;
      for (j = 0; j < 8; j++)
      {
        temp = (j << 3);
        for (i = 0; i < 8; i++)
        {
          // INTRA8X8
          LevelOffset8x8Comp[0][1][qp][j][i] = (int) OffsetList8x8[k][1][temp] << q_bits;

          // INTER8X8
          LevelOffset8x8Comp[0][0][qp][j][i] = (int) OffsetList8x8[k][3][temp] << q_bits;

          // INTRA8X8 CHROMAU
          LevelOffset8x8Comp[1][1][qp][j][i] = (int) OffsetList8x8[k][6][temp] << q_bits;

          // INTER8X8 CHROMAU
          LevelOffset8x8Comp[1][0][qp][j][i] = (int) OffsetList8x8[k][8][temp] << q_bits;

          // INTRA8X8 CHROMAV
          LevelOffset8x8Comp[2][1][qp][j][i] = (int) OffsetList8x8[k][11][temp] << q_bits;

          // INTER8X8 CHROMAV
          LevelOffset8x8Comp[2][0][qp][j][i] = (int) OffsetList8x8[k][13][temp++] << q_bits;
        }
      }
    }
  }
  else
  {
    for (qp = 0; qp < max_qp + 1; qp++)
    {
      q_bits = Q_BITS_8 + qp_per_matrix[qp] - OffsetBits;
      k = params->AdaptRoundingFixed ? 0 : qp;
      for (j = 0; j < 8; j++)
      {
        temp = (j << 3);
        for (i = 0; i < 8; i++)
        {
          // INTRA8X8
          LevelOffset8x8Comp[0][1][qp][j][i] = (int) OffsetList8x8[k][2][temp] << q_bits;
          // INTER8X8
          LevelOffset8x8Comp[0][0][qp][j][i] = (int) OffsetList8x8[k][4][temp] << q_bits;

          // INTRA8X8 CHROMAU
          LevelOffset8x8Comp[1][1][qp][j][i] = (int) OffsetList8x8[k][7][temp] << q_bits;

          // INTER8X8 CHROMAU
          LevelOffset8x8Comp[1][0][qp][j][i] = (int) OffsetList8x8[k][9][temp] << q_bits;

          // INTRA8X8 CHROMAV
          LevelOffset8x8Comp[2][1][qp][j][i] = (int) OffsetList8x8[k][12][temp] << q_bits;

          // INTER8X8 CHROMAV
          LevelOffset8x8Comp[2][0][qp][j][i] = (int) OffsetList8x8[k][14][temp++] << q_bits;
        }
      }
    }
  }

  // setting for 8x8 luma quantization offset
  if( IS_INDEPENDENT(params) )
  {
    if( img->colour_plane_id == 0 )
    {
      ptLevelOffset8x8 = LevelOffset8x8Comp[0];
    }
    else if( img->colour_plane_id == 1 )
    {
      ptLevelOffset8x8 = LevelOffset8x8Comp[1];
    }
    else if( img->colour_plane_id == 2 )
    {
      ptLevelOffset8x8 = LevelOffset8x8Comp[2];
    }
  }
  else
  {
    ptLevelOffset8x8 = LevelOffset8x8Comp[0];
  }
}

void SelectColorType (int colour_id)
{
  switch (colour_id)
  {
  case 0:
  default:
    ptLevelOffset8x8   = LevelOffset8x8Comp[0];
    break;
  case 1:
    ptLevelOffset8x8   = LevelOffset8x8Comp[1];
    break;
  case 2:
    ptLevelOffset8x8   = LevelOffset8x8Comp[2];
    break;
  }
}
