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

void testSdlLife(void)
{
  g_assert(ysdl2Init() != -1);
  g_assert(ysdl2Type() == 0);
  ysdl2Destroy();
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
  jsonManager = ydNewManager(t);
  g_assert(jsonManager != NULL);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json");
  g_assert(ret);
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywTextScreenInit();
  g_assert(t != -1);

  g_assert(ycursInit() != -1);
  g_assert(ycursType() == 0);
  
  g_assert(!ycursRegistreTextScreen());

  wid = ywidNewWidget(t, ret, NULL, NULL);
  g_assert(wid);

  
  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidHandleEvent(wid) != ACTION);

  g_assert(!ywTextScreeEnd());
  YWidDestroy(wid);
  ycursDestroy();
  /* end libs */
  YE_DESTROY(ret);

}

void testYWTextScreenSdl2(void)
{
  int t = ydJsonInit();
  void *jsonManager;
  Entity *ret;
  YWidgetState *wid;

  /* load files */
  g_assert(t != -1);
  g_assert(ydJsonGetType() == t);
  jsonManager = ydNewManager(t);
  g_assert(jsonManager != NULL);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json");
  g_assert(ret);
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywTextScreenInit();
  g_assert(t != -1);

  g_assert(ysdl2Init() != -1);
  g_assert(ysdl2Type() == 0);
  
  g_assert(!ysdl2RegistreTextScreen());

  wid = ywidNewWidget(t, ret, NULL, NULL);
  g_assert(wid);

  
  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidHandleEvent(wid) != ACTION);

  g_assert(!ywTextScreeEnd());
  YWidDestroy(wid);
  ysdl2Destroy();
  /* end libs */
  YE_DESTROY(ret);
}
