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

#include "utils.h"

/* TODO: uniformize that ... */
#define YBLOCK_ARRAY_BIG_CHUNK 1
#define YBLOCK_ARRAY_NOINIT 2
#define YBLOCK_ARRAY_NOMIDFREE 4
#define YBLOCK_ARRAY_NO_ALIGN 8
#define YBLOCK_ARRAY_NO_BLOCKS_NEXT0 16

#define Y_BLOCK_ARRAY_BLOCK_SIZE 64
#define YBA_MAX_ELEM_SIZE 1024

extern size_t yBlockArrayDataNextSize0;

typedef struct {             /* Byte cnt */
	uint64_t *blocks;    /* 0-7      */
	int8_t *elems;       /* 8-15     */
	int32_t block_cnt;   /* 16-19    */
	uint16_t elem_size;  /* 20-21    */
	uint16_t flag;       /* 22-23    */
} BlockArray;

typedef struct {
	BlockArray *array;
	uint64_t mask;
	uint16_t blockPos;
	uint16_t pos;
} BlockArrayIterator;

#define yBlockArrayIsBlockAllocated(ba, bPos) (bPos < SHRT_MAX && ((int32_t)bPos) < (ba).block_cnt)

#define yBlockArrayGetBlock(ba, bPos)					\
  (yBlockArrayIsBlockAllocated((ba), bPos) ? (ba).blocks[(bPos)] : 0)


static inline int32_t yBlockArrayComputeLastBlock(BlockArray *restrict const ba)
{
	int lastBlock;

	for (lastBlock = ba->block_cnt - 1; lastBlock >= 0; --lastBlock) {
		if (ba->blocks[lastBlock])
			return lastBlock;
	}
	return -1;
}

#define yBlockArrayFastComputeLastPos(ba, blockPos)			\
	(blockPos * 64 + YUI_GET_LAST_MASK_POS((ba).blocks[blockPos], -1))

static inline int32_t yBlockArrayComputeLastPos(BlockArray *ba)
{
	int32_t ret = yBlockArrayComputeLastBlock(ba);

	if (unlikely(ret < 0))
		return -1;
	return yBlockArrayFastComputeLastPos(*ba, ret);
}

#define yBlockArrayLastPos(ba) (yBlockArrayComputeLastPos(&ba))

void yBlockArrayInitInternal(BlockArray *ba, uint32_t elemSize, int flag);

#define yBlockArrayInit(ba, elemType)			\
  (yBlockArrayInitInternal((ba), sizeof(elemType), 0))

#define yBlockArrayInitExt(ba, elemType, flag)			\
  (yBlockArrayInitInternal((ba), sizeof(elemType), flag))


void yBlockArrayExpandBlocks(BlockArray *ba, int nb);

#define yBlockArrayBlockPos(pos) (pos / 64)

static inline int64_t yBlockArrayPosMask(uint64_t pos)
{
	return ONE64 << (pos & 63);
}

static inline int8_t *yBlockArrayAssureBlock(BlockArray *ba, size_t pos)
{
	uint64_t blockPos = yBlockArrayBlockPos(pos);

	if (unlikely(!yBlockArrayIsBlockAllocated(*ba, blockPos))) {
		yBlockArrayExpandBlocks(ba, blockPos - ba->block_cnt + 1);
	}
	return ba->elems + (pos * ba->elem_size);
}

#define yBlockArrayIsFree(ba, pos)				\
	(!(yBlockArrayGetBlock(ba, yBlockArrayBlockPos(pos)) &	\
	   yBlockArrayPosMask(pos)))

#define yBlockArrayIsSet(ba, pos) (!yBlockArrayIsFree(ba, pos))

static inline void yBlockArrayUnset(BlockArray *ba, int32_t pos)
{
	int16_t toFree = 0;
	uint16_t bPos = yBlockArrayBlockPos(pos);

	ba->blocks[bPos] &= (~yBlockArrayPosMask(pos));
	if (bPos == (ba->block_cnt - 1) && !ba->blocks[bPos]) {
		if (ba->flag & YBLOCK_ARRAY_NOMIDFREE) {
			ba->block_cnt = yBlockArrayComputeLastBlock(ba) + 1;
			return;
		}
	} else {
		return;
	}

again:
	if ((bPos + toFree) >= 0 && ba->blocks[bPos + toFree] == 0) {
		--toFree;
		goto again;
	}

	if (toFree)
		yBlockArrayExpandBlocks(ba, toFree);
}

static inline void yBlockArrayFreeBlocks(BlockArray *ba)
{
	if (!(ba->flag & YBLOCK_ARRAY_NO_BLOCKS_NEXT0) &&
	    ba->block_cnt * sizeof(uint64_t) >= yBlockArrayDataNextSize0)
		free(ba->blocks);
}

static inline void yBlockArrayClear(BlockArray *ba)
{
	if (!(ba->flag & YBLOCK_ARRAY_BIG_CHUNK)) {
		free(ba->elems);
		ba->elems = NULL;
	}
	yBlockArrayFreeBlocks(ba);
	ba->blocks = NULL;
	ba->block_cnt = 0;
}

static inline void yBlockArraySet(BlockArray *ba, int32_t pos)
{
	uint64_t blockPos = yBlockArrayBlockPos(pos);

	if (unlikely(!yBlockArrayIsBlockAllocated(*ba, blockPos))) {
		yBlockArrayExpandBlocks(ba, blockPos - ba->block_cnt + 1);
	}

	ba->blocks[blockPos] |= yBlockArrayPosMask(pos);
}

void yBlockArrayCopyElemInternal(BlockArray *ba, size_t pos,
				 const void *elem);

#define yBlockArrayCopyElem(ba, pos, elem)	\
  (yBlockArrayCopyElemInternal((ba), (pos), (const void *)&(elem)))

#define yBlockArrayFastGet(ba, pos) ((ba).elems + ((pos) * (ba).elem_size))

#define yBlockArrayGetDirect(ba, pos, type)	\
  (*((type *)yBlockArrayFastGet((ba), (pos))))

#define yBlockArrayGetPtrDirect(ba, pos, type)		\
  ((type *)yBlockArrayFastGet((ba), (pos)))

#define yBlockArrayGetSuperDirect(ba, pos, type)	\
	((ba).elems + (pos) * sizeof(type))

static inline int8_t *yBlockArrayGetInternal(BlockArray *ba, size_t pos)
{
	if (unlikely(!yBlockArrayIsBlockAllocated(*ba, yBlockArrayBlockPos(pos)))) {
		static uint8_t nullPtr[YBA_MAX_ELEM_SIZE];
		return (int8_t *)&nullPtr[0];
	}
	return yBlockArrayFastGet(*ba, pos);
}

static inline int8_t *yBlockArraySetGetPtrInternal(BlockArray *ba, size_t pos)
{
	yBlockArraySet(ba, pos);
	return yBlockArrayFastGet(*ba, pos);
}

#define yBlockArraySetGetPtr(ba, pos, type)		\
  ((type *)yBlockArraySetGetPtrInternal((ba), (pos)))

#define yBlockArrayGetPtr(ba, pos, type)	\
  ((type *)yBlockArrayGetInternal((ba), (pos)))

#define yBlockArrayTryGetPtr(ba, pos, type)	\
	(yBlockArrayIsSet((*ba), (pos)) ?		\
	 (type *)yBlockArrayGetInternal((ba), (pos)) :	\
	(type *)NULL)

#define yBlockArrayGet(ba, pos, type)		\
  (*((type *)yBlockArrayGetInternal((ba), (pos))))

#define Y_BLOCK_ARRAY_MAKE_SET(i)		\
  ((ONE64 << (i)) - 1LLU)

#define Y_BLOCK_ARRAY_FOREACH_INT(ba, beg, elem, it, type, elemType, getter) \
  elemType elem;							\
  uint64_t tmpBeg##elem = (beg);					\
  (void)elem;								\
  for (uint16_t yfi = yBlockArrayBlockPos(beg);				\
       yfi < (ba).block_cnt;						\
       tmpBeg##elem = 0, ++yfi)						\
    for (uint64_t tmpmask =						\
	   ((ba).blocks[yfi] &						\
	    (~Y_BLOCK_ARRAY_MAKE_SET(tmpBeg##elem & 63LLU))),		\
	   tmp##it, it;							\
	 tmpmask &&							\
		 ({ tmp##it = ctz64(tmpmask); (void)it;			\
			 it = yfi * 64 + tmp##it;			\
			 elem = (elemType)getter(ba, it, type); 1; });	\
	 tmpmask ^= (1LLU << tmp##it))


#define Y_BLOCK_ARRAY_FOREACH_SINCE(ba, beg, elem, it, type)		\
  Y_BLOCK_ARRAY_FOREACH_INT(ba, beg, elem, it,				\
			    type, type, yBlockArrayGetDirect)

#define Y_BLOCK_ARRAY_FOREACH_PTR_SINCE(ba, beg, elem, it, type)	\
  Y_BLOCK_ARRAY_FOREACH_INT(ba, beg, elem, it, type,			\
			    type *, yBlockArrayGetPtrDirect)

#define Y_BLOCK_ARRAY_FOREACH_PTR(ba, elem, it, type)		\
  Y_BLOCK_ARRAY_FOREACH_PTR_SINCE(ba, 0, elem, it, type)

#define Y_BLOCK_ARRAY_FOREACH(ba, elem, it, type)	\
  Y_BLOCK_ARRAY_FOREACH_SINCE(ba, 0, elem, it, type)

#define Y_BLOCK_ARRAY_SIZED_FOREACH_PTR(ba, elem, it, ptr_type, elem_type) \
	Y_BLOCK_ARRAY_FOREACH_INT(ba, 0, elem, it, elem_type, ptr_type *, \
				  yBlockArrayGetSuperDirect)

static inline int yBlockArrayIteratorIsEnd(BlockArrayIterator *it)
{
  return (!it->array ||
	  (!it->mask && !yBlockArrayIsBlockAllocated(*it->array,
						     it->blockPos)));
}

void yBlockArrayIteratorIncr(BlockArrayIterator *it);

#define yBlockArrayIteratorInit(it, arrayPtr, beg)	do {		\
    (it).blockPos = yBlockArrayBlockPos((beg));				\
    (it).array = (arrayPtr);						\
    (it).mask = yBlockArrayGetBlock(*(it).array, (it).blockPos);		\
    yBlockArrayIteratorIncr(&(it));					\
  } while (0)

BlockArrayIterator yBlockArrayIteratorCreate(BlockArray *array,
					     int beg);

#define yBlockArrayIteratorIdx(it) ((it).blockPos * 64 + (it).pos)

#define yBlockArrayIteratorGetPtr(it, type)				\
  ((type *)yBlockArrayGetInternal((it.array), (it.blockPos * 64 + it.pos)))

#define yBlockArrayIteratorGet(it, type)				\
  (*((type *)yBlockArrayGetInternal((it.array), (it.blockPos * 64 + it.pos))))


void yBlockArrayFree(BlockArray *ba);

/* Get the pos of the last used bit */

#endif
