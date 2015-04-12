#include <stdlib.h>
#include <glib.h>
#include "entity.h"


static void lifecycle(void)
{
  Entity *test = yeCreateStruct(NULL);
  g_assert(test);
  yeDestroy(test);
}

int main(void)
{
  lifecycle();
}
