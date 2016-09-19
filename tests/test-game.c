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
#include "native-script.h"

static const char *testPath = "./testMod"; 

#define MAP_SIZE_W 5
#define MAP_SIZE_H 5

static void *shooterAction(va_list ap)
{
  va_arg(ap, YWidgetState *);
  YEvent *eve = va_arg(ap, YEvent *);
  InputStatue ret = NOTHANDLE;

  if (!eve)
    return NOTHANDLE;
  if (eve->key == '\t' || eve->key == 'q') {
    ygCall(NULL, "FinishGame");
    ret = ACTION;
  }

  return (void *)ret;
}

static void *shooterInit(va_list ap)
{
  YWidgetState *wid = va_arg(ap, YWidgetState *);
  va_arg(ap, YEvent *);
  Entity *arg = va_arg(ap, Entity *);

  yeCreateInt(MAP_SIZE_W, arg, "width");
  arg = yeCreateArray(arg, "map");
  for (int i = 0; i < MAP_SIZE_W * MAP_SIZE_H; ++i) {
    yeCreateInt(0, yeCreateArray(arg, NULL), NULL);
  }
  ysRegistreNativeFunc("shooterAction", shooterAction);
  ygBind(wid, "action", "shooterAction");
  return NOTHANDLE;
}

void testYGameSdlLibBasic(void)
{
  GameConfig cfg;

  ysRegistreNativeFunc("shooterInit", shooterInit);
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
  ysRegistreNativeFunc("shooterInit", shooterInit);
  g_assert(!ygStartLoop(&cfg));
  ygCleanGameConfig(&cfg);
  ygEnd();
}
