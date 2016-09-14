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
#include <string.h>

#include "tests.h"
#include "entity.h"

void testSetSimple(void)
{
  yeInitMem();
  Entity *mainStruct = yeCreateArray(NULL, NULL);
  Entity *test1 = yeCreateInt(1, mainStruct, NULL);
  Entity *test2 = yeCreateFloat(1, mainStruct,NULL);
  Entity *test3 = yeCreateString("test", mainStruct, NULL);
  Entity *test4 = yeCreateFunction("funcName", NULL, mainStruct, NULL);

  int testInt;
  double testDouble;
  const char *testStr;
  const char *testFunc;
  
  testInt = yeGetInt(test1);
  testDouble = yeGetFloat(test2);
  testStr = yeGetString(test3);
  testFunc = yeGetFunction(test4);

  g_assert(testInt == 1);
  g_assert(testDouble == 1);
  g_assert(!strcmp("test", testStr));
  g_assert(!strcmp("funcName", testFunc));

  yeSet(test1, 2);
  yeSet(test2, (double)2.0);
  yeSet(test3, "test2");
  yeSet(test4, "funcName2");

  testInt = yeGetInt(test1);
  testDouble = yeGetFloat(test2);
  testStr = yeGetString(test3);
  testFunc = yeGetFunction(test4);

  g_assert(testInt == 2);
  g_assert(testDouble == 2);
  g_assert(!strcmp("test2", testStr));
  g_assert(!strcmp("funcName2", testFunc));

  YE_DESTROY(mainStruct);
  g_assert(mainStruct == NULL);
  yeEnd();
}

void testSetGeneric(void)
{
  yeInitMem();
  int val = 4;
  Entity *test = yeCreate(YINT, &val, NULL, NULL);
  Entity *testStr = yeCreate(YSTRING, "myJoeIsBigerThanYours", NULL, NULL);

  g_assert(!strcmp(yeGetString(testStr), "myJoeIsBigerThanYours"));
  g_assert(yeGetInt(test) == val);
  YE_DESTROY(test);
  g_assert(test == NULL);
  YE_DESTROY(testStr);
  g_assert(testStr == NULL);
  yeEnd();
}

void testSetComplex(void)
{
  yeInitMem();
  Entity *mainStruct = yeCreateArray(NULL, NULL);

  for (int i = 0; i < 10; ++i) {
    char *tmp = g_strdup_printf("Dude%d", i);
    
    yeCreateInt(i, mainStruct, tmp);
    g_free(tmp);
  }

  for (int i = 0; i < 10; ++i) {
    g_assert(yeGet(mainStruct, i));
    g_assert(yeGetInt(yeGet(mainStruct, i)) == i);
    yeSetAt(mainStruct, i, i * i);
  }

  for (int i = 0; i < 10; ++i) {
    char *tmp = g_strdup_printf("Dude%d", i);

    g_assert(yeGetInt(yeGet(mainStruct, tmp)) == i * i);
    g_free(tmp);
  }

  YE_DESTROY(mainStruct);
  g_assert(mainStruct == NULL);
  yeEnd();
}
