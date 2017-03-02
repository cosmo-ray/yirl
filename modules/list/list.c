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

static int ELEM_POS = 0;
static int NEXT_POS = 1;
static int PREV_POS = 2;
static int HEAD_GETTER_POS = 3;

void *ylist_init(void *list);
void *ylist_next(void *list);
void *ylist_prev(void *list);
void *ylist_last(void *list);
void *ylist_head(void *list);
void *ylist_insert(void *list, void *elem);
void *ylist_pop(void *list);
void *ylist_elem(void *list);


static inline void *setNewHead(Entity *old, Entity *newHead)
{
  return yeReplaceAtIdx(yeGetByIdx(old, HEAD_GETTER_POS), newHead, 0);
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

  nextRet = ylist_next(list);
  next = nextRet;
  newElem = yeReCreateArray(list, "next", NULL);
  yePushBack(newElem, elem, "elem");
  yePushBack(newElem, next, "next");
  yePushBack(newElem, list, "prev");
  yePushBack(newElem, yeGetByIdx(list, HEAD_GETTER_POS), "head_getter");
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
    yeDestroy(elem);
    return NULL;
  }

  yeReplace(prev, elem, next);
  yeReplace(next, elem, prev);
  if (ylist_head(elem) == elem) {
    setNewHead(next, next);
    yeDestroy(elem);
    return next;
  }
  yeDestroy(elem);
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
  yePushBack(newElem, yeGetByIdx(list, HEAD_GETTER_POS), "head_getter");
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
  return setNewHead(list, ylist_next(ylist_head(list)));
}

void *list_back_roll(int nbArg, void **args)
{
  Entity *list = args[0];

  if (nbArg < 1)
    return NULL;
  return setNewHead(list, ylist_prev(ylist_head(list)));
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
  return yeGetByIdx(yeGetByIdx(lst, HEAD_GETTER_POS), 0);
}

void *list_next(int nbArg, void **args)
{
  void *ret;
  if (nbArg < 1)
    return NULL;
  ret = yeGetByIdx(args[0], NEXT_POS);
  return ret;
}

void *list_prev(int nbArg, void **args)
{
  if (nbArg < 1)
    return NULL;
  return (yeGetByIdx(args[0], PREV_POS));
}

void *list_elem(int nbArg, void **args)
{
  if (nbArg < 1)
    return NULL;
  return (yeGetByIdx(args[0], ELEM_POS));
}
