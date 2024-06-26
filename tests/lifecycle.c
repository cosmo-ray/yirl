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
  yeInitMem();

  Entity *test1 = yeCreateArray(NULL, NULL);
  Entity *test2 = yeCreateInt(1, NULL, NULL);
  Entity *test3 = yeCreateFloat(1, NULL, NULL);
  Entity *test4 = yeCreateString("test", NULL, NULL);
  Entity *test5 = yeCreateFunction("funcName", NULL, NULL, NULL);

  g_assert(yeEntitiesArraySize() == 4);
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
  yeEnd();
}

/* this tests is actually usefull only with valgrind */
void testLifecycleFlow(void)
{
  yeInitMem();

  Entity *mainStruct = yeCreateArray(NULL, NULL);
  Entity *subStruct1 = yeCreateArray(mainStruct, NULL);
  Entity *subStruct2 = yeCreateArray(NULL, NULL);
  Entity *subHash = yeCreateHash(mainStruct, NULL);
  Entity *test2 = yeCreateInt(1, subStruct2, NULL);
  Entity *test3 = yeCreateFloat(1, subStruct2, NULL);
  Entity *test4 = yeCreateString("yes ?", subHash, "a");
  Entity *test5 = yeCreateString("yes yes ?", NULL, NULL);

  g_assert(test2);
  g_assert(test3);
  g_assert(mainStruct);
  g_assert(subStruct1);
  g_assert(subStruct2);
  g_assert(yeLen(mainStruct) == 2);
  g_assert(yeLen(subStruct2) == 2);
  g_assert(yeLen(subHash) == 1);

  g_assert(!yePushBack(mainStruct, subStruct2, NULL));
  g_assert(yeLen(mainStruct) == 3);

  g_assert(mainStruct->refCount == 1);
  g_assert(subStruct1->refCount == 1);
  g_assert(test3->refCount == 1);
  g_assert(test2->refCount == 1);
  g_assert(subStruct2->refCount == 2);
  g_assert(subHash->refCount == 1);
  g_assert(test4->refCount == 1);
  g_assert(test5->refCount == 1);

  yePush(subHash, test5, "a");
  g_assert(yeLen(subHash) == 1);
  g_assert(test4->refCount == 0);
  g_assert(test5->refCount == 2);
  yeDestroy(test5);

  YE_DESTROY(subStruct2);
  g_assert(subStruct2);
  g_assert(subStruct2->refCount == 1);
  YE_DESTROY(mainStruct);
  g_assert(mainStruct == NULL);
  g_assert(test5->refCount == 0);
  yeEnd();
}

void testLifecycleComplex(void)
{
  yeInitMem();
  Entity *mainStruct = yeCreateArray(NULL, NULL);
  Entity *subStruct1 = yeCreateArray(mainStruct, NULL);
  Entity *subStruct2 = yeCreateArray(mainStruct, NULL);
  Entity *test3 = yeCreateFloat(1, subStruct2, NULL);

  g_assert(mainStruct);
  g_assert(subStruct1);
  g_assert(subStruct2);
  g_assert(!yePushBack(subStruct1, subStruct2, NULL));
  yeExpandArray(subStruct2, 5);
  yeAttach(subStruct2, test3, 2, NULL, 0);
  g_assert(mainStruct->refCount == 1);
  g_assert(subStruct1->refCount == 1);
  g_assert(subStruct2->refCount == 2);
  YE_DESTROY(mainStruct);
  g_assert(mainStruct == NULL);
  yeEnd();
}

void testLifecycleAkwarde(void)
{
  yeInitMem();
  Entity *mainStruct = yeCreateArray(NULL, NULL);
  Entity *subStruct1 = yeCreateArray(mainStruct, NULL);
  Entity *subStruct2 = yeCreateArray(subStruct1, NULL);
  Entity *test3 = yeCreateString("i am an int entity", NULL, NULL);

  g_assert(mainStruct);
  g_assert(subStruct1);
  g_assert(subStruct2);
  g_assert(test3);
  g_assert(mainStruct->refCount == 1);
  g_assert(subStruct1->refCount == 1);
  g_assert(subStruct2->refCount == 1);
  g_assert(test3->refCount == 1);
  g_assert(!yePushBack(mainStruct, test3, NULL));
  g_assert(test3->refCount == 2);
  g_assert(!yePushBack(subStruct1, test3, NULL));
  g_assert(test3->refCount == 3);
  g_assert(!yePushBack(subStruct2, test3, NULL));
  g_assert(test3->refCount == 4);
  YE_DESTROY(mainStruct);
  g_assert(test3->refCount == 1);
  g_assert(mainStruct == NULL);
  YE_DESTROY(test3);
  yeEnd();
}

void testLifeDeathRebirdAndAgain(void)
{
  yeInitMem();
  Entity *mainEnt = yeCreateArray(NULL, NULL);
  Entity *map;
  Entity *tmp;

  map = yeCreateArray(mainEnt, "map");
  g_assert(map);
  for (int i = 0; i < 2000; ++i) {
    tmp = yeCreateArray(map, NULL);
    g_assert(tmp);
    g_assert(yeCreateInt(0, tmp, NULL));
  }

  tmp = yeGet(map, 500);
  g_assert(tmp);
  yeCreateInt(1, tmp, "hr");

  int good = 0;

  for (int j = 500; j < 1000; ++j) {
    tmp = yeGet(map, j);
    for (unsigned int i = 0; i < yeLen(tmp); ++i) {
      Entity *curHero = yeGet(tmp, i);

      if (yeGetInt(curHero) == 1) {
	good = 1;

	/* You can get it Noww !!!! */
	g_assert(yeLen(tmp) == 2);
	yeRemoveChild(tmp, curHero);
	g_assert(yeLen(tmp) == 1);
	break;
      }
    }
  }
  g_assert(good);

  YE_DESTROY(mainEnt);
  yeEnd();
}

void testAutoFree_(void)
{
  YE_NEW(Array, ar);
  YE_NEW(Int, test1, 10);
  YE_NEW(INT, test, 3);
  YE_NEW(Float, testf, 10.5);
  YE_NEW(String, tests, "hej");
  YE_NEW(int, test2, 46);

  g_assert(yeGetInt(test) == 3 &&
	   yeGetInt(test1) == 10 &&
	   yeGetInt(test2) == 46);
  yePushBack(ar, test1, "test1");
  g_assert(yeGetFloat(testf) == 10.5);
  g_assert(!yeStrCmp(tests, "hej"));
  g_assert(yeGetIntAt(ar, "test1") == 10);
}

void testAutoFree(void)
{
  yeInitMem();
  testAutoFree_();
  g_assert(!yeEntitiesArraySize());
  yeEnd();
}
