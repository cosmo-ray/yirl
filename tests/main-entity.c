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

int main(int argc, char **argv)
{
  g_test_init(&argc, &argv, NULL);

  g_test_add_func("/entity/lifecycle/simple", testLifecycleSimple);
  g_test_add_func("/entity/lifecycle/flow", testLifecycleFlow);
  g_test_add_func("/entity/lifecycle/complex", testLifecycleComplex);
  g_test_add_func("/entity/lifecycle/akward", testLifecycleAkwarde);
  g_test_add_func("/entity/lifecycle/again", testLifeDeathRebirdAndAgain);

  g_test_add_func("/entity/setunset/simple", testSetSimple);
  g_test_add_func("/entity/setunset/complex", testSetComplex);
  g_test_add_func("/entity/setunset/generic", testSetGeneric);

  return g_test_run();
}
