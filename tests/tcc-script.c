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
#include "tcc-script.h"

static void *addPtr(int nbArg, va_list *ap)
{
  void *arg1 = va_arg(ap, void *);
  void *arg2 = va_arg(ap, void*);

  printf("hello %d\n", nbArg);

  return (void *)((long)arg1 + (long)arg2);
}

void testTccScritLifecycle(void)
{
  void *sm = NULL;

  g_assert(!ysTccInit());
  g_assert(!ysTccGetType());
  sm = ysNewManager(NULL, 0);
  g_assert(sm);

  g_assert(!ysRegistreFunc(sm, "addPtr", addPtr));
  g_assert((long)ysCall(sm, "addPtr", 2, 1, 2) == 3);

  /* g_assert(!ysRegistreFunc(sm, "toNbr", luaToNumber)); */
  /* g_assert(!ysRegistreFunc(sm, "toPtr", luaToPtr)); */

  /* if (ysLoadFile(sm, TESTS_PATH"/simple.lua")) { */
  /*   ysPrintError(sm); */
  /*   g_assert(0); */
  /* } */

  /* g_assert((long)ysCall(sm, "addPtr2", 2, 1, 2) == 3); */

  g_assert(!ysDestroyManager(sm));
  g_assert(!ysTccEnd());
}
