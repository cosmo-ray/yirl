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

#ifndef _YIRL_UTILS_H_
#define _YIRL_UTILS_H_


#ifndef MAX_NB_MANAGER
#define MAX_NB_MANAGER 64
#endif

#ifdef Y_INSIDE_TCC
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
#else
#include <math.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#include <stdio.h>

static inline char *get_current_dir_name(void)
{
  char tmp[PATH_MAX];

  GetCurrentDirectory(PATH_MAX, tmp);
  return strdup(tmp);
}

#endif

#endif

#ifndef __INT64_C
#define __INT64_C(c) (c ## LL)
#define __UINT64_C(c) (c ## LLU)
#endif

#define ONE64      1LLU

#if   __SIZEOF_POINTER__ == 4
typedef int32_t int_ptr_t;
#define PRIiptr	"%d"
#ifdef _WIN32
#define PRIint64 "%lli"
#else
#define PRIint64 "%" PRIi64
#endif

#elif __SIZEOF_POINTER__ == 8
/* don't know why PRIi64 doesn't seems to work on MinGw 32 */
typedef int64_t int_ptr_t;
#ifdef _WIN32
#define PRIint64 "%lli"
#define PRIiptr	"%I64d"
#else
#define PRIiptr	"%li"
#define PRIint64 "%" PRIi64
#endif

#else
typedef int64_t int_ptr_t;
#endif

#ifndef Y_INSIDE_TCC
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#else
#include "basic.h"
#endif

// if compiller gcc
#if defined(__GNUC__) && (__GNUC__ >= 4)

# ifndef likely
# define likely(x)      __builtin_expect(!!(x), 1)
# define unlikely(x)    __builtin_expect(!!(x), 0)
# endif

# define ctz64 	   __builtin_ctzll
# define clz64      __builtin_clzll
# define popcount64 __builtin_popcountll

#else

/* TODO: test this */
# define ctz64 yuiCtz64
# define clz64 yuiClz64
# define popcount64 yuiPopcount64

# ifndef likely
# define likely(x) (x)
# define unlikely(x) (x)
# endif

#endif

/* Code from http://jhnet.co.uk/articles/cpp_magic */

#define YUI_GET_ARG_COUNT(...) _GET_ARG_COUNT(0, ## __VA_ARGS__, 70, 69, 68, \
					      67, 66, 65, 64, 63, 62, 61, 60, \
					      59, 58, 57, 56, 55, 54, 53, 52, \
					      51, 50, 49, 48, 47, 46, 45, 44, \
					      43, 42, 41, 40, 39, 38, 37, 36, \
					      35, 34, 33, 32, 31, 30, 29, 28, \
					      27, 26, 25, 24, 23, 22, 21, 20, \
					      19, 18, 17, 16, 15, 14, 13, 12, \
					      11, 10, 9, 8, 7, 6, 5, 4, 3, 2, \
					      1, 0)

#define _GET_ARG_COUNT(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, \
		       _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, \
		       _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, \
		       _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, \
		       _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, \
		       _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, \
		       _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, \
		       _68, _69, _70, count, ...) count

#define YUI_SECOND(a, b, ...) b

#define YUI_IS_PROBE(...) YUI_SECOND(__VA_ARGS__, 0)
#define YUI_PROBE() ~, 1

#define NOP(x) x

#define YUI_CAT(a,b) a ## b

#define YUI_CATCAT(a,b,c) a ## b ## c

#define YUI_NOT(x) YUI_IS_PROBE(YUI_CAT(_NOT_, x))
#define _NOT_0 YUI_PROBE()

#define YUI_BOOL(x) YUI_NOT(YUI_NOT(x))

#define _IF_ELSE(condition) YUI_CAT(_IF_, condition)
#define YUI_IF_ELSE(condition) _IF_ELSE(YUI_BOOL(condition))

#define _IF_1(...) __VA_ARGS__ _IF_1_ELSE
#define _IF_0(...)             _IF_0_ELSE

#define _IF_1_ELSE(...)
#define _IF_0_ELSE(...) __VA_ARGS__

/*
 * yuiPopcount64, yuiClz64, and yuiCtz64 come from
 * qCountTrailingZeroBits, qPopulationCount and
 * qCountLeadingZeroBits from Qt(QtCore/qalgorithms.h)
 * http://www.qt.io
 */

static inline unsigned int yuiPopcount64(uint64_t v)
{
  /*
   * See http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
   */
  return
    (((v) & 0xfff) * __UINT64_C(0x1001001001001) &
     __UINT64_C(0x84210842108421)) % 0x1f +
    (((v >> 12) & 0xfff) * __UINT64_C(0x1001001001001) &
     __UINT64_C(0x84210842108421)) % 0x1f +
    (((v >> 24) & 0xfff) * __UINT64_C(0x1001001001001) &
     __UINT64_C(0x84210842108421)) % 0x1f +
    (((v >> 36) & 0xfff) * __UINT64_C(0x1001001001001) &
     __UINT64_C(0x84210842108421)) % 0x1f +
    (((v >> 48) & 0xfff) * __UINT64_C(0x1001001001001) &
     __UINT64_C(0x84210842108421)) % 0x1f +
    (((v >> 60) & 0xfff) * __UINT64_C(0x1001001001001) &
     __UINT64_C(0x84210842108421)) % 0x1f;
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


/* helper for generic, because there is too much integer types in C */
/* inly _Bool is missing, but I'm not sure wecan consider it a number */
#define Y_GENERIC_INT(action)			\
	signed char : action,			\
		unsigned char : action,		\
		short : action,			\
		unsigned short : action,	\
		int : action,			\
		unsigned int : action,		\
		long : action,			\
		long long : action,		\
		unsigned long : action,		\
		unsigned long long : action

/* helper for generic, because there is too much number types in C */
#define Y_GENERIC_NUMBER(action)		\
	Y_GENERIC_INT(action),			\
		double : action,		\
		float : action

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
    type [18]: func,				\
    type [1024]: func


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


#define YUI_MASK_FULL 0xffffffffffffffff

#define YUI_FIRST_ZERO(m)(m == 0 ? 0 : ctz64(~m))

/*TODO: change this name to YUI_GET_FIRST_MASK_POS*/
#define YUI_GET_FIRST_BIT(mask)(mask == 0 ? 0 : ctz64(mask))

#define YUI_GET_LAST_MASK_POS(mask, fail_ret) (mask == 0 ? fail_ret : 63LL - clz64(mask))

#define YUI_COUNT_1_BIT(mask) (mask == 0LU ? 0LU : popcount64(mask))

#define YUI_FOREACH_BITMASK(mask, it, tmpmask)				\
  for (uint64_t tmpmask = mask, it; tmpmask &&				\
	 ((it = YUI_GET_FIRST_BIT(tmpmask)) || 1);			\
       tmpmask &= ~(ONE64 << it))

/**
 * @brief return "" if str is NULL
 */
#define yuiMaybeEmpty(str)			\
  (str ? str : "")

/* string.h sandboxing for tcc */
char *yuistrcpy(char *dest, const char *src);
char *yuistrncpy(char *dest, const char *src, size_t n);
size_t yuistrlen(const char *s);

static inline char *yuiStrdup(const char *s)
{
	if (!s)
		return NULL;
	return strdup(s);
}

/* This is usefull for macro and constant, otherwise use abs(so if a variable) */
#define yuiAbs(val) (val > 0 ? val : - val)

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
    return str1 == str2;
  }
  return yuiStrEqual(str1, str2);
}


static int yuiIsCharAlphaNum(char c)
{
	return (c >= 'a' &&  c <= 'z') || (c >= 'A' && c <= 'Z') ||
		(c >= '0' && c <= '9');
}

int  yuiRand(void);
void yuiRandInit(void);
void yuiRandInitSeed(int s);

void yuiMkdir(const char *dir);

int yuiFileExist(const char *path);

static inline int yuiStrCountCh(const char *str, char c, int *longerLine)
{
  int ret = 0;
  int i = 0;

  if (unlikely(!str))
    return 0;
  if (longerLine)
    *longerLine = 0;
  for (; *str; ++str) {
    if (unlikely(*str == c)) {
      if (longerLine && i > *longerLine)
	*longerLine = i;
      i = 0;
      ++ret;
    } else {
      ++i;
    }
  }
  if (longerLine && i > *longerLine)
    *longerLine = i;
  return ret;
}

/**
 * @return percent of @value
 */
static inline int yuiPercentOf(int value, int percent)
{
  return value * percent / 100;
}

#define ypow1(X) X
#define ypow2(X) X * ypow1(X)
#define ypow3(X) X * ypow2(X)
#define ypow4(X) X * ypow3(X)
#define ypow5(X) X * ypow4(X)
#define ypow6(X) X * ypow5(X)
#define ypow7(X) X * ypow6(X)
#define ypow8(X) X * ypow7(X)
#define ypow9(X) X * ypow8(X)
#define ypow10(X) X * ypow9(X)

#define YUI_SWAP_VALUE(a, b) ((a) += (b), (b) = ((a) - (b)), (a) -= (b))

#define YUI_SWAP_PTR(a, b, ptr_type) ({				\
      int_ptr_t tmp_a = ((int_ptr_t)a), tmp_b = ((int_ptr_t)b);	\
      YUI_SWAP_VALUE(tmp_a, tmp_b);				\
      a = (ptr_type)tmp_a; b = (ptr_type)tmp_b;			\
    })

#define yuiUsleep usleep

int yuiLinesIntersect(int x1, int y1, int x2, int y2,
		      int x3, int y3, int x4, int y4,
		      int *x, int *y);

enum yLineRectIntersect {
	Y_NO_INTERSECT = 0,
	Y_LR_UP_INTERSECT = 1,
	Y_LR_LEFT_INTERSECT = 2,
	Y_LR_DOWN_INTERSECT = 3,
	Y_LR_RIGHT_INTERSECT = 4,
};

extern const char *yLineRectIntersectStr[];

/* return Dist or 0 if no intersect */
int yuiLinesRectIntersect(int x1, int y1, int x2, int y2,
			  int rx, int ry, int rw, int rh,
			  int *x, int *y, int *type);

static inline int yuiPointsDist(int x1, int y1, int x2, int y2)
{
	int x = x2 - x1;
	int y = y2 - y1;

	return sqrt(x * x + y * y);
}

static inline int yuiTurnX(int x, int y, double r)
{
	return x * cos(r) - y * sin(r);
}

static inline int yuiTurnY(int x, int y, double r)
{
	return y * cos(r) - x * sin(r);
}

void yuiPrintErrno(const char *);

#endif
