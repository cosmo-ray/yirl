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
#include "native-script.h"

static void *testTXQuitOnQ(va_list ap)
{
  va_arg(ap, Entity *);
  Entity *eve = va_arg(ap, Entity *);

  if (eve && (ywidEveType(eve) == YKEY_DOWN &&
	      (ywidEveKey(eve) == '\n' || ywidEveKey(eve) == 'q' )))
    return (void *)ACTION;
  return (void *)NOTHANDLE;
}

#if WITH_CURSES == 1

void testCursesLife(void)
{
  yeInitMem();
  g_assert(ycursInit() != -1);
  g_assert(ycursType() == 0);
  ycursDestroy();
  yeEnd();
}

void testYWTextScreenCurses(void)
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
  ret = yeGet(ret, "TextScreenTest");
  g_assert(ret);
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywTextScreenInit();
  g_assert(t != -1);

  g_assert(ycursInit() != -1);
  g_assert(ycursType() == 0);

  g_assert(!ycursRegistreTextScreen());

  ysRegistreNativeFunc("txQuitOnQ", testTXQuitOnQ);
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
  yeEnd();
}

#endif

#if WITH_SDL == 1

void testYWTextScreenSdl2(void)
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
  ret = yeGet(ret, "TextScreenTest");
  g_assert(ret);
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywTextScreenInit();
  g_assert(t != -1);

  g_assert(ysdl2Init() != -1);
  g_assert(ysdl2Type() == 0);

  t = ysdl2RegistreTextScreen();
  g_assert(!t);
  ysRegistreNativeFunc("txQuitOnQ", testTXQuitOnQ);

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
  yeEnd();
}

void testSdlLife(void)
{
  yeInitMem();
  g_assert(ysdl2Init() != -1);
  g_assert(ysdl2Type() == 0);
  ysdl2Destroy();
  yeEnd();
}

#if WITH_CURSES == 1

void testYWTextScreenAll(void)
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

  ysRegistreNativeFunc("txQuitOnQ", testTXQuitOnQ);
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
  yeEnd();
}

void testAllLife(void)
{
  yeInitMem();
  g_assert(ysdl2Init() != -1);
  g_assert(ysdl2Type() == 0);
  g_assert(ycursInit() != -1);
  g_assert(ycursType() == 1);
  ycursDestroy();
  ysdl2Destroy();
  yeEnd();
}

#endif
#endif
