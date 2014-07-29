/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Bitstream reader/writer inlined functions and constants-
 *
 *  Copyright (C) 2001-2003 Peter Ross <pross@xvid.org>
 *
 *  This program is free software ; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation ; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY ; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program ; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * $Id$
 *
 ****************************************************************************/

#ifndef _BITSTREAM_H_
#define _BITSTREAM_H_

#include "../portab.h"
//#include "../decoder.h"
#include "../encoder.h"


/*****************************************************************************
 * Constants
 ****************************************************************************/

/* comment any #defs we dont use */

#define VIDOBJ_START_CODE		0x00000100	/* ..0x0000011f  */
#define VIDOBJLAY_START_CODE	0x00000120	/* ..0x0000012f */
#define VISOBJSEQ_START_CODE	0x000001b0
#define VISOBJSEQ_STOP_CODE		0x000001b1	/* ??? */
#define USERDATA_START_CODE		0x000001b2
#define GRPOFVOP_START_CODE		0x000001b3
/*#define VIDSESERR_ERROR_CODE  0x000001b4 */
#define VISOBJ_START_CODE		0x000001b5
#define VOP_START_CODE			0x000001b6
/*#define STUFFING_START_CODE	0x000001c3 */


#define VISOBJ_TYPE_VIDEO				1
/*#define VISOBJ_TYPE_STILLTEXTURE      2 */
/*#define VISOBJ_TYPE_MESH              3 */
/*#define VISOBJ_TYPE_FBA               4 */
/*#define VISOBJ_TYPE_3DMESH            5 */


#define VIDOBJLAY_TYPE_SIMPLE			1
#define VIDOBJLAY_TYPE_ART_SIMPLE		10
#define VIDOBJLAY_TYPE_ASP              17
#define VIDOBJLAY_AR_EXTPAR				15


#define VIDOBJLAY_SHAPE_RECTANGULAR		0
#define VIDOBJLAY_SHAPE_BINARY			1
#define VIDOBJLAY_SHAPE_BINARY_ONLY		2
#define VIDOBJLAY_SHAPE_GRAYSCALE		3


#define WRITE_MARKER()	BitstreamPutBit(bs, 1)

/* vop coding types  */
/* intra, prediction, backward, sprite, not_coded */
#define I_VOP	0
#define P_VOP	1
#define B_VOP	2
#define S_VOP	3
#define N_VOP	4

/* resync-specific */
#define NUMBITS_VP_RESYNC_MARKER  17
#define RESYNC_MARKER 1


/*****************************************************************************
 * Prototypes
 ****************************************************************************/

void BitstreamWriteVolHeader(Bitstream * const bs,
							 const MBParam * pParam,
							 const FRAMEINFO * const frame);

void BitstreamWriteVopHeader(Bitstream * const bs,
							 const MBParam * pParam,
							 const FRAMEINFO * const frame,
							 int vop_coded,
							 unsigned int quant);

void BitstreamWriteUserData(Bitstream * const bs,
							uint8_t * data,
							const int length);

void myBitstreamPutBits(Bitstream * const bs,
				 const uint32_t value,
				 const uint32_t size);
/* initialise bitstream structure */

static void __inline
BitstreamInit(Bitstream * const bs,
			  void *const bitstream,
			  uint32_t length)
{
//	uint32_t tmp;
//	size_t bitpos;
//	ptr_t adjbitstream = (ptr_t)bitstream;

	/*
	 * Start the stream on a uint32_t boundary, by rounding down to the
	 * previous uint32_t and skipping the intervening bytes.
	 */
//	bitpos = 0;//((sizeof(uint32_t)-1) & (size_t)bitstream);
	bs->start = bs->tail = (uint32_t *)bitstream;//adjbitstream;
	bs->buf = 0;
	bs->pos = 0;//bs->initpos = bitpos*8;
	bs->length = length;
}

static void __inline
BitstreamReset(Bitstream * const bs)
{
	bs->tail = bs->start;
	bs->buf = 0;
	bs->pos = 0;//bs->initpos;
}
/* skip n bits forward in bitstream */

static __inline void
BitstreamSkip(Bitstream * const bs,
			  const uint32_t bits)
{
	bs->pos += bits;

	if (bs->pos >= 32) {
		bs->tail++;
		bs->pos -= 32;
	}
}


/* number of bits to next byte alignment */
static __inline uint32_t
BitstreamNumBitsToByteAlign(Bitstream *bs)
{
	uint32_t n = (32 - bs->pos) % 8;
	return n == 0 ? 8 : n;
}

/* move forward to the next byte boundary */

static __inline void
BitstreamByteAlign(Bitstream * const bs)
{
	uint32_t remainder = bs->pos % 8;

	if (remainder) {
		BitstreamSkip(bs, 8 - remainder);
	}
}


/* bitstream length (unit bits) */

static uint32_t __inline
BitstreamPos(const Bitstream * const bs)
{
	return ((uint32_t)(8*((ptr_t)bs->tail - (ptr_t)bs->start) + bs->pos));// - bs->initpos));
}


/*
 * flush the bitstream & return length (unit bytes)
 * NOTE: assumes no futher bitstream functions will be called.
 */


static uint32_t __inline
BitstreamLength(Bitstream * const bs)
{
	uint32_t len = (uint32_t)((ptr_t)bs->tail - (ptr_t)bs->start);

	if (bs->pos) {
		uint32_t b = bs->buf;

#if 1
		BSWAP(b);
#else
        
#endif
		*bs->tail = b;

		len += (bs->pos + 7) / 8;
	}

	/* initpos is always on a byte boundary */
//	if (bs->initpos)
//		len -= bs->initpos/8;

	return len;
}


/* move bitstream position forward by n bits and write out buffer if needed */

static void __inline
BitstreamForward(Bitstream * const bs,
				 const uint32_t bits)
{
	bs->pos += bits;

	if (bs->pos >= 32) 
    {
		uint32_t b = bs->buf;
		BSWAP(b);
		*bs->tail++ = b;
		bs->buf = 0;
		bs->pos -= 32;
	}
}


/* write single bit to bitstream */

static void __inline
BitstreamPutBit(Bitstream * const bs,
				const uint32_t bit)
{
	if (bit) bs->buf |= (0x80000000 >> bs->pos);
	BitstreamForward(bs, 1);
}


/* write n bits to bitstream */

static void __inline
BitstreamPutBits(Bitstream * const bs,
				 const uint32_t value,
				 const uint32_t size)
{
#if 1
// Ç¶ÈëÊ½»ã±à???
	uint32_t shift = 32 - bs->pos - size;

	if (shift <= 32) 
    {
		bs->buf |= value << shift;
		BitstreamForward(bs, size);
	} 
    else 
    {
		uint32_t remainder;
		shift = size - (32 - bs->pos);
		bs->buf |= value >> shift;
		BitstreamForward(bs, size - shift);
		remainder = shift;

		shift = 32 - shift;

		bs->buf |= value << shift;
		BitstreamForward(bs, remainder);
	}
#else
	int b,mask,oppos,op;
	
	mask = 0;oppos = 0;
	op = bs->pos + size;
	if (op<=32){
		bs->buf |= value << (32 - op);
		bs->pos += size;
		oppos = bs->pos-32;
		mask = 1;
	} 
	else{
		int shift = op - 32;
		bs->buf |= value >> (op - 32);
		b = bs->buf;
		BSWAP(b);
		*bs->tail++ = b;
		bs->buf = 0;
		bs->pos = shift;
		bs->buf |= value << (32 - shift);
	} 
	if (mask&&!oppos){
		b = bs->buf;
		BSWAP(b);
		*bs->tail++ = b;
		bs->buf = bs->pos = 0;
	}
#endif
}

static const int stuffing_codes[8] =
{
	        /* nbits     stuffing code */
	0,		/* 1          0 */
	1,		/* 2          01 */
	3,		/* 3          011 */
	7,		/* 4          0111 */
	0xf,	/* 5          01111 */
	0x1f,	/* 6          011111 */
	0x3f,   /* 7          0111111 */
	0x7f,	/* 8          01111111 */
};

/* pad bitstream to the next byte boundary */

static void __inline
BitstreamPad(Bitstream * const bs)
{
	int bits = 8 - (bs->pos % 8);
	if (bits < 8)
		BitstreamPutBits(bs, stuffing_codes[bits - 1], bits);
}


/*
 * pad bitstream to the next byte boundary
 * alway pad: even if currently at the byte boundary
 */

static void __inline
BitstreamPadAlways(Bitstream * const bs)
{
	int bits = 8 - (bs->pos % 8);
	BitstreamPutBits(bs, stuffing_codes[bits - 1], bits);
}

#endif /* _BITSTREAM_H_ */
