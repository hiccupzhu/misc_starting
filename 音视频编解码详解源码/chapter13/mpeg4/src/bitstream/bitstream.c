/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Bitstream reader/writer -
 *
 ****************************************************************************/

#include <string.h>
#include <stdio.h>

#include "bitstream.h"
//#include "zigzag.h"
//#include "../quant/quant_matrix.h"
#include "mbcoding.h"


static uint32_t __inline
log2bin(uint32_t value)
{
	int n = 0;

	while (value) {
		value >>= 1;
		n++;
	}
	return n;
}

/*
	write vol header
*/
void
BitstreamWriteVolHeader(Bitstream * const bs,
						const MBParam * pParam,
						const FRAMEINFO * const frame)
{
	/* Write the VOS header */
	BitstreamPutBits(bs, VISOBJSEQ_START_CODE, 32);
	BitstreamPutBits(bs, pParam->profile, 8); 	/* profile_and_level_indication */

	/* visual_object_start_code */
	BitstreamPad(bs);
	BitstreamPutBits(bs, VISOBJ_START_CODE, 32);
	BitstreamPutBits(bs, 0, 1);		/* is_visual_object_identifier */

	/* Video type */
	BitstreamPutBits(bs, VISOBJ_TYPE_VIDEO, 4);		/* visual_object_type */
	BitstreamPutBit(bs, 0); /* video_signal_type */

	/* video object_start_code & vo_id */
	BitstreamPadAlways(bs); /* next_start_code() */
	BitstreamPutBits(bs, VIDOBJ_START_CODE, 32);

	/* video_object_layer_start_code & vol_id */
	BitstreamPad(bs);
	BitstreamPutBits(bs, VIDOBJLAY_START_CODE, 32);

	BitstreamPutBit(bs, 0);		/* random_accessible_vol */
	BitstreamPutBits(bs, VIDOBJLAY_TYPE_SIMPLE, 8);	/* video_object_type_indication */

	BitstreamPutBit(bs, 0);				/* is_object_layer_identified (0=not given) */

	/* Aspect ratio */
	BitstreamPutBits(bs, pParam->par, 4); /* aspect_ratio_info (1=1:1) */

	BitstreamPutBit(bs, 1);	/* vol_control_parameters */
	BitstreamPutBits(bs, 1, 2);	/* chroma_format 1="4:2:0" */

	BitstreamPutBit(bs, 1);	/* low_delay */

	BitstreamPutBit(bs, 0);	/* vbv_parameters (0=not given) */

	BitstreamPutBits(bs, 0, 2);	/* video_object_layer_shape (0=rectangular) */

	WRITE_MARKER();

	/*
	 * time_inc_resolution; ignored by current decore versions
	 * eg. 2fps     res=2       inc=1
	 *     25fps    res=25      inc=1
	 *     29.97fps res=30000   inc=1001
	 */
	BitstreamPutBits(bs, pParam->fbase, 16);

	WRITE_MARKER();

//    if (pParam->fincr>0)
	{
	    BitstreamPutBit(bs, 1);		/* fixed_vop_rate = 1 */
	    BitstreamPutBits(bs, pParam->fincr, log2bin(pParam->fbase-1));	/* fixed_vop_time_increment */
    }
//	else{
//        BitstreamPutBit(bs, 0);		/* fixed_vop_rate = 0 */
//    }

	WRITE_MARKER();
	BitstreamPutBits(bs, pParam->width, 13);	/* width */
	WRITE_MARKER();
	BitstreamPutBits(bs, pParam->height, 13);	/* height */
	WRITE_MARKER();

	BitstreamPutBit(bs, 0);	/* interlace */
	BitstreamPutBit(bs, 1);		/* obmc_disable (overlapped block motion compensation) */
	BitstreamPutBit(bs, 0);		/* sprite_enable==off */

	BitstreamPutBit(bs, 0);		/* not_8_bit */

	/* quant_type   0=h.263  1=mpeg4(quantizer tables) */
	BitstreamPutBit(bs, 0);
	BitstreamPutBit(bs, 1);		/* complexity_estimation_disable */
	BitstreamPutBit(bs, 1);		/* resync_marker_disable */
	BitstreamPutBit(bs, 0);		/* data_partitioned */
	BitstreamPutBit(bs, 0);		/* scalability */
	BitstreamPadAlways(bs); /* next_start_code(); */
	{
		unsigned char xvid_id_string[10]="Thakral-it";
		BitstreamWriteUserData(bs, xvid_id_string, 10);//
	}
}


/*
  write vop header
*/
void
BitstreamWriteVopHeader(
						Bitstream * const bs,
						const MBParam * pParam,
						const FRAMEINFO * const frame,
						int vop_coded,
						unsigned int quant)
{
	uint32_t i;

	/*
	 * no padding here, anymore. You have to make sure that you are
	 * byte aligned, and that always 1-8 padding bits have been written
	 */

	BitstreamPutBits(bs, VOP_START_CODE, 32);

	BitstreamPutBits(bs, frame->coding_type, 2);

	for (i = 0; i < frame->seconds; i++) {
		BitstreamPutBit(bs, 1);
	}
	BitstreamPutBit(bs, 0);

	WRITE_MARKER();

	/* time_increment: value=nth_of_sec, nbits = log2(resolution) */

	BitstreamPutBits(bs, frame->ticks, log2bin(pParam->fbase-1));

	WRITE_MARKER();
#if 0
	if (!vop_coded) {
		BitstreamPutBits(bs, 0, 1);
		/* NB: It's up to the function caller to write the next_start_code().
		 * At the moment encoder.c respects that requisite because a VOP
		 * always ends with a next_start_code either if it's coded or not
		 * and encoder.c terminates a frame with a next_start_code in whatever
		 * case */
		return;
	}
#endif
	BitstreamPutBits(bs, 1, 1);	/* vop_coded */

    // to be fixed. ÓëmcÓÐ¹Ø
	if (frame->coding_type == P_VOP)// || (frame->coding_type == S_VOP)
	{
		// BitstreamPutBits(bs, frame->rounding_type, 1);
		BitstreamPutBits(bs, 0, 1);
    }
    
	BitstreamPutBits(bs, 0, 3);	/* intra_dc_vlc_threshold */
	BitstreamPutBits(bs, quant, 5);	/* quantizer */

	if (frame->coding_type != I_VOP)
		BitstreamPutBits(bs, frame->fcode, 3);	/* forward_fixed_code */
}

void
BitstreamWriteUserData(Bitstream * const bs,
						uint8_t * data,
						const int length)
{
	int i;
	BitstreamPad(bs);
	BitstreamPutBits(bs, USERDATA_START_CODE, 32);
	for (i = 0; i < length; i++) {
		BitstreamPutBits(bs, data[i], 8);
	}

}

void myBitstreamPutBits(Bitstream * const bs,
				 const uint32_t value,
				 const uint32_t size)
{
#if 0
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
		//bs->buf = 0;
		bs->pos = shift;
		bs->buf = value << (32 - shift);
	} 
	if (mask&&!oppos){
		b = bs->buf;
		BSWAP(b);
		*bs->tail++ = b;
		bs->buf = bs->pos = 0;
	}	
#else
	int b,mask,oppos,op;
	int pos,buf,A_pos,A_buf;;
	
	mask = 0;oppos = 0;
	pos = bs->pos;
	buf = bs->buf;
	op = pos + size;
	if (op<=32){
		A_buf = buf | (value << (32 - op));
		A_pos = pos + size;
		oppos = A_pos-32;
		mask = 1;
	} 
	else{
		int shift = op - 32;
		A_buf = buf |( value >> (op - 32));
		b = BSWAP(A_buf);
		*bs->tail++ = b;
		A_pos = shift;
		A_buf = value << (32 - shift);
	} 
	if (mask&&!oppos){
		b = BSWAP(A_buf);
		*bs->tail++ = b;
		A_buf = A_pos = 0;
	}
	bs->pos=A_pos;
	bs->buf=A_buf;
/*
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
*/
#endif
}
