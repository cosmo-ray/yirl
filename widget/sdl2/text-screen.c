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
  const char *img = yeGetString(yeGet(state->entity, "img"));
  int   len = strlen(toPrint);
  SDL_Color color;
  int text_width , text_height;
  int   y = wid->rect.y;
  int   x = wid->rect.x;
  char  buff[CARACT_PER_LINE + 1];  
  SDL_Renderer *renderer = sgRenderer();

  printf("in\n");
  color.r = 0;
  color.g = 0;
  color.b = 0;
  color.a = 250;
  if (img)
    sdlFillImgBg(wid, img);
  else
    sdlFillColorBg(wid, 255, 255, 255, 255);
  SDL_Rect      rect = wid->rect;
  SDL_RenderDrawRect(renderer, &rect);
  SDL_RenderPresent(renderer);

  for (int i = 0; i < len; i += CARACT_PER_LINE)
    {
      if (i + CARACT_PER_LINE > len) {
	  strncpy(buff, toPrint + i, (i + CARACT_PER_LINE) - len);
	  buff[(i + CARACT_PER_LINE) - len] = 0;
	}
      else {
	  strncpy(buff, toPrint + i, CARACT_PER_LINE);
	  buff[CARACT_PER_LINE] = 0;
	}
      SDL_Surface *textSurface = TTF_RenderUTF8_Solid(sgDefaultFont(), buff, color);
      SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, textSurface);
      text_width = textSurface->w;
      text_height = textSurface->h;
      SDL_FreeSurface(textSurface);
      // printf("%s\nx:%d y:%d\nw:%dh:%d\n", txt, x, y, text_width, text_height);
      SDL_Rect renderQuad = { x, y, text_width, text_height };
      SDL_RenderCopy(renderer, text, NULL, &renderQuad);
      SDL_DestroyTexture(text);
      y += text_height;
    }

  SDL_RenderPresent(renderer);
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
