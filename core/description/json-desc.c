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

#include <glib.h>
#include <json-c/json.h>

#include "json-desc.h"

static int t = -1;
static const char *nameType = "json";

static Entity *parseGen(struct json_object *obj, const char *name,
			Entity *father);

static Entity *parseObject(struct json_object *obj,
			   const char *name, Entity *father)
{
  Entity *ret = yeCreateArray(name, father);
  
  if (ret == NULL) {
    YE_DESTROY(father); // change this to free the head entty on the tree
    return NULL;
  }

  json_object_object_foreach(obj, key, val) {
    if (g_str_equal(key, "_name") &&
	(json_object_get_type(val) == json_type_string)) {
      yeSetName(ret, json_object_get_string(val));
    } else if (parseGen(val, key, ret) == NULL) {
      return NULL;
    }
  }

  return ret;
}

static Entity *parseArray(struct json_object *obj,
			  const char *name, Entity *father)
{
  Entity *ret = yeCreateArray(name, father);
  int len = json_object_array_length(obj);
  
  if (ret == NULL) {
    YE_DESTROY(father); // change this to free the head entty on the tree
    return NULL;
  }

  for (int i = 0; i < len; ++i) {
    struct json_object *val = json_object_array_get_idx(obj, i);

    if (parseGen(val, NULL, ret) == NULL)
      return NULL;
  }

  return ret;
}

static Entity *parseInt(struct json_object *obj, const char *name,
			Entity *father)
{
  return yeCreateInt(name, json_object_get_int(obj), father);
}

static Entity *parseFloat(struct json_object *obj, const char *name,
			  Entity *father)
{
  return yeCreateFloat(name, json_object_get_double(obj), father);
}

static Entity *parseString(struct json_object *obj, const char *name,
			   Entity *father)
{
  return yeCreateString(name, json_object_get_string(obj), father);
}


static Entity *parseGen(struct json_object *obj, const char *name,
			Entity *father)
{
  switch (json_object_get_type(obj))
    {
    case json_type_object:
      return parseObject(obj, name, father);
    case json_type_int:
      return parseInt(obj, name, father);
    case json_type_double:
      return parseFloat(obj, name, father);
    case json_type_string:
      return parseString(obj, name, father);
    case json_type_array:
      return parseArray(obj, name, father);
    default:
      return NULL;
    }
}

static Entity *jsonFromFile(void *opac, const char *fileName)
{
  struct json_object *file = json_object_from_file(fileName);
  Entity *ret;
  
  (void)opac;
  if (!file) {
    if (!g_file_test(fileName, G_FILE_TEST_EXISTS))
      DPRINT_ERR("can not open %s, no sure file", fileName);
    else
      DPRINT_ERR("Error in json of %s", fileName);
    return NULL;
  }
  ret = parseGen(file, NULL, NULL);
  json_object_put(file);
  return ret;
}

static int jsonToFile(void *opac, const char *fileName, Entity *entity)
{
  (void)fileName;
  (void)opac;
  (void)entity;
  return -1;
}

static int jsonDestroy(void *opac)
{
  free(opac);
  return 0;
}

static void *jsonAllocator(void)
{
  YDescriptionOps *ret;
  
  ret = g_new(YDescriptionOps, 1);
  if (ret == NULL)
    return NULL;
  ret->name = nameType;
  ret->toFile = jsonToFile;
  ret->fromFile = jsonFromFile;
  ret->destroy = jsonDestroy;
  return ret;
}

int ydJsonGetType(void)
{
  return t;
}

int ydJsonInit(void)
{
  t = ydRegister(jsonAllocator);
  return t;
}

int ydJsonEnd(void)
{
  return ydUnregiste(t);
}

