SDL_CFLAGS = -I$(MINGW_PREFIX)/include/SDL2 -Dmain=SDL_main
SDL_LIBS   = -L$(MINGW_PREFIX)/lib -lmingw32 -lSDL2main -lSDL2

piano: piano.c tia_audio.h
	$(CC) -std=c23 -Wall -O2 -o $@ piano.c $(SDL_CFLAGS) $(SDL_LIBS)

hello-yirl.ybin: hello-yirl.o
	ld65 -o hello-yirl.ybin -C yirl-cc64.cfg hello-yirl.o

hello-yirl.o: hello-yirl.asm
	ca65 hello-yirl.asm --cpu 6502

clean:
	rm hello-yirl.o hello-yirl.ybin piano piano.exe
