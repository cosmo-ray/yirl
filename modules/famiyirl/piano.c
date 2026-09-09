/**
 * famipiano - piano TIA standalone (terminal).
 *
 * touches:
 *   a-p      waveform (AUDC 0 a 15)
 *   -/=      volume -/+ (sans shift :)
 *   1-9,0    pitch (AUDF 0..9)
 *   q        quitter
 *
 * entree clavier par conio (_kbhit/_getch): fonctionne via la console
 * win32/ConPTY (tmux, ssh...). Le binaire doit etre subsystem console
 * (=> pas de -mwindows), sinon stdin est un pipe et _kbhit ne voit rien.
 *
 * build: make piano   (dans modules/famiyirl/)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tia_audio.h"

#ifdef _WIN32
#include <conio.h>
static int piano_getkey(void)
{
	if (!_kbhit())
		return 0;
	return _getch();
}
#else
#include <unistd.h>
#include <fcntl.h>
static int piano_getkey(void)
{
	char c;
	int flags = fcntl(0, F_GETFL);

	fcntl(0, F_SETFL, flags | O_NONBLOCK);
	int ret = read(0, &c, 1);
	fcntl(0, F_SETFL, flags);
	return ret == 1 ? (unsigned char)c : 0;
}
#endif

static const char *wave_names[] = {
	"set to 1",       "4-bit poly",     "div15 -> poly4", "poly5 -> poly4",
	"pure tone div2", "pure tone div2", "div31 pure",     "poly5 -> div2",
	"poly9 noise",    "poly5 noise",    "div31 -> poly5", "set last 4 to 1",
	"div6 pure",      "div6 pure",      "div93 pure",     "poly5 div3",
};

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	setbuf(stdout, NULL);

	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return 1;
	}
	tia_audio_init();

	Uint8 cur_audc = 4;
	Uint8 cur_audf = 10;
	Uint8 cur_audv = 10;

	printf("famipiano!\n"
	       "  a-p: waveform   -/=: volume   1-9,0: pitch   q: quit\n");

	unsigned long last_ticks = SDL_GetTicks();
	double sample_acc = 0;
	int running = 1;

	while (running) {
		SDL_Delay(10);

		/* generation audio en temps reel, a chaque tour */
		unsigned long now = SDL_GetTicks();

		sample_acc += (now - last_ticks) * AU_SAMPLE_RATE / 1000.;
		last_ticks = now;
		int n = (int)sample_acc;

		if (n > 0) {
			/* canal 1 muet */
			Uint8 audc[2] = {cur_audc, 0};
			Uint8 audf[2] = {cur_audf, 0};
			Uint8 audv[2] = {cur_audv, 0};

			tia_audio_gen(n, audc, audf, audv);
			sample_acc -= n;
		}

		int key = piano_getkey();

		if (!key)
			continue;
		switch (key) {
		case 'a' ... 'p':
			cur_audc = key - 'a';
			break;
		case '-':
		case '_':
			cur_audv = cur_audv > 0 ? cur_audv - 1 : 0;
			break;
		case '=':
		case '+':
			cur_audv = cur_audv < 15 ? cur_audv + 1 : 15;
			break;
		case '0' ... '9':
			cur_audf = key == '0' ? 0 : key - '0';
			break;
		case 'q':
		case 'Q':
			running = 0;
			continue;
		default:
			key = '-';
			break;
		}
		printf("touche %c: wave %x (%s), vol %2d, "
		       "AUDF %2d (%.0f hz)\n",
		       key, cur_audc, wave_names[cur_audc],
		       cur_audv, cur_audf,
		       AU_SAMPLE_RATE / ((cur_audf + 1) * 2.));
	}

	tia_audio_clear();
	SDL_Quit();
	return 0;
}
