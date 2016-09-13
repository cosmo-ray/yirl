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
#elif __SIZEOF_POINTER__ == 8
#define ONE64      1LU
#endif

// if compiller gcc
#if defined(__GNUC__) && (__GNUC__ >= 4)

# ifndef likely
# define likely(x)      __builtin_expect(!!(x), 1)
# define unlikely(x)    __builtin_expect(!!(x), 0)
# endif

# if   __SIZEOF_POINTER__ == 4
# define ctz64 	   __builtin_ctzll
# define clz64      __builtin_clzll
# define popcount64 __builtin_popcountll
# elif __SIZEOF_POINTER__ == 8
# define ctz64      __builtin_ctzl
# define clz64      __builtin_clzl
# define popcount64 __builtin_popcountl
# endif

#else
/* TODO: test this */
# define ctz64 yuiCtz64
# define clz64 yuiClz64
# define popcount64 yuiPopcount64

# ifndef likely
# define likely(x)
# define unlikely(x)
# endif

#endif


/*
 * yuiPopcount64, yuiClz64, and yuiCtz64 come from
 * qCountTrailingZeroBits, qPopulationCount and
 * qCountLeadingZeroBits from Qt(QtCore/qalgorithms.h)
 * http://www.qt.io
 */

static inline uint yuiPopcount64(uint64_t v)
{
  /*
   * See http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
   */
  return
    (((v) & 0xfff) * __UINT64_C(0x1001001001001) & __UINT64_C(0x84210842108421)) % 0x1f +
    (((v >> 12) & 0xfff) * __UINT64_C(0x1001001001001) & __UINT64_C(0x84210842108421)) % 0x1f +
    (((v >> 24) & 0xfff) * __UINT64_C(0x1001001001001) & __UINT64_C(0x84210842108421)) % 0x1f +
    (((v >> 36) & 0xfff) * __UINT64_C(0x1001001001001) & __UINT64_C(0x84210842108421)) % 0x1f +
    (((v >> 48) & 0xfff) * __UINT64_C(0x1001001001001) & __UINT64_C(0x84210842108421)) % 0x1f +
    (((v >> 60) & 0xfff) * __UINT64_C(0x1001001001001) & __UINT64_C(0x84210842108421)) % 0x1f;
}

static inline unsigned int yuiClz64(uint64_t v)
{
  v = v | (v >> 1);
  v = v | (v >> 2);
  v = v | (v >> 4);
  v = v | (v >> 8);
  v = v | (v >> 16);
  return yuiPopcount64(~v);
}

static inline unsigned int yuiCtz32(int32_t v)
{
  unsigned int c = 32; // c will be the number of zero bits on the right

  v &= -v;
  if (v) c--;
  if (v & 0x0000FFFF) c -= 16;
  if (v & 0x00FF00FF) c -= 8;
  if (v & 0x0F0F0F0F) c -= 4;
  if (v & 0x33333333) c -= 2;
  if (v & 0x55555555) c -= 1;
  return c;
}

static inline unsigned int yuiCtz64(uint64_t v)
{
  uint32_t x = (uint32_t)v;
  return x ? yuiCtz32(x)
    : 32 + yuiCtz32((uint32_t)(v >> 32));
}


/*
 * because clang is unable to guess that a 'char[1]' may be cast in 'char *'
 * when using generic
 */
#define Y_GEN_CLANG_ARRAY(type, func)		\
  type [1]: func,				\
    type [2]: func,				\
    type [3]: func,				\
    type [4]: func,				\
    type [5]: func,				\
    type [6]: func,				\
    type [7]: func,				\
    type [8]: func,				\
    type [9]: func,				\
    type [10]: func,				\
    type [11]: func,				\
    type [12]: func,				\
    type [13]: func,				\
    type [14]: func,				\
    type [15]: func,				\
    type [16]: func,				\
    type [17]: func,				\
    type [18]: func


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

#define YUI_GET_LAST_MASK_POS(mask, fail_ret) (mask == 0 ? fail_ret : __UINT64_C(63) - clz64(mask))

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
