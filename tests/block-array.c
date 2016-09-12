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
#include <sys/types.h>
#include <stdio.h>

#include "tests.h"
#include "block-array.h"

struct block_test {
  BlockArray array;
};

void testBlockArray(void)
{
  struct block_test test;
  uint64_t tmp = 1337;
  BlockArrayIterator iterator;

  yBlockArrayInit(&test.array, uint64_t);

  iterator = yBlockArrayIteratorCreate(&test.array, 0);
  g_assert(yBlockArrayIteratorIsEnd(&iterator));
  yBlockArrayIteratorIncr(&iterator);
  g_assert(yBlockArrayIteratorIsEnd(&iterator));
  Y_BLOCK_ARRAY_FOREACH(&test.array, useless1, useless2, uint64_t) {
    /* should not be here */
    g_assert(0);
  }
  g_assert(yBlockArrayBlockPos(2) == 0);
  g_assert(yBlockArrayBlockPos(64) == 1);
  g_assert(yBlockArrayBlockPos(128) == 2);

  g_assert(!yBlockArrayIsBlockAllocated(test.array, 0));
  g_assert(!yBlockArrayIsBlockAllocated(test.array, 1));
  yBlockArrayAssureBlock(&test.array, 0);
  g_assert(yBlockArrayIsBlockAllocated(test.array, 0));
  g_assert(yBlockArrayGet(&test.array, 0, uint64_t) == 0);

  yBlockArrayCopyElem(&test.array, 0, tmp);
  tmp = 23;
  g_assert(yBlockArrayGet(&test.array, 0, uint64_t) == 1337);
  yBlockArrayCopyElem(&test.array, 63, tmp);
  g_assert(yBlockArrayGet(&test.array, 63, uint64_t) == 23);
  tmp = (uint64_t)-1LL;
  yBlockArrayCopyElem(&test.array, 2, tmp);
  g_assert(yBlockArrayGet(&test.array, 1, uint64_t) == 0);
  g_assert(yBlockArrayGet(&test.array, 2, int64_t) == -1LL);
  g_assert(yBlockArrayGet(&test.array, 3, uint64_t) == 0);
  yBlockArrayFree(&test.array);

  for (int i = 0; i < 1000; ++i) {
    yBlockArrayInit(&test.array, int);
    g_assert(yBlockArrayLastPos(&test.array) == 0);
    yBlockArrayCopyElem(&test.array, i, i);
    g_assert(yBlockArrayLastPos(&test.array) == (size_t)i);
    g_assert(yBlockArrayGet(&test.array, i, int) == i);
    g_assert(yBlockArrayIsSet(test.array, i));
    g_assert(!yBlockArrayIsFree(test.array, i));
    yBlockArrayFree(&test.array);
  }

#define VAL_TEST (64 * 7 + 5)
  yBlockArrayInit(&test.array, uint64_t);
  yBlockArrayCopyElem(&test.array, VAL_TEST, tmp);
  for (int i = 0; i < VAL_TEST + (64 - 7); ++i) {
    if (i == VAL_TEST) {
      g_assert(yBlockArrayGet(&test.array, i, uint64_t) == tmp);
      g_assert(!yBlockArrayIsFree(test.array, i));
    } else {
      g_assert(!yBlockArrayGet(&test.array, i, uint64_t));
      g_assert(yBlockArrayIsFree(test.array, i));
    }
  }

  yBlockArrayIteratorInit(iterator, &test.array, 0);
  g_assert(!yBlockArrayIteratorIsEnd(&iterator));
  g_assert(yBlockArrayIteratorGet(iterator, uint64_t) == tmp);
  g_assert(*yBlockArrayIteratorGetPtr(iterator, uint64_t) == tmp);
  yBlockArrayIteratorIncr(&iterator);
  g_assert(yBlockArrayIteratorIsEnd(&iterator));

  yBlockArrayFree(&test.array);
    Y_BLOCK_ARRAY_FOREACH(&test.array, useless3, useless2, uint64_t) {
    /* should not be here */
    g_assert(0);
  }

#undef VAL_TEST

  yBlockArrayInit(&test.array, uint64_t);
  yBlockArrayCopyElem(&test.array, 64 * 4 , tmp);
  yBlockArrayCopyElem(&test.array, 64 * 4 - 1 , tmp);

  g_assert(!yBlockArrayGetBlock(test.array, 0));
  g_assert(!yBlockArrayGetBlock(test.array, 1));
  g_assert(!yBlockArrayGetBlock(test.array, 2));
  g_assert(yBlockArrayGetBlock(test.array, 3));
  g_assert(yBlockArrayGetBlock(test.array, 4));

  g_assert(yBlockArrayIsBlockAllocated(test.array, 0));
  g_assert(yBlockArrayIsBlockAllocated(test.array, 1));
  g_assert(yBlockArrayIsBlockAllocated(test.array, 2));
  g_assert(yBlockArrayIsBlockAllocated(test.array, 3));
  g_assert(yBlockArrayIsBlockAllocated(test.array, 4));
  g_assert(!yBlockArrayIsBlockAllocated(test.array, 5));

  int nbIteration = 0;
  Y_BLOCK_ARRAY_FOREACH(&test.array, elem, it, uint64_t) {
    g_assert(elem == tmp);
    g_assert((it == 64 * 4 - 1) || (it == 64 * 4));
    ++nbIteration;
  }
  g_assert(nbIteration == 2);


  yBlockArrayIteratorInit(iterator, &test.array, 0);
  g_assert(!yBlockArrayIteratorIsEnd(&iterator));
  g_assert(yBlockArrayIteratorGet(iterator, uint64_t) == tmp);
  g_assert(*yBlockArrayIteratorGetPtr(iterator, uint64_t) == tmp);
  g_assert(iterator.blockPos == 3);
  g_assert(iterator.pos == 63);
  yBlockArrayIteratorIncr(&iterator);

  g_assert(!yBlockArrayIteratorIsEnd(&iterator));
  g_assert(yBlockArrayIteratorGet(iterator, uint64_t) == tmp);
  g_assert(*yBlockArrayIteratorGetPtr(iterator, uint64_t) == tmp);
  g_assert(iterator.blockPos == 4);
  g_assert(iterator.pos == 0);
  yBlockArrayIteratorIncr(&iterator);
  g_assert(yBlockArrayIteratorIsEnd(&iterator));

  nbIteration = 0;
  Y_BLOCK_ARRAY_FOREACH_SINCE(&test.array, 64 * 4, elem0, it, uint64_t) {
    g_assert(elem0 == tmp);
    g_assert((it == 64 * 4));
    ++nbIteration;    
  }
  g_assert(nbIteration == 1);

  yBlockArrayUnset(&test.array, 64 * 4);

  g_assert(!yBlockArrayGetBlock(test.array, 0));
  g_assert(!yBlockArrayGetBlock(test.array, 1));
  g_assert(!yBlockArrayGetBlock(test.array, 2));
  g_assert(yBlockArrayGetBlock(test.array, 3));

  g_assert(yBlockArrayIsBlockAllocated(test.array, 0));
  g_assert(yBlockArrayIsBlockAllocated(test.array, 1));
  g_assert(yBlockArrayIsBlockAllocated(test.array, 2));
  g_assert(yBlockArrayIsBlockAllocated(test.array, 3));
  g_assert(!yBlockArrayIsBlockAllocated(test.array, 4));
  g_assert(!yBlockArrayIsBlockAllocated(test.array, 5));

  nbIteration = 0;
  Y_BLOCK_ARRAY_FOREACH(&test.array, elem2, it, uint64_t) {
    g_assert(elem2 == tmp);
    g_assert(it == 64 * 4 - 1);
    ++nbIteration;
  }
  g_assert(nbIteration == 1);
  
  yBlockArrayUnset(&test.array, 64 * 4 - 1);

  g_assert(!yBlockArrayIsBlockAllocated(test.array, 4));
  g_assert(!yBlockArrayIsBlockAllocated(test.array, 3));
  g_assert(!yBlockArrayIsBlockAllocated(test.array, 2));
  g_assert(!yBlockArrayIsBlockAllocated(test.array, 1));
  g_assert(!yBlockArrayIsBlockAllocated(test.array, 0));
  Y_BLOCK_ARRAY_FOREACH(&test.array, useless4, useless2, uint64_t) {
    /* should not be here */
    g_assert(0);
  }

  yBlockArrayInit(&test.array, uint64_t);
  for (int i = 0; i < 134; ++i) {
    yBlockArrayCopyElem(&test.array, i , tmp);
  }

  nbIteration = 0;
  Y_BLOCK_ARRAY_FOREACH(&test.array, elem10, it, uint64_t) {
    g_assert(elem10 == tmp);
    g_assert(it < 134);
    ++nbIteration;
  }
  g_assert(nbIteration == 134);
  nbIteration = 0;
  Y_BLOCK_ARRAY_FOREACH_SINCE(&test.array, 43, elem11, it, uint64_t) {
    g_assert(elem11 == tmp);
    g_assert(it > 42 && it < 134);
    ++nbIteration;
  }
  g_assert(nbIteration == 134 - 43);
  yBlockArrayFree(&test.array);
}
