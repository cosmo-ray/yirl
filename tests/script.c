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
#include "yirl/game.h"
#include "tests.h"

void testScriptAddFunction(void)
{
  GameConfig cfg;

  g_assert(!ygInitGameConfig(&cfg, NULL, NONE));
  g_assert(!ygInit(&cfg));
  ygCleanGameConfig(&cfg);
  g_assert(!ysLoadString(ygGetTccManager(),
			 "#include <yirl/game.h>\n"
			 "void *toto(void){"
			 "return (void *)0xfa57f00D;"
			 "}"
			 "void *init(void){"
			 "return (void *)ygRegistreFunc(0, \"toto\", NULL);"
			 "}"
			 "void *titinit(void){"
			 "return (void *)ygRegistreFunc(0, \"toto\", \"titi\");"
			 "}"));

  g_assert(!ysLoadString(ygGetLuaManager(),
			 "function callTest()\n"
			 "return toto()\nend"));

  g_assert(!ysLoadString(ygGetLuaManager(),
			 "function init()\n"
			 "return ygRegistreFunc(0, \"callTest\")\nend"));

  g_assert(ysCall(ygGetLuaManager(), "init") == NULL);
  g_assert(ysCall(ygGetTccManager(), "init") == NULL);
  g_assert(ysCall(ygGetTccManager(), "titinit") == NULL);
  g_assert((intptr_t)ysCall(ygGetTccManager(), "toto") == (intptr_t)0xfa57f00D);
  g_assert((intptr_t)ysCall(ygGetLuaManager(), "toto") == (intptr_t)0xfa57f00D);
  g_assert((intptr_t)ysCall(ygGetTccManager(), "callTest") == (intptr_t)0xfa57f00D);
  g_assert((intptr_t)ysCall(ygGetLuaManager(), "callTest") == (intptr_t)0xfa57f00D);
  g_assert((intptr_t)ysCall(ygGetLuaManager(), "titi") == (intptr_t)0xfa57f00D);
  g_assert((intptr_t)ysCall(ygGetTccManager(), "titi") == (intptr_t)0xfa57f00D);

  g_assert(!ysLoadString(ygGetTccManager(),
			 "#include <yirl/game.h>"
			 "void *testadd(int nbArgs, void **args) {\n"
			 "return (void *)((long)args[0] + (long)args[1]);"
			 "}\n"
			 "void *initadd(void){"
			 "return (void *)ygRegistreFunc(2, \"testadd\", NULL);"
			 "}"));

  g_assert(!ysCall(ygGetTccManager(), "initadd"));

  g_assert(!ysLoadString(ygGetLuaManager(),
			 "function addator()"
			 "return testadd(yloveNbrToPtr(21), yloveNbrToPtr(5))"
			 "end"));

  g_assert((long)ysCall(ygGetTccManager(), "testadd", 3, 5) == 8);
  g_assert((long)ysCall(ygGetLuaManager(), "addator") == 26);
  ygEnd();
}
