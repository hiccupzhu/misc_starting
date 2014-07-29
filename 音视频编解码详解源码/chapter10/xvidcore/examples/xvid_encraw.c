
/*****************************************************************************
 *  Application notes :
 *		                   
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "xvid.h"

/*****************************************************************************
 *                            Quality presets
 ****************************************************************************/

static const int ME_tables[] = {
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
#define ME_ELEMENTS (sizeof(ME_tables)/sizeof(ME_tables[0]))

static const int picture_pixels[] = {
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
#define VOP_ELEMENTS (sizeof(picture_pixels)/sizeof(picture_pixels[0]))

/*****************************************************************************
 *                     Command line global variables
 ****************************************************************************/

/* Maximum number of frames to encode */
#define ABS_MAXFRAMENR 9999

static int ARG_QUANT = 0;
static int ARG_STATS = 0;
//static int ARG_DUMP = 0;
//static int ARG_LUMIMASKING = 0;
static int ARG_BITRATE = 0;
static int ARG_SINGLE = 1;
//static char *ARG_PASS1 = 0;
//static char *ARG_PASS2 = 0;
static int ARG_QUALITY = ME_ELEMENTS - 1;
static float ARG_FRAMERATE = 25.00f;
static int ARG_MAXFRAMENR = ABS_MAXFRAMENR;
static int ARG_MAXKEYINTERVAL = 0;
static char *ARG_INPUTFILE = NULL;
static int ARG_INPUTTYPE = 0;
static int ARG_SAVEMPEGSTREAM = 0;
//static int ARG_SAVEINDIVIDUAL = 0;
static char *ARG_OUTPUTFILE = NULL;
static int XDIM = 0;
static int YDIM = 0;
//static int ARG_BQRATIO = 150;
//static int ARG_BQOFFSET = 100;
static int ARG_MAXBFRAMES = 0;
//static int ARG_PACKED = 0;
static int ARG_DEBUG = 0;



#define IMAGE_SIZE(x,y) ((x)*(y)*3/2)


//#define MAX(A,B) ( ((A)>(B)) ? (A) : (B) )
#define SMALL_EPS (1e-10)

/****************************************************************************
 *                     Nasty global vars ;-)
 ***************************************************************************/

static int i;

/* the path where to save output */
static char filepath[256] = "./";

/* Internal structures (handles) for encoding and decoding */
static void *enc_handle = NULL;

/*****************************************************************************
 *               Local prototypes
 ****************************************************************************/

/* Prints program usage message */
static void usage();

/* Statistical functions */
static double msecond();

static int read_yuvdata(FILE * handle,
						unsigned char *image);

/* Encoder related functions */
static int XVID_enc_init(int use_assembler);
static int XVID_enc_stop();
static int XVID_enc_main(unsigned char *image,
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

	int totalsize;
	int result;
	int m4v_size;
	int key;
	int stats_type;
	int stats_quant;
	int stats_length;
	int use_assembler = 1;

	int input_num;
	int output_num;


	FILE *in_file = stdin;
	FILE *out_file = NULL;

	printf("\n XVID encoder\n");
	printf("\n Is Encoding Image, Plese wait...\n\n");

/*****************************************************************************
 *                            Command line parsing
 ****************************************************************************/

	for (i = 1; i < argc; i++) {

		 if (strcmp("-w", argv[i]) == 0 && i < argc - 1) {
			i++;
			XDIM = atoi(argv[i]);
		} else if (strcmp("-h", argv[i]) == 0 && i < argc - 1) {
			i++;
			YDIM = atoi(argv[i]);
        } else if (strcmp("-bitrate", argv[i]) == 0 && i < argc - 1) {
			i++;
			ARG_BITRATE = atoi(argv[i]);
		} else if (strcmp("-key_interval", argv[i]) == 0 && i < argc - 1) {
			i++;
			ARG_MAXKEYINTERVAL = atoi(argv[i]);
		} else if (strcmp("-frames", argv[i])==0 && i < argc - 1){
			i++;
			ARG_MAXFRAMENR = atoi(argv[i]);
		} else if (strcmp("-i", argv[i]) == 0 && i < argc - 1) {
			i++; 
			ARG_INPUTFILE = argv[i];
		} else if (strcmp("-o", argv[i]) == 0 && i < argc - 1) {
			ARG_SAVEMPEGSTREAM = 1;
			i++;
			ARG_OUTPUTFILE = argv[i];
		} else {
			usage();
			exit(-1);
		}

	}
	
	ARG_QUALITY = 2;
	ARG_QUANT = 0;
	ARG_FRAMERATE = 25;

/*****************************************************************************
 *                            Arguments checking
 ****************************************************************************/

	if (XDIM <= 0 || XDIM >= 4096 || YDIM <= 0 || YDIM >= 4096) {
		printf("Now , Only support [4096,4096]! \n");
		return (-1);
	}
	
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

	if (ARG_INPUTFILE == NULL || strcmp(ARG_INPUTFILE, "stdin") == 0) {
		in_file = stdin;
	} else {
		{
			in_file = fopen(ARG_INPUTFILE, "rb");
			if (in_file == NULL) {
				fprintf(stderr, "Error opening input file %s\n", ARG_INPUTFILE);
				return (-1);
			}
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
 *                            XVID PART  Start
 ****************************************************************************/


	result = XVID_enc_init(use_assembler);
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

		if (!result) {
			{
				/* read raw data (YUV-format) */
				result = read_yuvdata(in_file, in_buffer);
			}
		}

/*****************************************************************************
 *                       Encode and decode this frame
 ****************************************************************************/

		enctime = msecond();

		m4v_size =
			XVID_enc_main(!result ? in_buffer : 0, mp4_buffer, &key, &stats_type,
					 &stats_quant, &stats_length, sse);
		enctime = msecond() - enctime;
		//enctime -= 20;
		/* Write the Frame statistics */

		printf("  No.%4d: time=%4.0f", !result ? input_num : -1,
			   (float) enctime);

		if (stats_type > 0) {	/* !XVID_TYPE_NOTHING */

			switch (stats_type) {
			case XVID_TYPE_IVOP:
				type = "I";
				break;
			case XVID_TYPE_PVOP:
				type = "P";
				break;
			case XVID_TYPE_SVOP:
				type = "S";
				break;
			default:
				type = "U";
				break;
			}

			printf(" | type=%s, quant= %2d, len= %6d", type, stats_quant+20, stats_length);

#define SSE2PSNR(sse, width, height) ((!(sse))?0.0f : 48.131f - 10*(float)log10((float)(sse)/((float)((width)*(height)))))

			if (ARG_STATS) {
				printf(", psnr y = %2.2f, psnr u = %2.2f, psnr v = %2.2f",
					   SSE2PSNR(sse[0], XDIM, YDIM), SSE2PSNR(sse[1], XDIM / 2,
															  YDIM / 2),
					   SSE2PSNR(sse[2], XDIM / 2, YDIM / 2));

				totalPSNR[0] += SSE2PSNR(sse[0], XDIM, YDIM);
				totalPSNR[1] += SSE2PSNR(sse[1], XDIM/2, YDIM/2);
				totalPSNR[2] += SSE2PSNR(sse[2], XDIM/2, YDIM/2);
			}

		}
#undef SSE2PSNR

		printf("\n");

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

			/* Save ES stream */
			if (ARG_OUTPUTFILE && out_file)
				fwrite(mp4_buffer, 1, m4v_size, out_file);
		}

		input_num++;


	} while (1);



/*****************************************************************************
 *         Calculate totals and averages for output, print results
 ****************************************************************************/
#if 0
	printf("Tot: enctime(ms) =%7.2f,          length(bytes) = %6d\n",
		   totalenctime, (int) totalsize);
#endif

	if (input_num > 0) {
		totalsize /= input_num;
		totalenctime /= input_num;
		totalPSNR[0] /= input_num;
		totalPSNR[1] /= input_num;
		totalPSNR[2] /= input_num;
	} else {
		totalsize = -1;
		totalenctime = -1;
	}

	printf("  Avg: enctime(ms) =%7.2f, fps =%7.2f, length(bytes) = %6d",
		   totalenctime, 1000 / totalenctime, (int) totalsize);

   if (ARG_STATS) {
       printf(", psnr y = %2.2f, psnr u = %2.2f, psnr v = %2.2f",
    		  totalPSNR[0],totalPSNR[1],totalPSNR[2]);
	}
	printf("\n");


/*****************************************************************************
 *                            XVID PART  Stop
 ****************************************************************************/

  release_all:

	if (enc_handle) {
		result = XVID_enc_stop();
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

	return (0);

}


/*****************************************************************************
 *                        "statistical" functions
 *
 *  these are not needed for encoding or decoding, but for measuring
 *  time and quality, there in nothing specific to XVID in these
 *
 *****************************************************************************/

/* Return time elapsed time in miliseconds since the program started */
static double
msecond()
{
#ifndef WIN32
	struct timeval tv;

	gettimeofday(&tv, 0);
	return (tv.tv_sec * 1.0e3 + tv.tv_usec * 1.0e-3);
#else
	clock_t clk;

	clk = clock();
	return (clk * 1000.0 / CLOCKS_PER_SEC);
#endif
}

/*****************************************************************************
 *                             Usage message
 *****************************************************************************/

static void
usage()
{
	printf("Look the readme file!\n");
}

/*****************************************************************************
 *                       Input and output functions
 *
 *      the are small and simple routines to read and write PGM and YUV
 *      image. It's just for convenience, again nothing specific to XVID
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

#define FRAMERATE_INCR 1001


/* Initialize encoder for first use, pass all needed parameters to the codec */
static int
XVID_enc_init(int use_assembler)
{
	int xerr;

    xvid_plugin_single_t single;

	xvid_enc_plugin_t plugins[7];
	xvid_gbl_init_t xvid_gbl_init;
	xvid_enc_create_t xvid_enc_create;

	/*------------------------------------------------------------------------
	 * xvid core initialization
	 *----------------------------------------------------------------------*/

	/* Set version -- version checking will done by xvidcore */
	memset(&xvid_gbl_init, 0, sizeof(xvid_gbl_init));
	xvid_gbl_init.version = XVID_VERSION;
    xvid_gbl_init.debug = ARG_DEBUG;

	/* Do we have to enable ASM optimizations ? */
	if (use_assembler) {

#ifdef ARCH_IS_IA64
		xvid_gbl_init.cpu_flags = XVID_CPU_FORCE | XVID_CPU_ASM;
#else
		xvid_gbl_init.cpu_flags = 0;
#endif
	} else {
		xvid_gbl_init.cpu_flags = XVID_CPU_FORCE;
	}

	/* Initialize XVID core -- Should be done once per __process__ */
	xvid_global(NULL, XVID_GBL_INIT, &xvid_gbl_init, NULL);

	/* Version again */
	memset(&xvid_enc_create, 0, sizeof(xvid_enc_create));
	xvid_enc_create.version = XVID_VERSION;

	/* Width and Height of input frames */
	xvid_enc_create.width = XDIM;
	xvid_enc_create.height = YDIM;
	xvid_enc_create.profile = XVID_PROFILE_AS_L4;

	xvid_enc_create.plugins = plugins;
	xvid_enc_create.num_plugins = 0;

	if (ARG_SINGLE) {
		memset(&single, 0, sizeof(xvid_plugin_single_t));
		single.version = XVID_VERSION;
		single.bitrate = ARG_BITRATE;

		plugins[xvid_enc_create.num_plugins].func = xvid_plugin_single;
		plugins[xvid_enc_create.num_plugins].param = &single;
		xvid_enc_create.num_plugins++;
	}

	/* No fancy thread tests */
	xvid_enc_create.num_threads = 0;

	/* Frame rate - Do some quick float fps = fincr/fbase hack */
	if ((ARG_FRAMERATE - (int) ARG_FRAMERATE) < SMALL_EPS) {
		xvid_enc_create.fincr = 1;
		xvid_enc_create.fbase = (int) ARG_FRAMERATE;
	} else {
		xvid_enc_create.fincr = FRAMERATE_INCR;
		xvid_enc_create.fbase = (int) (FRAMERATE_INCR * ARG_FRAMERATE);
	}

	/* Maximum key frame interval */
    if (ARG_MAXKEYINTERVAL > 0) {
        xvid_enc_create.max_key_interval = ARG_MAXKEYINTERVAL;
    }else {
	    xvid_enc_create.max_key_interval = (int) ARG_FRAMERATE *10;
    }


	/* Global encoder options */
	xvid_enc_create.global = 0;

	/* I use a small value here, since will not encode whole movies, but short clips */
	xerr = xvid_encore(NULL, XVID_ENC_CREATE, &xvid_enc_create, NULL);

	/* Retrieve the encoder instance from the structure */
	enc_handle = xvid_enc_create.handle;

	return (xerr);
}

static int
XVID_enc_stop()
{
	int xerr;

	/* Destroy the encoder instance */
	xerr = xvid_encore(enc_handle, XVID_ENC_DESTROY, NULL, NULL);

	return (xerr);
}

static int
XVID_enc_main(unsigned char *image,
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
	xvid_enc_frame.version = XVID_VERSION;

	memset(&xvid_enc_stats, 0, sizeof(xvid_enc_stats));
	xvid_enc_stats.version = XVID_VERSION;

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

	if (ARG_STATS)
		xvid_enc_frame.vol_flags |= XVID_VOL_EXTRASTATS;

	/* Set up core's general features */
	xvid_enc_frame.vop_flags = picture_pixels[ARG_QUALITY];

	/* Frame type -- let core decide for us */
	xvid_enc_frame.type = XVID_TYPE_AUTO;

	/* Force the right quantizer -- It is internally managed by RC plugins */
	xvid_enc_frame.quant = ARG_QUANT;//0;

	/* Set up motion estimation flags */
	xvid_enc_frame.motion = ME_tables[ARG_QUALITY];

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
