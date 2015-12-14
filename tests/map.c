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
#include "widget-callback.h"
#include "sdl-driver.h"
#include "json-desc.h"
#include "entity.h"
#include "tests.h"
#include "map.h"

#ifdef WITH_CURSES

/* static unsigned int testMapPosToIdx(int w, int x, int y) */
/* { */
/*   return w * y + x; */
/* } */

static int testMapEnter(YWidgetState *wid, YEvent *eve, Entity *arg)
{
  /* Entity *mapEnt = yeGet(wid->entity, "map"); */
  /* int w = yeToInt(yeGet(mapEnt, "width")); */

  /* g_assert(yeGetInt(yeGet(arg, "x")) == 2); */
  /* g_assert(yeGetInt(yeGet(arg, "y")) == 2); */
  /* g_assert(yeGet(yeGet(mapEnt, "map"), testMapPosToIdx(w, x, y))); */
  (void)wid;
  (void)arg;
  (void)eve;
  return ACTION;
}

void testYWMapCurses(void)
{
  int t = ydJsonInit();
  void *jsonManager;
  Entity *ret;
  YWidgetState *wid;

  /* load files */
  g_assert(t != -1);
  g_assert(ydJsonGetType() == t);
  g_assert(ywidInitCallback() >= 0);
  jsonManager = ydNewManager(t);
  g_assert(jsonManager != NULL);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json");
  ret = yeGet(ret, "MapTest");
  g_assert(ret);
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywMapInit();
  g_assert(t != -1);

  g_assert(ycursInit() != -1);
  g_assert(ycursType() == 0);
  
  g_assert(!ycursRegistreMap());
  ywinAddCallback(ywinCreateNativeCallback("mapTest", testMapEnter));

  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);

  
  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidHandleEvent(wid) != ACTION);

  g_assert(!ywMapEnd());
  YWidDestroy(wid);
  ycursDestroy();
  /* end libs */
  YE_DESTROY(ret);  
}

#endif
#ifdef WITH_SDL

void testYWMapSdl2(void)
{
  int t = ydJsonInit();
  void *jsonManager;
  Entity *ret;
  YWidgetState *wid;

  /* load files */
  g_assert(t != -1);
  g_assert(ydJsonGetType() == t);
  g_assert(ywidInitCallback() >= 0);
  jsonManager = ydNewManager(t);
  g_assert(jsonManager != NULL);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json");
  ret = yeGet(ret, "MapTest");
  g_assert(ret);
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywMapInit();
  g_assert(t != -1);

  g_assert(ysdl2Init() != -1);
  g_assert(ysdl2Type() == 0);
  
  g_assert(!ysdl2RegistreMap());

  ywinAddCallback(ywinCreateNativeCallback("mapTest", testMapEnter));
  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);

  
  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidHandleEvent(wid) != ACTION);

  g_assert(!ywMapEnd());
  YWidDestroy(wid);
  ysdl2Destroy();
  /* end libs */
  YE_DESTROY(ret);  
}
#endif
