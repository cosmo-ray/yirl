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
  Entity *canvas = yeCreateArray(NULL, NULL);
  Entity *caracter = yeCreateArray(NULL, NULL);
  Entity *clothes;
  Entity *texture = NULL;
  Entity *obj;
  Entity *mod;
  YWidgetState *wid;
  Entity *actions;

  g_assert(!ygInitGameConfig(&cfg, NULL, YSDL2));
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
  clothes = yeCreateArray(caracter, "clothes");
  yeCreateString("hands/gloves/female/golden_gloves_female.png", clothes, NULL);
  yeCreateString("feet/boots/female/brown_longboots_female.png", clothes, NULL);
  yeCreateString("torso/corset_female/corset_red.png", clothes, NULL);
  yeCreateString("hair/female/longknot/pink.png", clothes, NULL);
  texture = yesCall(ygGet("lpcs.textureFromCaracter"), caracter);
  g_assert(texture);

  obj = yesCall(ygGet("lpcs.loadCanvas"), canvas, texture, 0, 0, 50, 50);
  g_assert(obj);
  obj = yesCall(ygGet("lpcs.loadCanvas"), canvas, texture, 1, 0, 100, 50);
  g_assert(obj);
  obj = yesCall(ygGet("lpcs.loadCanvas"), canvas, texture, 2, 0, 150, 50);
  g_assert(obj);
  obj = yesCall(ygGet("lpcs.loadCanvas"), canvas, texture, 3, 0, 200, 50);
  g_assert(obj);

  obj = yesCall(ygGet("lpcs.loadCanvas"), canvas, texture, 0, 7, 50, 120);
  g_assert(obj);
  obj = yesCall(ygGet("lpcs.loadCanvas"), canvas, texture, 1, 7, 100, 120);
  g_assert(obj);
  obj = yesCall(ygGet("lpcs.loadCanvas"), canvas, texture, 2, 7, 150, 120);
  g_assert(obj);
  obj = yesCall(ygGet("lpcs.loadCanvas"), canvas, texture, 3, 7, 200, 120);
  g_assert(obj);

  yeCreateString("canvas", canvas, "<type>");
  yeCreateString("rgba: 255 255 255 255", canvas, "background");
  wid = ywidNewWidget(canvas, NULL);
  g_assert(wid);
  ywidSetMainWid(wid);
  ygDoLoop();

 exit:
  yeDestroy(texture);
  ygCleanGameConfig(&cfg);
  ygEnd();
}
