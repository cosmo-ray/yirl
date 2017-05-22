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

#include "yirl/game.h"
#include "tests.h"

void testListMod(void)
{
  yeInitMem();
  GameConfig cfg;
  Entity *gc = yeCreateArray(NULL, NULL);
  Entity *e1 = yeCreateInt(0, gc, NULL);
  Entity *e2 = yeCreateInt(0, gc, NULL);
  Entity *e3 = yeCreateInt(0, gc, NULL);

  Entity *l;
  Entity *l2;
  Entity *l3;

  g_assert(!ygInitGameConfig(&cfg, NULL, NONE));
  g_assert(!ygInit(&cfg));
  ygLoadMod("../modules/list/");

#define list_init_from_array(elem, father, name) ysCall(ygGetTccManager(), \
							"list_init_from_array",	\
							elem, father, name)
#define list_elem(list) ysCall(ygGetTccManager(), "list_elem", list)
#define list_insert(list, elem) ysCall(ygGetTccManager(), "list_insert", \
				       list, elem)
#define list_next(list) ysCall(ygGetTccManager(), "list_next", list)
#define list_prev(list) ysCall(ygGetTccManager(), "list_prev", list)
#define list_head(list) ysCall(ygGetTccManager(), "list_head", list)
#define list_last(list) ysCall(ygGetTccManager(), "list_last", list)
#define list_pop(list) ysCall(ygGetTccManager(), "list_pop", list)
#define list_insert_before(list, elem) ysCall(ygGetTccManager(),	\
					      "list_insert_before",	\
					      list, elem)
#define list_roll(list) ysCall(ygGetTccManager(), "list_roll", list)
#define list_back_roll(list) ysCall(ygGetTccManager(), "list_back_roll", list)
#define list_to_array(list, father, name) ysCall(ygGetTccManager(),\
						 "list_to_array",  \
						 list, father, name)

  l = ysCall(ygGetTccManager(), "list_init", e1);
  g_assert(l);
  g_assert(list_elem(l) == e1);
  g_assert(list_prev(l) == l);
  g_assert(list_next(l) == l);

  /* test insert */
  g_assert(list_insert(l, e2) == l);
  g_assert(list_elem(list_next(l)) == e2);
  g_assert(list_elem(list_prev(l)) == e2);
  g_assert(list_elem(list_next(list_next(l))) == e1);
  g_assert(list_elem(list_next(list_prev(l))) == e1);

  /* pop second elem */
  l2 = list_pop(list_next(l));
  g_assert(l2 == l);
  g_assert(list_elem(l) == e1);
  g_assert(list_prev(l) == l);
  g_assert(list_next(l) == l);

  /* insert l2 before l1 */
  l2 = list_insert_before(l, e2);
  g_assert(l2 != l);
  g_assert(list_elem(l) == e1);
  g_assert(list_elem(l2) == e2);
  g_assert(list_elem(list_next(l)) == e2);
  g_assert(list_elem(list_prev(l)) == e2);
  g_assert(list_elem(list_next(list_next(l))) == e1);
  g_assert(list_elem(list_next(list_prev(l))) == e1);

  /* insert l3 before l1 */
  /* l2 -> l3 -> l */
  l3 = list_insert_before(l, e3);
  g_assert(l3 == l2);
  l3 = list_next(l2);
  g_assert(list_head(l) == l2);
  g_assert(l3 != l2);
  g_assert(list_elem(l) == e1);
  g_assert(list_elem(l2) == e2);
  g_assert(list_elem(l3) == e3);

  g_assert(list_elem(list_next(l)) == e2);
  g_assert(list_elem(list_prev(l)) == e3);
  g_assert(list_elem(list_next(l3)) == e1);
  g_assert(list_elem(list_prev(l3)) == e2);
  g_assert(list_elem(list_next(list_next(l))) == e3);
  g_assert(list_elem(list_next(list_prev(l))) == e1);

  l3 = list_pop(l3);
  g_assert(l3 == l2);
  g_assert(list_head(l) == l2);
  g_assert(list_next(l2) == l);
  g_assert(list_prev(l2) == l);
  g_assert(list_next(l) == l2);
  g_assert(list_prev(l) == l2);

  l2 = list_pop(l2);
  g_assert(list_head(l) == l);
  g_assert(l == l2);
  g_assert(list_next(l) == l2);
  g_assert(list_prev(l) == l2);

  g_assert(list_insert(l, e2) == l);
  l2 = list_last(l);
  g_assert(list_insert(l2, e2) == l);
  l3 = list_last(l);
  g_assert(l != l2);
  g_assert(l2 != l3);
  g_assert(l != l3);
  g_assert(list_prev(l) == l3);
  g_assert(list_next(l) == l2);
  g_assert(list_next(l2) == l3);
  g_assert(list_head(l) == l);

  g_assert(l2 == list_roll(l3));
  g_assert(l3 == list_roll(l3));
  g_assert(l == list_roll(l3));

  g_assert(l3 == list_back_roll(l3));
  g_assert(l2 == list_back_roll(l3));
  g_assert(l == list_back_roll(l3));

  ysCall(ygGetTccManager(), "list_destroy", l);

  l = list_init_from_array(gc, gc, "list :p");
  g_assert(l);
  g_assert(list_elem(l) == e1);
  g_assert(list_elem(list_next(l)) == e2);
  g_assert(list_elem(list_prev(l)) == e3);
  g_assert(list_elem(list_next(list_next(l))) == e3);
  g_assert(list_elem(list_next(list_prev(l))) == e1);
  g_assert(yeGet(gc, "list :p") == l);
  Entity *ar2 = list_to_array(l, gc, "ar2");
  g_assert(ar2 && ar2 == yeGet(gc, "ar2"));
  g_assert(yeGet(ar2, 0) == e1);
  g_assert(yeGet(ar2, 1) == e2);
  g_assert(yeGet(ar2, 2) == e3);
  ygCleanGameConfig(&cfg);

  yeDestroy(gc);
  ygEnd();
}
