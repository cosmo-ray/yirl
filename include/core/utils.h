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

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#ifndef MAX_NB_MANAGER
#define MAX_NB_MANAGER 64
#endif

#if   __SIZEOF_POINTER__ == 4
#define ONE64      1LLU
#define ctz64 	   __builtin_ctzll
#define clz64      __builtin_clzll
#define popcount64 __builtin_popcountll
#elif __SIZEOF_POINTER__ == 8
#define ONE64      1LU
#define ctz64      __builtin_ctzl
#define clz64      __builtin_clzl
#define popcount64 __builtin_popcountl
#endif

/* Define to use for error handeling */
#define MAYBE(var) var

/* example usage: */
/* MAYBE(void *) test; */
/* MAYDO((test = myFunc()), pocessMyPtr()) */
#define MAYDO(var, cmd)				\
  if (var) {					\
    cmd;					\
  }

typedef struct {
  void *(*allocator[MAX_NB_MANAGER])(void);
  int len;
} YManagerAllocator;

#include	"debug.h"

/* These functions are helpers to manage "drivers" */

/**
 * registre a new Type to a Manager.
 * @ma: The manager
 * @allocator: The function which will be use to allocate a new type.
 */
int yuiRegister(YManagerAllocator *ma, void *(*allocator)(void));

int yuiUnregiste(YManagerAllocator *ma, int t);


/*TODO: change this name to YUI_GET_FIRST_MASK_POS*/
#define YUI_GET_FIRST_BIT(mask) ctz64(mask)

#define YUI_GET_LAST_MASK_POS(mask) (mask == 0LU ? 0LU : 63LU - clz64(mask))

#define YUI_COUNT_1_BIT(mask) (mask == 0LU ? 0LU : popcount64(mask))

#define YUI_FOREACH_BITMASK(mask, it, tmpmask)				\
  for (uint64_t tmpmask = mask, it; ((it = YUI_GET_FIRST_BIT(tmpmask)) || 1) && \
	 tmpmask;							\
       tmpmask &= ~(ONE64 << it))

static inline int yuiStrEqual(const char *str1, const char *str2)
{
  int i;

  for (i = 0; str1[i]; ++i)
    if (str1[i] != str2[i])
      return 0;
  return (str1[i] == str2[i]);
}

static inline int yuiStrEqual0(const char *str1, const char *str2)
{
  if (!str1 || !str2) {
    if (str1 == str2)
      return 1;
    return 0;
  }
  return yuiStrEqual(str1, str2);
}


#define yuiRand()	rand()
#define yuiRandInit()	srand(time(NULL) + getpid() + getuid())

/**
 * example:
 * UNIMPLEMENTED_FUNCTION(false, bool	updateElems(std::list<IGameElm*> elems))
 */
#define	UNIMPLEMENTED_FUNCTION(ret, function...) \
  function				    \
  { \
    DPRINT_WARN(#function" not implemented\n");	\
    return (ret);\
  }

/**
 * this macro is use for function unimplemented on a system
 */
#define	UNIMPLEMENTED_FUNCTIONALITY_SYSTEM(ret, cmdName, system, function...) \
  function	\
  {\
  DPRINT_WARN(#cmdName" not implemented on %s\n", system);	\
  return (ret);\
  }


/**
 * this macro is use for function returning a void unimplemented on a system
 */
#define	V_UNIMPLEMENTED_FUNCTIONALITY_SYSTEM(cmdName, system, function...) \
  function	\
  {\
  DPRINT_WARN(#cmdName" not implemented on %s\n", system);	\
  }


#endif
