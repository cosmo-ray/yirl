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
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "utils.h"

int yuiRegister(YManagerAllocator *ma, void *(*allocator)(void))
{
  if (unlikely(!ma || ma->len >= MAX_NB_MANAGER - 1))
    return -1;
  ma->allocator[ma->len] = allocator;
  ma->len += 1;
  return ma->len - 1;
}

int yuiUnregiste(YManagerAllocator *ma, int t)
{
  if (unlikely(!ma || ma->len <= t || t < 0))
    return -1;
  ma->allocator[t] = NULL;
  if (t == ma->len - 1)
    ma->len -= 1;
  while (ma->len && ma->allocator[ma->len - 1] == NULL) {
    ma->len -= 1;
  }
  return 0;
}

size_t yuistrlen(const char *s)
{
  return strlen(s);
}

char *yuistrcpy(char *dest, const char *src)
{
  return strcpy(dest, src);
}

char *yuistrncpy(char *dest, const char *src, size_t n)
{
  return strncpy(dest, src, n);
}

void yuiRandInitSeed(int s)
{
  srand(s);
}

void yuiRandInit(void)
{
  yuiRandInitSeed(time(NULL) + getpid());
}

int  yuiRand(void)
{
  return rand();
}
