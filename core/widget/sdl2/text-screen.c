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


/* That's ugly and we should detect it in the init function of SDL */
#define CARACT_PER_LINE 70

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>
#include <glib.h>
#include "sdl-internal.h"
#include "widget.h"
#include "entity.h"



static int sdlRender(YWidgetState *state, int t)
{
  SDLWid *wid = ywidGetRenderData(state, t);
  const char *toPrint = yeGetString(yeGet(state->entity, "text"));
  YBgConf cfg;
  SDL_Color color;
  SDL_Renderer *renderer = sgRenderer();

  color.r = 0;
  color.g = 0;
  color.b = 0;
  color.a = 250;

  if (ywidBgConfFill(yeGet(state->entity, "background"), &cfg) >= 0)
    sdlFillBg(wid, &cfg);

  SDL_Rect      rect = wid->rect;
  SDL_RenderDrawRect(renderer, &rect);

  if (!sgDefaultFont()) {
    DPRINT_WARN("NO Font Set !");
    return 0;
  }
  sdlPrintText(wid, toPrint, CARACT_PER_LINE, color, 0, 0);
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
