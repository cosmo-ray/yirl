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

#include "tests.h"
#include "game.h"
#include "utils.h"
#include "widget-callback.h"

static const char *testPath = "./testMod"; 

#define MAP_SIZE_W 5
#define MAP_SIZE_H 5

static int shooterAction(YWidgetState *wid, YEvent *eve, Entity *arg)
{
  InputStatue ret = NOTHANDLE;

  (void)wid;
  (void)arg;
  if (!eve)
    return NOTHANDLE;
  if (eve->key == '\t' || eve->key == 'q') {
    ywidCallCallbackByStr("FinishGame", wid, eve, arg);
    ret = ACTION;
  }

  return ret;
}

static int shooterInit(YWidgetState *wid, YEvent *eve, Entity *arg)
{
  Entity *tmp;

  (void)wid;
  (void)eve;
  yeCreateInt(MAP_SIZE_W, arg, "width");
  arg = yeCreateArray(arg, "map");
  for (int i = 0; i < MAP_SIZE_W * MAP_SIZE_H; ++i) {
    tmp = yeCreateArray(arg, NULL);
    yeCreateInt(0, tmp, NULL);
  }
  ywinAddCallback(ywinCreateNativeCallback("shooterAction", shooterAction));
  ywidBind(wid, "action", "shooterAction");
  ywidCallCallbackByStr("shooterAction", wid, eve, arg);
  return NOTHANDLE;
}

void testYGameSdlLibBasic(void)
{
  GameConfig cfg;

  g_assert(ywidInitCallback() >= 0);
  ywinAddCallback(ywinCreateNativeCallback("shooterInit", shooterInit));
  g_assert(!ygInitGameConfig(&cfg, testPath, SDL2));
  g_assert(!ygInit(&cfg));
  g_assert(!ygStartLoop(&cfg));

  ygCleanGameConfig(&cfg);
  ygEnd();
}

void testYGameAllLibBasic(void)
{
  GameConfig cfg;

  g_assert(!ygInitGameConfig(&cfg, testPath, ALL));
  g_assert(!ygInit(&cfg));
  ywinAddCallback(ywinCreateNativeCallback("shooterInit", shooterInit));
  g_assert(!ygStartLoop(&cfg));
  ygCleanGameConfig(&cfg);
  ygEnd();
}
