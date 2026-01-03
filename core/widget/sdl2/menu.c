/*
**Copyright (C) 2015 Matthias Gatto
**
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

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>
#include "sdl-internal.h"
#include "menu.h"
#include "rect.h"
#include "entity.h"

#define REND_PRE_TXT(txt, align, left_pix) do {					\
		destRect = ywRectReCreateInts(				\
			left_pix, y0 + pos * sgGetTxtH() + 1,		\
			wid->rect.w, sgGetTxtH() + 1,			\
			NULL, NULL);					\
		txtR = sdlRectFromRectEntity(destRect);			\
		yeDestroy(destRect);					\
		sdlPrintText(wid, yeGetString(txt), base_color,		\
			     txtR, align);				\
		if (yeLen(txt))						\
			pos += 1 + yeCountCharacters(txt, '\n', -1);	\
	} while (0)


static inline void draw_over_rect(SDLWid *wid, SDL_Rect txtR, SDL_Color color)
{
	color.a = 150;
	sdlDrawRect(wid, txtR, color);
}

static int sdlRend(YWidgetState *state, int t)
{
	SDLWid *wid = ywidGetRenderData(state, t);
	Entity *entries = yeGet(state->entity, "entries");
	unsigned int len = yeLen(entries);
	YBgConf cfg;
	int alignementType = YSDL_ALIGN_LEFT;
	Entity *type = yeGet(state->entity, "mn-type");
	SDL_Color base_color = {0,0,0,255};
	int isPane = 0;
	int pos = 0;
	int cur = ywMenuGetCurrent(state);
	int threshold = ywMenuGetThreshold(state);
	Entity *pre_text = yeGet(state->entity, "pre-text");
	int y0 = threshold;

	wid = sddComputeMargin(state, wid);

	if (!yeStrCmp(type, "panel"))
		isPane = 1;

	if (!yeStrCmp(yeGet(state->entity, "text-align"), "center"))
		alignementType = YSDL_ALIGN_CENTER;


	if (ywidInitBgConf(state->entity, &cfg) >= 0) {
		sdlFillBg(wid, &cfg);
	}

	ywidColorFromString((char *)yeGetString(yeGet(state->entity,
						      "text-color")),
			    &base_color.r, &base_color.g, &base_color.b,
			    &base_color.a);

	if (!sgDefaultFont()) {
		DPRINT_WARN("NO Font Set !");
	}

	if (pre_text) {
		Entity *destRect;
		SDL_Rect txtR;
		int align = yeGetIntAt(state->entity, "pre-txt-no-align");
		int left_pix = yeGetIntAt(state->entity, "pre-txt-left-pix");

		if (align) {
			align = YSDL_ALIGN_LEFT;
		} else {
			align = alignementType;
		}

		if (isPane) {
			DPRINT_ERR("pre_text not supported with panel yet");
		}

		if (yeType(pre_text) == YSTRING) {
			REND_PRE_TXT(pre_text, align, left_pix);
		} else {
			YE_FOREACH(pre_text, txt) {
				REND_PRE_TXT(txt, align, left_pix);
			}
		}
	}

	YE_ARRAY_FOREACH_EXT(entries, entry, it) {
		SDL_Color color = base_color;
		int hiden = yeGetInt(yeGet(entry, "hiden"));
		yeAutoFree Entity *complex_txt = NULL;
		const char *toPrint = yeGetString(yeGet(entry, "text"));
		Entity *destRect;
		Entity *type;
		SDL_Rect txtR;
		int has_loading_bar;
		Entity *slider = yeGet(entry, "slider");
		Entity *subentries = yeGet(entry, "subentries");
		Entity *input_text = yeGet(entry, "input-txt");

		if (hiden)
			continue;
		ywidColorFromString((char *)yeGetString(
					    yeGet(entry, "text-color")),
				    &color.r, &color.g, &color.b, &color.a);

		type = yeGet(entry, "type");
		has_loading_bar = (type && !yeStrCmp(type, "loading-bar"));

		if (isPane) {
			destRect = ywRectReCreateInts(wid->rect.w / len * pos, y0,
						      wid->rect.w / len,
						      sgGetTxtH() + 1,
						      entry, "$rect");
		} else {
			destRect = ywRectReCreateInts(
				0, y0 + pos * sgGetTxtH() + 1, wid->rect.w,
				sgGetTxtH() + 1, entry, "$rect");
		}
		/* ywRectPrint(destRect); */
		txtR = sdlRectFromRectEntity(destRect);
		if (slider) {
			int slider_idx = yeGetIntAt(entry, "slider_idx");
			Entity *cur_option = yeGet(slider, slider_idx);

			if (!cur_option) {
				toPrint = "!! BUGGY SLIDER !!";
			} else {
				complex_txt = yeCreateString(toPrint, NULL, NULL);
				yeStringAdd(complex_txt, "<--");
				yeAdd(complex_txt, yeGet(cur_option, "text"));
				yeStringAdd(complex_txt, "-->");
				toPrint = yeGetString(complex_txt);
			}
		} else if (subentries) {
			if (yeGetIntAt(entry, "is-click")) {
				complex_txt = yeCreateString(" V ", NULL, NULL);
			} else {
				complex_txt = yeCreateString("-> ", NULL, NULL);
			}
			yeAdd(complex_txt, toPrint);
			toPrint = yeGetString(complex_txt);
			if (yeGetIntAt(entry, "is-click")) {
				int idx = yeGetIntAt(entry, "slider_idx");
				if (cur == it.pos && idx == -1)
					draw_over_rect(wid, txtR, color);
				sdlPrintText(wid, toPrint, color, txtR, alignementType);
				for (int i = 0, len = yeLen(subentries); i < len; ++i) {
					Entity *sub = yeGet(subentries, i);

					++pos;
					if (isPane) {
						txtR.x = wid->rect.w / len * pos;
						txtR.y = y0;
						txtR.w = wid->rect.w / len;
						txtR.h = sgGetTxtH() + 1;
					} else {
						txtR.x = 0;
						txtR.y = y0 + pos * sgGetTxtH() + 1;
						txtR.w = wid->rect.w;
						txtR.h = sgGetTxtH() + 1;
					}
					if (i == idx)
						draw_over_rect(wid, txtR, color);
					sdlPrintText(wid, yeGetStringAt(sub, "text"),
						     color, txtR, alignementType);
				}
				goto next;
			}
		} else if (has_loading_bar) {
			char lb[17] = {[0] = '[', [15] = ']', [16] = 0};
			int barPercent = yeGetInt(
				yeGet(entry, "loading-bar-%"));
			const char *separator = yeGetString(
				yeGet(entry, "loading-bar-sep"));
			int barLen;

			if (unlikely(barPercent > 100))
				barPercent = 100;
			else if (unlikely(barPercent < 0))
				barPercent = 0;
			barLen = yuiPercentOf(barPercent, 14);
			memset(lb + 1, '#', barLen);
			memset(lb + 1 + barLen, '.', 14 - barLen);
			if (!separator)
				separator = "";
			if (!toPrint)
				toPrint = "";
			toPrint = (char *)y_strdup_printf(
				"%s%s%s", toPrint, separator, lb);
		}
		sdlPrintText(wid, toPrint, color, txtR, alignementType);
		if (input_text) {
			int rect_pos = sgGetTxtW() * strlen(toPrint);

			sdlDrawRect(wid, (SDL_Rect){rect_pos, txtR.y,
					txtR.w - rect_pos - 4, txtR.h}, (SDL_Color){
					255, 255, 255, 255
				});
			sdlPrintText(wid, yeGetString(input_text), color,
				     (SDL_Rect){rect_pos + 2, txtR.y,
					     txtR.w - rect_pos - 2, txtR.h},
				     alignementType);
		}
		if (cur == it.pos) {
			draw_over_rect(wid, txtR, color);
		}
		if (has_loading_bar)
			free((char *)toPrint);
	next:
		++pos;
	}
	if (ywidBgConfFill(yeGet(state->entity, "foreground"), &cfg) >= 0)
		sdlFillBg(wid, &cfg);
	return 0;
}

static int sdlRender(YWidgetState *state, int t)
{
	return sdlRend(state, t);
}

static int sdlInit(YWidgetState *wid, int t)
{
	wid->renderStates[t].opac = y_new(SDLWid, 1);
	sdlWidInit(wid, t);
	return 0;
}

int ysdl2RegistreMenu(void)
{
	return ywidRegistreTypeRender("menu", ysdl2Type(),
				      sdlRender, sdlInit, sdlWidDestroy);
}
