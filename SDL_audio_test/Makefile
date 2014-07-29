CC=gcc
CXX= g++
all:
	$(CC)  SDL_Play_wav.c -g -o SDL_Play_wav -lc -lSDL
	$(CXX) me_audio.cpp -g -o me_audio -lSDL
	
clean:
	rm -f me_audio SDL_Play_wav *.o