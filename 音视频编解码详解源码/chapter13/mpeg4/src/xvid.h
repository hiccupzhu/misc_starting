/*****************************************************************************
 *
 * XVID MPEG-4 VIDEO CODEC
 * - XviD Main header file -
 *
 *  Copyright(C) 2001-2003 Peter Ross <pross@xvid.org>
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

#ifndef _XVID_H_
#define _XVID_H_


#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * versioning
 ****************************************************************************/

#define XVID_MAKE_VERSION(a,b,c) ((((a)&0xff)<<16) | (((b)&0xff)<<8) | ((c)&0xff))

#define XVID_VERSION             XVID_MAKE_VERSION(1,0,-124)


/*****************************************************************************
 * error codes
 ****************************************************************************/

	/*	all functions return values <0 indicate error */

#define XVID_ERR_FAIL		-1		/* general fault */
#define XVID_ERR_MEMORY		-2		/* memory allocation error */
#define XVID_ERR_FORMAT		-3		/* file format error */
#define XVID_ERR_VERSION	-4		/* structure version not supported */
#define XVID_ERR_END		-5		/* encoder only; end of stream reached */


/*****************************************************************************
 * xvid_image_t
 ****************************************************************************/

/* colorspace values */

#define XVID_CSP_PLANAR   (1<< 0) /* 4:2:0 planar (==I420, except for pointers/strides) */
#define XVID_CSP_I420     (1<< 1) /* 4:2:0 planar */

#define XVID_CSP_NULL     (1<<14) /* decoder only: dont output anything */
#define XVID_CSP_VFLIP    (1<<31) /* vertical flip mask */

/* xvid_image_t
	for non-planar colorspaces use only plane[0] and stride[0]
	four plane reserved for alpha*/
typedef struct {
	int csp;				/* [in] colorspace; or with XVID_CSP_VFLIP to perform vertical flip */
	void * plane[4];		/* [in] image plane ptrs */
	int stride[4];			/* [in] image stride; "bytes per row"*/
} xvid_image_t;

/* video-object-sequence profiles */
#define XVID_PROFILE_AS_L4   0xf4

/* aspect ratios */
#define XVID_PAR_11_VGA    1 /* 1:1 vga (square), default if supplied PAR is not a valid value */
#define XVID_PAR_EXT      15 /* extended par; use par_width, par_height */

/* frame type flags */
#define XVID_TYPE_VOL     -1 /* decoder only: vol was decoded */
#define XVID_TYPE_NOTHING  0 /* decoder only (encoder stats): nothing was decoded/encoded */
#define XVID_TYPE_AUTO     0 /* encoder: automatically determine coding type */
#define XVID_TYPE_IVOP     1 /* intra frame */
#define XVID_TYPE_PVOP     2 /* predicted frame */
#define XVID_TYPE_BVOP     3 /* bidirectionally encoded */
#define XVID_TYPE_SVOP     4 /* predicted+sprite frame */

//#define XVID_GLOBAL_EXTRASTATS_ENABLE (1<<2)

/*****************************************************************************
 * xvid_global()
 ****************************************************************************/

/* cpu_flags definitions (make sure to sync this with cpuid.asm for ia32) */

//#define XVID_CPU_FORCE    (1<<31) /* force passed cpu flags */
//#define XVID_CPU_ASM      (1<< 7) /* native assembly */

#define XVID_DEBUG_COEFF     (1<< 5)
#define XVID_DEBUG_MV        (1<< 6)
#define XVID_DEBUG_DEBUG     (1<<31)
//extern int xvid_global(void *handle, int opt, void *param1, void *param2);

/*----------------------------------------------------------------------------
 * xvid_enc_stats_t structure
 *
 * Used in:
 *  - xvid_plg_data_t structure
 *  - optional parameter in xvid_encore() function
 *
 * .coding_type = XVID_TYPE_NOTHING if the stats are not given
 *--------------------------------------------------------------------------*/

typedef struct {
	int version;

	/* encoding parameters */
	int type;      /* [out] coding type */
	int quant;     /* [out] frame quantizer */
	int vol_flags; /* [out] vol flags (see above) */
	int vop_flags; /* [out] vop flags (see above) */

	/* bitrate */
	int length;    /* [out] frame length */

//	int hlength;   /* [out] header length (bytes) */
	int kblks;     /* [out] number of blocks compressed as Intra */
	int mblks;     /* [out] number of blocks compressed as Inter */
	int ublks;     /* [out] number of blocks marked as not_coded */

	int sse_y;     /* [out] Y plane's sse */
	int sse_u;     /* [out] U plane's sse */
	int sse_v;     /* [out] V plane's sse */
} xvid_enc_stats_t;

/*****************************************************************************
  xvid plugin system -- internals

  xvidcore will call XVID_PLG_INFO and XVID_PLG_CREATE during XVID_ENC_CREATE
  before encoding each frame xvidcore will call XVID_PLG_BEFORE
  after encoding each frame xvidcore will call XVID_PLG_AFTER
  xvidcore will call XVID_PLG_DESTROY during XVID_ENC_DESTROY
 ****************************************************************************/
#define XVID_PLG_AFTER   (1<<5)

/* xvid_plg_info_t.flags */
#define XVID_REQORIGINAL (1<<0) /* plugin requires a copy of the original (uncompressed) image */
#define XVID_REQPSNR     (1<<1) /* plugin requires psnr between the uncompressed and compressed image*/
#define XVID_REQDQUANTS  (1<<2) /* plugin requires access to the dquant table */

/*----------------------------------------------------------------------------
 * Encoder operations
 *--------------------------------------------------------------------------*/

#define XVID_ENC_CREATE  0 /* create encoder instance; returns 0 on success */
#define XVID_ENC_DESTROY 1 /* destroy encoder instance; returns 0 on success */
#define XVID_ENC_ENCODE  2 /* encode a frame: returns number of ouput bytes
                            * 0 means this frame should not be written (ie. encoder lag) */
/*----------------------------------------------------------------------------
 * Encoder entry point
 *--------------------------------------------------------------------------*/

extern int xvid_encore(void *handle, int opt, void *param1, void *param2);



//#define XVID_GLOBAL_PACKED            (1<<0) /* packed bitstream */
//#define XVID_GLOBAL_CLOSED_GOP        (1<<1) /* closed_gop:	was DX50BVOP dx50 bvop compatibility */
//#define XVID_GLOBAL_EXTRASTATS_ENABLE (1<<2)

/*----------------------------------------------------------------------------
 * "VOL" flags
 *
 * These flags are used for xvid_enc_frame_t->vol_flags field during frame
 * encoding (operation XVID_ENC_ENCODE)
 *--------------------------------------------------------------------------*/

#define XVID_VOL_MPEGQUANT      (1<<0) /* enable MPEG type quantization */
#define XVID_VOL_EXTRASTATS     (1<<1) /* enable plane sse stats */
#define XVID_VOL_QUARTERPEL     (1<<2) /* enable quarterpel: frames will encoded as quarterpel */
#define XVID_VOL_GMC            (1<<3) /* enable GMC; frames will be checked for gmc suitability */
#define XVID_VOL_REDUCED_ENABLE (1<<4) /* enable reduced resolution vops: frames will be checked for rrv suitability */
#define XVID_VOL_INTERLACING    (1<<5) /* enable interlaced encoding */

/* Always valid */
#define XVID_VOP_DEBUG                (1<< 0) /* print debug messages in frames */
#define XVID_VOP_HALFPEL              (1<< 1) /* use halfpel interpolation */
#define XVID_VOP_INTER4V              (1<< 2) /* use 4 motion vectors per MB */

/* only valid for vol_flags|=XVID_VOL_REDUCED_ENABLED */
#define XVID_VOP_REDUCED              (1<<11) /* reduced resolution vop */


/*----------------------------------------------------------------------------
 * "Motion" flags
 *
 * These flags are used for xvid_enc_frame_t->motion field during frame
 * encoding (operation XVID_ENC_ENCODE)
 *--------------------------------------------------------------------------*/

/* Motion Estimation Search Patterns */
#define XVID_ME_ADVANCEDDIAMOND16     (1<< 0) /* use advdiamonds instead of diamonds as search pattern */
//#define XVID_ME_ADVANCEDDIAMOND8      (1<< 1) /* use advdiamond for XVID_ME_EXTSEARCH8 */
//#define XVID_ME_USESQUARES16          (1<< 2) /* use squares instead of diamonds as search pattern */
//#define XVID_ME_USESQUARES8           (1<< 3) /* use square for XVID_ME_EXTSEARCH8 */

/* SAD operator based flags */

#define XVID_ME_HALFPELREFINE16       (1<< 4)

/*----------------------------------------------------------------------------
 * xvid_enc_create_t structure definition
 *
 * This structure is passed as param1 during an instance creation (operation
 * XVID_ENC_CREATE)
 *--------------------------------------------------------------------------*/

typedef struct {
	//int version;

	int profile;                 /* [in] profile@level; refer to XVID_PROFILE_xxx */
	int width;                   /* [in] frame dimensions; width, pixel units */
	int height;                  /* [in] frame dimensions; height, pixel units */

	int global;                  /* [in:opt] global flags; controls encoding behavior */

	/* --- vol-based stuff; included here for convenience */
	int fincr;                   /* [in:opt] framerate increment; set to zero for variable framerate */
	int fbase;                   /* [in] framerate base frame_duration = fincr/fbase seconds*/

	int max_key_interval;        /* [in:opt] the maximum interval between key frames */

	int frame_drop_ratio;        /* [in:opt] frame dropping: 0=drop none... 100=drop all */

	void *handle;                /* [out] encoder instance handle */
} xvid_enc_create_t;


/*----------------------------------------------------------------------------
 * xvid_enc_frame_t structure definition
 *
 * This structure is passed as param1 during a frame encoding (operation
 * XVID_ENC_ENCODE)
 *--------------------------------------------------------------------------*/

/* out value for the frame structure->type field
 * unlike stats output in param2, this field is not asynchronous and tells
 * the client app, if the frame written into the stream buffer is an ivop
 * usually used for indexing purpose in the container */
#define XVID_KEYFRAME (1<<1)

/* The structure */
typedef struct {
	//int version;
	int vol_flags;                     /* [in] vol flags */

	int par;                           /* [in:opt] pixel aspect ratio (refer to XVID_PAR_xxx above) */
	int par_width;                     /* [in:opt] aspect ratio width */
	int par_height;                    /* [in:opt] aspect ratio height */

	/* Other fields that can change on a frame base */

	int fincr;                         /* [in:opt] framerate increment, for variable framerate only */
	int vop_flags;                     /* [in] (general)vop-based flags */
	int motion;                        /* [in] ME options */

	xvid_image_t input;                /* [in] input image (read from) */

	int type;                          /* [in:opt] coding type */
	int quant;                         /* [in] frame quantizer; if <=0, automatic (ratecontrol) */
//	int bframe_threshold;

	void *bitstream;                   /* [in:opt] bitstream ptr (written to)*/
	int length;                        /* [in:opt] bitstream length (bytes) */

	int out_flags;                     /* [out] bitstream output flags */
} xvid_enc_frame_t;

#ifdef __cplusplus
}
#endif

#endif
