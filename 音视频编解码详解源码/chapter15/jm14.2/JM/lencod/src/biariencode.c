
/*!
 *************************************************************************************
 * \file biariencode.c
 *
 * \brief
 *   Routines for binary arithmetic encoding.
 *
 *   This modified implementation of the M Coder is based on JVT-U084 
 *   with the choice of M_BITS = 16.
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Detlev Marpe                    <marpe@hhi.de>
 *    - Gabi Blaettermann
 *    - Gunnar Marten
 *************************************************************************************
 */

#include "global.h"
#include "biariencode.h"

int binCount = 0;


int pic_bin_count;

void reset_pic_bin_count(void)
{
  pic_bin_count = 0;
}

int get_pic_bin_count(void)
{
  return pic_bin_count;
}



/*!
 ************************************************************************
 * \brief
 *    Allocates memory for the EncodingEnvironment struct
 ************************************************************************
 */
EncodingEnvironmentPtr arienco_create_encoding_environment(void)
{
  EncodingEnvironmentPtr eep;

  if ( (eep = (EncodingEnvironmentPtr) calloc(1,sizeof(EncodingEnvironment))) == NULL)
    no_mem_exit("arienco_create_encoding_environment: eep");

  return eep;
}



/*!
 ************************************************************************
 * \brief
 *    Frees memory of the EncodingEnvironment struct
 ************************************************************************
 */
void arienco_delete_encoding_environment(EncodingEnvironmentPtr eep)
{
  if (eep == NULL)
  {
    snprintf(errortext, ET_SIZE, "Error freeing eep (NULL pointer)");
    error (errortext, 200);
  }
  else
    free(eep);
}

/*!
 ************************************************************************
 * Macro for writing bytes of code
 ***********************************************************************
 */
#define put_one_byte(b) { \
  if(Epbuf==3){ \
    put_buffer(eep);\
  } \
  Ebuffer = (Ebuffer<<8) + (b); \
  Epbuf++; \
}

#define put_one_word(b) { \
  if(Epbuf>=3){ \
    put_buffer(eep);\
  } \
  Ebuffer = (Ebuffer<<16) + (b); \
  Epbuf+=2; \
}

#define put_one_byte_final(b) { \
  Ecodestrm[(*Ecodestrm_len)++] = b; \
}

#define _put_last_chunk_plus_outstanding(l) { \
  while (Echunks_outstanding > 0) \
  { \
    put_one_word(0xFFFF); \
    Echunks_outstanding--; \
  } \
  put_one_word(l); \
}

#define _put_last_chunk_plus_outstanding_final(l) { \
  while (Echunks_outstanding > 0) \
  { \
    put_one_word(0xFFFF); \
    Echunks_outstanding--; \
  } \
  put_one_byte(l); \
}

void put_buffer(EncodingEnvironmentPtr eep)
{
  while(Epbuf>=0)
  {
    Ecodestrm[(*Ecodestrm_len)++] =  (Ebuffer>>((Epbuf--)<<3))&0xFF; 
  }
  while(eep->C > 7)
  {
    eep->C-=8;
    eep->E++;
  }
  Ebuffer=0; 
}


void propagate_carry(EncodingEnvironmentPtr eep)
{
  Ebuffer +=1; 
  while (Echunks_outstanding > 0) 
  { 
    put_one_word(0); 
    Echunks_outstanding--; 
  }
}

/*!
************************************************************************
* \brief
*    Initializes the EncodingEnvironment E and C values to zero
************************************************************************
*/
void arienco_reset_EC(EncodingEnvironmentPtr eep)
{
  eep->E = 0;
  eep->C = 0;
}

/*!
 ************************************************************************
 * \brief
 *    Initializes the EncodingEnvironment for the arithmetic coder
 ************************************************************************
 */
void arienco_start_encoding(EncodingEnvironmentPtr eep,
                            unsigned char *code_buffer,
                            int *code_len )
{
  Elow = 0;
  Echunks_outstanding = 0;
  Ebuffer = 0;
  Epbuf = -1;  // to remove redundant chunk ^^
  Ebits_to_go = BITS_TO_LOAD + 1; // to swallow first redundant bit

  Ecodestrm = code_buffer;
  Ecodestrm_len = code_len;

  Erange = HALF-2;

}

/*!
 ************************************************************************
 * \brief
 *    Returns the number of currently written bits
 ************************************************************************
 */
int arienco_bits_written(EncodingEnvironmentPtr eep)
{
  return (((*Ecodestrm_len) + Epbuf + 1) << 3) + (Echunks_outstanding * BITS_TO_LOAD) + BITS_TO_LOAD - Ebits_to_go;
}



/*!
************************************************************************
* \brief
*    add slice bin number to picture bin counter
*    should be only used when slice is terminated
************************************************************************
*/
void set_pic_bin_count(EncodingEnvironmentPtr eep)
{
  pic_bin_count += eep->E*8 + eep->C; // no of processed bins
}

/*!
 ************************************************************************
 * \brief
 *    Terminates the arithmetic codeword, writes stop bit and stuffing bytes (if any)
 ************************************************************************
 */
void arienco_done_encoding(EncodingEnvironmentPtr eep)
{
  register unsigned int low = Elow;
  int bl = Ebits_to_go;
  int remaining_bits = BITS_TO_LOAD - bl; // output (2 + remaining) bits for terminating the codeword + one stop bit
  unsigned char mask;
  int* bitCount = img->mb_data[img->current_mb_nr].bitcounter;

  //pic_bin_count += eep->E*8 + eep->C; // no of processed bins

  if (remaining_bits <= 5) // one terminating byte 
  {
    bitCount[BITS_STUFFING]+=(5-remaining_bits);
    mask = 255 - ((1 << (6-remaining_bits)) - 1); 
    low = (low >> (MAX_BITS - 8)) & mask; // mask out the (2+remaining_bits) MSBs
    low += (1<<(5-remaining_bits));       // put the terminating stop bit '1'

    _put_last_chunk_plus_outstanding_final(low);
    put_buffer(eep);
  }
  else if(remaining_bits <=13)            // two terminating bytes
  {
    bitCount[BITS_STUFFING]+=(13-remaining_bits);
    _put_last_chunk_plus_outstanding_final(((low >> (MAX_BITS - 8)) & 0xFF)); // mask out the 8 MSBs for output

    put_buffer(eep);
    if (remaining_bits > 6)
    {
      mask = 255 - ((1 << (14 - remaining_bits)) - 1); 
      low = (low >> (MAX_BITS - 16)) & mask; 
      low += (1<<(13-remaining_bits));     // put the terminating stop bit '1'
      put_one_byte_final(low);
    }
    else
    {
      put_one_byte_final(128); // second byte contains terminating stop bit '1' only
    }
  }
  else             // three terminating bytes
  { 
    _put_last_chunk_plus_outstanding(((low >> (MAX_BITS - BITS_TO_LOAD)) & B_LOAD_MASK)); // mask out the 16 MSBs for output
    put_buffer(eep);
    bitCount[BITS_STUFFING]+=(21-remaining_bits);

    if (remaining_bits > 14)
    {
      mask = 255 - ((1 << (22 - remaining_bits)) - 1); 
      low = (low >> (MAX_BITS - 24)) & mask; 
      low += (1<<(21-remaining_bits));       // put the terminating stop bit '1'
      put_one_byte_final(low);
    }
    else
    {
      put_one_byte_final(128); // third byte contains terminating stop bit '1' only
    }
  }
  Ebits_to_go=8;
}

extern int cabac_encoding;

/*!
 ************************************************************************
 * \brief
 *    Actually arithmetic encoding of one binary symbol by using
 *    the probability estimate of its associated context model
 ************************************************************************
 */
void biari_encode_symbol(EncodingEnvironmentPtr eep, signed short symbol, BiContextTypePtr bi_ct )
{
  register unsigned int range = Erange;
  register unsigned int low = Elow;
  unsigned int rLPS = rLPS_table_64x4[bi_ct->state][(range>>6) & 3]; 
  register int bl = Ebits_to_go;

  range -= rLPS;

  eep->C++;
  bi_ct->count += cabac_encoding;

  /* covers all cases where code does not bother to shift down symbol to be 
  * either 0 or 1, e.g. in some cases for cbp, mb_Type etc the code simply 
  * masks off the bit position and passes in the resulting value */
  symbol = (short) (symbol != 0);

  if (symbol == bi_ct->MPS)  //MPS
  {
    bi_ct->state = AC_next_state_MPS_64[bi_ct->state]; // next state

    if( range >= QUARTER ) // no renorm
    {
      Erange = range;
      return;
    }
    else 
    {   
      range<<=1;
      if( --bl > MIN_BITS_TO_GO )  // renorm once, no output
      {
        Erange = range;
        Ebits_to_go = bl;
        return;
      }
    }

  } 
  else         //LPS
  {
    unsigned int renorm;

    low += range<<bl;
    range = rLPS;

    if (!bi_ct->state)
      bi_ct->MPS ^= 0x01;               // switch MPS if necessary

    bi_ct->state = AC_next_state_LPS_64[bi_ct->state]; // next state

    renorm= renorm_table_32[(rLPS>> 3) & 0x1F];
    bl-=renorm;

    range<<=renorm;

    if (low >= ONE) // output of carry needed
    {
      low -= ONE;
      propagate_carry(eep);
    }
    if( bl > MIN_BITS_TO_GO )
    {
      Erange = range;
      Elow = low;
      Ebits_to_go = bl;
      return;
    }
  }

  //renorm needed

  Elow = (low << BITS_TO_LOAD )& (ONE - 1);
  low = (low >> (MAX_BITS - BITS_TO_LOAD)) & B_LOAD_MASK; // mask out the 8/16 MSBs for output

  if (low < B_LOAD_MASK) // no carry possible, output now
  {
    _put_last_chunk_plus_outstanding(low);
  }
  else          // low == "FF.."; keep it, may affect future carry
  {
    Echunks_outstanding++;
  }
  Erange = range;
  bl += BITS_TO_LOAD;
  Ebits_to_go = bl;
}

/*!
 ************************************************************************
 * \brief
 *    Arithmetic encoding of one binary symbol assuming 
 *    a fixed prob. distribution with p(symbol) = 0.5
 ************************************************************************
 */
void biari_encode_symbol_eq_prob(EncodingEnvironmentPtr eep, signed short symbol)
{
  register unsigned int low = Elow;
  Ebits_to_go--;  
  eep->C++;

  if (symbol != 0)
  {
    low += Erange<<Ebits_to_go;
    if (low >= ONE) // output of carry needed
    {
      low -= ONE;
      propagate_carry(eep);
    }
  }
  if(Ebits_to_go == MIN_BITS_TO_GO)  // renorm needed
  {
    Elow = (low << BITS_TO_LOAD )& (ONE - 1);
    low = (low >> (MAX_BITS - BITS_TO_LOAD)) & B_LOAD_MASK; // mask out the 8/16 MSBs for output
    if (low < B_LOAD_MASK)      // no carry possible, output now
    {
      _put_last_chunk_plus_outstanding(low);}
    else          // low == "FF"; keep it, may affect future carry
    {
      Echunks_outstanding++;
    }

    Ebits_to_go = BITS_TO_LOAD;
    return;
  }
  else         // no renorm needed
  {
    Elow = low;
    return;
  }
}

/*!
 ************************************************************************
 * \brief
 *    Arithmetic encoding for last symbol before termination
 ************************************************************************
 */
void biari_encode_symbol_final(EncodingEnvironmentPtr eep, signed short symbol)
{
  register unsigned int range = Erange-2;
  register unsigned int low = Elow;
  int bl = Ebits_to_go; 

  eep->C++;

  if (symbol == 0) // MPS
  {
    if( range >= QUARTER ) // no renorm
    {
      Erange = range;
      return;
    }
    else 
    {   
      range<<=1;
      if( --bl > MIN_BITS_TO_GO )  // renorm once, no output
      {
        Erange =range;
        Ebits_to_go = bl;
        return;
      }
    }
  }
  else     // LPS
  {
    low += range<<bl;
    range = 2;

    if (low >= ONE) // output of carry needed
    {
      low -= ONE; // remove MSB, i.e., carry bit
      propagate_carry(eep);
    }
    bl -= 7; // 7 left shifts needed to renormalize

    range<<=7;
    if( bl > MIN_BITS_TO_GO )
    {
      Erange = range;
      Elow = low;
      Ebits_to_go = bl;
      return;
    }
  }


  //renorm needed

  Elow = (low << BITS_TO_LOAD ) & (ONE - 1);
  low = (low >> (MAX_BITS - BITS_TO_LOAD)) & B_LOAD_MASK; // mask out the 8/16 MSBs
  if (low < B_LOAD_MASK)
  {  // no carry possible, output now
    _put_last_chunk_plus_outstanding(low);
  }
  else
  {  // low == "FF"; keep it, may affect future carry
    Echunks_outstanding++;
  }

  Erange = range;
  bl += BITS_TO_LOAD;
  Ebits_to_go = bl;
}

/*!
 ************************************************************************
 * \brief
 *    Initializes a given context with some pre-defined probability state
 ************************************************************************
 */
void biari_init_context (BiContextTypePtr ctx, const int* ini)
{
  int pstate = iClip3 ( 1, 126, ((ini[0]* imax(0, img->currentSlice->qp)) >> 4) + ini[1]);

  if ( pstate >= 64 )
  {
    ctx->state  = (unsigned short) (pstate - 64);
    ctx->MPS    = 1;
  }
  else
  {
    ctx->state  = (unsigned short) (63 - pstate);
    ctx->MPS    = 0;
  }

  ctx->count = 0;
}

