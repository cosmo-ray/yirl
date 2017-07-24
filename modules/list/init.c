/*
**Copyright (C) 2017 Matthias Gatto
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

#include <yirl/game.h>
#include <yirl/script.h>

void *init(int nbArg, void **args)
{
  Entity *t = YE_TO_ENTITY(args[0]);

  yeCreateFunctionSimple("list_init", ygGetTccManager(), t);
  yeCreateFunctionSimple("list_init_from_array", ygGetTccManager(), t);
  yeCreateFunctionSimple("list_next", ygGetTccManager(), t);
  yeCreateFunctionSimple("list_prev", ygGetTccManager(), t);

  yeCreateFunctionSimple("list_head", ygGetTccManager(), t);
  yeCreateFunctionSimple("list_last", ygGetTccManager(), t);

  yeCreateFunctionSimple("list_elem", ygGetTccManager(), t);

  yeCreateFunctionSimple("list_roll", ygGetTccManager(), t);
  yeCreateFunctionSimple("list_back_roll", ygGetTccManager(), t);

  yeCreateFunctionSimple("list_insert", ygGetTccManager(), t);
  yeCreateFunctionSimple("list_insert_before", ygGetTccManager(), t);

  yeCreateFunctionSimple("list_pop", ygGetTccManager(), t);

  yeCreateFunctionSimple("list_destroy", ygGetTccManager(), t);

  ygRegistreFunc(3, "list_init", "ylist_init");
  ygRegistreFunc(3, "list_init_from_array", "ylist_init_from_array");
  ygRegistreFunc(3, "list_to_array", "ylist_to_array");

  ygRegistreFunc(1, "list_next", "ylist_next");
  ygRegistreFunc(1, "list_prev", "ylist_prev");

  ygRegistreFunc(1, "list_head", "ylist_head");
  ygRegistreFunc(1, "list_last", "ylist_last");

  ygRegistreFunc(1, "list_elem", "ylist_elem");

  ygRegistreFunc(1, "list_roll", "ylist_roll");
  ygRegistreFunc(1, "list_back_roll", "ylist_back_roll");

  ygRegistreFunc(2, "list_insert", "ylist_insert");
  ygRegistreFunc(2, "list_insert_before", "ylist_insert_before");

  ygRegistreFunc(1, "list_pop", "ylist_pop");
  ygRegistreFunc(1, "list_destroy", "ylist_destroy");

  ygLoadScript(t, ygGetTccManager(), "./list.c");

  return NULL;
}
