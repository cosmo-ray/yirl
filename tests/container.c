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

static void *testMenuEnter(va_list ap)
{
  va_arg(ap, Entity *);
  Entity *eve = va_arg(ap, Entity *);

  if (eve && (ywidEveType(eve) == YKEY_DOWN && ywidEveKey(eve) == '\n')) {
    ywidEveSetStatus(eve, ACTION);
    return (void *)ACTION;
  }
  return (void *)NOTHANDLE;
}

void testHorizontalContainerSdl(void)
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
  ret = yeGet(ret, "ContainerTest");
  g_assert(ret);
  ysRegistreNativeFunc("menuTest", testMenuEnter);

  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  YE_DESTROY(ret);
  ygEnd();
}

void testVerticalContainerSdl(void)
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
  ret = yeGet(ret, "VContainerTest");
  g_assert(ret);
  ysRegistreNativeFunc("menuTest", testMenuEnter);

  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  YE_DESTROY(ret);
  ygEnd();
}

void testStackContainerSdl(void)
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
  ret = yeGet(ret, "SContainerTest");
  g_assert(ret);
  ysRegistreNativeFunc("menuTest", testMenuEnter);

  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  YE_DESTROY(ret);
  ygEnd();
}

void testMixContainerSdl(void)
{
  GameConfig cfg;
  Entity *ret;
  Entity *cnt;
  YWidgetState *wid;
  int t = ydJsonInit();
  void *jsonManager;

  /* Init libs */
  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));

  /* Parsing json */
  jsonManager = ydNewManager(t);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json", NULL);
  cnt = yeGet(ret, "VContainerTest");
  g_assert(cnt);

  yeSetString(yeGet(yeGet(yeGet(cnt, "entries"), 1), "name"),
	      "ContainerTest");
  ret = yeGet(ret, "ContainerTest");
  g_assert(ret);
  yeSetString(yeGet(yeGet(yeGet(ret, "entries"), 1), "name"),
	      "MenuTest");
  yeCreateString("MenuTest", yeCreateArray(yeGet(ret, "entries"), NULL), "name");

  ysRegistreNativeFunc("menuTest", testMenuEnter);

  YE_ARRAY_FOREACH(yeGet(ret, "entries"), entry) {
    yeCreateString(NULL, entry, "copy");
  }
  wid = ywidNewWidget(cnt, NULL);
  g_assert(wid);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  YE_DESTROY(ret);
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
  g_assert(ysRegistreCreateNativeEntity(testMenuEnter, "menuTest", cnt, "action"));
  entries = yeCreateArray(cnt, "entries");

  curMap = ywMapCreateDefaultEntity(entries, NULL, resources, 0, 20, 20);
  yeCreateString("map", curMap, "<type>");

  wid = ywidNewWidget(cnt, NULL);

  curMap = ywMapCreateDefaultEntity(entries, NULL, resources, -1, 20, 20);
  yeCreateString("map", curMap, "<type>");
  ywMapDrawRect(curMap, ywRectCreateInts(5, 5, 5, 5, gc, NULL), 1);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  curMap = ywMapCreateDefaultEntity(entries, NULL, resources, -1, 20, 20);
  yeCreateString("map", curMap, "<type>");
  ywMapDrawRect(curMap, ywRectCreateInts(17, 17, 17, 17, gc, NULL), 2);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);


  curMap = ywMapCreateDefaultEntity(entries, NULL, resources, -1, 20, 20);
  yeCreateString("map", curMap, "<type>");
  ywMapDrawRect(curMap, ywRectCreateInts(7, 7, 4, 170, gc, NULL), 3);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  YE_DESTROY(gc);
  ygEnd();
}
