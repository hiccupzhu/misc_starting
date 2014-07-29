
/*****************************************************************************
 *  Application notes :
 *		                   
 ************************************************************************/
#include "stdafx.h"
#include "xvid_codec.h"
#include "xvid.h"

#define IMAGE_SIZE(x,y) ((x)*(y)*3)
static void *enc_handle = NULL;
static void *dec_handle = NULL;

/*****************************************************************************
 *                            Quality presets
 ****************************************************************************/

const int ME_tables[] = {
	/* quality 0 */
	0,

	/* quality 1 */
	XVID_ME_ADVANCEDDIAMOND16,

	/* quality 2 */
	XVID_ME_ADVANCEDDIAMOND16 | XVID_ME_HALFPELREFINE16,
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

};
//#define ME_ELEMENTS (sizeof(ME_tables)/sizeof(ME_tables[0]))

const int picture_pixels[] = {
	/* quality 0 */
	0,
	/* quality 1 */
	0,
	/* quality 2 */
	XVID_VOP_HALFPEL,
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
};

int			ARG_SINGLE = 1;
int			ARG_QUALITY = 2;
float		ARG_FRAMERATE = 25.00f;
int			use_assembler = 1;
int			enc_quant=0;
int			XDIM=0;
int			YDIM=0;
#define		FRAMERATE_INCR 1001

/* 第一次使用，需要初始化编码器，把预设参数传给编码器 */
int enc_init(ENC_PARAM *pParam)
{
	int xerr;

    xvid_plugin_single_t single;

	xvid_enc_plugin_t plugins[7];
	xvid_gbl_init_t xvid_gbl_init;
	xvid_enc_create_t xvid_enc_create;

	/*------------------------------------------------------------------------
	 * xvid core初始化
	 *----------------------------------------------------------------------*/

	/*设置版本，XVID CORE会检查版本*/
	memset(&xvid_gbl_init, 0, sizeof(xvid_gbl_init));
	xvid_gbl_init.version = XVID_VERSION;
    xvid_gbl_init.debug = 0;//ARG_DEBUG;

	/* 使用汇编优化 */
	if (1) {

#ifdef ARCH_IS_IA64
		xvid_gbl_init.cpu_flags = XVID_CPU_FORCE | XVID_CPU_ASM;
#else
		xvid_gbl_init.cpu_flags = 0;
#endif
	} else {
		xvid_gbl_init.cpu_flags = XVID_CPU_FORCE;
	}

	/* 初始化XVID codec -- 每次编码只能调用一次 */
	xvid_global(NULL, XVID_GBL_INIT, &xvid_gbl_init, NULL);

	/*XVID 版本*/
	memset(&xvid_enc_create, 0, sizeof(xvid_enc_create));
	xvid_enc_create.version = XVID_VERSION;

	/* 输入视频帧的宽和高 */
	xvid_enc_create.width = XDIM = pParam->width;
	xvid_enc_create.height = YDIM = pParam->height;
	xvid_enc_create.profile = XVID_PROFILE_AS_L4;

	xvid_enc_create.plugins = plugins;
	xvid_enc_create.num_plugins = 0;

	//使用码流控制
	if (ARG_SINGLE)
	{
		memset(&single, 0, sizeof(xvid_plugin_single_t));
		single.version = XVID_VERSION;
		single.bitrate = pParam->bitrate;//设置码流大小

		plugins[xvid_enc_create.num_plugins].func = xvid_plugin_single;
		plugins[xvid_enc_create.num_plugins].param = &single;
		xvid_enc_create.num_plugins++;
	}

	xvid_enc_create.num_threads = 0;

	/*视频帧率*/
	if (1) {
		xvid_enc_create.fincr = 1;
		xvid_enc_create.fbase = pParam->framerate;
	} else {
		xvid_enc_create.fincr = FRAMERATE_INCR;
		xvid_enc_create.fbase = (int) (FRAMERATE_INCR * ARG_FRAMERATE);
	}

	/*最大关键帧间隔*/
    if (1) {
		xvid_enc_create.max_key_interval = pParam->max_key_interval;
    }else {
	    xvid_enc_create.max_key_interval = (int) ARG_FRAMERATE *10;
    }

	/* 编码器全局选项 */
	xvid_enc_create.global = 0;
	xerr = xvid_encore(NULL, XVID_ENC_CREATE, &xvid_enc_create, NULL);
	/* 从结构体中取得编码器实例句柄*/
	enc_handle = xvid_enc_create.handle;
	//pParam->handle = xvid_enc_create.handle;
	return (xerr);
}

int
enc_stop()
{
	int xerr;

	/* 销毁编码器实例*/
	xerr = xvid_encore(enc_handle, XVID_ENC_DESTROY, NULL, NULL);

	return (xerr);
}

int 
enc_main(unsigned char *image, unsigned char *bitstream, int *key, int *stats_type,
		 int *stats_quant, int *stats_length)
{
	int ret;

	xvid_enc_frame_t xvid_enc_frame;
	xvid_enc_stats_t xvid_enc_stats;

	memset(&xvid_enc_frame, 0, sizeof(xvid_enc_frame));
	xvid_enc_frame.version = XVID_VERSION;

	memset(&xvid_enc_stats, 0, sizeof(xvid_enc_stats));
	xvid_enc_stats.version = XVID_VERSION;

	/* 初始化码流缓冲区指针 */
	xvid_enc_frame.bitstream = bitstream;
	xvid_enc_frame.length = -1;

	/* 输入图像、格式初始化 */
	if (image) {
		xvid_enc_frame.input.plane[0] = image;
		xvid_enc_frame.input.csp = XVID_CSP_I420;
		xvid_enc_frame.input.stride[0] = XDIM;
	} else {
		xvid_enc_frame.input.csp = XVID_CSP_NULL;
	}

	/* 设置编码器的VOL参数 */
	xvid_enc_frame.vol_flags = 0;

	//if (ARG_STATS)
	//	xvid_enc_frame.vol_flags |= XVID_VOL_EXTRASTATS;

	/* 设置编码器的VOP参数 */
	xvid_enc_frame.vop_flags = picture_pixels[ARG_QUALITY];

	/* 编码帧类型 -- 让编码器自动判决 */
	xvid_enc_frame.type = XVID_TYPE_AUTO;

	/* 强制量化步长，在CORE内使用码流控制管理quant*/
	xvid_enc_frame.quant = enc_quant;

	/* 设置运动估计参数 */
	xvid_enc_frame.motion = ME_tables[ARG_QUALITY];

	/* 编码视频帧 */
	ret = xvid_encore(enc_handle, XVID_ENC_ENCODE, &xvid_enc_frame,
					  &xvid_enc_stats);

	*key = (xvid_enc_frame.out_flags & XVID_KEYFRAME);
	*stats_type = xvid_enc_stats.type;
	*stats_quant = xvid_enc_stats.quant;
	*stats_length = xvid_enc_stats.length;
	//sse[0] = xvid_enc_stats.sse_y;
	//sse[1] = xvid_enc_stats.sse_u;
	//sse[2] = xvid_enc_stats.sse_v;

	return (ret);
}


/* 在第一次使用前，调用该函数初始化解码器*/
int
dec_init(int width, int height)
{
	int ret;

	xvid_gbl_init_t   xvid_gbl_init;
	xvid_dec_create_t xvid_dec_create;
	memset(&xvid_gbl_init, 0, sizeof(xvid_gbl_init_t));
	memset(&xvid_dec_create, 0, sizeof(xvid_dec_create_t));

	/*------------------------------------------------------------------------
	 * xvid core 初始化
	 *----------------------------------------------------------------------*/

	xvid_gbl_init.version = XVID_VERSION;
	
	if(1)/*强制使用汇编优化*/
#ifdef ARCH_IS_IA64
		xvid_gbl_init.cpu_flags = xvid_CPU_FORCE | xvid_CPU_IA64;
#else
	xvid_gbl_init.cpu_flags = 0;
#endif
	else
		xvid_gbl_init.cpu_flags = XVID_CPU_FORCE;

	xvid_gbl_init.debug = 0;

	xvid_global(NULL, 0, &xvid_gbl_init, NULL);

	/*------------------------------------------------------------------------
	 * xvid decoder 初始化 
	 *----------------------------------------------------------------------*/

	/* Version */
	xvid_dec_create.version = XVID_VERSION;

	/*
	 * 图像宽和高，由解码器输入
	 */
	xvid_dec_create.width = width;
	xvid_dec_create.height = height;

	ret = xvid_decore(NULL, XVID_DEC_CREATE, &xvid_dec_create, NULL);

	dec_handle = xvid_dec_create.handle;

	return(ret);
}

xvid_dec_stats_t xvid_dec_stats_l;
xvid_dec_stats_t *xvid_dec_stats = &xvid_dec_stats_l;

/*解码一帧图像*/
int
dec_main(unsigned char *istream,
		 unsigned char *ostream,
		 int istream_size,
		 int width)
{

	int ret;

	xvid_dec_frame_t xvid_dec_frame;

	/* 清零，变量状态确定*/
	memset(&xvid_dec_frame, 0, sizeof(xvid_dec_frame_t));
	memset(xvid_dec_stats, 0, sizeof(xvid_dec_stats_t));

	/* 设置版本 */
	xvid_dec_frame.version = XVID_VERSION;
	xvid_dec_stats->version = XVID_VERSION;

	/* 解码标志清零 */
	xvid_dec_frame.general          = 0;
	
	/*解码后的图像是否做去块效应，可以对Y和UV分开处理*/
	xvid_dec_frame.general |= XVID_DEBLOCKY;// | xvid_DEBLOCKUV;
	/* 输入码流 */
	xvid_dec_frame.bitstream        = istream;
	xvid_dec_frame.length           = istream_size;

	/* 输出图像结构体 */
	xvid_dec_frame.output.plane[0]  = ostream;
	xvid_dec_frame.output.stride[0] = width;
	xvid_dec_frame.output.csp = XVID_CSP_I420;//CSP;

	ret = xvid_decore(dec_handle, XVID_DEC_DECODE, &xvid_dec_frame, xvid_dec_stats);
	
	return(ret);
}

/* 关闭解码器，释放资源 */
int
dec_stop()
{
	int ret;

	ret = xvid_decore(dec_handle, XVID_DEC_DESTROY, NULL, NULL);

	return(ret);
}