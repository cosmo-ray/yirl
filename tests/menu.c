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
#include "entity.h"
#include "json-desc.h"
#include "curses-driver.h"
#include "sdl-driver.h"
#include "menu.h"
#include "native-script.h"

static void *testMenuEnter(va_list ap)
{
  (void)ap;
  return (void *)ACTION;
}

#if WITH_CURSES == 1
void testYWMenuCurses(void)
{
  yeInitMem();
  int t = ydJsonInit();
  void *jsonManager;
  Entity *ret;
  YWidgetState *wid;

  /* load files */
  ysRegistreFunc(ysNativeManager(), "menuTest", testMenuEnter);

  g_assert(t != -1);
  g_assert(ydJsonGetType() == t);
  jsonManager = ydNewManager(t);
  g_assert(jsonManager != NULL);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json", NULL);
  ret = yeGet(ret, "MenuTest");
  g_assert(ret);
  g_assert(!ydDestroyManager(jsonManager));
  g_assert(!ydJsonEnd());

  t = ywMenuInit();
  g_assert(t != -1);

  g_assert(ycursInit() != -1);
  g_assert(ycursType() == 0);

  g_assert(ycursRegistreMenu() == 0);

  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  g_assert(!ywMenuEnd());
  YWidDestroy(wid);
  ycursDestroy();
  /* end libs */
  YE_DESTROY(ret);
  yeEnd();
}
#endif

#if WITH_SDL == 1

void testPanelMenuSdl2(void)
{
  yeInitMem();
  int t = ydJsonInit();
  void *jsonManager;
  Entity *ret;
  YWidgetState *wid;

  ysRegistreFunc(ysNativeManager(), "menuTest", testMenuEnter);

  /* load files */
  g_assert(t != -1);
  g_assert(ydJsonGetType() == t);
  jsonManager = ydNewManager(t);
  g_assert(jsonManager != NULL);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json", NULL);
  ret = yeGet(ret, "MenuTest");
  g_assert(ret);
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywMenuInit();
  g_assert(t == 0);

  g_assert(ysdl2Init() != -1);
  g_assert(ysdl2Type() == 0);

  yeCreateString("panel", ret, "mn-type");
  g_assert(ysdl2RegistreMenu() == 0);

  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  g_assert(!ywMenuEnd());
  YWidDestroy(wid);
  ysdl2Destroy();
  /* end libs */
  YE_DESTROY(ret);
  yeEnd();
}

void testYWMenuSdl2(void)
{
  yeInitMem();
  int t = ydJsonInit();
  void *jsonManager;
  Entity *ret;
  YWidgetState *wid;

  ysRegistreFunc(ysNativeManager(), "menuTest", testMenuEnter);

  /* load files */
  g_assert(t != -1);
  g_assert(ydJsonGetType() == t);
  jsonManager = ydNewManager(t);
  g_assert(jsonManager != NULL);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json", NULL);
  ret = yeGet(ret, "MenuTest");
  g_assert(ret);
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywMenuInit();
  g_assert(t == 0);

  g_assert(ysdl2Init() != -1);
  g_assert(ysdl2Type() == 0);

  /* if sdl have type 0, ywidRegistreTypeRender must register this func at 0*/ 
  g_assert(ysdl2RegistreMenu() == 0);

  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  g_assert(!ywMenuEnd());
  YWidDestroy(wid);
  ysdl2Destroy();
  /* end libs */
  YE_DESTROY(ret);
  yeEnd();
}
#endif
