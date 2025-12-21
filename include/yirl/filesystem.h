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

static _Bool yFillPathList(const char dir[static 1], Entity *out_vector)
{
	return yFillPathListFilter(dir, out_vector, YFILE_ALL);
}

static _Bool yFillDirectoryPathList(const char dir[static 1], Entity *out_vector)
{
	return yFillPathListFilter(dir, out_vector, YFILE_DIR);
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

#endif
