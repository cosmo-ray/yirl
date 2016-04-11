/*
**Copyright (C) 2016 Matthias Gatto
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
#include <fcntl.h>

#include <stdio.h>
#include <entity.h>
#include <widget.h>
#include <game.h>

#define SM_BUFF_LEN 1024

void *load_map(int nb, void **args)
{
  int fd = open(args[0], O_RDONLY);
  char buff[SM_BUFF_LEN];
  int width = 0;
  Entity *mod_description = nb > 1 ? args[1] : NULL;
  Entity *father = nb > 2 ? args[1] : yeCreateArray(NULL, NULL);
  char *name = name = nb > 3 ? args[2] : NULL;
  Entity *ret = yeCreateArray(father, name);
  int check;

  printf("m-d: %p\n", mod_description);
 again:
  check = read(fd, buff, SM_BUFF_LEN);

  for (int i = 0; i < check; ++i) {
    if (buff[i] == '\n') {
      width = width ? width : i;
    } else {
      yeCreateInt(buff[i], yeCreateArray(ret, NULL), NULL);
    }
  }

  if (check > 0)
    goto again;
  yeReCreateInt(width, father, "width");
  return father;
}

void *init_sm_reader(int nbArg, void **args)
{
  Entity *t = YE_TO_ENTITY(args[0]);
  Entity *f = yeCreateFunction("load_map", 1, ygGetManager("tcc"), t, "load-map");

  return NULL;
}

