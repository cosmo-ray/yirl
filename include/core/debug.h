/*
**Copyright (C) 2013 Gaetan Becker
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

#ifndef	DEBUG_H_
#define	DEBUG_H_

#include <stdio.h>
#include <stdarg.h>

/**
 * Content of DEFINE for debug function.
 * Compilling with DEBUG option for use it.
 */
# ifdef __cplusplus
extern "C"
{
# endif
  /**
   * @brief Initiate the logger
   */
  void yuiDebugInit(void);
  
  /**
   * @brief Add log to the file or print it to the specified file descriptor.
   */
  void	yuiDebugPrint(int mode, char const* format, ...);
  
  /**
   * @brief Exit the logger
   */
  void yuiDebugExit(void);
#ifdef __cplusplus
}
#endif
/*DIFFRENT defin selon les besoin*/
#define	LOG		DPRINT
#define	LOG_INFO       	DPRINT_INFO
#define	LOG_WARN    	DPRINT_WARN
#define	LOG_ERR		DPRINT_ERR
#define	DPRINT(format, args...)		DPRINT_INFO(format, ## args)
#define	DPRINT_INFO(format, args...)	_DPRINT(INFO, format, ## args)
#define	DPRINT_WARN(format, args...)	_F_DPRINT(WARNING, format, ## args)
#define	DPRINT_ERR(format, args...)	_F_DPRINT(D_ERROR, format, ## args)

#ifdef		DEBUG

#define	_F_DPRINT(lvl, format, args...)	_DPRINT(lvl, format, ## args)
#define	_DPRINT(lvl, format, args...)	do { yuiDebugPrint(lvl, "[%11s][%s : %d] " format "\n", __DATE__, __FILE__, __LINE__, ## args); } while (0)
#else
#define	_DPRINT(lvl, format, args...)	do { } while (0)
#define	_F_DPRINT(lvl, format, args...)	do { yuiDebugPrint(lvl, "[%11s][%s : %d] " format "\n", __DATE__, __FILE__, __LINE__, ## args); } while (0)
#endif  // DEBUG
#define	STR(var)	 #var
/**
 * Logging mode Macros
 * The macros have to be defined after other macros to be used by other macros.
 * @see http://gcc.gnu.org/onlinedocs/cpp/Object-like-Macros.html#Object-like-Macros
 */

#define	INFO_STR	STR(INFO)
#define	WARNING_STR	STR(WARNING)
#define	ERROR_STR	STR(ERROR)

#define	INFO	0
#define	WARNING	1
#define	D_ERROR	2

#ifdef	__unix__
#define	y_vprintf(fd, format, vl)	vdprintf(fd, format, vl)
#else
#include "Windows.h"
#define	y_vprintf(fd, format, vl)	
#endif
#endif	// DEBUG_H_
