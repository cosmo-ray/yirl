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

static void *moveImg(va_list ap)
{
  Entity *wid = va_arg(ap, Entity *);
  Entity *eve = va_arg(ap, Entity *);
  Entity *cam = yeGet(wid, "cam");

  if (eve && (ywidEveType(eve) == YKEY_DOWN)) {
    if (ywidEveKey(eve) == Y_UP_KEY) {
      ywPosAddXY(cam, 0, -10);
      return (void *)ACTION;
    }
    if (ywidEveKey(eve) == Y_DOWN_KEY) {
      ywPosAddXY(cam, 0, 10);
      return (void *)ACTION;
    }
    if (ywidEveKey(eve) == Y_LEFT_KEY) {
      ywPosAddXY(cam, -10, 0);
      return (void *)ACTION;
    }
    if (ywidEveKey(eve) == Y_RIGHT_KEY) {
      ywPosAddXY(cam, 10, 0);
      return (void *)ACTION;
    }
  }
  return (void *)NOTHANDLE;
}

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
  ysRegistreNativeFunc("moveImg", moveImg);

  yeCreateString("QuitOnKeyDown", actions, NULL);
  yeCreateString("moveImg", actions, NULL);
  yeCreateString("rgba: 180 140 200 255", canvas, "background");
  yeCreateArray(canvas, "objs");

  wid = ywidNewWidget(canvas, NULL);
  g_assert(wid);
  yesCall(ygGet("tiled.setAssetPath"), TESTS_PATH"../modules/tiled/");
  g_assert(yesCall(ygGet("tiled.fileToCanvas"),
  		   TESTS_PATH"../modules/tiled/testile.json", canvas));
  /* g_assert(yesCall(ygGet("tiled.fileToCanvas"), */
  /* 		   TESTS_PATH"../modules/tiled/Test_bar_map_export.json", canvas)); */

  ywPosCreateInts(0, 50, canvas, "cam");
  ywidSetMainWid(wid);
  ygDoLoop();

  ygCleanGameConfig(&cfg);
  yesCall(ygGet("tiled.deinit"));
  ygEnd();
}
