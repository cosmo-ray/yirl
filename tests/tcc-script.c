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
#include "tcc-script.h"
#include "game.h"

static void *addPtr(int nbArg, void **args)
{
  void *arg1 = args[0];
  void *arg2 = args[1];

  (void)nbArg;
  return (void *)((long)arg1 + (long)arg2);
}

void testTccScritLifecycle(void)
{
  void *sm = NULL;
  yeInitMem();

  g_assert(!ysTccInit());
  g_assert(!ysTccGetType());
  sm = ysNewManager(NULL, 0);
  g_assert(sm);

  g_assert(!ysRegistreFunc(sm, "addPtr", addPtr));
  g_assert((long)ysCall(sm, "addPtr", 2, 1, 2) == 3);

  g_assert(!ysLoadString(sm, "void *test(){return (void *)5;}"));
  g_assert((long)ysCall(sm, "test") == 5);
  g_assert(!ysLoadString(sm, "void *test2(){return (void *)2;}"));
  g_assert((long)ysCall(sm, "test2") == 2);
  g_assert(!ysDestroyManager(sm));
  g_assert(!ysTccEnd());
  yeEnd();
}

void testTccTestsMacros(void)
{
  void *sm = NULL;
  yeInitMem();

  g_assert(!ysTccInit());
  g_assert(!ysTccGetType());
  sm = ysNewManager(NULL, 0);
  g_assert(sm);
  g_assert(!ysLoadString(sm, "void *test(){return (void *)5;}"));
  g_assert((long)ysCall(sm, "test") == 5);
  g_assert(!ysDestroyManager(sm));
  g_assert(!ysTccEnd());
  yeEnd();
}

void testTccAddDefine(void)
{
  GameConfig cfg;

  g_assert(!ygInitGameConfig(&cfg, NULL, YNONE));
  g_assert(!ygInit(&cfg));
  ygCleanGameConfig(&cfg);

 g_assert(!ysLoadString(ygGetTccManager(),
			 "#include <yirl/game.h>"
			 "void *test(){"
			 "return (void *)ygAddDefine(\"TEST\", \"28\");"
			 "}"));
  g_assert((long)ysCall(ygGetTccManager(), "test") == 0);
  g_assert(!ysLoadString(ygGetTccManager(),
			 "void *test2(){return (void *)TEST;}"));
  g_assert((long)ysCall(ygGetTccManager(), "test2") == 28);
  g_assert((long)ysCall(ygGetTccManager(), "test") == 0);
  ygEnd();
}
