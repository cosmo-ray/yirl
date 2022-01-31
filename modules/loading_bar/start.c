/*
**Copyright (C) 2022 Matthias Gatto
*
**This program is free software: you can redistribute it and/or modify
**it under the terms of the GNU Lesser General Public License as published by
**the Free Software Foundation, either version 3 of the License, or
**(at your option) any later version.
**
**This program is distributed in the hope that it will be useful,
**but WITHOUT ANY WARRANTY; without even the implied warranty of
**MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**GNU General Public License for more details.
**
**You should have received a copy of the GNU Lesser General Public License
**along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <yirl/all.h>

static int border_len = 5;

static int bar_w = 100;
static int bar_h = 10;

#define MAP_BUF_L 4058

static uint8_t map[MAP_BUF_L];

#define set(tmp, c, l) do {			\
		memset(tmp, c, l);		\
		tmp += l;			\
} while (0);

void *lbCreateLoadingBar(int nbArg, void **args)
{
	Entity *canvas_wid = args[0];
	int x = (intptr_t)args[1];
	int y = (intptr_t)args[2];
	Entity *father = nbArg > 3 ? args[3] : NULL;
	const char *name = nbArg > 4 ? args[4] : NULL;
	yeAutoFree Entity *info = yeCreateArray(NULL, NULL);
	uint8_t *tmp;

	ywSizeCreate(100, 10, info, NULL);
	yeCreateInt(0x000000, info, NULL);
	yeCreateInt(0xffffff, info, NULL);

	memset(map, 0, bar_w);
	tmp = map + bar_w;
	for (int l = 1; l < bar_h - 1; ++l) {
		set(tmp, 0, border_len);
		set(tmp, 1, bar_w - (2 * border_len));
		set(tmp, 0, border_len);
	}
	memset(tmp, bar_w, 0);
	return ywCanvasNewBicolorImg(canvas_wid, x, y, map, info);
}

void *lbSetPercent(int nbArg, void **args)
{
	Entity *entity = args[0];
	int percent = (intptr_t)args[1];
	int tot_pix = bar_w - (2 * border_len);
	yeAutoFree Entity *info = yeCreateArray(NULL, NULL);
	uint8_t *tmp = map;
	int complette_pix = percent * tot_pix / 100;

	ywSizeCreate(100, 10, info, NULL);
	yeCreateInt(0x000000, info, NULL);
	yeCreateInt(0xffffff, info, NULL);

	set(tmp, 0, bar_w);
	for (int l = 1; l < bar_h - 1; ++l) {
		set(tmp, 0, border_len + complette_pix);
		if (tot_pix - complette_pix > 0) {
			set(tmp, 1, (tot_pix - complette_pix));
		}
		set(tmp, 0, border_len);
	}
	memset(tmp, bar_w, 0);
	return (intptr_t)ywCanvasCacheBicolorImg(entity, map, info);
}

void *mod_init(int nbArg, void **args)
{
	Entity *mod = args[0];

	YEntityBlock {
		mod.name = "loading-bar";
		mod.setPercent = lbSetPercent;
		mod.create = lbCreateLoadingBar;
	}
	return (void *)1;
}
