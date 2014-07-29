//#include "stdfx.h"
#include "h264.h"


int FindStartCode (unsigned char *Buf, int zeros_in_startcode)
{
  int info;
  int i;

  info = 1;
  for (i = 0; i < zeros_in_startcode; i++)
    if(Buf[i] != 0)
      info = 0;

  if(Buf[i] != 1)
    info = 0;
  return info;
}

int getNextNal(FILE* inpf, unsigned char* Buf)
{
	int pos = 0;
	while(!feof(inpf) && (Buf[pos++]=fgetc(inpf))==0);

	int StartCodeFound = 0;
	int info2 = 0;
	int info3 = 0;

	while (!StartCodeFound)
	{
		if (feof (inpf))
		{
//			return -1;
			return pos-1;
		}
		Buf[pos++] = fgetc (inpf);
		info3 = FindStartCode(&Buf[pos-4], 3);
		if(info3 != 1)
			info2 = FindStartCode(&Buf[pos-3], 2);
		StartCodeFound = (info2 == 1 || info3 == 1);
	}
	fseek (inpf, -4, SEEK_CUR);
	return pos - 4;
}

void pgm_save(unsigned char *buf,int wrap, int xsize,int ysize,char *filename) 
{
    FILE *f;
    int i;

    f=fopen(filename,"w");
    fprintf(f,"P5\n%d %d\n%d\n",xsize,ysize,255);
    for(i=0;i<ysize;i++)
        fwrite(buf + i * wrap,1,xsize,f);
    fclose(f);
}

int main()
{
	FILE * inpf;
	FILE * outf;
	int nWrite;
	int i;

	char outfile[] = "test.pgm";
	inpf = fopen("test.264", "rb");
	outf = fopen("out.yuv", "wb");

	int nalLen = 0;
	unsigned char* Buf = (unsigned char*)calloc ( 1000000, sizeof(char));
//	int width = 352;
//	int height = 288;

	AVCodec *codec;			  // Codec
    AVCodecContext *c;		  // Codec Context
    AVFrame *picture;		  // Frame	

	avcodec_init(); 
	avcodec_register_all(); 
	codec = avcodec_find_decoder(CODEC_ID_H264);

	if (!codec)  {
		return 0; 
	} 
		//allocate codec context
    c = avcodec_alloc_context(); 

	if(!c){
		return 0;
	}
	//open codec
    if (avcodec_open(c, codec) < 0) {
		return 0; 
	} 
	
	//allocate frame buffer
    picture   = avcodec_alloc_frame();
	if(!picture){
		return 0;
	}

	while(!feof(inpf))
	{
		nalLen = getNextNal(inpf, Buf);

		//try to decode this frame
		int  got_picture, consumed_bytes; 
		consumed_bytes= decode_frame(c, picture, &got_picture, Buf, nalLen); 
		if (*got_picture)                           
	       avctx->frame_number++;

		if(consumed_bytes > 0)
		{
//CS by tiany for test
			for(i=0; i<c->height; i++)
				fwrite(picture->data[0] + i * picture->linesize[0], 1, c->width, outf);
			for(i=0; i<c->height/2; i++)
				fwrite(picture->data[1] + i * picture->linesize[1], 1, c->width/2, outf);
			for(i=0; i<c->height/2; i++)
				fwrite(picture->data[2] + i * picture->linesize[2], 1, c->width/2, outf);			

//			pgm_save(picture->data[0], picture->linesize[0], 
//				c->width, c->height, outfile);
//			pgm_save(picture->data[1], picture->linesize[1], 
//				c->width/2, c->height/2, outfile);
//			pgm_save(picture->data[2], picture->linesize[2], 
//				c->width/2, c->height/2, outfile);
//CE by tiany for test
		}
	}

	if(inpf)
		fclose(inpf);

	if(outf)
		fclose(outf);

	if(c) {
		avcodec_close(c); 
		av_free(c);
		c = NULL;
	} 
	if(picture) {
		av_free(picture);
		picture = NULL;
	}

	if(Buf)
	{
		free(Buf);
		Buf = NULL;
	}

	return 1;
}