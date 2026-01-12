/*
**Copyright (C) 2013-2023 Matthias Gatto
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
#include	<inttypes.h>
#include	<unistd.h>

#include	"entity.h"
#include	"entity-script.h"
#include	"utils.h"
#include	"stack.h"
#include	"script.h"
#include	"game.h"

/* Globale array that store every entitys */
static STACK_CREATE(freedElems, int64);
static int isInit;
static BlockArray entitysArray;
static Entity *curentFat;
static Entity *entity0;
int fatPosition;
static inline void yeInit(Entity *entity, EntityType type,
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

NO_SIDE_EFFECT static inline int yeGetPosInBase(Entity *e)
{
	return ((intptr_t)((char *)e - (char *)entity0) % FAT_SIZE) /
		SMALL_SIZE;
}

NO_SIDE_EFFECT static inline Entity *yeGetBase(Entity *e, int pos)
{
	return YE_TO_ENTITY(((union SmallEntity *)e - pos));
}

#define YE_DESTROY_ENTITY_SMALL(entity, type)				\
	do {								\
		YE_DECR_REF(entity);					\
		if (entity->refCount < 1) {				\
			int pos = yeGetPosInBase(entity);		\
			Entity *first = yeGetBase(entity, pos);		\
			first->flag ^= fatPosToFLag[pos];		\
			if (!first->flag) {				\
				if (first == curentFat) {		\
					curentFat = NULL;		\
					fatPosition = 0;		\
				}					\
				YE_DESTROY_ENTITY__(first);		\
			}						\
		}							\
	} while (0);

#define YE_ALLOC_ENTITY_SMALL(ret, type)				\
	do {								\
		if (fatPosition == 0) {					\
			YE_ALLOC_ENTITY_BASE(ret, type);		\
			curentFat = YE_TO_ENTITY(ret);			\
			++fatPosition;					\
		} else {						\
			ret = (type *)((union SmallEntity *)curentFat + fatPosition); \
			ret->refCount = 1;				\
			ret->flag = 0;					\
			curentFat->flag |= fatPosToFLag[fatPosition];	\
			++fatPosition;					\
			if (fatPosition > 3) {				\
				fatPosition = 0;			\
				curentFat = NULL;			\
			}						\
		}							\
} while (0);

#define YE_ALLOC_ENTITY_BASE(ret, type)					\
do {									\
	ret = &(yBlockArraySetGetPtr(&entitysArray,			\
				     stack_pop(freedElems,		\
					       yBlockArrayLastPos(entitysArray) + 1), \
				     union FatEntity)->type);		\
	ygAssert(ret);							\
	ret->flag = YENTITY_SMALL_SIZE_P0;				\
	ret->refCount = 1;						\
} while (0);

#define YE_ALLOC_ENTITY_VectorEntity(ret, type) YE_ALLOC_ENTITY_BASE(ret, type)
#define YE_ALLOC_ENTITY_ArrayEntity(ret, type) YE_ALLOC_ENTITY_BASE(ret, type)
#define YE_ALLOC_ENTITY_IntEntity(ret, type) YE_ALLOC_ENTITY_SMALL(ret, type)
#define YE_ALLOC_ENTITY_DataEntity(ret, type) YE_ALLOC_ENTITY_BASE(ret, type)
#define YE_ALLOC_ENTITY_FloatEntity(ret, type) YE_ALLOC_ENTITY_SMALL(ret, type)
#define YE_ALLOC_ENTITY_FunctionEntity(ret, type) YE_ALLOC_ENTITY_BASE(ret, type)
#define YE_ALLOC_ENTITY_StringEntity(ret, type) YE_ALLOC_ENTITY_SMALL(ret, type)
#define YE_ALLOC_ENTITY_HashEntity(ret, type) YE_ALLOC_ENTITY_BASE(ret, type)
#define YE_ALLOC_ENTITY_QuadIntEntity(ret, type) YE_ALLOC_ENTITY_SMALL(ret, type)

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

NO_SIDE_EFFECT EntityType yeType(const Entity * const entity)
{
	if (likely(entity != NULL)) {
		ygAssert(entity->refCount);
		return (EntityType)entity->type;
	}
	return (EntityType)BAD_TYPE;
}


NO_SIDE_EFFECT int yeFreeEntitiesInStack(void)
{
	return freedElems.len;
}

NO_SIDE_EFFECT int yeEntitiesUsed(void)
{
	return yeEntitiesArraySize() - yeFreeEntitiesInStack();
}

NO_SIDE_EFFECT int yeEntitiesArraySize(void)
{
	return yBlockArrayLastPos(entitysArray) + 1;
}

NO_SIDE_EFFECT int yeIsPtrAnEntity(void *ptr)
{
	return ptr >= (void *)entity0 &&
		((union FatEntity *)ptr) <
		(yBlockArrayGetPtrDirect(entitysArray, 0,
					 union FatEntity) +
		 yBlockArrayLastPos(entitysArray) + 1);
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


const char * EntityTypeStrings[] = { "int", "float", "string",
	"array", "function", "data", "hash"};


/**
 * @param entity
 * @param type
 * @return 1 if entity is not null and entity's type is the same as type, 0 otherwise
 */
NO_SIDE_EFFECT static inline int	checkType(const Entity *entity, EntityType type)
{
	return (likely(entity != NULL && entity->type == type));
}

NO_SIDE_EFFECT int yeStrCaseCmp(Entity *ent, const char *str)
{
	const char *eStr = yeGetString(ent);
	if (!eStr)
		return -1;
	return strcasecmp(eStr, str);
}

NO_SIDE_EFFECT int yeStrCmp(Entity *ent1, const char *str)
{
	const char *eStr = yeGetString(ent1);
	if (!eStr)
		return -1;
	return strcmp(eStr, str);
}


NO_SIDE_EFFECT EntityType yeStringToType(const char *str)
{
	int i;

	for (i = 0; i < NBR_ENTITYTYPE; ++i)
	{
		if (yuiStrEqual(str, EntityTypeStrings[i]))
			return i;
	}
	return -1;
}

NO_SIDE_EFFECT const char *yeTypeToString(int type)
{
	return (type < 0 || type >= NBR_ENTITYTYPE)
		? ("unknow")
		: (EntityTypeStrings[type]);
}

NO_SIDE_EFFECT size_t yeLen(Entity *entity)
{
	if (unlikely(!entity))
		return 0;

	ygAssert(entity->refCount);
	if (entity->type == YARRAY) {
		return yBlockArrayLastPos(YE_TO_ARRAY(entity)->values) + 1;
	} else if (entity->type == YHASH) {
		HashEntity *hon = (void *)entity;
		khash_t(entity_hash) *h = hon->values;
		int ret = 0;
		for (khiter_t k = kh_begin(h); k != kh_end(h); ++k) {
			if (kh_exist(h, k))
			    ++ret;
		}
		return ret;
	} else if (entity->type == YVECTOR) {
		return YE_TO_VECTOR(entity)->len;
	} else if (entity->type == YSTRING) {
		return YE_TO_STRING(entity)->len;
	} else if (entity->type == YDATA) {
		return YE_TO_DATA(entity)->len;
	}
	DPRINT_ERR("yeLen on non String entity !");
	return 0;
}

NO_SIDE_EFFECT Entity *yeGetByIdx(Entity *entity, size_t index)
{
	if (unlikely(entity == NULL))
		return NULL;
	ygAssert(entity->refCount);
	if (entity->type == YHASH)
		ygDgbAbort();
	else if (entity->type == YVECTOR) {
		VectorEntity *vec = (void *)entity;
		if (index >= vec->len)
			return NULL;
		return vec->data[index];
	}
	Entity *r = yBlockArrayGet(&YE_TO_ARRAY(entity)->values,
				   index, ArrayEntry).entity;
	ygAssert(!r || yeIsPtrAnEntity(r));
	return r;
}

NO_SIDE_EFFECT Entity *yeGetLast(Entity *array)
{
	size_t l = yeLen(array);

	if (unlikely(!l))
		return NULL;
	return yeGet(array, l - 1);
}

NO_SIDE_EFFECT char *yeLastKey(Entity *array)
{
	size_t l = yeLen(array);

	if (unlikely(!l))
		return NULL;

	return yeGetKeyAt(array, l - 1);
}

/**
 * @param name  the name we will search the character '.' into
 * @return the index of the charactere '.' in name
 */
NO_SIDE_EFFECT static inline int findIdxPoint(const char *name)
{
	char *res = strchr(name, '.');
	return (res == NULL)
		? -1
		: res - name;
}

NO_SIDE_EFFECT static inline ArrayEntry *yeGetArrayEntryByStr(Entity *entity, const char *str)
{
	Y_BLOCK_ARRAY_SIZED_FOREACH_PTR(YE_TO_ARRAY(entity)->values, elem,
					it, ArrayEntry, ArrayEntry) {
		if (elem && yuiStrEqual(str, elem->name))
			return elem;
	}
	return NULL;
}

NO_SIDE_EFFECT static inline ArrayEntry *yeGetArrayEntryByIdx(Entity *entity, uint32_t i)
{
	return yBlockArrayGetPtr(&YE_TO_ARRAY(entity)->values, i, ArrayEntry);
}

NO_SIDE_EFFECT char *yeGetKeyAt(const Entity *entity, int idx)
{
	if (entity)
		return yeGetArrayEntryByIdx((Entity *)entity, idx)->name;
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
NO_SIDE_EFFECT static Entity *yeGetByIdxFastWithEnd(Entity *entity, const char *name,
				     int end)
{
	char *isNum = NULL;
	int idx;

	if (entity->type == YVECTOR)
		ygDgbAbort();
	else if (entity->type == YHASH) {
		// kh_nget_##name
		HashEntity *hon = YE_TO_HASH(entity);
		khiter_t iterator = kh_nget_entity_hash(hon->values, name, end);

		if (iterator == kh_end(hon->values))
			return NULL;
		return kh_val(hon->values, iterator);
	}
	idx = strtod(name, &isNum);
	if (isNum != name)
		return yeGet(entity, idx);
	Y_BLOCK_ARRAY_SIZED_FOREACH_PTR(YE_TO_ARRAY(entity)->values, tmp,
					it, ArrayEntry, ArrayEntry) {
		if (unlikely(!tmp || !tmp->name))
			continue;
		if (strlen(tmp->name) == (unsigned int)end &&
		    !strncmp(tmp->name, name, end))
			return tmp->entity;
	}
	return NULL;
}

NO_SIDE_EFFECT Entity *yeGetByStrFast(Entity *entity, const char *name)
{
	if (unlikely(!name || !entity))
		return NULL;
	ygAssert(entity->refCount);
	if (yeType(entity) == YARRAY) {

		Y_BLOCK_ARRAY_SIZED_FOREACH_PTR(YE_TO_ARRAY(entity)->values, tmp,
						it, ArrayEntry, ArrayEntry) {
			if (unlikely(!tmp || !tmp->name))
				continue;
			if (yuiStrEqual(tmp->name, name))
				return tmp->entity;
		}
	} else if (yeType(entity) == YHASH) {
		HashEntity *hon = YE_TO_HASH(entity);
		khiter_t iterator = kh_get(entity_hash, hon->values, name);
		if (iterator == kh_end(hon->values))
			return NULL;
		return kh_val(hon->values, iterator);
	}
	return NULL;
}

Entity *yeGetByStrExt(Entity *entity, const char *name, int64_t *idx)
{
	if (unlikely(!entity || !name || yeType(entity) != YARRAY))
		return NULL;

	ygAssert(entity->refCount);

	Y_BLOCK_ARRAY_SIZED_FOREACH_PTR(YE_TO_ARRAY(entity)->values, tmp,
					it, ArrayEntry, ArrayEntry) {
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

NO_SIDE_EFFECT Entity *yeNGetByStr(Entity *entity, const char *name, int len)
{
	int cur = 0;
	Entity *ret = NULL;

	if (unlikely(!entity || !name || (entity->type != YARRAY && entity->type != YHASH))) {
		DPRINT_INFO("can not find entity for %s\n", name);
		return NULL;
	}
	ygAssert(entity->refCount);

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

NO_SIDE_EFFECT Entity *yeGetByStr(Entity *entity, const char *name)
{
	int	i;

	if (unlikely(!entity || !name || (entity->type != YARRAY && entity->type != YHASH))) {
		DPRINT_INFO("can not find entity for %s\n", name);
		return NULL;
	}
	ygAssert(entity->refCount);

	i = findIdxPoint(name);
	if (i == -1) {
		char *isNum = NULL;
		int idx;

		idx = strtol(name, &isNum, 0);
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

NO_SIDE_EFFECT int yeArrayIdx_str(Entity *entity, const char *lookup)
{
	if (unlikely(!entity || !lookup || yeType(entity) != YARRAY))
		return -1;

	ygAssert(entity->refCount);

	Y_BLOCK_ARRAY_SIZED_FOREACH_PTR(YE_TO_ARRAY(entity)->values, tmp,
					it, ArrayEntry, ArrayEntry) {
		if (unlikely(!tmp))
			continue;
		if (yuiStrEqual0(tmp->name, lookup))
			return it;
	}
	return -1;
}

Entity *yeCreateQuadIntAt(int i0, int i1, int i2, int i3, Entity *mother, const char *name, int idx)
{
	QuadIntEntity * restrict ret;

	YE_ALLOC_ENTITY(ret, QuadIntEntity);
	yeInitAt((Entity *)ret, YQUADINT, mother, name, idx);
	ret->x = i0;
	ret->y = i1;
	ret->w = i2;
	ret->h = i3;
	return ((Entity *)ret);
}

Entity *yeCreateQuadInt(int i0, int i1, int i2, int i3, Entity *parent, const char *name)
{
	QuadIntEntity * restrict ret;

	YE_ALLOC_ENTITY(ret, QuadIntEntity);
	yeInit((Entity *)ret, YQUADINT, parent, name);
	ret->x = i0;
	ret->y = i1;
	ret->w = i2;
	ret->h = i3;
	return ((Entity *)ret);
}

Entity *yeCreateQuadInt2(int i0, int i1, Entity *parent, const char *name)
{
	return yeCreateQuadInt(i0, i1, 0, 0, parent, name);
}

Entity *yeCreateQuadInt0(Entity *parent, const char *name)
{
	return yeCreateQuadInt(0, 0, 0, 0, parent, name);
}

Entity *yeCreateLong(int64_t value, Entity *father, const char *name)
{
	IntEntity * restrict ret;

	YE_ALLOC_ENTITY(ret, IntEntity);
	yeInit((Entity *)ret, YINT, father, name);
	ret->lval = value;
	return ((Entity *)ret);
}

Entity *yeCreateLongAt(int64_t value, Entity *father, const char *name, long idx)
{
	IntEntity * restrict ret;

	YE_ALLOC_ENTITY(ret, IntEntity);
	yeInitAt((Entity *)ret, YINT, father, name, idx);
	ret->lval = value;
	return ((Entity *)ret);
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

Entity *yeCreateHash(Entity *father, const char *name)
{
	HashEntity * restrict ret;

	YE_ALLOC_ENTITY(ret, HashEntity);
	yeInit((Entity *)ret, YHASH, father, name);
	ret->values = kh_init(entity_hash);
	return (YE_TO_ENTITY(ret));
}

Entity *yeReCreateContainer(Entity *father, const char *name,
			    Entity *child,
			    Entity *(*maker)(Entity *parent, const char *name))
{
	if (!father || !name)
		return maker(father, NULL);
	int type = yeType(father);
	int child_need_rm = 0;
	Entity *ret;

	if (type == YARRAY) {
		Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(father)->values, tmp,
					  it, ArrayEntry) {
			if (tmp && yuiStrEqual0(tmp->name, name)) {
				if (child) {
					YE_INCR_REF(child);
				} else {
					child = maker(NULL, NULL);
				}
				YE_DESTROY(tmp->entity);
				tmp->entity = child;
				return child;
			}
		}
	} else if (type == YHASH) {
		if (child) {
			YE_INCR_REF(child);
			child_need_rm = 1;
		}
		yeRemoveChild(father, name);
	} else if (type == YVECTOR) {
		if (child && yeDoesInclude(father, child)) {
			return child;
		}
	}
	if (child) {
		yePushBack(father, child, name) < 0 ? NULL : child;
		ret = child;
	} else {
		ret = maker(father, name);
	}
	if (child_need_rm) {
		YE_DESTROY(child);
	}
	return ret;
}

Entity *yeCreateVector(Entity *father, const char *name)
{
	VectorEntity * restrict ret;

	YE_ALLOC_ENTITY(ret, VectorEntity);
	yeInit((Entity *)ret, YVECTOR, father, name);
	ret->data = (void *)yeMetadata(ret, VectorEntity);
	ret->len = 0;
	ret->max = yeMetadataSize(VectorEntity) / sizeof(Entity *);
	return (YE_TO_ENTITY(ret));
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

Entity *yeStealString(char *string, Entity *father,
		      const char *name)
{
	StringEntity *ret;

	YE_ALLOC_ENTITY(ret, StringEntity);
	yeInit((Entity *)ret, YSTRING, father, name);
	ret->value = string;
	ret->origin = NULL;
	ret->len = strlen(string);
	return (YE_TO_ENTITY(ret));
}

Entity *yeCreateString(const char *string, Entity *father, const char *name)
{
	StringEntity *ret;

	unsigned int len = string ? strlen(string) : 0;

	if (string && len < yeMetadataSize(StringEntity)) {
		YE_ALLOC_ENTITY_BASE(ret, StringEntity);
		yeInit((Entity *)ret, YSTRING, father, name);
		char *dst = (char *)yeMetadata(ret, StringEntity);

		ret->len = len;
		ret->origin = NULL;
		ret->value = dst;
		memcpy(dst, string, len + 1);
	} else {
		YE_ALLOC_ENTITY(ret, StringEntity);
		yeInit((Entity *)ret, YSTRING, father, name);
		ret->value = NULL;
		ret->origin = NULL;
		yeSetString(YE_TO_ENTITY(ret), string);
	}
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
	free(ae->name);
	yeDestroy(ae->entity);
	arrayEntryInit(ae);
}

static void destroyChildsNoFree(Entity *entity)
{
	BlockArray *ba = &YE_TO_ARRAY(entity)->values;
	uint16_t flag = ba->flag;
	ba->flag = YBLOCK_ARRAY_NOMIDFREE;
	Y_BLOCK_ARRAY_SIZED_FOREACH_PTR(*ba, ae, i, ArrayEntry, ArrayEntry) {
		yBlockArrayUnset(ba, i);
		arrayEntryDestroy(ae);
	}
	ba->flag = flag;
}

void yeDestroyInt(Entity *entity)
{
	YE_DESTROY_ENTITY(entity, IntEntity);
}

void yeDestroyQuadInt(Entity *entity)
{
	YE_DESTROY_ENTITY(entity, QuadIntEntity);
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
		free(yeStringFreeable(entity));
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
	switch (entity->type) {
	case YARRAY:
	{
		Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(entity)->values, ae,
					  i, ArrayEntry) {
			arrayEntryDestroy(ae);
		}
		yBlockArrayClear(&YE_TO_ARRAY(entity)->values);
	}
	break;
	case YHASH:
	{
		Entity *vvar;
		const char *kkey;

		kh_foreach(((HashEntity *)entity)->values,
			   kkey, vvar, {free((char *)kkey);yeDestroy(vvar);});
		kh_clear(entity_hash, ((HashEntity *)entity)->values);
	}
	break;
	case YVECTOR:
	{
		VectorEntity *vec = (void *)entity;
		for (int i = 0; i < vec->len; ++i) {
			yeDestroy(vec->data[i]);
		}
		vec->len = 0;
	}
	break;
	default:
		DPRINT_ERR("yeClearArray with wrong type");
	}
}

void yeDestroyHash(Entity *entity)
{
	Entity *vvar;
	const char *kkey;

	if(entity->refCount == 1) {
		kh_foreach(((HashEntity *)entity)->values,
			   kkey, vvar, {free((char *)kkey); yeDestroy(vvar);});
		kh_destroy(entity_hash, ((HashEntity *)entity)->values);
		YE_DESTROY_ENTITY(entity, ArrayEntity);
	} else {
		YE_DECR_REF(entity);
	}
}

void yeDestroyVector(Entity *entity)
{
	if(entity->refCount == 1) {
		VectorEntity *vec = (void *)entity;
		yeClearArray(entity);
		if (vec->max > yeMetadataSize(VectorEntity) / sizeof(Entity *)) {
			free(vec->data);
		}
	} else {
		YE_DECR_REF(entity);
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
	switch (entity->type) {
	case YINT:
		return yeDestroyInt(entity);
	case YSTRING:
		return yeDestroyString(entity);
	case YFLOAT:
		return yeDestroyFloat(entity);
	case YFUNCTION:
		return yeDestroyFunction(entity);
	case YARRAY:
		return yeDestroyArray(entity);
	case YDATA:
		return yeDestroyData(entity);
	case YHASH:
		return yeDestroyHash(entity);
	case YVECTOR:
		return yeDestroyVector(entity);
	case YQUADINT:
		return yeDestroyQuadInt(entity);
	}
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
		return yeCreateFloat(((size_t)val), father, name);
	case YARRAY:
		return yeCreateArray(father, name);
	case YDATA:
		return yeCreateData(val, father, name);
	case YFUNCTION:
		return yeCreateFunction(val, NULL, father, name);
	case YHASH:
		return yeCreateHash(father, name);
	case YVECTOR:
		return yeCreateVector(father, name);
	case YQUADINT:
		return yeCreateQuadInt((size_t)val, (size_t)val,
				       (size_t)val, (size_t)val, father, name);
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


static ArrayEntity *manageArrayInternal(ArrayEntity *entity,
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

Entity *yeRemoveChildByIdx(Entity *array, int toRemove)
{
	if (array->type == YVECTOR) {
		VectorEntity *vec = (void *)array;
		int l = vec->len;
		if (toRemove >= l)
			return NULL;
		yeDestroy(vec->data[toRemove]);
		if  (toRemove == l - 1)
			return NULL;
		vec->data[toRemove] = vec->data[--l];
		return NULL;
	}
	BlockArray *ba = &YE_TO_ARRAY(array)->values;
	uint16_t flag = ba->flag;
	ArrayEntry *tmp = yeGetArrayEntryByIdx(array, toRemove);
	Entity *ret = NULL;

	ba->flag = YBLOCK_ARRAY_NOMIDFREE;
	if (tmp && (ret = tmp->entity) != NULL) {
		yBlockArrayUnset(ba, toRemove);
		arrayEntryDestroy(tmp);
	}
	ba->flag = flag;
	return ret;
}

Entity *yeRemoveChildByEntity(Entity *array, Entity *toRemove)
{
	Entity *ret;

	if (array == NULL) {
		DPRINT_ERR("bad argument 1 of type '%s', should not be NULL\n",
			   yeTypeToString(yeType(array)));
		ygDgbAbort();
		return NULL;
	} else if (array->type == YHASH) {
		HashEntity *hon = (void *)array;
		khash_t(entity_hash) *h = hon->values;
		khiter_t k;

		for (k = kh_begin(h); k != kh_end(h); ++k) {
			if (kh_exist(h, k) && kh_value(h, k) == toRemove) {
				Entity *to_destroy = kh_val(h, k);

				yeDestroy(to_destroy);
				kh_del(entity_hash, h, k);
				return to_destroy;
			}
		}
		return NULL;
	} else if (array->type == YVECTOR) {
		VectorEntity *vec = (void *)array;
		int l = vec->len;

		for (int i = 0; i < l; ++i) {
			if (toRemove == vec->data[i]) {
				yeDestroy(toRemove);
				if (i == l - 1)
					return NULL;
				vec->data[i] = vec->data[--l];
			}
		}
		return NULL;
	} else if (array->type != YARRAY) {
		DPRINT_ERR("bad argument 1 of type '%s', should be array/hash\n",
			   yeTypeToString(yeType(array)));
		ygDgbAbort();
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

void yeUnsetFirst(Entity *array)
{
	if (!checkType(array, YARRAY)) {
		DPRINT_ERR("bad argument 1 of type '%s', should be array\n",
			   yeTypeToString(yeType(array)));
		return;
	}

	BlockArray *ba = &YE_TO_ARRAY(array)->values;
	uint16_t flag = ba->flag;

	ba->flag = YBLOCK_ARRAY_NOMIDFREE;

	Y_BLOCK_ARRAY_FOREACH_PTR(*ba, tmp, it, ArrayEntry) {
		yBlockArrayUnset(ba, it);
		arrayEntryDestroy(tmp);
		goto exit;
	}

exit:
	ba->flag = flag;
}

Entity *yeRemoveChildByStr(Entity *array, const char *toRemove)
{
	Entity *ret;

	if (array == NULL) {
		DPRINT_ERR("bad argument 1 of type '%s', should not be NULL\n",
			   yeTypeToString(yeType(array)));
		ygDgbAbort();
		return NULL;
	} else if (array->type == YHASH) {
		HashEntity *hon = (void *)array;
		khiter_t iterator = kh_get(entity_hash, hon->values, toRemove);

		if (iterator == kh_end(hon->values))
			return NULL;

		Entity *to_destroy = kh_val(hon->values, iterator);

		yeDestroy(to_destroy);
		kh_del(entity_hash, hon->values, iterator);
	} else if (array->type != YARRAY) {
		DPRINT_ERR("bad argument 1 of type '%s', should be array/hash\n",
			   yeTypeToString(yeType(array)));
		ygDgbAbort();
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

_Bool yeEraseByE(Entity *array, Entity *t)
{
	BlockArray *ba = &YE_TO_ARRAY(array)->values;
	uint16_t flag = ba->flag;
	ArrayEntry *old = NULL;

	ba->flag = YBLOCK_ARRAY_NOMIDFREE;

	Y_BLOCK_ARRAY_FOREACH_PTR(*ba, tmp, it, ArrayEntry) {
		if (old) {
			old->entity = tmp->entity;
			old->name = tmp->name;
			old = tmp;
		} else if (tmp->entity == t) {
			arrayEntryDestroy(tmp);
			old = tmp;
		}
	}

	if (old) {
		arrayEntryInit(old);
		yBlockArrayUnset(ba, yeLen(array) - 1);
	}
	ba->flag = flag;
	return !!old;
}

Entity *yePopBack(Entity *entity)
{
	int len = yeLen(entity);
	Entity *ret;

	if (yeIsVector(entity)) {
		VectorEntity *vec = (void *)entity;
		vec->len--;
		yeDestroy(vec->data[vec->len]);
		return NULL;
	}
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
	free(yeStringFreeable(e));
	if (str != NULL) {
		char *tmp_val = strndup(str, n);
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

_Bool yeStringIsValueAllocated(Entity *e)
{
	if (YE_TO_STRING(e)->origin) {
		return YE_TO_STRING(e)->origin !=
			yeMetadata(e, StringEntity);
	}
	return YE_TO_STRING(e)->value != yeMetadata(e, StringEntity);
}

char *yeStringFreeable(Entity *e)
{
	if (unlikely(yeType(e) == YFUNCTION))
		return YE_TO_FUNC(e)->value;
	if (!yeStringIsValueAllocated(e))
		return NULL;
	if (YE_TO_STRING(e)->origin)
		return YE_TO_STRING(e)->origin;
	return YE_TO_STRING(e)->value;
}

Entity	*yeSetString(Entity *entity, const char *val)
{
	if (unlikely(!entity))
		return NULL;
	ygAssert(!(entity->flag & YENTITY_CONST));
	free(yeStringFreeable(entity));
	if (val != NULL) {
		YE_TO_STRING(entity)->value = yuiStrdup(val);
		if (entity->type == YSTRING)
			YE_TO_STRING(entity)->len = strlen(val);
	} else {
		YE_TO_STRING(entity)->value = NULL;
		if (entity->type == YSTRING)
			YE_TO_STRING(entity)->len = 0;
	}
	if (entity->type == YSTRING)
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
	if (on->type == YHASH) {
		if (unlikely(!name)) {
			DPRINT_ERR("name require if parent is an Hash");
			ygDgbAbort();
		}
		if (unlikely(yePushBack(on, entity, name) < 0)) {
			DPRINT_ERR("pushing '%s' on parent fail", name);
			ygDgbAbort();
		}
		yeDestroy(entity);
		return;
	} else if (on->type == YVECTOR) {
		if (unlikely(name)) {
			DPRINT_ERR("name can not be use in Vector");
			ygDgbAbort();
		}
		if (unlikely(yePushBack(on, entity, NULL) < 0)) {
			DPRINT_ERR("pushing '%s' on parent fail", name);
			ygDgbAbort();
		}
		yeDestroy(entity);
		return;
	}
	ygAssert(on->refCount);
	entry = yBlockArraySetGetPtr(&YE_TO_ARRAY(on)->values,
				     yeLen(on), ArrayEntry);
	entry->entity = entity;
	entry->name = yuiStrdup(name);
	entry->flags = 0;
	return;
}

/**
 * Set basic information to the entity <entity>
 * @param entity   the entity to set the basic informations
 * @param name     the name to set
 * @param type     the type of the entity
 * @param fathers  the parent entity of <entity>
 */
static inline void yeInit(Entity *entity, EntityType type,
			     Entity *father, const char *name)
{
	ygAssert(entity);
	entity->type = type;
	yeAttachChild(father, entity, name);
}

Entity *yeCreateCopy2(Entity *src, Entity *father, const char *name, _Bool just_ref)
{
	Entity *ret;

	switch (yeType(src)) {
	case YINT:
		return yeCreateInt(yeGetIntDirect(src), father, name);
	case YSTRING:
		return yeCreateString(yeGetString(src), father, name);
	case YFLOAT:
		return yeCreateFloat(yeGetFloatDirect(src), father, name);
	case YQUADINT:
		return yeCreateQuadInt(yeGetQuadInt0(src),
				       yeGetQuadInt1(src),
				       yeGetQuadInt2(src),
				       yeGetQuadInt3(src),
				       father, name);
	case YARRAY:
	case YFUNCTION:
		ret = src->type == YARRAY ?
			yeCreateArray(father, name) :
		yeCreateFunction(NULL, NULL, father, name);
		if (src->type == YARRAY && just_ref) {
			for (size_t i = 0; i < yeLen(src); ++i) {
				char *key = yeGetKeyAt(src, i);

				yePushBack(ret, yeGet(src, i), key);
			}

		} else {
			if (!yeCopy(src, ret)) {
				if (father)
					yeRemoveChild(father, ret);
				else
					yeDestroy(ret);
				return NULL;
			}
		}
		break;
	case YHASH:
		ret = yeCreateHash(father, name);
		{
			Entity *vvar;
			const char *kkey;

			kh_foreach(((HashEntity *)src)->values,
				   kkey, vvar, {
					   if (just_ref) {
						   yePushBack(ret, vvar, kkey);
					   } else {
						   yeCreateCopy2(vvar, ret, kkey, 0);
					   }
				   });
		}
		break;
	case YVECTOR:
		ret = yeCreateVector(father, name);
		{
			VectorEntity *vec = (void *)src;

			for (int i = 0; i < vec->len; ++i) {
				if (just_ref) {
					yePushBack(ret, vec->data[i], NULL);
				} else {
					yeCreateCopy2(vec->data[i], ret, NULL, 0);
				}
			}
		}
		break;
	default:
		return NULL;
	}
	return ret;
}

Entity *yeCreateCopy(Entity *src, Entity *father, const char *name)
{
	return yeCreateCopy2(src, father, name, 0);
}

int yeAttach(Entity *on, Entity *entity,
	     unsigned int idx, const char *name, uint64_t flag)
{
	ArrayEntry *entry;
	Entity *toRemove = NULL;
	char *oldName = NULL;

	if (unlikely(!on || !entity))
		return -1;

	ygAssert((int)idx >= 0);

	union {
		struct {
			uint32_t entry_flag;
			uint32_t attach_flag;
		};
		uint64_t f;
	} f;
	f.f = flag;
	if (on->type == YHASH) {
		HashEntity *hon = (void *)on;
		int ret;

		if (!name)
			return -1;
		khiter_t iterator = kh_get(entity_hash, hon->values, name);

		if (iterator != kh_end(hon->values)) {
			toRemove = kh_val(hon->values, iterator);
			if (toRemove == entity)
				return 0;
			if (toRemove && !(flag & YE_ATTACH_NO_MEM_FREE)) {
				yeDestroy(toRemove);
			}
		}
		iterator = kh_put(entity_hash, hon->values, strdup(name), &ret);
		if (ret < 0)
			return -1;
		if (!(flag & YE_ATTACH_NO_INC_REF))
			yeIncrRef(entity);
		kh_val(hon->values, iterator) = entity;
		return 0;
	} else if (on->type == YVECTOR) {
		VectorEntity *vec = (void *)on;

		if (idx > vec->len) {
			DPRINT_ERR("YVector doesn't allow for sparce elems\n");
			ygDgbAbort();
		} else if (idx == vec->max) {
			if (vec->max == yeMetadataSize(VectorEntity) / sizeof(Entity *)) {
				Entity **anew = malloc(128 * sizeof(Entity *));
				if (!anew)
					ygDgbAbort();
				memcpy(anew, vec->data, yeMetadataSize(VectorEntity));
				vec->max = 128;
				vec->data = anew;
			} else {
				vec->max = vec->max << 1;
				vec->data = realloc(vec->data, vec->max * sizeof(Entity *));
				ygAssert(vec->data);
			}
			vec->data[idx] = entity;
		} else {
			if (vec->data[idx] != entity) {
				if (idx != vec->len)
					yeDestroy(vec->data[idx]);
				vec->data[idx] = entity;
			}
		}
		++vec->len;
		if (!(flag & YE_ATTACH_NO_INC_REF))
			yeIncrRef(entity);
		return 0;
	} else if (on->type != YARRAY)
		return -1;
	ygAssert(!(on->flag & YENTITY_CONST));

	entry = yBlockArraySetGetPtr(&YE_TO_ARRAY(on)->values,
				     idx, ArrayEntry);

	if (likely(!(YE_TO_ARRAY(on)->values.flag & YBLOCK_ARRAY_NOINIT))) {
		if (entry->entity == entity)
			return 0;
		toRemove = entry->entity;
		oldName = entry->name;
		ygAssert(!toRemove || yeIsPtrAnEntity(toRemove));
		ygAssert(!toRemove || toRemove->refCount);
	}
	entry->entity = entity;
	if (flag & YE_ATTACH_STEAL_NAME)
		entry->name = (char *)name;
	else
		entry->name = yuiStrdup(name);
	entry->flags = f.entry_flag;
	if (!(flag & YE_ATTACH_NO_INC_REF))
		yeIncrRef(entity);
	if (toRemove && !(flag & YE_ATTACH_NO_MEM_FREE)) {
		YE_DESTROY(toRemove);
		free(oldName);
	}
	return 0;
}

int yePushAt(Entity *array, Entity *toPush, int idx)
{
	return yeAttach(array, toPush, idx, NULL, 0);
}

int yePushAt2(Entity *array, Entity *toPush, int idx, const char *name)
{
	return yeAttach(array, toPush, idx, name, 0);
}

int yePush(Entity array[static 1], Entity toPush[static 1], const char *name)
{
	if (array->type == YHASH) {
		return yePushBack(array, toPush, name);
	}
	BlockArray *ba = &YE_TO_ARRAY(array)->values;
	uint64_t m;

	ygAssert(array->refCount)
	ygAssert(toPush->refCount)

	for (int i = 0; i < ba->block_cnt; ++i) {
		int j;
		ArrayEntry *e;

		m = ~ba->blocks[i];
		if (!m)
			continue;
		j = ctz64(m);
		e = yBlockArraySetGetPtr(ba, i * 64 + j, ArrayEntry);
		e->entity = toPush;
		yeIncrRef(toPush);
		e->name = yuiStrdup(name);
		e->flags = 0;
		return i * 64 + j;
	}
	if (yePushBack(array, toPush, name) < 0)
		return -1;
	return yeLen(array) - 1;
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
	if (yeType(entity) == YQUADINT) {
		switch(index) {
		case 0:
			YE_TO_QINT(entity)->x = value;
			break;
		case 1:
			YE_TO_QINT(entity)->y = value;
			break;
		case 2:
			YE_TO_QINT(entity)->w = value;
			break;
		case 3:
			YE_TO_QINT(entity)->h = value;
			break;
		default:
			DPRINT_ERR("can't get elem '%d', Quand int contain only 4 int", index);
		}
		return;
	}
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
	if (yeType(entity) == YQUADINT) {
		switch(index[0]) {
		case 'x':
			YE_TO_QINT(entity)->x = value;
			break;
		case 'y':
			YE_TO_QINT(entity)->y = value;
			break;
		case 'w':
			YE_TO_QINT(entity)->w = value;
			break;
		case 'h':
			YE_TO_QINT(entity)->h = value;
			break;
		default:
			DPRINT_ERR("can't get elem '%s' in quad", index);
		}
		return;
	}
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
	ygAssert(!(entity->flag & YENTITY_CONST));
	((IntEntity *)entity)->value = value;
}

void	yeSetLong(Entity *entity, int64_t value)
{
	if (unlikely(!entity || (yeType(entity) != YINT &&
				 yeType(entity) != YFLOAT)))
		return;
	ygAssert(!(entity->flag & YENTITY_CONST));
	((IntEntity *)entity)->lval = value;
}

void	yeSetFloat(Entity *entity, double value)
{
	if (unlikely(!entity))
		return;
	ygAssert(!(entity->flag & YENTITY_CONST));
	((FloatEntity *)entity)->value = value;
}

NO_SIDE_EFFECT const char *yeGetString(Entity *entity)
{
	if (unlikely(!entity)) {
		return NULL;
	}
	return ((StringEntity *)entity)->value;
}

NO_SIDE_EFFECT int	yeGetInt(Entity *entity)
{
	if (unlikely(!entity)) {
		return 0;
	} else if (yeType(entity) == YFLOAT) {
		return (int64_t)yeGetFloatDirect(entity);
	}
	return YE_TO_INT(entity)->value;
}

NO_SIDE_EFFECT int64_t	yeGetLong(Entity *entity)
{
	if (unlikely(!entity)) {
		return 0;
	} else if (yeType(entity) == YFLOAT) {
		return (int64_t)yeGetFloatDirect(entity);
	}
	return YE_TO_INT(entity)->lval;
}

NO_SIDE_EFFECT void	*yeGetData(Entity *entity)
{
	if (unlikely(!entity)) {
		return NULL;
	}
	return YE_TO_DATA(entity)->value;
}

NO_SIDE_EFFECT void	*yeGetFunctionFastPath(Entity *entity)
{
	return YE_TO_FUNC(entity)->fastPath;
}

NO_SIDE_EFFECT const char	*yeGetFunction(Entity *entity)
{
	if (unlikely(yeType(entity) != YFUNCTION)) {
		return NULL;
	}
	return YE_TO_FUNC(entity)->value;
}

NO_SIDE_EFFECT double	yeGetFloat(Entity *entity)
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
		destElem->name = yuiStrdup(elem->name);

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
		case YQUADINT:
			yeSetAt(dest, 0, yeGetQuadInt0(src));
			yeSetAt(dest, 1, yeGetQuadInt1(src));
			yeSetAt(dest, 2, yeGetQuadInt2(src));
			yeSetAt(dest, 3, yeGetQuadInt3(src));
			break;
		case YARRAY:
			yeCopyContainer((ArrayEntity*)src,
					(ArrayEntity*)dest, used, refs);
			break;
		case YHASH:
			yeClearArray(dest);
			{
				Entity *vvar;
				const char *kkey;

				kh_foreach(((HashEntity *)src)->values,
					   kkey, vvar, {
						   if (yeDoesInclude(used, vvar)) {
							   DPRINT_ERR("inifnit loop referance, at elem %s", kkey);
							   return NULL;
						   }

						   yeCreateCopy2(vvar, dest, kkey, 0);
					   });
			}

			break;
		case YVECTOR:
			yeClearArray(dest);
			{
				VectorEntity *vec = (void *)src;

				for (int i = 0; i < vec->len; ++i) {
					yeCreateCopy2(vec->data[i], dest, NULL, 0);
				}
			}
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

static void append_pretty(Entity *str, int deep, int origDeep, int flag)
{
	if (!(flag & YE_FORMAT_PRETTY))
		return;
	yeStringAdd(str, "\n");
	for (int i = 0; i < origDeep - deep; ++i)
		yeStringAddCh(str, '\t');
}

static void yeToCStrInternal(Entity *entity, int deep, Entity *str,
			     int flag, int origDeep, Entity *ignore_keys)
{
	if (!deep)
		return;
	switch (yeType(entity)) {
	case YSTRING :
		if (deep == origDeep) {
			yeStringAdd(str, yeGetString(entity));
		} else {
			yeStringAppendPrintf(str, "\"%s\"",
					     yeGetString(entity));
		}
		break;
	case YINT :
		yeStringAppendPrintf(str, "'"PRIint64"'",
				     yeGetIntDirect(entity));
		break;
	case YQUADINT :
		yeStringAppendPrintf(str, "<q: %d, %d, %d, %d>",
				     yeGetIntAt(entity, 0),
				     yeGetIntAt(entity, 1),
				     yeGetIntAt(entity, 2),
				     yeGetIntAt(entity, 3)
			);
		break;
	case YFLOAT :
		yeStringAppendPrintf(str, "'%f'", yeGetFloatDirect(entity));
		break;
	case YFUNCTION :
		yeStringAppendPrintf(str, "(%s - %p/%"PRIxPTR")()",
				     yeGetFunction(entity),
				     yeGetFunctionFastPath(entity),
				     yeIData(entity)
			);
		break;
	case YHASH :
		yeStringAddCh(str, '{');
		if (yeLen(entity) > 20)
			flag |= YE_FORMAT_OPT_PRINT_ONLY_VAL_ARRAY;
		{
			const char *key;
			Entity *vvar;
			int i = 0;

			kh_foreach(((HashEntity *)entity)->values, key,
				   vvar,
				   {
					   if (i++) {
						   if (flag & YE_FORMAT_PRETTY) {
							   yeStringAdd(str, ",");
							   append_pretty(str, deep,
									 origDeep, flag);
						   } else {
							   yeStringAdd(str, ", ");
						   }
					   }

					   yeStringAppendPrintf(str,
								"\"%s\": ",
								key);
					   if (yeIsContainer(vvar))
						   append_pretty(str, deep - 1, origDeep, flag);
				   if (!(deep - 1)) {
					   yeStringAdd(str, "...");
				   } else {
					   if (yeFindString(ignore_keys, key)) {
						   yeStringAdd(str, "IGNORED");
					   } else {
						   yeToCStrInternal(vvar, deep - 1, str,
								    flag, origDeep,
								    ignore_keys);
					   }
				   }
				   });
		}
		yeStringAddCh(str, '}');
		if (flag & YE_FORMAT_OPT_BREAK_ARRAY_END)
			yeStringAddCh(str, '\n');

		break;
	case YVECTOR :
		{
			VectorEntity *vec = (void *)entity;
			yeStringAddCh(str, '[');
			for (int i = 0; i < vec->len; ++i) {
				Entity *tmp = vec->data[i];

				if (!(flag & YE_FORMAT_OPT_PRINT_ONLY_VAL_ARRAY)) {
					if (flag & YE_FORMAT_PRETTY) {
						append_pretty(str, deep,
							      origDeep, flag);
					} else {
						yeStringAdd(str, ", ");
					}
				}
				if (yeIsContainer(tmp))
					append_pretty(str, deep - 1, origDeep, flag);
				if (!(deep - 1)) {
					yeStringAdd(str, "...");
				} else {
					yeToCStrInternal(tmp, deep - 1, str,
							 flag, origDeep, ignore_keys);
				}

			}
			yeStringAddCh(str, ']');
			if (flag & YE_FORMAT_OPT_BREAK_ARRAY_END)
				yeStringAddCh(str, '\n');
			break;
		}
	case YARRAY :
		yeStringAddCh(str, '[');
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
						yeStringAdd(str, " | ");
					}
				}
				if (tmp->name) {
					yeStringAppendPrintf(str,
							     "\"%s\"",
							     tmp->name);
				}
				yeStringAppendPrintf(str, "[" PRIint64
						     "] : ", it);
			}
			if (yeIsContainer(tmp->entity))
				append_pretty(str, deep - 1, origDeep, flag);
			if (!(deep - 1)) {
				yeStringAdd(str, "...");
			} else if (tmp->name && yeFindString(ignore_keys, tmp->name)) {
				yeStringAdd(str, "IGNORED");
			} else {
				yeToCStrInternal(tmp->entity, deep - 1, str,
						 flag, origDeep, ignore_keys);
			}
		}

		yeStringAddCh(str, ']');
		if (flag & YE_FORMAT_OPT_BREAK_ARRAY_END)
			yeStringAddCh(str, '\n');
		break;
	case YDATA :
		{
			void *d = yeGetData(entity);
			yeStringAppendPrintf(str, "'%p'", d);
			break;
		}
	default :
		break;
	}
}

char *yeToCStr2(Entity *entity, int deep, int flag, Entity *ignore_keys)
{
	if (!entity)
		return NULL;
	YE_NEW(String, str, "");
	char *ret;

	if (flag & YE_FORMAT_PRETTY && !YE_FORMAT_NO_NL)
		yeStringAddCh(str, '\n');
	yeToCStrInternal(entity, deep, str, flag, deep, ignore_keys);
	ret = strdup(yeGetString(str));
	return ret;
}

char *yeToCStr(Entity *entity, int deep, int flag)
{
	return yeToCStr2(entity, deep, flag, NULL);
}

int yeRenameStrStr(Entity *array, const char *old_name, const char *new_name)
{
	YE_ARRAY_FOREACH_ENTRY(array, ae) {
		if (!ae)
			continue;
		if (yuiStrEqual0(ae->name, old_name)) {
			free(ae->name);
			ae->name = yuiStrdup(new_name);
		}
	}
	return 0;
}

int yeRenameIdxStr(Entity *array, int idx, const char *str)
{
	ArrayEntry *ae = yeGetArrayEntryByIdx(array, idx);

	if (!ae)
		return -1;
	free(ae->name);
	ae->name = yuiStrdup(str);
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

int yeDumbSort(Entity *a, Entity *func, int start)
{
	int have_change;

	do {
		int last = yeLen(a) - 1;
		have_change = 0;

		for (int i = start; i < last; ++i) {
			int i2 = i + 1;

			int cmp = (intptr_t)yesCall(func, yeGet(a, i),
						    yeGet(a, i2));
			if (cmp > 0) {
				yeSwapByIdx(a, i, i2);
				have_change = 1;
			}
		}
	} while (have_change);
	return 0;
}

static void yeQuickSort_pivot(Entity *array, Entity *func, int start, int last)
{
	if (last <= start)
		return;
	int pivot = start + (last - start) / 2;

	if (pivot <= start)
		return;

	Entity *pivot_ent = yeGet(array, pivot);
	yeSwapByIdx(array, pivot, last);
	int pivot_left = start - 1;
	int pivot_right = start - 1;
	do
	{
		++pivot_right;
		int cmp = (intptr_t)yesCall(func, yeGet(array, pivot_right), pivot_ent);
		if (cmp <= 0) {
			++pivot_left;
			if (pivot_left == pivot_right)
				continue;
			int cmp = (intptr_t)yesCall(func, yeGet(array, pivot_left), yeGet(array, pivot_right));
			if (cmp > 0) {
				yeSwapByIdx(array, pivot_right, pivot_left);
			}
		}
	} while (pivot_right + 1 < last);
	yeSwapByIdx(array, last, pivot_left + 1);
	yeQuickSort_pivot(array, func, start, pivot);
	yeQuickSort_pivot(array, func, pivot, last);
}

int yeQuickSort(Entity *array, Entity *func, int start)
{
	int last = yeLen(array) - 1;

	start = start < 1 ? 0 : start;

	yeQuickSort_pivot(array, func, start, last);
	return 0;
}

int yeShuffle(Entity *array)
{
	if (!array || yeType(array) != YARRAY)
		return -1;

	int array_l = yeLen(array);
	for (int i = 0; i < array_l; ++i) {
		int b = yuiRand() % array_l;

		if (i == b)
			continue;

		yeSwapByIdx(array, i, b);
	}

	for (int i = 0; i < array_l; ++i) {
		int a = yuiRand() % array_l;
		int b = yuiRand() % array_l;

		if (a == b)
			continue;
		yeSwapByIdx(array, a, b);
	}
	return 0;
}


#undef YE_DECR_REF

#undef YE_DESTROY_ENTITY

#undef YE_ALLOC_ENTITY
