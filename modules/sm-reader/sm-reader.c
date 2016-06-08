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

static int read_map(int fd, Entity *map, Entity *resources,
		    const char *file_name, int *width)
{
  int check;
  char buff[SM_BUFF_LEN];

 again:
  check = read(fd, buff, SM_BUFF_LEN);
  if (check < 0) {
    DPRINT_ERR("error when reading '%s'\n", file_name);
    return -1;
  }

  for (int i = 0; i < check; ++i) {
    if (buff[i] == '\n') {
      *width = *width ? *width : i;
    } else {
      create_tild(resources, buff[i], map);
    }
  }

  if (check > 0)
    goto again;
  
  return 0;
}

void *load_map(int nb, void **args)
{
  const char *file_name = args[0];
  int fd = open(yeGetString(file_name), O_RDONLY);
  int width = 0;
  Entity *mod_description = nb > 1 ? args[1] : NULL;
  char *name = nb > 3 ? args[3] : NULL;
  Entity *father = nb > 2 ? args[2] : yeCreateArray(NULL, name);
  Entity *map = yeCreateArray(father, "map");

  if (fd < 0) {
    DPRINT_ERR("error when opening '%s'\n", yeGetString(file_name));
    goto error;
  }

  if (!read_map(fd, map, mod_description, file_name, &width)) {
    yeReCreateInt(width, father, "width");
    return father;
  }

 error:
  if (nb > 2)
    YE_DESTROY(father);
  YE_DESTROY(map);
  return NULL;
}

void *init_sm_reader(int nbArg, void **args)
{
  Entity *t = YE_TO_ENTITY(args[0]);
  yeCreateFunction("load_map", ygGetManager("tcc"), t, "load-map");
  /* yeCreateFunction("load_entity", ygGetManager("tcc"), t, "load-entity"); */

  return NULL;
}

