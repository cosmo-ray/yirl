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
#include "text-screen.h"
#include "widget-callback.h"

static int testTXQuitOnQ(YWidgetState *wid, YEvent *eve, Entity *arg)
{
  (void)wid;
  (void)arg;
  (void)eve;

  if (eve && (eve->type == YKEY_DOWN && (eve->key == '\n' || eve->key == 'q' )))
    return ACTION;
  return NOTHANDLE;
}

#ifdef WITH_CURSES

void testCursesLife(void)
{
  g_assert(ycursInit() != -1);
  g_assert(ycursType() == 0);
  ycursDestroy();
}

void testYWTextScreenCurses(void)
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
  ret = yeGet(ret, "TextScreenTest");
  g_assert(ret);
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywTextScreenInit();
  g_assert(t != -1);

  g_assert(ycursInit() != -1);
  g_assert(ycursType() == 0);
  
  g_assert(!ycursRegistreTextScreen());

  ywinAddCallback(ywinCreateNativeCallback("txQuitOnQ", testTXQuitOnQ));
  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);

  
  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);

  g_assert(!ywTextScreenEnd());
  YWidDestroy(wid);
  ycursDestroy();
  /* end libs */
  YE_DESTROY(ret);
}

#endif

#ifdef WITH_SDL

void testYWTextScreenSdl2(void)
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
  ret = yeGet(ret, "TextScreenTest");
  g_assert(ret);
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywTextScreenInit();
  g_assert(t != -1);

  g_assert(ysdl2Init() != -1);
  g_assert(ysdl2Type() == 0);
  
  g_assert(!ysdl2RegistreTextScreen());
  ywinAddCallback(ywinCreateNativeCallback("txQuitOnQ", testTXQuitOnQ));

  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);
  
  do {
    g_assert(ywidRend(wid) != -1);
    usleep(100000);
  } while(ywidDoTurn(wid) != ACTION);

  g_assert(!ywTextScreenEnd());
  YWidDestroy(wid);
  ysdl2Destroy();
  /* end libs */
  YE_DESTROY(ret);
}

void testSdlLife(void)
{
  g_assert(ysdl2Init() != -1);
  g_assert(ysdl2Type() == 0);
  ysdl2Destroy();
}

#ifdef WITH_CURSES

void testYWTextScreenAll(void)
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
  ret = yeGet(ret, "TextScreenTest");
  g_assert(ret);
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywTextScreenInit();
  g_assert(t != -1);

  /* Init all */
  g_assert(ysdl2Init() == 0);
  g_assert(ycursInit() == 1);

  /* registre curses and sdl text screen */
  g_assert(!ysdl2RegistreTextScreen());
  g_assert(!ycursRegistreTextScreen());

  ywinAddCallback(ywinCreateNativeCallback("txQuitOnQ", testTXQuitOnQ));
  /* create widgets */
  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);

  
  do {
    g_assert(ywidRend(wid) != -1);
    usleep(100000);
  } while(ywidDoTurn(wid) != ACTION);

  /* end libs */
  g_assert(!ywTextScreenEnd());
  YWidDestroy(wid);
  ycursDestroy();
  ysdl2Destroy();
  YE_DESTROY(ret);
}

void testAllLife(void)
{
  g_assert(ysdl2Init() != -1);
  g_assert(ysdl2Type() == 0);
  g_assert(ycursInit() != -1);
  g_assert(ycursType() == 1);
  ycursDestroy();
  ysdl2Destroy();
}

#endif
#endif
