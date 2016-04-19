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

#include "tests.h"
#include "entity.h"

/* wrapper to satifie linker */
Entity *ygGetMod(const char *path);
void *ygGetLuaManager(void);

Entity *ygGetMod(const char *path)
{
  (void)path;
  return NULL;
}

void *ygGetLuaManager(void)
{
  return NULL;
}

int main(int argc, char **argv)
{
  g_test_init(&argc, &argv, NULL);

  g_test_add_func("/script/lua/lifecycle", testLuaScritLifecycle);
  g_test_add_func("/script/lua/entity", testLuaScritEntityBind);

  return g_test_run();
}
