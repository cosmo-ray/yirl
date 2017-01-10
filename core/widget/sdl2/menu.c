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
#include "menu.h"
#include "rect.h"
#include "entity.h"

static int sdlRender(YWidgetState *state, int t)
{
  SDLWid *wid = ywidGetRenderData(state, t);
  Entity *entries = yeGet(state->entity, "entries");
  unsigned int   len = yeLen(entries);
  YBgConf cfg;
  int alignementType = YSDL_ALIGN_LEFT;

  if (!ywMenuHasChange(state))
    return 0;

  if (!yeStrCmp(yeGet(state->entity, "text-align"), "center"))
    alignementType = YSDL_ALIGN_CENTER;

  if (ywidBgConfFill(yeGet(state->entity, "background"), &cfg) >= 0)
    sdlFillBg(wid, &cfg);
  else
    sdlFillColorBg(wid, 255, 255, 255, 255);

  if (!sgDefaultFont()) {
    DPRINT_WARN("NO Font Set !");
  }
  for (unsigned int i = 0; i < len; ++i) {
    Entity *entry = yeGet(entries, i);
    SDL_Color color = {0,0,0,255};
    const char *toPrint = yeGetString(yeGet(entry, "text"));
    unsigned int cur = ywMenuGetCurrent(state);
    Entity *destRect = ywRectReCreateInts(0, i * sgGetFontSize() + 1,
					  wid->rect.w, sgGetFontSize() + 1,
					  entry, "$rect");
    SDL_Rect txtR = sdlRectFromRectEntity(destRect);

    sdlPrintText(wid, toPrint, color, txtR, alignementType);
    if (cur == i) {
      color.a = 150;
      sdlDrawRect(txtR, color);
      color.a = 255;
    }
  }
  return 0;
}

static int sdlInit(YWidgetState *wid, int t)
{
  wid->renderStates[t].opac = g_new(SDLWid, 1);
  sdlWidInit(wid, t);
  return 0;
}

int ysdl2RegistreMenu(void)
{
  return ywidRegistreTypeRender("menu", ysdl2Type(),
				sdlRender, sdlInit, sdlWidDestroy);
}
