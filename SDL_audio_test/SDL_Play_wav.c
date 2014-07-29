//�����ʱ��Ҫ����SDL���ļ�
//eg:   gcc  SDL_Play_wav.c -o SDL_Play_wav -lc -lSDL
#include <SDL/SDL.h>
#include <SDL/SDL_main.h>

static double volume = 1.0f; 
struct {
	SDL_AudioSpec spec;
	Uint8   *sound;			/* Pointer to wave data */
	Uint32   soundlen;		/* Length of wave data */
	int      soundpos;		/* Current play position */
} wave;
 
/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void quit(int rc)
{
	SDL_Quit();
	exit(rc);
}
 
/* Process the audio buffer, here only adjust the volume. */
void process_audio(Uint8 *audio_buf, int buf_size) {
	int i;
    switch (wave.spec.format) {
    case AUDIO_U8: {
        // unsigned 8-bit sample
        Uint8 *ptr = (Uint8 *)audio_buf;
        if (wave.spec.channels == 1) {
            // process mono audio
            for (i=0; i<buf_size; i += sizeof(Uint8)) {
                double tmp = (*ptr) * volume;
                tmp = tmp > 255.0 ? 255.0 : tmp;
                *ptr = (Uint8)tmp;
                ptr++;
            }
        } else if (wave.spec.channels == 2) {
            // process stero audio
            for (i=0; i<buf_size; i += sizeof(Uint8)*2) {
                // process left channel
                double tmp = (*ptr) * volume;
                tmp = tmp > 255.0 ? 255.0 : tmp;
                *ptr = (Uint8)tmp;
                ptr++;
                
                // process right channel
                tmp = (*ptr) * volume;
                tmp = tmp > 255.0 ? 255.0 : tmp;
                *ptr = (Uint8)tmp;
                ptr++;
            }
        } else {
            fprintf(stderr, "Doesn't support more than 2 channels!\n");
            exit(1);
        }
        break; }
    case AUDIO_S8: {
        // signed 8-bit sample
        Sint8 *ptr = (Sint8 *)audio_buf;
        if (wave.spec.channels == 1) {
            // process mono audio
            for (i=0; i<buf_size; i += sizeof(Sint8)) {
                double tmp = (*ptr) * volume;
                tmp = tmp > 127.0 ? 127.0 : tmp;
                tmp = tmp < -128.0 ? -128.0 : tmp;
                *ptr = (Sint8)tmp;
                ptr++; // note here ptr will move to 8 bits higher
            }
        } else if (wave.spec.channels == 2) {
            // process stero audio
            for (i=0; i<buf_size; i += sizeof(Uint8)*2) {
                // process left channel
                double tmp = (*ptr) * volume;
                tmp = tmp > 127.0 ? 127.0 : tmp;
                tmp = tmp < -128.0 ? -128.0 : tmp;
                *ptr = (Sint8)tmp;
                ptr++;
                
                // process right channel
                tmp = (*ptr) * volume;
                tmp = tmp > 127.0 ? 127.0 : tmp;
                tmp = tmp < -128.0 ? -128.0 : tmp;
                *ptr = (Sint8)tmp;
                ptr++;
            }
        } else {
            fprintf(stderr, "Doesn't support more than 2 channels!\n");
            exit(1);
        }
        break; }
    case AUDIO_U16LSB: {
        // unsigned 16-bit sample, little-endian
        Uint16 *ptr = (Uint16 *)audio_buf;
        if (wave.spec.channels == 1) {
            // process mono audio
            for (i=0; i<buf_size; i += sizeof(Uint16)) {
#if (SDL_BYTE_ORDER == SDL_BIG_ENDIAN)
                Uint16 swap = SDL_Swap16(*ptr);
                double tmp = swap * volume;
                tmp = tmp > 65535.0 ? 65535.0 : tmp;
                swap = (Uint16)tmp;
                *ptr = SDL_Swap16(swap);
                ptr++; // note here ptr will move to 16 bits higher
#else
                double tmp = (*ptr) * volume;
                tmp = tmp > 65535.0 ? 65535.0 : tmp;
                *ptr = (Uint16)tmp;
                ptr++;
#endif
            }
        } else if (wave.spec.channels == 2) {
            // process stero audio
            for (i=0; i<buf_size; i += sizeof(Uint16)*2) {
#if (SDL_BYTE_ORDER == SDL_BIG_ENDIAN)
                // process left channel
                Uint16 swap = SDL_Swap16(*ptr);
                double tmp = swap * volume;
                tmp = tmp > 65535.0 ? 65535.0 : tmp;
                swap = (Uint16)tmp;
                *ptr = SDL_Swap16(swap);
                ptr++;
                
                // process right channel
                swap = SDL_Swap16(*ptr);
                tmp = swap * volume;
                tmp = tmp > 65535.0 ? 65535.0 : tmp;
                swap = (Uint16)tmp;
                *ptr = SDL_Swap16(swap);
                ptr++;
#else
                // process left channel
                double tmp = (*ptr) * volume;
                tmp = tmp > 65535.0 ? 65535.0 : tmp;
                *ptr = (Uint16)tmp;
                ptr++;
                
                // process right channel
                tmp = (*ptr) * volume;
                tmp = tmp > 65535.0 ? 65535.0 : tmp;
                *ptr = (Uint16)tmp;
                ptr++;
#endif
            }
        } else {
            fprintf(stderr, "Doesn't support more than 2 channels!\n");
            exit(1);
        }
        break; }
    case AUDIO_S16LSB: {
        // signed 16-bit sample, little-endian
        Sint16 *ptr = (Sint16 *)audio_buf;
        if (wave.spec.channels == 1) {
            // process mono audio
            for (i=0; i<buf_size; i += sizeof(Sint16)) {
#if (SDL_BYTE_ORDER == SDL_BIG_ENDIAN)
                Sint16 swap = SDL_Swap16(*ptr);
                double tmp = swap * volume;
                tmp = tmp > 32767.0 ? 32767.0 : tmp;
                tmp = tmp < -32768.0 ? -32768.0 : tmp;
                swap = (Sint16)tmp;
                *ptr = SDL_Swap16(swap);
                ptr++;
#else
                double tmp = (*ptr) * volume;
                tmp = tmp > 32767.0 ? 32767.0 : tmp;
                tmp = tmp < -32768.0 ? -32768.0 : tmp;
                *ptr = (Sint16)tmp;
                ptr++;
#endif
            }
        } else if (wave.spec.channels == 2) {
            // process stero audio
            for (i=0; i<buf_size; i += sizeof(Sint16)*2) {
#if (SDL_BYTE_ORDER == SDL_BIG_ENDIAN)
                // process left channel
                Sint16 swap = SDL_Swap(*ptr);
                double tmp = swap * volume;
                tmp = tmp > 32767.0 ? 32767.0 : tmp;
                tmp = tmp < -32768.0 ? -32768.0 : tmp;
                swap = (Sint16)tmp;
                *ptr = SDL_Swap16(swap);
                ptr++;
                
                // process right channel
                swap = SDL_Swap(*ptr);
                tmp = swap * volume;
                tmp = tmp > 32767.0 ? 32767.0 : tmp;
                tmp = tmp < -32768.0 ? -32768.0 : tmp;
                swap = (Sint16)tmp;
                *ptr = SDL_Swap16(swap);
                ptr++;
#else
                // process left channel
                double tmp = (*ptr) * volume;
                tmp = tmp > 32767.0 ? 32767.0 : tmp;
                tmp = tmp < -32768.0 ? -32768.0 : tmp;
                *ptr = (Sint16)tmp;
                ptr++;
                
                // process right channel
                tmp = (*ptr) * volume;
                tmp = tmp > 32767.0 ? 32767.0 : tmp;
                tmp = tmp < -32768.0 ? -32768.0 : tmp;
                *ptr = (Sint16)tmp;
                ptr++;
#endif
            }
        } else {
            fprintf(stderr, "Doesn't support more than 2 channels!\n");
            exit(1);
        }
        break; }
    case AUDIO_U16MSB: {
        // unsigned 16-bit sample, big-endian
        Uint16 *ptr = (Uint16 *)audio_buf;
        if (wave.spec.channels == 1) {
            // process mono audio
            for (i=0; i<buf_size; i += sizeof(Uint16)) {
#if (SDL_BYTE_ORDER == SDL_LIL_ENDIAN)
                Uint16 swap = SDL_Swap16(*ptr);
                double tmp = swap * volume;
                tmp = tmp > 65535.0 ? 65535.0 : tmp;
                swap = (Uint16)tmp;
                *ptr = SDL_Swap16(swap);
                ptr++; // note here ptr will move to 16 bits higher
#else
                double tmp = (*ptr) * volume;
                tmp = tmp > 65535.0 ? 65535.0 : tmp;
                *ptr = (Uint16)tmp;
                ptr++;
#endif
            }
        } else if (wave.spec.channels == 2) {
            // process stero audio
            for (i=0; i<buf_size; i += sizeof(Uint16)*2) {
#if (SDL_BYTE_ORDER == SDL_LIL_ENDIAN)
                // process left channel
                Uint16 swap = SDL_Swap16(*ptr);
                double tmp = swap * volume;
                tmp = tmp > 65535.0 ? 65535.0 : tmp;
                swap = (Uint16)tmp;
                *ptr = SDL_Swap16(swap);
                ptr++;
                
                // process right channel
                swap = SDL_Swap16(*ptr);
                tmp = swap * volume;
                tmp = tmp > 65535.0 ? 65535.0 : tmp;
                swap = (Uint16)tmp;
                *ptr = SDL_Swap16(swap);
                ptr++;
#else
                // process left channel
                double tmp = (*ptr) * volume;
                tmp = tmp > 65535.0 ? 65535.0 : tmp;
                *ptr = (Uint16)tmp;
                ptr++;
                
                // process right channel
                tmp = (*ptr) * volume;
                tmp = tmp > 65535.0 ? 65535.0 : tmp;
                *ptr = (Uint16)tmp;
                ptr++;
#endif
            }
        } else {
            fprintf(stderr, "Doesn't support more than 2 channels!\n");
            exit(1);
        }
        break; }
    case AUDIO_S16MSB: {
        // signed 16-bit sample, big-endian
        Sint16 *ptr = (Sint16 *)audio_buf;
        if (wave.spec.channels == 1) {
            // process mono audio
            for (i=0; i<buf_size; i += sizeof(Sint16)) {
#if (SDL_BYTE_ORDER == SDL_LIL_ENDIAN)
                Sint16 swap = SDL_Swap16(*ptr);
                double tmp = swap * volume;
                tmp = tmp > 32767.0 ? 32767.0 : tmp;
                tmp = tmp < -32768.0 ? -32768.0 : tmp;
                swap = (Sint16)tmp;
                *ptr = SDL_Swap16(swap);
                ptr++;
#else
                double tmp = (*ptr) * volume;
                tmp = tmp > 32767.0 ? 32767.0 : tmp;
                tmp = tmp < -32768.0 ? -32768.0 : tmp;
                *ptr = (Sint16)tmp;
                ptr++;
#endif
            }
        } else if (wave.spec.channels == 2) {
            // process stero audio
            for (i=0; i<buf_size; i += sizeof(Sint16)*2) {
#if (SDL_BYTE_ORDER == SDL_LIL_ENDIAN)
                // process left channel
                Sint16 swap = SDL_Swap(*ptr);
                double tmp = swap * volume;
                tmp = tmp > 32767.0 ? 32767.0 : tmp;
                tmp = tmp < -32768.0 ? -32768.0 : tmp;
                swap = (Sint16)tmp;
                *ptr = SDL_Swap16(swap);
                ptr++;
                
                // process right channel
                swap = SDL_Swap(*ptr);
                tmp = swap * volume;
                tmp = tmp > 32767.0 ? 32767.0 : tmp;
                tmp = tmp < -32768.0 ? -32768.0 : tmp;
                swap = (Sint16)tmp;
                *ptr = SDL_Swap16(swap);
                ptr++;
#else
                // process left channel
                double tmp = (*ptr) * volume;
                tmp = tmp > 32767.0 ? 32767.0 : tmp;
                tmp = tmp < -32768.0 ? -32768.0 : tmp;
                *ptr = (Sint16)tmp;
                ptr++;
                
                // process right channel
                tmp = (*ptr) * volume;
                tmp = tmp > 32767.0 ? 32767.0 : tmp;
                tmp = tmp < -32768.0 ? -32768.0 : tmp;
                *ptr = (Sint16)tmp;
                ptr++;
#endif
            }
        } else {
            fprintf(stderr, "Doesn't support more than 2 channels!\n");
            exit(1);
        }
        break; }
    }
}
 
void SDLCALL fillerup(void *unused, Uint8 *stream, int len)
{
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
		process_audio(process_buf, waveleft);
	
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
	process_audio(process_buf, len);
	
	// play the processed samples
	SDL_memcpy(stream, process_buf, len);
	wave.soundpos += len;
}
 

int PlayWavFile(char *wavfile, int playtime)
{
	/* Load the SDL library */
	if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		return(1);
	}
	
	/* Load the wave file into memory */
	if ( SDL_LoadWAV(wavfile,
			&wave.spec, &wave.sound, &wave.soundlen) == NULL ) {
		fprintf(stderr, "Couldn't load %s: %s\n",
						wavfile, SDL_GetError());
		quit(1);
	}
 
	wave.spec.callback = fillerup;  //���ûص�����

	/* Initialize fillerup() variables */
	if ( SDL_OpenAudio(&wave.spec, NULL) < 0 ) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		SDL_FreeWAV(wave.sound);
		quit(2);
	}
	// start playing 
    printf("Playing audio:%s\n",wavfile); 
	SDL_PauseAudio(0);  //���ſ�ʼ
    sleep(playtime);    //wav�ļ�����ʱ��(��)
	SDL_CloseAudio();   //ֹͣ����
	SDL_FreeWAV(wave.sound);
	SDL_Quit();
	return(0);
}

int main()
{
    PlayWavFile("hello.wav",10);//���� welcome.wav 10��

    return 0;
}

