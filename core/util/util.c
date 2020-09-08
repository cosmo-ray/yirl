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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <setjmp.h>
#include "utils.h"

#if defined(_WIN32)
#include <windows.h>
#endif

const char *yLineRectIntersectStr[] = {
	"Y_NO_INTERSECT",
	"Y_LR_UP_INTERSECT",
	"Y_LR_LEFT_INTERSECT",
	"Y_LR_DOWN_INTERSECT",
	"Y_LR_RIGHT_INTERSECT"
};

static jmp_buf exit_bufs[64];
uint16_t jmp_buf_idx;

void yuiLongExit(int status)
{
	longjmp(exit_bufs[jmp_buf_idx -1], status);
}

int yuiTryMain(int (*main_f)(int, char **), int argc, char **argv)
{
	int ret;

	if (unlikely(!main_f || jmp_buf_idx > 63))
		return 1;
	ret = setjmp(exit_bufs[jmp_buf_idx]);
	if (ret) {
		--jmp_buf_idx;
		return ret - 1;
	}
	++jmp_buf_idx;
	return main_f(argc, argv);
}

int yuiFileExist(const char *path)
{
	return access(path, F_OK);
}

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

void yuiMkdir(const char *dir)
{
	(void)dir;
#if defined(_WIN32)
	mkdir(dir);
#else
	mkdir(dir, 0755);
#endif
}

void yuiPrintErrno(const char *s)
{
	if (s)
		DPRINT_ERR("%s%s\n", s, strerror(errno));
	else
		DPRINT_ERR("error: %s\n", strerror(errno));
}
