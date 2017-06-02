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

#ifndef YIRL_LIST_H_
#define YIRL_LIST_H_

void *ylist_init(void *elem, void *father, void *name);
void *ylist_init_from_array(void *elems, void *father, void *name);
void *ylist_next(void *list);
void *ylist_prev(void *list);
void *ylist_last(void *list);
void *ylist_head(void *list);
void *ylist_insert(void *list, void *elem);
void *ylist_insert_before(void *list, void *elem);
void *ylist_pop(void *list);
void *ylist_elem(void *list);
void *ylist_roll(void *list);
void *ylist_roll_back(void *list);

#define YLIST_FOREACH(head, elem)					\
  for (Entity *cur = NULL; head != cur &&				\
	 (cur = cur == NULL ? head : cur); cur = ylist_next(cur))

#endif
