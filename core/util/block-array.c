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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utils.h"
#include "block-array.h"

#ifdef USING_EMCC
#define ALLOC_SIZE 0x3ffffff
#else
#define ALLOC_SIZE 0xfffffff
#endif
size_t yBlockArrayDataNextSize0;

inline void yBlockArrayInitInternal(BlockArray *ba, uint32_t elemSize, int flag)
{
	assert(elemSize < YBA_MAX_ELEM_SIZE);
	ba->elem_size = elemSize;
	if (flag & YBLOCK_ARRAY_BIG_CHUNK)
		ba->elems = malloc(ALLOC_SIZE);
	else
		ba->elems = NULL;
	ba->blocks = NULL;
	ba->flag = flag;
	ba->block_cnt = 0;
}

inline void yBlockArrayFree(BlockArray *ba)
{
	yBlockArrayFreeBlocks(ba);
	free(ba->elems);
}

#define BLOCK_REAL_SIZE(ba)			\
	((ba)->elem_size * 64)

void yBlockArrayExpandBlocks(BlockArray *ba, int nb)
{
	uint64_t old_cnt = ba->block_cnt;

	if (ba->flag & YBLOCK_ARRAY_BIG_CHUNK) {
		nb = nb + abs(nb) % 16;
	}

	if (ba->flag & YBLOCK_ARRAY_NO_BLOCKS_NEXT0 &&
	    ba->block_cnt * sizeof(uint64_t) < yBlockArrayDataNextSize0) {
		int16_t nNb = ba->block_cnt + nb;
		char *src = (void *)ba;

		src += sizeof(BlockArray);
		if (nNb * sizeof(uint64_t) >= yBlockArrayDataNextSize0) {
			ba->blocks = malloc(nNb * sizeof(uint64_t));
			/* So we can free memory */
			ba->flag ^= YBLOCK_ARRAY_NO_BLOCKS_NEXT0;
			memcpy(ba->blocks, src, ba->block_cnt * sizeof(uint64_t));
		} else {
			ba->blocks = (void *)src;
		}
		ba->block_cnt = nNb;
	} else {
		ba->block_cnt += nb;
		ba->blocks = realloc(ba->blocks,
				     ba->block_cnt * sizeof(uint64_t));
	}

	if (!(ba->flag & YBLOCK_ARRAY_BIG_CHUNK)) {
		ba->elems = realloc(ba->elems,
				    ba->block_cnt * BLOCK_REAL_SIZE(ba));
	}

	if (nb > 0) {
		if (!(ba->flag & YBLOCK_ARRAY_NOINIT))
			memset(ba->elems + (old_cnt  * BLOCK_REAL_SIZE(ba)), 0,
			       nb * BLOCK_REAL_SIZE(ba));
		memset(ba->blocks + old_cnt, 0, nb * sizeof(uint64_t));
	}
}

#undef BLOCK_REAL_SIZE


inline void yBlockArrayCopyElemInternal(BlockArray *ba, size_t pos,
					const void *elem)
{
	yBlockArraySet(ba, pos);
	memcpy(ba->elems + (pos * ba->elem_size), elem, ba->elem_size);
	return;
}


inline void yBlockArrayIteratorIncr(BlockArrayIterator *it)
{
	if (!it->mask) {
		uint64_t j = 1;
		for (int i = it->blockPos + 1; i < it->array->block_cnt &&
			     !yBlockArrayGetBlock(*it->array, i); ++i, ++j);
		it->blockPos += j;
		it->mask = yBlockArrayGetBlock(*it->array, it->blockPos);
		it->pos = 0;
		if (!it->mask)
			return;
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
