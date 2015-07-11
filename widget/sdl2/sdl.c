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

#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include "sdl-internal.h"
#include "utils.h"
#include "widget.h"

static int type = -1;

static SDL_Global sg;

SDL_Rect      getRect(void)
{
  return (wSurface()->clip_rect);
}

SDL_Surface *wSurface(void)
{
  return (SDL_GetWindowSurface(sg.pWindow));
}

void    ysdl2Destroy(void)
{
  SDL_DestroyWindow(sg.pWindow);
  IMG_Quit();
  TTF_Quit();
  SDL_Quit();
}

int ysdl2Type(void)
{
  return type;
}

int    ysdl2Init(void)
{
  printf("init sdl\n");
  /* Initialisation simple */
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0 ) {
    DPRINT_ERR("Échec de l'initialisation de la SDL (%s)\n",SDL_GetError());
    return -1;
  }

  if(TTF_Init()==-1) {
    DPRINT_ERR("TTF_Init: %s\n", TTF_GetError());
    goto ttf_fail;
  }


  // initializing Flags for PNG Images
  int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_TIF;

  // Simple check of the Flags
  if(!(IMG_Init(imgFlags)&imgFlags)) {
    DPRINT_ERR("SDL_image could not initialize! SDL_image Error: %s\n",
	       IMG_GetError());
    goto img_fail;
  }

  /* Création de la fenêtre */
  sg.pWindow = SDL_CreateWindow("YIRL isn't a rogue like", SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				WIN_W_SIZE,
				WIN_H_SIZE,
				SDL_WINDOW_SHOWN);
  if(!sg.pWindow) {
      DPRINT_ERR("Error to creeate window:: %s\n",SDL_GetError());
      goto win_fail;
  }
  // load font.ttf at size 16 into font
  sg.font=TTF_OpenFont("./gui/SDL2/font/FOO.ttf", 16);
  if(!sg.font) {
    DPRINT_ERR("TTF_OpenFont: %s\n", TTF_GetError());
    goto fail;
  }
  
  // Render for the main windows
  sg.renderer = SDL_CreateRenderer(sg.pWindow, -1, SDL_RENDERER_TARGETTEXTURE);
  if (!sg.renderer) {
    DPRINT_ERR("Get render from window: %s\n", TTF_GetError());
    goto fail;
  }

  // fill the window with a black rectangle
  // SDL_Rect   rect = sg.getRect();

  SDL_RenderClear(sg.renderer);
  SDL_RenderPresent(sg.renderer);
  if (type == -1)
    type = ywidRegistreRender(NULL, NULL, NULL);
  return type;

 fail:
  SDL_DestroyWindow(sg.pWindow);
 win_fail:
  TTF_Quit();
 img_fail:
  IMG_Quit();
 ttf_fail:
  SDL_Quit();
  return -1;
}
