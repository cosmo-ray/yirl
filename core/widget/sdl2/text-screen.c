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
#include <glib.h>
#include "sdl-internal.h"
#include "widget.h"
#include "text-screen.h"
#include "entity.h"

static int sdlRender(YWidgetState *state, int t)
{
  SDLWid *wid = ywidGetRenderData(state, t);
  const char *toPrint = ywTextScreenText(state->entity);
  YBgConf cfg;
  SDL_Color color = {0, 0, 0, 255};
  int alignementType = YSDL_ALIGN_LEFT;
  Entity *cursor = yeGet(state->entity, "cursor");

  if (ywidBgConfFill(yeGet(state->entity, "background"), &cfg) >= 0)
    sdlFillBg(wid, &cfg);
  if (unlikely(!toPrint))
	  return 0;
  if (!yeStrCmp(yeGet(state->entity, "text-align"), "center"))
    alignementType = YSDL_ALIGN_CENTER;
  ywidColorFromString((char *)yeGetString(yeGet(state->entity, "text-color")),
		      &color.r, &color.g, &color.b, &color.a);

  if (!sgDefaultFont()) {
    DPRINT_WARN("NO Font Set !");
    return 0;
  }
  int threshold = yeGetIntAt(state->entity, "text-threshold");
  SDL_Rect txtR = {0, threshold,
		   wid->rect.w, wid->rect.h};
  sdlPrintText(wid, toPrint, color, txtR, alignementType);
  if (cursor) {
    int c_pos = yeGetInt(cursor);
    int32_t f_sw = sgGetTxtW();
    int32_t f_sh = sgGetTxtH();
    SDL_Rect rect = {f_sw * c_pos, threshold, 2, f_sh};
    SDL_Color color = {0, 0, 0, 255};

    sdlDrawRect(wid, rect, color);
  }
  if (ywidBgConfFill(yeGet(state->entity, "foreground"), &cfg) >= 0)
    sdlFillBg(wid, &cfg);
  return 0;
}

static int sdlInit(YWidgetState *wid, int t)
{
  wid->renderStates[t].opac = g_new(SDLWid, 1);
  sdlWidInit(wid, t);
  return 0;
}

int ysdl2RegistreTextScreen(void)
{
  return ywidRegistreTypeRender("text-screen", ysdl2Type(),
				sdlRender, sdlInit, sdlWidDestroy);
}
