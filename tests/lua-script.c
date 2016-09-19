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
#include "lua-convert.h"
#include "lua-binding.h"
#include "entity-script.h"

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

  yeInitMem();
  g_assert(!ysLuaInit());
  g_assert(!ysLuaGetType());
  sm = ysNewManager(NULL, 0);
  g_assert(sm);

  g_assert(!ysRegistreFunc(sm, "addPtr", luaAddPtr));
  g_assert((long)ysCall(sm, "addPtr", 1, 2) == 3);

  g_assert(!ysRegistreFunc(sm, "toNbr", luaPtrToNumber));
  g_assert(!ysRegistreFunc(sm, "toPtr", luaNbrToPtr));

  if (ysLoadFile(sm, TESTS_PATH"/simple.lua")) {
    DPRINT_ERR("%s\n", ysGetError(sm));
    g_assert(0);
  }

  g_assert((long)ysCall(sm, "addPtr2", 1, 2) == 3);

  g_assert(!ysDestroyManager(sm));
  g_assert(!ysLuaEnd());
  yeEnd();
}

void testLuaScritEntityBind(void)
{
  Entity *ret;
  void *sm;
  Entity *func;
  
  yeInitMem();
  g_assert(!ysLuaInit());
  sm = ysNewManager(NULL, 0);
  yesLuaRegister(sm);
  g_assert(sm);
  func = yeCreateFunction("createInt", sm, NULL, NULL);

  g_assert(ysLoadFile(sm,  "I should fail") < 0);

  if (ysLoadFile(sm, TESTS_PATH"/test-entity.lua")) {
    g_assert(ysGetError(sm));
    g_assert(0);
  }

  ret = ysCall(sm, "fail incoming", "tests");
  g_assert(ret == NULL);
  ret = ysCall(sm, "yeCreateArray");
  g_assert(ret);
  g_assert(yeType(ret) == YARRAY);
  YE_DESTROY(ret);
  ret = ysCall(sm, "createString", "tests");
  g_assert(ret);
  g_assert(yeType(ret) == YSTRING);
  g_assert(yuiStrEqual(yeGetString(ret), "tests"));
  //call func
  YE_DESTROY(ret);
  ret = yesCall(func, 6);
  g_assert(ret);
  g_assert(yeType(ret) == YINT);
  g_assert(yeGetInt(ret));
  YE_DESTROY(ret);
  YE_DESTROY(func);
  g_assert(!ysDestroyManager(sm));
  g_assert(!ysLuaEnd());
  yeEnd();
}
