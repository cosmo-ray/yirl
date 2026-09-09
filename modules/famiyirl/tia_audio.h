#ifndef TIA_AUDIO_H
#define TIA_AUDIO_H

/**
 * TIA audio emulation (Atari 2600), based on Ron Fries / MAME.
 * Header-only: everything is static, include it in a single .c.
 *
 * - In a tcc module (Y_INSIDE_TCC defined by the manager):
 *   minimal SDL declarations, symbols exported by core/script/tcc-syms.c.
 * - In a regular program: real SDL2 headers
 *   (sdl2-config --cflags --libs).
 *
 * Audio clock: 3.58Mhz / 114 = 31400hz, ie. 2 ticks per scanline
 * (228 color clocks). Registers AUDC0..AUDV1 ($15..$1a) are owned by
 * the caller (the famiyirl TIA emulation): pass their state to
 * tia_audio_gen()/tia_audio_tick_chan().
 */

#ifdef Y_INSIDE_TCC
/* minimal declarations for tcc */
#include <stddef.h>
int printf(const char *fmt, ...);

typedef unsigned char Uint8;
typedef unsigned short Uint16;
typedef unsigned int Uint32;
typedef Uint32 SDL_AudioDeviceID;

typedef struct SDL_AudioSpec {
	int freq;
	Uint16 format;
	Uint8 channels;
	Uint8 silence;
	Uint16 samples;
	Uint16 padding;
	Uint32 size;
	void (*callback)(void *userdata, Uint8 *stream, int len);
	void *userdata;
} SDL_AudioSpec;

#define AUDIO_S16SYS 0x8010

extern SDL_AudioDeviceID SDL_OpenAudioDevice(const char *device, int iscapture,
					     const SDL_AudioSpec *desired,
					     SDL_AudioSpec *obtained,
					     int allowed_changes);
extern int SDL_QueueAudio(SDL_AudioDeviceID dev, const void *data, Uint32 len);
extern void SDL_PauseAudioDevice(SDL_AudioDeviceID dev, int pause_on);
extern Uint32 SDL_GetQueuedAudioSize(SDL_AudioDeviceID dev);
extern void SDL_ClearQueuedAudio(SDL_AudioDeviceID dev);
extern const char *SDL_GetError(void);
#else
#include <SDL2/SDL.h>
#endif

#define AU_POLY4_SIZE 15
#define AU_POLY5_SIZE 31
#define AU_POLY9_SIZE 511
#define AU_SAMPLE_RATE 31440
#define AU_GAIN 1092          /* 2 channels * 15 * 1092 < 32768 */
#define AU_QUEUE_MAX 16384    /* max queued bytes, backpressure fast-forward */

static const Uint8 au_bit4[AU_POLY4_SIZE] = {1,1,0,1,1,1,0,0,0,0,1,0,1,0,0};
static const Uint8 au_bit5[AU_POLY5_SIZE] = {
	0,0,1,0,1,1,0,0,1,1,1,1,1,0,0,0,1,1,0,1,
	1,1,0,1,0,1,0,0,0,0,1 };
/* single pulse per 31-cycle: toggle 1x/31 ticks -> f = 31440/62 */
static const Uint8 au_div31[AU_POLY5_SIZE] = {
	0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0 };
static Uint8 au_bit9[AU_POLY9_SIZE];

static SDL_AudioDeviceID au_dev;

struct tia_au_channel {
	Uint8 p4;
	Uint8 p5;
	Uint16 p9;
	Uint8 div_n_cnt;
	Uint8 div_3_cnt;
	int outvol;
};

static struct tia_au_channel au_ch[2];

static short au_buf[512];
static int au_buf_cnt;

static void tia_audio_flush(void)
{
	if (!au_dev || !au_buf_cnt)
		return;
	/* discard when we run too far ahead (fast-forward) */
	if (SDL_GetQueuedAudioSize(au_dev) > AU_QUEUE_MAX) {
		au_buf_cnt = 0;
		return;
	}
	SDL_QueueAudio(au_dev, au_buf, au_buf_cnt * sizeof au_buf[0]);
	au_buf_cnt = 0;
}

/**
 * advance one audio tick on channel c (0 or 1); audc/audf/audv are the
 * current TIA register values. output accumulates in ch->outvol and is
 * mixed in tia_audio_gen().
 */
static void tia_audio_tick_chan(int c, Uint8 audc, Uint8 audf, Uint8 audv)
{
	struct tia_au_channel *ch = &au_ch[c];

	/* constant output modes (set to 1) */
	if (audc == 0x0 || audc == 0xb) {
		ch->outvol = audv;
		return;
	}

	Uint8 div_n_max = audf + 1;
	/* div6/div93: base divider is multiplied by 3 */
	if ((audc & 0x0c) == 0x0c && audc != 0x0f)
		div_n_max *= 3;

	/* never go down to 0: 0 means no clock on this tick */
	if (ch->div_n_cnt > 1) {
		--ch->div_n_cnt;
		return;
	}
	ch->div_n_cnt = div_n_max;

	Uint8 prev_bit5 = au_bit5[ch->p5];
	/* p5 serves several purposes, it advances on every clock tick */
	if (++ch->p5 == AU_POLY5_SIZE)
		ch->p5 = 0;

	switch (audc) {
	case 0x1:               /* poly4 */
	case 0x2:               /* div31 -> poly4 */
	case 0x3:               /* poly5 -> poly4 */
		if ((audc == 0x1) ||
		    (audc == 0x2 && au_div31[ch->p5]) ||
		    (audc == 0x3 && au_bit5[ch->p5])) {
			if (++ch->p4 == AU_POLY4_SIZE)
				ch->p4 = 0;
			ch->outvol = au_bit4[ch->p4] ? audv : 0;
		}
		break;
	case 0x4:               /* pure */
	case 0x5:               /* pure */
	case 0x6:               /* div31 -> pure */
	case 0x7:               /* poly5 -> pure */
		if ((audc < 0x6) ||
		    (audc == 0x6 && au_div31[ch->p5]) ||
		    (audc == 0x7 && au_bit5[ch->p5]))
			ch->outvol = ch->outvol ? 0 : audv;
		break;
	case 0x8:               /* poly9 */
		if (++ch->p9 == AU_POLY9_SIZE)
			ch->p9 = 0;
		ch->outvol = au_bit9[ch->p9] ? audv : 0;
		break;
	case 0x9:               /* poly5 */
		ch->outvol = au_bit5[ch->p5] ? audv : 0;
		break;
	case 0xa:               /* div31 -> poly5 */
		if (au_div31[ch->p5])
			ch->outvol = au_bit5[ch->p5] ? audv : 0;
		break;
	case 0xc:               /* div6 pure */
	case 0xd:               /* div6 pure */
	case 0xe:               /* div93 pure */
		if (audc < 0xe || au_div31[ch->p5])
			ch->outvol = ch->outvol ? 0 : audv;
		break;
	case 0xf:               /* poly5 -> div3 -> pure */
		if (au_bit5[ch->p5] != prev_bit5 && !--ch->div_3_cnt) {
			ch->div_3_cnt = 3;
			ch->outvol = ch->outvol ? 0 : audv;
		}
		break;
	}
}

/**
 * generate n samples @31440hz and queue them (built-in backpressure).
 * audc/audf/audv are arrays of 2 elements (channel 0 and 1).
 */
static void tia_audio_gen(int n, const Uint8 audc[2], const Uint8 audf[2],
			  const Uint8 audv[2])
{
	for (int i = 0; i < n; ++i) {
		tia_audio_tick_chan(0, audc[0], audf[0], audv[0]);
		tia_audio_tick_chan(1, audc[1], audf[1], audv[1]);

		au_buf[au_buf_cnt++] =
			(au_ch[0].outvol + au_ch[1].outvol) * AU_GAIN;
		if (au_buf_cnt >= (int)(sizeof au_buf / sizeof au_buf[0]))
			tia_audio_flush();
	}
}

/**
 * drain the queue (fast/debug mode switch, pause, ...)
 */
static void tia_audio_clear(void)
{
	au_buf_cnt = 0;
	if (au_dev)
		SDL_ClearQueuedAudio(au_dev);
}

static void tia_audio_init(void)
{
	/* poly9: LFSR 9 bits, taps 9/5 (like MAME), deterministic */
	unsigned int x = AU_POLY9_SIZE;

	for (int i = 0; i < AU_POLY9_SIZE; ++i) {
		int bit0 = x & 1;
		int bit1 = (x >> 4) & 1;

		au_bit9[i] = x & 1;
		x = (x >> 1) | ((bit0 ^ bit1) << 8);
	}
	au_ch[0] = (struct tia_au_channel){.div_3_cnt = 3};
	au_ch[1] = (struct tia_au_channel){.div_3_cnt = 3};

	au_dev = SDL_OpenAudioDevice(NULL, 0, &(SDL_AudioSpec){
		.freq = AU_SAMPLE_RATE,
		.format = AUDIO_S16SYS,
		.channels = 1,
		.samples = 512,
	}, NULL, 0);
	if (!au_dev) {
		printf("audio open fail: %s\n", SDL_GetError());
		return;
	}
	SDL_PauseAudioDevice(au_dev, 0);
}

#endif /* TIA_AUDIO_H */