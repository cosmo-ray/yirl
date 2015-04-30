#include <stdlib.h>
#include <glib.h>
#include "entity.h"
#include "tests.h"

int main(int argc, char **argv)
{
  g_test_init(&argc, &argv, NULL);
  g_test_add_func("/entity/lifecycle/simple", testLifecycleSimple);
  g_test_add_func("/entity/lifecycle/flow", testsLifecycleFlow);
  g_test_add_func("/entity/lifecycle/complex", testsLifecycleComplex);
  g_test_add_func("/entity/setunset/simple", testsSetSimple);
  g_test_run();
}
