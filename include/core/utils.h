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

#ifndef MAX_NB_MANAGER
#define MAX_NB_MANAGER 64
#endif

#include <stdint.h>

typedef struct {
  void *(*allocator[MAX_NB_MANAGER])(void);
  int len;
} YManagerAllocator;

#include	"debug.h"


/**
 * registre a new type in scriptsTab
 */
int yuiRegister(YManagerAllocator *ma, void *(*allocator)(void));

int yuiUnregiste(YManagerAllocator *ma, int t);

#define YUI_GET_FiRST_BYTE(mask) __builtin_ctzl(mask)

#define YUI_COUNT_1_BYTE(mask) __builtin_popcountl(mask)

#define YUI_FOREACH_BITMASK(mask, it, tmpmask)				\
  for (uint64_t tmpmask = mask, it; ((it = __builtin_ctzl(tmpmask)) || 1) && \
	 tmpmask;							\
       tmpmask &= ~(1LLU << it))

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
