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

#ifdef JSON_C_FULL_INCLUDE_PATH
#include <json-c/json.h>
#else
#include <json.h>
#endif


#include "json-desc.h"
#include "game.h"


static int t = -1;
static const char *nameType = "json";

static Entity *parseGen(struct json_object *obj, const char *name,
			Entity *father);

static Entity *parseObject(struct json_object *obj,
			   const char *name, Entity *father)
{
  Entity *ret = yeCreateArray(father, name);

  if (ret == NULL) {
    YE_DESTROY(father); // change this to free the head entty on the tree
    return NULL;
  }

  json_object_object_foreach(obj, key, val) {
    if (parseGen(val, key, ret) == NULL) {
      DPRINT_ERR("fail to parse json obj %s", name);
      return NULL;
    }
  }

  return ret;
}

static Entity *parseArray(struct json_object *obj,
			  const char *name, Entity *father)
{
  Entity *ret = yeCreateArray(father, name);
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
  return yeCreateInt(json_object_get_int64(obj), father, name);
}

static Entity *parseBool(struct json_object *obj, const char *name,
			 Entity *father)
{
  return yeCreateInt(json_object_get_boolean(obj), father, name);
}

static Entity *parseFloat(struct json_object *obj, const char *name,
			  Entity *father)
{
  return yeCreateFloat(json_object_get_double(obj), father, name);
}

static Entity *parseString(struct json_object *obj, const char *name,
			   Entity *father)
{
  return yeCreateString(json_object_get_string(obj), father, name);
}


static Entity *parseGen(struct json_object *obj, const char *name,
			Entity *father)
{
  switch (json_object_get_type(obj))
    {
    case json_type_object:
      return parseObject(obj, name, father);
    case json_type_boolean:
      return parseBool(obj, name, father);
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

static Entity *jsonFromFile(void *opac, const char *fileName, Entity *father)
{
  struct json_object *file = json_object_from_file(fileName);
  Entity *ret;

  (void)opac;
  if (!file) {
	  if (access(fileName, F_OK) < 0) {
		  DPRINT_ERR("can not open %s, no sure file", fileName ?
			     fileName : "(nil)");
		  ygDgbAbort();
	  } else {
		  DPRINT_ERR("Error in json of %s", fileName);
	  }
	  return NULL;
  }
  ret = parseGen(file, NULL, father);
  json_object_put(file);
  return ret;
}

static json_object *jsonObjectFromEntity(Entity *entity);

static json_object *jsonObjectFromIntEntity(Entity *entity)
{
  return json_object_new_int(yeGetInt(entity));
}

static json_object *jsonObjectFromDoubleEntity(Entity *entity)
{
  return json_object_new_double(yeGetFloat(entity));
}

static json_object *jsonObjectFromStringEntity(Entity *entity)
{
	const char *str = yeGetString(entity);
	return json_object_new_string(str ? str : "(nil)");
}

static int isArray(Entity *entity)
{
  YE_ARRAY_FOREACH_EXT(entity, child, it) {
    if (yBlockArrayIteratorGetPtr(it, ArrayEntry)->name)
      return 0;
  }
  return 1;
}

static json_object *arrayJsonObjectFromArrayEntity(Entity *entity)
{
  struct json_object *json_obj = json_object_new_array();

  YE_ARRAY_FOREACH_EXT(entity, child, it) {
    json_object_array_add(json_obj, jsonObjectFromEntity(child));
  }
  return json_obj;
}

static json_object *jsonObjectFromArrayEntity(Entity *entity)
{
  struct json_object *json_obj;

  if (isArray(entity))
    return arrayJsonObjectFromArrayEntity(entity);

  json_obj = json_object_new_object();

  YE_ARRAY_FOREACH_EXT(entity, child, it) {
    char buf[32];
    const char *key = yBlockArrayIteratorGetPtr(it, ArrayEntry)->name;

    if (!key) {
      snprintf(buf, 32, "%d", it.pos);
      key = buf;
    }
    json_object_object_add(json_obj, key,
			   jsonObjectFromEntity(child));
  }
  return json_obj;
}

static json_object *jsonObjectFromEntity(Entity *entity)
{
  switch (yeType(entity)) {
  case YINT:
    return jsonObjectFromIntEntity(entity);
  case YFLOAT:
    return jsonObjectFromDoubleEntity(entity);
  case YSTRING:
  case YFUNCTION:
    return jsonObjectFromStringEntity(entity);
  case YARRAY:
    return jsonObjectFromArrayEntity(entity);
  case YDATA:
  default:
    return NULL;
  }
}

static int jsonToFile(void *opac, const char *fileName, Entity *entity)
{
  struct json_object *json_obj = jsonObjectFromEntity(entity);

  (void)opac;
  json_object_to_file_ext((char *)fileName, json_obj, JSON_C_TO_STRING_PRETTY);
  json_object_put(json_obj);
  return 0;
}

static int jsonDestroy(void *opac)
{
  free(opac);
  return 0;
}

static void *jsonAllocator(void)
{
  YDescriptionOps *ret;

  ret = malloc(sizeof *ret);
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

