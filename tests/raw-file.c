/*
**Copyright (C) 2017 Matthias Gatto
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
#include "rawfile-desc.h"
#include "description.h"

void testRawFileLoad(void)
{
  yeInitMem();
  int t = ydRawFileInit();
  void *jsonManager;
  Entity *ret;

  g_assert(t != -1);
  g_assert(ydRawFileGetType() == t);
  jsonManager = ydNewManager(t);
  g_assert(jsonManager != NULL);
  ret = ydFromFile(jsonManager, TESTS_PATH"/simple.txt", NULL);
  g_assert(ret);
  g_assert(!yeStrCmp(ret, "mega titi\n"));
  g_assert(!ydDestroyManager(jsonManager));
  YE_DESTROY(ret);
  yeEnd();
}
