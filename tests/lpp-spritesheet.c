/*
**Copyright (C) 2018 Matthias Gatto
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

#include "yirl/game.h"
#include "yirl/entity-script.h"
#include "tests.h"

void testsLpcSpritesheet(void)
{
  yeInitMem();
  GameConfig cfg;
  Entity *gc = yeCreateArray(NULL, NULL);
  Entity *canvas = yeCreateArray(gc, NULL);
  Entity *caracter = yeCreateArray(gc, NULL);
  Entity *texture;
  Entity *obj;
  Entity *mod;
  YWidgetState *wid;
  Entity *actions;

  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));
  mod = ygLoadMod(TESTS_PATH"../modules/Universal-LPC-spritesheet/");
  if (!mod) {
    printf("can't load Universal-LPC-spritesheet\n"
	   "you might need to init that submodule\n");
    goto exit;
  }

  actions = yeCreateArray(canvas, "actions");
  yeCreateString("QuitOnKeyDown", actions, NULL);

  yeCreateString("female", caracter, "sex");
  yeCreateString("light", caracter, "type");
  texture = yesCall(ygGet("lpcs.textureFromCaracter"), caracter);
  g_assert(texture);
  obj = yesCall(ygGet("lpcs.loadCanvas"), canvas, texture, 0, 0, 50, 50);
  g_assert(obj);

  yeCreateString("canvas", canvas, "<type>");
  yeCreateString("rgba: 255 255 255 255", canvas, "background");
  wid = ywidNewWidget(canvas, NULL);
  g_assert(wid);
  ywidSetMainWid(wid);
  ygDoLoop();

 exit:
  yeDestroy(gc);
  ygCleanGameConfig(&cfg);
  ygEnd();
}
