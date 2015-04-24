#include <glib.h>
#include "entity.h"
#include "tests.h"

#define LIFECYCLE_TEST(type)			\
  Entity *test = yeCreate##type(NULL);		\
  g_assert(test);				\
  yeDestroy(test);				\
  

void lifecycle(void)
{
  Entity *test1 = yeCreateStruct(NULL);
  Entity *test2 = yeCreateInt(1, NULL);
  Entity *test3 = yeCreateFloat(1, NULL);
  Entity *test4 = yeCreateString("test", NULL);
  Entity *test5 = yeCreateFunction("funcName", NULL);
  Entity *test6 = yeCreateArray(NULL);

  g_assert(test1);
  g_assert(test2);
  g_assert(test3);
  g_assert(test4);
  g_assert(test5);
  g_assert(test6);

  yeDestroy(test1);
  yeDestroy(test2);
  yeDestroy(test3);
  yeDestroy(test4);
  yeDestroy(test5);
  yeDestroy(test6);
}
