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

void testLifecycleSimple(void)
{
  Entity *test1 = yeCreateArray(NULL, NULL);
  Entity *test2 = yeCreateInt(1, NULL, NULL);
  Entity *test3 = yeCreateFloat(1, NULL, NULL);
  Entity *test4 = yeCreateString("test", NULL, NULL);
  Entity *test5 = yeCreateFunction("funcName", NULL, NULL);

  g_assert(test1);
  g_assert(test2);
  g_assert(test3);
  g_assert(test4);
  g_assert(test5);

  YE_DESTROY(test1);
  YE_DESTROY(test2);
  YE_DESTROY(test3);
  YE_DESTROY(test4);
  YE_DESTROY(test5);

  g_assert(test1 == NULL);
  g_assert(test2 == NULL);
  g_assert(test3 == NULL);
  g_assert(test4 == NULL);
  g_assert(test5 == NULL);

}

/* this tests is actually usefull only with valgrind */
void testLifecycleFlow(void)
{
  Entity *mainStruct = yeCreateArray(NULL, NULL);
  Entity *subStruct1 = yeCreateArray(mainStruct, NULL);
  Entity *subStruct2 = yeCreateArray(NULL, NULL);
  Entity *test2 = yeCreateInt(1, subStruct2, NULL);
  Entity *test3 = yeCreateFloat(1, subStruct2, NULL);

  g_assert(test2);
  g_assert(test3);
  g_assert(mainStruct);
  g_assert(subStruct1);
  g_assert(subStruct2);
  g_assert(!yePushBack(mainStruct, subStruct2, NULL));
  g_assert(mainStruct->refCount == 1);
  g_assert(subStruct1->refCount == 1);
  g_assert(test3->refCount == 1);
  g_assert(test2->refCount == 1);
  g_assert(subStruct2->refCount == 2);
  YE_DESTROY(subStruct2);
  g_assert(subStruct2);
  g_assert(subStruct2->refCount == 1);  
  YE_DESTROY(mainStruct);
  g_assert(mainStruct == NULL);
}

void testLifecycleComplex(void)
{
  Entity *mainStruct = yeCreateArray(NULL, NULL);
  Entity *subStruct1 = yeCreateArray(mainStruct, NULL);
  Entity *subStruct2 = yeCreateArray(mainStruct, NULL);
  Entity *test3 = yeCreateFloat(1, subStruct2, NULL);

  g_assert(mainStruct);
  g_assert(subStruct1);
  g_assert(subStruct2);
  g_assert(!yePushBack(subStruct1, subStruct2, NULL));
  yeExpandArray(subStruct2, 5);
  yeAttach(subStruct2, test3, 2, NULL);
  g_assert(mainStruct->refCount == 1);
  g_assert(subStruct1->refCount == 1);
  g_assert(subStruct2->refCount == 2);
  YE_DESTROY(mainStruct);
  g_assert(mainStruct == NULL);
}
