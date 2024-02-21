#include <yirl/all.h>

/* 
 * color palet steal from https://bugzmanov.github.io/nes_ebook/chapter_6_3.html
 * Yes I did convert the rust code to C, I like rust, but if there is one thing I prefer to rust:
 * it's to port rust code to C
 */

int SYSTEM_PALLETE [] = {
   0x808080ff, 0x003DA6ff, 0x0012B0ff, 0x440096ff, 0xA1005Eff,
   0xC70028ff, 0xBA0600ff, 0x8C1700ff, 0x5C2F00ff, 0x104500ff,
   0x054A00ff, 0x00472Eff, 0x004166ff, 0x000000ff, 0x050505ff,
   0x050505ff, 0xC7C7C7ff, 0x0077FFff, 0x2155FFff, 0x8237FAff,
   0xEB2FB5ff, 0xFF2950ff, 0xFF2200ff, 0xD63200ff, 0xC46200ff,
   0x358000ff, 0x058F00ff, 0x008A55ff, 0x0099CCff, 0x212121ff,
   0x090909ff, 0x090909ff, 0xFFFFFFff, 0x0FD7FFff, 0x69A2FFff,
   0xD480FFff, 0xFF45F3ff, 0xFF618Bff, 0xFF8833ff, 0xFF9C12ff,
   0xFABC20ff, 0x9FE30Eff, 0x2BF035ff, 0x0CF0A4ff, 0x05FBFFff,
   0x5E5E5Eff, 0x0D0D0Dff, 0x0D0D0Dff, 0xFFFFFFff, 0xA6FCFFff,
   0xB3ECFFff, 0xDAABEBff, 0xFFA8F9ff, 0xFFABB3ff, 0xFFD2B0ff,
   0xFFEFA6ff, 0xFFF79Cff, 0xD7E895ff, 0xA6EDAFff, 0xA2F2DAff,
   0x99FFFCff, 0xDDDDDDff, 0x111111ff, 0x111111ff
};

int bit_to_col(int bit)
{
	switch (bit) {
	case 1:
		return SYSTEM_PALLETE[0];
	case 2:
		return SYSTEM_PALLETE[0x10];
	case 3:
		return SYSTEM_PALLETE[0x20];
	}
	return 0;
}

void *draw_tile(Entity *wid, Entity *chr_data, int idx, int x, int y)
{
	if (idx >= 512)
		return NULL;
	yeAutoFree Entity *size = yeCreateQuadInt2(8, 8, NULL, NULL);
	Entity *r = ywCanvasNewDrawableImg(wid, x, y, size);
	char *data = yeGetData(chr_data) + idx * 16;

	/* https://www.nesdev.org/wiki/PPU_pattern_tables */
	for (int y = 0; y < 8; ++y) {
		for (int x = 7; x >= 0; --x) {
			int bit = !!(data[y] & (1 << x)) | (!!(data[y + 8] & (1 << x))) << 1;
			int color = bit_to_col(bit);
			if (color)
				ywCanvasDrawableSetPix(r, 7 - x, y, color);
		}
	}
	ywCanvasDrawableFinalyze(r);
	return r;
}

void *ychr_draw_tile(int nbArgs, void **args)
{
	return draw_tile(args[0], args[1], (intptr_t)args[2], (intptr_t)args[3], (intptr_t)args[4]);
}

void *printf_tile(Entity *chr_data, int idx)
{
	if (idx >= 512)
		return NULL;
	char *data = yeGetData(chr_data) + idx * 16;

	/* https://www.nesdev.org/wiki/PPU_pattern_tables */
	for (int y = 0; y < 8; ++y) {
		for (int x = 7; x >= 0; --x) {
			int bit = !!(data[y] & (1 << x)) | (!!(data[y + 8] & (1 << x))) << 1;
			printf("%x", bit);
		}
		printf("\n");
	}
	return NULL;
}

void *ychr_printf_tile(int nbArgs, void **args)
{
	return printf_tile(args[0], (intptr_t)args[1]);
}

void *chr_v_action(int nbArgs, void **args)
{
	printf("chr_v_action\n");
	return NULL;
}

void *chr_v_init(int nbArgs, void **args)
{
	Entity *wid = args[0];
	yeConvert(wid, YHASH);
	yeCreateString("rgba: 40 40 40 255", wid, "background");
	yeCreateFunction("chr_v_action", ygGetTccManager(), wid, "action");
	yeAutoFree Entity *chr = ygFileToEnt(YRAW_FILE_DATA, "file.chr", NULL);
	yePushBack(wid, chr, "chr");
	void *ret = ywidNewWidget(wid, "canvas");
	printf_tile(chr, 0);
	printf_tile(chr, 1);
	printf_tile(chr, 0x10);
	yeAutoFree Entity *size = ywSizeCreate(20, 20, 0, 0);
	Entity *sprite = NULL;

	for (int y = 0; y < 0x10; ++y) {
		for (int x = 0; x < 0x10; ++x) {
			sprite = draw_tile(wid, chr, x + y * 0x10, x * 20, y * 20);
			//ywCanvasHFlip(sprite);
			ywCanvasForceSize(sprite, size);
		}
	}
	for (int y = 0; y < 0x10; ++y) {
		for (int x = 0; x < 0x10; ++x) {
			sprite = draw_tile(wid, chr, (0x10 * 0x10) + x + y * 0x10, 320 + x * 20, y * 20);
			//ywCanvasHFlip(sprite);
			ywCanvasForceSize(sprite, size);
		}
	}
	return ret;
}

void *mod_init(int nbArgs, void **args)
{
	Entity *mod = args[0];
	yeAutoFree Entity *init = yeCreateFunction("chr_v_init", ygGetTccManager(), NULL, NULL);

	ygInitWidgetModule(mod, "chr_viewer", init);
	yeCreateFunction("ychr_draw_tile", ygGetTccManager(), mod, "draw_tile");
	yeCreateFunction("ychr_printf_tile", ygGetTccManager(), mod, "printf_tile");
	ygRegistreFunc(2, "ychr_printf_tile", "ychr_printf_tile");
	ygRegistreFunc(5, "ychr_draw_tile", "ychr_draw_tile");
	return mod;
}
