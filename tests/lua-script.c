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
#include "tests.h"
#include "lua-script.h"

static void *addPtr(const void *arg1, const void *arg2)
{
  return (void *)((long)arg1 + (long)arg2);
}

static int     luaAddPtr(lua_State *l)
{
  void *ret = addPtr(lua_topointer(l, 1), lua_topointer(l, 2));

  lua_pushlightuserdata(l, ret);
  return (1);
}

void testLuaScritLifecycle(void)
{
  void *sm = NULL;

  g_assert(!ysLuaInit());
  g_assert(!ysLuaGetType());
  sm = ysNewScriptManager(NULL, 0);
  g_assert(sm);
  g_assert((long)addPtr((void *)1, (void *)2) == 3);
  g_assert(!ysRegistreFunc(sm, "addPtr", luaAddPtr));
  g_assert((long)ysCall(sm, "addPtr", 2, 1, 2) == 3);
  g_assert(!ysDestroyScriptManager(sm));
  g_assert(!ysLuaEnd());
}
