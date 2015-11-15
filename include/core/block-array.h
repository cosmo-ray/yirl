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

#define Y_BLOCK_ARRAY_BLOCK_SIZE 64

typedef struct {
  uint64_t *blocks;
  int8_t *elems;
  size_t lastElem;
  size_t elemSize;
  uint16_t nbBlock;
} BlockArray;

static intptr_t nullPtr = 0;

static inline void yBlockArrayInitInternal(BlockArray *ba, size_t elemSize)
{
  ba->elemSize = elemSize;
  ba->elems = NULL;
  ba->blocks = NULL;
  ba->nbBlock = 0;
  ba->lastElem = 0;
}

#define yBlockArrayInit(ba, elemType)			\
  (yBlockArrayInitInternal((ba), sizeof(elemType)))

#define yBlockArrayGetBlock(ba, pos)			\
  (ba)->blocks[(pos)]

#define BLOCK_REAL_SIZE(ba)			\
  ((ba)->elemSize * 64)

static inline void yBlockArrayExpandBlocks(BlockArray *ba, int nb)
{
  uint16_t oldPos = ba->nbBlock;

  ba->nbBlock += nb;
  ba->elems = g_realloc(ba->elems, ba->nbBlock * BLOCK_REAL_SIZE(ba));
  ba->blocks = g_realloc(ba->blocks, sizeof(uint64_t) * ba->nbBlock);

  if (nb > 0) {
    memset(ba->elems + (oldPos  * BLOCK_REAL_SIZE(ba)), 0,
	   nb * BLOCK_REAL_SIZE(ba));
    memset(ba->blocks + (oldPos  * sizeof(uint64_t)),
	   0, nb * sizeof(uint64_t));
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
  return !(ba->blocks[yBlockArrayBlockPos(pos)] & (1LLU << (pos & 63LLU)));
}

static inline void yBlockUnset(BlockArray *ba, size_t pos)
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
  if (pos > ba->lastElem)
    ba->lastElem = pos;
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

#define yBlockArrayGet(ba, pos, type)		\
  (*((type *)yBlockArrayGetInternal((ba), (pos))))

static inline void yBlockArrayFree(BlockArray *ba)
{
  g_free(ba->elems);
  g_free(ba->blocks);
  ba->nbBlock = 0;
  ba->lastElem = 0;
}


#endif
