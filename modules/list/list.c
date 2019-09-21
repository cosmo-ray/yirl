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
#include "list.h"

static int ELEM_POS = 0;
static int NEXT_POS = 1;
static int PREV_POS = 2;
static int HEAD_GETTER_POS = 3;

static inline Entity *headFather(Entity *elem)
{
  return yeGet(yeGet(elem, HEAD_GETTER_POS), 1);
}


static inline void *setNewHead(Entity *old, Entity *newHead)
{
  Entity *father = headFather(old);
  Entity *ret;

  if (!father) {
    yeIncrRef(newHead);
    yeDestroy(old);
  } else {
    yeReplace(father, old, newHead);
  }
  ret = yeReplaceAtIdx(yeGet(old, HEAD_GETTER_POS), newHead, 0);
  return ret;
}

void *list_init(int nbArg, void **args)
{
  Entity *elem = nbArg > 0 ? args[0] : NULL;
  Entity *father = nbArg > 1 ? args[1] : NULL;
  const char *fatherName = nbArg > 2 ? args[2] : NULL;
  Entity *ret = yeCreateArray(father, fatherName);
  Entity *headGetter;

  if (!elem)
    return NULL;
  yePushBack(ret, elem, "elem");
  yePushBack(ret, ret, "next");
  yePushBack(ret, ret, "prev");
  headGetter = yeCreateArray(ret, "head_getter");
  yePushBack(headGetter, ret, "head");
  if (father)
    yePushBack(headGetter, father, "father");
  return ret;
}

void *list_init_from_array(int nbArg, void **args)
{
  Entity *elem = nbArg > 0 ? args[0] : NULL;
  Entity *father = nbArg > 1 ? args[1] : NULL;
  const char *fatherName = nbArg > 2 ? args[2] : NULL;
  int first = 1;
  Entity *ret = NULL;

  YE_ARRAY_FOREACH(elem, var) {
    if (first) {
      ret = ylist_init(var, father, (void *)fatherName);
      first = 0;
      continue;
    }
    ylist_insert(ret, var);
    ret = ylist_next(ret);
  }
  return ylist_head(ret);
}

void *list_to_array(int nbArg, void **args)
{
  Entity *list = nbArg > 0 ? args[0] : NULL;
  Entity *father = nbArg > 1 ? args[1] : NULL;
  const char *fatherName = nbArg > 2 ? args[2] : NULL;
  Entity *head = list;
  Entity *ret = yeReCreateArray(father, fatherName, NULL);

  do {
    yePushBack(ret, ylist_elem(list), NULL);
    list = ylist_next(list);
  } while (list != head);
  return ret;
}

void *list_insert(int nbArg, void **args)
{
  Entity *list = nbArg > 0 ? args[0] : NULL;
  Entity *elem = nbArg > 1 ? args[1] : NULL;
  void *nextRet;
  Entity *next;
  Entity *newElem;

  if (!list || !elem)
    return NULL;

  next = ylist_next(list);
  newElem = yeReCreateArray(list, "next", NULL);
  yePushBack(newElem, elem, "elem");
  yePushBack(newElem, next, "next");
  yePushBack(newElem, list, "prev");
  yePushBack(newElem, yeGet(list, HEAD_GETTER_POS), "head_getter");
  yeReplaceAtIdx(next, newElem, PREV_POS);
  return ylist_head(list);
}

void *list_pop(int nbArg, void **args)
{
  Entity *elem = nbArg > 0 ? args[0] : NULL;
  Entity *next;
  Entity *prev;

  if (!elem) {
    DPRINT_ERR("can't remove an NULL elem");
    return NULL;
  }

  next = ylist_next(elem);
  prev = ylist_prev(elem);
  /* it's the last elem */
  if (prev == elem) {
    yeClearArray(yeGet(elem, HEAD_GETTER_POS));
    yeClearArray(elem);
    yeDestroy(elem);
    return NULL;
  }

  yeReplace(prev, elem, next);
  if (ylist_head(next) == elem) {
    setNewHead(elem, next);
    yeReplace(next, elem, prev);
    return next;
  }
  yeReplace(next, elem, prev);
  return ylist_head(next);
}

void *list_destroy(int nbArg, void **args)
{
  void *list = nbArg > 0 ? args[0] : NULL;

  while (list) {
    list = ylist_pop(list);
  }
  return NULL;
}

void *list_insert_before(int nbArg, void **args)
{
  Entity *list = nbArg > 0 ? args[0] : NULL;
  Entity *elem = nbArg > 1 ? args[1] : NULL;
  Entity *prev;
  Entity *newElem;

  if (!list || !elem)
    return NULL;

  prev = ylist_prev(list);
  newElem = yeReCreateArray(list, "prev", NULL);
  yePushBack(newElem, elem, "elem");
  yePushBack(newElem, list, "next");
  yePushBack(newElem, prev, "prev");
  yePushBack(newElem, yeGet(list, HEAD_GETTER_POS), "head_getter");
  yeReplaceAtIdx(prev, newElem, NEXT_POS);
  if (ylist_head(list) != list)
    return ylist_head(list);
  return setNewHead(list, newElem);
}

void *list_roll(int nbArg, void **args)
{
  Entity *list = args[0];
  if (nbArg < 1)
    return NULL;
  return setNewHead(ylist_head(list), ylist_next(ylist_head(list)));
}

void *list_back_roll(int nbArg, void **args)
{
  Entity *list = args[0];

  if (nbArg < 1)
    return NULL;
  return setNewHead(ylist_head(list), ylist_prev(ylist_head(list)));
}

void *list_last(int nbArg, void **args)
{
  Entity *lst = args[0];
  if (nbArg < 1)
    return NULL;
  return ylist_prev(ylist_head(lst));
}

void *list_head(int nbArg, void **args)
{
  Entity *lst = args[0];

  if (nbArg < 1)
    return NULL;
  return yeGet(yeGet(lst, HEAD_GETTER_POS), 0);
}

void *list_next(int nbArg, void **args)
{
  void *ret;
  if (nbArg < 1)
    return NULL;
  ret = yeGet(args[0], NEXT_POS);
  return ret;
}

void *list_prev(int nbArg, void **args)
{
  if (nbArg < 1)
    return NULL;
  return (yeGet(args[0], PREV_POS));
}

void *list_elem(int nbArg, void **args)
{
  if (nbArg < 1)
    return NULL;
  return (yeGet(args[0], ELEM_POS));
}
