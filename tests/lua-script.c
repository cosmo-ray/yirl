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

static int     luaToNumber(lua_State *l)
{
  lua_pushnumber(l, (long)lua_topointer(l, 1));
  return (1);
}

static int     luaToPtr(lua_State *l)
{
  lua_pushlightuserdata(l, (void *)lua_tointeger(l, 1));
  return (1);
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
  g_assert(!ysRegistreFunc(sm, "addPtr", luaAddPtr));
  g_assert(!ysRegistreFunc(sm, "toNbr", luaToNumber));
  g_assert(!ysRegistreFunc(sm, "toPtr", luaToPtr));

  if (ysLoadFile(sm, TESTS_PATH"/simple.lua")) {
    ysPrintError(sm);
    g_assert(0);
  }

  g_assert((long)ysCall(sm, "addPtr2", 2, 1, 2) == 3);

  g_assert(!ysDestroyScriptManager(sm));
  g_assert(!ysLuaEnd());
}
