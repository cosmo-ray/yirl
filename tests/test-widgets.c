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
#include "entity.h"
#include "json-desc.h"
#include "sdl-driver.h"
#include "text-screen.h"
#include "native-script.h"

static void *testTXQuitOnQ(int nb, union ycall_arg *args, int *types)
{
  Entity *eve = args[1].e;

  if (eve && (ywidEveType(eve) == YKEY_DOWN &&
	      (ywidEveKey(eve) == '\n' || ywidEveKey(eve) == 'q' )))
    return (void *)ACTION;
  return (void *)NOTHANDLE;
}


void fmtTxtScreen(void)
{
  GameConfig cfg;
  Entity *ret0;
  Entity *ret;
  YWidgetState *wid;
  Entity *stuff;

  g_assert(!ygInitGameConfig(&cfg, NULL, YSDL2));
  g_assert(!ygInit(&cfg));

  ret0 = ygFileToEnt(YJSON, TESTS_PATH"/widget.json", NULL);
  ret = yeGet(ret0, "FmtTextScreen");

  stuff = yeCreateArray(NULL, NULL);
  yePushToGlobalScope(stuff, "main");
  yeCreateString("initial comit", stuff, "first_commit_name");
  ygSetInt("yirl_age", 3);
  yeCreateString("12 10:38:02 2015", stuff, "first_commit_date");
  wid = ywidNewWidget(ret, NULL);
  g_assert(wid);

  do {
    g_assert(ywidRend(wid) != -1);
  } while(ywidDoTurn(wid) != ACTION);


  ygCleanGameConfig(&cfg);
  YWidDestroy(wid);
  yeDestroy(ret0);
  yeDestroy(stuff);
  ygEnd();

}

void testYWTextScreenSdl2(void)
{
  yeInitMem();
  int t = ydJsonInit();
  void *jsonManager;
  Entity *ret, *map;
  YWidgetState *wid;
  Entity *txt;

  /* load files */
  g_assert(t != -1);
  g_assert(ydJsonGetType() == t);
  jsonManager = ydNewManager(t);
  g_assert(jsonManager != NULL);
  ret = ydFromFile(jsonManager, TESTS_PATH"/widget.json", NULL);
  map = yeGet(ret, "ScroolingTest");
  g_assert(map);
  g_assert(!ydJsonEnd());
  g_assert(!ydDestroyManager(jsonManager));

  t = ywTextScreenInit();
  g_assert(t != -1);

  g_assert(ysdl2Init() != -1);
  g_assert(ysdl2Type() == 0);

  t = ysdl2RegistreTextScreen();
  g_assert(!t);
  ysRegistreNativeFunc("txQuitOnQ", testTXQuitOnQ);

  yeCreateInt(10, map, "cursor");
  wid = ywidNewWidget(map, NULL);
  g_assert(wid);

  // replace the set with an add, once eature add :)
  txt = ywTextScreenTextEnt(map);

  /* yeAdd(txt, "fffffffffffffffffffffffffffffffffffffffffffffff\n"); */
  yeAdd(txt, "--\33[32m Color Test \33[31m red here \33[0m now unset :)\n");

  printf("go \n%s\n !!!!\n", yeGetString(txt));
  do {
    g_assert(ywidRend(wid) != -1);
    ywidDoTurn(wid);
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

