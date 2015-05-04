#include <glib.h>
#include "entity.h"
#include "tests.h"  

void testLifecycleSimple(void)
{
  Entity *test1 = yeCreateStruct("tests", NULL);
  Entity *test2 = yeCreateInt(NULL, 1, NULL);
  Entity *test3 = yeCreateFloat(NULL, 1, NULL);
  Entity *test4 = yeCreateString(NULL, "test", NULL);
  Entity *test5 = yeCreateFunction(NULL, "funcName", NULL);
  Entity *test6 = yeCreateArray(NULL, NULL);

  g_assert(test1);
  g_assert(test2);
  g_assert(test3);
  g_assert(test4);
  g_assert(test5);
  g_assert(test6);

  YE_DESTROY(test1);
  YE_DESTROY(test2);
  YE_DESTROY(test3);
  YE_DESTROY(test4);
  YE_DESTROY(test5);
  YE_DESTROY(test6);
  g_assert(test1 == NULL);
  g_assert(test2 == NULL);
  g_assert(test3 == NULL);
  g_assert(test4 == NULL);
  g_assert(test5 == NULL);
  g_assert(test6 == NULL);

}

/* this tests is actually usefull only with valgrind */
void testsLifecycleFlow(void)
{
  Entity *mainStruct = yeCreateStruct(NULL, NULL);
  Entity *subStruct1 = yeCreateStruct(NULL, mainStruct);
  Entity *subStruct2 = yeCreateArray(NULL, NULL);
  Entity *test2 = yeCreateInt(NULL, 1, subStruct2);
  Entity *test3 = yeCreateFloat(NULL, 1, subStruct2);

  g_assert(test2);
  g_assert(test3);
  g_assert(mainStruct);
  g_assert(subStruct1);
  g_assert(subStruct2);
  g_assert(!yePushBack(mainStruct, subStruct2));
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

void testsLifecycleComplex(void)
{
  Entity *mainStruct = yeCreateStruct(NULL, NULL);
  Entity *subStruct1 = yeCreateStruct(NULL, mainStruct);
  Entity *subStruct2 = yeCreateArray(NULL, mainStruct);

  g_assert(mainStruct);
  g_assert(subStruct1);
  g_assert(subStruct2);
  g_assert(!yePushBack(subStruct1, subStruct2));
  g_assert(mainStruct->refCount == 1);
  g_assert(subStruct1->refCount == 1);
  g_assert(subStruct2->refCount == 2);
  YE_DESTROY(mainStruct);
  g_assert(mainStruct == NULL);
}
