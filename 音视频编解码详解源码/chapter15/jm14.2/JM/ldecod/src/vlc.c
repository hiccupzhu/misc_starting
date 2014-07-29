/*!
 ************************************************************************
 * \file vlc.c
 *
 * \brief
 *    VLC support functions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Inge Lille-Langøy               <inge.lille-langoy@telenor.com>
 *    - Detlev Marpe                    <marpe@hhi.de>
 *    - Gabi Blaettermann
 ************************************************************************
 */
#include "contributors.h"

#include "global.h"
#include "vlc.h"
#include "elements.h"


// A little trick to avoid those horrible #if TRACE all over the source code
#if TRACE
#define SYMTRACESTRING(s) strncpy(symbol.tracestring,s,TRACESTRING_SIZE)
#else
#define SYMTRACESTRING(s) // do nothing
#endif

void tracebits(const char *trace_str,  int len,  int info,int value1);



int UsedBits;      // for internal statistics, is adjusted by se_v, ue_v, u_1

// Note that all NA values are filled with 0

//! gives CBP value from codeword number, both for intra and inter
static const unsigned char NCBP[2][48][2]=
{
  {  // 0      1        2       3       4       5       6       7       8       9      10      11
    {15, 0},{ 0, 1},{ 7, 2},{11, 4},{13, 8},{14, 3},{ 3, 5},{ 5,10},{10,12},{12,15},{ 1, 7},{ 2,11},
    { 4,13},{ 8,14},{ 6, 6},{ 9, 9},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},
    { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},
    { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0}
  },
  {
    {47, 0},{31,16},{15, 1},{ 0, 2},{23, 4},{27, 8},{29,32},{30, 3},{ 7, 5},{11,10},{13,12},{14,15},
    {39,47},{43, 7},{45,11},{46,13},{16,14},{ 3, 6},{ 5, 9},{10,31},{12,35},{19,37},{21,42},{26,44},
    {28,33},{35,34},{37,36},{42,40},{44,39},{ 1,43},{ 2,45},{ 4,46},{ 8,17},{17,18},{18,20},{20,24},
    {24,19},{ 6,21},{ 9,26},{22,28},{25,23},{32,27},{33,29},{34,30},{36,22},{40,25},{38,38},{41,41}
  }
};

//! for the linfo_levrun_inter routine
const byte NTAB1[4][8][2] =
{
  {{1,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
  {{1,1},{1,2},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
  {{2,0},{1,3},{1,4},{1,5},{0,0},{0,0},{0,0},{0,0}},
  {{3,0},{2,1},{2,2},{1,6},{1,7},{1,8},{1,9},{4,0}},
};
const byte LEVRUN1[16]=
{
  4,2,2,1,1,1,1,1,1,1,0,0,0,0,0,0,
};


const byte NTAB2[4][8][2] =
{
  {{1,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
  {{1,1},{2,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
  {{1,2},{3,0},{4,0},{5,0},{0,0},{0,0},{0,0},{0,0}},
  {{1,3},{1,4},{2,1},{3,1},{6,0},{7,0},{8,0},{9,0}},
};

//! for the linfo_levrun__c2x2 routine
const byte LEVRUN3[4] =
{
  2,1,0,0
};
const byte NTAB3[2][2][2] =
{
  {{1,0},{0,0}},
  {{2,0},{1,1}},
};

/*!
 *************************************************************************************
 * \brief
 *    ue_v, reads an ue(v) syntax element, the length in bits is stored in
 *    the global UsedBits variable
 *
 * \param tracestring
 *    the string for the trace file
 *
 * \param bitstream
 *    the stream to be read from
 *
 * \return
 *    the value of the coded syntax element
 *
 *************************************************************************************
 */
int ue_v (char *tracestring, Bitstream *bitstream)
{
  SyntaxElement symbol;

  assert (bitstream->streamBuffer != NULL);
  symbol.type = SE_HEADER;
  symbol.mapping = linfo_ue;   // Mapping rule
  SYMTRACESTRING(tracestring);
  readSyntaxElement_VLC (&symbol, bitstream);
  UsedBits+=symbol.len;
  return symbol.value1;
}


/*!
 *************************************************************************************
 * \brief
 *    ue_v, reads an se(v) syntax element, the length in bits is stored in
 *    the global UsedBits variable
 *
 * \param tracestring
 *    the string for the trace file
 *
 * \param bitstream
 *    the stream to be read from
 *
 * \return
 *    the value of the coded syntax element
 *
 *************************************************************************************
 */
int se_v (char *tracestring, Bitstream *bitstream)
{
  SyntaxElement symbol;

  assert (bitstream->streamBuffer != NULL);
  symbol.type = SE_HEADER;
  symbol.mapping = linfo_se;   // Mapping rule: signed integer
  SYMTRACESTRING(tracestring);
  readSyntaxElement_VLC (&symbol, bitstream);
  UsedBits+=symbol.len;
  return symbol.value1;
}


/*!
 *************************************************************************************
 * \brief
 *    ue_v, reads an u(v) syntax element, the length in bits is stored in
 *    the global UsedBits variable
 *
 * \param LenInBits
 *    length of the syntax element
 *
 * \param tracestring
 *    the string for the trace file
 *
 * \param bitstream
 *    the stream to be read from
 *
 * \return
 *    the value of the coded syntax element
 *
 *************************************************************************************
 */
int u_v (int LenInBits, char*tracestring, Bitstream *bitstream)
{
  SyntaxElement symbol;
  symbol.inf = 0;

  assert (bitstream->streamBuffer != NULL);
  symbol.type = SE_HEADER;
  symbol.mapping = linfo_ue;   // Mapping rule
  symbol.len = LenInBits;
  SYMTRACESTRING(tracestring);
  readSyntaxElement_FLC (&symbol, bitstream);
  UsedBits+=symbol.len;
  return symbol.inf;
}

/*!
 *************************************************************************************
 * \brief
 *    i_v, reads an i(v) syntax element, the length in bits is stored in
 *    the global UsedBits variable
 *
 * \param LenInBits
 *    length of the syntax element
 *
 * \param tracestring
 *    the string for the trace file
 *
 * \param bitstream
 *    the stream to be read from
 *
 * \return
 *    the value of the coded syntax element
 *
 *************************************************************************************
 */
int i_v (int LenInBits, char*tracestring, Bitstream *bitstream)
{
  SyntaxElement symbol;

  symbol.inf = 0;

  assert (bitstream->streamBuffer != NULL);
  symbol.type = SE_HEADER;
  symbol.mapping = linfo_ue;   // Mapping rule
  symbol.len = LenInBits;
  SYMTRACESTRING(tracestring);
  readSyntaxElement_FLC (&symbol, bitstream);
  UsedBits+=symbol.len;

  // can be negative
  symbol.inf = -( symbol.inf & (1 << (LenInBits - 1)) ) | symbol.inf;

  return symbol.inf;
}


/*!
 *************************************************************************************
 * \brief
 *    ue_v, reads an u(1) syntax element, the length in bits is stored in
 *    the global UsedBits variable
 *
 * \param tracestring
 *    the string for the trace file
 *
 * \param bitstream
 *    the stream to be read from
 *
 * \return
 *    the value of the coded syntax element
 *
 *************************************************************************************
 */
Boolean u_1 (char *tracestring, Bitstream *bitstream)
{
  return (Boolean) u_v (1, tracestring, bitstream);
}



/*!
 ************************************************************************
 * \brief
 *    mapping rule for ue(v) syntax elements
 * \par Input:
 *    lenght and info
 * \par Output:
 *    number in the code table
 ************************************************************************
 */
void linfo_ue(int len, int info, int *value1, int *dummy)
{
  assert ((len >> 1) < 32);
  *value1 = (1 << (len >> 1)) + info - 1;
}

/*!
 ************************************************************************
 * \brief
 *    mapping rule for se(v) syntax elements
 * \par Input:
 *    lenght and info
 * \par Output:
 *    signed mvd
 ************************************************************************
 */
void linfo_se(int len,  int info, int *value1, int *dummy)
{
  int n;
  assert ((len >> 1) < 32);
  n = (1 << (len >> 1)) + info - 1;
  *value1 = (n + 1) >> 1;
  if((n & 0x01) == 0)                           // lsb is signed bit
    *value1 = -*value1;
}


/*!
 ************************************************************************
 * \par Input:
 *    length and info
 * \par Output:
 *    cbp (intra)
 ************************************************************************
 */
void linfo_cbp_intra(int len,int info,int *cbp, int *dummy)
{
  int cbp_idx;

  linfo_ue(len, info, &cbp_idx, dummy);
  if((active_sps->chroma_format_idc==0)||(active_sps->chroma_format_idc==3))
    *cbp=NCBP[0][cbp_idx][0];
  else
    *cbp=NCBP[1][cbp_idx][0];

}

/*!
 ************************************************************************
 * \par Input:
 *    length and info
 * \par Output:
 *    cbp (inter)
 ************************************************************************
 */
void linfo_cbp_inter(int len,int info,int *cbp, int *dummy)
{
  int cbp_idx;

  linfo_ue(len, info, &cbp_idx, dummy);
  if((active_sps->chroma_format_idc==0)||(active_sps->chroma_format_idc==3))
    *cbp=NCBP[0][cbp_idx][1];
  else
    *cbp=NCBP[1][cbp_idx][1];
}

/*!
 ************************************************************************
 * \par Input:
 *    length and info
 * \par Output:
 *    level, run
 ************************************************************************
 */
void linfo_levrun_inter(int len, int info, int *level, int *irun)
{
  int l2;
  int inf;
  assert (((len >> 1) - 5) < 32);
  
  if (len <= 9)
  {
    l2     = imax(0,(len >> 1)-1);
    inf    = info >> 1;
    *level = NTAB1[l2][inf][0];
    *irun  = NTAB1[l2][inf][1];
    if ((info & 0x01) == 1)
      *level = -*level;                   // make sign
  }
  else                                  // if len > 9, skip using the array
  {
    *irun  = (info & 0x1e) >> 1;
    *level = LEVRUN1[*irun] + (info >> 5) + ( 1 << ((len >> 1) - 5));
    if ((info & 0x01) == 1)
      *level = -*level;
  }
  
  if (len == 1) // EOB
    *level = 0;
}


/*!
 ************************************************************************
 * \par Input:
 *    length and info
 * \par Output:
 *    level, run
 ************************************************************************
 */
void linfo_levrun_c2x2(int len, int info, int *level, int *irun)
{
  int l2;
  int inf;

  if (len<=5)
  {
    l2     = imax(0, (len >> 1) - 1);
    inf    = info >> 1;
    *level = NTAB3[l2][inf][0];
    *irun  = NTAB3[l2][inf][1];
    if ((info & 0x01) == 1)
      *level = -*level;                 // make sign
  }
  else                                  // if len > 5, skip using the array
  {
    *irun  = (info & 0x06) >> 1;
    *level = LEVRUN3[*irun] + (info >> 3) + (1 << ((len >> 1) - 3));
    if ((info & 0x01) == 1)
      *level = -*level;
  }
  if (len == 1) // EOB
    *level = 0;
}

/*!
 ************************************************************************
 * \brief
 *    read next UVLC codeword from UVLC-partition and
 *    map it to the corresponding syntax element
 ************************************************************************
 */
int readSyntaxElement_VLC(SyntaxElement *sym, Bitstream *currStream)
{
  int frame_bitoffset        = currStream->frame_bitoffset;
  int BitstreamLengthInBytes = currStream->bitstream_length;
  byte *buf                  = currStream->streamBuffer;

  sym->len =  GetVLCSymbol (buf, frame_bitoffset, &(sym->inf), BitstreamLengthInBytes);
  if (sym->len == -1)
    return -1;
  currStream->frame_bitoffset += sym->len;
  sym->mapping(sym->len,sym->inf,&(sym->value1),&(sym->value2));

#if TRACE
  tracebits(sym->tracestring, sym->len, sym->inf, sym->value1);
#endif

  return 1;
}


/*!
 ************************************************************************
 * \brief
 *    read next UVLC codeword from UVLC-partition and
 *    map it to the corresponding syntax element
 ************************************************************************
 */
int readSyntaxElement_UVLC(SyntaxElement *sym, ImageParameters *img, struct datapartition *dP)
{
  return (readSyntaxElement_VLC(sym, dP->bitstream));
}

/*!
 ************************************************************************
 * \brief
 *    read next VLC codeword for 4x4 Intra Prediction Mode and
 *    map it to the corresponding Intra Prediction Direction
 ************************************************************************
 */
int readSyntaxElement_Intra4x4PredictionMode(SyntaxElement *sym, ImageParameters *img, Bitstream   *currStream)
{
  int         *frame_bitoffset       = &currStream->frame_bitoffset;

  sym->len = GetVLCSymbol_IntraMode (currStream->streamBuffer, *frame_bitoffset, &(sym->inf), currStream->bitstream_length);

  if (sym->len == -1)
    return -1;

  *frame_bitoffset += sym->len;
  sym->value1       = (sym->len == 1) ? -1 : sym->inf;

#if TRACE
  tracebits2(sym->tracestring, sym->len, sym->value1);
#endif

  return 1;
}

int GetVLCSymbol_IntraMode (byte buffer[],int totbitoffset,int *info, int bytecount)
{

  register int inf;
  long byteoffset = (totbitoffset >> 3);        // byte from start of buffer
  int bitoffset   = (7 - (totbitoffset & 0x07)); // bit from start of byte
  byte *cur_byte  = &(buffer[byteoffset]);
  int ctr_bit     = (*cur_byte & (0x01 << bitoffset));      // control bit for current bit posision
  int bitcounter  = 1;
  int len         = 0;

  //First bit
  if (ctr_bit)
  {
    *info = 0;
    return bitcounter;
  }
  else
    len = 3;

  if (byteoffset + ((len + 7) >> 3) > bytecount)
    return -1;

  // make infoword
  inf = 0;                          // shortest possible code is 1, then info is always 0    

  while (len--)
  {
    bitcounter++;
    bitoffset --;
    bitoffset &= 0x07;
    cur_byte  += (bitoffset == 7);
    inf <<= 1;
    inf |= ((*cur_byte)>> (bitoffset)) & 0x01;
  }

  *info = inf;
  return bitcounter;           // return absolute offset in bit from start of frame
}


/*!
 ************************************************************************
 * \brief
 *    test if bit buffer contains only stop bit
 *
 * \param buffer
 *    buffer containing VLC-coded data bits
 * \param totbitoffset
 *    bit offset from start of partition
 * \param bytecount
 *    buffer length
 * \return
 *    true if more bits available
 ************************************************************************
 */
int more_rbsp_data (byte buffer[],int totbitoffset,int bytecount)
{
  int bitoffset   = (7 - (totbitoffset & 0x07));      // bit from start of byte
  long byteoffset = (totbitoffset >> 3);      // byte from start of buffer
  byte *cur_byte  = &(buffer[byteoffset]);
  int ctr_bit     = 0;      // control bit for current bit posision
  int cnt         = 0;

  assert (byteoffset<bytecount);

  // there is more until we're in the last byte
  if (byteoffset < (bytecount - 1)) return TRUE;

  // read one bit
  ctr_bit = ((*cur_byte)>> (bitoffset--)) & 0x01;

  // a stop bit has to be one
  if (ctr_bit==0) return TRUE;  

  while (bitoffset>=0 && !cnt)
  {
    cnt |= ((*cur_byte)>> (bitoffset--)) & 0x01;   // set up control bit
  }

  return (cnt);
}


/*!
 ************************************************************************
 * \brief
 *    Check if there are symbols for the next MB
 ************************************************************************
 */
int uvlc_startcode_follows(Slice *currSlice, int dummy)
{
  int dp_Nr = assignSE2partition[currSlice->dp_mode][SE_MBTYPE];
  DataPartition *dP = &(currSlice->partArr[dp_Nr]);
  Bitstream   *currStream = dP->bitstream;
  byte *buf  = currStream->streamBuffer;

  //KS: new function test for End of Buffer
  return (!(more_rbsp_data(buf, currStream->frame_bitoffset,currStream->bitstream_length)));
}



/*!
 ************************************************************************
 * \brief
 *  read one exp-golomb VLC symbol
 *
 * \param buffer
 *    containing VLC-coded data bits
 * \param totbitoffset
 *    bit offset from start of partition
 * \param  info
 *    returns the value of the symbol
 * \param bytecount
 *    buffer length
 * \return
 *    bits read
 ************************************************************************
 */
int GetVLCSymbol (byte buffer[],int totbitoffset,int *info, int bytecount)
{

  register int inf;
  long byteoffset = (totbitoffset >> 3);         // byte from start of buffer
  int  bitoffset  = (7 - (totbitoffset & 0x07)); // bit from start of byte
  int  bitcounter = 1;
  int  len        = 0;
  byte *cur_byte = &(buffer[byteoffset]);
  int  ctr_bit    = ((*cur_byte) >> (bitoffset)) & 0x01;  // control bit for current bit posision

  while (ctr_bit == 0)
  {                 // find leading 1 bit
    len++;
    bitcounter++;
    bitoffset--;
    bitoffset &= 0x07;
    cur_byte  += (bitoffset == 7);
    byteoffset+= (bitoffset == 7);      
    ctr_bit    = ((*cur_byte) >> (bitoffset)) & 0x01;
  }

  if (byteoffset + ((len + 7) >> 3) > bytecount)
    return -1;

  // make infoword
  inf = 0;                          // shortest possible code is 1, then info is always 0    

  while (len--)
  {
    bitoffset --;    
    bitoffset &= 0x07;
    cur_byte  += (bitoffset == 7);
    bitcounter++;
    inf <<= 1;    
    inf |= ((*cur_byte) >> (bitoffset)) & 0x01;
  }

  *info = inf;
  return bitcounter;           // return absolute offset in bit from start of frame
}

extern void tracebits2(const char *trace_str,  int len,  int info) ;

/*!
 ************************************************************************
 * \brief
 *    code from bitstream (2d tables)
 ************************************************************************
 */

int code_from_bitstream_2d(SyntaxElement *sym,
                           Bitstream *currStream,
                           const int *lentab,
                           const int *codtab,
                           int tabwidth,
                           int tabheight,
                           int *code)
{
  int frame_bitoffset        = currStream->frame_bitoffset;
  int BitstreamLengthInBytes = currStream->bitstream_length - (frame_bitoffset >> 3);
  byte *buf                  = &currStream->streamBuffer[frame_bitoffset>>3];
  int current_bit_pos        = 7 - (frame_bitoffset & 0x07);

  int i, j;
  const int *len = &lentab[0], *cod = &codtab[0];

  // this VLC decoding method is not optimized for speed
  for (j = 0; j < tabheight; j++) 
  {
    for (i = 0; i < tabwidth; i++)
    {
      //if ((*len == 0) || (ShowBits(buf, frame_bitoffset, BitstreamLengthInBytes, *len) != *cod))
      if ((*len == 0) || (ShowBitsThres(buf, current_bit_pos, BitstreamLengthInBytes, *len, *cod) != *cod))
      {
        len++;
        cod++;
      }
      else
      {
        sym->value1 = i;
        sym->value2 = j;        
        sym->len = *len;
        currStream->frame_bitoffset += *len; // move bitstream pointer
        *code = *cod;                        // found code and return
        return 0;
      }
    }
  }
  return -1;  // failed to find code
}


/*!
 ************************************************************************
 * \brief
 *    read FLC codeword from UVLC-partition
 ************************************************************************
 */
int readSyntaxElement_FLC(SyntaxElement *sym, Bitstream *currStream)
{
  int frame_bitoffset        = currStream->frame_bitoffset;
  int BitstreamLengthInBytes = currStream->bitstream_length;
  byte *buf                  = currStream->streamBuffer;

  if ((GetBits(buf, frame_bitoffset, &(sym->inf), BitstreamLengthInBytes, sym->len)) < 0)
    return -1;

  sym->value1 = sym->inf;
  currStream->frame_bitoffset += sym->len; // move bitstream pointer

#if TRACE
  tracebits2(sym->tracestring, sym->len, sym->inf);
#endif

  return 1;
}



/*!
 ************************************************************************
 * \brief
 *    read NumCoeff/TrailingOnes codeword from UVLC-partition
 ************************************************************************
 */

int readSyntaxElement_NumCoeffTrailingOnes(SyntaxElement *sym,  
                                           Bitstream *currStream,
                                           char *type)
{
  int frame_bitoffset        = currStream->frame_bitoffset;
  int BitstreamLengthInBytes = currStream->bitstream_length;
  byte *buf                  = currStream->streamBuffer;

  static const int lentab[3][4][17] =
  {
    {   // 0702
      { 1, 6, 8, 9,10,11,13,13,13,14,14,15,15,16,16,16,16},
      { 0, 2, 6, 8, 9,10,11,13,13,14,14,15,15,15,16,16,16},
      { 0, 0, 3, 7, 8, 9,10,11,13,13,14,14,15,15,16,16,16},
      { 0, 0, 0, 5, 6, 7, 8, 9,10,11,13,14,14,15,15,16,16},
    },
    {
      { 2, 6, 6, 7, 8, 8, 9,11,11,12,12,12,13,13,13,14,14},
      { 0, 2, 5, 6, 6, 7, 8, 9,11,11,12,12,13,13,14,14,14},
      { 0, 0, 3, 6, 6, 7, 8, 9,11,11,12,12,13,13,13,14,14},
      { 0, 0, 0, 4, 4, 5, 6, 6, 7, 9,11,11,12,13,13,13,14},
    },
    {
      { 4, 6, 6, 6, 7, 7, 7, 7, 8, 8, 9, 9, 9,10,10,10,10},
      { 0, 4, 5, 5, 5, 5, 6, 6, 7, 8, 8, 9, 9, 9,10,10,10},
      { 0, 0, 4, 5, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,10,10,10},
      { 0, 0, 0, 4, 4, 4, 4, 4, 5, 6, 7, 8, 8, 9,10,10,10},
    },
  };

  static const int codtab[3][4][17] =
  {
    {
      { 1, 5, 7, 7, 7, 7,15,11, 8,15,11,15,11,15,11, 7,4},
      { 0, 1, 4, 6, 6, 6, 6,14,10,14,10,14,10, 1,14,10,6},
      { 0, 0, 1, 5, 5, 5, 5, 5,13, 9,13, 9,13, 9,13, 9,5},
      { 0, 0, 0, 3, 3, 4, 4, 4, 4, 4,12,12, 8,12, 8,12,8},
    },
    {
      { 3,11, 7, 7, 7, 4, 7,15,11,15,11, 8,15,11, 7, 9,7},
      { 0, 2, 7,10, 6, 6, 6, 6,14,10,14,10,14,10,11, 8,6},
      { 0, 0, 3, 9, 5, 5, 5, 5,13, 9,13, 9,13, 9, 6,10,5},
      { 0, 0, 0, 5, 4, 6, 8, 4, 4, 4,12, 8,12,12, 8, 1,4},
    },
    {
      {15,15,11, 8,15,11, 9, 8,15,11,15,11, 8,13, 9, 5,1},
      { 0,14,15,12,10, 8,14,10,14,14,10,14,10, 7,12, 8,4},
      { 0, 0,13,14,11, 9,13, 9,13,10,13, 9,13, 9,11, 7,3},
      { 0, 0, 0,12,11,10, 9, 8,13,12,12,12, 8,12,10, 6,2},
    },
  };

  int retval = 0, code;
  int vlcnum = sym->value1;
  // vlcnum is the index of Table used to code coeff_token
  // vlcnum==3 means (8<=nC) which uses 6bit FLC

  if (vlcnum == 3)
  {
    // read 6 bit FLC
    code = ShowBits(buf, frame_bitoffset, BitstreamLengthInBytes, 6);
    currStream->frame_bitoffset += 6;
    sym->value2 = (code & 3);
    sym->value1 = (code >> 2);

    if (!sym->value1 && sym->value2 == 3)
    {
      // #c = 0, #t1 = 3 =>  #c = 0
      sym->value2 = 0;
    }
    else
      sym->value1++;

    sym->len = 6;
  }
  else
  {
    //retval = code_from_bitstream_2d(sym, currStream, &lentab[vlcnum][0][0], &codtab[vlcnum][0][0], 17, 4, &code);    
    retval = code_from_bitstream_2d(sym, currStream, lentab[vlcnum][0], codtab[vlcnum][0], 17, 4, &code);
    if (retval)
    {
      printf("ERROR: failed to find NumCoeff/TrailingOnes\n");
      exit(-1);
    }
  }

#if TRACE
  snprintf(sym->tracestring,
    TRACESTRING_SIZE, "%s # c & tr.1s vlc=%d #c=%d #t1=%d",
           type, vlcnum, sym->value1, sym->value2);
  tracebits2(sym->tracestring, sym->len, code);
#endif

  return retval;
}


/*!
 ************************************************************************
 * \brief
 *    read NumCoeff/TrailingOnes codeword from UVLC-partition ChromaDC
 ************************************************************************
 */
int readSyntaxElement_NumCoeffTrailingOnesChromaDC(SyntaxElement *sym,  Bitstream *currStream)
{
  static const int lentab[3][4][17] =
  {
    //YUV420
   {{ 2, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    { 0, 1, 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    { 0, 0, 3, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    { 0, 0, 0, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    //YUV422
   {{ 1, 7, 7, 9, 9,10,11,12,13, 0, 0, 0, 0, 0, 0, 0, 0},
    { 0, 2, 7, 7, 9,10,11,12,12, 0, 0, 0, 0, 0, 0, 0, 0},
    { 0, 0, 3, 7, 7, 9,10,11,12, 0, 0, 0, 0, 0, 0, 0, 0},
    { 0, 0, 0, 5, 6, 7, 7,10,11, 0, 0, 0, 0, 0, 0, 0, 0}},
    //YUV444
   {{ 1, 6, 8, 9,10,11,13,13,13,14,14,15,15,16,16,16,16},
    { 0, 2, 6, 8, 9,10,11,13,13,14,14,15,15,15,16,16,16},
    { 0, 0, 3, 7, 8, 9,10,11,13,13,14,14,15,15,16,16,16},
    { 0, 0, 0, 5, 6, 7, 8, 9,10,11,13,14,14,15,15,16,16}}
  };

  static const int codtab[3][4][17] =
  {
    //YUV420
   {{ 1, 7, 4, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    { 0, 1, 6, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    { 0, 0, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    { 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    //YUV422
   {{ 1,15,14, 7, 6, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0},
    { 0, 1,13,12, 5, 6, 6, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0},
    { 0, 0, 1,11,10, 4, 5, 5, 4, 0, 0, 0, 0, 0, 0, 0, 0},
    { 0, 0, 0, 1, 1, 9, 8, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0}},
    //YUV444
   {{ 1, 5, 7, 7, 7, 7,15,11, 8,15,11,15,11,15,11, 7, 4},
    { 0, 1, 4, 6, 6, 6, 6,14,10,14,10,14,10, 1,14,10, 6},
    { 0, 0, 1, 5, 5, 5, 5, 5,13, 9,13, 9,13, 9,13, 9, 5},
    { 0, 0, 0, 3, 3, 4, 4, 4, 4, 4,12,12, 8,12, 8,12, 8}}

  };
   
  int code;
  int yuv = active_sps->chroma_format_idc - 1;
  int retval = code_from_bitstream_2d(sym, currStream, &lentab[yuv][0][0], &codtab[yuv][0][0], 17, 4, &code);

  if (retval)
  {
    printf("ERROR: failed to find NumCoeff/TrailingOnes ChromaDC\n");
    exit(-1);
  }

#if TRACE
    snprintf(sym->tracestring,
      TRACESTRING_SIZE, "ChrDC # c & tr.1s  #c=%d #t1=%d",
              sym->value1, sym->value2);
    tracebits2(sym->tracestring, sym->len, code);

#endif

  return retval;
}




/*!
 ************************************************************************
 * \brief
 *    read Level VLC0 codeword from UVLC-partition
 ************************************************************************
 */
int readSyntaxElement_Level_VLC0(SyntaxElement *sym, Bitstream *currStream)
{
  int frame_bitoffset        = currStream->frame_bitoffset;
  int BitstreamLengthInBytes = currStream->bitstream_length;
  byte *buf                  = currStream->streamBuffer;
  int len = 0, sign=0, level=0, code = 1;
  int offset, addbit;

  while (!ShowBits(buf, frame_bitoffset+len, BitstreamLengthInBytes, 1))
    len++;

  len++;
  frame_bitoffset += len;

  if (len < 15)
  {
    sign  = (len - 1) & 1;
    level = ((len - 1) >> 1) + 1;
  }
  else if (len == 15)
  {
    // escape code
    code <<= 4;
    code |= ShowBits(buf, frame_bitoffset, BitstreamLengthInBytes, 4);
    len  += 4;
    frame_bitoffset += 4;
    sign = (code & 0x01);
    level = ((code >> 1) & 0x07) + 8;
  }
  else if (len >= 16)
  {
    // escape code
    addbit = (len - 16);
    len   -= 4;
    code   = ShowBits(buf, frame_bitoffset, BitstreamLengthInBytes, len);
    sign   = (code & 0x01);
    frame_bitoffset += len;    

    offset = (2048 << addbit) - 2032;
    level = (code >> 1) + offset;
    code |= (1 << (len)); // for display purpose only
    len += addbit + 16;
 }

  sym->inf = (sign) ? -level : level ;
  sym->len = len;

#if TRACE
  tracebits2(sym->tracestring, sym->len, code);
#endif
  currStream->frame_bitoffset = frame_bitoffset;
  return 0;

}

/*!
 ************************************************************************
 * \brief
 *    read Level VLC codeword from UVLC-partition
 ************************************************************************
 */
int readSyntaxElement_Level_VLCN(SyntaxElement *sym, int vlc, Bitstream *currStream)
{
  int frame_bitoffset        = currStream->frame_bitoffset;
  int BitstreamLengthInBytes = currStream->bitstream_length;
  byte *buf                  = currStream->streamBuffer;

  int levabs, sign;
  int len = 0;
  int code = 1, sb;

  int numPrefix = 0;
  int shift = vlc - 1;
  int escape = (15 << shift) + 1;
  int addbit, offset;

  // read pre zeros
  while (!ShowBits(buf, frame_bitoffset + numPrefix, BitstreamLengthInBytes, 1))
    numPrefix++;


  len = numPrefix + 1;

  if (numPrefix < 15)
  {
    levabs = (numPrefix << shift) + 1;

    // read (vlc-1) bits -> suffix
    if (shift)
    {
      sb =  ShowBits(buf, frame_bitoffset + len, BitstreamLengthInBytes, shift);
      code = (code << (shift) )| sb;
      levabs += sb;
      len += (shift);
    }

    // read 1 bit -> sign
    sign = ShowBits(buf, frame_bitoffset + len, BitstreamLengthInBytes, 1);
    code = (code << 1)| sign;
    len ++;
  }
  else // escape
  {
    addbit = numPrefix - 15;

    sb = ShowBits(buf, frame_bitoffset + len, BitstreamLengthInBytes, (11 + addbit));
    code = (code << (11 + addbit) )| sb;

    len   += (11 + addbit);

    offset = (2048 << addbit) + escape - 2048;

    levabs = sb + offset;
    
    // read 1 bit -> sign
    sign = ShowBits(buf, frame_bitoffset + len, BitstreamLengthInBytes, 1);

    code = (code << 1)| sign;

    len++;
  }

  sym->inf = (sign)? -levabs : levabs;
  sym->len = len;

  currStream->frame_bitoffset = frame_bitoffset+len;

#if TRACE
  tracebits2(sym->tracestring, sym->len, code);
#endif

  return 0;
}

/*!
 ************************************************************************
 * \brief
 *    read Total Zeros codeword from UVLC-partition
 ************************************************************************
 */
int readSyntaxElement_TotalZeros(SyntaxElement *sym,  Bitstream *currStream)
{
  static const int lentab[TOTRUN_NUM][16] =
  {

    { 1,3,3,4,4,5,5,6,6,7,7,8,8,9,9,9},
    { 3,3,3,3,3,4,4,4,4,5,5,6,6,6,6},
    { 4,3,3,3,4,4,3,3,4,5,5,6,5,6},
    { 5,3,4,4,3,3,3,4,3,4,5,5,5},
    { 4,4,4,3,3,3,3,3,4,5,4,5},
    { 6,5,3,3,3,3,3,3,4,3,6},
    { 6,5,3,3,3,2,3,4,3,6},
    { 6,4,5,3,2,2,3,3,6},
    { 6,6,4,2,2,3,2,5},
    { 5,5,3,2,2,2,4},
    { 4,4,3,3,1,3},
    { 4,4,2,1,3},
    { 3,3,1,2},
    { 2,2,1},
    { 1,1},
  };

  static const int codtab[TOTRUN_NUM][16] =
  {
    {1,3,2,3,2,3,2,3,2,3,2,3,2,3,2,1},
    {7,6,5,4,3,5,4,3,2,3,2,3,2,1,0},
    {5,7,6,5,4,3,4,3,2,3,2,1,1,0},
    {3,7,5,4,6,5,4,3,3,2,2,1,0},
    {5,4,3,7,6,5,4,3,2,1,1,0},
    {1,1,7,6,5,4,3,2,1,1,0},
    {1,1,5,4,3,3,2,1,1,0},
    {1,1,1,3,3,2,2,1,0},
    {1,0,1,3,2,1,1,1,},
    {1,0,1,3,2,1,1,},
    {0,1,1,2,1,3},
    {0,1,1,1,1},
    {0,1,1,1},
    {0,1,1},
    {0,1},
  };
  int code;
  int vlcnum = sym->value1;
  int retval = code_from_bitstream_2d(sym, currStream, &lentab[vlcnum][0], &codtab[vlcnum][0], 16, 1, &code);

  if (retval)
  {
    printf("ERROR: failed to find Total Zeros !cdc\n");
    exit(-1);
  }


#if TRACE
    tracebits2(sym->tracestring, sym->len, code);

#endif

  return retval;
}

/*!
 ************************************************************************
 * \brief
 *    read Total Zeros Chroma DC codeword from UVLC-partition
 ************************************************************************
 */
int readSyntaxElement_TotalZerosChromaDC(SyntaxElement *sym,  Bitstream *currStream)
{
  static const int lentab[3][TOTRUN_NUM][16] =
  {
    //YUV420
   {{ 1,2,3,3},
    { 1,2,2},
    { 1,1}},
    //YUV422
   {{ 1,3,3,4,4,4,5,5},
    { 3,2,3,3,3,3,3},
    { 3,3,2,2,3,3},
    { 3,2,2,2,3},
    { 2,2,2,2},
    { 2,2,1},
    { 1,1}},
    //YUV444
   {{ 1,3,3,4,4,5,5,6,6,7,7,8,8,9,9,9},
    { 3,3,3,3,3,4,4,4,4,5,5,6,6,6,6},
    { 4,3,3,3,4,4,3,3,4,5,5,6,5,6},
    { 5,3,4,4,3,3,3,4,3,4,5,5,5},
    { 4,4,4,3,3,3,3,3,4,5,4,5},
    { 6,5,3,3,3,3,3,3,4,3,6},
    { 6,5,3,3,3,2,3,4,3,6},
    { 6,4,5,3,2,2,3,3,6},
    { 6,6,4,2,2,3,2,5},
    { 5,5,3,2,2,2,4},
    { 4,4,3,3,1,3},
    { 4,4,2,1,3},
    { 3,3,1,2},
    { 2,2,1},
    { 1,1}}
  };

  static const int codtab[3][TOTRUN_NUM][16] =
  {
    //YUV420
   {{ 1,1,1,0},
    { 1,1,0},
    { 1,0}},
    //YUV422
   {{ 1,2,3,2,3,1,1,0},
    { 0,1,1,4,5,6,7},
    { 0,1,1,2,6,7},
    { 6,0,1,2,7},
    { 0,1,2,3},
    { 0,1,1},
    { 0,1}},
    //YUV444
   {{1,3,2,3,2,3,2,3,2,3,2,3,2,3,2,1},
    {7,6,5,4,3,5,4,3,2,3,2,3,2,1,0},
    {5,7,6,5,4,3,4,3,2,3,2,1,1,0},
    {3,7,5,4,6,5,4,3,3,2,2,1,0},
    {5,4,3,7,6,5,4,3,2,1,1,0},
    {1,1,7,6,5,4,3,2,1,1,0},
    {1,1,5,4,3,3,2,1,1,0},
    {1,1,1,3,3,2,2,1,0},
    {1,0,1,3,2,1,1,1,},
    {1,0,1,3,2,1,1,},
    {0,1,1,2,1,3},
    {0,1,1,1,1},
    {0,1,1,1},
    {0,1,1},
    {0,1}}
  };

  int code;
  int yuv = active_sps->chroma_format_idc - 1;
  int vlcnum = sym->value1;
  int retval = code_from_bitstream_2d(sym, currStream, &lentab[yuv][vlcnum][0], &codtab[yuv][vlcnum][0], 16, 1, &code);

  if (retval)
  {
    printf("ERROR: failed to find Total Zeros\n");
    exit(-1);
  }


#if TRACE
  tracebits2(sym->tracestring, sym->len, code);
#endif

  return retval;
}


/*!
 ************************************************************************
 * \brief
 *    read  Run codeword from UVLC-partition
 ************************************************************************
 */
int readSyntaxElement_Run(SyntaxElement *sym, Bitstream *currStream)
{
  static const int lentab[TOTRUN_NUM][16] =
  {
    {1,1},
    {1,2,2},
    {2,2,2,2},
    {2,2,2,3,3},
    {2,2,3,3,3,3},
    {2,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,4,5,6,7,8,9,10,11},
  };

  static const int codtab[TOTRUN_NUM][16] =
  {
    {1,0},
    {1,1,0},
    {3,2,1,0},
    {3,2,1,1,0},
    {3,2,3,2,1,0},
    {3,0,1,3,2,5,4},
    {7,6,5,4,3,2,1,1,1,1,1,1,1,1,1},
  };
  int code;
  int vlcnum = sym->value1;
  int retval = code_from_bitstream_2d(sym, currStream, &lentab[vlcnum][0], &codtab[vlcnum][0], 16, 1, &code);

  if (retval)
  {
    printf("ERROR: failed to find Run\n");
    exit(-1);
  }

#if TRACE
    tracebits2(sym->tracestring, sym->len, code);
#endif

  return retval;
}


/*!
 ************************************************************************
 * \brief
 *  Reads bits from the bitstream buffer
 *
 * \param buffer
 *    containing VLC-coded data bits
 * \param totbitoffset
 *    bit offset from start of partition
 * \param info
 *    returns value of the read bits
 * \param bytecount
 *    total bytes in bitstream
 * \param numbits
 *    number of bits to read
 *
 ************************************************************************
 */
int GetBits (byte buffer[],int totbitoffset,int *info, int bytecount,
             int numbits)
{
  register int inf;
  int  bitoffset  = (totbitoffset & 0x07); // bit from start of byte
  long byteoffset = (totbitoffset >> 3);       // byte from start of buffer
  int  bitcounter = numbits;
  static byte *curbyte;

  if ((byteoffset) + ((numbits + bitoffset)>> 3)  > bytecount)
    return -1;

  curbyte = &(buffer[byteoffset]);

  bitoffset = 7 - bitoffset;

  inf=0;

  while (numbits--)
  {
    inf <<=1;    
    inf |= ((*curbyte)>> (bitoffset--)) & 0x01;    
    //curbyte   += (bitoffset >> 3) & 0x01;
    curbyte   -= (bitoffset >> 3);
    bitoffset &= 0x07;
    //curbyte   += (bitoffset == 7);    
  }

  *info = inf;
  return bitcounter;           // return absolute offset in bit from start of frame
}

/*!
 ************************************************************************
 * \brief
 *  Reads bits from the bitstream buffer
 *
 * \param buffer
 *    buffer containing VLC-coded data bits
 * \param totbitoffset
 *    bit offset from start of partition
 * \param bytecount
 *    total bytes in bitstream
 * \param numbits
 *    number of bits to read
 *
 ************************************************************************
 */

int ShowBits (byte buffer[],int totbitoffset,int bytecount, int numbits)
{

  register int inf;
  int  bitoffset  = (totbitoffset & 0x07); // bit from start of byte
  long byteoffset = (totbitoffset >> 3);       // byte from start of buffer
  static byte *curbyte;
  
  if ((byteoffset) + ((numbits + bitoffset)>> 3)  > bytecount)
    return -1;

  curbyte = &(buffer[byteoffset]);

  bitoffset = 7 - bitoffset;

  inf=0;

  while (numbits--)
  {
    inf <<=1;    
    inf |= ((*curbyte)>> (bitoffset--)) & 0x01;
    //curbyte   += (bitoffset >> 3) & 0x01;
    curbyte   -= (bitoffset >> 3);
    bitoffset &= 0x07;
    //curbyte   += (bitoffset == 7);    
  }

  return inf;           // return absolute offset in bit from start of frame
}


/*!
 ************************************************************************
 * \brief
 *  Reads bits from the bitstream buffer (Threshold based)
 *
 * \param curbyte
 *    current byte position in buffer
 * \param bitoffset
 *    bit offset at current position
 * \param bytecount
 *    total bytes in bitstream
 * \param numbits
 *    number of bits to read
 * \param code
 *    threshold parameter 
 *
 ************************************************************************
 */

int ShowBitsThres (byte *curbyte, int bitoffset, int bytecount, int numbits, int code)
{
  register int inf;  
     
  if (((numbits + 7)>> 3) > bytecount)
    return -1;
  else
  {
    inf=0;

#if 0
    while (numbits--)
    {
      inf <<=1;
      inf |= ((*curbyte)>> (bitoffset--)) & 0x01;
      if ( inf > code)
      {
        return -1;
      }

      //curbyte   += (bitoffset >> 3) & 0x01;
      curbyte   -= (bitoffset >> 3);
      bitoffset &= 0x07;
      //curbyte   += (bitoffset == 7);    
    }
#else
    while (numbits--)
    {
      inf <<=1;
      inf |= ((*curbyte)>> (bitoffset--)) & 0x01;
      if ( (inf << numbits) > code)
      {
        return -1;
      }

      //curbyte   += (bitoffset >> 3) & 0x01; // the mask does not seem to be needed since value can be 0 or -1 only.
      curbyte   -= (bitoffset >> 3);
      bitoffset &= 0x07;
      //curbyte   += (bitoffset == 7);    
    }
#endif
    return inf;           // return absolute offset in bit from start of frame
  }
}

/*!
 ************************************************************************
 * \brief
 *    peek at the next 2 UVLC codeword from UVLC-partition to determine
 *    if a skipped MB is field/frame
 ************************************************************************
 */
int peekSyntaxElement_UVLC(SyntaxElement *sym, ImageParameters *img, struct datapartition *dP)
{
  Bitstream   *currStream    = dP->bitstream;
  int frame_bitoffset        = currStream->frame_bitoffset;
  int BitstreamLengthInBytes = currStream->bitstream_length;
  byte *buf                  = currStream->streamBuffer;

  sym->len =  GetVLCSymbol (buf, frame_bitoffset, &(sym->inf), BitstreamLengthInBytes);
  if (sym->len == -1)
    return -1;
  frame_bitoffset += sym->len;
  sym->mapping(sym->len, sym->inf, &(sym->value1), &(sym->value2));

#if TRACE
  tracebits(sym->tracestring, sym->len, sym->inf, sym->value1);
#endif

  return 1;
}


