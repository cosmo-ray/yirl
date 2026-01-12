/*
**Copyright (C) 2025 Matthias Gatto
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

#ifndef _YIRL_FILESYSTEM_H_
#define _YIRL_FILESYSTEM_H_

#include "yirl/utils.h"
#include "yirl/entity.h"

#ifndef Y_INSIDE_TCC

enum {
	YPATH_PATH_IDX = 0,
	YPATH_TYPE_IDX = 1
};

#ifndef _WIN32
/* need windows support here */
#include <dirent.h>

static int unix_file_to_y(int ft)
{
	switch (ft) {
	case DT_BLK:
		return YFILE_BLK;
	case DT_CHR:
		return YFILE_CHR;
	case DT_DIR:
		return YFILE_DIR;
	case DT_FIFO:
		return YFILE_FIFO;
	case DT_LNK:
		return YFILE_LNK;
	case DT_REG:
		return YFILE_REG;
	case DT_SOCK:
		return YFILE_SOCK;
	default: /* include DT_UNKNOWN and anything else */
		return YFILE_UNKNOWN;
	}
}

static _Bool yFillPathListFilter(const char dir[static 1], Entity *out_vector, int filter)
{
	if (!out_vector)
		return 0;
	DIR *d = opendir(dir);
	if (!d)
		return 0;
	struct dirent *de;
	while ((de = readdir(d)) != NULL) {
		int t = unix_file_to_y(de->d_type);

		if (!(t & filter))
			continue;
		/* let's skipp '.' and '..' */
		if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
			continue;
		Entity *v = yeCreateVector(out_vector, NULL);
		yeCreateString(de->d_name, v, NULL);
		yeCreateInt(t, v, NULL);
	}
	closedir(d);
	return 1;
}
#else

#include <fileapi.h>

static _Bool yFillPathListFilter(const char dir[static 1], Entity *out_vector, int filter)
{
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
	yeAutoFree Entity *path_to_use = yeCreateString(dir, NULL, NULL);

	if (!out_vector)
		return 0;
	yeAdd(path_to_use, "\\*");
	if((hFind = FindFirstFile(yeGetString(path_to_use), &FindFileData)) != INVALID_HANDLE_VALUE){
		do {
			int t = strchr(FindFileData.cFileName, '.') ? YFILE_REG : YFILE_DIR;

			if (!(t & filter))
				continue;
			if (!strcmp(FindFileData.cFileName, ".") || !strcmp(FindFileData.cFileName, ".."))
				continue;
			Entity *v = yeCreateVector(out_vector, NULL);

			yeCreateString(FindFileData.cFileName, v, NULL);
			yeCreateInt(t, v, NULL);
		} while(FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
	return 1;
}

#endif

static _Bool yFillPathList(const char dir[static 1], Entity *out_vector)
{
	return yFillPathListFilter(dir, out_vector, YFILE_ALL);
}

static _Bool yFillDirectoryPathList(const char dir[static 1], Entity *out_vector)
{
	return yFillPathListFilter(dir, out_vector, YFILE_DIR);
}

/**
 * return true, if path_to_use is a directory
 * path_info, been one of the entity filled by yFillPathList
 */
static _Bool yPathInfoIsDir(Entity path_info[static 1])
{
	return yeGetIntAt(path_info, YPATH_TYPE_IDX) == YFILE_DIR;
}

static int yPathAdd(Entity path[static 1], const char to_append[static 1])
{
	if (yeTypeNoCheck(path) != YSTRING)
		return -1;
	if (yeStrLastCh(path) != PATH_SEPARATOR)
		yeStringAddCh(path, PATH_SEPARATOR);
	yeStringAdd(path, to_append);
	return 0;
}

/**
 * say you give /tmp/b, will return /tmp
 * give /tmp/b/ will return /tmp
 */
static _Bool yPathUp(Entity path[static 1])
{
	/* nothing up to '/', and I basically don't know how to handle 1 char path */
	if (yeLen(path) < 2)
		return 0;
	if (yeStrLastCh(path) == PATH_SEPARATOR)
		yeStringTruncate(path, 1);
	while (yeLen(path) && yeStrLastCh(path) != PATH_SEPARATOR)
		yeStringTruncate(path, 1);
	if (yeLen(path) < 2)
		return 1;
	/* remove last path separator */
	yeStringTruncate(path, 1);
	return 1;
}

#endif /* not in tcc */

#endif
