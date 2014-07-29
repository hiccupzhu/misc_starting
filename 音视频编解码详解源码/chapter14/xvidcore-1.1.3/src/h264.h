/*****************************************************************************
 *
 * H264 MPEG-4 VIDEO CODEC
 * - H264 Main header file -
 *
 *  Copyright(C) 2001-2004 Peter Ross <pross@H264.org>
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
 * $Id: H264.h,v 1.51.2.5 2007/06/27 18:57:42 Isibaar Exp $
 *
 ****************************************************************************/

#ifndef _H264_H_
#define _H264_H_


#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * versioning
 ****************************************************************************/

/* versioning
	version takes the form "$major.$minor.$patch"
	$patch is incremented when there is no api change
	$minor is incremented when the api is changed, but remains backwards compatible
	$major is incremented when the api is changed significantly

	when initialising an H264 structure, you must always zero it, and set the version field.
		memset(&struct,0,sizeof(struct));
		struct.version = H264_VERSION;

	H264_UNSTABLE is defined only during development.
	*/

#define H264_MAKE_VERSION(a,b,c) ((((a)&0xff)<<16) | (((b)&0xff)<<8) | ((c)&0xff))
#define H264_VERSION_MAJOR(a)    ((char)(((a)>>16) & 0xff))
#define H264_VERSION_MINOR(a)    ((char)(((a)>> 8) & 0xff))
#define H264_VERSION_PATCH(a)    ((char)(((a)>> 0) & 0xff))

#define H264_MAKE_API(a,b)       ((((a)&0xff)<<16) | (((b)&0xff)<<0))
#define H264_API_MAJOR(a)        (((a)>>16) & 0xff)
#define H264_API_MINOR(a)        (((a)>> 0) & 0xff)

#define H264_VERSION             H264_MAKE_VERSION(1,1,3)
#define H264_API                 H264_MAKE_API(4, 1)

/* Bitstream Version
 * this will be writen into the bitstream to allow easy detection of H264
 * encoder bugs in the decoder, without this it might not possible to
 * automatically distinquish between a file which has been encoded with an
 * old & buggy H264 from a file which has been encoded with a bugfree version
 * see the infamous interlacing bug ...
 *
 * this MUST be increased if an encoder bug is fixed, increasing it too often
 * doesnt hurt but not increasing it could cause difficulty for decoders in the
 * future
 */
#define H264_BS_VERSION 46

/*****************************************************************************
 * error codes
 ****************************************************************************/

	/*	all functions return values <0 indicate error */

#define H264_ERR_FAIL		-1		/* general fault */
#define H264_ERR_MEMORY		-2		/* memory allocation error */
#define H264_ERR_FORMAT		-3		/* file format error */
#define H264_ERR_VERSION	-4		/* structure version not supported */
#define H264_ERR_END		-5		/* encoder only; end of stream reached */



/*****************************************************************************
 * H264_image_t
 ****************************************************************************/

/* colorspace values */

#define H264_CSP_PLANAR   (1<< 0) /* 4:2:0 planar (==I420, except for pointers/strides) */
#define H264_CSP_USER	  H264_CSP_PLANAR
#define H264_CSP_I420     (1<< 1) /* 4:2:0 planar */
#define H264_CSP_YV12     (1<< 2) /* 4:2:0 planar */
#define H264_CSP_YUY2     (1<< 3) /* 4:2:2 packed */
#define H264_CSP_UYVY     (1<< 4) /* 4:2:2 packed */
#define H264_CSP_YVYU     (1<< 5) /* 4:2:2 packed */
#define H264_CSP_BGRA     (1<< 6) /* 32-bit bgra packed */
#define H264_CSP_ABGR     (1<< 7) /* 32-bit abgr packed */
#define H264_CSP_RGBA     (1<< 8) /* 32-bit rgba packed */
#define H264_CSP_ARGB     (1<<15) /* 32-bit argb packed */
#define H264_CSP_BGR      (1<< 9) /* 24-bit bgr packed */
#define H264_CSP_RGB555   (1<<10) /* 16-bit rgb555 packed */
#define H264_CSP_RGB565   (1<<11) /* 16-bit rgb565 packed */
#define H264_CSP_SLICE    (1<<12) /* decoder only: 4:2:0 planar, per slice rendering */
#define H264_CSP_INTERNAL (1<<13) /* decoder only: 4:2:0 planar, returns ptrs to internal buffers */
#define H264_CSP_NULL     (1<<14) /* decoder only: dont output anything */
#define H264_CSP_VFLIP    (1<<31) /* vertical flip mask */

/* H264_image_t
	for non-planar colorspaces use only plane[0] and stride[0]
	four plane reserved for alpha*/
typedef struct {
	int csp;				/* [in] colorspace; or with H264_CSP_VFLIP to perform vertical flip */
	void * plane[4];		/* [in] image plane ptrs */
	int stride[4];			/* [in] image stride; "bytes per row"*/
} H264_image_t;

/* video-object-sequence profiles */
#define H264_PROFILE_S_L0    0x08 /* simple */
#define H264_PROFILE_S_L1    0x01
#define H264_PROFILE_S_L2    0x02
#define H264_PROFILE_S_L3    0x03
#define H264_PROFILE_ARTS_L1 0x91 /* advanced realtime simple */
#define H264_PROFILE_ARTS_L2 0x92
#define H264_PROFILE_ARTS_L3 0x93
#define H264_PROFILE_ARTS_L4 0x94
#define H264_PROFILE_AS_L0   0xf0 /* advanced simple */
#define H264_PROFILE_AS_L1   0xf1
#define H264_PROFILE_AS_L2   0xf2
#define H264_PROFILE_AS_L3   0xf3
#define H264_PROFILE_AS_L4   0x67 //f4

/* aspect ratios */
#define H264_PAR_11_VGA    1 /* 1:1 vga (square), default if supplied PAR is not a valid value */
#define H264_PAR_43_PAL    2 /* 4:3 pal (12:11 625-line) */
#define H264_PAR_43_NTSC   3 /* 4:3 ntsc (10:11 525-line) */
#define H264_PAR_169_PAL   4 /* 16:9 pal (16:11 625-line) */
#define H264_PAR_169_NTSC  5 /* 16:9 ntsc (40:33 525-line) */
#define H264_PAR_EXT      15 /* extended par; use par_width, par_height */

/* frame type flags */
#define H264_TYPE_VOL     -1 /* decoder only: vol was decoded */
#define H264_TYPE_NOTHING  0 /* decoder only (encoder stats): nothing was decoded/encoded */
#define H264_TYPE_AUTO     0 /* encoder: automatically determine coding type */
#define H264_TYPE_IVOP     1 /* intra frame */
#define H264_TYPE_PVOP     2 /* predicted frame */
#define H264_TYPE_BVOP     3 /* bidirectionally encoded */
#define H264_TYPE_SVOP     4 /* predicted+sprite frame */


/*****************************************************************************
 * H264_global()
 ****************************************************************************/

/* cpu_flags definitions (make sure to sync this with cpuid.asm for ia32) */

#define H264_CPU_FORCE    (1<<31) /* force passed cpu flags */
#define H264_CPU_ASM      (1<< 7) /* native assembly */
/* ARCH_IS_IA32 */
#define H264_CPU_MMX      (1<< 0) /*       mmx : pentiumMMX,k6 */
#define H264_CPU_MMXEXT   (1<< 1) /*   mmx-ext : pentium2, athlon */
#define H264_CPU_SSE      (1<< 2) /*       sse : pentium3, athlonXP */
#define H264_CPU_SSE2     (1<< 3) /*      sse2 : pentium4, athlon64 */
#define H264_CPU_3DNOW    (1<< 4) /*     3dnow : k6-2 */
#define H264_CPU_3DNOWEXT (1<< 5) /* 3dnow-ext : athlon */
#define H264_CPU_TSC      (1<< 6) /*       tsc : Pentium */
/* ARCH_IS_PPC */
#define H264_CPU_ALTIVEC  (1<< 0) /* altivec */


#define H264_DEBUG_ERROR     (1<< 0)
#define H264_DEBUG_STARTCODE (1<< 1)
#define H264_DEBUG_HEADER    (1<< 2)
#define H264_DEBUG_TIMECODE  (1<< 3)
#define H264_DEBUG_MB        (1<< 4)
#define H264_DEBUG_COEFF     (1<< 5)
#define H264_DEBUG_MV        (1<< 6)
#define H264_DEBUG_RC        (1<< 7)
#define H264_DEBUG_DEBUG     (1<<31)

/* H264_GBL_INIT param1 */
typedef struct {
	int version;
	unsigned int cpu_flags; /* [in:opt] zero = autodetect cpu; H264_CPU_FORCE|{cpu features} = force cpu features */
	int debug;     /* [in:opt] debug level */
} H264_gbl_init_t;


/* H264_GBL_INFO param1 */
typedef struct {
	int version;
	int actual_version; /* [out] returns the actual H264core version */
	const char * build; /* [out] if !null, points to description of this H264 core build */
	unsigned int cpu_flags;      /* [out] detected cpu features */
	int num_threads;    /* [out] detected number of cpus/threads */
} H264_gbl_info_t;


/* H264_GBL_CONVERT param1 */
typedef struct {
	int version;
	H264_image_t input;  /* [in] input image & colorspace */
	H264_image_t output; /* [in] output image & colorspace */
	int width;           /* [in] width */
	int height;          /* [in] height */
	int interlacing;     /* [in] interlacing */
} H264_gbl_convert_t;


#define H264_GBL_INIT    0 /* initialize H264core; must be called before using H264_decore, or H264_encore) */
#define H264_GBL_INFO    1 /* return some info about H264core, and the host computer */
#define H264_GBL_CONVERT 2 /* colorspace conversion utility */

extern int H264_global(void *handle, int opt, void *param1, void *param2);


/*****************************************************************************
 * H264_decore()
 ****************************************************************************/

#define H264_DEC_CREATE  0 /* create decore instance; return 0 on success */
#define H264_DEC_DESTROY 1 /* destroy decore instance: return 0 on success */
#define H264_DEC_DECODE  2 /* decode a frame: returns number of bytes consumed >= 0 */

extern int H264_decore(void *handle, int opt, void *param1, void *param2);

/* H264_DEC_CREATE param 1
	image width & height may be specified here when the dimensions are
	known in advance. */
typedef struct {
	int version;
	int width;     /* [in:opt] image width */
	int height;    /* [in:opt] image width */
	void * handle; /* [out]	   decore context handle */
} H264_dec_create_t;


/* H264_DEC_DECODE param1 */
/* general flags */
#define H264_LOWDELAY      (1<<0) /* lowdelay mode  */
#define H264_DISCONTINUITY (1<<1) /* indicates break in stream */
#define H264_DEBLOCKY      (1<<2) /* perform luma deblocking */
#define H264_DEBLOCKUV     (1<<3) /* perform chroma deblocking */
#define H264_FILMEFFECT    (1<<4) /* adds film grain */
#define H264_DERINGUV      (1<<5) /* perform chroma deringing, requires deblocking to work */
#define H264_DERINGY       (1<<6) /* perform luma deringing, requires deblocking to work */

#define H264_DEC_FAST      (1<<29) /* disable postprocessing to decrease cpu usage *todo* */
#define H264_DEC_DROP      (1<<30) /* drop bframes to decrease cpu usage *todo* */
#define H264_DEC_PREROLL   (1<<31) /* decode as fast as you can, don't even show output *todo* */

typedef struct {
	int version;
	int general;         /* [in:opt] general flags */
	void *bitstream;     /* [in]     bitstream (read from)*/
	int length;          /* [in]     bitstream length */
	H264_image_t output; /* [in]     output image (written to) */
/* ------- v1.1.x ------- */
	int brightness;		 /* [in]	 brightness offset (0=none) */
} H264_dec_frame_t;


/* H264_DEC_DECODE param2 :: optional */
typedef struct
{
	int version;

	int type;                   /* [out] output data type */
	union {
		struct { /* type>0 {H264_TYPE_IVOP,H264_TYPE_PVOP,H264_TYPE_BVOP,H264_TYPE_SVOP} */
			int general;        /* [out] flags */
			int time_base;      /* [out] time base */
			int time_increment; /* [out] time increment */

			/* XXX: external deblocking stuff */
			int * qscale;	    /* [out] pointer to quantizer table */
			int qscale_stride;  /* [out] quantizer scale stride */

		} vop;
		struct {	/* H264_TYPE_VOL */
			int general;        /* [out] flags */
			int width;          /* [out] width */
			int height;         /* [out] height */
			int par;            /* [out] pixel aspect ratio (refer to H264_PAR_xxx above) */
			int par_width;      /* [out] aspect ratio width  [1..255] */
			int par_height;     /* [out] aspect ratio height [1..255] */
		} vol;
	} data;
} H264_dec_stats_t;

#define H264_ZONE_QUANT  (1<<0)
#define H264_ZONE_WEIGHT (1<<1)

typedef struct
{
	int frame;
	int mode;
	int increment;
	int base;
} H264_enc_zone_t;


/*----------------------------------------------------------------------------
 * H264_enc_stats_t structure
 *
 * Used in:
 *  - H264_plg_data_t structure
 *  - optional parameter in H264_encore() function
 *
 * .coding_type = H264_TYPE_NOTHING if the stats are not given
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

	int hlength;   /* [out] header length (bytes) */
	int kblks;     /* [out] number of blocks compressed as Intra */
	int mblks;     /* [out] number of blocks compressed as Inter */
	int ublks;     /* [out] number of blocks marked as not_coded */

	int sse_y;     /* [out] Y plane's sse */
	int sse_u;     /* [out] U plane's sse */
	int sse_v;     /* [out] V plane's sse */
} H264_enc_stats_t;

/*****************************************************************************
  H264 plugin system -- internals

  H264core will call H264_PLG_INFO and H264_PLG_CREATE during H264_ENC_CREATE
  before encoding each frame H264core will call H264_PLG_BEFORE
  after encoding each frame H264core will call H264_PLG_AFTER
  H264core will call H264_PLG_DESTROY during H264_ENC_DESTROY
 ****************************************************************************/


#define H264_PLG_CREATE  (1<<0)
#define H264_PLG_DESTROY (1<<1)
#define H264_PLG_INFO    (1<<2)
#define H264_PLG_BEFORE  (1<<3)
#define H264_PLG_FRAME   (1<<4)
#define H264_PLG_AFTER   (1<<5)

/* H264_plg_info_t.flags */
#define H264_REQORIGINAL (1<<0) /* plugin requires a copy of the original (uncompressed) image */
#define H264_REQPSNR     (1<<1) /* plugin requires psnr between the uncompressed and compressed image*/
#define H264_REQDQUANTS  (1<<2) /* plugin requires access to the dquant table */


typedef struct
{
	int version;
	int flags;   /* [in:opt] plugin flags */
} H264_plg_info_t;


typedef struct
{
	int version;

	int num_zones;           /* [out] */
	H264_enc_zone_t * zones; /* [out] */

	int width;               /* [out] */
	int height;              /* [out] */
	int mb_width;            /* [out] */
	int mb_height;           /* [out] */
	int fincr;               /* [out] */
	int fbase;               /* [out] */

	void * param;            /* [out] */
} H264_plg_create_t;


typedef struct
{
	int version;

	int num_frames; /* [out] total frame encoded */
} H264_plg_destroy_t;

typedef struct
{
	int version;

	H264_enc_zone_t * zone; /* [out] current zone */

	int width;              /* [out] */
	int height;             /* [out] */
	int mb_width;           /* [out] */
	int mb_height;          /* [out] */
	int fincr;              /* [out] */
	int fbase;              /* [out] */

	int min_quant[3];       /* [out] */
	int max_quant[3];       /* [out] */

	H264_image_t reference; /* [out] -> [out] */
	H264_image_t current;   /* [out] -> [in,out] */
	H264_image_t original;  /* [out] after: points the original (uncompressed) copy of the current frame */
	int frame_num;          /* [out] frame number */

	int type;               /* [in,out] */
	int quant;              /* [in,out] */

	int * dquant;           /* [in,out]	pointer to diff quantizer table */
	int dquant_stride;      /* [in,out]	diff quantizer stride */

	int vop_flags;          /* [in,out] */
	int vol_flags;          /* [in,out] */
	int motion_flags;       /* [in,out] */

/* Deprecated, use the stats field instead.
 * Will disapear before 1.0 */
	int length;             /* [out] after: length of encoded frame */
	int kblks;              /* [out] number of blocks compressed as Intra */
	int mblks;              /* [out] number of blocks compressed as Inter */
	int ublks;              /* [out] number of blocks marked not_coded */
	int sse_y;              /* [out] Y plane's sse */
	int sse_u;              /* [out] U plane's sse */
	int sse_v;              /* [out] V plane's sse */
/* End of duplicated data, kept only for binary compatibility */

	int bquant_ratio;       /* [in] */
	int bquant_offset;      /* [in] */

	H264_enc_stats_t stats; /* [out] frame statistics */
} H264_plg_data_t;

/*****************************************************************************
  H264 plugin system -- external

  the application passes H264 an array of "H264_plugin_t" at H264_ENC_CREATE. the array
  indicates the plugin function pointer and plugin-specific data.
  H264core handles the rest. example:

  H264_enc_create_t create;
  H264_enc_plugin_t plugins[2];

  plugins[0].func = H264_psnr_func;
  plugins[0].param = NULL;
  plugins[1].func = H264_cbr_func;
  plugins[1].param = &cbr_data;

  create.num_plugins = 2;
  create.plugins = plugins;

 ****************************************************************************/

typedef int (H264_plugin_func)(void * handle, int opt, void * param1, void * param2);

typedef struct
{
	H264_plugin_func * func;
	void * param;
} H264_enc_plugin_t;


extern H264_plugin_func H264_plugin_single;   /* single-pass rate control */
extern H264_plugin_func H264_plugin_2pass1;   /* two-pass rate control: first pass */
extern H264_plugin_func H264_plugin_2pass2;   /* two-pass rate control: second pass */

extern H264_plugin_func H264_plugin_lumimasking;  /* lumimasking */

extern H264_plugin_func H264_plugin_psnr;	/* write psnr values to stdout */
extern H264_plugin_func H264_plugin_dump;	/* dump before and after yuvpgms */


/* single pass rate control
 * CBR and Constant quantizer modes */
typedef struct
{
	int version;

	int bitrate;               /* [in] bits per second */
	int reaction_delay_factor; /* [in] */
	int averaging_period;      /* [in] */
	int buffer;                /* [in] */
} H264_plugin_single_t;


typedef struct {
	int version;

	char * filename;
} H264_plugin_2pass1_t;


#define H264_PAYBACK_BIAS 0 /* payback with bias */
#define H264_PAYBACK_PROP 1 /* payback proportionally */

typedef struct {
	int version;

	int bitrate;                  /* [in] target bitrate (bits per second) */
	char * filename;              /* [in] first pass stats filename */

	int keyframe_boost;           /* [in] keyframe boost percentage: [0..100] */
	int curve_compression_high;   /* [in] percentage of compression performed on the high part of the curve (above average) */
	int curve_compression_low;    /* [in] percentage of compression performed on the low  part of the curve (below average) */
	int overflow_control_strength;/* [in] Payback delay expressed in number of frames */
	int max_overflow_improvement; /* [in] percentage of allowed range for a frame that gets bigger because of overflow bonus */
	int max_overflow_degradation; /* [in] percentage of allowed range for a frame that gets smaller because of overflow penalty */

	int kfreduction;              /* [in] maximum bitrate reduction applied to an iframe under the kfthreshold distance limit */
	int kfthreshold;              /* [in] if an iframe is closer to the next iframe than this distance, a quantity of bits
								   *      is substracted from its bit allocation. The reduction is computed as multiples of
								   *      kfreduction/kthreshold. It reaches kfreduction when the distance == kfthreshold,
								   *      0 for 1<distance<kfthreshold */

	int container_frame_overhead; /* [in] How many bytes the controller has to compensate per frame due to container format overhead */

/* ------- v1.1.x ------- */
	int vbv_size;                 /* [in] buffer size (bits) */
	int vbv_initial;              /* [in] initial buffer occupancy (bits) */
	int vbv_maxrate;              /* [in] max processing bitrate (bits per second) */
	int vbv_peakrate;             /* [in:opt] max average bitrate over 3 seconds (bits per second) */

}H264_plugin_2pass2_t;

/*****************************************************************************
 *                             ENCODER API
 ****************************************************************************/

/*----------------------------------------------------------------------------
 * Encoder operations
 *--------------------------------------------------------------------------*/

#define H264_ENC_CREATE  0 /* create encoder instance; returns 0 on success */
#define H264_ENC_DESTROY 1 /* destroy encoder instance; returns 0 on success */
#define H264_ENC_ENCODE  2 /* encode a frame: returns number of ouput bytes
                            * 0 means this frame should not be written (ie. encoder lag) */


/*----------------------------------------------------------------------------
 * Encoder entry point
 *--------------------------------------------------------------------------*/

extern int H264_encore(void *handle, int opt, void *param1, void *param2);

/* Quick API reference
 *
 * H264_ENC_CREATE operation
 *  - handle: ignored
 *  - opt: H264_ENC_CREATE
 *  - param1: address of a H264_enc_create_t structure
 *  - param2: ignored
 *
 * H264_ENC_ENCODE operation
 *  - handle: an instance returned by a CREATE op
 *  - opt: H264_ENC_ENCODE
 *  - param1: address of a H264_enc_frame_t structure
 *  - param2: address of a H264_enc_stats_t structure (optional)
 *            its return value is asynchronous to what is written to the buffer
 *            depending on the delay introduced by bvop use. It's display
 *            ordered.
 *
 * H264_ENC_DESTROY operation
 *  - handle: an instance returned by a CREATE op
 *  - opt: H264_ENC_DESTROY
 *  - param1: ignored
 *  - param2: ignored
 */


/*----------------------------------------------------------------------------
 * "Global" flags
 *
 * These flags are used for H264_enc_create_t->global field during instance
 * creation (operation H264_ENC_CREATE)
 *--------------------------------------------------------------------------*/

#define H264_GLOBAL_PACKED            (1<<0) /* packed bitstream */
#define H264_GLOBAL_CLOSED_GOP        (1<<1) /* closed_gop:	was DX50BVOP dx50 bvop compatibility */
#define H264_GLOBAL_EXTRASTATS_ENABLE (1<<2)
#if 0
#define H264_GLOBAL_VOL_AT_IVOP       (1<<3) /* write vol at every ivop: WIN32/divx compatibility */
#define H264_GLOBAL_FORCE_VOL         (1<<4) /* when vol-based parameters are changed, insert an ivop NOT recommended */
#endif
#define H264_GLOBAL_DIVX5_USERDATA    (1<<5) /* write divx5 userdata string 
                                                this is implied if H264_GLOBAL_PACKED is set */

/*----------------------------------------------------------------------------
 * "VOL" flags
 *
 * These flags are used for H264_enc_frame_t->vol_flags field during frame
 * encoding (operation H264_ENC_ENCODE)
 *--------------------------------------------------------------------------*/

#define H264_VOL_MPEGQUANT      (1<<0) /* enable MPEG type quantization */
#define H264_VOL_EXTRASTATS     (1<<1) /* enable plane sse stats */
#define H264_VOL_QUARTERPEL     (1<<2) /* enable quarterpel: frames will encoded as quarterpel */
#define H264_VOL_GMC            (1<<3) /* enable GMC; frames will be checked for gmc suitability */
#define H264_VOL_REDUCED_ENABLE (1<<4) /* enable reduced resolution vops: frames will be checked for rrv suitability */
									   /* NOTE:  the reduced resolution feature is not supported anymore. This flag will have no effect! */
#define H264_VOL_INTERLACING    (1<<5) /* enable interlaced encoding */


/*----------------------------------------------------------------------------
 * "VOP" flags
 *
 * These flags are used for H264_enc_frame_t->vop_flags field during frame
 * encoding (operation H264_ENC_ENCODE)
 *--------------------------------------------------------------------------*/

/* Always valid */
#define H264_VOP_DEBUG                (1<< 0) /* print debug messages in frames */
#define H264_VOP_HALFPEL              (1<< 1) /* use halfpel interpolation */
#define H264_VOP_INTER4V              (1<< 2) /* use 4 motion vectors per MB */
#define H264_VOP_TRELLISQUANT         (1<< 3) /* use trellis based R-D "optimal" quantization */
#define H264_VOP_CHROMAOPT            (1<< 4) /* enable chroma optimization pre-filter */
#define H264_VOP_CARTOON              (1<< 5) /* use 'cartoon mode' */
#define H264_VOP_GREYSCALE            (1<< 6) /* enable greyscale only mode (even for  color input material chroma is ignored) */
#define H264_VOP_HQACPRED             (1<< 7) /* high quality ac prediction */
#define H264_VOP_MODEDECISION_RD      (1<< 8) /* enable DCT-ME and use it for mode decision */
#define H264_VOP_FAST_MODEDECISION_RD (1<<12) /* use simplified R-D mode decision */
#define H264_VOP_RD_BVOP              (1<<13) /* enable rate-distortion mode decision in b-frames */

/* Only valid for vol_flags|=H264_VOL_INTERLACING */
#define H264_VOP_TOPFIELDFIRST        (1<< 9) /* set top-field-first flag  */
#define H264_VOP_ALTERNATESCAN        (1<<10) /* set alternate vertical scan flag */

/* only valid for vol_flags|=H264_VOL_REDUCED_ENABLED */
#define H264_VOP_REDUCED              (1<<11) /* reduced resolution vop */
											  /* NOTE: reduced resolution feature is not supported anymore. This flag will have no effect! */

/*----------------------------------------------------------------------------
 * "Motion" flags
 *
 * These flags are used for H264_enc_frame_t->motion field during frame
 * encoding (operation H264_ENC_ENCODE)
 *--------------------------------------------------------------------------*/

/* Motion Estimation Search Patterns */
#define H264_ME_ADVANCEDDIAMOND16     (1<< 0) /* use advdiamonds instead of diamonds as search pattern */
#define H264_ME_ADVANCEDDIAMOND8      (1<< 1) /* use advdiamond for H264_ME_EXTSEARCH8 */
#define H264_ME_USESQUARES16          (1<< 2) /* use squares instead of diamonds as search pattern */
#define H264_ME_USESQUARES8           (1<< 3) /* use square for H264_ME_EXTSEARCH8 */

/* SAD operator based flags */
#define H264_ME_HALFPELREFINE16       (1<< 4)
#define H264_ME_HALFPELREFINE8        (1<< 6)
#define H264_ME_QUARTERPELREFINE16    (1<< 7)
#define H264_ME_QUARTERPELREFINE8     (1<< 8)
#define H264_ME_GME_REFINE            (1<< 9)
#define H264_ME_EXTSEARCH16           (1<<10) /* extend PMV by more searches */
#define H264_ME_EXTSEARCH8            (1<<11) /* use diamond/square for extended 8x8 search */
#define H264_ME_CHROMA_PVOP           (1<<12) /* also use chroma for P_VOP/S_VOP ME */
#define H264_ME_CHROMA_BVOP           (1<<13) /* also use chroma for B_VOP ME */
#define H264_ME_FASTREFINE16          (1<<25) /* use low-complexity refinement functions */
#define H264_ME_FASTREFINE8           (1<<29) /* low-complexity 8x8 sub-block refinement */

/* Rate Distortion based flags
 * Valid when H264_VOP_MODEDECISION_RD is enabled */
#define H264_ME_HALFPELREFINE16_RD    (1<<14) /* perform RD-based halfpel refinement */
#define H264_ME_HALFPELREFINE8_RD     (1<<15) /* perform RD-based halfpel refinement for 8x8 mode */
#define H264_ME_QUARTERPELREFINE16_RD (1<<16) /* perform RD-based qpel refinement */
#define H264_ME_QUARTERPELREFINE8_RD  (1<<17) /* perform RD-based qpel refinement for 8x8 mode */
#define H264_ME_EXTSEARCH_RD          (1<<18) /* perform RD-based search using square pattern enable H264_ME_EXTSEARCH8 to do this in 8x8 search as well */
#define H264_ME_CHECKPREDICTION_RD    (1<<19) /* always check vector equal to prediction */

/* Other */
#define H264_ME_DETECT_STATIC_MOTION  (1<<24) /* speed-up ME by detecting stationary scenes */
#define H264_ME_SKIP_DELTASEARCH      (1<<26) /* speed-up by skipping b-frame delta search */
#define H264_ME_FAST_MODEINTERPOLATE  (1<<27) /* speed-up by partly skipping interpolate mode */
#define H264_ME_BFRAME_EARLYSTOP      (1<<28) /* speed-up by early exiting b-search */

/* Unused */
#define H264_ME_UNRESTRICTED16        (1<<20) /* unrestricted ME, not implemented */
#define H264_ME_OVERLAPPING16         (1<<21) /* overlapping ME, not implemented */
#define H264_ME_UNRESTRICTED8         (1<<22) /* unrestricted ME, not implemented */
#define H264_ME_OVERLAPPING8          (1<<23) /* overlapping ME, not implemented */


/*----------------------------------------------------------------------------
 * H264_enc_create_t structure definition
 *
 * This structure is passed as param1 during an instance creation (operation
 * H264_ENC_CREATE)
 *--------------------------------------------------------------------------*/

typedef struct {
	int version;

	int profile;                 /* [in] profile@level; refer to H264_PROFILE_xxx */
	int width;                   /* [in] frame dimensions; width, pixel units */
	int height;                  /* [in] frame dimensions; height, pixel units */

	int num_zones;               /* [in:opt] number of bitrate zones */
	H264_enc_zone_t * zones;     /*          ^^ zone array */

	int num_plugins;             /* [in:opt] number of plugins */
	H264_enc_plugin_t * plugins; /*          ^^ plugin array */

	int num_threads;             /* [in:opt] number of threads */
	int max_bframes;             /* [in:opt] max sequential bframes (0=disable bframes) */

	int global;                  /* [in:opt] global flags; controls encoding behavior */

	/* --- vol-based stuff; included here for convenience */
	int fincr;                   /* [in:opt] framerate increment; set to zero for variable framerate */
	int fbase;                   /* [in] framerate base frame_duration = fincr/fbase seconds*/
    /* ---------------------------------------------- */

	/* --- vop-based; included here for convenience */
	int max_key_interval;        /* [in:opt] the maximum interval between key frames */

	int frame_drop_ratio;        /* [in:opt] frame dropping: 0=drop none... 100=drop all */

	int bquant_ratio;            /* [in:opt] bframe quantizer multipier/offeset; used to decide bframes quant when bquant==-1 */
	int bquant_offset;           /* bquant = (avg(past_ref_quant,future_ref_quant)*bquant_ratio + bquant_offset) / 100 */

	int min_quant[3];            /* [in:opt] */
	int max_quant[3];            /* [in:opt] */
	/* ---------------------------------------------- */

	void *handle;                /* [out] encoder instance handle */
} H264_enc_create_t;


/*----------------------------------------------------------------------------
 * H264_enc_frame_t structure definition
 *
 * This structure is passed as param1 during a frame encoding (operation
 * H264_ENC_ENCODE)
 *--------------------------------------------------------------------------*/

/* out value for the frame structure->type field
 * unlike stats output in param2, this field is not asynchronous and tells
 * the client app, if the frame written into the stream buffer is an ivop
 * usually used for indexing purpose in the container */
#define H264_KEYFRAME (1<<1)

/* The structure */
typedef struct {
	int version;

	/* VOL related stuff
	 * unless H264_FORCEVOL is set, the encoder will not react to any changes
	 * here until the next VOL (keyframe). */

	int vol_flags;                     /* [in] vol flags */
	unsigned char *quant_intra_matrix; /* [in:opt] custom intra qmatrix */
	unsigned char *quant_inter_matrix; /* [in:opt] custom inter qmatrix */

	int par;                           /* [in:opt] pixel aspect ratio (refer to H264_PAR_xxx above) */
	int par_width;                     /* [in:opt] aspect ratio width */
	int par_height;                    /* [in:opt] aspect ratio height */

	/* Other fields that can change on a frame base */

	int fincr;                         /* [in:opt] framerate increment, for variable framerate only */
	int vop_flags;                     /* [in] (general)vop-based flags */
	int motion;                        /* [in] ME options */

	H264_image_t input;                /* [in] input image (read from) */

	int type;                          /* [in:opt] coding type */
	int quant;                         /* [in] frame quantizer; if <=0, automatic (ratecontrol) */
	int bframe_threshold;

	void *bitstream;                   /* [in:opt] bitstream ptr (written to)*/
	int length;                        /* [in:opt] bitstream length (bytes) */

	int out_flags;                     /* [out] bitstream output flags */
} H264_enc_frame_t;

#ifdef __cplusplus
}
#endif

#endif
