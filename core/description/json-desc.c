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

typedef struct {
  /* the name of our future entity */
  char *refName;
  /* the name of the entity we are looking for */
  char *targetName;
  /* the entity which should contan the ref */
  Entity *refFather;
} LinkInfo;

static int t = -1;
static const char *nameType = "json";
static GList *linkList;

static Entity *parseGen(struct json_object *obj, const char *name,
			Entity *father);

static inline void destroyLinkInfo(LinkInfo *li)
  __attribute__((nonnull));


static inline void destroyLinkInfo(LinkInfo *li)
{
  g_free(li->refName);
  g_free(li->targetName);
  g_free(li);  
}

/**
 * @return 1 if the link was sucessful
 */
static int jsonLink(Entity *father, Entity *target, const char *refName,
		    const char *targetName)
{
  Entity *ret;

  if ((ret = yeGet(father, targetName)) != NULL) {
    yePushBack(target, ret, refName);
    return 1;
  }
  return 0;
}

static int tryLinkForeachFathers(LinkInfo *data,
				 Entity *current)
{
  YE_FOREACH_FATHER(current, father) {
    if (jsonLink(father, data->refFather, data->refName, data->targetName) ||
	(father->nbFathers && tryLinkForeachFathers(data, father))) {
      return 1;
    }
  }
  return 0;
}

/**
 * @return the number of entity that have been linked, -1 on error
 */
static int jsonLinker(void)
{
  int ret = 0;

  for (GList *cur = linkList; cur != NULL;) {
    LinkInfo *data = cur->data;
    GList *next = cur->next;

    if (jsonLink(data->refFather, data->refFather,
		 data->refName, data->targetName) ||
	tryLinkForeachFathers(data, data->refFather)) {
      destroyLinkInfo(data);
      linkList = g_list_delete_link(linkList, cur);
      ++ret;
      goto next;
    }
  next:
    cur = next;
  }

  return ret;
}

static void jsonClearLinker(void)
{
  for (GList *cur = linkList; cur != NULL; cur = cur->next) {
    LinkInfo *tmp;
    tmp = cur->data;
    destroyLinkInfo(tmp);
  }
  g_list_free(linkList);
  linkList = NULL;
}

static void addLinkInfo(Entity *refFather, const char *refName,
			const char *targetName)
{
  LinkInfo *linkInfo = g_new(LinkInfo, 1);
  
  linkInfo->refName = g_strdup(refName);
  linkInfo->targetName = g_strdup(targetName);
  linkInfo->refFather = refFather;
  linkList = g_list_append(linkList, linkInfo);
}

static Entity *parseObject(struct json_object *obj,
			   const char *name, Entity *father)
{
  Entity *ret = yeCreateArray(father, name);
  
  if (ret == NULL) {
    YE_DESTROY(father); // change this to free the head entty on the tree
    return NULL;
  }

  json_object_object_foreach(obj, key, val) {
    if (key[0] == '&') {
      addLinkInfo(ret, key + 1, json_object_get_string(val));
      continue;
    }
    if (g_str_equal(key, "_name") &&
    	(json_object_get_type(val) == json_type_string)) {
      continue;
    /*   yeSetName(ret, json_object_get_string(val)); */
    } else {
      if (parseGen(val, key, ret) == NULL) {
	DPRINT_ERR("fail to parse json obj %s", name);
	return NULL;
      }
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
  return yeCreateInt(json_object_get_int(obj), father, name);
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
  jsonLinker(); // if jsonLinker is not empty free it, return an error ?
  jsonClearLinker();
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

