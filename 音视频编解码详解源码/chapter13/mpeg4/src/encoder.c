/*****************************************************************************
 *
 *  XVID MPEG-4 VIDEO CODEC
 *  - Encoder main module -
 *
 *  Copyright(C) 2002	  Michael Militzer <isibaar@xvid.org>
 *			   2002-2003 Peter Ross <pross@xvid.org>
 *			   2002	  Daniel Smith <danielsmith@astroboymail.com>
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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "encoder.h"
#include "prediction/mbprediction.h"
#include "global.h"
#include "image/interpolate8x8.h"
#include "image/image.h"
#include "motion/sad.h"
#include "motion/motion.h"

//#include "bitstream/cbp.h"
#include "utils/mbfunctions.h"
#include "bitstream/bitstream.h"
#include "bitstream/mbcoding.h"
#include "bitstream/mbcoding.h"
#include "utils/mem_align.h"

#include "dct/idct.h"
#include "dct/fdct.h"

/*****************************************************************************
 * Local function prototypes
 ****************************************************************************/
LUMA_MC_T luma_mc[4];

static int FrameCodeI(Encoder * pEnc,
					  Bitstream * bs);

static int FrameCodeP(Encoder * pEnc,
					  Bitstream * bs,
					  int force_inter,
					  int vol_header);

/*****************************************************************************
 * Encoder creation
 *
 * This function creates an Encoder instance, it allocates all necessary
 * image buffers (reference, current and bframes) and initialize the internal
 * xvid encoder paremeters according to the XVID_ENC_PARAM input parameter.
 *
 * The code seems to be very long but is very basic, mainly memory allocation
 * and cleaning code.
 *
 * Returned values :
 *	- 0				- no errors
 *	- XVID_ERR_MEMORY - the libc could not allocate memory, the function
 *						cleans the structure before exiting.
 *						pParam->handle is also set to NULL.
 *
 ****************************************************************************/

/*
 * Simplify the "fincr/fbase" fraction
*/
#if 0
static void
simplify_time(int *inc, int *base)
{
	/* common factor */
	int i = *inc;
	while (i > 1) {
		if (*inc % i == 0 && *base % i == 0) {
			*inc /= i;
			*base /= i;
			i = *inc;
			continue;
		}
		i--;
	}

	/* if neccessary, round to 65535 accuracy */
	if (*base > 65535) {
		float div = (float) *base / 65535;
		*base = (int) (*base / div);
		*inc = (int) (*inc / div);
	}
}
#endif

Encoder *pEnc;
extern unsigned int xvid_debug;
int
enc_create(xvid_enc_create_t * create)
{
	//Encoder *pEnc;
	//int n;
#if 0
    luma_mc[MC_16X16_COPY] = mc16x16_hpel_copy_asm_dm642;
    luma_mc[MC_16X16_V] = mc16x16_hpel_v_asm_dm642;
    luma_mc[MC_16X16_H] = mc16x16_hpel_h_asm_dm642;
    luma_mc[MC_16X16_COPY] = mc16x16_hpel_copy_c;
#else
    //luma_mc[MC_16X16_V] = mc16x16_hpel_v_asm_dm642;
    //luma_mc[MC_16X16_H] = mc16x16_hpel_h_asm_dm642;
#endif

    // luma_mc[MC_16X16_V] = mc16x16_hpel_v_c;
    // luma_mc[MC_16X16_H] = mc16x16_hpel_h_c;

	// idct_int32_init();
	init_vlc_tables();

	/* allocate encoder struct */
	pEnc = (Encoder *) xvid_malloc(sizeof(Encoder), CACHE_LINE);
	if (pEnc == NULL)
		return XVID_ERR_MEMORY;
	memset(pEnc, 0, sizeof(Encoder));

	pEnc->mbParam.profile = create->profile;

	/* global flags */
	pEnc->mbParam.global_flags = create->global;

	/* width, height */
	pEnc->mbParam.width = create->width;
	pEnc->mbParam.height = create->height;
	pEnc->mbParam.mb_width = (pEnc->mbParam.width + 15) / 16;
	pEnc->mbParam.mb_height = (pEnc->mbParam.height + 15) / 16;
	pEnc->mbParam.edged_width = 16 * pEnc->mbParam.mb_width + 2 * EDGE_SIZE;
	pEnc->mbParam.edged_height = 16 * pEnc->mbParam.mb_height + 2 * EDGE_SIZE;

	/* framerate */
	pEnc->mbParam.fincr = MAX(create->fincr, 0);
	pEnc->mbParam.fbase = create->fincr <= 0 ? 25 : create->fbase;
//	if (pEnc->mbParam.fincr>0)
//		simplify_time(&pEnc->mbParam.fincr, &pEnc->mbParam.fbase);

	/* frame drop ratio */
	pEnc->mbParam.frame_drop_ratio = MAX(create->frame_drop_ratio, 0);

	/* max keyframe interval */
	pEnc->mbParam.iMaxKeyInterval = create->max_key_interval <= 0 ? (10 * (int)pEnc->mbParam.fbase) / (int)pEnc->mbParam.fincr : create->max_key_interval;

	/* allocate working frame-image memory */

	pEnc->current = xvid_malloc(sizeof(FRAMEINFO), CACHE_LINE);
	pEnc->reference = xvid_malloc(sizeof(FRAMEINFO), CACHE_LINE);

	if (pEnc->current == NULL || pEnc->reference == NULL)
		goto xvid_err_memory1;

	/* allocate macroblock memory */

	pEnc->current->mbs =
		xvid_malloc(sizeof(MACROBLOCK) * pEnc->mbParam.mb_width *
					pEnc->mbParam.mb_height, CACHE_LINE);
	pEnc->reference->mbs =
		xvid_malloc(sizeof(MACROBLOCK) * pEnc->mbParam.mb_width *
					pEnc->mbParam.mb_height, CACHE_LINE);

	if (pEnc->current->mbs == NULL || pEnc->reference->mbs == NULL)
		goto xvid_err_memory2;

	/* allocate quant matrix memory */

	pEnc->mbParam.mpeg_quant_matrices = 0;//
	/* allocate interpolation image memory */

    image_null(&pEnc->sOriginal); // for psnr
    
    image_null(&pEnc->postproc_img);
    
	image_null(&pEnc->current->image);
	image_null(&pEnc->reference->image);
	image_null(&pEnc->vInterH);
	image_null(&pEnc->vInterV);
	image_null(&pEnc->vInterHV);


	if (image_create
		(&pEnc->current->image, pEnc->mbParam.edged_width,
		 pEnc->mbParam.edged_height) < 0)
		goto xvid_err_memory3;
	if (image_create
		(&pEnc->reference->image, pEnc->mbParam.edged_width,
		 pEnc->mbParam.edged_height) < 0)
		goto xvid_err_memory3;
	if (image_create
		(&pEnc->vInterH, pEnc->mbParam.edged_width,
		 pEnc->mbParam.edged_height) < 0)
		goto xvid_err_memory3;
	if (image_create
		(&pEnc->vInterV, pEnc->mbParam.edged_width,
		 pEnc->mbParam.edged_height) < 0)
		goto xvid_err_memory3;

	if (image_create
		(&pEnc->vInterHV, pEnc->mbParam.edged_width,
		 pEnc->mbParam.edged_height) < 0)
		goto xvid_err_memory3;
    
    if (image_create
			(&pEnc->sOriginal, pEnc->mbParam.edged_width,
			 pEnc->mbParam.edged_height) < 0)
			goto xvid_err_memory3;
    if (image_create
    (&pEnc->postproc_img, pEnc->mbParam.edged_width,
    pEnc->mbParam.edged_height) < 0)
    goto xvid_err_memory3;
    
	/* init bframe image buffers */
	/* B Frames specific init */
	/* init incoming frame queue */
	pEnc->queue_head = 0;
	pEnc->queue_tail = 0;
	pEnc->queue_size = 0;

	pEnc->queue =
		xvid_malloc((1) * sizeof(QUEUEINFO),
					CACHE_LINE);


	//for (n = 0; n < 1; n++)
	//	image_null(&pEnc->queue[n].image);


	//for (n = 0; n < 1; n++)
	{
		if (image_create
			(&pEnc->queue[0].image, pEnc->mbParam.edged_width,
			 pEnc->mbParam.edged_height) < 0)
			goto xvid_err_memory5;
	}

	/* timestamp stuff */

	pEnc->mbParam.m_stamp = 0;
//	pEnc->m_framenum = 0;
	pEnc->current->stamp = 0;
	pEnc->reference->stamp = 0;

	/* other stuff */

	pEnc->iFrameNum = 0;
	pEnc->fMvPrevSigma = -1;

	create->handle = (void *) pEnc;

	return 0;   /* ok */

	/*
	 * We handle all XVID_ERR_MEMORY here, this makes the code lighter
	 */

  xvid_err_memory5:

	//for (n = 0; n < 1; n++)
	{
		image_destroy(&pEnc->queue[0].image, pEnc->mbParam.edged_width,
					  pEnc->mbParam.edged_height);
	}

	xvid_free(pEnc->queue);

    image_destroy(&pEnc->sOriginal, pEnc->mbParam.edged_width,
					  pEnc->mbParam.edged_height);
    image_destroy(&pEnc->postproc_img, pEnc->mbParam.edged_width,
					  pEnc->mbParam.edged_height);
					  					  
	image_destroy(&pEnc->current->image, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
	image_destroy(&pEnc->reference->image, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
	image_destroy(&pEnc->vInterH, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
	image_destroy(&pEnc->vInterV, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
	image_destroy(&pEnc->vInterHV, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);

  xvid_err_memory2:
	xvid_free(pEnc->current->mbs);
	xvid_free(pEnc->reference->mbs);

  xvid_err_memory1:
	xvid_free(pEnc->current);
	xvid_free(pEnc->reference);

	xvid_free(pEnc);
xvid_err_memory3:

	create->handle = NULL;

	return XVID_ERR_MEMORY;
}

/*****************************************************************************
 * Encoder destruction
 *
 * This function destroy the entire encoder structure created by a previous
 * successful enc_create call.
 *
 * Returned values (for now only one returned value) :
 *	- 0	 - no errors
 *
 ****************************************************************************/

int
enc_destroy(Encoder * pEnc)
{
	int i;

	/* B Frames specific */
	for (i = 0; i < 1; i++) {
		image_destroy(&pEnc->queue[i].image, pEnc->mbParam.edged_width,
					  pEnc->mbParam.edged_height);
	}

	xvid_free(pEnc->queue);

	/* All images, reference, current etc ... */

	image_destroy(&pEnc->current->image, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
	image_destroy(&pEnc->reference->image, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
	image_destroy(&pEnc->vInterH, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
	image_destroy(&pEnc->vInterV, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);
	image_destroy(&pEnc->vInterHV, pEnc->mbParam.edged_width,
				  pEnc->mbParam.edged_height);

	if ((pEnc->mbParam.plugin_flags & XVID_REQORIGINAL)) {
		image_destroy(&pEnc->sOriginal, pEnc->mbParam.edged_width,
					  pEnc->mbParam.edged_height);
	}

	/* Encoder structure */

	xvid_free(pEnc->current->mbs);
	xvid_free(pEnc->current);

	xvid_free(pEnc->reference->mbs);
	xvid_free(pEnc->reference);

	if ((pEnc->mbParam.plugin_flags & XVID_REQDQUANTS)) {
		xvid_free(pEnc->temp_dquants);
	}
	xvid_free(pEnc);

	return 0;  /* ok */
}

/*
  call the plugins
  */

static __inline void inc_frame_num(Encoder * pEnc)
{
	pEnc->current->stamp = pEnc->mbParam.m_stamp;	/* first frame is zero */
	pEnc->mbParam.m_stamp += pEnc->current->fincr;
}

static __inline void
set_timecodes(FRAMEINFO* pCur,FRAMEINFO *pRef, int32_t time_base)
{
	pCur->ticks = (int32_t)pCur->stamp % time_base;
	pCur->seconds =  ((int32_t)pCur->stamp / time_base)	- ((int32_t)pRef->stamp / time_base) ;
}

/*****************************************************************************
 * IPB frame encoder entry point
 *
 * Returned values :
 *	- >0			   - output bytes
 *	- 0				- no output
 *	- XVID_ERR_VERSION - wrong version passed to core
 *	- XVID_ERR_END	 - End of stream reached before end of coding
 *	- XVID_ERR_FORMAT  - the image subsystem reported the image had a wrong
 *						 format
 ****************************************************************************/

QUEUEINFO *q;// = &pEnc->queue[0];//pEnc->queue_tail

int
enc_encode(Encoder * pEnc,
			   xvid_enc_frame_t * xFrame,
			   xvid_enc_stats_t * stats)
{
	xvid_enc_frame_t * frame;
	int type;
		
	Bitstream bs;
	xFrame->out_flags = 0;
//	start_global_timer();
	BitstreamInit(&bs, xFrame->bitstream, 0);

	/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	 * enqueue image to the encoding-queue
	 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	if (xFrame->input.csp != XVID_CSP_NULL)
	{
		//QUEUEINFO * q = &pEnc->queue[0];//pEnc->queue_tail
		q = &pEnc->queue[0];//pEnc->queue_tail
		if (image_input
			(&q->image, pEnc->mbParam.width, pEnc->mbParam.height,
			pEnc->mbParam.edged_width, (uint8_t**)xFrame->input.plane, xFrame->input.stride,
			xFrame->input.csp, 0))
		{
			return XVID_ERR_FORMAT;
		}
		q->frame = *xFrame;

		pEnc->queue_tail = 0;//(pEnc->queue_tail + 1) % (1);
		pEnc->queue_size++;
	}

	/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	 * dequeue frame from the encoding queue
	 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	if (pEnc->queue_size == 0)		/* empty */
	{
		if (xFrame->input.csp == XVID_CSP_NULL)	/* no futher input */
		{
			return XVID_ERR_END;	/* end of stream reached */
		}
		goto done;	/* nothing to encode yet; encoder lag */
	}

	/* the current FRAME becomes the reference */
	SWAP(FRAMEINFO*, pEnc->current, pEnc->reference);

	/* remove frame from encoding-queue (head), and move it into the current */
	image_swap(&pEnc->current->image, &pEnc->queue[0].image);
	frame = &pEnc->queue[0].frame;
	pEnc->queue_head = 0;//(pEnc->queue_head + 1) % (1);
	pEnc->queue_size--;


	/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	 * init pEnc->current fields
	 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	pEnc->current->fincr = pEnc->mbParam.fincr>0 ? pEnc->mbParam.fincr : frame->fincr;
	inc_frame_num(pEnc);
	pEnc->current->vol_flags = frame->vol_flags;
	pEnc->current->vop_flags = frame->vop_flags;
	pEnc->current->motion_flags = frame->motion;
	pEnc->current->fcode = pEnc->mbParam.m_fcode;

	/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	 * frame type & quant selection
	 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

	type = frame->type;
	pEnc->current->quant = frame->quant;

	if (type > 0){ 	/* XVID_TYPE_?VOP */
		type = type2coding(type);	/* convert XVID_TYPE_?VOP to bitstream coding type */
	} else{		/* XVID_TYPE_AUTO */
		if (pEnc->iFrameNum == 0 || (pEnc->mbParam.iMaxKeyInterval > 0 && pEnc->iFrameNum >= pEnc->mbParam.iMaxKeyInterval)){
			pEnc->iFrameNum = 0;
			type = I_VOP;
		}else{
			type = B_VOP;
		}
	}

	if (type != I_VOP) 
		pEnc->current->vol_flags = pEnc->mbParam.vol_flags; /* don't allow VOL changes here */

	/* bframes buffer overflow check */
	if (type == B_VOP) {
		type = P_VOP;
	}

	pEnc->iFrameNum++;
    
#ifndef DISABLE_PSNR
		image_copy(&pEnc->sOriginal, &pEnc->current->image,
				   pEnc->mbParam.edged_width, pEnc->mbParam.height);
#endif
    
	if (type == I_VOP) {

		pEnc->iFrameNum = 1;

		/* ---- update vol flags at IVOP ----------- */
		pEnc->mbParam.vol_flags = pEnc->current->vol_flags;

		pEnc->mbParam.par = XVID_PAR_11_VGA;

		FrameCodeI(pEnc, &bs);
		xFrame->out_flags |= XVID_KEYFRAME;

	} else { /* (type == P_VOP || type == S_VOP) */
		FrameCodeP(pEnc, &bs, 1, 0);
	}

#ifndef DISABLE_PSNR
    if (0)
    {
            int nDiffY = 0;
            int nDiffU = 0;
            int nDiffV = 0;
            
            nDiffY = plane_sse(pEnc->sOriginal.y, pEnc->current->image.y,
    						   pEnc->mbParam.edged_width, pEnc->mbParam.width,
    						   pEnc->mbParam.height);
            
    		nDiffU = plane_sse(pEnc->sOriginal.u, pEnc->current->image.u,
    						   pEnc->mbParam.edged_width/2, pEnc->mbParam.width/2,
    						   pEnc->mbParam.height/2);
            
    	    nDiffV = plane_sse(pEnc->sOriginal.v, pEnc->current->image.v,
    						   pEnc->mbParam.edged_width/2, pEnc->mbParam.width/2,
    						   pEnc->mbParam.height/2);
            
       		printf("sse_y=%d sse_u=%d sse_v=%d \n",nDiffY, nDiffU, nDiffV);	
    #define SSE2PSNR(sse, width, height) ((!(sse))?0.0f : 48.131f - 10*(float)log10((float)(sse)/((float)((width)*(height)))))
			printf("\npsnr y = %2.2f, psnr u = %2.2f, psnr v = %2.2f\n",
					   SSE2PSNR(nDiffY, pEnc->mbParam.width, pEnc->mbParam.height), 
					   SSE2PSNR(nDiffU, pEnc->mbParam.width / 2, pEnc->mbParam.height / 2),
					   SSE2PSNR(nDiffV, pEnc->mbParam.width / 2, pEnc->mbParam.height / 2));
    #undef SSE2PSNR     
    }
#endif   

	if (0)
	{
        IMAGE img;

        img.y = pEnc->current->image.y;
        img.u = pEnc->current->image.u;
        img.v = pEnc->current->image.v;
        
        image_copy(&pEnc->postproc_img, &img, pEnc->mbParam.edged_width, pEnc->mbParam.height);
	 /*	
        image_postproc(&postproc, 
                        &pEnc->postproc_img, 
                       pEnc->mbParam.edged_width, 
					   pEnc->current->mbs, 
					   pEnc->mbParam.mb_width, 
					   pEnc->mbParam.mb_height, 
					   pEnc->mbParam.mb_width,
					   flag, 0, 0);
    	  */     
        image_dump_yuv420(&pEnc->postproc_img, pEnc->mbParam.edged_width, pEnc->mbParam.width, pEnc->mbParam.height, "xvid_recon.yuv");
    } 
    
#if 1
	stats->type			= pEnc->current->coding_type+1;//coding2type(pEnc->current->coding_type);
	stats->quant		= pEnc->current->quant;
	stats->vol_flags	= pEnc->current->vol_flags;
	stats->vop_flags	= pEnc->current->vop_flags;
	stats->length		= pEnc->current->length;
	stats->kblks		= pEnc->current->sStat.kblks;
	stats->mblks		= pEnc->current->sStat.mblks;
	stats->ublks		= pEnc->current->sStat.ublks;
#endif	

done:
	return BitstreamLength(&bs);

}


static void SetMacroblockQuants(MBParam * const pParam, FRAMEINFO * frame)
{
	unsigned int i;
	MACROBLOCK * pMB = frame->mbs;
	int quant = frame->quant;//frame->mbs[0].quant; /* set by XVID_PLG_FRAME */

	for (i = 0; i < pParam->mb_height * pParam->mb_width; i++) {
		pMB->quant = quant;
		pMB++;
	}
}


static __inline void
CodeIntraMB(Encoder * pEnc,
			MACROBLOCK * pMB)
{
	pMB->mode = MODE_INTRA;
    
	pMB->mvs[0].x = pMB->mvs[1].x = pMB->mvs[2].x = pMB->mvs[3].x = 0;
	pMB->mvs[0].y = pMB->mvs[1].y = pMB->mvs[2].y = pMB->mvs[3].y = 0;
	pMB->sad8[0] =0; // pMB->sad8[1] = pMB->sad8[2] = pMB->sad8[3] = 0; // for inter_4v
	pMB->sad16 = 0;
	
}

far uint8_t yIn[416*16+64];
far uint8_t uIn[416/2*8+32];
far	uint8_t vIn[416/2*8+32];

static int
FrameCodeI(Encoder * pEnc,
		   Bitstream * bs)
{
	int bits = BitstreamPos(bs);
	int mb_width = pEnc->mbParam.mb_width;
	int mb_height = pEnc->mbParam.mb_height;
//	int edged_width = pEnc->mbParam.edged_width;

		
	DECLARE_ALIGNED_MATRIX(dct_codes, 6, 64, int16_t, CACHE_LINE);
	DECLARE_ALIGNED_MATRIX(qcoeff, 6, 64, int16_t, CACHE_LINE);

	uint16_t x, y;
	
	pEnc->mbParam.m_rounding_type = 1;
	pEnc->current->rounding_type = pEnc->mbParam.m_rounding_type;
	pEnc->current->coding_type = I_VOP;

	pEnc->current->mbs[0].quant = pEnc->current->quant;

	SetMacroblockQuants(&pEnc->mbParam, pEnc->current);

	BitstreamWriteVolHeader(bs, &pEnc->mbParam, pEnc->current);

	set_timecodes(pEnc->current,pEnc->reference,pEnc->mbParam.fbase);

	BitstreamPad(bs);

	BitstreamWriteVopHeader(bs, &pEnc->mbParam, pEnc->current, 1, pEnc->current->mbs[0].quant);

	pEnc->current->sStat.iTextBits = 0;
	pEnc->current->sStat.kblks = mb_width * mb_height;
	pEnc->current->sStat.mblks = pEnc->current->sStat.ublks = 0;

	for (y = 0; y < mb_height; y++)
		for (x = 0; x < mb_width; x++) {
			MACROBLOCK *pMB =
				&pEnc->current->mbs[x + y * pEnc->mbParam.mb_width];
			CodeIntraMB(pEnc, pMB);
			MBTransQuantIntra(&pEnc->mbParam, pEnc->current, pMB, x, y,
							  dct_codes, qcoeff);
			MBPrediction(pEnc->current, x, y, pEnc->mbParam.mb_width, qcoeff);
			MBCoding(pEnc->current, pMB, qcoeff, bs, &pEnc->current->sStat);
		}

	BitstreamPadAlways(bs); /* next_start_code() at the end of VideoObjectPlane() */

	pEnc->current->length = (BitstreamPos(bs) - bits) / 8;

	pEnc->fMvPrevSigma = -1;
	pEnc->mbParam.m_fcode = 2;

	pEnc->current->is_edged = 0; /* not edged */
	pEnc->current->is_interpolated = -1; /* not interpolated (fake rounding -1) */

	return 1;					/* intra */
}

//#define INTRA_THRESHOLD 0.5


extern far uint8_t yInRef[416*16*3+64];
extern far uint8_t uInRef[416/2*8*3+32];
extern far uint8_t vInRef[416/2*8*3+32];

/* FrameCodeP also handles S(GMC)-VOPs */
static int
FrameCodeP(Encoder * pEnc,
		   Bitstream * bs,
		   int force_inter,
		   int vol_header)
{
	float fSigma;
	int bits = BitstreamPos(bs);
	//IMAGE CurImage,RefImage,SrcCur,SrcRef;
	

	DECLARE_ALIGNED_MATRIX(dct_codes, 6, 64, int16_t, CACHE_LINE);
	DECLARE_ALIGNED_MATRIX(qcoeff, 6, 64, int16_t, CACHE_LINE);

	int iLimit;
	int x, y;//, k;
	int iSearchRange;
	int bIntra=0, skip_possible;
	FRAMEINFO *const current = pEnc->current;
	FRAMEINFO *const reference = pEnc->reference;
	MBParam * const pParam = &pEnc->mbParam;
	int mb_width = pParam->mb_width;
	int mb_height = pParam->mb_height;
	
	/* IMAGE *pCurrent = &current->image; */
	IMAGE *pRef = &reference->image;

	if (!reference->is_edged) {	
		image_setedges(pRef, pParam->edged_width, pParam->edged_height,
					   pParam->width, pParam->height, 0);
		reference->is_edged = 1;
	}

	pParam->m_rounding_type = 1 - pParam->m_rounding_type;
	current->rounding_type = pParam->m_rounding_type;
	current->fcode = pParam->m_fcode;

	iLimit = mb_width * mb_height + 1;//

	{
		if (reference->is_interpolated != current->rounding_type) 
		{
		   
			image_interpolate(pRef, &pEnc->vInterH, &pEnc->vInterV,
							  &pEnc->vInterHV, pParam->edged_width,
							  pParam->edged_height,
							  0,//(pParam->vol_flags & XVID_VOL_QUARTERPEL),
							  current->rounding_type);
            
			reference->is_interpolated = current->rounding_type;
		}
	}

	current->coding_type = P_VOP;

	pEnc->current->mbs[0].quant = pEnc->current->quant; /* FRAME will not affect the quant in stats */

	SetMacroblockQuants(&pEnc->mbParam, current);

	bIntra =
		MotionEstimation(&pEnc->mbParam, current, reference,
					 &pEnc->vInterH, &pEnc->vInterV, &pEnc->vInterHV,
					 iLimit);

	if (bIntra == 1) return FrameCodeI(pEnc, bs);

	set_timecodes(current,reference,pParam->fbase);

	BitstreamWriteVopHeader(bs, &pEnc->mbParam, current, 1, current->mbs[0].quant);

	current->sStat.iTextBits = current->sStat.iMvSum = current->sStat.iMvCount =
		current->sStat.kblks = current->sStat.mblks = current->sStat.ublks = 0;

	for (y = 0; y < mb_height; y++) {
		for (x = 0; x < mb_width; x++) {
			MACROBLOCK *pMB = &current->mbs[x + y * pParam->mb_width];

			bIntra = (pMB->mode == MODE_INTRA);
			if (bIntra)
		       {
				CodeIntraMB(pEnc, pMB);
				MBTransQuantIntra(&pEnc->mbParam, current, pMB, x, y,
								  dct_codes, qcoeff);
				MBPrediction(current, x, y, pParam->mb_width, qcoeff);
				MBCoding(current, pMB, qcoeff, bs, &current->sStat);
				continue;
			}

			MBMotionCompensation(pMB, x, y, &reference->image,
								 &pEnc->vInterH, &pEnc->vInterV,
								 &pEnc->vInterHV,
								 &current->image,
								 dct_codes, 
								 pParam->width,
								 pParam->height,
								 pParam->edged_width,
								 current->rounding_type);

			if (pMB->mode != MODE_NOT_CODED)
			{	
			    pMB->cbp =	MBTransQuantInter(&pEnc->mbParam, current, pMB, x, y, dct_codes, qcoeff);
			}                  
            
#if 1
			if (pMB->cbp || pMB->mvs[0].x || pMB->mvs[0].y ||
				   pMB->mvs[1].x || pMB->mvs[1].y || pMB->mvs[2].x ||
				   pMB->mvs[2].y || pMB->mvs[3].x || pMB->mvs[3].y) {
				current->sStat.mblks++;
			}  else {
				current->sStat.ublks++;
			}
#endif
			/* Finished processing the MB, now check if to CODE or SKIP */
			skip_possible = (pMB->cbp == 0) && (pMB->mode == MODE_INTER);
			if (current->coding_type == P_VOP)
			{
				skip_possible &= ( (pMB->mvs[0].x == 0) && (pMB->mvs[0].y == 0) );
			}

			if ( (pMB->mode == MODE_NOT_CODED) || (skip_possible)) 
            {
				pMB->mode = MODE_NOT_CODED;
				MBSkip(bs);
				continue;	/* next MB */
			}

			MBCoding(current, pMB, qcoeff, bs, &pEnc->current->sStat);
		}
	}
	


	if (current->sStat.iMvCount == 0)
		current->sStat.iMvCount = 1;
/*****************************************************************************/
#if 1
	fSigma = (float) sqrt((float) current->sStat.iMvSum / current->sStat.iMvCount);

	iSearchRange = 1 << (3 + pParam->m_fcode);

	if ((fSigma > iSearchRange / 3)
		&& (pParam->m_fcode <= (3 + 0 )))	/*(pParam->vol_flags & XVID_VOL_QUARTERPEL?1:0) maximum search range 128 */
	{
		pParam->m_fcode++;
		iSearchRange *= 2;
	} else if ((fSigma < iSearchRange / 6)
			   && (pEnc->fMvPrevSigma >= 0)
			   && (pEnc->fMvPrevSigma < iSearchRange / 6)
			   && (pParam->m_fcode >= (2 + 0 )))	/*(pParam->vol_flags & XVID_VOL_QUARTERPEL?1:0) minimum search range 16 */
	{
		pParam->m_fcode--;
		iSearchRange /= 2;
	}
	pEnc->fMvPrevSigma = fSigma;
#else
	pParam->m_fcode = 2;
#endif

	/* frame drop code */

	pEnc->current->is_edged = 0; /* not edged */
	pEnc->current->is_interpolated = -1; /* not interpolated (fake rounding -1) */
	BitstreamPadAlways(bs); /* next_start_code() at the end of VideoObjectPlane() */
	current->length = (BitstreamPos(bs) - bits) / 8;
	return 0;					/* inter */
}

