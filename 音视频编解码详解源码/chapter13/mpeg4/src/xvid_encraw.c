#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include <math.h>
#include <time.h>
#include <csl.h>
#include <csl_dat.h>
#include <csl_cache.h>
#include "xvid.h"

#undef READ_PNM

#define MAX_ZONES   64
/* Maximum number of frames to encode */
#define ABS_MAXFRAMENR 9999
static int ARG_STATS = 0;
//static int ARG_DUMP = 0;
//static int ARG_LUMIMASKING = 0;
//static int ARG_BITRATE = 0;
//static int ARG_SINGLE = 0;
//static char *ARG_PASS1 = 0;
//static char *ARG_PASS2 = 0;
static int ARG_QUALITY = 2;
static float ARG_FRAMERATE = 25.00f;
static int ARG_MAXFRAMENR = 20;
static int ARG_MAXKEYINTERVAL = 100;
static char *ARG_INPUTFILE = "e:\\yuv\\foreman.cif";
//static char *ARG_INPUTFILE = "c:\\stefan.yuv";
//static int ARG_INPUTTYPE = 0;
static int ARG_SAVEMPEGSTREAM = 1;
//ARG_SAVEMPEGSTREAM
static int ARG_SAVEINDIVIDUAL = 0;
static char *ARG_OUTPUTFILE = "c:\\mpeg4_dsp.mp4";
//static int XDIM = 176;
//static int YDIM = 144;
static int XDIM = 352;
static int YDIM = 288;
//static int ARG_BQRATIO = 150;
//static int ARG_BQOFFSET = 100;
//static int ARG_MAXBFRAMES = 0;
//static int ARG_PACKED = 0;
//static int ARG_DEBUG = 0;
static int ARG_VOPDEBUG = 0;
//static int ARG_GMC = 0;
//static int ARG_QPEL = 0;
//static int ARG_CLOSED_GOP = 0;
static int ARG_QUANT = 7;

/*****************************************************************************
 *                            Quality presets
 ****************************************************************************/

static const int motion_presets[] = {
	/* quality 0 */
	0,

	/* quality 1 */
	XVID_ME_ADVANCEDDIAMOND16,

	/* quality 2 */
	XVID_ME_ADVANCEDDIAMOND16 | XVID_ME_HALFPELREFINE16,
#if 0
	/* quality 3 */
	XVID_ME_ADVANCEDDIAMOND16 | XVID_ME_HALFPELREFINE16 |
	XVID_ME_ADVANCEDDIAMOND8 | XVID_ME_HALFPELREFINE8,

	/* quality 4 */
	XVID_ME_ADVANCEDDIAMOND16 | XVID_ME_HALFPELREFINE16 |
	XVID_ME_ADVANCEDDIAMOND8 | XVID_ME_HALFPELREFINE8 |
	XVID_ME_CHROMA_PVOP | XVID_ME_CHROMA_BVOP,

	/* quality 5 */
	XVID_ME_ADVANCEDDIAMOND16 | XVID_ME_HALFPELREFINE16 |
	XVID_ME_ADVANCEDDIAMOND8 | XVID_ME_HALFPELREFINE8 |
	XVID_ME_CHROMA_PVOP | XVID_ME_CHROMA_BVOP,

	/* quality 6 */
	XVID_ME_ADVANCEDDIAMOND16 | XVID_ME_HALFPELREFINE16 | XVID_ME_EXTSEARCH16 |
	XVID_ME_ADVANCEDDIAMOND8 | XVID_ME_HALFPELREFINE8 | XVID_ME_EXTSEARCH8 |
	XVID_ME_CHROMA_PVOP | XVID_ME_CHROMA_BVOP,
#endif
};
#define ME_ELEMENTS (sizeof(motion_presets)/sizeof(motion_presets[0]))

static const int vop_presets[] = {
	/* quality 0 */
	0,

	/* quality 1 */
	0,

	/* quality 2 */
	XVID_VOP_HALFPEL,
#if 0
	/* quality 3 */
	XVID_VOP_HALFPEL | XVID_VOP_INTER4V,

	/* quality 4 */
	XVID_VOP_HALFPEL | XVID_VOP_INTER4V,

	/* quality 5 */
	XVID_VOP_HALFPEL | XVID_VOP_INTER4V |
	XVID_VOP_TRELLISQUANT,

	/* quality 6 */
	XVID_VOP_HALFPEL | XVID_VOP_INTER4V |
	XVID_VOP_TRELLISQUANT | XVID_VOP_HQACPRED,
#endif
};
#define VOP_ELEMENTS (sizeof(vop_presets)/sizeof(vop_presets[0]))

/*****************************************************************************
 *                     Command line global variables
 ****************************************************************************/




#define IMAGE_SIZE(x,y) ((x)*(y)*3/2)
#define MAX(A,B) ( ((A)>(B)) ? (A) : (B) )
#define SMALL_EPS (1e-10)

#define SWAP(a) ( (((a)&0x000000ff)<<24) | (((a)&0x0000ff00)<<8) | \
                  (((a)&0x00ff0000)>>8)  | (((a)&0xff000000)>>24) )

/****************************************************************************
 *                     Nasty global vars ;-)
 ***************************************************************************/

//static int i;

/* the path where to save output */
static char filepath[256] = "./";

/* Internal structures (handles) for encoding and decoding */
static void *enc_handle = NULL;

/*****************************************************************************
 *               Local prototypes
 ****************************************************************************/

/* Prints program usage message */
//static void usage();

/* Statistical functions */
static double msecond();

/* PGM related functions */

static int read_yuvdata(FILE * handle,
						unsigned char *image);

/* Encoder related functions */
static int enc_init(int use_assembler);
static int enc_stop();
static int enc_main(unsigned char *image,
					unsigned char *bitstream,
					int *key,
					int *stats_type,
					int *stats_quant,
					int *stats_length,
					int stats[3]);

/*****************************************************************************
 *               Main function
 ****************************************************************************/

int
main(int argc,
	 char *argv[])
{

	unsigned char *mp4_buffer = NULL;
	unsigned char *in_buffer = NULL;
	unsigned char *out_buffer = NULL;

	double enctime;
	double totalenctime = 0.;
	float totalPSNR[3] = {0., 0., 0.};

	float fBitRate;
	int totalsize;
	int result;
	int m4v_size;
	int key;
	int stats_type;
	int stats_quant;
	int stats_length;
	int use_assembler = 0;
	FILE *in_file = stdin;
	FILE *out_file = NULL;
	int input_num;
	int output_num;

	char filename[256];
	clock_t start,stop;
	
	///////////////////////////////////////////////////////
	CSL_init();	
	CACHE_clean(CACHE_L2ALL, 0, 0);
	CACHE_setL2Mode(CACHE_64KCACHE);
	CACHE_enableCaching(CACHE_EMIFA_CE00);
	CACHE_enableCaching(CACHE_EMIFA_CE01);
	DAT_open(DAT_CHAANY, DAT_PRI_LOW, DAT_OPEN_2D);
	CACHE_setPriL2Req(CACHE_L2PRIHIGH);
	//////////////////////////////////////////////////////	
	
	printf(" xvid_encraw - raw mpeg4 bitstream encoder ");
	printf("\n\n Is Encoding Image, Plese wait...\n");
	/* Is there a dumb XviD coder ? */
	if(ME_ELEMENTS != VOP_ELEMENTS) {
		fprintf(stderr, "Presets' arrays should have the same number of elements -- Please fill a bug to xvid-devel@xvid.org\n");
		return(-1);
	}

/*****************************************************************************
 *                            Arguments checking
 ****************************************************************************/
	if (ARG_QUALITY < 0 ) {
		ARG_QUALITY = 0;
	} else if (ARG_QUALITY >= ME_ELEMENTS) {
		ARG_QUALITY = ME_ELEMENTS - 1;
	}

	if (ARG_FRAMERATE <= 0) {
		fprintf(stderr, "Wrong Framerate %s \n", argv[5]);
		return (-1);
	}

	if (ARG_MAXFRAMENR <= 0) {
		fprintf(stderr, "Wrong number of frames\n");
		return (-1);
	}

	if (ARG_INPUTFILE == NULL || strcmp(ARG_INPUTFILE, "stdin") == 0) 
    {
		in_file = stdin;
	}
    else 
    {
		in_file = fopen(ARG_INPUTFILE, "rb");
		if (in_file == NULL) 
        {
			fprintf(stderr, "Error opening input file %s\n", ARG_INPUTFILE);
			return (-1);
		}
	}

	/* now we know the sizes, so allocate memory */
	in_buffer = (unsigned char *) malloc(IMAGE_SIZE(XDIM, YDIM));
	if (!in_buffer)
		goto free_all_memory;

	/* this should really be enough memory ! */
	mp4_buffer = (unsigned char *) malloc(IMAGE_SIZE(XDIM, YDIM) * 2);
	if (!mp4_buffer)
		goto free_all_memory;

/*****************************************************************************
 *                            XviD PART  Start
 ****************************************************************************/
	result = enc_init(use_assembler);
	if (result) {
		fprintf(stderr, "Encore INIT problem, return value %d\n", result);
		goto release_all;
	}

/*****************************************************************************
 *                            Main loop
 ****************************************************************************/

	if (ARG_SAVEMPEGSTREAM && ARG_OUTPUTFILE) {

		if ((out_file = fopen(ARG_OUTPUTFILE, "w+b")) == NULL) {
			fprintf(stderr, "Error opening output file %s\n", ARG_OUTPUTFILE);
			goto release_all;
		}

	} else {
		out_file = NULL;
	}
/*****************************************************************************
 *                       Encoding loop
 ****************************************************************************/
	totalsize = 0;
	result = 0;
	input_num = 0;				/* input frame counter */
	output_num = 0;				/* output frame counter */
	do {

		char *type;
		int sse[3];

		if (input_num >= ARG_MAXFRAMENR) {
			result = 1;
		}

		if (!result) 
        {
			{
				result = read_yuvdata(in_file, in_buffer);
				// fread(in_buffer,1,152064,in_file);//IMAGE_SIZE(XDIM,YDIM)
			}
		}

/*****************************************************************************
 *                       Encode and decode this frame
 ****************************************************************************/

		//enctime = msecond();
		start = clock();
		m4v_size =
			enc_main(!result ? in_buffer : 0, mp4_buffer, &key, &stats_type,
					 &stats_quant, &stats_length, sse);
		stop = clock();
		//enctime = msecond() - enctime;

		/* Write the Frame statistics */
		/*
		printf("%5d: key=%i, time= %6.0f, len= %7d", !result ? input_num : -1,
			   key, (float) enctime, (int) m4v_size);
		*/
		if (stats_type > 0) {	/* !XVID_TYPE_NOTHING */

			switch (stats_type) {
			case XVID_TYPE_IVOP:
				type = "I";
				break;
			case XVID_TYPE_PVOP:
				type = "P";
				break;
			case XVID_TYPE_BVOP:
				type = "B";
				break;
			case XVID_TYPE_SVOP:
				type = "S";
				break;
			default:
				type = "U";
				break;
			}

			printf(" type=%s, quant= %2d, len= %7d, cycles=%d\n", type, stats_quant,
				   stats_length, stop-start);

			}

		if (m4v_size < 0) {
			break;
		}

		/* Update encoding time stats */
		totalenctime += enctime;
		totalsize += m4v_size;

/*****************************************************************************
 *                       Save stream to file
 ****************************************************************************/

		if (m4v_size > 0 && ARG_SAVEMPEGSTREAM) {

			/* Save single files */
			if (ARG_SAVEINDIVIDUAL) {
				FILE *out;
				sprintf(filename, "%sframe%05d.m4v", filepath, output_num);
				out = fopen(filename, "w+b");
				fwrite(mp4_buffer, m4v_size, 1, out);
				fclose(out);
				output_num++;
			}

			/* Save ES stream */
			if (ARG_OUTPUTFILE && out_file)
				fwrite(mp4_buffer, 1, m4v_size, out_file);
		}

		input_num++;
	} while (1);



/*****************************************************************************
 *         Calculate totals and averages for output, print results
 ****************************************************************************/

	printf("Tot: enctime(ms) =%7.2f,               length(bytes) = %7d\n",
		   totalenctime, (int) totalsize);

	if (input_num > 0) {
		fBitRate  = (float)((totalsize*8*25)/input_num);
		totalsize /= input_num;
		totalenctime /= input_num;
		totalPSNR[0] /= input_num;
		totalPSNR[1] /= input_num;
		totalPSNR[2] /= input_num;
	} else {
		totalsize = -1;
		totalenctime = -1;
	}

	printf("Avg: enctime(ms) =%7.2f, fps =%7.2f, length(bytes) = %7d, \nBit Rates(kbps) is %5.2f ",
		   totalenctime, 1000 / totalenctime, (int) totalsize, fBitRate/1000);
   if (ARG_STATS) {
       printf("\n, psnr y = %2.2f, psnr u = %2.2f, psnr v = %2.2f\n",
    		  totalPSNR[0],totalPSNR[1],totalPSNR[2]);
	}
	printf("\n");


/*****************************************************************************
 *                            XviD PART  Stop
 ****************************************************************************/

  release_all:

	if (enc_handle) {
		result = enc_stop();
		if (result)
			fprintf(stderr, "Encore RELEASE problem return value %d\n",
					result);
	}

	if (in_file)
		fclose(in_file);
	if (out_file)
		fclose(out_file);

  free_all_memory:
	free(out_buffer);
	free(mp4_buffer);
	free(in_buffer);
	printf("\n Encode Finished!");
	return (0);
}

/*****************************************************************************
 *                        "statistical" functions
 *
 *  these are not needed for encoding or decoding, but for measuring
 *  time and quality, there in nothing specific to XviD in these
 *
 *****************************************************************************/

/* Return time elapsed time in miliseconds since the program started */
static double
msecond()
{
/*
#ifndef WIN32
	struct timeval tv;

	gettimeofday(&tv, 0);
	return (tv.tv_sec * 1.0e3 + tv.tv_usec * 1.0e-3);
#else
	clock_t clk;

	clk = clock();
	return (clk * 1000 / CLOCKS_PER_SEC);
#endif
*/
	return 0;
}

/*****************************************************************************
 *                             Usage message
 *****************************************************************************/


/*****************************************************************************
 *                       Input and output functions
 *
 *      the are small and simple routines to read and write PGM and YUV
 *      image. It's just for convenience, again nothing specific to XviD
 *
 *****************************************************************************/

static int
read_yuvdata(FILE * handle,
			 unsigned char *image)
{

	if (fread(image, 1, IMAGE_SIZE(XDIM, YDIM), handle) !=
		(unsigned int) IMAGE_SIZE(XDIM, YDIM))
		return (1);
	else
		return (0);
}

/*****************************************************************************
 *     Routines for encoding: init encoder, frame step, release encoder
 ****************************************************************************/

/* sample plugin */

#define FRAMERATE_INCR 1001


/* Initialize encoder for first use, pass all needed parameters to the codec */
static int
enc_init(int use_assembler)
{
	int xerr;
	xvid_enc_create_t xvid_enc_create;

	/*------------------------------------------------------------------------
	 * XviD encoder initialization
	 *----------------------------------------------------------------------*/

	/* Version again */
	memset(&xvid_enc_create, 0, sizeof(xvid_enc_create));
//	xvid_enc_create.version = XVID_VERSION;

	/* Width and Height of input frames */
	xvid_enc_create.width = XDIM;
	xvid_enc_create.height = YDIM;
	xvid_enc_create.profile = XVID_PROFILE_AS_L4;

	/* No fancy thread tests */
	//xvid_enc_create.num_threads = 0;

	/* Frame rate - Do some quick float fps = fincr/fbase hack */
#if 1
	if ((ARG_FRAMERATE - (int) ARG_FRAMERATE) < SMALL_EPS) {
		xvid_enc_create.fincr = 1;
		xvid_enc_create.fbase = (int) ARG_FRAMERATE;
	} else {
		xvid_enc_create.fincr = FRAMERATE_INCR;
		xvid_enc_create.fbase = (int) (FRAMERATE_INCR * ARG_FRAMERATE);
	}
#endif

	/* Maximum key frame interval */
    if (ARG_MAXKEYINTERVAL > 0) {
        xvid_enc_create.max_key_interval = ARG_MAXKEYINTERVAL;
    }else {
	    xvid_enc_create.max_key_interval = (int) ARG_FRAMERATE *10;
    }

	/* Bframes settings */
//	xvid_enc_create.max_bframes = ARG_MAXBFRAMES;
//	xvid_enc_create.bquant_ratio = ARG_BQRATIO;
//	xvid_enc_create.bquant_offset = ARG_BQOFFSET;

	/* Dropping ratio frame -- we don't need that */
	xvid_enc_create.frame_drop_ratio = 0;

	/* Global encoder options */
	xvid_enc_create.global = 0;

	/* I use a small value here, since will not encode whole movies, but short clips */
	xerr = xvid_encore(NULL, XVID_ENC_CREATE, &xvid_enc_create, NULL);

	/* Retrieve the encoder instance from the structure */
	enc_handle = xvid_enc_create.handle;

	return (xerr);
}

static int
enc_stop()
{
	int xerr;

	/* Destroy the encoder instance */
	xerr = xvid_encore(enc_handle, XVID_ENC_DESTROY, NULL, NULL);

	return (xerr);
}

static int
enc_main(unsigned char *image,
		 unsigned char *bitstream,
		 int *key,
		 int *stats_type,
		 int *stats_quant,
		 int *stats_length,
		 int sse[3])
{
	int ret;

	xvid_enc_frame_t xvid_enc_frame;
	xvid_enc_stats_t xvid_enc_stats;

	/* Version for the frame and the stats */
	memset(&xvid_enc_frame, 0, sizeof(xvid_enc_frame));
//	xvid_enc_frame.version = XVID_VERSION;

	memset(&xvid_enc_stats, 0, sizeof(xvid_enc_stats));
//	xvid_enc_stats.version = XVID_VERSION;

	/* Bind output buffer */
	xvid_enc_frame.bitstream = bitstream;
	xvid_enc_frame.length = -1;

	/* Initialize input image fields */
	if (image) {
		xvid_enc_frame.input.plane[0] = image;
		xvid_enc_frame.input.csp = XVID_CSP_I420;
		xvid_enc_frame.input.stride[0] = XDIM;
	} else {
		xvid_enc_frame.input.csp = XVID_CSP_NULL;
	}

	/* Set up core's general features */
	
	xvid_enc_frame.vol_flags = 0;
	
	//xvid_enc_frame.vol_flags |= XVID_VOL_MPEGQUANT;

	/* Set up core's general features */
	xvid_enc_frame.vop_flags = vop_presets[ARG_QUALITY];

    if (ARG_VOPDEBUG) {
        xvid_enc_frame.vop_flags |= XVID_VOP_DEBUG;
    }

	/* Frame type -- let core decide for us */
	xvid_enc_frame.type = XVID_TYPE_AUTO;

	/* Force the right quantizer -- It is internally managed by RC plugins */
	xvid_enc_frame.quant = ARG_QUANT;

	/* Set up motion estimation flags */
	xvid_enc_frame.motion = motion_presets[ARG_QUALITY];
#if 0
	if (ARG_GMC)
		xvid_enc_frame.motion |= XVID_ME_GME_REFINE;

	if (ARG_QPEL)
		xvid_enc_frame.motion |= XVID_ME_QUARTERPELREFINE16;
	if (ARG_QPEL && (xvid_enc_frame.vop_flags & XVID_VOP_INTER4V))
		xvid_enc_frame.motion |= XVID_ME_QUARTERPELREFINE8;
#endif
	/* We don't use special matrices */
	/* Encode the frame */
	ret = xvid_encore(enc_handle, XVID_ENC_ENCODE, &xvid_enc_frame,
					  &xvid_enc_stats);

	*key = (xvid_enc_frame.out_flags & XVID_KEYFRAME);
	*stats_type = xvid_enc_stats.type;
	*stats_quant = xvid_enc_stats.quant;
	*stats_length = xvid_enc_stats.length;
	sse[0] = xvid_enc_stats.sse_y;
	sse[1] = xvid_enc_stats.sse_u;
	sse[2] = xvid_enc_stats.sse_v;

	return (ret);
}
