
#ifndef _ENC_DEC_HEADER_
#define _ENC_DEC_HEADER_

#ifdef __cplusplus
extern "C" {
#endif

/** Structure used for encoder instance creation */
typedef struct	{
	int width;				//编码图像宽度
	int height;				//编码图像高度
	int max_key_interval;	//关键帧间隔
	int framerate;			//编码帧率
	int quant;				//量化步长
	int bitrate;			//码流大小，实现CBR
	void *handle;			//编码器句柄
} ENC_PARAM;

///////////////////////////////////////////////////////////////////////////
int enc_init(ENC_PARAM *enc_param);		//创建编码器
int enc_stop();							//销毁编码器
int enc_main(unsigned char *image,		//编码图像指针
			 unsigned char *bitstream,	//编码后码流指针
			 int *key,					//关键帧标志
			 int *stats_type,			//编码类型
			 int *stats_quant,			//量化步长
			 int *stats_length);		//编码后码流大小

////////////////////////////////////////////////////////////////////////////
int dec_init(int width, int height);	//创建解码器
int dec_stop();							//销毁解码器
int dec_main(unsigned char *istream,	//码流指针
			unsigned char *ostream,		//解码图像的指针
			int istream_size,			//码流长度
			int width);					//图像宽度

////////////////////////////////////////////////////////////////////////////

void SwapBitmap(unsigned char *pInBuf, unsigned char *pOutBuf, int nPicType);

#ifdef __cplusplus
}
#endif


#endif

