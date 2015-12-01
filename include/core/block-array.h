
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

static uint8_t nullPtr[YBA_MAX_ELEM_SIZE];

typedef struct {
  BlockArray *array;
  uint64_t mask;
  uint16_t blockPos;
  uint16_t pos;
} BlockArrayIterator;

static inline void yBlockArrayInitInternal(BlockArray *ba, size_t elemSize)
{
  g_assert(elemSize < YBA_MAX_ELEM_SIZE);
  ba->elemSize = elemSize;
  ba->elems = NULL;
  ba->blocks = NULL;
  ba->nbBlock = 0;
  ba->size = 0;
}


#define yBlockArrayInit(ba, elemType)			\
  (yBlockArrayInitInternal((ba), sizeof(elemType)))

#define yBlockArrayGetBlock(ba, bPos)					\
  (yBlockArrayIsBlockAllocated(ba, bPos) ? (ba)->blocks[(bPos)] : 0LU)


#define BLOCK_REAL_SIZE(ba)			\
  ((ba)->elemSize * 64)

static inline void yBlockArrayExpandBlocks(BlockArray *ba, int nb)
{
  uint16_t oldPos = ba->nbBlock;

  ba->nbBlock += nb;
  ba->elems = g_realloc(ba->elems, ba->nbBlock * BLOCK_REAL_SIZE(ba));
  ba->blocks = g_realloc(ba->blocks, ba->nbBlock * sizeof(uint64_t));
  ba->size = ba->nbBlock * 64 - (1 * !!ba->nbBlock);

  if (nb > 0) {
    memset(ba->elems + (oldPos  * BLOCK_REAL_SIZE(ba)), 0,
	   nb * BLOCK_REAL_SIZE(ba));
    memset(ba->blocks + oldPos, 0, nb * sizeof(uint64_t));
  }
}

#undef BLOCK_REAL_SIZE

static inline uint16_t yBlockArrayBlockPos(size_t pos)
{
  return (pos / 64);
}

static inline int yBlockArrayIsBlockAllocated(BlockArray *ba, uint16_t bPos) {
  return bPos < ba->nbBlock;
}

static inline int8_t *yBlockArrayAssureBlock(BlockArray *ba, size_t pos)
{
  uint16_t blockPos = yBlockArrayBlockPos(pos);

  if (!yBlockArrayIsBlockAllocated(ba, blockPos)) {
    yBlockArrayExpandBlocks(ba, blockPos - ba->nbBlock + 1);
  }
  return ba->elems + (pos * ba->elemSize);
}

static inline int yBlockArrayIsFree(BlockArray *ba, size_t pos)
{
  return !(yBlockArrayGetBlock(ba, yBlockArrayBlockPos(pos)) &
	   (1LLU << (pos & 63)));
}

static inline int yBlockArrayIsSet(BlockArray *ba, size_t pos)
{
  return !yBlockArrayIsFree(ba, pos);
}

static inline void yBlockArrayUnset(BlockArray *ba, size_t pos)
{
  int16_t toFree = 0;
  uint16_t bPos = yBlockArrayBlockPos(pos);

  ba->blocks[bPos] ^= (1LLU << (pos & 63));

 again:
  if ((bPos + toFree) >= 0 && ba->blocks[bPos + toFree] == 0) {
    --toFree;
    goto again;
  }

  if (toFree)
    yBlockArrayExpandBlocks(ba, toFree);
}

static inline void yBlockArraySet(BlockArray *ba, size_t pos)
{
  ba->blocks[yBlockArrayBlockPos(pos)] |= (1LLU << (pos & 63));
}

static inline void yBlockArrayCopyElemInternal(BlockArray *ba, size_t pos,
					       const void *elem)
{  
  yBlockArrayAssureBlock(ba, pos);
  memcpy(ba->elems + (pos * ba->elemSize), elem, ba->elemSize);
  yBlockArraySet(ba, pos);
  return;
}

#define yBlockArrayCopyElem(ba, pos, elem)	\
  (yBlockArrayCopyElemInternal((ba), (pos), (const void *)&(elem)))

static inline int8_t *yBlockArrayGetInternal(BlockArray *ba, size_t pos)
{
  uint16_t blockPos = yBlockArrayBlockPos(pos);

  if (!yBlockArrayIsBlockAllocated(ba, blockPos)) {
    return (int8_t *)nullPtr;
  }
  return ba->elems + (pos * ba->elemSize);
}

static inline int8_t *yBlockArrayGetPtrInternal(BlockArray *ba, size_t pos)
{
  if (yBlockArrayIsFree(ba, pos)) {
    return NULL;
  }
  return ba->elems + (pos * ba->elemSize);
}

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
  return (!it->mask && !yBlockArrayIsBlockAllocated(it->array, it->blockPos));
}

static inline void yBlockArrayIteratorIncr(BlockArrayIterator *it)
{
  if (!it->mask) {
    int j = 1;
    for (int i = it->blockPos + 1; i < it->array->nbBlock &&
	   !yBlockArrayGetBlock(it->array, i); ++i, ++j);
    it->blockPos += j;
    it->mask = yBlockArrayGetBlock(it->array, it->blockPos);
    it->pos = 0;
  }
  it->pos = YUI_GET_FIRST_BIT(it->mask);
  it->mask &= ~(1LLU << it->pos);
}

#define yBlockArrayIteratorInit(it, arrayPtr, beg)	do {		\
    (it).blockPos = yBlockArrayBlockPos((beg));				\
    (it).mask = yBlockArrayGetBlock((arrayPtr), (it).blockPos) ^	\
      Y_BLOCK_ARRAY_MAKE_SET(beg & 63);					\
    (it).pos = YUI_GET_FIRST_BIT((it).mask);				\
    (it).array = (arrayPtr);						\
    if (yBlockArrayIsFree((arrayPtr), (beg))) yBlockArrayIteratorIncr(&(it)); \
  } while (0);

static inline BlockArrayIterator yBlockArrayIteratorCreate(BlockArray *array,
							   int beg)
{
  BlockArrayIterator ret;

  yBlockArrayIteratorInit(ret, array, beg);
  return ret;
}

#define yBlockArrayIteratorGetPtr(it, type)				\
  ((type *)yBlockArrayGetInternal((it.array), (it.blockPos * 64 + it.pos)))

#define yBlockArrayIteratorGet(it, type)				\
  (*((type *)yBlockArrayGetInternal((it.array), (it.blockPos * 64 + it.pos))))


static inline void yBlockArrayFree(BlockArray *ba)
{
  g_free(ba->elems);
  g_free(ba->blocks);
  ba->nbBlock = 0;
  ba->size = 0;
}

/* Get the pos of the last used bit */
static inline size_t yBlockArrayLastPos(BlockArray *ba)
{
  return (ba->nbBlock - (1 * !!ba->nbBlock)) * 64 +
    YUI_GET_LAST_MASK_POS(yBlockArrayGetBlock(ba, ba->nbBlock - 1));
}


#endif
