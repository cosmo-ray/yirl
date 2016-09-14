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
#include "entity.h"
#include "json-desc.h"

void testJsonLoadFile(void)
{
  yeInitMem();
  int t = ydJsonInit();
  void *jsonManager;
  Entity *ret;
  
  g_assert(t != -1);
  g_assert(ydJsonGetType() == t);
  jsonManager = ydNewManager(t);
  g_assert(jsonManager != NULL);
  ret = ydFromFile(jsonManager, TESTS_PATH"/simple.json", NULL);
  g_assert(ret);
  g_assert(!ydDestroyManager(jsonManager));
  g_assert(yeLen(ret) == 2);
  g_assert(yeGet(ret, "lvl") != NULL);
  g_assert(yeGet(ret, "clase") != NULL);
  g_assert(yeGetString(yeGet(ret, "clase")) != NULL);
  g_assert(g_str_equal(yeGetString(yeGet(ret, "clase")), "sukeban"));
  g_assert(yeGetInt(yeGet(ret, "lvl")) == 1);
  g_assert(!ydJsonEnd());
  YE_DESTROY(ret);
  yeEnd();
}

void testJsonMultipleObj(void)
{
  yeInitMem();
  int t = ydJsonInit();
  void *jsonManager;
  Entity *ret;
  Entity *sub;
  
  g_assert(t != -1);
  jsonManager = ydNewManager(t);
  g_assert(jsonManager != NULL);

  ret = ydFromFile(jsonManager, TESTS_PATH"/multipleObj.json", NULL);
  g_assert(ret);
  sub = yeGet(ret, "enemy position");
  g_assert(yeLen(sub) == 3);

  for (int i = 0; i < 3; ++i) {
    Entity *sub2;

    sub2 = yeGet(sub, i);
    g_assert(yeLen(sub2) == 2);

    g_assert(yeGetInt(yeGet(sub2, 0)) == (i + 1));
    g_assert(yeGetInt(yeGet(sub2, 1)) == ((i + 1) * 2));
  }
  
  sub = yeGet(ret, "size");
  g_assert(yeLen(sub) == 2);
  g_assert(yeGetInt(yeGet(sub, "x")) == 10);
  g_assert(yeGetInt(yeGet(sub, "y")) == 12);

  YE_DESTROY(ret);
  g_assert(!ydDestroyManager(jsonManager));
  g_assert(!ydJsonEnd());
  yeEnd();
}
