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

// I have no index...
static void create_tild_multi_layers(Entity *mod_description, int tild,
				     Entity **map, Entity *desc, int pos,
				     int width)
{
  Entity *sp = yeGetByStrFast(desc, "start_pos");
  int start_pos = yeGetInt(yeGetByIdx(sp, 0)) +
    yeGetInt(yeGetByIdx(sp, 0)) * (width ? width : 8000);

  if (start_pos == pos) {
    yePushBack(yeCreateArrayAt(map[1], NULL, pos),
	       yeGetByStrFast(desc, "start_id"), NULL);
  }

  Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(mod_description)->values, tmp,
			    it, ArrayEntry) {
    const char *char_str;

    if (!tmp)
      continue;

    char_str = yeGetString(yeGetByStr(tmp->entity, "map-char"));
    if (char_str[0] == tild) {
      if (yeStringIndexChar(yeGetByStrFast(desc, "ground_char"), char_str) >= 0) {
	yeCreateInt(it, yeCreateArrayAt(map[0], NULL, pos), NULL);
      } else {
	yeCreateInt(it, yeCreateArrayAt(map[2], NULL, pos), NULL);
      }
      return;
    }
  }
  yeCreateInt(tild, yeCreateArrayAt(map[0], NULL, pos), NULL);
}

static void create_tild(Entity *mod_description, int tild,
			Entity *map)
{
  if (mod_description) {
    Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(mod_description)->values, tmp,
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
		    const char *file_name, int *width, int *len, Entity *desc)
{
  int check, pos = 0, i;
  char buff[SM_BUFF_LEN];
  Entity *layers[3];

  if (desc) {
    if (!(yeGetByStrFast(desc, "start_pos") &&
	  yeGetByStrFast(desc, "start_id") &&
	  yeGetByStrFast(desc, "ground_char"))) {
      DPRINT_ERR("missing field in map entity");
      return -1;
    }
    layers[0] = yeCreateArray(yeGetByIdx(map, 0), "map");
    layers[1] = yeCreateArray(yeGetByIdx(map, 1), "map");
    layers[2] = yeCreateArray(yeGetByIdx(map, 2), "map");
  }

 again:
  check = read(fd, buff, SM_BUFF_LEN);
  if (check < 0) {
    DPRINT_ERR("error when reading '%s'\n", file_name);
    return -1;
  }

  for (i = 0; i < check; ++i) {
    if (buff[i] == '\n') {
      *width = *width ? *width : i;
    } else {
      if (desc) {
	create_tild_multi_layers(resources, buff[i], layers, desc, pos, *width);
      } else {
	create_tild(resources, buff[i], map);	
      }
      ++pos;
    }
  }
  if (check > 0)
    goto again;
  if (len)
    *len = pos;
  return 0;
}

void *load_map(int nb, void **args)
{
  const char *file_name = yeGetString(args[0]);
  int fd = open(file_name, O_RDONLY);
  int width = 0;
  Entity *mod_description = nb > 1 ? args[1] : NULL;
  char *name = nb > 3 ? args[3] : NULL;
  Entity *father = nb > 2 ? args[2] : yeCreateArray(NULL, name);
  Entity *map = yeCreateArray(father, "map");

  if (fd < 0) {
    DPRINT_ERR("error when opening '%s'\n", file_name);
    goto error;
  }

  if (!read_map(fd, map, mod_description, file_name, &width, NULL, NULL)) {
    yeReCreateInt(width, father, "width");
    return father;
  }

 error:
  if (nb > 2)
    YE_DESTROY(father);
  YE_DESTROY(map);
  return NULL;
}

void *load_entity(int nb, void **args)
{
  Entity *desc = args[0];

  if (nb != 1) {
    DPRINT_ERR("numbers of arguments incorect\n");
    return NULL;
  }

  const char *file_name = yeGetString(yeGetByStrFast(desc, "map"));
  int fd = open(file_name, O_RDONLY);
  int width = 0, len = 0;
  Entity *resources = yeGetByStrFast(desc, "resources");
  char *name = nb > 2 ? args[2] : NULL;
  Entity *father = nb > 1 ? args[1] : NULL;
  int ret = 0;
  Entity *entries;

  if (fd < 1) {
    DPRINT_ERR("error when opening '%s'\n", file_name);
    goto error;
  }

  yeReCreateString("contener", desc, "<type>");
  yeCreateString("stacking", desc, "cnt-type");
  entries = yeCreateArray(desc, "entries");

  yeCreateArray(entries, NULL);
  yeCreateArray(entries, NULL);
  yeCreateArray(entries, NULL);

  if (read_map(fd, entries, resources, file_name, &width, &len, desc))
    goto error;

  YE_ARRAY_FOREACH(entries, map) {
    yeCreateString("map", map, "<type>");
    yeCreateInt(width, map, "width");
    yeCreateInt(len, map, "len");
    yePushBack(map, resources, "resources");
  }
  return (void *)0x1;
 error:
  yeRemoveChild(desc, entries);
  yeRemoveChildByStr(desc, "cnt-type");
  yeReCreateString("sukeban-map", desc, "<type>");
  return NULL;
}

void *init_sm_reader(int nbArg, void **args)
{
  Entity *t = YE_TO_ENTITY(args[0]);
  yeCreateFunction("load_map", ygGetManager("tcc"), t, "load-map");
  yeCreateFunction("load_entity", ygGetManager("tcc"), t, "load-entity");

  return NULL;
}

