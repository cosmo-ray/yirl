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
#include "entity.h"
#include "tests.h"  

void	testCopy(void)
{
  yeInitMem();
  Entity *array = yeCreateArray(NULL, NULL);
  Entity *array2 = yeCreateArray(NULL, NULL);
  Entity *tmp = array2;

  g_assert(array);
  g_assert(array2);
  yeCreateInt(1, array, NULL);
  g_assert(yeLen(array) == 1);
  g_assert(yeGetInt(yeGet(array, 0)) == 1);
  array2 = yeCopy(array, array2);
  g_assert(tmp == array2);
  g_assert(yeLen(array2) == 1);
  g_assert(yeGetInt(yeGet(array2, 0)) == 1);

  yeSetInt(yeGet(array2, 0), 7);
  g_assert(yeGetInt(yeGet(array2, 0)) == 7);
  yeCopy(array2, array);
  g_assert(yeGetInt(yeGet(array, 0)) == 7);  
  
  yeDestroy(array);
  yeDestroy(array2);
  yeEnd();
}
