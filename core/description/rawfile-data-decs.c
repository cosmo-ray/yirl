/*
**Copyright (C) 2024 Matthias Gatto
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

#include <unistd.h>
#include <fcntl.h>
#include "rawfile-desc.h"
#include "entity.h"
#include "description.h"

static int t = -1;

static Entity *fromFile(void *opac, const char *file_name, Entity *father)
{
	Entity *ret = NULL;

	(void)opac;
	FILE *f = fopen(file_name, "rb");
	if (!f) {
		DPRINT_ERR("fail to open %s", file_name);
		return NULL;
	}

	if (fseek(f, 0, SEEK_END) < 0) {
		DPRINT_ERR("fseek fail for %s", file_name);
		goto out;
	}
	long fsize = ftell(f);
	if (fseek(f, 0, SEEK_SET) < 0) {
		DPRINT_ERR("fseek fail for %s", file_name);
		goto out;
	}

	char *buf = malloc(fsize);
	if (!buf) {
		DPRINT_ERR("malloc fail for %s", file_name);
		goto out;
	}
	fread(buf, fsize, 1, f);
	if (ferror(f)) {
		DPRINT_ERR("fread fail for %s", file_name);
		free(buf);
		goto out;
	}
	ret = yeCreateData(buf, father, NULL);
	YE_TO_DATA(ret)->len = fsize;
	yeSetFreeAdDestroy(ret);
out:
	fclose(f);
	return ret;
}

static int destroy(void *opac)
{
	free(opac);
	return 0;
}

static void *rawFileAllocator(void)
{
	YDescriptionOps *ret;

	ret = malloc(sizeof *ret);
	if (ret == NULL)
		return NULL;
	ret->name = "raw-file-data";
	ret->toFile = NULL;
	ret->fromFile = fromFile;
	ret->destroy = destroy;
	return ret;
}

int ydRawFileDataGetType(void)
{
	return t;
}

int ydRawFileDataInit(void)
{
	t = ydRegister(rawFileAllocator);
	return t;
}

int ydRawFileDataEnd(void)
{
	return ydUnregiste(t);
}
