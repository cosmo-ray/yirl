/**        DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *                   Version 2, December 2004
 *
 * Copyright (C) 2019 Matthias Gatto <uso.cosmo.ray@gmail.com>
 *
 * Everyone is permitted to copy and distribute verbatim or modified
 * copies of this license document, and changing it is allowed as long
 * as the name is changed.
 *
 *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 *  TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
 *
 *  0. You just DO WHAT THE FUCK YOU WANT TO.
 */

#include <yirl/entity.h>
#include <yirl/events.h>
#include <yirl/canvas.h>
#include <yirl/entity-script.h>
#include <yirl/game.h>
#include <yirl/timer.h>

union mem {
	uint16_t w;
	struct {
		uint8_t l;
		uint8_t h;
	};
	uint8_t b;
};

#define MK_REG(r) union {			\
		uint16_t r##x;			\
		struct {			\
			uint8_t r##l;		\
			uint8_t r##h;		\
		};				\
	}

struct regs {
	union {
		struct {
			MK_REG(a);
			MK_REG(b);
			MK_REG(c);
			MK_REG(d);
			uint16_t di;
			uint16_t si;
			uint16_t ds;
			uint16_t es;
			uint16_t flag; /* last operation result*/
		};
		uint16_t buf_16[16];
		uint8_t buf_8[32];
	};
};

struct state_8086 {
	Entity *e;
	void (*set_mem)(struct state_8086 *, int32_t, int16_t);
	char mem[0xfffff]; /* 2n bits of mems */
	Entity *char_ent_map;
	YTimer timer;
};

uint32_t to_vga_icolor[0x10] = {
	0, 0x0000aa, 0x00aa00, 0x00aaaa ,0xaa0000,
	0xaa00aa, 0xaa5500, 0xaaaaaa, 0x555555, 0x5555ff,
	0x55ff55,  0x55ffff, 0xff5555, 0xff55ff, 0xffff55,
	0xffffff
};

const char *to_vga_strcolor[0x10] = {
	"rgba: 0 0 0 255", "rgba: 0 0 170 255", "rgba: 0 170 0 255",
	"rgba: 0 170 170 255", "rgba: 170 0 0 255", "rgba: 170 0 170 255",
	"rgba: 170 85 0 255", "rgba: 170 170 170 255", "rgba: 85 85 85 255",
	"rgba: 85 85 255 255", "rgba: 85 255 85 255", "rgba: 85 255 255 255",
	"rgba: 255 85 85 255","rgba: 255 85 255 255", "rgba: 255 255 85 255",
	"rgba: 255 255 255 255"
};

static inline char *to_vga_strcharset(int c)
{
	if (!c)
		return " ";
	if (c >= 0x20 && c <= 0x7e) {
		/* +1 becase I'm tired and not sure */
		static char rets[0x7e - 0x20 + 1][2];

		rets[c - 0x20][0] = c;
		/* printf("convert %d\n", c); */
		return rets[c - 0x20];
	}
	return "?";
}

#define ORIG_CHAR_H 16
#define ORIG_CHAR_W 9
#define NB_CHAR_H 25
#define NB_CHAR_W 80

#define ORIG_H (ORIG_CHAR_H * NB_CHAR_H)
#define ORIG_W (ORIG_CHAR_W * NB_CHAR_W)

#include "charset.h"

static inline uint8_t *to_vga_imgcharset(int c)
{
	if (c == 2) {
		return head_charset2;
	} else if (c == 1) {
		return head_charset1;
	} else if (!c || c == ' ') {
		return blank;
	} else if (c == 0xb0) {
		return char_b0;
	} else if (c == 0x1e) {
		return up_arrow;
	} else if (c == 0x13) {
		return double_exclamation_mark;
	}
	return NULL;
}

static inline void color_txt_video_scan(struct state_8086 *s, int32_t pos,
					int16_t l)
{
	static int should_refresh;
	int wid_h;
	int wid_w;
	int ch_h;
	int ch_w;

	/* printf("color text scan 0x%x\n", pos); */
	if (!(pos >= 0xb8000 && pos < 0xb8fa0))
		return;

	/* we write word to screen, not byte */
	if (pos & 1)
		pos -= 1;

	wid_w = ywRectW(yeGet(s->e, "wid-pix"));
	wid_h = ywRectH(yeGet(s->e, "wid-pix"));
	ch_h = wid_h * ORIG_CHAR_H / ORIG_H;
	ch_w = wid_w * ORIG_CHAR_W / ORIG_W;
	/* printf("screen: %d %d\nch: %d %d\n", wid_w, wid_h, ch_w, ch_h); */

	for (; l > 0; pos += 2, l -= 2) {
		int r_pos = pos - 0xb8000;
		int x = r_pos / 2;
		int y = x / 80;
		char *mem = s->mem;
		Entity *elem;
		Entity *tmp;
		union mem *di = (union mem *)&mem[pos];
		const uint8_t *imgchar;
		int rx, ry;

		YE_NEW(String, str, to_vga_strcharset(di->l));

		x = x % 80;
		rx = x * ch_w;
		ry = y * ch_h;
		/* printf("(%d)write (%x)%x (%x)'%s' to screen at %d %d\n", */
		/*        l, di->h, to_vga_icolor[di->h], */
		/*        di->l, to_vga_strcharset(di->l), x, y); */

		if (imgchar = to_vga_imgcharset(di->l)) {
			YE_NEW(Array, nfo);
			int32_t bg, fg;

			bg = to_vga_icolor[(di->h & 0xf0) >> 4];
			fg = to_vga_icolor[di->h & 0x0f];
			ywSizeCreate(ORIG_CHAR_W, ORIG_CHAR_H, nfo, NULL);
			yeCreateInt(bg, nfo, NULL); /* backgroung */
			yeCreateInt(fg, nfo, NULL); /* forgroung */
			elem = ywCanvasNewBicolorImg(s->e, rx, ry, imgchar,
						     nfo);
			yeAutoFree Entity *size = ywSizeCreate(ch_w, ch_h,
							       NULL, NULL);
			ywCanvasForceSize(elem, size);

		} else {
			const char *bg, *fg;

			bg = to_vga_strcolor[(di->h & 0xf0) >> 4];
			fg = to_vga_strcolor[di->h & 0x0f];
			const char *strchar = to_vga_strcharset(di->l);
			yeAutoFree Entity *size;
			Entity *el_txt;
			Entity *el_r;

			elem = yeCreateArray(NULL, NULL);
			el_r = ywCanvasNewRectangle(s->e, rx, ry, ch_w,
						    ch_h, bg);
			el_txt = ywCanvasNewTextExt(s->e, rx, ry, str, fg);
			size = ywSizeCreate(40, 90, NULL, NULL);
			yePushBack(elem, el_r, NULL);
			yePushBack(elem, el_txt, NULL);
		}
		if ((tmp = yeGet(s->char_ent_map, r_pos)) != NULL) {
			if (yeLen(tmp) == 2) {
				ywCanvasRemoveObj(s->e, yeGet(tmp, 0));
				ywCanvasRemoveObj(s->e, yeGet(tmp, 1));
			} else {
				ywCanvasRemoveObj(s->e, tmp);
			}
			yeRemoveChild(s->char_ent_map, r_pos);
		}
		yePushAt(s->char_ent_map, elem, r_pos);
		if (yeLen(elem) == 2) {
			yeDestroy(elem);
		}

	}
}

static inline void empty_video_scan(struct state_8086 *s, int32_t pos, int16_t l)
{
	printf("do nothing in this mode\n");
}

static inline void set_video_mode(struct state_8086 *s, int16_t m)
{
	switch (m) {
	case 2:
		s->set_mem = color_txt_video_scan;
		return;
	default:
		s->set_mem = empty_video_scan;
		return;
	}
}

void scan_ptr_mem(struct state_8086 *state, void *dest, int len)
{
	char *cdest = dest;

	 /* if true we're not in machine memory, so assuming registre */
	if (cdest < state->mem || dest > (state->mem + sizeof(state->mem)))
		return;
	state->set_mem(state, (intptr_t)(cdest - state->mem), len);
}

static inline void write_word(struct state_8086 *s, int32_t pos, uint16_t w)
{
	char *mem = s->mem;
	union mem *di = (union mem *)&mem[pos];

	/* it's store as "big endian" */
	/* if w is 0x1234 we need mem[pos] = 0x12 and mem[pos +1] = 0x34 */
	di->l = w & 0x00ff;
	di->h = (w & 0xff00) >> 8;
	(*s).set_mem(s, pos, 2);
}

static inline void write_byte(struct state_8086 *s, int32_t pos, uint8_t b)
{
	char *mem = s->mem;

	mem[pos] = b;
	/* printf("%x - %x\n", mem[pos], b); */
	(*s).set_mem(s, pos, 1);
}

void *test_wid(int nbArgs, void **args)
{
	Entity *emu = args[0];

	struct state_8086 *s = yeGetDataAt(emu, "state");
	Entity *events;

	printf("8086 tester function %p\n", s);
	write_word(s, 0xb8000, 0x0f64);
	set_video_mode(s, 2);
	write_word(s, 0xb8000, 0x0f64);
	write_byte(s, 0xb8002, 0x4f);
	write_byte(s, 0xb8003, 'e');
	write_word(s, 0xb8004, 0x1d01);
	do {
		ywidRend(ywidGetMainWid());
		events = ywidGenericPollEvent();
	} while (!yevIsKeyDown(events, 'q'));
	return NULL;
}

void *call_func(int nbArgs, void **args)
{
	Entity *emu = args[0];

	yesCall(yeGet(emu, "func"), emu);
	ygTerminate();
	return (void *)ACTION;
}

static void clear_state(struct state_8086 *s)
{
	memset(&s->mem[0xb8000], 0, 0xfa0);
	ywCanvasClear(s->e);
	yeClearArray(s->char_ent_map);
}

/* I should not include this file, but the way I handle tcc make it hard to use 2 file like I would by calling gcc */
#include "asm.c"


static void destroy_state(void *arg)
{
	struct state_8086 *s = arg;

	yeDestroy(s->char_ent_map);
	s->char_ent_map = NULL;
	printf("destroy 8086\n");
	free(s);
}

void *init_8086(int nbArgs, void **args)
{
	Entity *emu = args[0];
	void *ret;
	Entity *data;
	struct state_8086 *s;

	ret = ywidNewWidget(emu, "canvas");

	YEntityBlock { emu.background = "rgba: 0 0 0 255"; }
	if (yeGet(emu, "asm")) {
		YEntityBlock { emu.action = call_asm; }
	} else if (yeGet(emu, "func")) {
		YEntityBlock { emu.action = call_func; }
	}
	s = malloc(sizeof(*s));
	s->set_mem = empty_video_scan;
	s->e = emu;
	s->char_ent_map = yeCreateArray(NULL, NULL);
	YTimerReset(&s->timer);
	data = yeCreateData(s, emu, "state");
	yeSetDestroy(data, destroy_state);
	return ret;
}

void *mod_init(int nbArg, void **args)
{
	Entity *mod = args[0];
	Entity *init;

	yuiRandInit();
	init = yeCreateArray(NULL, NULL);
	YEntityBlock {
		init.name = "8086";
		init.callback = init_8086;
		mod.name = "8086-emu";
		mod.starting_widget = "test_8086";
		mod.test_8086 = [];
		mod.test_8086["<type>"] = "8086";
		mod.test_8086.func = test_wid;
	}
	ywidAddSubType(init);
	return mod;
}
