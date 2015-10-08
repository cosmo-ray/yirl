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

#include <stdlib.h>
#include <glib.h>
#include "entity.h"
#include "debug.h"
#include "tests.h"

int main(int argc, char **argv)
{
  int no_wid = 0;
  GOptionContext *ctx;
  const GOptionEntry entries[] = {{"no-widget", 0, 0,  G_OPTION_ARG_NONE, &no_wid,
				   "don't test gui(usefull for perf)", NULL},
				  {NULL}};
  GError *error = NULL;

  ctx = g_option_context_new(NULL);
  g_option_context_set_help_enabled(ctx, 1);
  g_option_context_add_main_entries(ctx, entries, NULL);
  if (!g_option_context_parse(ctx, &argc, &argv, &error)) {
    printf("option parsing failed: %s\n", error->message);
    g_option_context_free(ctx);
    return 1;
  }
  g_option_context_free(ctx);

  // TODO: add [MY-OPTIONS] -- [TEST-OPTIONS] like syntaxe
  g_test_init(&argc, &argv, NULL);

  yuiDebugInit();
  g_test_add_func("/entity/lifecycle/simple", testLifecycleSimple);
  g_test_add_func("/entity/lifecycle/flow", testLifecycleFlow);
  g_test_add_func("/entity/lifecycle/complex", testLifecycleComplex);
  g_test_add_func("/entity/lifecycle/awakware", testLifecycleAwakwar);

  g_test_add_func("/entity/setunset/simple", testSetSimple);
  g_test_add_func("/entity/setunset/complex", testSetComplex);
  g_test_add_func("/entity/setunset/generic", testSetGeneric);

  g_test_add_func("/script/lua/lifecycle", testLuaScritLifecycle);
  g_test_add_func("/script/lua/entity", testLuaScritEntityBind);
  g_test_add_func("/parser/json/simple-file", testJsonLoadFile);
  g_test_add_func("/parser/json/complex-file", testJsonMultipleObj);
  if (no_wid)
    goto run_test;

#ifdef WITH_CURSES
  g_test_add_func("/widget/lifecycle/curses", testCursesLife);
  g_test_add_func("/widget/textScreen/curses", testYWTextScreenCurses);
  g_test_add_func("/widget/menu/curses", testYWMenuCurses);
  g_test_add_func("/widget/map/curses", testYWMapCurses);
  #endif
  #ifdef WITH_SDL
  g_test_add_func("/widget/lifecycle/sdl", testSdlLife);
  g_test_add_func("/widget/textScreen/sdl", testYWTextScreenSdl2);
  g_test_add_func("/widget/menu/sdl", testYWMenuSdl2);
  g_test_add_func("/widget/map/sdl2", testYWMapSdl2);
  #ifdef WITH_CURSES
  g_test_add_func("/widget/lifecycle/all", testAllLife);
  g_test_add_func("/widget/textScreen/all", testYWTextScreenAll);
  g_test_add_func("/game/all/simple", testYGameAllLibBasic);
  #endif
  #endif

 run_test:
  g_test_run();
  yuiDebugExit();
}
