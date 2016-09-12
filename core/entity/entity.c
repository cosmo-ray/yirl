/*
**Copyright (C) 2013 Matthias Gatto
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
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<glib.h>
#include	<inttypes.h>
#include	"entity.h"
#include	"utils.h"
#include	"stack.h"

union FatEntity {
	Entity Entity;
	ArrayEntity ArrayEntity;
	IntEntity IntEntity;
	FloatEntity FloatEntity;
	StringEntity StringEntity;
	DataEntity DataEntity;
	FunctionEntity FunctionEntity;
};

/* Globale array that store every entitys */
static STACK_CREATE(freedElems, int64);
static BlockArray entitysArray;
static int entitysArrayisInit;

/**
 * Here some macros to mutualise the code of entity
 */
#define RETURN_ERROR_BAD_TYPE(function, entity, returnValue) do {	\
  DPRINT_INFO("%s: bad entity, this entity is of type %s\n",		\
	      (function), yeTypeToString(yeType(entity)));		\
  return (returnValue);							\
  } while (0)

#define YE_DECR_REF(entity) do {		\
    entity->refCount -= 1;		       	\
  } while (0)

#define YE_DESTROY_ENTITY(entity, type) do {				\
    YE_DECR_REF(entity);						\
    if (entity->refCount < 1) {						\
      size_t unset =							\
	(size_t)(((union FatEntity *)entity)				\
		 - yBlockArrayGetPtr(&entitysArray, 0, union FatEntity));\
      free(entity->fathers);						\
      yBlockArrayUnset(&entitysArray, unset);				\
      stack_push(freedElems, unset);					\
    }									\
  } while (0);
/* free(((type *)entity));			\ */

#define YE_ALLOC_ENTITY(ret, type) do {					\
    if (unlikely(!entitysArrayisInit)) {				\
      yBlockArrayInitExt(&entitysArray, union FatEntity, YBLOCK_ARRAY_NUMA); \
      entitysArrayisInit = 1;						\
      ret = &(yBlockArraySetGetPtr(&entitysArray,			\
				   stack_pop(freedElems,		\
					     0),			\
				   union FatEntity)->type);		\
      ret->refCount = 1;						\
    } else {								\
      ret = &(yBlockArraySetGetPtr(&entitysArray,			\
				   stack_pop(freedElems,		\
					     yBlockArrayLastPos(&entitysArray) + 1), \
				   union FatEntity)->type);		\
      ret->refCount = 1;						\
    }									\
  } while (0);


static inline Entity *yeInitAt(Entity *entity, EntityType type,
			       Entity *father, const char *name,
			       int at);

/**
 * Contain all functions used to destruct entity
 * Must be in the same order than the EntityType Enum
 * TODO up to date
 */
void (*destroyTab[])(Entity *) = {
  yeDestroyInt,
  yeDestroyFloat,
  yeDestroyString,
  yeDestroyArray,
  yeDestroyFunction,
  yeDestroyData,
};

const char * EntityTypeStrings[] = { "int", "float", "string",
				     "array", "function", "data"};

/**
 * @param entity
 * @param type
 * @return 1 if entity is not null and entity's type is the same as type, 0 otherwise
 */
static inline int	checkType(const Entity *entity, EntityType type)
{
  return (likely(entity != NULL && entity->type == type));
}

EntityType yeStringToType(const char *str)
{
  int i;
  
  for (i = 0; i < NBR_ENTITYTYPE; ++i)
  {
    if (yuiStrEqual(str, EntityTypeStrings[i]))
    	return (i);
  }
  return (-1);
}

const char *yeTypeToString(int type)
{
  return (type < 0 || type >= NBR_ENTITYTYPE)
    ? ("(null)")
    : (EntityTypeStrings[type]);
}

size_t yeLen(Entity *entity)
{
  if (unlikely(!entity))
    return (0);

  if (likely(yeType(entity) == YARRAY)) {
    if (!yBlockArrayIsBlockAllocated(&YE_TO_ARRAY(entity)->values, 0))
      return 0;
    return yBlockArrayLastPos(&YE_TO_ARRAY(entity)->values) + 1;
  }
 
  return YE_TO_STRING(entity)->len;
}

Entity *yeGetByIdx(Entity *entity, size_t index)
{
  if (unlikely(entity == NULL))
    return NULL;
  Entity *tmp;
  tmp = yBlockArrayGet(&YE_TO_ARRAY(entity)->values, index, ArrayEntry).entity;
  return tmp;
}

/**
 * @param name  the name we will search the character '.' into
 * @return the index of the charactere '.' in name
 */
static inline int	findIdxPoint(const char *name)
{
  char *res = strchr(name, '.');
  return (res == NULL)
    ? -1
    : res - name;
}


static inline ArrayEntry *yeGetArrayEntryByIdx(Entity *entity, uint32_t i)
{
  return yBlockArrayGetPtr(&YE_TO_ARRAY(entity)->values, i, ArrayEntry);
}

/**
 * Look for an entity in @entity
 * @param entity  the parent entity
 * @param name    The entity name we are looking for
 * @param end     the size of the @name parameter we look for
 * @return        return the first entity in the parent @entity found
 */
static Entity *yeGetByIdxFastWithEnd(Entity *entity, const char *name, int end)
{

  Y_BLOCK_ARRAY_FOREACH_PTR(&YE_TO_ARRAY(entity)->values, tmp, it, ArrayEntry) {
    if (!strncmp(tmp->name, name, end))
      return tmp->entity;
  }
  return NULL;
}

Entity *yeGetByStrFast(Entity *entity, const char *name)
{
  if (unlikely(!entity || !name || yeType(entity) != YARRAY))
    return NULL;

  Y_BLOCK_ARRAY_FOREACH_PTR(&YE_TO_ARRAY(entity)->values, tmp, it, ArrayEntry) {
    if (!tmp || !tmp->name)
      continue;
    if (yuiStrEqual(tmp->name, name))
      return tmp->entity;
  }
  return NULL; 
}

Entity *yeGetByStrExt(Entity *entity, const char *name, int64_t *idx)
{
  if (!entity || !name || yeType(entity) != YARRAY)
    return NULL;

  Y_BLOCK_ARRAY_FOREACH_PTR(&YE_TO_ARRAY(entity)->values, tmp, it, ArrayEntry) {
    if (!tmp || !tmp->name)
      continue;
    if (yuiStrEqual(tmp->name, name)) {
      *idx = it;
      return tmp->entity;
    }
  }
  *idx = -1;
  return NULL;
}

Entity *yeGetByStr(Entity *entity, const char *name)
{
  int	i;

  if (unlikely(!entity)) {
    DPRINT_INFO("can not find entity for %s\n", name);
    return NULL;
  }

  i = findIdxPoint(name);
  return (i != -1) ?
    (yeGetByStr(yeGetByIdxFastWithEnd(entity, name, i), name + i + 1)) :
    (yeGet(entity, name));
}

int yeArrayIdx(Entity *entity, const char *lookup)
{
  if (!entity || !lookup || yeType(entity) != YARRAY)
    return -1;

  Y_BLOCK_ARRAY_FOREACH_PTR(&YE_TO_ARRAY(entity)->values, tmp, it, ArrayEntry) {
    if (!tmp || !tmp->name)
      continue;
    if (yuiStrEqual(tmp->name, lookup))
      return it;
  }
  return -1; 
  
}

Entity *yeCreateInt(int value, Entity *father, const char *name)
{
  IntEntity *ret;

  YE_ALLOC_ENTITY(ret, IntEntity);
  yeInit((Entity *)ret, YINT, father, name);
  ret->value = value;
  return ((Entity *)ret);
}

Entity *yeCreateData(void *value, Entity *father, const char *name)
{
  DataEntity *ret;

  YE_ALLOC_ENTITY(ret, DataEntity);
  yeInit((Entity *)ret, YDATA, father, name);
  ret->value = value;
  return ((Entity *)ret);
}

Entity *yeCreateArray(Entity *father, const char *name)
{
  ArrayEntity *ret;

  YE_ALLOC_ENTITY(ret, ArrayEntity);
  yeInit((Entity *)ret, YARRAY, father, name);
  yBlockArrayInit(&ret->values, ArrayEntry);
  return (YE_TO_ENTITY(ret));
}

Entity *yeCreateArrayAt(Entity *father, const char *name, int idx)
{
  ArrayEntity *ret;

  YE_ALLOC_ENTITY(ret, ArrayEntity);
  yeInitAt((Entity *)ret, YARRAY, father, name, idx);
  yBlockArrayInit(&ret->values, BlockArray);
  return (YE_TO_ENTITY(ret));
}

Entity *yeCreateFloat(double value, Entity *father, const char *name)
{
  FloatEntity *ret;

  YE_ALLOC_ENTITY(ret, FloatEntity);
  yeInit((Entity *)ret, YFLOAT, father, name);
  ret->value = value;
  return ((Entity *)ret);
}

Entity *yeCreateFunction(const char *funcName, void *manager,
			 Entity *father, const char *name)
{
  FunctionEntity *ret;

  YE_ALLOC_ENTITY(ret, FunctionEntity);
  yeInit((Entity *)ret, YFUNCTION, father, name);
  ret->value = NULL;
  ret->manager = manager;
  ret->fastPath = NULL;
  yeSetString(YE_TO_ENTITY(ret), funcName);
  return (YE_TO_ENTITY(ret));
}

Entity *yeCreateString(const char *string, Entity *father, const char *name)
{
  StringEntity *ret;

  YE_ALLOC_ENTITY(ret, StringEntity);
  yeInit((Entity *)ret, YSTRING, father, name);
  ret->value = NULL;
  yeSetString(YE_TO_ENTITY(ret), string);
  return (YE_TO_ENTITY(ret));
}

static inline void arrayEntryInit(ArrayEntry *ae)
{
  ae->entity = NULL;
  ae->name = NULL;
}

static inline void arrayEntryDestroy(ArrayEntry *ae)
{
  if (unlikely(!ae))
    return;
  g_free(ae->name);
  YE_DESTROY(ae->entity);
  arrayEntryInit(ae);
}

static void yeRemoveFather(Entity *entity, Entity *father)
{
  if (unlikely(!entity || !father))
    return;
  Entity **fathers = entity->fathers;

  // if the father is in last position, so we don't care about it because
  // we decr_ref at the end.
  for (int i = 0, end = entity->nbFathers; i < end ; ++i)
    {
      if (fathers[i] == father) {
	fathers[i] = fathers[end - 1];
	fathers[end - 1] = NULL;
	entity->nbFathers -= 1;
	return;
      }
    }
}

static void destroyChilds(Entity *entity)
{
  
  for (int i = 0, end = yeLen(entity); i < end; ++i) {
    ArrayEntry *ae = yeGetArrayEntryByIdx(entity, i);

    yeRemoveFather(ae->entity, entity);
    arrayEntryDestroy(ae);
    yBlockArrayUnset(&YE_TO_ARRAY(entity)->values, i);
  }
}

void yeDestroyInt(Entity *entity)
{
  YE_DESTROY_ENTITY(entity, IntEntity);
}

void yeDestroyFloat(Entity *entity)
{
  YE_DESTROY_ENTITY(entity, FloatEntity);
}

void yeDestroyFunction(Entity *entity)
{
  if (YE_TO_FUNC(entity)->value != NULL &&
      entity->refCount == 1) {
    free(YE_TO_FUNC(entity)->value);
  }
  YE_DESTROY_ENTITY(entity, FunctionEntity);
}

void yeDestroyString(Entity *entity)
{
  if (YE_TO_STRING(entity)->value != NULL &&
      entity->refCount == 1) {
    free(YE_TO_STRING(entity)->value);
  }
  YE_DESTROY_ENTITY(entity, StringEntity);
}

void yeDestroyData(Entity *entity)
{
  if (YE_TO_DATA(entity)->value && YE_TO_DATA(entity)->destroy &&
      entity->refCount == 1)
    YE_TO_DATA(entity)->destroy(YE_TO_DATA(entity)->value);
  YE_DESTROY_ENTITY(entity, DataEntity);
}

void yeDestroyArray(Entity *entity)
{
  if(entity->refCount == 1) {
    destroyChilds(entity);
    yBlockArrayFree(&YE_TO_ARRAY(entity)->values);
  }
  YE_DESTROY_ENTITY(entity, ArrayEntity);
}

void yeDestroy(Entity *entity)
{
  if (unlikely(!entity))
    return;
  destroyTab[entity->type](entity);
}

Entity *yeCreate(EntityType type, void *val, Entity *father, const char *name)
{
  switch (type)
    {
    case YSTRING:
      return yeCreateString(val, father, name);
    case YINT:
      return yeCreateInt(*((int *)&val), father, name);
    case YFLOAT:
      return yeCreateFloat(*((double *)&val), father, name);
    case YARRAY:
      return yeCreateArray(father, name);
    case YDATA:
      return yeCreateData(val, father, name);
    case YFUNCTION:
      return yeCreateFunction(val, NULL, father, name);
    default:
      DPRINT_ERR( "%s generic constructor not yet implemented\n",
		  yeTypeToString(type));
      break;
    }
  return (NULL);
}

typedef enum
  {
    NONE = 0,
    NO_ENTITY_DESTROY = 1
  } ManageArrayFlag;


static ArrayEntity	*manageArrayInternal(ArrayEntity *entity,
					     unsigned int size,
					     ManageArrayFlag flag)
{
  unsigned int len = yeLen(YE_TO_ENTITY(entity));

  if (size < yeLen(YE_TO_ENTITY(entity)) && !(flag & NO_ENTITY_DESTROY)) {
    for (unsigned int i = size; i < len; ++i) {
      arrayEntryDestroy(&yBlockArrayGet(&entity->values, i, ArrayEntry));
      yBlockArrayUnset(&entity->values, i);
    }
  }

  yBlockArrayAssureBlock(&entity->values, size);

  for (unsigned int i = len; i < size; ++i) {
    yBlockArraySet(&entity->values, i);
    arrayEntryInit(yeGetArrayEntryByIdx(YE_TO_ENTITY(entity), i));
  }
  return entity;
}

Entity *yeExpandArray(Entity *entity, unsigned int size)
{
  if (!checkType(entity, YARRAY)) {
    DPRINT_ERR("bad argument 1 of type '%s', should be array\n",
	       yeTypeToString( yeType(entity)));
    return NULL;
  }
  return ((Entity*)manageArrayInternal((ArrayEntity*)entity, size, NONE));
}

int	yePushBack(Entity *entity, Entity *toPush, const char *name)
{
  int ret;

  if (unlikely(!entity || !toPush))
    return -1;
  if (unlikely(!checkType(entity, YARRAY))) {
    DPRINT_ERR("bad argument 1 of type '%s', should be array\n",
	       yeTypeToString( yeType(entity)));
    return -1;
  }
  ret = yeAttach(entity, toPush, yeLen(entity), name);
  return ret;
}

Entity *yeRemoveChild(Entity *array, Entity *toRemove)
{
  if (!checkType(array, YARRAY)) {
    DPRINT_ERR("bad argument 1 of type '%s', should be array\n",
	       yeTypeToString(yeType(array)));
    return NULL;
  }

  Y_BLOCK_ARRAY_FOREACH_PTR(&YE_TO_ARRAY(array)->values, tmp, it, ArrayEntry) {
    Entity *ret;

    tmp = yeGetArrayEntryByIdx(array, it);
    ret = tmp->entity;
    if (ret == toRemove) {
      arrayEntryDestroy(tmp);

      yBlockArrayUnset(&YE_TO_ARRAY(array)->values, it);
      return ret;
    }
  }
  return NULL;
}


Entity *yePopBack(Entity *entity)
{
  int	len;
  Entity *ret;

  if (!checkType(entity, YARRAY)) {
    DPRINT_ERR("yePopBack: bad entity\n");
    return NULL;
  }
  len = yeLen(entity);
  ret = yeGet(entity, len - 1);
  yeExpandArray(entity, len - 1);
  return (ret);
}

static void yeAttachFather(Entity *entity, Entity *father)
{
  if (unlikely(!entity || !father))
    return;
  entity->fathers =
    realloc(entity->fathers, sizeof(Entity *) * (entity->nbFathers + 1));
  entity->fathers[entity->nbFathers] = father;
  entity->nbFathers += 1;
}

static inline Entity *yeInitAt(Entity *entity, EntityType type,
			       Entity *father, const char *name,
			       int at)
{
  if (unlikely(!entity))
    return NULL;
  entity->type = type;
  entity->nbFathers = 0;
  entity->fathers = NULL;
  /* this can be simplifie */
  if (at < 0) {
    if (!yePushBack(father, entity, name))
      YE_DECR_REF(entity);
  } else {
    if (!yeAttach(father, entity, at, name))
      YE_DECR_REF(entity);
  }
  return entity;
}


Entity *yeInit(Entity *entity, EntityType type, Entity *father, const char *name)
{
  return yeInitAt(entity, type, father, name, -1);
}

void	yeSetString(Entity *entity, const char *val)
{
  if (unlikely(!entity))
    return;
  if (YE_TO_STRING(entity)->value != NULL)
    free(YE_TO_STRING(entity)->value);
  if (val != NULL) {
    YE_TO_STRING(entity)->value = strdup(val);
    if (entity->type == YSTRING)
      YE_TO_STRING(entity)->len = strlen(val);
  } else {
    YE_TO_STRING(entity)->value = NULL;
    if (entity->type == YSTRING)
      YE_TO_STRING(entity)->len = 0;
  }
}

void yeSetDestroy(Entity *entity, void (*destroyFunc)(void *))
{
  YE_TO_DATA(entity)->destroy = destroyFunc;
}

int yeAttach(Entity *on, Entity *entity,
	     unsigned int idx, const char *name)
{
  ArrayEntry *entry;
  
  if (unlikely(!on || !entity))
    return -1;
  if (on->type != YARRAY)
    return -1;

  yBlockArrayAssureBlock(&YE_TO_ARRAY(on)->values, idx);
  entry = yeGetArrayEntryByIdx(on, idx);
  if (yBlockArrayIsSet(&YE_TO_ARRAY(on)->values, idx))
    arrayEntryDestroy(entry);

  yBlockArraySet(&YE_TO_ARRAY(on)->values, idx);
  entry->entity = entity;
  entry->name = g_strdup(name);  
  yeAttachFather(entity, on);
  YE_INCR_REF(entity);
  return 0;
}

void	yeSetStringAt(Entity *entity, unsigned int index, const char *value)
{
  return yeSetString(yeGet(entity, index), value);
}

void	yeSetIntAt(Entity *entity, unsigned int index, int value)
{
  return yeSetInt(yeGet(entity, index), value);
}

void	yeSetFloatAt(Entity *entity, unsigned int index, double value)
{
  return yeSetFloat(yeGet(entity, index), value);
}

void	yeSetStringAtStrIdx(Entity *entity, const char *index, const char *value)
{
  return yeSetString(yeGet(entity, index), value);
}

void	yeSetIntAtStrIdx(Entity *entity, const char *index, int value)
{
  return yeSetInt(yeGet(entity, index), value);
}

void	yeSetFloatAtStrIdx(Entity *entity, const char *index, double value)
{
  return yeSetFloat(yeGet(entity, index), value);
}

void	yeUnsetFunction(Entity *entity)
{
  if (unlikely(!entity))
    return;
  yeSetFunction(entity, NULL);
}

void	yeSetFunction(Entity *entity, const char *value)
{
  if (unlikely(!entity))
    return;
  return yeSetString(entity, value);
}

void	yeSetInt(Entity *entity, int value)
{
  if (unlikely(!entity))
    return;
  ((IntEntity *)entity)->value = value;
}

void	yeSetFloat(Entity *entity, double value)
{
  if (unlikely(!entity))
      return;
  ((FloatEntity *)entity)->value = value;
}

const char *yeGetString(Entity *entity)
{
  if (!checkType(entity, YSTRING)) {
    RETURN_ERROR_BAD_TYPE("getStringVal", entity, NULL);
  }
  return ((StringEntity *)entity)->value;
}

int	yeGetInt(Entity *entity)
{
  if (!checkType(entity, YINT)) {
    RETURN_ERROR_BAD_TYPE("getIntVal", entity, -1);
  }
  return YE_TO_INT(entity)->value;
}

void	*yeGetData(Entity *entity)
{
  if (!checkType(entity, YDATA)) {
    RETURN_ERROR_BAD_TYPE("getDataVal", entity, NULL);
  }
  return YE_TO_DATA(entity)->value;
}


const char	*yeGetFunction(Entity *entity)
{
  if (!checkType(entity, YFUNCTION)) {
    RETURN_ERROR_BAD_TYPE("getFunctionVal", entity, NULL);
  }
  return YE_TO_FUNC(entity)->value;
}

double	yeGetFloat(Entity *entity)
{
  if (!checkType(entity, YFLOAT)) {
    RETURN_ERROR_BAD_TYPE("yeGetFloat", entity, -1);
  }
  return ((FloatEntity *)entity)->value;
}


Entity **yeFathers(Entity *entity)
{
  return entity->fathers;
}

Entity*		yeCopy(Entity* src, Entity* dest)
{
  const char* strVal = NULL;

  if (src != NULL && dest != NULL
      && yeType(src) == yeType(dest)) {
    DPRINT_INFO("\tentity are '%s'\n", yeTypeToString(yeType(src)));
    switch (yeType(src))
    {
    case YINT:
      yeSetInt(dest, yeGetInt(src));
      break;
    case YFLOAT:
      yeSetFloat(dest, yeGetFloat(src));
      break;
    case YSTRING:
      strVal = yeGetString(src);
      DPRINT_INFO("\t\tvalue is string \"%s\"\n",
		  (strVal != NULL) ? strVal : "null");
      yeSetString(dest, strVal);
      break;
    case YARRAY:
      yeCopyContener((ArrayEntity*)src, (ArrayEntity*)dest);
      break;
    case YFUNCTION:
      strVal = yeGetFunction(src);
      DPRINT_INFO("\t\tvalue is function '%s'\n", strVal);
      yeSetFunction(dest, strVal);
      YE_TO_FUNC(dest)->manager = YE_TO_FUNC(src)->manager;
      break;
    default:
      DPRINT_ERR("type %s not handle", yeTypeToString(yeType(src)));
      goto error;
    }
    return dest;
  }
 error:
  return NULL;
}

ArrayEntity	*yeCopyContener(ArrayEntity* src, ArrayEntity* dest)
  {
    if (src == NULL || dest == NULL)
      return NULL;

    ArrayEntry *destElem;

    Y_BLOCK_ARRAY_FOREACH_PTR(&src->values, elem, it, ArrayEntry) {

      yBlockArrayCopyElem(&dest->values, it, elem);

      destElem = yBlockArrayGetPtr(&dest->values, it, ArrayEntry);
      destElem->name = g_strdup(elem->name);
      yeCopy(elem->entity, destElem->entity);
    }
    return dest;
  }

Entity *yeFindLink(Entity *array, const char *targetPath, int flag)
{
  Entity *ret = NULL;
  
  if (yeType(array) != YARRAY)
    return NULL;
  if (!(flag & YE_FIND_LINK_NO_GET) && (ret = yeGet(array, targetPath)) != NULL)
    return ret;

  if (flag & YE_FIND_LINK_NO_DEEP)
    return NULL;

  YE_FOREACH_FATHER(array, tmp) {
    if ((ret = yeGet(tmp, targetPath)) != NULL)
      return ret;
  }

  YE_FOREACH_FATHER(array, tmp2) {
    ret = yeFindLink(tmp2, targetPath, YE_FIND_LINK_NO_GET | flag);
    if (ret)
      return ret;
  }

  return NULL;
}

static void yeToCStrInternal(Entity *entity, int deep, GString *str, int flag)
{
  if (!deep)
    return;
  switch (yeType(entity)) {
  case YSTRING :
    g_string_append_printf(str, "\"%s\"", yeGetString(entity));
    break;
  case YINT :
    g_string_append_printf(str, "'%d'", yeGetInt(entity));
    break;
  case YFLOAT :
    g_string_append_printf(str, "'%f'", yeGetFloat(entity));
    break;
  case YFUNCTION :
    g_string_append_printf(str, "(%s)", yeGetFunction(entity));
    break;
  case YARRAY :
    g_string_append_c(str, '[');
    if (yeLen(entity) > 20)
      flag |= YE_FORMAT_OPT_PRINT_ONLY_VAL_ARRAY;
    Y_BLOCK_ARRAY_FOREACH_PTR(&YE_TO_ARRAY(entity)->values,
			      tmp, it, ArrayEntry) {
      if (!(flag & YE_FORMAT_OPT_PRINT_ONLY_VAL_ARRAY)) {
	if (it) {
	  g_string_append(str, " | ");
	}
	if (tmp->name)
	  g_string_append_printf(str, "name: \"%s\", ", tmp->name);
	g_string_append_printf(str, "idx: %"PRIu64", ", it);
	g_string_append_printf(str, "val: ");
      }
      yeToCStrInternal(tmp->entity, deep - 1, str, flag);
    }

    g_string_append_c(str, ']');
    if (flag & YE_FORMAT_OPT_BREAK_ARRAY_END)
      g_string_append_c(str, '\n');
    break;
  case YDATA :
    g_string_append_printf(str, "'%p'", yeGetData(entity));
    break;
  default :
    break;
  }
}

char *yeToCStr(Entity *entity, int deep, int flag)
{
  GString *str = g_string_new(NULL);

  yeToCStrInternal(entity, deep, str, flag);
  return g_string_free(str, 0);
}

int yeRenamePtrStr(Entity *array, Entity *ptr, const char *str)
{
  YE_INCR_REF(ptr);
  if (yeRemoveChild(array, ptr))
    yePushBack(array, ptr, str);
  YE_DESTROY(ptr);
  return 0;
}

  
#undef YE_DECR_REF

#undef YE_DESTROY_ENTITY
  
#undef YE_ALLOC_ENTITY

#undef RETURN_ERROR_BAD_TYPE
