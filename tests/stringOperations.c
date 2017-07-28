/*
**Copyright (C) 2016 Matthias Gatto
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

#include "entity.h"
#include "tests.h"

void stringsTests(void)
{
  yeInitMem();

  Entity *str = yeCreateString(NULL, NULL, NULL);

  g_assert(str);
  g_assert(yeLen(str) == 0);
  g_assert(!yeStringAddNl(str, NULL));
  g_assert(!yeStrCmp(str, "\n"));
  g_assert(yeCountCharacters(str, '\n', -1) == 1);
  g_assert(!yeStringAdd(str, "ma lite !\n"));
  g_assert(!yeStrCmp(str, "\nma lite !\n"));
  g_assert(!yeStringAddNl(str, "ma lite !"));
  g_assert(!yeStrCmp(str, "\nma lite !\nma lite !\n"));
  g_assert(yeCountCharacters(str, '\n', -1) == 3);
  g_assert(yeCountCharacters(str, '\n', 9) == 5);
  g_assert(yeCountCharacters(str, '\n', 10) == 3);
  g_assert(!yeStringShrink(str, 1));
  g_assert(!yeStrCmp(str, "ma lite !\nma lite !\n"));
  g_assert(!yeStringShrink(str, 3));
  g_assert(!yeStrCmp(str, "lite !\nma lite !\n"));
  g_assert(!yeStringAddNl(str, "2 le retour"));
  g_assert(!yeStrCmp(str, "lite !\nma lite !\n2 le retour\n"));
  yeDestroy(str);
  yeEnd();
}
