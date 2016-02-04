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

#include "block-array.h"

static uint8_t nullPtr[YBA_MAX_ELEM_SIZE];

inline void yBlockArrayInitInternal(BlockArray *ba, size_t elemSize)
{
  g_assert(elemSize < YBA_MAX_ELEM_SIZE);
  ba->elemSize = elemSize;
  ba->elems = NULL;
  ba->blocks = NULL;
  ba->nbBlock = 0;
  ba->size = 0;
}

#define BLOCK_REAL_SIZE(ba)			\
  ((ba)->elemSize * 64)

void yBlockArrayExpandBlocks(BlockArray *ba, int nb)
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

inline void yBlockArrayUnset(BlockArray *ba, size_t pos)
{
  int16_t toFree = 0;
  uint16_t bPos = yBlockArrayBlockPos(pos);

  ba->blocks[bPos] ^= (1LLU << (pos & 63));

  if (bPos != yBlockArrayBlockPos(ba->size))
    return;
 again:
  if ((bPos + toFree) >= 0 && ba->blocks[bPos + toFree] == 0) {
    --toFree;
    goto again;
  }

  if (toFree)
    yBlockArrayExpandBlocks(ba, toFree);
}


int8_t *yBlockArrayAssureBlock(BlockArray *ba, size_t pos)
{
  uint16_t blockPos = yBlockArrayBlockPos(pos);

  if (!yBlockArrayIsBlockAllocated(ba, blockPos)) {
    yBlockArrayExpandBlocks(ba, blockPos - ba->nbBlock + 1);
  }
  return ba->elems + (pos * ba->elemSize);
}

inline void yBlockArrayCopyElemInternal(BlockArray *ba, size_t pos,
					       const void *elem)
{  
  yBlockArrayAssureBlock(ba, pos);
  memcpy(ba->elems + (pos * ba->elemSize), elem, ba->elemSize);
  yBlockArraySet(ba, pos);
  return;
}

inline int8_t *yBlockArrayGetInternal(BlockArray *ba, size_t pos)
{
  uint16_t blockPos = yBlockArrayBlockPos(pos);

  if (!yBlockArrayIsBlockAllocated(ba, blockPos)) {
    return (int8_t *)nullPtr;
  }
  return ba->elems + (pos * ba->elemSize);
}

inline int8_t *yBlockArrayGetPtrInternal(BlockArray *ba, size_t pos)
{
  if (yBlockArrayIsFree(ba, pos)) {
    return NULL;
  }
  return ba->elems + (pos * ba->elemSize);
}

inline void yBlockArrayIteratorIncr(BlockArrayIterator *it)
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

inline BlockArrayIterator yBlockArrayIteratorCreate(BlockArray *array,
						    int beg)
{
  BlockArrayIterator ret = {NULL, 0, 0, 0};

  if (!array)
    return ret;
  yBlockArrayIteratorInit(ret, array, beg);
  return ret;
}

inline void yBlockArrayFree(BlockArray *ba)
{
  g_free(ba->elems);
  g_free(ba->blocks);
  ba->nbBlock = 0;
  ba->size = 0;
}

inline size_t yBlockArrayLastPos(BlockArray *ba)
{
  return (ba->nbBlock - (1 * !!ba->nbBlock)) * 64 +
    YUI_GET_LAST_MASK_POS(yBlockArrayGetBlock(ba, ba->nbBlock - 1));
}
