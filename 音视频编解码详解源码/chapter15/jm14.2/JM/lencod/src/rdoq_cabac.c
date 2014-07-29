/*!
 *************************************************************************************
 * \file rdoq_cabac.c
 *
 * \brief
 *    Rate Distortion Optimized Quantization based on VCEG-AH21 related to CABAC
 *************************************************************************************
 */

#include "contributors.h"

#include <math.h>
#include <float.h>

#include "global.h"
#include "image.h"
#include "fmo.h"
#include "macroblock.h"
#include "mb_access.h"
#include "ratectl.h"
#include "rdoq.h"

const int entropyBits[128]= 
{
     895,    943,    994,   1048,   1105,   1165,   1228,   1294, 
    1364,   1439,   1517,   1599,   1686,   1778,   1875,   1978, 
    2086,   2200,   2321,   2448,   2583,   2725,   2876,   3034, 
    3202,   3380,   3568,   3767,   3977,   4199,   4435,   4684, 
    4948,   5228,   5525,   5840,   6173,   6527,   6903,   7303, 
    7727,   8178,   8658,   9169,   9714,  10294,  10914,  11575, 
   12282,  13038,  13849,  14717,  15650,  16653,  17734,  18899, 
   20159,  21523,  23005,  24617,  26378,  28306,  30426,  32768, 
   32768,  35232,  37696,  40159,  42623,  45087,  47551,  50015, 
   52479,  54942,  57406,  59870,  62334,  64798,  67262,  69725, 
   72189,  74653,  77117,  79581,  82044,  84508,  86972,  89436, 
   91900,  94363,  96827,  99291, 101755, 104219, 106683, 109146, 
  111610, 114074, 116538, 119002, 121465, 123929, 126393, 128857, 
  131321, 133785, 136248, 138712, 141176, 143640, 146104, 148568, 
  151031, 153495, 155959, 158423, 160887, 163351, 165814, 168278, 
  170742, 173207, 175669, 178134, 180598, 183061, 185525, 187989
};

extern const int maxpos       [];
extern const int type2ctx_bcbp[];
extern const int type2ctx_map [];
extern const int type2ctx_last[]; 
extern const int type2ctx_one []; // 7
extern const int type2ctx_abs []; // 7
extern const int max_c2       []; // 9

extern const int  pos2ctx_map8x8 [];
extern const int  pos2ctx_map8x4 [];
extern const int  pos2ctx_map4x4 [];
extern const int  pos2ctx_map2x4c[];
extern const int  pos2ctx_map4x4c[];
extern const int* pos2ctx_map    [];

extern const int  pos2ctx_last8x8 [];
extern const int  pos2ctx_last8x4 [];
extern const int  pos2ctx_last4x4 [];
extern const int  pos2ctx_last2x4c[];
extern const int  pos2ctx_last4x4c[];
extern const int* pos2ctx_last    [];

extern const int estErr4x4[6][4][4];
extern const int estErr8x8[6][8][8];


int precalcUnaryLevelTab[128][MAX_PREC_COEFF];
estBitsCabacStruct estBitsCabac[NUM_BLOCK_TYPES];

void precalculate_unary_exp_golomb_level()
{
  int state, ctx_state0, ctx_state1, estBits0, estBits1, symbol;

  for (state=0; state<=63; state++)
  {
    // symbol 0 is MPS
    ctx_state0=64+state;
    estBits0=entropyBits[127-ctx_state0];
    ctx_state1=63-state;
    estBits1=entropyBits[127-ctx_state1];

    for (symbol=0; symbol<MAX_PREC_COEFF; symbol++)
    {
      precalcUnaryLevelTab[ctx_state0][symbol]=est_unary_exp_golomb_level_bits(symbol, estBits0, estBits1);

      // symbol 0 is LPS
      precalcUnaryLevelTab[ctx_state1][symbol]=est_unary_exp_golomb_level_bits(symbol, estBits1, estBits0);
    }
  }
}

int est_unary_exp_golomb_level_bits(unsigned int symbol, int bits0, int bits1)
{
  unsigned int l,k;
  unsigned int exp_start = 13; // 15-2 : 0,1 level decision always sent
  int estBits;

  if (symbol==0)
  {
    return (bits0);
  }
  else
  {
    estBits=bits1;
    l=symbol;
    k=1;
    while (((--l)>0) && (++k <= exp_start))
    {
      estBits += bits1;
    }
    if (symbol < exp_start)
    {
      estBits += bits0;
    }
    else 
    {
      estBits += est_exp_golomb_encode_eq_prob(symbol-exp_start);
    }
  }
  return(estBits);
}

/*!
****************************************************************************
* \brief
*    estimate exp golomb bit cost 
****************************************************************************
*/
int est_exp_golomb_encode_eq_prob(unsigned int symbol)
{
  int k = 0, estBits = 0;

  while(1)
  {
    if (symbol >= (unsigned int)(1<<k))   
    {
      estBits++;
      symbol -= (1<<k);
      k++;
    }
    else                  
    {
      estBits += (k + 1);  
      break;
    }
  }
  return(estBits);
}

/*!
****************************************************************************
* \brief
*   estimate bit cost for CBP, significant map and significant coefficients
****************************************************************************
*/
void estRunLevel_CABAC (Macroblock *currMB, int context) // marta - writes CABAC run/level 
{
  DataPartition*  dataPart = &(img->currentSlice->partArr[0]); // assumed that no DP is used (table assignSE2partition_NoDP)
  EncodingEnvironmentPtr eep_dp = &(dataPart->ee_cabac); 

  est_CBP_block_bit  (currMB, eep_dp, context);      
  //===== encode significance map =====
  est_significance_map         (currMB, eep_dp, context);      
  //===== encode significant coefficients =====
  est_significant_coefficients (currMB, eep_dp, context);
}

/*!
****************************************************************************
* \brief
*    estimate bit cost for each CBP bit
****************************************************************************
*/
void est_CBP_block_bit (Macroblock* currMB, EncodingEnvironmentPtr eep_dp, int type)
{
  estBitsCabacStruct *cabacEstBits = &estBitsCabac[type];
  int ctx;

  for (ctx=0; ctx<=3; ctx++)
  {
    cabacEstBits->blockCbpBits[ctx][0]=biari_no_bits(eep_dp, 0, img->currentSlice->tex_ctx->bcbp_contexts[type2ctx_bcbp[type]]+ctx);

    cabacEstBits->blockCbpBits[ctx][1]=biari_no_bits(eep_dp, 1, img->currentSlice->tex_ctx->bcbp_contexts[type2ctx_bcbp[type]]+ctx);
  }
}

/*!
****************************************************************************
* \brief
*    estimate CABAC bit cost for significant coefficient map
****************************************************************************
*/
void est_significance_map(Macroblock* currMB, EncodingEnvironmentPtr eep_dp, int type)
{
  int   k;
  int   k1  = maxpos[type]-1;
#if ENABLE_FIELD_CTX
  int   fld = ( img->structure!=FRAME || currMB->mb_field );
#else
  int   fld = 0;
#endif
  BiContextTypePtr  map_ctx   = img->currentSlice->tex_ctx->map_contexts [fld][type2ctx_map [type]];
  BiContextTypePtr  last_ctx  = img->currentSlice->tex_ctx->last_contexts[fld][type2ctx_last[type]];
  estBitsCabacStruct *cabacEstBits = &estBitsCabac[type];


  for (k = 0; k < k1; k++) // if last coeff is reached, it has to be significant
  {
    cabacEstBits->significantBits[pos2ctx_map[type][k]][0]=biari_no_bits  (eep_dp, 0,  map_ctx+pos2ctx_map     [type][k]);

    cabacEstBits->significantBits[pos2ctx_map[type][k]][1]=biari_no_bits  (eep_dp, 1,  map_ctx+pos2ctx_map     [type][k]);

    cabacEstBits->lastBits[pos2ctx_last[type][k]][0]=biari_no_bits(eep_dp, 0, last_ctx+pos2ctx_last[type][k]);

    cabacEstBits->lastBits[pos2ctx_last[type][k]][1]=biari_no_bits(eep_dp, 1, last_ctx+pos2ctx_last[type][k]);
  }
  // if last coeff is reached, it has to be significant
  cabacEstBits->significantBits[pos2ctx_map[type][k1]][0]=0;
  cabacEstBits->significantBits[pos2ctx_map[type][k1]][1]=0;
  cabacEstBits->lastBits[pos2ctx_last[type][k1]][0]=0;
  cabacEstBits->lastBits[pos2ctx_last[type][k1]][1]=0;
}

/*!
****************************************************************************
* \brief
*    estimate bit cost of significant coefficient
****************************************************************************
*/
void est_significant_coefficients (Macroblock* currMB, EncodingEnvironmentPtr eep_dp,  int type)
{
  int   ctx;
  int maxCtx = imin(4, max_c2[type]);
  estBitsCabacStruct *cabacEstBits = &estBitsCabac[type];

  for (ctx=0; ctx<=4; ctx++){    
    cabacEstBits->greaterOneBits[0][ctx][0]=
      biari_no_bits (eep_dp, 0, img->currentSlice->tex_ctx->one_contexts[type2ctx_one[type]] + ctx);

    cabacEstBits->greaterOneBits[0][ctx][1]=
      biari_no_bits (eep_dp, 1, img->currentSlice->tex_ctx->one_contexts[type2ctx_one[type]] + ctx);
  }

  for (ctx=0; ctx<=maxCtx; ctx++){
    cabacEstBits->greaterOneBits[1][ctx][0]=
      biari_no_bits(eep_dp, 0, img->currentSlice->tex_ctx->abs_contexts[type2ctx_abs[type]] + ctx);

    cabacEstBits->greaterOneState[ctx]=biari_state(eep_dp, 0, img->currentSlice->tex_ctx->abs_contexts[type2ctx_abs[type]] + ctx);

    cabacEstBits->greaterOneBits[1][ctx][1]=
      biari_no_bits(eep_dp, 1, img->currentSlice->tex_ctx->abs_contexts[type2ctx_abs[type]] + ctx);
  }
}

int biari_no_bits(EncodingEnvironmentPtr eep, signed short symbol, BiContextTypePtr bi_ct )
{
  int ctx_state, estBits;

  symbol = (short) (symbol != 0);

  ctx_state = (symbol == bi_ct->MPS) ? 64 + bi_ct->state : 63 - bi_ct->state;
  estBits=entropyBits[127 - ctx_state];

  return(estBits);
}

int biari_state(EncodingEnvironmentPtr eep, signed short symbol, BiContextTypePtr bi_ct )
{ 
  int ctx_state;

  symbol = (short) (symbol != 0);
  ctx_state = (symbol == bi_ct->MPS) ? 64 + bi_ct->state : 63 - bi_ct->state;

  return(ctx_state);
}

#define BIT_SET(x,n)  ((int)(((x)&((int64)1<<(n)))>>(n)))

/*!
****************************************************************************
* \brief
*    estimate CABAC CBP bits
****************************************************************************
*/
int est_write_and_store_CBP_block_bit(Macroblock* currMB, int type) 
{
  estBitsCabacStruct *cabacEstBits = &estBitsCabac[type];

  int y_ac        = (type==LUMA_16AC || type==LUMA_8x8 || type==LUMA_8x4 || type==LUMA_4x8 || type==LUMA_4x4
    || type==CB_16AC || type==CB_8x8 || type==CB_8x4 || type==CB_4x8 || type==CB_4x4
    || type==CR_16AC || type==CR_8x8 || type==CR_8x4 || type==CR_4x8 || type==CR_4x4);
  int y_dc        = (type==LUMA_16DC || type==CB_16DC || type==CR_16DC); 
  int u_ac        = (type==CHROMA_AC && !img->is_v_block);
  int v_ac        = (type==CHROMA_AC &&  img->is_v_block);
  int chroma_dc   = (type==CHROMA_DC || type==CHROMA_DC_2x4 || type==CHROMA_DC_4x4);
  int u_dc        = (chroma_dc && !img->is_v_block);
  int v_dc        = (chroma_dc &&  img->is_v_block);
  int j           = ((y_ac || u_ac || v_ac ? img->subblock_y : 0) << 2);
  int i           =  (y_ac || u_ac || v_ac ? img->subblock_x : 0);
  int bit;
  int default_bit =  (IS_INTRA(currMB) ? 1 : 0);
  int upper_bit   = default_bit;
  int left_bit    = default_bit;
  int ctx, estBits = 0;

  int bit_pos_a   = 0;
  int bit_pos_b   = 0;

  PixelPos block_a, block_b;

  if (y_ac || y_dc)
  {
    get4x4Neighbour(currMB, (i << 2) - 1, j   , img->mb_size[IS_LUMA], &block_a);
    get4x4Neighbour(currMB, (i << 2),     j -1, img->mb_size[IS_LUMA], &block_b);
    if (y_ac)
    {
      if (block_a.available)
        bit_pos_a = 4*block_a.y + block_a.x;
      if (block_b.available)
        bit_pos_b = 4*block_b.y + block_b.x;
    }
  }
  else
  {
    get4x4Neighbour(currMB, (i << 2) - 1, j    , img->mb_size[IS_CHROMA], &block_a);
    get4x4Neighbour(currMB, (i << 2),     j - 1, img->mb_size[IS_CHROMA], &block_b);
    if (u_ac||v_ac)
    {
      if (block_a.available)
        bit_pos_a = (block_a.y << 2) + block_a.x;
      if (block_b.available)
        bit_pos_b = (block_b.y << 2) + block_b.x;
    }
  }

  bit = (y_dc ? 0 : y_ac ? 1 : u_dc ? 17 : v_dc ? 18 : u_ac ? 19 : 35);

  if (enc_picture->chroma_format_idc!=YUV444 || IS_INDEPENDENT(params))
  {
    if (type!=LUMA_8x8)
    {
      if (block_b.available)
      {
        if(img->mb_data[block_b.mb_addr].mb_type==IPCM)
          upper_bit = 1;
        else
          upper_bit = BIT_SET(img->mb_data[block_b.mb_addr].cbp_bits[0],bit+bit_pos_b);
      }

      if (block_a.available)
      {
        if(img->mb_data[block_a.mb_addr].mb_type==IPCM)
          left_bit = 1;
        else
          left_bit = BIT_SET(img->mb_data[block_a.mb_addr].cbp_bits[0],bit+bit_pos_a);
      }

      ctx = 2*upper_bit+left_bit;
      //===== encode symbol =====
      estBits = cabacEstBits->blockCbpBits[ctx][0] - cabacEstBits->blockCbpBits[ctx][1];
    }
  }
  else 
  {
    if (block_b.available)
    {
      if(img->mb_data[block_b.mb_addr].mb_type == IPCM)
        upper_bit=1;
      else
      {
        if(type==LUMA_8x8)
          upper_bit = BIT_SET(img->mb_data[block_b.mb_addr].cbp_bits_8x8[0], bit + bit_pos_b);
        else if (type==CB_8x8)
          upper_bit = BIT_SET(img->mb_data[block_b.mb_addr].cbp_bits_8x8[1], bit + bit_pos_b);
        else if (type==CR_8x8)
          upper_bit = BIT_SET(img->mb_data[block_b.mb_addr].cbp_bits_8x8[2], bit + bit_pos_b);
        else if ((type==CB_4x4)||(type==CB_4x8)||(type==CB_8x4)||(type==CB_16AC)||(type==CB_16DC))
          upper_bit = BIT_SET(img->mb_data[block_b.mb_addr].cbp_bits[1], bit + bit_pos_b);
        else if ((type==CR_4x4)||(type==CR_4x8)||(type==CR_8x4)||(type==CR_16AC)||(type==CR_16DC))
          upper_bit = BIT_SET(img->mb_data[block_b.mb_addr].cbp_bits[2], bit + bit_pos_b);
        else
          upper_bit = BIT_SET(img->mb_data[block_b.mb_addr].cbp_bits[0], bit + bit_pos_b);
      }
    }

    if (block_a.available)
    {
      if(img->mb_data[block_a.mb_addr].mb_type==IPCM)
        left_bit = 1;
      else
      {
        if(type==LUMA_8x8)
          left_bit = BIT_SET(img->mb_data[block_a.mb_addr].cbp_bits_8x8[0],bit + bit_pos_a);
        else if (type==CB_8x8)
          left_bit = BIT_SET(img->mb_data[block_a.mb_addr].cbp_bits_8x8[1],bit + bit_pos_a);
        else if (type==CR_8x8)
          left_bit = BIT_SET(img->mb_data[block_a.mb_addr].cbp_bits_8x8[2],bit + bit_pos_a);
        else if ((type==CB_4x4)||(type==CB_4x8)||(type==CB_8x4)||(type==CB_16AC)||(type==CB_16DC))
          left_bit = BIT_SET(img->mb_data[block_a.mb_addr].cbp_bits[1],bit + bit_pos_a);
        else if ((type==CR_4x4)||(type==CR_4x8)||(type==CR_8x4)||(type==CR_16AC)||(type==CR_16DC))
          left_bit = BIT_SET(img->mb_data[block_a.mb_addr].cbp_bits[2],bit + bit_pos_a);
        else
          left_bit = BIT_SET(img->mb_data[block_a.mb_addr].cbp_bits[0],bit + bit_pos_a);
      }
    }

    ctx = 2*upper_bit+left_bit;

    //===== encode symbol =====
    estBits = cabacEstBits->blockCbpBits[ctx][0] - cabacEstBits->blockCbpBits[ctx][1];
  }

  return(estBits);
}
/*!
****************************************************************************
* \brief
*    Rate distortion optimized trellis quantization
****************************************************************************
*/
void est_writeRunLevel_CABAC(levelDataStruct levelData[], int levelTabMin[], int type, double lambda, int kInit, int kStop, 
                             int noCoeff, int estCBP)
{
  estBitsCabacStruct *cabacEstBits = &estBitsCabac[type];
  int   k, i;
  int   estBits;
  double lagr, lagrMin=0, lagrTabMin, lagrTab;
  int   c1 = 1, c2 = 0, c1Tab[3], c2Tab[3];
  int   iBest, levelTab[64];
  int   ctx, greater_one, last, maxK = maxpos[type];
  double   lagrAcc, lagrLastMin=0, lagrLast;
  int      kBest=0, kStart, first;
  levelDataStruct *dataLevel;

  for (k = 0; k <= maxK; k++)
  {
    levelTabMin[k] = 0;
  }

  if (noCoeff > 0)
  {
    if (noCoeff > 1)
    {
      kStart = kInit; kBest = 0; first = 1; 
      lagrAcc = 0; 
      for (k = kStart; k <= kStop; k++)
      {
        lagrAcc += levelData[k].errLevel[0];
      }

      if (levelData[kStart].noLevels > 2)
      { 
        lagrAcc -= levelData[kStart].errLevel[0];
        lagrLastMin=lambda * (cabacEstBits->lastBits[pos2ctx_last[type][kStart]][1] - cabacEstBits->lastBits[pos2ctx_last[type][kStart]][0]) + lagrAcc;

        kBest = kStart;
        kStart++;
        first = 0;
      }

      for (k = kStart; k <= kStop; k++)
      {
        dataLevel = &levelData[k];
        lagrMin  = dataLevel->errLevel[0] + lambda * cabacEstBits->significantBits[pos2ctx_map[type][k]][0];
        lagrAcc -= dataLevel->errLevel[0];

        if (dataLevel->noLevels > 1)
        { 
          estBits = SIGN_BITS + cabacEstBits->significantBits[pos2ctx_map[type][k]][1]+
            cabacEstBits->greaterOneBits[0][4][0];

          lagrLast = dataLevel->errLevel[1] + lambda * (estBits + cabacEstBits->lastBits[pos2ctx_last[type][k]][1]) + lagrAcc;
          lagr     = dataLevel->errLevel[1] + lambda * (estBits + cabacEstBits->lastBits[pos2ctx_last[type][k]][0]);

          lagrMin = (lagr < lagrMin) ? lagr : lagrMin;

          if (lagrLast < lagrLastMin || first==1)
          {
            kBest = k;
            first = 0;
            lagrLastMin = lagrLast;
          }
        }
        lagrAcc += lagrMin;
      }
      kStart = kBest;
    }
    else
    {
      kStart = kStop;
    }

    lagrTabMin = 0;
    for (k = 0; k <= kStart; k++)
    {
      lagrTabMin += levelData[k].errLevel[0];
    }
    // Initial Lagrangian calculation
    lagrTab=0;

    //////////////////////////

    lagrTabMin += (lambda*estCBP);
    iBest = 0; first = 1;
    for (k = kStart; k >= 0; k--)
    {
      dataLevel = &levelData[k];
      last = (k == kStart);

      if (!last)
      {
        lagrMin = dataLevel->errLevel[0] + lambda * cabacEstBits->significantBits[pos2ctx_map[type][k]][0];
        iBest = 0;
        first = 0;
      }

      for (i = 1; i < dataLevel->noLevels; i++)
      {
        estBits = SIGN_BITS + cabacEstBits->significantBits[pos2ctx_map[type][k]][1];
        estBits += cabacEstBits->lastBits[pos2ctx_last[type][k]][last];

        // greater than 1
        greater_one = (dataLevel->level[i] > 1);

        c1Tab[i] = c1;   
        c2Tab[i] = c2;

        ctx = imin(c1Tab[i], 4);  
        estBits += cabacEstBits->greaterOneBits[0][ctx][greater_one];

        // magnitude if greater than 1
        if (greater_one)
        {
          ctx = imin(c2Tab[i], max_c2[type]);
          if ( (dataLevel->level[i] - 2) < MAX_PREC_COEFF)
          {
            estBits += precalcUnaryLevelTab[cabacEstBits->greaterOneState[ctx]][dataLevel->level[i] - 2];
          }
          else
          {
            estBits += est_unary_exp_golomb_level_encode(dataLevel->level[i] - 2, ctx, type);
          }

          c1Tab[i] = 0;
          c2Tab[i]++;
        }
        else if (c1Tab[i])
        {
          c1Tab[i]++;
        }

        lagr = dataLevel->errLevel[i] + lambda*estBits;
        if (lagr<lagrMin || first==1)
        {
          iBest = i;
          lagrMin=lagr;
          first = 0;
        }
      }

      if (iBest>0)
      {
        c1 = c1Tab[iBest]; 
        c2 = c2Tab[iBest];
      }

      levelTab[k] = dataLevel->level[iBest];
      lagrTab += lagrMin;
    }
    ///////////////////////////////////

    if (lagrTab < lagrTabMin)
    {
      memcpy(levelTabMin, levelTab, (kStart + 1) * sizeof(int));
    }
  }
}

/*!
****************************************************************************
* \brief
*    estimate unary exp golomb bit cost
****************************************************************************
*/
int est_unary_exp_golomb_level_encode(unsigned int symbol, int ctx, int type)
{
  estBitsCabacStruct *cabacEstBits = &estBitsCabac[type];
  unsigned int l = symbol, k = 1;
  unsigned int exp_start = 13; // 15-2 : 0,1 level decision always sent
  int estBits;

  if (symbol==0)
  {
    estBits = cabacEstBits->greaterOneBits[1][ctx][0];
    return (estBits);
  }
  else
  {
    estBits = cabacEstBits->greaterOneBits[1][ctx][1];
    // this could be done using min of the two conditionals * value
    while (((--l)>0) && (++k <= exp_start))
    {
      estBits += cabacEstBits->greaterOneBits[1][ctx][1];
    }
    
    if (symbol < exp_start)
    {
      estBits += cabacEstBits->greaterOneBits[1][ctx][0];
    }
    else 
    {
      estBits+=est_exp_golomb_encode_eq_prob(symbol - exp_start);
    }
  }
  return(estBits);
}

/*!
****************************************************************************
* \brief
*    Initialize levelData 
****************************************************************************
*/
int init_trellis_data_4x4_CABAC(int (*tblock)[16], int block_x, int qp_per, int qp_rem, int **levelscale, int **leveloffset, const byte *p_scan, Macroblock *currMB,  
                      levelDataStruct *dataLevel, int* kStart, int* kStop, int type)
{
  int noCoeff = 0;
  int i, j, coeff_ctr;
  static int *m7;
  int end_coeff_ctr = ( ( type == LUMA_4x4 ) ? 16 : 15 );
  int q_bits = Q_BITS + qp_per; 
  int q_offset = ( 1 << (q_bits - 1) );
  int level, lowerInt, k;
  double err, estErr;

  
  for (coeff_ctr = 0; coeff_ctr < end_coeff_ctr; coeff_ctr++)
  {
    i = *p_scan++;  // horizontal position
    j = *p_scan++;  // vertical position

    m7 = &tblock[j][block_x + i];

    if (*m7 == 0)
    {
      dataLevel->levelDouble = 0;
      dataLevel->level[0] = 0;
      dataLevel->noLevels = 1;
      err = 0.0;
      dataLevel->errLevel[0] = 0.0;
    }
    else
    {
      estErr = ((double) estErr4x4[qp_rem][j][i]) / norm_factor_4x4;

      dataLevel->levelDouble = iabs(*m7 * levelscale[j][i]);
      level = (dataLevel->levelDouble >> q_bits);

      lowerInt = (((int)dataLevel->levelDouble - (level << q_bits)) < q_offset )? 1 : 0;

      dataLevel->level[0] = 0; 
      if (level == 0 && lowerInt == 1)
      {
        dataLevel->noLevels = 1;
      }
      else if (level == 0 && lowerInt == 0)
      {
        dataLevel->level[1] = 1;
        dataLevel->noLevels = 2;
        *kStop = coeff_ctr;
        noCoeff++;
      }
      else if (level > 0 && lowerInt == 1)
      {
        dataLevel->level[1] = level;
        dataLevel->noLevels = 2;
        *kStop = coeff_ctr;
        noCoeff++;
      }
      else
      {
        dataLevel->level[1] = level;
        dataLevel->level[2] = level + 1;
        dataLevel->noLevels = 3;
        *kStop  = coeff_ctr;
        *kStart = coeff_ctr;
        noCoeff++;
      }

      for (k = 0; k < dataLevel->noLevels; k++)
      {
        err = (double)(dataLevel->level[k] << q_bits) - (double)dataLevel->levelDouble;
        dataLevel->errLevel[k] = (err * err * estErr); 
      }
    }

    dataLevel++;
  }
  return (noCoeff);
}

/*
****************************************************************************
* \brief
*    Initialize levelData 
****************************************************************************
*/
int init_trellis_data_8x8_CABAC(int (*tblock)[16], int block_x, int qp_per, int qp_rem, int **levelscale, int **leveloffset, const byte *p_scan, Macroblock *currMB,  
                      levelDataStruct *dataLevel, int* kStart, int* kStop, int symbolmode)
{
  int noCoeff = 0;
  static int *m7;
  int i, j, coeff_ctr, end_coeff_ctr = 64;
  int q_bits = Q_BITS_8 + qp_per;
  int q_offset = ( 1 << (q_bits - 1) );
  double err, estErr; 
  int level, lowerInt, k;
  
  for (coeff_ctr = 0; coeff_ctr < end_coeff_ctr; coeff_ctr++)
  {
    i = *p_scan++;  // horizontal position
    j = *p_scan++;  // vertical position
    
    m7 = &tblock[j][block_x + i];

    if (*m7 == 0)
    {
      dataLevel->levelDouble = 0;
      dataLevel->level[0] = 0;
      dataLevel->noLevels = 1;
      err = 0.0;
      dataLevel->errLevel[0] = 0.0;
    }
    else
    {
      estErr = (double) estErr8x8[qp_rem][j][i] / norm_factor_8x8;

      dataLevel->levelDouble = iabs(*m7 * levelscale[j][i]);
      level = (dataLevel->levelDouble >> q_bits);

      lowerInt = (((int)dataLevel->levelDouble - (level << q_bits)) < q_offset )? 1 : 0;

      dataLevel->level[0] = 0;
      if (level == 0 && lowerInt == 1)
      {
        dataLevel->noLevels = 1;
      }
      else if (level == 0 && lowerInt == 0)
      {
        dataLevel->level[1] = 1;
        dataLevel->noLevels = 2;
        *kStop = coeff_ctr;
        noCoeff++;
      }
      else if (level > 0 && lowerInt == 1)
      {
        dataLevel->level[1] = level;
        dataLevel->noLevels = 2;
        *kStop = coeff_ctr;
        noCoeff++;
      }
      else
      {
        dataLevel->level[1] = level;
        dataLevel->level[2] = level + 1;
        dataLevel->noLevels = 3;
        *kStop  = coeff_ctr;
        *kStart = coeff_ctr;
        noCoeff++;
      }

      for (k = 0; k < dataLevel->noLevels; k++)
      {
        err = (double)(dataLevel->level[k] << q_bits) - (double)dataLevel->levelDouble;
        dataLevel->errLevel[k] = err * err * estErr; 
      }
    }
 
    dataLevel++;
  }
  return (noCoeff);
}

/*!
****************************************************************************
* \brief
*    Initialize levelData for Luma DC
****************************************************************************
*/
int init_trellis_data_DC_CABAC(int (*tblock)[4], int qp_per, int qp_rem, 
                         int levelscale, int leveloffset, const byte *p_scan, Macroblock *currMB,  
                         levelDataStruct *dataLevel, int* kStart, int* kStop)
{
  int noCoeff = 0;
  int i, j, coeff_ctr, end_coeff_ctr = 16;
  int q_bits   = Q_BITS + qp_per + 1; 
  int q_offset = ( 1 << (q_bits - 1) );
  int level, lowerInt, k;
  int *m7;
  double err, estErr = (double) estErr4x4[qp_rem][0][0] / norm_factor_4x4; // note that we could also use int64

  for (coeff_ctr = 0; coeff_ctr < end_coeff_ctr; coeff_ctr++)
  {
    i = *p_scan++;  // horizontal position
    j = *p_scan++;  // vertical position
    m7 = &tblock[j][i];

    if (*m7 == 0)
    {
      dataLevel->levelDouble = 0;
      dataLevel->level[0] = 0;
      dataLevel->noLevels = 1;
      err = 0.0;
      dataLevel->errLevel[0] = 0.0;
    }
    else
    {
      dataLevel->levelDouble = iabs(*m7 * levelscale);
      level = (dataLevel->levelDouble >> q_bits);

      lowerInt=( ((int)dataLevel->levelDouble - (level<<q_bits)) < q_offset )? 1 : 0;

      dataLevel->level[0] = 0;    
      if (level == 0 && lowerInt == 1)
      {
        dataLevel->noLevels = 1;
      }
      else if (level == 0 && lowerInt == 0)
      {
        dataLevel->level[1] = 1;
        dataLevel->noLevels = 2;
        *kStop = coeff_ctr;
        noCoeff++;
      }
      else if (level > 0 && lowerInt == 1)
      {
        dataLevel->level[1] = level;
        dataLevel->noLevels = 2;
        *kStop = coeff_ctr;
        noCoeff++;
      }
      else
      {
        dataLevel->level[1] = level;
        dataLevel->level[2] = level + 1;
        dataLevel->noLevels = 3;
        *kStop  = coeff_ctr;
        *kStart = coeff_ctr;
        noCoeff++;
      }

      for (k = 0; k < dataLevel->noLevels; k++)
      {
        err = (double)(dataLevel->level[k] << q_bits) - (double)dataLevel->levelDouble;
        dataLevel->errLevel[k] = (err * err * estErr); 
      }
    }
    dataLevel++;
  }
  return (noCoeff);
}



