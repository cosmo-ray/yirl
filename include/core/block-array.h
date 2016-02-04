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

#ifndef	_BLOCK_ARRAY_H_
#define	_BLOCK_ARRAY_H_

#include <glib.h>
#include <stdint.h>
#include <string.h>
#include "utils.h"

#define Y_BLOCK_ARRAY_BLOCK_SIZE 64
#define YBA_MAX_ELEM_SIZE 1024

typedef struct {
  uint64_t *blocks;
  int8_t *elems;
  size_t size;
  size_t elemSize;
  uint16_t nbBlock;
} BlockArray;

typedef struct {
  BlockArray *array;
  uint64_t mask;
  uint16_t blockPos;
  uint16_t pos;
} BlockArrayIterator;

typedef struct {
   void (*expand)(BlockArray *ba, int nb);
   void (*assure)(BlockArray *ba, int nb);
   void (*copy)(BlockArray *ba, int nb);
   void (*free)(BlockArray *ba, int nb);
} BlockArrayDriver;

void yBlockArrayInitInternal(BlockArray *ba, size_t elemSize);

#define yBlockArrayInit(ba, elemType)			\
  (yBlockArrayInitInternal((ba), sizeof(elemType)))

#define yBlockArrayGetBlock(ba, bPos)					\
  (yBlockArrayIsBlockAllocated(ba, bPos) ? (ba)->blocks[(bPos)] : 0LU)


void yBlockArrayExpandBlocks(BlockArray *ba, int nb);

static inline uint16_t yBlockArrayBlockPos(size_t pos)
{
  return (pos / 64);
}

static inline int yBlockArrayIsBlockAllocated(BlockArray *ba, uint16_t bPos) {
  return bPos < ba->nbBlock;
}

int8_t *yBlockArrayAssureBlock(BlockArray *ba, size_t pos);

static inline int yBlockArrayIsFree(BlockArray *ba, size_t pos)
{
  return !(yBlockArrayGetBlock(ba, yBlockArrayBlockPos(pos)) &
	   (1LLU << (pos & 63)));
}

static inline int yBlockArrayIsSet(BlockArray *ba, size_t pos)
{
  return !yBlockArrayIsFree(ba, pos);
}

void yBlockArrayUnset(BlockArray *ba, size_t pos);

static inline void yBlockArraySet(BlockArray *ba, size_t pos)
{
  ba->blocks[yBlockArrayBlockPos(pos)] |= (1LLU << (pos & 63));
}

void yBlockArrayCopyElemInternal(BlockArray *ba, size_t pos,
				 const void *elem);

#define yBlockArrayCopyElem(ba, pos, elem)	\
  (yBlockArrayCopyElemInternal((ba), (pos), (const void *)&(elem)))

int8_t *yBlockArrayGetInternal(BlockArray *ba, size_t pos);

int8_t *yBlockArrayGetPtrInternal(BlockArray *ba, size_t pos);

#define yBlockArrayGetPtr(ba, pos, type)	\
  ((type *)yBlockArrayGetInternal((ba), (pos)))

#define yBlockArrayGet(ba, pos, type)		\
  (*((type *)yBlockArrayGetInternal((ba), (pos))))

#define Y_BLOCK_ARRAY_MAKE_SET(i)		\
  ((1LLU << (i)) - 1)

#define Y_BLOCK_ARRAY_FOREACH_INT(ba, beg, elem, it, type, elemType, getter) \
  elemType elem;							\
  size_t tmpBeg##elem = (beg);						\
  for (uint16_t yfi = yBlockArrayBlockPos(beg);				\
       yfi < (ba)->nbBlock;						\
       tmpBeg##elem = 0, ++yfi)						\
    for (uint64_t tmpmask =						\
	   (yBlockArrayGetBlock((ba), yfi) ^				\
	    Y_BLOCK_ARRAY_MAKE_SET(tmpBeg##elem & 63)),			\
	   tmp##it, it;							\
	 ((tmp##it = YUI_GET_FIRST_BIT(tmpmask)) || 1) &&		\
	   ((it = yfi * 64 + tmp##it) || 1) &&				\
	   ((elem = getter(ba, it, type)) || 1) &&			\
	   tmpmask;							\
	 tmpmask &= ~(1LLU << tmp##it))


#define Y_BLOCK_ARRAY_FOREACH_SINCE(ba, beg, elem, it, type)		\
  Y_BLOCK_ARRAY_FOREACH_INT(ba, beg, elem, it, type, type, yBlockArrayGet)

#define Y_BLOCK_ARRAY_FOREACH_PTR_SINCE(ba, beg, elem, it, type)	\
  Y_BLOCK_ARRAY_FOREACH_INT(ba, beg, elem, it, type, type *, yBlockArrayGetPtr)

#define Y_BLOCK_ARRAY_FOREACH_PTR(ba, elem, it, type)		\
  Y_BLOCK_ARRAY_FOREACH_PTR_SINCE(ba, 0, elem, it, type)

#define Y_BLOCK_ARRAY_FOREACH(ba, elem, it, type)	\
  Y_BLOCK_ARRAY_FOREACH_SINCE(ba, 0, elem, it, type)

static inline int yBlockArrayIteratorIsEnd(BlockArrayIterator *it)
{
  return (!it->array ||
	  (!it->mask && !yBlockArrayIsBlockAllocated(it->array, it->blockPos)));
}

void yBlockArrayIteratorIncr(BlockArrayIterator *it);

#define yBlockArrayIteratorInit(it, arrayPtr, beg)	do {		\
    (it).blockPos = yBlockArrayBlockPos((beg));				\
    (it).array = (arrayPtr);						\
    (it).mask = yBlockArrayGetBlock((it).array, (it).blockPos);		\
    yBlockArrayIteratorIncr(&(it));					\
  } while (0);

BlockArrayIterator yBlockArrayIteratorCreate(BlockArray *array,
					     int beg);


#define yBlockArrayIteratorGetPtr(it, type)				\
  ((type *)yBlockArrayGetInternal((it.array), (it.blockPos * 64 + it.pos)))

#define yBlockArrayIteratorGet(it, type)				\
  (*((type *)yBlockArrayGetInternal((it.array), (it.blockPos * 64 + it.pos))))


void yBlockArrayFree(BlockArray *ba);

/* Get the pos of the last used bit */
	size_t yBlockArrayLastPos(BlockArray *ba);


#endif
