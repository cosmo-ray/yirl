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

#include <glib.h>
#include "utils.h"
#include "block-array.h"
#include <numa.h>

#define NUMA_SIZE 0xfffffff

inline void yBlockArrayInitInternal(BlockArray *ba, size_t elemSize, int flag)
{
  g_assert(elemSize < YBA_MAX_ELEM_SIZE);
  ba->elemSize = elemSize;
  if (flag & YBLOCK_ARRAY_NUMA)
    ba->elems = numa_alloc(NUMA_SIZE);
  else
    ba->elems = NULL;
  ba->blocks = NULL;
  ba->flag = flag;
  ba->lastPos = 0;
  ba->nbBlock = 0;
  ba->size = 0;
}

#define BLOCK_REAL_SIZE(ba)			\
  ((ba)->elemSize * 64)

void yBlockArrayExpandBlocks(BlockArray *ba, int nb)
{
  uint64_t oldPos = ba->nbBlock;

  if (ba->flag & YBLOCK_ARRAY_NUMA)
    nb += 16;
  ba->nbBlock += nb;
  if (!(ba->flag & YBLOCK_ARRAY_NUMA))
    ba->elems = g_realloc(ba->elems, ba->nbBlock * BLOCK_REAL_SIZE(ba));
  ba->blocks = g_realloc(ba->blocks, ba->nbBlock * sizeof(uint64_t));
  ba->size = ba->nbBlock * 64 - (1 * !!ba->nbBlock);

  if (nb > 0) {
    if (!(ba->flag & YBLOCK_ARRAY_NOINIT))
      memset(ba->elems + (oldPos  * BLOCK_REAL_SIZE(ba)), 0,
	     nb * BLOCK_REAL_SIZE(ba));
    memset(ba->blocks + oldPos, 0, nb * sizeof(uint64_t));
  }
}

#undef BLOCK_REAL_SIZE


inline void yBlockArrayCopyElemInternal(BlockArray *ba, size_t pos,
					       const void *elem)
{  
  yBlockArrayAssureBlock(ba, pos);
  memcpy(ba->elems + (pos * ba->elemSize), elem, ba->elemSize);
  yBlockArraySet(ba, pos);
  return;
}


inline void yBlockArrayIteratorIncr(BlockArrayIterator *it)
{
  if (!it->mask) {
    uint64_t j = 1;
    for (uint64_t i = it->blockPos + 1; i < it->array->nbBlock &&
	   !yBlockArrayGetBlock(*it->array, i); ++i, ++j);
    it->blockPos += j;
    it->mask = yBlockArrayGetBlock(*it->array, it->blockPos);
    it->pos = 0;
  }
  it->pos = YUI_GET_FIRST_BIT(it->mask);
  it->mask &= ~(ONE64 << it->pos);
}

inline BlockArrayIterator yBlockArrayIteratorCreate(BlockArray *array,
						    int beg)
{
  BlockArrayIterator ret = {NULL, 0, 0, 0};

  if (unlikely(!array))
    return ret;
  yBlockArrayIteratorInit(ret, array, beg);
  return ret;
}

inline void yBlockArrayFree(BlockArray *ba)
{
  if (ba->flag & YBLOCK_ARRAY_NUMA)
    numa_free(ba->elems, NUMA_SIZE);
  else
    g_free(ba->elems);
  g_free(ba->blocks);
  ba->nbBlock = 0;
  ba->size = 0;
}
