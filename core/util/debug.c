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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include "debug.h"
/*
Change open and all the dprintf with
fopen et fprintf because dprintf don't existe on mingw
and fprintf need FILE handler.
*/

static int isInit;

typedef struct s_log_mode {
	const char * const str;
	FILE*		file;
}		t_log_mode;

static t_log_mode log_confs[] = {
	{ INFO_STR, NULL },
	{ WARNING_STR, NULL },
	{ ERROR_STR, NULL },
};

void	debug_print_(char const* mode, char const* format, va_list vl);
void	debug_print_info(FILE* fd, const char* mode);

static FILE*	get_file(int mode)
{
	return stderr;
}

#if defined(__unix__) || defined(__APPLE__)
void	yuiDebugPrint(int mode, char const* format, ...)
{
	if (mode >= 0 && mode < 3)
	{
		va_list	vl;

		va_start(vl, format);
#ifndef NO_PRINT_ON_CERR
		if (mode > 0)
		{
			va_list	vl2;

			va_copy(vl2, vl);
			y_vprintf(2, format, vl2);
			va_end(vl2);
		}
#endif
		debug_print_(log_confs[mode].str, format, vl);
		va_end(vl);
	}
}
#endif

void	yuiDebugInit(void)
{
	if (isInit)
		return;
	isInit = 1;
	log_confs[INFO].file = get_file(0);
	log_confs[WARNING].file = get_file(0);
	log_confs[D_ERROR].file = get_file(0);
}

void	yuiDebugExit()
{
	debug_print_info(log_confs[INFO].file, INFO_STR);
	isInit = 0;
}

void	debug_print_info(FILE* fd, const char* mode)
{
	fprintf(fd, "[%.7s]", mode);
}

void	debug_print_(char const* mode, char const* format, va_list vl)
{
	debug_print_info(log_confs[INFO].file, mode);
	if (format == NULL)
		fprintf(log_confs[INFO].file, "Unknow Error");
	else
		vfprintf(log_confs[INFO].file, format, vl);
	fflush(log_confs[INFO].file);
}

