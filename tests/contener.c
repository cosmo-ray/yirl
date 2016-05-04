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

#include <glib.h>
#include <stdio.h>
#include <unistd.h>

#include "tests.h"
#include "game.h"
#include "utils.h"
#include "widget-callback.h"
#include "json-desc.h"

static int testMenuEnter(YWidgetState *wid, YEvent *eve, Entity *arg)
{
  (void)wid;
  (void)eve;
  (void)arg;
  return ACTION;
}

void testHorizontalContenerSdl(void)
{
  GameConfig cfg;
  Entity *ret;
  YWidgetState *wid;
  int t = ydJsonInit();
  void *jsonManager;

  /* Init libs */
  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));

  /* Parsing json */
  jsonManager = ydNewManager(t);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json", NULL);
  ret = yeGet(ret, "ContenerTest");
  g_assert(ret);
  ywinAddCallback(ywinCreateNativeCallback("menuTest", testMenuEnter));

  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  YE_DESTROY(ret);
  ygEnd();
}

void testVerticalContenerSdl(void)
{
  GameConfig cfg;
  Entity *ret;
  YWidgetState *wid;
  int t = ydJsonInit();
  void *jsonManager;

  /* Init libs */
  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));

  /* Parsing json */
  jsonManager = ydNewManager(t);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json", NULL);
  ret = yeGet(ret, "VContenerTest");
  g_assert(ret);
  ywinAddCallback(ywinCreateNativeCallback("menuTest", testMenuEnter));

  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  YE_DESTROY(ret);
  ygEnd();
}
