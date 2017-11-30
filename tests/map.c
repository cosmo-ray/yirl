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
#include "curses-driver.h"
#include "sdl-driver.h"
#include "json-desc.h"
#include "entity.h"
#include "tests.h"
#include "map.h"
#include "native-script.h"


static void *testMapEnter(va_list ap)
{
  va_arg(ap, Entity *);
  Entity *eve = va_arg(ap, Entity *);

  if (eve && (ywidEveType(eve) == YKEY_DOWN && ywidEveKey(eve) == '\n'))
    return (void *)ACTION;
  return (void *)NOTHANDLE;
}

#if WITH_CURSES == 1

void testYWMapCurses(void)
{
  yeInitMem();
  int t = ydJsonInit();
  void *jsonManager;
  Entity *ret;
  YWidgetState *wid;

  /* load files */
  g_assert(t != -1);
  g_assert(ydJsonGetType() == t);
  jsonManager = ydNewManager(t);
  g_assert(jsonManager != NULL);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json", NULL);
  ret = yeGet(ret, "MapTest");
  g_assert(ret);
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywMapInit();
  g_assert(t != -1);

  g_assert(ycursInit() != -1);
  g_assert(ycursType() == 0);

  g_assert(!ycursRegistreMap());
  ysRegistreFunc(ysNativeManager(), "mapTest", testMapEnter);

  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);


  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  g_assert(!ywMapEnd());
  YWidDestroy(wid);
  ycursDestroy();
  /* end libs */
  YE_DESTROY(ret);
  yeEnd();
}

#endif
#if WITH_SDL == 1

void testYWMapSdl2(void)
{
  yeInitMem();
  int t = ydJsonInit();
  void *jsonManager;
  Entity *ret;
  YWidgetState *wid;

  /* load files */
  g_assert(t != -1);
  g_assert(ydJsonGetType() == t);
  jsonManager = ydNewManager(t);
  g_assert(jsonManager != NULL);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json", NULL);
  g_assert(yeGet(ret, "MapTest"));
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywMapInit();
  g_assert(t != -1);

  g_assert(ysdl2Init() != -1);
  g_assert(ysdl2Type() == 0);

  g_assert(!ysdl2RegistreMap());

  ysRegistreFunc(ysNativeManager(), "mapTest", testMapEnter);
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
  Entity *ret;
  YWidgetState *wid;

  /* load files */
  g_assert(t != -1);
  g_assert(ydJsonGetType() == t);
  jsonManager = ydNewManager(t);
  g_assert(jsonManager != NULL);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json", NULL);
  ret = yeGet(ret, "BigMap");
  g_assert(ret);
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywMapInit();
  g_assert(t != -1);

  g_assert(ysdl2Init() != -1);
  g_assert(ysdl2Type() == 0);

  g_assert(!ysdl2RegistreMap());

  genBigMap(ret);
  ysRegistreFunc(ysNativeManager(), "mapTest", testMapEnter);
  wid = ywidNewWidget(ret, NULL);
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


#if WITH_CURSES == 1

void testYWMapAll(void)
{
  yeInitMem();
  int t = ydJsonInit();
  void *jsonManager;
  Entity *ret;
  YWidgetState *wid;

  /* load files */
  g_assert(t != -1);
  g_assert(ydJsonGetType() == t);
  jsonManager = ydNewManager(t);
  g_assert(jsonManager != NULL);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json", NULL);
  ret = yeGet(ret, "MapTest++");
  g_assert(ret);
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywMapInit();
  g_assert(t != -1);

  g_assert(ysdl2Init() != -1);
  g_assert(ysdl2Type() == 0);
  g_assert(ycursInit() != -1);
  g_assert(ycursType() == 1);

  g_assert(!ysdl2RegistreMap());
  g_assert(!ycursRegistreMap());

  ysRegistreNativeFunc("mapTest", testMapEnter);
  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  g_assert(!ywMapEnd());
  YWidDestroy(wid);
  ysdl2Destroy();
  ycursDestroy();
  /* end libs */
  YE_DESTROY(ret);
  yeEnd();
}

#endif

#endif
