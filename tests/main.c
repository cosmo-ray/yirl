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
  g_test_init(&argc, &argv, NULL);
  yuiDebugInit();
  g_test_add_func("/entity/lifecycle/simple", testLifecycleSimple);
  g_test_add_func("/entity/lifecycle/flow", testLifecycleFlow);
  g_test_add_func("/entity/lifecycle/complex", testLifecycleComplex);
  g_test_add_func("/entity/setunset/simple", testSetSimple);
  g_test_add_func("/entity/setunset/complex", testSetComplex);
  g_test_add_func("/entity/setunset/generic", testSetGeneric);

  g_test_add_func("/script/luaScript/lifecycle", testLuaScritLifecycle);

  g_test_add_func("/parser/json/simple-file", testJsonLoadFile);
  g_test_add_func("/parser/json/complex-file", testJsonMultipleObj);

  g_test_add_func("/widget/lifecycle/sdl", testSdlLife);
  g_test_add_func("/widget/lifecycle/curses", testCursesLife);
  g_test_add_func("/widget/lifecycle/all", testAllLife);

  g_test_add_func("/widget/textScreen/sdl", testYWTextScreenSdl2);
  g_test_add_func("/widget/textScreen/curses", testYWTextScreenCurses);
  g_test_add_func("/widget/textScreen/all", testYWTextScreenAll);

  g_test_add_func("/widget/menu/curses", testYWMenuCurses);
  g_test_add_func("/widget/menu/sdl", testYWMenuSdl2);

  g_test_add_func("/widget/map/curses", testYWMapCurses);
  g_test_add_func("/widget/map/sdl2", testYWMapSdl2);

  g_test_add_func("/game/all/simple", testYGameAllLibBasic);

  g_test_run();
  yuiDebugExit();
}
