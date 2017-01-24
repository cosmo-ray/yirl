/*
**Copyright (C) 2017 Matthias Gatto
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

#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "rawfile-desc.h"
#include "entity.h"
#include "description.h"

static int t = -1;

static Entity *fromFile(void *opac, const char *fileName, Entity *father)
{
  Entity *ret = yeCreateString(NULL, father, NULL);
  struct stat st;
  int fd = 0;

  (void)opac;
  if (stat(fileName, &st) < 0 || (fd = open(fileName, O_RDONLY) ) < 0) {
    DPRINT_ERR("cannot open/stat '%s'", fileName);
    return NULL;
  }
  yeAddStrFromFd(ret, fd, st.st_size);
  return ret;
}

static int destroy(void *opac)
{
  g_free(opac);
  return 0;
}

static void *rawFileAllocator(void)
{
  YDescriptionOps *ret;

  ret = g_new(YDescriptionOps, 1);
  if (ret == NULL)
    return NULL;
  ret->name = "raw-file";
  ret->toFile = NULL;
  ret->fromFile = fromFile;
  ret->destroy = destroy;
  return ret;
}

int ydRawFileGetType(void)
{
  return t;
}

int ydRawFileInit(void)
{
  t = ydRegister(rawFileAllocator);
  return t;
}

int ydRawFileEnd(void)
{
  return ydUnregiste(t);
}
