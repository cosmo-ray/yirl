/*
**Copyright (C) 2019 Matthias Gatto
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
#include "yirl/game.h"
#include "yirl/pos.h"
#include "yirl/video.h"
#include "yirl/rect.h"
#include "yirl/texture.h"
#include "tests.h"

void testVideoSdl2(void)
{
	  yeInitMem();
	  GameConfig cfg;
	  Entity *example = yeCreateArray(NULL, NULL);
	  Entity *actions;
	  YWidgetState *wid;

	  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
	  g_assert(!ygInit(&cfg));

	  yeCreateString("video", example, "<type>");
	  actions = yeCreateArray(example, "actions");
	  yeCreateString("QuitOnKeyDown", actions, NULL);
	  yeCreateString("Mobile Suit Gundam - AMV - The Soldiers of Sorrow.flv",
			 example, "video");

	  wid = ywidNewWidget(example, NULL);
	  g_assert(wid);
	  ywidSetMainWid(wid);
	  ygDoLoop();
	  yeDestroy(example);
	  ygEnd();
	  ygCleanGameConfig(&cfg);
}
