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
#include <utils.h>
#include <entity.h>
#include <widget.h>
#include <game.h>

#define SM_BUFF_LEN 1024

void create_tild(Entity *mod_description, int tild, Entity *map)
{
  if (mod_description) {
    Y_BLOCK_ARRAY_FOREACH_PTR(&YE_TO_ARRAY(mod_description)->values, tmp,
			      it, ArrayEntry) {
      const char *char_str;

      if (!tmp)
	continue;

      char_str = yeGetString(yeGetByStr(tmp->entity, "map-char"));
      if (char_str[0] == tild) {
	yeCreateInt(it, yeCreateArray(map, NULL), NULL);
	return;
      }
    }
  }
  yeCreateInt(tild, yeCreateArray(map, NULL), NULL);
}

void *load_map(int nb, void **args)
{
  int fd = open(yeGetString(args[0]), O_RDONLY);
  char buff[SM_BUFF_LEN];
  int width = 0;
  Entity *mod_description = nb > 1 ? args[1] : NULL;
  Entity *father = nb > 2 ? args[2] : yeCreateArray(NULL, NULL);
  char *name = name = nb > 3 ? args[3] : NULL;
  Entity *map = yeCreateArray(father, name);
  int check;

  if (fd < 0) {
    DPRINT_ERR("error when opening '%s'\n", yeGetString(args[0]));
    goto error;
  }

 again:
  check = read(fd, buff, SM_BUFF_LEN);
  if (check < 0) {
    DPRINT_ERR("error when reading '%s'\n", args[0]);
    goto error;
  }

  for (int i = 0; i < check; ++i) {
    if (buff[i] == '\n') {
      width = width ? width : i;
    } else {
      create_tild(mod_description, buff[i], map);
    }
  }

  if (check > 0)
    goto again;

  yeReCreateInt(width, father, "width");
  return father;
 error:
  if (nb > 2)
    YE_DESTROY(father);
  YE_DESTROY(map);
  return NULL;
}

void *init_sm_reader(int nbArg, void **args)
{
  Entity *t = YE_TO_ENTITY(args[0]);
  Entity *f = yeCreateFunction("load_map", ygGetManager("tcc"), t, "load-map");

  return NULL;
}

