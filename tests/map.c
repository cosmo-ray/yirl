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
#include "sdl-driver.h"
#include "json-desc.h"
#include "entity.h"
#include "tests.h"
#include "map.h"
#include "native-script.h"


static void *testMapEnter(int nb, union ycall_arg *args, int *types)
{
  Entity *eve = args[1].e;

  if (eve && (ywidEveType(eve) == YKEY_DOWN && ywidEveKey(eve) == '\n'))
    return (void *)ACTION;
  return (void *)NOTHANDLE;
}

void testYWMapSdl2(void)
{
  yeInitMem();
  int t = ydJsonInit();
  void *jsonManager;
  Entity *ret;
  YWidgetState *wid;
  Entity *map;

  /* load files */
  g_assert(t != -1);
  g_assert(ydJsonGetType() == t);
  jsonManager = ydNewManager(t);
  g_assert(jsonManager != NULL);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json", NULL);
  g_assert(yeGet(ret, "MapTest"));
  map = yeGet(ret, "MapTest");
  g_assert(map);
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywMapInit();
  g_assert(t != -1);

  g_assert(ysdl2Init() != -1);
  g_assert(ysdl2Type() == 0);

  g_assert(!ysdl2RegistreMap());

  ysRegistreFunc(ysNativeManager(), "mapTest", testMapEnter);
  yeReplaceBack(map, yeGet(ret, "MapTestResources"), "resources");
  wid = ywidNewWidget(yeGet(ret, "MapTest"), NULL);
  g_assert(wid);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  g_assert(!ywMapEnd());
  YWidDestroy(wid);
  ysdl2Destroy();
  /* end libs */
  YE_DESTROY(ret);
  yeEnd();
}

static void genBigMap(Entity *map)
{
  Entity *cases = yeCreateArray(map, "map");

  for (int i = 0; i < 10000; ++i) {
    Entity *tmp = yeCreateArray(cases, NULL);
    yeCreateInt(0, tmp, NULL);
  }
  yeCreateInt(1, yeGet(cases, 2), NULL);
  yeCreateInt(1, yeGet(cases, 1002), NULL);
  yeCreateInt(1, yeGet(cases, 2002), NULL);
  yeCreateInt(1, yeGet(cases, 3002), NULL);
  yeCreateInt(1, yeGet(cases, 4002), NULL);
  yeCreateInt(1, yeGet(cases, 5002), NULL);
  yeCreateInt(1, yeGet(cases, 6002), NULL);
  yeCreateInt(1, yeGet(cases, 9024), NULL);
  yeCreateInt(1, yeGet(cases, 9000), NULL);
}

void testYBigWMapSdl2(void)
{
  yeInitMem();
  int t = ydJsonInit();
  void *jsonManager;
  Entity *ret, *map;
  YWidgetState *wid;

  /* load files */
  g_assert(t != -1);
  g_assert(ydJsonGetType() == t);
  jsonManager = ydNewManager(t);
  g_assert(jsonManager != NULL);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json", NULL);
  map = yeGet(ret, "BigMap");
  g_assert(ret);
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywMapInit();
  g_assert(t != -1);

  g_assert(ysdl2Init() != -1);
  g_assert(ysdl2Type() == 0);

  g_assert(!ysdl2RegistreMap());

  genBigMap(map);
  yeReplaceBack(map, yeGet(ret, "MapTestResources"), "resources");
  ysRegistreFunc(ysNativeManager(), "mapTest", testMapEnter);
  wid = ywidNewWidget(map, NULL);
  g_assert(wid);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  g_assert(!ywMapEnd());
  YWidDestroy(wid);
  ysdl2Destroy();
  /* end libs */
  YE_DESTROY(ret);
  yeEnd();
}

