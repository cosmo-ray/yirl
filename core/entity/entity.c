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
static Entity *curentFat;
static Entity *entity0;
int fatPosition;
static inline Entity *yeInit(Entity *entity, EntityType type,
			     Entity *father, const char *name);

uint8_t fatPosToFLag[4] = {YENTITY_SMALL_SIZE_P0, YENTITY_SMALL_SIZE_P1,
			   YENTITY_SMALL_SIZE_P2, YENTITY_SMALL_SIZE_P3};

#define YE_DECR_REF(entity) do {		\
		entity->refCount -= 1;		\
	} while (0)

#define YE_DESTROY_ENTITY__(entity)					\
	int64_t unset =							\
		(size_t)(((union FatEntity *)entity)			\
			 - yBlockArrayGetPtrDirect(entitysArray, 0,	\
						   union FatEntity));	\
	yBlockArrayUnset(&entitysArray, unset);				\
	stack_push(freedElems, unset);					\

#define YE_DESTROY_ENTITY_SMALL(entity, type)				\
do {								\
	YE_DECR_REF(entity);						\
	if (entity->refCount < 1) {					\
		int pos = ((intptr_t)((char *)entity - (char *)entity0) % 128) / 32; \
		Entity *first = YE_TO_ENTITY(((union SmallEntity *)entity - pos)); \
		first->flag ^= fatPosToFLag[pos];			\
		if (!first->flag) {					\
			if (first == curentFat) {			\
				curentFat = NULL;			\
				fatPosition = 0;			\
			}						\
			YE_DESTROY_ENTITY__(first);			\
		}							\
	}								\
} while (0);

#define YE_ALLOC_ENTITY_SMALL(ret, type)				\
do {								\
	if (fatPosition == 0) {						\
		YE_ALLOC_ENTITY_BASE(ret, type);			\
		curentFat = YE_TO_ENTITY(ret);				\
		++fatPosition;						\
	} else {							\
		ret = (type *)((union SmallEntity *)curentFat + fatPosition); \
		ret->refCount = 1;					\
		curentFat->flag |= fatPosToFLag[fatPosition];		\
		++fatPosition;						\
		if (fatPosition > 3) {					\
			fatPosition = 0;				\
			curentFat = NULL;				\
		}							\
	}								\
} while (0);

#define YE_ALLOC_ENTITY_BASE(ret, type)					\
do {									\
	ret = &(yBlockArraySetGetPtr(&entitysArray,			\
				     stack_pop(freedElems,		\
					       yBlockArrayLastPos(entitysArray) + 1), \
				     union FatEntity)->type);		\
	ret->flag = YENTITY_SMALL_SIZE_P0;				\
	ret->refCount = 1;						\
} while (0);

#define YE_ALLOC_ENTITY_ArrayEntity(ret, type) YE_ALLOC_ENTITY_BASE(ret, type)
#define YE_ALLOC_ENTITY_IntEntity(ret, type) YE_ALLOC_ENTITY_SMALL(ret, type)
#define YE_ALLOC_ENTITY_DataEntity(ret, type) YE_ALLOC_ENTITY_BASE(ret, type)
#define YE_ALLOC_ENTITY_FloatEntity(ret, type) YE_ALLOC_ENTITY_SMALL(ret, type)
#define YE_ALLOC_ENTITY_FunctionEntity(ret, type) YE_ALLOC_ENTITY_BASE(ret, type)
#define YE_ALLOC_ENTITY_StringEntity(ret, type) YE_ALLOC_ENTITY_BASE(ret, type)

#define YE_ALLOC_ENTITY(ret, type) YUI_CAT(YE_ALLOC_ENTITY_, type)(ret, type)
#define YE_DESTROY_ENTITY(entity, type) YE_DESTROY_ENTITY_SMALL(entity, type)

void yeInitMem(void)
{
	if (!isInit) {
		yBlockArrayInitExt(&entitysArray, union FatEntity,
				   YBLOCK_ARRAY_BIG_CHUNK | YBLOCK_ARRAY_NOINIT |
				   YBLOCK_ARRAY_NOMIDFREE);
		entity0 = yBlockArrayGetPtrDirect(entitysArray, 0, Entity);
		isInit = 1;
		yBlockArrayDataNextSize0 = yeMetadataSize(ArrayEntity);
	}
}

int yeFreeEntitiesInStack(void)
{
	return freedElems.len;
}

int yeEntitiesUsed(void)
{
	return yeEntitiesArraySize() - yeFreeEntitiesInStack();
}

int yeEntitiesArraySize(void)
{
	return yBlockArrayLastPos(entitysArray) + 1;
}

int yeIsPtrAnEntity(void *ptr)
{
	return ((Entity *)ptr) >= entity0 &&
		((union FatEntity *)ptr) <=
		(yBlockArrayGetPtrDirect(entitysArray, 0,
					 union FatEntity) +
		 yBlockArrayLastPos(entitysArray));
}

Entity *yeCreateInts_(Entity *fathers, int nbVars, ...)
{
	va_list ap;

	if (!fathers)
		fathers = yeCreateArray(NULL, NULL);
	va_start(ap, nbVars);
	for (int i = 0; i < nbVars; ++i) {
		uint64_t ii = va_arg(ap, uint64_t);
		yeCreateInt(ii, fathers, NULL);
	}
	va_end(ap);
	return fathers;
}

void yeEnd(void)
{
	isInit = 0;
	fatPosition = 0;
	curentFat = NULL;
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

int yeStrCaseCmp(Entity *ent, const char *str)
{
	const char *eStr = yeGetString(ent);
	if (!eStr)
		return -1;
	return strcasecmp(eStr, str);
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
	assert(entity->refCount);
	return yBlockArrayGet(&YE_TO_ARRAY(entity)->values,
			      index, ArrayEntry).entity;
}

Entity *yeGetLast(Entity *array)
{
	size_t l = yeLen(array);

	if (unlikely(!l))
		return NULL;
	return yeGet(array, l - 1);
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
	Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(entity)->values, elem,
				  it, ArrayEntry) {
		if (elem && yuiStrEqual(str, elem->name))
			return elem;
	}
	return NULL;
}

static inline ArrayEntry *yeGetArrayEntryByIdx(Entity *entity, uint32_t i)
{
	return yBlockArrayGetPtr(&YE_TO_ARRAY(entity)->values, i, ArrayEntry);
}

char *yeGetKeyAt(Entity *entity, int idx)
{
	if (entity)
		return yeGetArrayEntryByIdx(entity, idx)->name;
	return NULL;
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
static Entity *yeGetByIdxFastWithEnd(Entity *entity, const char *name,
				     int end)
{
	char *isNum = NULL;
	int idx;

	idx = strtod(name, &isNum);
	if (isNum != name)
		return yeGet(entity, idx);
	Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(entity)->values, tmp,
				  it, ArrayEntry) {
		if (unlikely(!tmp || !tmp->name))
			continue;
		if (strlen(tmp->name) == (unsigned int)end &&
		    !strncmp(tmp->name, name, end))
			return tmp->entity;
	}
	return NULL;
}

Entity *yeGetByStrFast(Entity *entity, const char *name)
{
	if (unlikely(!entity || !name || yeType(entity) != YARRAY))
		return NULL;
	assert(entity->refCount);

	Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(entity)->values, tmp,
				  it, ArrayEntry) {
		if (unlikely(!tmp || !tmp->name))
			continue;
		if (yuiStrEqual(tmp->name, name))
			return tmp->entity;
	}
	return NULL;
}

Entity *yeGetByStrExt(Entity *entity, const char *name, int64_t *idx)
{
	if (unlikely(!entity || !name || yeType(entity) != YARRAY))
		return NULL;

	assert(entity->refCount);

	Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(entity)->values, tmp,
				  it, ArrayEntry) {
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

	if (unlikely(!entity || !name || yeType(entity) != YARRAY)) {
		DPRINT_INFO("can not find entity for %s\n", name);
		return NULL;
	}
	assert(entity->refCount);

	while (cur < len) {
		int i;

		i = findIdxPoint(name);
		if (i == -1)
			return yeGetByIdxFastWithEnd(entity, name, len - cur);

		ret = yeGetByIdxFastWithEnd(entity, name, i);
		name += i + 1;
		cur += i + 1;
		entity = ret;
	}

	return ret;
}

Entity *yeGetByStr(Entity *entity, const char *name)
{
	int	i;

	if (unlikely(!entity || !name || yeType(entity) != YARRAY)) {
		DPRINT_INFO("can not find entity for %s\n", name);
		return NULL;
	}
	assert(entity->refCount);

	i = findIdxPoint(name);
	if (i == -1) {
		char *isNum = NULL;
		int idx;

		idx = strtod(name, &isNum);
		if (isNum != name)
			return yeGet(entity, idx);
		return yeGet(entity, name);
	}
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

int yeArrayIdx_str(Entity *entity, const char *lookup)
{
	if (unlikely(!entity || !lookup || yeType(entity) != YARRAY))
		return -1;

	assert(entity->refCount);

	Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(entity)->values, tmp,
				  it, ArrayEntry) {
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


Entity *yeCreateDataAt(void *value, Entity *father, const char *name, int idx)
{
	DataEntity * restrict ret;

	YE_ALLOC_ENTITY(ret, DataEntity);
	yeInitAt((Entity *)ret, YDATA, father, name, idx);
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
	yBlockArrayInitExt(&ret->values, ArrayEntry, YBLOCK_ARRAY_NO_BLOCKS_NEXT0);
	return (YE_TO_ENTITY(ret));
}

Entity *yeCreateArrayExt(Entity *father, const char *name, uint32_t flags)
{
	ArrayEntity * restrict ret;

	YE_ALLOC_ENTITY(ret, ArrayEntity);
	yeInit((Entity *)ret, YARRAY, father, name);
	yBlockArrayInitExt(&ret->values, ArrayEntry,
			   flags | YBLOCK_ARRAY_NO_BLOCKS_NEXT0);
	return (YE_TO_ENTITY(ret));
}

Entity *yeCreateArrayAt(Entity *father, const char *name, int idx)
{
	ArrayEntity *ret;

	YE_ALLOC_ENTITY(ret, ArrayEntity);
	yeInitAt((Entity *)ret, YARRAY, father, name, idx);
	yBlockArrayInitExt(&ret->values, ArrayEntry, YBLOCK_ARRAY_NO_BLOCKS_NEXT0);
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

Entity *yeCreateFloatAt(double value, Entity *father, const char *name, int idx)
{
	FloatEntity * restrict ret;

	YE_ALLOC_ENTITY(ret, FloatEntity);
	yeInitAt((Entity *)ret, YFLOAT, father, name, idx);
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
		ret->fastPath = ysGetFastPath(manager, funcName);
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

Entity *yeCreateStringAt(const char *string, Entity *father,
			 const char *name, int idx)
{
	StringEntity *ret;

	YE_ALLOC_ENTITY(ret, StringEntity);
	yeInitAt((Entity *)ret, YSTRING, father, name, idx);
	ret->value = NULL;
	ret->origin = NULL;
	yeSetString(YE_TO_ENTITY(ret), string);
	return (YE_TO_ENTITY(ret));
}


Entity *yeCreateNString(const char *string, int n, Entity *father,
			const char *name)
{
	StringEntity *ret;

	YE_ALLOC_ENTITY(ret, StringEntity);
	yeInit((Entity *)ret, YSTRING, father, name);
	ret->value = NULL;
	ret->origin = NULL;
	yeSetNString(YE_TO_ENTITY(ret), string, n);
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
	BlockArray *ba = &YE_TO_ARRAY(entity)->values;
	uint16_t flag = ba->flag;
	ba->flag = YBLOCK_ARRAY_NOMIDFREE;
	Y_BLOCK_ARRAY_FOREACH_PTR(*ba, ae, i, ArrayEntry) {
		yBlockArrayUnset(ba, i);
		arrayEntryDestroy(ae);
	}
	ba->flag = flag;
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
		ysEDestroy(YE_TO_FUNC(entity)->manager, entity);
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
	Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(entity)->values, ae,
				  i, ArrayEntry) {
		arrayEntryDestroy(ae);
	}
	yBlockArrayClear(&YE_TO_ARRAY(entity)->values);
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
			arrayEntryDestroy(&yBlockArrayGet(&entity->values,
							  i, ArrayEntry));
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
		      const char *name, uint64_t flag)
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

Entity *yeRemoveChildByEntity(Entity *array, Entity *toRemove)
{
	Entity *ret;

	if (!checkType(array, YARRAY)) {
		DPRINT_ERR("bad argument 1 of type '%s', should be array\n",
			   yeTypeToString(yeType(array)));
		return NULL;
	}

	BlockArray *ba = &YE_TO_ARRAY(array)->values;
	uint16_t flag = ba->flag;

	ba->flag = YBLOCK_ARRAY_NOMIDFREE;

	Y_BLOCK_ARRAY_FOREACH_PTR(*ba, tmp, it, ArrayEntry) {

		tmp = yeGetArrayEntryByIdx(array, it);
		ret = tmp->entity;
		if (ret == toRemove) {
			yBlockArrayUnset(ba, it);
			arrayEntryDestroy(tmp);
			goto exit;
		}
	}
	ret = NULL;

exit:
	ba->flag = flag;
	return ret;
}

Entity *yeRemoveChildByStr(Entity *array, const char *toRemove)
{
	Entity *ret;

	if (!checkType(array, YARRAY)) {
		DPRINT_ERR("bad argument 1 of type '%s', should be array\n",
			   yeTypeToString(yeType(array)));
		return NULL;
	}

	BlockArray *ba = &YE_TO_ARRAY(array)->values;
	uint16_t flag = ba->flag;

	ba->flag = YBLOCK_ARRAY_NOMIDFREE;

	Y_BLOCK_ARRAY_FOREACH_PTR(*ba, tmp, it, ArrayEntry) {

		tmp = yeGetArrayEntryByIdx(array, it);
		if (yuiStrEqual0(tmp->name, toRemove)) {
			ret = tmp->entity;
			yBlockArrayUnset(ba, it);
			arrayEntryDestroy(tmp);
			goto exit;
		}
	}
	ret = NULL;

exit:
	ba->flag = flag;
	return ret;
}

Entity *yePopBack(Entity *entity)
{
	int len = yeLen(entity);
	Entity *ret;

	if (unlikely(!checkType(entity, YARRAY) || !len)) {
		return NULL;
	}
	ret = yeGet(entity, len - 1);
	if (ret->refCount == 1)
		ret = NULL;
	yeExpandArray(entity, len - 1);
	return (ret);
}

void	yeSetNString(Entity *e, const char *str, size_t n)
{
	if (unlikely(!e))
		return;
	if (YE_TO_STRING(e)->value != NULL) {
		if (e->type == YSTRING && YE_TO_STRING(e)->origin != NULL)
			free(YE_TO_STRING(e)->origin);
		else
			free(YE_TO_STRING(e)->value);
	}
	if (str != NULL) {
		char *tmp_val = g_strndup(str, n);
		YE_TO_STRING(e)->value = tmp_val;
		if (e->type == YSTRING)
			YE_TO_STRING(e)->len = strlen(tmp_val);
	} else {
		YE_TO_STRING(e)->value = NULL;
		if (e->type == YSTRING)
			YE_TO_STRING(e)->len = 0;
	}
	YE_TO_STRING(e)->origin = NULL;
}

Entity	*yeSetString(Entity *entity, const char *val)
{
	if (unlikely(!entity))
		return NULL;
	if (YE_TO_STRING(entity)->value != NULL) {
		if (entity->type == YSTRING && YE_TO_STRING(entity)->origin)
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
	return entity;
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

Entity *yeCreateCopy(Entity *src, Entity *father, const char *name)
{
	Entity *ret;

	switch (yeType(src)) {
	case YINT:
		return yeCreateInt(yeGetIntDirect(src), father, name);
	case YSTRING:
		return yeCreateString(yeGetString(src), father, name);
	case YFLOAT:
		return yeCreateFloat(yeGetFloatDirect(src), father, name);
	case YARRAY:
	case YFUNCTION:
		ret = yeType(src) == YARRAY ?
			yeCreateArray(father, name) :
		yeCreateFunction(NULL, NULL, father, name);
		if (!yeCopy(src, ret)) {
			if (father)
				yeRemoveChild(father, ret);
			else
				yeDestroy(ret);
			return NULL;
		}
		break;
	default:
		return NULL;
	}
	return ret;
}


int yeAttach(Entity *on, Entity *entity,
	     unsigned int idx, const char *name, uint64_t flag)
{
  ArrayEntry *entry;
  Entity *toRemove = NULL;
  char *oldName = NULL;
  union {
	  struct {
		  uint32_t entry_flag;
		  uint32_t attach_flag;
	  };
	  uint64_t f;
  } f;
  f.f = flag;

  if (unlikely(!on || !entity || on->type != YARRAY))
	  return -1;

  entry = yBlockArraySetGetPtr(&YE_TO_ARRAY(on)->values,
			       idx, ArrayEntry);

  if (likely(!(YE_TO_ARRAY(on)->values.flag & YBLOCK_ARRAY_NOINIT))) {
	  if (entry->entity == entity)
		  return 0;
	  toRemove = entry->entity;
	  oldName = entry->name;
  }
  entry->entity = entity;
  if (flag & YE_ATTACH_STEAL_NAME)
	  entry->name = (char *)name;
  else
	  entry->name = g_strdup(name);
  entry->flags = f.entry_flag;
  if (!(flag & YE_ATTACH_NO_INC_REF))
	  yeIncrRef(entity);
  if (toRemove && !(flag & YE_ATTACH_NO_MEM_FREE)) {
	  YE_DESTROY(toRemove);
	  g_free(oldName);
  }
  return 0;
}

int yePushAt(Entity *array, Entity *toPush, int idx)
{
	return yeAttach(array, toPush, idx, NULL, 0);
}

int yePush(Entity *array, Entity *toPush, const char *name)
{
	BlockArray *ba = &YE_TO_ARRAY(array)->values;
	uint64_t m;

	for (int i = 0; i < ba->nbBlock; ++i) {
		int j;
		ArrayEntry *e;

		m = ~ba->blocks[i];
		if (!m)
			continue;
		j = ctz64(m);
		e = yBlockArraySetGetPtr(ba, i * 64 + j, ArrayEntry);
		e->entity = toPush;
		yeIncrRef(toPush);
		e->name = g_strdup(name);
		e->flags = 0;
		return 0;
	}
	return yePushBack(array, toPush, name);
}

int yeInsertAt(Entity *array, Entity *toPush, size_t idx, const char *name)
{
	uint64_t attachFlag = 0;

	if (yeLen(array) < idx || !yeGet(array, idx)) {
		yeAttach(array, toPush, idx, name, 0);
		return 0;
	}

	for (size_t i = idx; i < yeLen(array); ++i) {
		Entity *tmpToPush = toPush;
		const char *tmpName = name;

		toPush = yeGet(array, i);
		name = yeGetKeyAt(array, i);
		yeAttach(array, tmpToPush, i, tmpName,
			 attachFlag | YE_ATTACH_NO_MEM_FREE);
		if (!toPush)
			return 0;

		if (!attachFlag)
			attachFlag = YE_ATTACH_NO_INC_REF | YE_ATTACH_STEAL_NAME;
	}
	yePushBackExt(array, toPush, name, attachFlag);
	return 0;
}

void	yeSetStringAt(Entity *entity, unsigned int index, const char *value)
{
	yeSetString(yeGet(entity, index), value);
}

void	yeSetIntAt(Entity *entity, unsigned int index, int value)
{
	yeSetInt(yeGet(entity, index), value);
}

void	yeSetFloatAt(Entity *entity, unsigned int index, double value)
{
	yeSetFloat(yeGet(entity, index), value);
}

void	yeSetStringAtStrIdx(Entity *entity, const char *index, const char *value)
{
	yeSetString(yeGet(entity, index), value);
}

void	yeSetIntAtStrIdx(Entity *entity, const char *index, int value)
{
	yeSetInt(yeGet(entity, index), value);
}

void	yeSetFloatAtStrIdx(Entity *entity, const char *index, double value)
{
	yeSetFloat(yeGet(entity, index), value);
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
	yeSetString(entity, value);
}

void	yeSetInt(Entity *entity, int value)
{
	if (unlikely(!entity || (yeType(entity) != YINT &&
				 yeType(entity) != YFLOAT)))
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
	} else if (yeType(entity) == YFLOAT) {
		return (int64_t)yeGetFloatDirect(entity);
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

		if (yeDoesInclude(used, elem->entity)) {
			DPRINT_ERR("inifnit loop referance, at elem %s",
				   elem->name ? elem->name : "(null)");
			return NULL;
		}

		destElem->entity = yeCreate(elem->entity->type, 0,
					    NULL, NULL);
		Entity *tmp = yeCreateArray(refs, NULL);
		yePushBack(tmp, elem->entity, NULL);
		yePushBack(tmp, destElem->entity, NULL);

		if (!yeCopyInternal(elem->entity, destElem->entity,
				    used, refs)) {
			DPRINT_ERR("fail to copy elem %s",
				   elem->name ? elem->name : "(null)");
			return NULL;
		}
	}

	return dest;
}

static Entity *yeCopyInternal(Entity* src, Entity* dest,
			      Entity *used, Entity *refs)
{
	const char* strVal = NULL;

	yePushBack(used, dest, NULL);
	if (src != NULL && dest != NULL
	    && yeType(src) == yeType(dest)) {

		switch (yeType(src))
		{
		case YINT:
			yeSetInt(dest, yeGetIntDirect(src));
			break;
		case YFLOAT:
			yeSetFloat(dest, yeGetFloatDirect(src));
			break;
		case YSTRING:
			strVal = yeGetString(src);
			yeSetString(dest, strVal);
			break;
		case YARRAY:
			yeCopyContainer((ArrayEntity*)src,
					(ArrayEntity*)dest, used, refs);
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

Entity	*yeCopy(Entity* src, Entity* dest)
{
	Entity *refs = yeCreateArray(NULL, NULL);
	Entity *used = yeCreateArray(NULL, NULL);
	Entity *ret = yeCopyInternal(src, dest, used, refs);

	yeMultDestroy(used, refs);
	return ret;
}

static void append_pretty(GString *str, int deep, int origDeep, int flag)
{
	if (!(flag & YE_FORMAT_PRETTY))
		return;
	g_string_append(str, "\n");
	for (int i = 0; i < origDeep - deep; ++i)
		g_string_append_c(str, '\t');
}


static void yeToCStrInternal(Entity *entity, int deep, GString *str,
			     int flag, int origDeep)
{
	if (!deep)
		return;
	switch (yeType(entity)) {
	case YSTRING :
		if (deep == origDeep) {
			g_string_append_printf(str, "%s", yeGetString(entity));
		} else {
			g_string_append_printf(str, "\"%s\"",
					       yeGetString(entity));
		}
		break;
	case YINT :
		g_string_append_printf(str, "'%d'",
				       (uint32_t)yeGetIntDirect(entity));
		break;
	case YFLOAT :
		g_string_append_printf(str, "'%f'", yeGetFloatDirect(entity));
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
					if (flag & YE_FORMAT_PRETTY) {
						append_pretty(str, deep,
							      origDeep, flag);
					} else {
						g_string_append(str, " | ");
					}
				}
				if (tmp->name) {
					g_string_append_printf(str,
							       "\"%s\"",
							       tmp->name);
				}
				g_string_append_printf(str, "[" PRIint64
						       "] : ", it);
			}
			if (yeType(tmp->entity) == YARRAY)
				append_pretty(str, deep - 1, origDeep, flag);
			if (!(deep - 1)) {
				g_string_append(str, "...");
			} else {
				yeToCStrInternal(tmp->entity, deep - 1, str,
						 flag, origDeep);
			}
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

	if (flag & YE_FORMAT_PRETTY && !YE_FORMAT_NO_NL)
		g_string_append_c(str, '\n');
	yeToCStrInternal(entity, deep, str, flag, deep);
	return g_string_free(str, 0);
}

int yeRenameIdxStr(Entity *array, int idx, const char *str)
{
	ArrayEntry *ae = yeGetArrayEntryByIdx(array, idx);

	if (!ae)
		return -1;
	g_free(ae->name);
	ae->name = strdup(str);
	return 0;
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

int yeIsPureArray(Entity *e)
{
	if (yeType(e) != YARRAY)
		return 0;

	YE_ARRAY_FOREACH_ENTRY(e, entry) {
		if (entry->name)
			return 0;
	}
	return 1;
}

#undef YE_DECR_REF

#undef YE_DESTROY_ENTITY

#undef YE_ALLOC_ENTITY
