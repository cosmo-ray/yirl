/*
**Copyright (C) 2013 Matthias Gatto
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
#ifndef	MACRO_H
#define MACRO_H

#include	"debug.h"

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
