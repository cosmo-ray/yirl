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
#include	<unistd.h>
#include	"entity.h"
#include	"utils.h"
#include	"stack.h"
#include	"script.h"

/* Globale array that store every entitys */
static STACK_CREATE(freedElems, int64);
static int isInit;
static BlockArray entitysArray;
static inline Entity *yeInit(Entity *entity, EntityType type,
			     Entity *father, const char *name);

#define YE_DECR_REF(entity) do {		\
    entity->refCount -= 1;		       	\
  } while (0)

#define YE_DESTROY_ENTITY(entity, type) do {				\
    YE_DECR_REF(entity);						\
    if (entity->refCount < 1) {						\
      int64_t unset =							\
	(size_t)(((union FatEntity *)entity)				\
		 - yBlockArrayGetPtrDirect(entitysArray, 0, union FatEntity));	\
      yBlockArrayUnset(&entitysArray, unset);				\
      stack_push(freedElems, unset);					\
    }									\
  } while (0);

#define YE_ALLOC_ENTITY(ret, type) do {					\
    ret = &(yBlockArraySetGetPtr(&entitysArray,				\
				 stack_pop(freedElems,			\
					   yBlockArrayLastPos(entitysArray) + 1), \
				 union FatEntity)->type);		\
    ret->refCount = 1;							\
  } while (0);


void yeInitMem(void)
{
  if (!isInit) {
    yBlockArrayInitExt(&entitysArray, union FatEntity,
		       YBLOCK_ARRAY_NUMA | YBLOCK_ARRAY_NOINIT |
		       YBLOCK_ARRAY_NOMIDFREE);
    isInit = 1;
  }
}

int yeIsPtrAnEntity(void *ptr)
{
  return ((union FatEntity *)ptr) >= yBlockArrayGetPtrDirect(entitysArray, 0,
							     union FatEntity) &&
    ((union FatEntity *)ptr) <= (yBlockArrayGetPtrDirect(entitysArray,
							 0, union FatEntity) +
				 yBlockArrayLastPos(entitysArray));
}

int yeCreateInts_(Entity *fathers, int nbVars, ...)
{
  va_list ap;

  if (!fathers)
    return -1;
  va_start(ap, nbVars);
  for (int i = 0; i < nbVars; ++i) {
    yeCreateInt(va_arg(ap, int), fathers, NULL);
  }
  va_end(ap);
  return 0;
}

void yeEnd(void)
{
  isInit = 0;
  stack_destroy(freedElems);
  yBlockArrayFree(&entitysArray);
}

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

int yeStrCmp(Entity *ent1, const char *str)
{
  const char *eStr = yeGetString(ent1);
  if (!eStr)
    return -1;
  return strcmp(eStr, str);
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
    ? ("unknow")
    : (EntityTypeStrings[type]);
}

size_t yeLen(Entity *entity)
{
  if (unlikely(!entity))
    return (0);

  if (likely(yeType(entity) == YARRAY)) {
    return yBlockArrayLastPos(YE_TO_ARRAY(entity)->values) + 1;
  }

  return YE_TO_STRING(entity)->len;
}

Entity *yeGetByIdx(Entity *entity, size_t index)
{
  if (unlikely(entity == NULL))
    return NULL;
  return yeGetByIdxDirect(entity, index);
}

Entity *yeGetLast(Entity *array)
{
  return yeGet(array, yeLen(array) - 1);
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

static inline ArrayEntry *yeGetArrayEntryByStr(Entity *entity, const char *str)
{
  Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(entity)->values, elem, it, ArrayEntry) {
    if (elem && yuiStrEqual(str, elem->name))
      return elem;
  }
  return NULL;
}

static inline ArrayEntry *yeGetArrayEntryByIdx(Entity *entity, uint32_t i)
{
  return yBlockArrayGetPtr(&YE_TO_ARRAY(entity)->values, i, ArrayEntry);
}

int yeSetFlagByIdx(Entity *array, int idx, int flag)
{
  ArrayEntry *ae = yeGetArrayEntryByIdx(array, idx);

  if (unlikely(!ae))
    return -1;
  ae->flags |= flag;
  return 0;
}

int yeSetFlagByStr(Entity *array, const char *name, int flag)
{
  ArrayEntry *ae = yeGetArrayEntryByStr(array, name);

  if (unlikely(!ae))
    return -1;
  ae->flags |= flag;
  return 0;
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

  Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(entity)->values, tmp, it, ArrayEntry) {
    if (unlikely(!tmp || !tmp->name))
      continue;
    if (!strncmp(tmp->name, name, end))
      return tmp->entity;
  }
  return NULL;
}

Entity *yeGetByStrFast(Entity *entity, const char *name)
{
  if (unlikely(!entity || !name || yeType(entity) != YARRAY))
    return NULL;

  Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(entity)->values, tmp, it, ArrayEntry) {
    if (unlikely(!tmp || !tmp->name))
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

  Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(entity)->values, tmp, it, ArrayEntry) {
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

Entity *yeNGetByStr(Entity *entity, const char *name, int len)
{
  int cur = 0;
  Entity *ret = NULL;

  if (unlikely(!entity || !name)) {
    DPRINT_INFO("can not find entity for %s\n", name);
    return NULL;
  }

  while (cur < len) {
    int i;

    i = findIdxPoint(name);
    if (i == -1)
      return yeGetByIdxFastWithEnd(entity, name, len - cur);

    ret = yeGetByIdxFastWithEnd(entity, name, i);
    name += i + 1;
    cur += i;
  }

  return ret;
}

Entity *yeGetByStr(Entity *entity, const char *name)
{
  int	i;

  if (unlikely(!entity || !name)) {
    DPRINT_INFO("can not find entity for %s\n", name);
    return NULL;
  }

  i = findIdxPoint(name);
  if (i == -1)
    return yeGet(entity, name);
 return yeGetByStr(yeGetByIdxFastWithEnd(entity, name, i), name + i + 1);
}

static inline Entity *yeInitAt(Entity * restrict const entity,
			       EntityType type,
			       Entity * restrict const father,
			       const char * const restrict name,
			       int at)
{
  if (unlikely(!entity))
    return NULL;
  entity->type = type;
  /* this can be simplifie */
  if (at < 0) {
    if (!yePushBack(father, entity, name))
      YE_DECR_REF(entity);
  } else {
    if (!yeAttach(father, entity, at, name, 0))
      YE_DECR_REF(entity);
  }
  return entity;
}

int yeArrayIdx(Entity *entity, const char *lookup)
{
  if (!entity || !lookup || yeType(entity) != YARRAY)
    return -1;

  Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(entity)->values, tmp, it, ArrayEntry) {
    if (unlikely(!tmp))
      continue;
    if (yuiStrEqual0(tmp->name, lookup))
      return it;
  }
  return -1;
}

Entity *yeCreateInt(int value, Entity *father, const char *name)
{
  IntEntity * restrict ret;

  YE_ALLOC_ENTITY(ret, IntEntity);
  yeInit((Entity *)ret, YINT, father, name);
  ret->value = value;
  return ((Entity *)ret);
}

Entity *yeCreateIntAt(int value, Entity *father, const char *name, int idx)
{
  IntEntity * restrict ret;

  YE_ALLOC_ENTITY(ret, IntEntity);
  yeInitAt((Entity *)ret, YINT, father, name, idx);
  ret->value = value;
  return ((Entity *)ret);
}

Entity *yeCreateData(void *value, Entity *father, const char *name)
{
  DataEntity * restrict ret;

  YE_ALLOC_ENTITY(ret, DataEntity);
  yeInit((Entity *)ret, YDATA, father, name);
  ret->value = value;
  ret->destroy = NULL;
    return ((Entity *)ret);
}

Entity *yeCreateDataExt(void *value, Entity *father, const char *name,
			YDataFlag flag)
{
  DataEntity * restrict ret;

  YE_ALLOC_ENTITY(ret, DataEntity);
  yeInit((Entity *)ret, YDATA, father, name);
  if (flag == YE_DATA_USE_OWN_METADATA)
	  ret->value = yeMetadata(ret, DataEntity);
  else
	  ret->value = value;
  ret->destroy = NULL;
  return ((Entity *)ret);
}

Entity *yeCreateArrayByCStr(Entity *father, const char *name)
{
  ArrayEntity * restrict ret;

  YE_ALLOC_ENTITY(ret, ArrayEntity);
  yeInit((Entity *)ret, YARRAY, father, name);
  yBlockArrayInit(&ret->values, ArrayEntry);
  return (YE_TO_ENTITY(ret));
}

Entity *yeCreateArrayExt(Entity *father, const char *name, uint32_t flags)
{
  ArrayEntity * restrict ret;

  YE_ALLOC_ENTITY(ret, ArrayEntity);
  yeInit((Entity *)ret, YARRAY, father, name);
  yBlockArrayInitExt(&ret->values, ArrayEntry, flags);
  return (YE_TO_ENTITY(ret));
}

Entity *yeCreateArrayAt(Entity *father, const char *name, int idx)
{
  ArrayEntity *ret;

  YE_ALLOC_ENTITY(ret, ArrayEntity);
  yeInitAt((Entity *)ret, YARRAY, father, name, idx);
  yBlockArrayInit(&ret->values, ArrayEntry);
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

Entity *yeCreateFunctionExt(const char *funcName, void *manager,
			    Entity *father, const char *name, uint64_t flags)
{
  FunctionEntity *ret;

  YE_ALLOC_ENTITY(ret, FunctionEntity);
  yeInit((Entity *)ret, YFUNCTION, father, name);
  ret->value = NULL;
  ret->manager = manager;
  if (likely(manager) && !(flags & YE_FUNC_NO_FASTPATH_INIT))
    ret->fastPath = ysGetFastPath(manager, name);
  else
    ret->fastPath = NULL;
  yeSetString(YE_TO_ENTITY(ret), funcName);
  return (YE_TO_ENTITY(ret));
}

Entity *yeCreateFunction(const char *funcName, void *manager,
			 Entity *father, const char *name)
{
  return yeCreateFunctionExt(funcName, manager, father, name, 0);
}

Entity *yeCreateString(const char *string, Entity *father, const char *name)
{
  StringEntity *ret;

  YE_ALLOC_ENTITY(ret, StringEntity);
  yeInit((Entity *)ret, YSTRING, father, name);
  ret->value = NULL;
  ret->origin = NULL;
  yeSetString(YE_TO_ENTITY(ret), string);
  return (YE_TO_ENTITY(ret));
}

Entity *yeReCreateData(void *value, Entity *father, const char *name)
{
  Entity *ret = yeGet(father, name);

  if (ret) {
    yeRemoveChild(father, ret);
  }
  return yeCreateData(value, father, name);
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
  yeDestroy(ae->entity);
  arrayEntryInit(ae);
}

static void destroyChildsNoFree(Entity *entity)
{
  Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(entity)->values, ae, i, ArrayEntry) {
    arrayEntryDestroy(ae);
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
  if (entity->refCount == 1) {
    free(YE_TO_FUNC(entity)->value);
    YE_DESTROY_ENTITY(entity, FunctionEntity);
  } else {
    YE_DECR_REF(entity);
  }
}

void yeDestroyString(Entity *entity)
{
  if (entity->refCount == 1) {
    if (YE_TO_STRING(entity)->origin)
      free(YE_TO_STRING(entity)->origin);
    else
      free(YE_TO_STRING(entity)->value);
    YE_DESTROY_ENTITY(entity, StringEntity);
  } else {
    YE_DECR_REF(entity);
  }
}

void yeDestroyData(Entity *entity)
{
  if (YE_TO_DATA(entity)->value && YE_TO_DATA(entity)->destroy &&
      entity->refCount == 1)
    YE_TO_DATA(entity)->destroy(YE_TO_DATA(entity)->value);
  YE_DESTROY_ENTITY(entity, DataEntity);
}

void yeClearArray(Entity *entity)
{
  if (unlikely(!entity))
    return;
  Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(entity)->values, ae, i, ArrayEntry) {
    arrayEntryDestroy(ae);
    yBlockArrayUnset(&YE_TO_ARRAY(entity)->values, i);
  }
}

void yeDestroyArray(Entity *entity)
{
  if(entity->refCount == 1) {
    destroyChildsNoFree(entity);
    yBlockArrayFree(&YE_TO_ARRAY(entity)->values);
    YE_DESTROY_ENTITY(entity, ArrayEntity);
  } else {
    YE_DECR_REF(entity);
  }
}

void yeDestroy(Entity *entity)
{
  if (unlikely(!entity))
    return;
  destroyTab[entity->type](entity);
}

void yeMultDestroy_(Entity *toRm, ...)
{
  va_list ap;

  yeDestroy(toRm);
  va_start(ap, toRm);
 again:
  toRm = va_arg(ap, Entity *);
  if (toRm) {
    yeDestroy(toRm);
    goto again;
  }
  va_end(ap);
}


Entity *yeCreate(EntityType type, void *val, Entity *father, const char *name)
{
  switch (type)
    {
    case YSTRING:
      return yeCreateString(val, father, name);
    case YINT:
      return yeCreateInt(((size_t)val), father, name);
    case YFLOAT:
      return yeCreateFloat(((size_t)&val), father, name);
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

int	yePushBackExt(Entity *entity, Entity *toPush,
		      const char *name, int flag)
{
  return yeAttach(entity, toPush, yeLen(entity), name, flag);
}

int	yePushBack(Entity *entity, Entity *toPush, const char *name)
{
  return yePushBackExt(entity, toPush, name, 0);
}

Entity	*yeMoveByEntity(Entity* src, Entity* dest, Entity *what,
			const char *dstName)
{
  yePushBack(dest, what, dstName);
  yeRemoveChild(src, what);
  return what;
}

Entity *yeRemoveChild(Entity *array, Entity *toRemove)
{
  if (!checkType(array, YARRAY)) {
    DPRINT_ERR("bad argument 1 of type '%s', should be array\n",
	       yeTypeToString(yeType(array)));
    return NULL;
  }

  Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(array)->values, tmp, it, ArrayEntry) {
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
  if (ret->refCount == 1)
    ret = NULL;
  yeExpandArray(entity, len - 1);
  return (ret);
}

void	yeSetString(Entity *entity, const char *val)
{
  if (unlikely(!entity))
    return;
  if (YE_TO_STRING(entity)->value != NULL) {
    if (entity->type == YSTRING && YE_TO_STRING(entity)->origin != NULL)
      free(YE_TO_STRING(entity)->origin);
    else
      free(YE_TO_STRING(entity)->value);
  }
  if (val != NULL) {
    YE_TO_STRING(entity)->value = strdup(val);
    if (entity->type == YSTRING)
      YE_TO_STRING(entity)->len = strlen(val);
  } else {
    YE_TO_STRING(entity)->value = NULL;
    if (entity->type == YSTRING)
      YE_TO_STRING(entity)->len = 0;
  }
  YE_TO_STRING(entity)->origin = NULL;
}

void yeSetDestroy(Entity *entity, void (*destroyFunc)(void *))
{
  YE_TO_DATA(entity)->destroy = destroyFunc;
}

static inline void yeAttachChild(Entity *on, Entity *entity,
				const char *name)
{
  ArrayEntry *entry;

  if (!on)
    return;
  entry = yBlockArraySetGetPtr(&YE_TO_ARRAY(on)->values,
			       yeLen(on), ArrayEntry);
  entry->entity = entity;
  entry->name = g_strdup(name);
  entry->flags = 0;
  return;
}

/**
 * Set basic information to the entity <entity>
 * @param entity   the entity to set the basic informations
 * @param name     the name to set
 * @param type     the type of the entity
 * @param fathers  the parent entity of <entity>
 * @return the entity <entity>
 */
static inline Entity *yeInit(Entity *entity, EntityType type,
			     Entity *father, const char *name)
{
  if (unlikely(!entity))
    return NULL;
  entity->type = type;
  yeAttachChild(father, entity, name);
  return entity;
}

int yeAttach(Entity *on, Entity *entity,
	     unsigned int idx, const char *name, uint32_t flag)
{
  ArrayEntry *entry;

  if (unlikely(!on || !entity || on->type != YARRAY))
    return -1;

  entry = yBlockArraySetGetPtr(&YE_TO_ARRAY(on)->values,
			       idx, ArrayEntry);

  if (likely(!(YE_TO_ARRAY(on)->values.flag & YBLOCK_ARRAY_NOINIT))) {
    YE_DESTROY(entry->entity);
    g_free(entry->name);
  }
  entry->entity = entity;
  entry->name = g_strdup(name);
  entry->flags = flag;
  yeIncrRef(entity);
  return 0;
}

int yePushAt(Entity *array, Entity *toPush, int idx)
{
  return yeAttach(array, toPush, idx, NULL, 0);
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
  YE_TO_FUNC(entity)->fastPath = NULL;
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
  if (unlikely(!entity)) {
    return NULL;
  }
  return ((StringEntity *)entity)->value;
}

int	yeGetInt(Entity *entity)
{
  if (unlikely(!entity)) {
    return 0;
  }
  return YE_TO_INT(entity)->value;
}

void	*yeGetData(Entity *entity)
{
  if (unlikely(!entity)) {
    return NULL;
  }
  return YE_TO_DATA(entity)->value;
}

void	*yeGetFunctionFastPath(Entity *entity)
{
  return YE_TO_FUNC(entity)->fastPath;
}

const char	*yeGetFunction(Entity *entity)
{
  if (unlikely(yeType(entity) != YFUNCTION)) {
    return NULL;
  }
  return YE_TO_FUNC(entity)->value;
}

double	yeGetFloat(Entity *entity)
{
  if (unlikely(!entity)) {
    return 0;
  }
  return ((FloatEntity *)entity)->value;
}


static Entity *	yeCopyInternal(Entity* src, Entity* dest,
			       Entity *used, Entity *refs);

static Entity *yeCopyFindRef(const char *name, Entity *entity, void *arg)
{
  (void) name;
  if (yeGet(entity, 0) == arg)
    return yeGet(entity, 1);
  return NULL;
}

static ArrayEntity *yeCopyContainer(ArrayEntity* src, ArrayEntity* dest,
				   Entity *used, Entity *refs)
{
  if (src == NULL || dest == NULL)
    return NULL;
  Entity *tmp;

  yeClearArray(YE_TO_ENTITY(dest));
  yBlockArrayAssureBlock(&dest->values, yeLen(YE_TO_ENTITY(src)));
  Y_BLOCK_ARRAY_FOREACH_PTR(src->values, elem, it, ArrayEntry) {
    ArrayEntry *destElem;

    if (elem->entity->type == YDATA)
      continue;
    destElem  = yBlockArraySetGetPtr(&dest->values, it, ArrayEntry);

    if (!elem || !elem->entity)
      continue;

    destElem->flags = elem->flags;
    destElem->name = g_strdup(elem->name);

    if (elem->flags & YE_FLAG_NO_COPY) {
      tmp = elem->entity;
    } else {
      tmp = yeFind(refs, yeCopyFindRef, elem->entity);
    }

    if (tmp) {
      destElem->entity = tmp;
      yeIncrRef(tmp);
      continue;
    }

    if (yeDoestInclude(used, elem->entity)) {
      DPRINT_ERR("inifnit loop referance, at elem %s",
		 elem->name ? elem->name : "(null)");
      return NULL;
    }

    destElem->entity = yeCreate(elem->entity->type, 0,
				NULL, NULL);
    Entity *tmp = yeCreateArray(refs, NULL);
    yePushBack(tmp, elem->entity, NULL);
    yePushBack(tmp, destElem->entity, NULL);

    if (!yeCopyInternal(elem->entity, destElem->entity, used, refs)) {
      DPRINT_ERR("fail to copy elem %s",
		 elem->name ? elem->name : "(null)");
      return NULL;
    }
  }

  return dest;
}

static Entity*		yeCopyInternal(Entity* src, Entity* dest,
				       Entity *used, Entity *refs)
{
  const char* strVal = NULL;

  yePushBack(used, dest, NULL);
  if (src != NULL && dest != NULL
      && yeType(src) == yeType(dest)) {

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
      yeSetString(dest, strVal);
      break;
    case YARRAY:
      yeCopyContainer((ArrayEntity*)src, (ArrayEntity*)dest, used, refs);
      break;
    case YFUNCTION:
      strVal = yeGetFunction(src);
      yeSetFunction(dest, strVal);
      YE_TO_FUNC(dest)->manager = YE_TO_FUNC(src)->manager;
      break;
    case YDATA:
      return NULL;
    default:
      DPRINT_ERR("entity of type %s not handle",
		 yeTypeToString(yeType(src)));
      goto error;
    }
    return dest;
  }
 error:
  return NULL;
}

Entity*		yeCopy(Entity* src, Entity* dest)
{
  Entity *refs = yeCreateArray(NULL, NULL);
  Entity *used = yeCreateArray(NULL, NULL);
  Entity *ret = yeCopyInternal(src, dest, used, refs);

  yeMultDestroy(used, refs);
  return ret;
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
    Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(entity)->values,
			      tmp, it, ArrayEntry) {
      if (!(flag & YE_FORMAT_OPT_PRINT_ONLY_VAL_ARRAY)) {
	if (it) {
	  g_string_append(str, " | ");
	}
	if (tmp->name)
	  g_string_append_printf(str, "name: \"%s\", ", tmp->name);
	g_string_append_printf(str, "idx: " PRIint64 ", ", it);
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


void yeIncrChildsRef(Entity *array)
{
  YE_ARRAY_FOREACH(array, child) {
    if (!child)
      continue;
    yeIncrRef(child);
    if (yeType(child) == YARRAY)
      yeIncrChildsRef(child);
  }
}

int yeArrayContainEntitiesInternal(Entity *entity, ...)
{
  va_list ap;
  const char *tmp;

  va_start(ap, entity);
  while((tmp = va_arg(ap, const char *)) != NULL) {
    if (!yeArrayContainEntity(entity, tmp)) {
      va_end(ap);
      return 0;
    }
  }
  va_end(ap);
  return 1;
}

#undef YE_DECR_REF

#undef YE_DESTROY_ENTITY

#undef YE_ALLOC_ENTITY
