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

/*
 * This file contain the entity API.
 * Entities are just some basic type that are used everywhere in the engine
 * they can either be int,string,float,function,data or array
 *
 * father/mother and parent are the same thing, when creating an entity
 * this entity can be place inside an array directly, in this case the
 * parent is the array containing it.
 *
 * I've first start to use father instead of parent because of some bias
 * I can't explain, it was a mistake, so I'm presently fixig it using this rule:
 * father should be randomly keep or replace by either mother and parent
 * Why use 3 definition for the same thing ? why not ?
 */

#ifndef	_YIRL_ENTITY_H
#define	_YIRL_ENTITY_H

#include "entity-macro.h"
#include "block-array.h"
#include "utils.h"

#ifdef Y_INSIDE_TCC
#define kn_no_include
#endif

#include "klib/khash.h"

#define NONNULL(arg) __attribute__ ((nonnull (arg)))

typedef enum
{
	YE_FIND_MONE = 0,
	YE_FIND_LINK_NO_GET = 1,
	YE_FIND_LINK_NO_DEEP = 2
} YeFindFlag;

/* All entity type, each is defined later inside a structure */
typedef enum
{
	BAD_TYPE = -1,
	YINT = 0,
	YFLOAT = 1,
	YSTRING = 2,
	YARRAY = 3,
	YFUNCTION = 4,
	YDATA = 5,
	YHASH = 6,
	YQUADINT = 7,
	NBR_ENTITYTYPE = 8
} EntityType;

typedef enum
{
	YENTITY_SMALL_SIZE_P0 = 1 << 0,
	YENTITY_SMALL_SIZE_P1 = 1 << 1,
	YENTITY_SMALL_SIZE_P2 = 1 << 2,
	YENTITY_SMALL_SIZE_P3 = 1 << 3,
	YENTITY_CONST = 1 << 4
} EntityFlag;

#define YENTITY_FLAG_LAST = YENTITY_SMALL_SIZE_P3;

/* yeAttach flags */
#define YE_ATTACH_NO_MEM_FREE (1LLU << 32)
#define YE_ATTACH_NO_INC_REF (1LLU << 33)
#define YE_ATTACH_STEAL_NAME (1LLU << 34)


/* yeToCStr falgs */
#define YE_FORMAT_OPT_BREAK_ARRAY_END 1
#define YE_FORMAT_OPT_PRINT_ONLY_VAL_ARRAY 2
#define YE_FORMAT_PRETTY 4
#define YE_FORMAT_NO_NL 8

#define	YE_TO_ENTITY(X) ((Entity *)X)
#define	YE_TO_C_ENTITY(X) ((const Entity *)X)
#define	YE_TO_INT(X) ((IntEntity *)X)
#define	YE_TO_C_INT(X) ((const IntEntity *)X)
#define	YE_TO_FLOAT(X) ((FloatEntity *)X)
#define	YE_TO_C_FLOAT(X) ((const FloatEntity *)X)
#define	YE_TO_STRING(X) ((StringEntity *)X)
#define	YE_TO_C_STRING(X) ((const StringEntity *)X)
#define	YE_TO_ARRAY(X) ((ArrayEntity *)X)
#define	YE_TO_C_ARRAY(X) ((const ArrayEntity *)X)
#define	YE_TO_FUNC(X) ((FunctionEntity *)X)
#define	YE_TO_C_FUNC(X) ((const FunctionEntity *)X)
#define	YE_TO_DATA(X) ((DataEntity *)X)
#define	YE_TO_C_DATA(X) ((const DataEntity *)X)
#define YE_TO_HASH(X) ((HashEntity *)X)
#define YE_TO_QINT(X) ((QuadIntEntity *)X)

  /* TODO: move most of this code to yeDestroy, remove this */
#define YE_DESTROY(X) do {			\
		if (X == NULL)			\
			break;			\
		if (X->refCount == 1) {		\
			yeDestroy(X);		\
			X = NULL;		\
		} else {			\
			yeDestroy(X);		\
		}				\
	} while (0);


#define YE_INCR_REF(entity) ({YE_TO_ENTITY(entity)->refCount += 1; entity ;})

#define YE_NULLIFY(e)				\
	({ yeDestroy(e); e = NULL; })

typedef enum {
	YE_FLAG_NULL = 0,
	YE_FLAG_NO_COPY = 1
} ArrayEntryFlags;

typedef enum {
	YE_FUNC_NONE = 0,
	YE_FUNC_NO_FASTPATH_INIT = 1
} FunctionFlags;

typedef enum {
	YE_DATA_NONE = 0,
	YE_DATA_USE_OWN_METADATA = 1,
} YDataFlag;


#define	ENTITY_HEADER				\
	uint32_t refCount;			\
	uint16_t type;				\
	uint16_t flag;				\

typedef struct Entity_
{
	ENTITY_HEADER
} Entity;

typedef	struct {
	Entity *entity;
	char *name;
	uint32_t flags;
} ArrayEntry;

typedef	struct
{
	ENTITY_HEADER

	BlockArray values;
} ArrayEntity;

typedef	struct
{
	ENTITY_HEADER

	int_ptr_t	value;
} IntEntity;

typedef	struct
{
	ENTITY_HEADER

	int	x;
	int	y;
	int	w;
	int	h;
} QuadIntEntity;

typedef	struct
{
	ENTITY_HEADER

	double	value;
} FloatEntity;

typedef	struct
{
	ENTITY_HEADER

	char	*value;
	unsigned int len;
	char *origin;
} StringEntity;

typedef	struct
{
	ENTITY_HEADER

	void	*value;
	void	(*destroy)(void *);
} DataEntity;

KHASH_MAP_INIT_STR(entity_hash, Entity *);

KHASH_IMPLEMENT_STR_N_GET(entity_hash);

typedef	struct
{
	ENTITY_HEADER
	khash_t(entity_hash) *values;
} HashEntity;

typedef	struct
{
	ENTITY_HEADER

	/* Name of the function */
	char	*value;
	/* A pointer to the coresponding script manager */
	void	*manager;
	/*
	 * some data use by the script manager to call a function faster
	 * it can also handle lambda and local functions,
	 * which can't be call using a global name
	 * This is initialyse to 0
	 */
	union {
		void	*fastPath;
		intptr_t idata;
	};
} FunctionEntity;

union SmallEntity {
	Entity Entity;
	IntEntity IntEntity;
	FloatEntity FloatEntity;
	StringEntity StringEntity;
	QuadIntEntity QuadIntEntity;
	uint8_t totalSize[32];
};

#define SMALL_SIZE sizeof(union SmallEntity)

union FatEntity {
	Entity Entity;
	ArrayEntity ArrayEntity;
	IntEntity IntEntity;
	FloatEntity FloatEntity;
	StringEntity StringEntity;
	DataEntity DataEntity;
	QuadIntEntity QuadIntEntity;
	FunctionEntity FunctionEntity;
	HashEntity HashEntity;
	union SmallEntity SnallEntities[4];
	uint8_t totalSize[128];
};

#define FAT_SIZE sizeof(union FatEntity)

#ifdef __cplusplus
#include "entity-cplusplus.h"
#endif

static inline Entity *yeIncrRef(Entity *e)
{
	if (unlikely(!e))
		return NULL;
	return YE_INCR_REF(e);
}

NO_SIDE_EFFECT static inline int yeRefCount(Entity *e)
{
	if (likely(e))
		return e->refCount;
	return 0;
}

#define yeMetadata(Entity, EntityType)			\
	(((char *)Entity) + sizeof(EntityType))

#define yeMetadataSize(EntityType)			\
	(sizeof(union FatEntity) - sizeof(EntityType))

_Bool yeStringIsValueAllocated(Entity *e);
char *yeStringFreeable(Entity *e);

void yeInitMem(void);

void yeEnd(void);

NO_SIDE_EFFECT int yeIsPtrAnEntity(void *ptr);

NO_SIDE_EFFECT int yeEntitiesArraySize(void);

NO_SIDE_EFFECT int yeEntitiesUsed(void);

NO_SIDE_EFFECT int yeFreeEntitiesInStack(void);

NO_SIDE_EFFECT EntityType yeType(const Entity * const entity);

NO_SIDE_EFFECT static inline int yeIsNum(const Entity * const e)
{
	return yeType(e) == YINT || yeType(e) == YFLOAT;
}

/**
 * @return 1 if e is an array that doen't contain any named key
 */
int yeIsPureArray(Entity *e);

/**
 * @param str   the type name
 * @return the corresponding type, -1 if type not found
 */
NO_SIDE_EFFECT EntityType yeStringToType(const char *str);

/**
 * @param type
 * @return the corresponding string of the type
 */
NO_SIDE_EFFECT const char *yeTypeToString(int type);

NO_SIDE_EFFECT static inline const char *yeTypeAsString(Entity *e)
{
	return yeTypeToString(yeType(e));
}

/**
 * Get the len attribute of an Entity
 * @param entity  The Entity we want to get the len
 * @return the attribute len of the entity
 */
NO_SIDE_EFFECT size_t yeLen(Entity *entity);

#define yeLenAt(e, at)				\
	yeLen(yeGet(e, (at)))

/**
 * @brief convert @entity to @type
 * Will try to be smarter than brutal cast to have javascripts like convertion
 * NOTE: this function isn't complet yet, and support only convertion:
 * From int to int
 * From int to float
 * From float to float
 * From float to int
 * From string to string
 * From string to  array
 * @return NULL if fail, @entity otherwise
 */
Entity *yeConvert(Entity *entity, int type);

/**
 * @brief cast @entity to @type
 * in comparaison to yeConvert, there is no logic here, as example
 * a convertion from a string to an int, will set the string adresse
 * as the int value
 * @return NULL if fail, @entity otherwise
 */
Entity *yeBrutalCast(Entity *entity, int type);

#define YE_ARRAY_FOREACH_INIT(array)					\
	(array == NULL ? yBlockArrayIteratorCreate(NULL, 0) :		\
	 yBlockArrayIteratorCreate(&YE_TO_ARRAY(array)->values, 0))

#define YE_ARRAY_FOREACH_SET_VAL(it, val)				\
	((val = yBlockArrayIteratorGetPtr(it, ArrayEntry)->entity) || 1)

#define YE_ARRAY_FOREACH_EXT(array, val, it)		\
	Entity *val;					\
	for (BlockArrayIterator it =			\
		     YE_ARRAY_FOREACH_INIT(array);	\
	     !yBlockArrayIteratorIsEnd(&it) &&		\
		     YE_ARRAY_FOREACH_SET_VAL(it, val);	\
	     yBlockArrayIteratorIncr(&it))

#define YE_ARRAY_FOREACH_ENTRY(array, val)				\
	ArrayEntry *val;						\
	for (BlockArrayIterator it =					\
		     YE_ARRAY_FOREACH_INIT(array);			\
	     !yBlockArrayIteratorIsEnd(&it) &&				\
		     ({							\
			     val =  yBlockArrayIteratorGetPtr(it, ArrayEntry); \
			     1;						\
		     });						\
	     yBlockArrayIteratorIncr(&it))

#define YE_ARRAY_FOREACH(array, val)		\
	YE_ARRAY_FOREACH_EXT(array, val, it##val)

#define YE_FOREACH YE_ARRAY_FOREACH

/**
 * @brief get first entity of array
 */
NO_SIDE_EFFECT static inline Entity *yeFirst(Entity *array)
{
	YE_FOREACH(array, el)
		return el;
	return NULL;
}

/**
 * @brief remove first elem
 * call it unset to emphasis on the fast that it just
 * let an empty elem at the begin of the array, but dont move all elems
 */
void yeUnsetFirst(Entity *array);

/**
 * @brief get last elem of array
 */
NO_SIDE_EFFECT Entity *yeGetLast(Entity *array);

/*
 * turn out yeLast seems a better name than get last
 * I mean it basically say the same and yeLast is shorter so cuter
 * KAWAIIIII ne ?
 */
#define yeLast yeGetLast

/**
 * @return the entity at the position of @index or NULL
 */
NO_SIDE_EFFECT Entity *yeGetByIdx(Entity *entity, size_t index);

NO_SIDE_EFFECT static inline Entity *yeGetByIdxDirect(Entity entity[static 1], size_t index)
{
	return yBlockArrayGetPtrDirect(YE_TO_ARRAY(entity)->values,
				       index, ArrayEntry)->entity;
}

/**
 * @param entity  the entity we are looking into
 * @param name    the entity name whe are looking for
 * @return The found Entity named @name in @entity
 */
NO_SIDE_EFFECT Entity *yeGetByStr(Entity *entity, const char *name);
NO_SIDE_EFFECT Entity *yeNGetByStr(Entity *entity, const char *name, int len);

/**
 * Same as yeGetByStrFast, but store the index in @idx
 */
Entity *yeGetByStrExt(Entity *entity, const char *name, int64_t *idx);

static inline Entity *yeGetByEntity(Entity *array, Entity *key);

/**
 * Like yeGetByStr but dosn't work with sytaxe like this (entity1.entity11)
 */
NO_SIDE_EFFECT Entity *yeGetByStrFast(Entity *entity, const char *name);

#ifndef __cplusplus
#define yeGet(ENTITY, INDEX) _Generic((INDEX),				\
				      unsigned short int: yeGetByIdx,		\
				      short int: yeGetByIdx,			\
				      unsigned int: yeGetByIdx,		\
				      int: yeGetByIdx,			\
				      long : yeGetByIdx,		\
				      long long : yeGetByIdx,		\
				      unsigned long long : yeGetByIdx,	\
				      unsigned long: yeGetByIdx,	\
				      Entity *: yeGetByEntity,		\
				      const Entity *: yeGetByEntity,		\
				      Y_GEN_CLANG_ARRAY(char, yeGetByStrFast), \
				      const char *: yeGetByStrFast,	\
				      char *: yeGetByStrFast) (ENTITY, INDEX)

#endif

#define yeArrayIdx(a, lookup)					\
  _Generic(lookup,						\
	   char *: yeArrayIdx_str,				\
	   Entity *: yeArrayIdx_ent,				\
	   const Entity *: yeArrayIdx_ent,			\
	   const char *: yeArrayIdx_str)(a, lookup)		\

NO_SIDE_EFFECT int yeArrayIdx_str(Entity *array, const char *lookup);

NO_SIDE_EFFECT static inline int yeArrayIdx_ent(Entity *array, Entity *lookup)
{
	for (size_t i = 0; i < yeLen(array); ++i)
		if (yeGet(array, i) == lookup)
			return i;
	return -1;
}

/**
 * @return the key string if there is one
 */
NO_SIDE_EFFECT char *yeGetKeyAt(Entity *entity, int idx);

NO_SIDE_EFFECT char *yeLastKey(Entity *array);

/* crappy ifndef for crappy language */
#ifndef __cplusplus

/**
 * @return the key string if there is one
 */
NO_SIDE_EFFECT static inline char *yeFindKey(Entity *entity, Entity *target)
{
	int idx = yeArrayIdx(entity, target);

	if (idx >= 0)
		return yeGetKeyAt(entity, idx);
	return NULL;
}

#endif

/**
 * push array_src[idx] in array_dest, keep the string key if there is one
 * the element is push at the end of array_dest.
 */
#define yePushKeyAndVal(array_dest, array_src, idx)	\
	yePushBack(array_dest, yeGet(array_src, idx),	\
		   yeGetKeyAt(array_src, idx))


#define yeGetIntDirect(entity) (YE_TO_INT(entity)->value)


static inline int ye_revforeach_eval_(Entity *a, int *i, Entity **e)
{
	if (*i <= 0)
		return 0;
	*e = yeGet(a, *i - 1);
	if (*e)
		return 1;
	*i -= 1;
	return ye_revforeach_eval_(a, i, e);
}

#define YE_REVFOREACH(array, val)					\
		Entity *val;						\
		for (int i_YE_REVFOREACH = yeLen(array);		\
		     ye_revforeach_eval_(array, &i_YE_REVFOREACH, &val); \
		     --i_YE_REVFOREACH)

/**
 * @return 0 if @entity is NULL
 */
NO_SIDE_EFFECT int yeGetInt(Entity *entity);

/**
 * @return 0 if @entity is NULL
 */
NO_SIDE_EFFECT static inline int yeGetQuadInt0(Entity *entity) {
	if (yeType(entity) != YQUADINT) {
		return 0;
	} 
	return YE_TO_QINT(entity)->x;
}

/**
 * @return 0 if @entity is NULL
 */
NO_SIDE_EFFECT static inline int yeGetQuadInt1(Entity *entity) {
	if (yeType(entity) != YQUADINT) {
		return 0;
	} 
	return YE_TO_QINT(entity)->y;
}

/**
 * @return 0 if @entity is NULL
 */
NO_SIDE_EFFECT static inline int yeGetQuadInt2(Entity *entity) {
	if (yeType(entity) != YQUADINT) {
		return 0;
	} 
	return YE_TO_QINT(entity)->w;
}

/**
 * @return 0 if @entity is NULL
 */
NO_SIDE_EFFECT static inline int yeGetQuadInt3(Entity *entity) {
	if (yeType(entity) != YQUADINT) {
		return 0;
	} 
	return YE_TO_QINT(entity)->h;
}

/**
 * @return	value of entity at @pos in @array, 0 if entity doesn't existe
 */
NO_SIDE_EFFECT static inline int yeGetIntAtByIdx(Entity *array, int pos)
{
	if (yeType(array) == YQUADINT) {
		switch(pos) {
		case 0:
			return yeGetQuadInt0(array);
		case 1:
			return yeGetQuadInt1(array);
		case 2:
			return yeGetQuadInt2(array);
		case 3:
			return yeGetQuadInt3(array);
		default:
			DPRINT_ERR("can't get elem '%d', Quand int contain only 4 int", pos);
			return 0;
		}
	}
	return yeGetInt(yeGetByIdx(array, pos));
}

NO_SIDE_EFFECT static inline int yeGetIntAtByStr(Entity *array, const char *pos)
{
	if (yeType(array) == YQUADINT) {
		if (!pos)
			return 0;
		switch(pos[0]) {
		case 'x':
			return yeGetQuadInt0(array);
		case 'y':
			return yeGetQuadInt1(array);
		case 'w':
			return yeGetQuadInt2(array);
		case 'h':
			return yeGetQuadInt3(array);
		default:
			DPRINT_ERR("can't get elem '%s'\nQuand int can only get string begining by 'x', 'y', 'w', 'h'", pos);
			return 0;
		}
	}
	return yeGetInt(yeGetByStrFast(array, pos));
}

static inline uint32_t yeGetUInt(Entity *i) {
	return (uint32_t)yeGetInt(i);
}

#define yeGetIntAt(array, pos) (yeGetInt(yeGet(array, pos)))

#define yeGetBoolAt(array, pos) (!!yeGetIntAt(array, pos))

/**
 * @return 0 if @entity is NULL
 */
NO_SIDE_EFFECT double yeGetFloat(Entity *entity);

#define yeGetFloatDirect(entity) (YE_TO_FLOAT(entity)->value)

/**
 * @return	value of entity at @pos in @array, 0 if entity doesn't existe
 */
NO_SIDE_EFFECT static inline int yeGetFloatAtByIdx(Entity *array, int pos)
{
	return yeGetFloat(yeGetByIdx(array, pos));
}

NO_SIDE_EFFECT static inline int yeGetFloatAtByStr(Entity *array, const char *pos)
{
	return yeGetFloat(yeGetByStrFast(array, pos));
}

#define yeGetFloatAt(array, pos) (yeGetFloat(yeGet(array, pos)))

/**
 * @return the string value
 */
NO_SIDE_EFFECT const char *yeGetString(Entity *entity);

/**
 * @TODO	do the generic version for strings
 * @return	value of entity at @pos in @array, 0 if entity doesn't existe
 */
NO_SIDE_EFFECT static inline const char *yeGetStringAtByIdx(Entity *array, int pos)
{
	return yeGetString(yeGetByIdx(array, pos));
}

NO_SIDE_EFFECT static inline const char *yeGetStringAtByStr(Entity *array, const char *key)
{
	return yeGetString(yeGet(array, key));
}

#define yeGetStringAt(array, pos) yeGetString(yeGet(array, pos))

NO_SIDE_EFFECT void *yeGetData(Entity *entity);

#define yeGetDataAt(array, pos) yeGetData(yeGet(array, pos))
/**
 * @return	value of entity at @pos in @array, 0 if entity doesn't existe
 */
NO_SIDE_EFFECT static inline void *yeGetDataAtByIdx(Entity *array, int pos)
{
	return yeGetData(yeGetByIdx(array, pos));
}

/**
 * @return	value of entity at @pos in @array, 0 if entity doesn't existe
 */
NO_SIDE_EFFECT static inline void *yeGetDataAtByStr(Entity *array, const char *pos)
{
	return yeGetData(yeGetByStr(array, pos));
}


/**
 * change the capacity than the array can store
 */
Entity *yeExpandArray(Entity *entity, unsigned int size);

/**
 * Add @toPush to @array>
 * @param	array the entity where we will add a new entity
 * @param	toPush the entity to add
 * @param	name the name
 * @return	0 or -1
 */
int yePushBack(Entity *array, Entity *toPush, const char *name);

/**
 * like yePushBack
 * @param	flag yeAttach flag
 * @return	0 or -1
 */
int yePushBackExt(Entity *entity, Entity *toPush,
		  const char *name, uint64_t flag);


static inline Entity *yePushArrayBack(Entity *dst, Entity *src)
{
	YE_ARRAY_FOREACH_ENTRY(src, v) {
		yePushBack(dst, v->entity, v->name);
	}
	return dst;
}

/* only useful if ndebug isn't set */
#ifndef NDEBUG
static inline void yeSetConst(Entity *e) {e->flag |= YENTITY_CONST;}
#else
static inline void yeSetConst(Entity *e) {(void)e;}
#endif

static inline void yeStoreAll(Entity *dst, Entity *array, const char *key)
{
	YE_ARRAY_FOREACH_ENTRY(array, v) {
		if (v->name && yuiStrEqual(v->name, key))
			yePushBack(dst, v->entity, NULL);
	}
}

/**
 * Push @toPush at @idx
 */
int yePushAt(Entity *array, Entity *toPush, int idx);

/**
 * Push @toPush at @idx, with @name
 */
int yePushAt2(Entity *array, Entity *toPush, int idx, const char *name);


/**
 * Push @toPush anywhere in @array
 * it's slower than push back but saffer
 */
int yePush(Entity array[static 1], Entity toPush[static 1], const char *name);

/**
 * Insert toPush at idx, might be ways lot eavierst that yePushAt, because it
 * move all elems after idx
 */
int yeInsertAt(Entity *array, Entity *toPush, size_t idx, const char *name);

#define yePushFront(array, toPush, name)	\
	yeInsertAt(array, toPush, 0, name)

/**
 * @param	array the array
 * @return	the entity that is erased from the entity @array
 */
Entity *yePopBack(Entity *array);

#ifndef __cplusplus
#define yeRemoveChild(array, toRemove)					\
  (_Generic(toRemove,							\
	    Entity *: yeRemoveChildByEntity,				\
	    void *: yeRemoveChildByEntity,				\
	    int: yeRemoveChildByIdx,					\
	    double : yeRemoveChildByIdx,				\
	    unsigned long long: yeRemoveChildByIdx,			\
	    long long: yeRemoveChildByIdx,				\
	    unsigned long: yeRemoveChildByIdx,				\
	    long: yeRemoveChildByIdx,					\
	    unsigned int: yeRemoveChildByIdx,				\
	    Y_GEN_CLANG_ARRAY(char, yeRemoveChildByStr),		\
	    const char *: yeRemoveChildByStr,				\
	    char *: yeRemoveChildByStr)((array), (toRemove)))
#endif

Entity *yeRemoveChildByEntity(Entity *array, Entity *toRemove);

/**
 * @brief	like yeRemoveChildByEntity, but toRemove can be NULL
 */
static inline Entity *yeTryRemoveChild(Entity *array, Entity *toRemove)
{
	if (!toRemove || !array)
		return NULL;
	return yeRemoveChildByEntity(array, toRemove);
}

static inline Entity *yeRemoveChildByIdx(Entity *array, int toRemove);

Entity *yeRemoveChildByStr(Entity *array, const char *toRemove);

static inline Entity *yeRemoveChildByIdx(Entity *array, int toRemove)
{
	return yeRemoveChild(array, yeGet(array, toRemove));
}

/**
 * @brief remove an element from array, in comparaison to yeRemoveChild, reorder so it don't let blank
 *
 * Exampe: let's say you have an array [ 0, 1, 2, 3 ], erase 2, you'll have [0, 1, 3]
 * with yeRemoveChild, you would have got: [ 0, 1, NULL, 3 ]
 */
_Bool yeEraseByE(Entity *array, Entity *target);

/**
 * function that create an entity and set it to  0, "" or NULL
 * mostly use internally
 */
Entity *yeCreate(EntityType type, void *val, Entity *parent, const char *name);

/*
 * Destructor and constructors.
 */
Entity *yeCreateQuadInt(int i0, int i1, int i2, int i3, Entity *parent, const char *name);
Entity *yeCreateQuadIntAt(int i0, int i1, int i2, int i3, Entity *parent, const char *name, int idx);
Entity *yeCreateInt(int value, Entity *parent, const char *name);
Entity *yeCreateIntAt(int value, Entity *parent, const char *name, int idx);
Entity *yeCreateFloat(double value, Entity *parent, const char *name);
Entity *yeCreateFloatAt(double value, Entity *parent, const char *name, int idx);
Entity *yeCreateString(const char *string, Entity *parent, const char *name);
Entity *yeCreateStringAt(const char *string, Entity *parent,
			 const char *name, int idx);
Entity *yeCreateNString(const char *string, int l, Entity *parent,
			const char *name);


Entity *yeCreateInts_(Entity *parent, int nbVars, ...);

#define yeCreateInts(parent, args...)				\
	(yeCreateInts_((parent), YUI_GET_ARG_COUNT(args), args))


#define yeCreateFunctionSimple(name, manager, father)	\
  yeCreateFunction(name, manager, father, name)

Entity *yeCreateFunction(const char *funcName, void *manager,
			 Entity *parent, const char *name);

Entity *yeCreateFunctionExt(const char *funcName, void *manager,
			    Entity *parent, const char *name, uint64_t flags);

Entity *yeCreateArrayByCStr(Entity *parent, const char *name);

static inline Entity *yeCreateArrayByEntity(Entity *parent, Entity *name)
{
	return yeCreateArrayByCStr(parent, yeGetString(name));
}

Entity *yeCreateHash(Entity *parent, const char *name);

#ifndef __cplusplus
#define yeCreateArray(mother, name)					\
  _Generic((name),							\
	   Entity *: yeCreateArrayByEntity,				\
	   void *: yeCreateArrayByEntity,				\
	   int: yeCreateArrayByCStr,					\
	   Y_GEN_CLANG_ARRAY(char, yeCreateArrayByCStr),		\
	   const char *: yeCreateArrayByCStr,				\
	   char *: yeCreateArrayByCStr)((mother), (name))
#endif

static inline Entity *yeTryCreateArray(Entity *mother, const char *name)
{
	Entity *ret = yeGet(mother, name);

	return ret ? ret : yeCreateArrayByCStr(mother, name);
}

Entity *yeCreateArrayAt(Entity *father, const char *name, int idx);

Entity *yeCreateArrayExt(Entity *father, const char *name, uint32_t flags);

Entity *yeCreateDataAt(void *value, Entity *father, const char *name, int idx);
Entity *yeCreateData(void *value, Entity *father, const char *name);
Entity *yeCreateDataExt(void *value, Entity *father, const char *name,
			YDataFlag flag);

void yeDestroy(Entity *entity);
void yeDestroyInt(Entity *entity);
void yeDestroyFloat(Entity *entity);
void yeDestroyString(Entity *entity);
void yeDestroyFunction(Entity *entity);
void yeDestroyArray(Entity *entity);
void yeDestroyData(Entity *entity);
void yeDestroyHash(Entity *entity);
void yeDestroyQuadInt(Entity *entity);

static void yeDestroy_VoidPtr(void *e)
{
	yeDestroy(e);
}

/**
 * utility for YE_NEW
 */
static inline void yeAutoFreeDestroy(Entity **entity)
{
	yeDestroy(*entity);
}

void yeMultDestroy_(Entity *toRm, ...);

#define yeMultDestroy(args...) (yeMultDestroy_(args, NULL))


void yeClearArray(Entity *entity);

#define yeIncrementIntDirect(entity) (((IntEntity *)entity)->value += 1)

#define yeSetIntDirect(entity, value_) (((IntEntity *)entity)->value = (value_))

/**
 * @param entity
 * @param value
 * @return -1 if entity is not og type YINT, <value> otherwise
 */
void	yeSetInt(Entity *entity, int value);

#define yeSetFloatDirect(entity, value_)		\
	(((FloatEntity *)entity)->value = (value_))

/**
 * @param entity
 * @param value
 * @return -1 if entity is not og type YFLOAT, <value> otherwise
 */
void	yeSetFloat(Entity *entity, double value);

/**
 * Set a value to a StringEntity. Free the value if <entity> already had one
 * @param entity  the StringEntity to set the string to
 * @param val     the string to set to the StringEntity
 */
Entity	*yeSetString(Entity *entity, const char *value);

/**
 * similar to @yeSetString but copy at most n byte
 */
void	yeSetNString(Entity *e, const char *str, size_t n);

/**
 * @brief set a function entity to NULL
 */
void	yeUnsetFunction(Entity *entity);

/**
 * Free the entity's value and set the new value to the entity
 * @param entity
 * @param value
 * @return return <value>
 */
void	yeSetFunction(Entity *entity, const char *value);

void  yeSetDestroy(Entity *entity, void (*func)(void *));

static void  yeSetFreeAdDestroy(Entity *entity)
{
	yeSetDestroy(entity, free);
}

#define yeSet(ENTITY, VALUE) _Generic((VALUE),				\
				      int: yeSetInt,			\
				      double: yeSetFloat,		\
				      const char *: yeSetString,	\
				      Y_GEN_CLANG_ARRAY(char, yeSetString), \
				      char *: yeSetString)(ENTITY, VALUE)

/**
 * @brief set the information about the arguments of a function
 * @param nArgs	 number of arguments
 */
void	yeSetFunctionArgs(Entity *entity, unsigned int nArgs);

/**
 * set @value to @index if the entity is an array
 */
void	yeSetIntAt(Entity *entity, unsigned int index, int value);
void	yeSetFloatAt(Entity *entity, unsigned int index, double value);
void	yeSetStringAt(Entity *entity, unsigned int index, const char *value);
void	yeSetIntAtStrIdx(Entity *entity, const char *index, int value);
void	yeSetFloatAtStrIdx(Entity *entity, const char *index, double value);
void	yeSetStringAtStrIdx(Entity *entity, const char *index, const char *value);


/**
 * Attach @entity on @on at @idx, set @name as ... name
 */
int yeAttach(Entity *on, Entity *entity, unsigned int idx,
	     const char *name, uint64_t flag);

#ifndef __cplusplus
/* TODO: should create an element if doesn't exist */
#define yeSetAtIntIxd(ENTITY, INDEX, VALUE)			\
	_Generic((VALUE),					\
		 int: yeSetIntAt,				\
		 float: yeSetFloatAt,				\
		 Y_GEN_CLANG_ARRAY(char, yeSetStringAt),	\
		 const char *: yeSetStringAt,			\
		 char *: yeSetStringAt)(ENTITY, INDEX, VALUE)

#define YE_SET_AT_STRIDX_INTERNAL(WHAT)				\
	_Generic((WHAT),					\
		 int: yeSetIntAtStrIdx,				\
		 float: yeSetFloatAtStrIdx,			\
		 const char *: yeSetStringAtStrIdx,		\
		 Y_GEN_CLANG_ARRAY(char, yeSetStringAtStrIdx),	\
		 char *: yeSetStringAtStrIdx)

#define yeSetAtStrIdx(ENTITY, INDEX, VALUE)			\
	yeSetAtStrIdxInternal(VALUE)(ENTITY, INDEX, VALUE)

#define yeSetAt(ENTITY, INDEX, VALUE)					\
	_Generic((INDEX),						\
		 int: _Generic((VALUE),					\
			       int: yeSetIntAt,				\
			       float: yeSetFloatAt,			\
			       const char *: yeSetStringAt,		\
			       Y_GEN_CLANG_ARRAY(char, yeSetStringAt),	\
			       char *: yeSetStringAt),			\
		 char *: YE_SET_AT_STRIDX_INTERNAL(VALUE),		\
		 Y_GEN_CLANG_ARRAY(char, YE_SET_AT_STRIDX_INTERNAL(VALUE)), \
		 const char *: YE_SET_AT_STRIDX_INTERNAL(VALUE)		\
		)(ENTITY, INDEX, VALUE)

#endif

//char *: YE_SET_AT_STRIDX_INTERNAL(VALUE))(ENTITY, INDEX, VALUE)

#define RECREATE_IS_Float YFLOAT
#define RECREATE_IS_Int YINT
#define RECREATE_IS_Function YFUNCTION
#define RECREATE_IS_String YSTRING

#define YE_GET_REAL_TYPE(type) RECREATE_IS_##type

#define YE_IMPL_RECREATE(_type, value, mother, name)			\
	Entity *ret = yeGetByStrFast(mother, name);			\
									\
	if (ret) {							\
		if (unlikely(ret->type != YE_GET_REAL_TYPE(_type))) {	\
			yeRemoveChild(mother, ret);			\
		} else {						\
			yeSet##_type(ret, value);			\
			return ret;					\
		}							\
	}								\
	return yeCreate##_type(value, mother, name);			\


static inline Entity *yeReCreateFunction(const char *funcName, void *manager,
					 Entity *parent, const char *name)
{
	yeRemoveChildByStr(parent, name);
	return yeCreateFunction(funcName, manager, parent, name);
}

Entity *yeReCreateData(void *value, Entity *parent, const char *name);

/*
 * I could add cast everywhere to allow c++, but because C++
 * annoy me, I won't add a line of code for it.
 */
#ifndef __cplusplus

/**
 * child is the newly created array
 */
static inline Entity *yeReCreateArray(Entity *father, const char *name,
				      Entity *child)
{
	if (!father || !name)
		return yeCreateArray(father, NULL);

	Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(father)->values, tmp,
				  it, ArrayEntry) {
		if (tmp && yuiStrEqual0(tmp->name, name)) {
			if (child) {
				YE_INCR_REF(child);
			} else {
				child = yeCreateArray(NULL, NULL);
			}
			YE_DESTROY(tmp->entity);
			tmp->entity = child;
			return child;
		}
	}
	if (child) {
		return yePushBack(father, child, name) < 0 ? NULL : child;
	}
	return yeCreateArray(father, name);
}
#endif

static inline Entity *yeReCreateInt(int value, Entity *parent,
				    const char *name)
{
	YE_IMPL_RECREATE(Int, value, parent, name);
}


static inline Entity *yeReCreateFloat(double value, Entity *parent,
				      const char *name)
{
	YE_IMPL_RECREATE(Float, value, parent, name);
}

static inline Entity *yeReCreateString(const char *string,
				       Entity *parent, const char *name)
{
	YE_IMPL_RECREATE(String, string, parent, name);
}

static inline Entity *yeTryCreateInt(int value, Entity *parent,
				     const char *name)
{
	Entity *ret = yeGetByStrFast(parent, name);
	if (!ret) {
		ret = yeCreateInt(value, parent, name);
	}
	return ret;
}

static inline Entity *yeTryCreateString(const char *value, Entity *parent,
					const char *name)
{
	Entity *ret = yeGetByStrFast(parent, name);
	if (!ret) {
		ret = yeCreateString(value, parent, name);
	}
	return ret;
}


/**
 * @return the number of non null elems in the array
 */
NO_SIDE_EFFECT static inline size_t yeNbElems(Entity *array)
{
	int i = 0;
	YE_FOREACH(array, e)
		++i;
	return i;
}

#include "entity-string.h"


/**
 * @param entity
 * @return the entity's value if entity is of type YFUNCTION, NULL otherwise
 */
NO_SIDE_EFFECT const char *yeGetFunction(Entity *entity);

NO_SIDE_EFFECT void *yeGetFunctionFastPath(Entity *entity);

NO_SIDE_EFFECT static inline intptr_t yeIData(Entity *e)
{
	return YE_TO_FUNC(e)->idata;
}

/**
 * Check if Entity are the same type and if they are not NULL and copy the values from src to dest.
 * @src	The source Entity from where the values will be copied from.
 * @dest The destination Entity to where the values will be pasted.
 * @return NULL if entities do not have the same type or are NULL, @dest otherwise.
 */
Entity *yeCopy(Entity* src, Entity* dest);

/**
 * a more C-style copy
 * this is here because C-style function are in fact easier to
 * use than "logical" function:
 * sure, in english you generally say, copy this into that,
 * so src, dest make more sence than dest, src
 * but in C you do src = dest; so dest, src make more sence in this case.
 */
#define yecpy(dest, src)			\
	yeCopy(src, dest)

Entity *yeCreateCopy2(Entity *src, Entity *father, const char *name, _Bool just_ref);
Entity *yeCreateCopy(Entity *src, Entity *father, const char *name);

/**
 * @brief move @what from @src to @dest
 */
Entity	*yeMoveByEntity(Entity* src, Entity* dest, Entity *what,
			const char *dstName);

/**
 * Convert an Entity to a C String (char *)
 * @param	entity The entity
 * @param	deep If @entity is an array, how deep we should print it
 * @param	flag Aditional falgs (YE_FORMAT_PRETTY for pretty format)
 * @return	the newly allocated string, need to be free
 */
char *yeToCStr(Entity *entity, int deep, int flag);

static inline void yePrintAddr(Entity *e)
{
	printf("%p\n", e);
}

static inline void yePrint(Entity *e)
{
	if (!e) {
		puts("(nil)");
		return;
	}
	char *r = yeToCStr(e, 3, YE_FORMAT_PRETTY);

	printf("%s\n", r);
	free(r);
}

static inline Entity *yeMoveByStr(Entity* src, Entity* dest, const char *what)
{
	return yeMoveByEntity(src, dest, yeGet(src, what), what);
}

/**
 * @brief get @name in @src and push it in @dst
 */
static inline Entity *yeGetPush(Entity *src, Entity *dst, const char *name)
{
	Entity *ret = yeGet(src, name);

	if (!ret)
		return NULL;
	yePushBack(dst, ret, name);
	return ret;
}

/**
 * @briefJust read the code
 */
static inline Entity *yeGetPush2(Entity *src, const char *src_name, Entity *dst, const char *dst_name)
{
	Entity *ret = yeGet(src, src_name);

	if (!ret)
		return NULL;
	yePushBack(dst, ret, dst_name);
	return ret;
}


static inline Entity *yeGetByEntity(Entity *array, Entity *key)
{
	if (yeType(key) == YINT)
		return yeGet(array, yeGetInt(key));
	else if (yeType(key) == YSTRING)
		return yeGet(array, yeGetString(key));
	return NULL;
}

NO_SIDE_EFFECT static inline _Bool yeArrayContainEntity(Entity *array, const char *str)
{
	return !!yeGet(array, str);
}

/**
 * Assure that and int entity value is not lower/higher than min/max
 * if not, set the value to min/max
 */
static inline void yeIntForceBound(Entity *e, int min, int max) {
	int v = yeGetInt(e);

	if (v < min)
		yeSetInt(e, min);
	if (v > max)
		yeSetInt(e, max);
}

/**
 * Assure that and int entity value is not lower/higher than min/max
 * if not, set the value to the oposite value:
 * example: min is 0, max is 4, e value is -1, this function
 * will set e value to 4
 */
static inline void yeIntRoundBound(Entity *e, int min, int max) {
	int v = yeGetInt(e);

	if (v < min)
		yeSetInt(e, max);
	if (v > max)
		yeSetInt(e, min);
}


#define yeIncrAt(e, at) yeAdd(yeGet(e, at), 1)

#define YE_ADD_AT_STRIDX_INTERNAL(WHAT)		\
	_Generic((WHAT),			\
		 int: yeAddIntAtStr,		\
		 long : yeAddLongAtStr,		\
		 long long : yeAddLongAtStr,	\
		 float: yeAddFloatAtStr,	\
		 const char *: yeAddStrAtStr,	\
		 char *: yeAddStrAtStr)

#define yeAddAt(ENTITY, INDEX, VALUE)					\
	_Generic((INDEX),						\
		 long: _Generic((VALUE),				\
			       int: yeAddIntAtIdx,			\
			       long : yeAddLongAtIdx,			\
			       long long : yeAddLongAtIdx,		\
			       float: yeAddFloatAtIdx,			\
			       const char *: yeAddStrAtIdx,		\
			       char *: yeAddStrAtIdx),			\
		 long long: _Generic((VALUE),				\
			       int: yeAddIntAtIdx,			\
			       long : yeAddLongAtIdx,			\
			       long long : yeAddLongAtIdx,		\
			       float: yeAddFloatAtIdx,			\
			       const char *: yeAddStrAtIdx,		\
			       char *: yeAddStrAtIdx),			\
		 int: _Generic((VALUE),					\
			       int: yeAddIntAtIdx,			\
			       long : yeAddLongAtIdx,			\
			       long long : yeAddLongAtIdx,		\
			       float: yeAddFloatAtIdx,			\
			       const char *: yeAddStrAtIdx,		\
			       char *: yeAddStrAtIdx),			\
		 char *: YE_ADD_AT_STRIDX_INTERNAL(VALUE),		\
		 const char *: YE_ADD_AT_STRIDX_INTERNAL(VALUE)		\
		)(ENTITY, INDEX, VALUE)


#define yeAdd(e, toAdd)				\
	_Generic(toAdd, int: yeAddInt,		\
		 long : yeAddLong,		\
		 long long : yeAddLong,		\
		 const char *: yeAddStr,	\
		 char *: yeAddStr,		\
		 Entity *: yeAddEnt)		\
		(e, toAdd)

int yeArrayContainEntitiesInternal(Entity *entity, ...);

#define yeArrayContainEntities(array, ARGS...)			\
	yeArrayContainEntitiesInternal(array, ARRAY, NULL)

NO_SIDE_EFFECT static inline int yeStringIndexChar(Entity *entityStr, const char *chars)
{
	const char *str = yeGetString(entityStr);
	int len = yeLen(entityStr);

	for (int i = 0; i < len; ++i) {
		for (int j = 0; chars[j]; ++j) {
			if (str[i] == chars[j])
				return i;
			return -1;
		}
	}
	return -1;
}

static inline int yeIntAddInt(IntEntity *e, int i)
{
	assert(!(e->flag & YENTITY_CONST));
	e->value += i;
	return 0;
}

static inline int yeAddLong(Entity *e, long i)
{
	switch (yeType(e)) {
	case YINT:
		return yeIntAddInt(YE_TO_INT(e), i);
	case YSTRING:
		yeStringAddLong(e, i);
		return 0;
	default :
		return -1;
	}
}

static inline int yeAddInt(Entity *e, int i)
{
	switch (yeType(e)) {
	case YINT:
		return yeIntAddInt(YE_TO_INT(e), i);
	case YSTRING:
		yeStringAddInt(e, i);
		return 0;
	default :
		return -1;
	}
}

static inline int yeAddFloat(Entity *e, double i)
{
	switch (yeType(e)) {
	case YFLOAT:
		YE_TO_FLOAT(e)->value += i;
		return 0;
	case YINT:
		YE_TO_INT(e)->value += i;
		return 0;
	default :
		return -1;
	}
}

static inline int yeAddStr(Entity *e, const char *str)
{
	switch (yeType(e)) {
	case YSTRING:
		yeStringAdd(e, str);
		return 0;
	default :
		return -1;
	}
}

static inline int yeAddEnt(Entity *e, Entity *e2)
{
	/* yeToCStr add a \n and do pretty stuff which can be buggy */
	if (yeType(e) == YSTRING && yeType(e2) != YSTRING) {
		char *str = yeToCStr(e2, 4, YE_FORMAT_PRETTY | YE_FORMAT_NO_NL);

		yeStringAdd(e, str);
		free(str);
		return 0;
	}
	switch (yeType(e2)) {
	case YINT:
		return yeAddInt(e, yeGetInt(e2));
	case YSTRING:
		return yeAddStr(e, yeGetString(e2));
	default :
		return -1;
	}
	return -1;
}

static inline int yeMultInt(Entity *e, int i)
{
	switch (yeType(e)) {
	case YINT:
		YE_TO_INT(e)->value *= i;
		return 0;
	default :
		return -1;
	}
}

static inline int yeSubInt(Entity *e, int i)
{
	switch (yeType(e)) {
	case YINT:
		YE_TO_INT(e)->value -= i;
		return 0;
	case YFLOAT:
		YE_TO_FLOAT(e)->value -= i;
		return 0;
	default :
		return -1;
	}
}

static inline int yeSubFloat(Entity *e, float i)
{
	switch (yeType(e)) {
	case YFLOAT:
		YE_TO_FLOAT(e)->value -= i;
		return 0;
	case YINT:
		YE_TO_INT(e)->value -= i;
		return 0;
	default :
		return -1;
	}
}

static inline int yeSubEntInt(Entity *e, IntEntity *ie)
{
	return yeSubInt(e, yeGetInt(YE_TO_ENTITY(ie)));
}

static inline int yeSubEnt(Entity *e, Entity *e2)
{
	switch (yeType(e2)) {
	case YINT:
		return yeSubEntInt(e, YE_TO_INT(e2));
	default :
		return -1;
	}
	return -1;
}

static inline int yeAddIntAtIdx(Entity *container, int at, int to_add)
{
	if (yeType(container) == YQUADINT) {
		switch(at) {
		case 0:
			YE_TO_QINT(container)->x += to_add;
			return 0;
		case 1:
			YE_TO_QINT(container)->y += to_add;
			return 0;
		case 2:
			YE_TO_QINT(container)->w += to_add;
			return 0;
		case 3:
			YE_TO_QINT(container)->h += to_add;
			return 0;
		default:
			DPRINT_ERR("can't get elem '%d', Quand int contain only 4 int", at);
			return -1;
		}
	}
	return yeAddInt(yeGet(container, at), to_add);
}

static inline int yeAddLongAtStr(Entity *container, const char *at, long to_add)
{
	return yeAddLong(yeGet(container, at), to_add);
}

static inline int yeAddLongAtIdx(Entity *container, int at, long to_add)
{
	return yeAddLong(yeGet(container, at), to_add);
}

static inline int yeAddStrAtStr(Entity *container, const char *at, const char *to_add)
{
	return yeAddStr(yeGet(container, at), to_add);
}

static inline int yeAddStrAtIdx(Entity *container, int at, const char *to_add)
{
	return yeAddStr(yeGet(container, at), to_add);
}

static inline int yeAddFloatAtStr(Entity *container, const char *at, double to_add)
{
	return yeAddFloat(yeGet(container, at), to_add);
}

static inline int yeAddFloatAtIdx(Entity *container, int at, double to_add)
{
	return yeAddFloat(yeGet(container, at), to_add);
}

static inline int yeAddIntAtStr(Entity *container, const char *at, int to_add)
{
	if (yeType(container) == YQUADINT) {
		switch(at[0]) {
		case 'x':
			YE_TO_QINT(container)->x += to_add;
			return 0;
		case 'y':
			YE_TO_QINT(container)->y += to_add;
			return 0;
		case 'w':
			YE_TO_QINT(container)->w += to_add;
			return 0;
		case 'h':
			YE_TO_QINT(container)->h += to_add;
			return 0;
		default:
			DPRINT_ERR("can't get elem '%s' in quad", at);
			return -1;
		}
	}
	return yeAddInt(yeGet(container, at), to_add);
}

static inline Entity *yeFind(Entity *entity,
			     Entity *(*finder)(const char *,
					       Entity *, void *),
			     void *arg)
{
	if (!entity)
		return NULL;
	Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(entity)->values, tmp,
				  it, ArrayEntry) {
		Entity *ret;

		if ((ret = finder(tmp->name, tmp->entity, arg)) != NULL)
			return ret;
	}
	return NULL;
}

/**
 * @brief helper for yeFind
 */
static inline Entity *yeStrEqFinder(const char *name, Entity *str, void *o_str)
{
	(void)name;
	if (yeStrEq(str, (const char *)o_str))
		return str;
	return NULL;
}

/**
 * @brief Find a string in array
*/
static inline Entity *yeFindString(Entity *array, const char *str)
{
	return yeFind(array, yeStrEqFinder, (void *)str);
}

/**
 * rename @ptr inside @array with @name if
 * @ptr is found.
 */
int yeRenamePtrStr(Entity *array, Entity *ptr, const char *str);
int yeRenameIdxStr(Entity *array, int idx, const char *str);
int yeRenameStrStr(Entity *array, const char *old_name, const char *new_name);


/**
 * @brief remove all entity name @name inside @array and push @toPush
 * @return the entity that have been push
 */
static inline Entity *yeReplaceBackExt(Entity *array, Entity *toPush,
				       const char *name, int flag)
{
	Entity *tmp;
	Entity *ret = NULL;

	if (unlikely(!toPush))
		return NULL;
	yeIncrRef(toPush);

again:
	tmp = yeRemoveChildByStr(array, name);
	if (tmp)
		goto again;

	if (!yePushBackExt(array, toPush, name, flag))
		ret = toPush;

	yeDestroy(toPush);
	return ret;
}

static inline Entity *yeReplaceBack(Entity *array, Entity *toPush,
				    const char *name)
{
	return yeReplaceBackExt(array, toPush, name, 0);
}

/**
 * @brief get @name in @src and replace_back it in @dst
 */
static inline Entity *yeGetReplace(Entity *src, Entity *dst, const char *name)
{
	Entity *ret = yeGet(src, name);

	if (!ret)
		return NULL;
	yeReplaceBack(dst, ret, name);
	return ret;
}

static inline Entity *yeReplaceAtIdx(Entity *array, Entity *toPush, int idx)
{
	if (!array || !toPush)
		return NULL;

	ArrayEntry *entry = yBlockArrayGetPtr(&YE_TO_ARRAY(array)->values, idx,
					      ArrayEntry);
	if (!entry)
		return NULL;

	yeIncrRef(toPush);
	yeDestroy(entry->entity);
	entry->entity = toPush;
	return toPush;
}

static inline int yeReplace(Entity *array, Entity *toReplace, Entity *toPush)
{
	if (!array || !toReplace || !toPush)
		return -1;

	Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(array)->values, tmp,
				  it, ArrayEntry) {
		if (tmp && tmp->entity == toReplace) {
			yeIncrRef(toPush);
			yeDestroy(toReplace);
			tmp->entity = toPush;
			return 0;
		}
	}

	return -1;
}

/**
 * Check if @array contain @toFind
 * @return 1 if the @toFind is found, 0 otherwise.
 */
NO_SIDE_EFFECT static inline _Bool yeDoesInclude(Entity *array, Entity *toFind)
{
	if (!array || !toFind)
		return 0;
	Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(array)->values, tmp,
				  it, ArrayEntry) {
		if (tmp && tmp->entity == toFind)
			return 1;
	}
	return 0;
}

void yeIncrChildsRef(Entity *array);

int yeSetFlagByStr(Entity *array, const char *name, int flag);
int yeSetFlagByIdx(Entity *array, int idx, int flag);

#define yeSetFlag(array, idx, flag) _Generic((idx),			\
					     int : yeSetFlagByIdx,	\
					     Y_GEN_CLANG_ARRAY(char,	\
							       yeSetFlagByStr), \
					     char *: yeSetFlagByStr,	\
					     const char *: yeSetFlagByStr \
					     )(array, idx, flag)

static inline int yeSwapByIdx(Entity *array, size_t idx0, size_t idx1)
{
	if (idx0 >= yeLen(array) || idx1 >= yeLen(array))
		return -1;
	if (idx0 == idx1)
		return 0;

	BlockArray *ba = &YE_TO_ARRAY(array)->values;
	ArrayEntry *entry0 = yBlockArrayTryGetPtr(ba, idx0, ArrayEntry);
	ArrayEntry *entry1 = yBlockArrayTryGetPtr(ba, idx1, ArrayEntry);

	if (!entry1 && ! entry0)
		return 0;
	if (entry1 && entry0) {
		YUI_SWAP_PTR(entry0->entity, entry1->entity, Entity *);
		YUI_SWAP_PTR(entry0->name, entry1->name, char *);
		return 0;
	}
	if (!entry0) {
		YUI_SWAP_PTR(entry0, entry1, ArrayEntry *);
		YUI_SWAP_VALUE(idx0, idx1);
	}
	if (!entry1) {
		entry1 = yBlockArraySetGetPtr(ba, idx0, ArrayEntry);
		entry1->entity = entry0->entity;
		entry1->name = entry0->name;
		yBlockArrayUnset(ba, idx0);
	}
	return 0;
}


/**
 * @brief	swap @elem0 with @elem1
 * @return	0 on sucess, -1 on error
 */
static inline int yeSwapElems(Entity *array, Entity *elem0, Entity *elem1)
{
	ArrayEntry *entry0 = NULL;
	ArrayEntry *entry1 = NULL;

	if (!array || !elem0 || !elem1 || elem0 == elem1)
		return -1;
	Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(array)->values, tmp,
				  it, ArrayEntry) {
		if (!tmp)
			continue;
		if (tmp->entity == elem0)
			entry0 = tmp;
		if (tmp->entity == elem1)
			entry1 = tmp;
	}
	if (!entry0 || !entry1)
		return -1;

	YUI_SWAP_PTR(entry0->entity, entry1->entity, Entity *);
	YUI_SWAP_PTR(entry0->name, entry1->name, char *);
	return 0;
}

NO_SIDE_EFFECT static inline Entity *yeGetRandomElem(Entity *array)
{
	if (!array || yeType(array) != YARRAY)
		return NULL;

	int array_l = yeLen(array);
	if (!array_l)
		return NULL;
	return yeGet(array, yuiRand() % array_l);
}

static inline int yeShuffle(Entity *array)
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

/**
 * @return 1 if content of a and b is the same, work only for string, int and float
 */
NO_SIDE_EFFECT static inline int yeEqual(Entity *a, Entity *b)
{
	if (yeType(a) != yeType(b))
		return 0;
	switch (yeType(a)) {
	case YINT:
		return yeGetInt(a) == yeGetInt(b);
	case YFLOAT:
		return yeGetFloat(a) == yeGetFloat(b);
	case YSTRING:
		return !yeStrCmp(a, yeGetString(b));
	default:
		break;
	}
	return 0;
}

/**
 * check if an int is superior or inferior to e
 */
NO_SIDE_EFFECT static inline _Bool yeIntInfTo(Entity *e, int o)
{
	if (yeType(e) != YINT)
		return 0;
	return yeGetIntDirect(e) < o;
}

NO_SIDE_EFFECT static inline _Bool yeIntSupTo(Entity *e, int o)
{
	if (yeType(e) != YINT)
		return 0;
	return yeGetIntDirect(e) > o;
}

NO_SIDE_EFFECT static inline int yeIntBAnd(Entity *e, int o)
{
	if (yeType(e) != YINT)
		return 0;
	return yeGetIntDirect(e) & o;
}

NO_SIDE_EFFECT static inline _Bool yeIntCheckBAnd(Entity *e, int o)
{
	return !!yeIntBAnd(e, o);
}

enum {
	YE_PATCH_NO_SUP = 1 << 0
};

/**
 * create an entity that contain a representation of the diference between
 * orginalEntity and patchEntity
 *
 * @param father
 * @param name
 * @return the patch entity
 */
Entity *yePatchCreate(Entity *orginalEntity, Entity *patchEntity,
		      Entity *father, const char *name);

void yePatchAply(Entity *dest, Entity *patch);

/**
 * if flag is set to YE_PATCH_NO_SUP, no supresions are made
 */
void yePatchAplyExt(Entity *dest, Entity *patch, uint32_t flag);

/**
 * merge src into dst, if strength > 0,
 * then what is in both dst and src, are replace by src in dst. otherwise they are ignore
 */
static inline void yeMergeInto(Entity *dst, Entity *src, int strength)
{
	if (!strength) {
		YE_ARRAY_FOREACH_ENTRY(src, entry) {
			if (entry->name && !yeGet(dst, entry->name))
				yePushBack(dst, entry->entity, entry->name);
		}
	} else {
		YE_ARRAY_FOREACH_ENTRY(src, entry) {
			yeReplaceBack(dst, entry->entity, entry->name);
		}

	}
}

#endif
