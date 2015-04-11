#include <stdlib.h>
#include <glib.h>
#include "entity.h"

int main(void)
{
  Entity *test = creatStructEntity(NULL);

  destroyEntity(test);
}
