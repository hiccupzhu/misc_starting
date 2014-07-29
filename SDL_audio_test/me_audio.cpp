#include <SDL/SDL.h>
#include <unistd.h>

typedef struct {
	SDL_AudioSpec spec;
	Uint8   *sound;			/* Pointer to wave data */
	Uint32   soundlen;		/* Length of wave data */
	int      soundpos;		/* Current play position */
} WAVE;

WAVE wave;

static int count = 0;
void SDLCALL fillerup(void *unused, Uint8 *stream, int len)
{
	printf("[%s] count = %d\n",__func__,++count);
	Uint8 *waveptr;
	int    waveleft;

	/* Set up the pointers */
	waveptr = wave.sound + wave.soundpos;
	waveleft = wave.soundlen - wave.soundpos;

	/* Go! */
	while ( waveleft <= len ) {
		/* Process samples */
		Uint8 process_buf[waveleft];
		SDL_memcpy(process_buf, waveptr, waveleft);
//		process_audio(process_buf, waveleft);

		// play the end of the audio
		SDL_memcpy(stream, process_buf, waveleft);
		stream += waveleft;
		len -= waveleft;

		// ready to repeat play the audio
		waveptr = wave.sound;
		waveleft = wave.soundlen;
		wave.soundpos = 0;
	}

	/* Process samples */
	Uint8 process_buf[len];
	SDL_memcpy(process_buf, waveptr, len);
	//process_audio(process_buf, len);

	// play the processed samples
	SDL_memcpy(stream, process_buf, len);
	wave.soundpos += len;
}

int main(int argc,char* argv[])
{
	if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		return(1);
	}

	FILE *fp = fopen("hello.wav","rb");
	fseek(fp,   0,   SEEK_END);
	int fileSize = ftell(fp);
	fseek(fp,0,SEEK_SET);

	uint8_t* buffer = (uint8_t*)malloc(fileSize);

	int res = fread(buffer,1,fileSize,fp);
	if(res <= 0)
	{
		printf("fread failed\n");
	}

	wave.spec.channels = 2;
	wave.spec.format = AUDIO_S16LSB;
	wave.spec.freq = 22050;
	wave.spec.samples = 4096;

	wave.sound = buffer;
	wave.soundpos = 0;
	wave.soundlen = fileSize;

	wave.spec.callback = fillerup;

	if ( SDL_OpenAudio(&wave.spec, NULL) < 0 )
	{
		return -1;
	}
	SDL_PauseAudio(0);  //���ſ�ʼ
	sleep(10);    //wav�ļ�����ʱ��(��)
	SDL_CloseAudio();   //ֹͣ����
	SDL_FreeWAV(wave.sound);
	SDL_Quit();

	fclose(fp);
	if(buffer)
	{
		free(buffer);
		buffer = NULL;
	}
	return 0;
}

