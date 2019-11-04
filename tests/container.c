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
#include "rect.h"
#include "json-desc.h"
#include "yirl/native-script.h"
#include "container.h"

void testHorizontalContainerSdl(void)
{
  GameConfig cfg;
  Entity *ret0;
  Entity *ret;
  YWidgetState *wid;

  /* Init libs */
  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));

  /* Parsing json */
  ret0 = ygFileToEnt(YJSON, TESTS_PATH"/widget.json", NULL);
  ret = yeGet(ret0, "ContainerTest");
  g_assert(ret0 && ret);
  yePushToGlobalScope(ret0, "test");

  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  ygCleanGameConfig(&cfg);
  YWidDestroy(wid);
  YE_DESTROY(ret0);
  ygEnd();
}

void testVerticalContainerSdl(void)
{
  GameConfig cfg;
  Entity *ret0;
  Entity *ret;
  YWidgetState *wid;

  /* Init libs */
  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));

  /* Parsing json */
  ret0 = ygFileToEnt(YJSON, TESTS_PATH"/widget.json", NULL);
  ret = yeGet(ret0, "VContainerTest");
  g_assert(ret);
  yePushToGlobalScope(ret0, "test");

  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  ygCleanGameConfig(&cfg);
  YWidDestroy(wid);
  YE_DESTROY(ret0);
  ygEnd();
}

void testStackContainerSdl(void)
{
  GameConfig cfg;
  Entity *ret;
  YWidgetState *wid;
  Entity *ret0;

  /* Init libs */
  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));

  /* Parsing json */
  ret0 = ygFileToEnt(YJSON, TESTS_PATH"/widget.json", NULL);
  ret = yeGet(ret0, "SContainerTest");
  g_assert(ret);
  yePushToGlobalScope(ret0, "test");

  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  ygCleanGameConfig(&cfg);
  YWidDestroy(wid);
  YE_DESTROY(ret0);
  ygEnd();
}

void testMixContainerSdl(void)
{
  GameConfig cfg;
  Entity *fileRet;
  Entity *ret;
  Entity *cnt;
  YWidgetState *wid;

  /* Init libs */
  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));

  /* Parsing json */
  fileRet = ygFileToEnt(YJSON, TESTS_PATH"/widget.json", NULL);
  cnt = yeGet(fileRet, "VContainerTest");
  g_assert(cnt);

  yeSetString(yeGet(yeGet(yeGet(cnt, "entries"), 1), "name"),
	      "ContainerTest");
  ret = yeGet(fileRet, "ContainerTest");
  g_assert(ret);
  yeSetString(yeGet(yeGet(yeGet(ret, "entries"), 1), "name"),
	      "MenuTest");
  yeCreateString("MenuTest", yeCreateArray(yeGet(ret, "entries"), NULL), "name");

  YE_ARRAY_FOREACH(yeGet(ret, "entries"), entry) {
    yeCreateString(NULL, entry, "copy");
  }
  yePushToGlobalScope(fileRet, "test");

  wid = ywidNewWidget(cnt, NULL);
  g_assert(wid);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  ygCleanGameConfig(&cfg);
  YWidDestroy(wid);
  YE_DESTROY(fileRet);
  ygEnd();
}

void testDynamicStackContainerSdl(void)
{
  GameConfig cfg;
  Entity *cnt, *resources, *entries, *curMap, *gc;
  YWidgetState *wid;

  /* Init libs */
  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));

  gc = yeCreateArray(NULL, NULL);
  cnt = yeCreateArray(gc, NULL);
  resources = yeCreateArray(cnt, "resources");
  yeCreateString(".", yeCreateArray(resources, NULL), "map-char");
  yeCreateString("1", yeCreateArray(resources, NULL), "map-char");
  yeCreateString("2", yeCreateArray(resources, NULL), "map-char");
  yeCreateString("3", yeCreateArray(resources, NULL), "map-char");

  yeReCreateString("container", cnt, "<type>");
  yeCreateString("stacking", cnt, "cnt-type");
  yeCreateString("rgba: 255 255 255 255", cnt, "background");
  yeCreateString("QuitOnKeyDown", cnt, "action");
  entries = yeCreateArray(cnt, "entries");

  curMap = ywMapCreateDefaultEntity(entries, NULL, resources, 0, 20, 20);
  yeCreateString("map", curMap, "<type>");

  wid = ywidNewWidget(cnt, NULL);

  curMap = ywMapCreateDefaultEntity(NULL, NULL, resources, -1, 20, 20);
  yeCreateString("map", curMap, "<type>");
  ywPushNewWidget(cnt, curMap, 1);
  ywMapDrawRect(curMap, ywRectCreateInts(5, 5, 5, 5, gc, NULL), 1);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  curMap = ywMapCreateDefaultEntity(NULL, NULL, resources, -1, 20, 20);
  yeCreateString("map", curMap, "<type>");
  ywPushNewWidget(cnt, curMap, 1);
  ywMapDrawRect(curMap, ywRectCreateInts(17, 17, 17, 17, gc, NULL), 2);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);


  curMap = ywMapCreateDefaultEntity(NULL, NULL, resources, -1, 20, 20);
  yeCreateString("map", curMap, "<type>");
  ywPushNewWidget(cnt, curMap, 1);
  ywMapDrawRect(curMap, ywRectCreateInts(7, 7, 4, 170, gc, NULL), 3);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  YWidDestroy(wid);
  YE_DESTROY(gc);
  ygCleanGameConfig(&cfg);
  ygEnd();
}
