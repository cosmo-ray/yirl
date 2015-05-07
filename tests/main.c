#include <stdlib.h>
#include <glib.h>
#include "entity.h"
#include "debug.h"
#include "tests.h"

int main(int argc, char **argv)
{
  g_test_init(&argc, &argv, NULL);
  debug_init();
  g_test_add_func("/entity/lifecycle/simple", testLifecycleSimple);
  g_test_add_func("/entity/lifecycle/flow", testLifecycleFlow);
  g_test_add_func("/entity/lifecycle/complex", testLifecycleComplex);
  g_test_add_func("/entity/setunset/simple", testSetSimple);
  g_test_add_func("/entity/setunset/complex", testSetComplex);
  g_test_add_func("/entity/setunset/generic", testSetGeneric);
  g_test_run();
  debug_exit();
}
