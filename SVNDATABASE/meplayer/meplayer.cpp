#include <iostream>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <unistd.h>
#include <pthread.h>

#ifdef __cplusplus
#define __STDC_CONSTANT_MACROS
#ifdef _STDINT_H
#undef _STDINT_H
#endif
# include <stdint.h>
#endif


extern "C"
{
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}
#include "matroska_reader_t.h"

using namespace std;

jmp_buf jmp_exit;

size_t url_read(void *handle, void *buf, size_t size, int full_read)
{
//	if(feof((FILE*)handle))
//		longjmp(jmp_exit,1);
	return fread(buf,1,size,(FILE*)handle);
}

int64_t url_seek(void *handle, int64_t offset, int origin)
{
	return fseek((FILE*)handle,offset,origin);
}

int64_t url_tell(void *handle)
{
	return ftell((FILE*)handle);
}

int main(int argc,char* argv[])
{
	avcodec_init();
	av_register_all();

	source_access_callbacks_t *source = new source_access_callbacks_t;
	void* buffer = malloc(0x400000);
	video_parameter_t vpara;
	audio_parameter_t apara;

	cout<<argv[1]<<endl;
	FILE *fp = NULL;
	fp = fopen(argv[1],"rb");
	printf("fp=%X\n",fp);
	if( NULL == fp )
	{
		printf("file opened failed\n");
		exit(1);
	}

	source->m_pHandle = fp;
	source->m_Read = url_read;
	source->m_Seek = url_seek;
	source->m_Tell = url_tell;


	int type = 0;
	void * buf = 0;
	int size = 0;
	int64 pts = 0;
	int64 dts = 0;
	int64 end_pts = 0;
	int au_start = 0;

	DMP::matroska_reader_t mkv;
	int res = mkv.init(source);
	if(res<0){	printf("init fialed\n");goto exit;	}
	res = mkv.get_audio_parameter(&apara,0);
	if(res<0){	printf("get_audio_parameter fialed\n");goto exit;	}
	res = mkv.get_video_parameter(&vpara,0);
	if(res<0){	printf("get_video_parameter fialed\n");goto exit;	}


	static FILE * fpv = fopen("vidoe_stream.bin","wb");
	static FILE * fpa = fopen("audio_stream.bin","wb");
	if(NULL == fpv && NULL == fpa)
	{
		printf("open file failed,exit()\n");
		exit(1);
	}
	while(res >= 0)
	{
		res = mkv.read_next_chunk(type,buffer,size,pts,dts,end_pts,au_start);
		if(type == 0)
		{
			fwrite(buffer,size,1,fpv);
			fflush(fpv);
		}else if(type == 1){
			fwrite(buffer,size,1,fpa);
			fflush(fpa);
		}
	}
	fclose(fpv);
	fclose(fpa);


exit:
	printf("<<<<<<<<<<<<<process exit\n");
	if(buffer)
	{
		delete (uint8_t*)buffer;
		buffer = NULL;
	}
	if(source)
	{
		delete source;
		source = NULL;
	}
	fclose(fp);
	return 0;
}
