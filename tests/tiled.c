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

void testsTiled(void)
{
  yeInitMem();
  GameConfig cfg;
  Entity *canvas = yeCreateArray(NULL, NULL);
  YWidgetState *wid;
  Entity *actions;

  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));

  g_assert(ygLoadMod(TESTS_PATH"../modules/tiled/"));
  yeCreateString("canvas", canvas, "<type>");
  actions = yeCreateArray(canvas, "actions");
  yeCreateString("QuitOnKeyDown", actions, NULL);
  yeCreateString("rgba: 180 140 200 255", canvas, "background");
  yeCreateArray(canvas, "objs");

  wid = ywidNewWidget(canvas, NULL);
  g_assert(wid);
  yesCall(ygGet("tiled.setAssetPath"), TESTS_PATH"../modules/tiled/");
  g_assert(yesCall(ygGet("tiled.fileToCanvas"),
		   TESTS_PATH"../modules/tiled/testile.json", canvas));

  ywPosCreateInts(0, 50, canvas, "cam");
  ywidSetMainWid(wid);
  ygDoLoop();
  yeDestroy(canvas);

  ygCleanGameConfig(&cfg);
  yesCall(ygGet("tiled.deinit"));
  ygEnd();
}
