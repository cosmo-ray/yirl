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

/*
 * This file contain the entity API.
 */

#ifndef	_YIRL_ENTITY_H
#define	_YIRL_ENTITY_H

#include "entity-macro.h"
#include "block-array.h"

#define NONNULL(arg) __attribute__ ((nonnull (arg)))

typedef enum
  {
   YE_FIND_MONE = 0,
   YE_FIND_LINK_NO_GET = 1,
   YE_FIND_LINK_NO_DEEP = 2
  } YeFindFlag;

/* All entity type, each is defined later inside a struture */
typedef enum
  {
   BAD_TYPE = -1,
    YINT = 0,
    YFLOAT = 1,
    YSTRING = 2,
    YARRAY = 3,
    YFUNCTION = 4,
    YDATA = 5,
    NBR_ENTITYTYPE = 6
  } EntityType;

typedef enum
  {
   YENTITY_SMALL_SIZE_P0 = 1 << 0,
   YENTITY_SMALL_SIZE_P1 = 1 << 1,
   YENTITY_SMALL_SIZE_P2 = 1 << 2,
   YENTITY_SMALL_SIZE_P3 = 1 << 3
  } EntityFlag;

#define YENTITY_FLAG_LAST = YENTITY_SMALL_SIZE_P3;

#define YE_ATTACH_NO_MEM_FREE (1LLU << 32)
#define YE_ATTACH_NO_INC_REF (1LLU << 33)
#define YE_ATTACH_STEAL_NAME (1LLU << 34)


#define YE_FORMAT_OPT_BREAK_ARRAY_END 1
#define YE_FORMAT_OPT_PRINT_ONLY_VAL_ARRAY 2

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

/* TODO: move most of this code to yeDestroy, remove this */
#define YE_DESTROY(X) do {			\
    if (X == NULL)				\
      break;					\
    if (X->refCount == 1) {			\
      yeDestroy(X);				\
      X = NULL;					\
    } else {					\
      yeDestroy(X);				\
    }						\
  } while (0);


#define YE_INCR_REF(entity) do {		\
      entity->refCount += 1;			\
    } while (0)


    /* TODO: remove this macros and do a static inline function  */
#define yeIncrRef YE_INCR_REF

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
  uint32_t refCount;				\
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

typedef	struct
{
  ENTITY_HEADER

  /* Name of the function */
  char	*value;
  /* A pointer to the coresponding script manager */
  void	*manager;
  /*
   * A ptr use by the scripts to call a function faster
   * than if we was using the name of the function.
   * This is initialyse to NULL
   */
  void	*fastPath;
} FunctionEntity;

union SmallEntity {
  Entity Entity;
  IntEntity IntEntity;
  FloatEntity FloatEntity;
  StringEntity StringEntity;
  uint8_t totalSize[32];
};

union FatEntity {
  Entity Entity;
  ArrayEntity ArrayEntity;
  IntEntity IntEntity;
  FloatEntity FloatEntity;
  StringEntity StringEntity;
  DataEntity DataEntity;
  FunctionEntity FunctionEntity;
  union SmallEntity SnallEntities[4];
  uint8_t totalSize[128];
};

#ifdef __cplusplus
#include "entity-cplusplus.h"
#endif

static inline int yeRefCount(Entity *e)
{
  if (likely(e))
    return e->refCount;
  return 0;
}

#define yeMetadata(Entity, EntityType)			\
	(((uint8_t *)Entity) + sizeof(EntityType))

#define yeMetadataSize(EntityType)			\
	(sizeof(union FatEntity) - sizeof(EntityType))

void yeInitMem(void);

void yeEnd(void);

int yeIsPtrAnEntity(void *ptr);

int yeEntitiesArraySize(void);

int yeEntitiesUsed(void);

int yeFreeEntitiesInStack(void);

static inline EntityType yeType(const Entity * const entity)
{
  if (likely(entity != NULL))
    return (EntityType)entity->type;
  return (EntityType)BAD_TYPE;
}

static inline int yeIsNum(const Entity * const e)
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
EntityType yeStringToType(const char *str);

/**
 * @param type
 * @return the corresponding string of the type
 */
const char *yeTypeToString(int type);

static inline const char *yeTypeAsString(Entity *e)
{
  return yeTypeToString(yeType(e));
}


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

#define YE_ARRAY_FOREACH_INIT(array)				\
  (array == NULL ? yBlockArrayIteratorCreate(NULL, 0) :		\
   yBlockArrayIteratorCreate(&YE_TO_ARRAY(array)->values, 0))

#define YE_ARRAY_FOREACH_SET_VAL(it, val)				\
  ((val = yBlockArrayIteratorGetPtr(it, ArrayEntry)->entity) || 1)

#define YE_ARRAY_FOREACH_EXT(array, val, it)	\
  Entity *val;					\
  for (BlockArrayIterator it =			\
	 YE_ARRAY_FOREACH_INIT(array);		\
       !yBlockArrayIteratorIsEnd(&it) &&	\
	 YE_ARRAY_FOREACH_SET_VAL(it, val);	\
       yBlockArrayIteratorIncr(&it))

#define YE_ARRAY_FOREACH_ENTRY(array, val)	\
  ArrayEntry *val;					\
  for (BlockArrayIterator it =			\
	 YE_ARRAY_FOREACH_INIT(array);		\
       !yBlockArrayIteratorIsEnd(&it) &&	\
	 ({ val =  yBlockArrayIteratorGetPtr(it, ArrayEntry) ; 1; });	\
       yBlockArrayIteratorIncr(&it))

#define YE_ARRAY_FOREACH(array, val)		\
  YE_ARRAY_FOREACH_EXT(array, val, it##val)

#define YE_FOREACH YE_ARRAY_FOREACH

int	yeArrayIdx(Entity *array, const char *lookup);

Entity *yeGetLast(Entity *array);

/**
 * @return the entity at the position of @index or NULL
 */
Entity *yeGetByIdx(Entity *entity, size_t index);

static inline Entity *yeGetByIdxDirect(Entity *entity, size_t index)
{
  return yBlockArrayGetPtrDirect(YE_TO_ARRAY(entity)->values,
				 index, ArrayEntry)->entity;
}

/**
 * @param entity  the entity whe are looking into
 * @param name    the entity name whe are looking for
 * @return The found Entity named @name in @entity
 */
Entity *yeGetByStr(Entity *entity, const char *name);
Entity *yeNGetByStr(Entity *entity, const char *name, int len);

/**
 * Same as yeGetByStrFast, but store the index in @idx
 */
Entity *yeGetByStrExt(Entity *entity, const char *name, int64_t *idx);

static inline Entity *yeGetByEntity(Entity *array, Entity *key);

#ifndef __cplusplus
#define yeGet(ENTITY, INDEX) _Generic((INDEX),				\
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

/**
 * @return the key string if there is one
 */
char *yeGetKeyAt(Entity *entity, int idx);

/**
 * Like yeGetByStr but dosn't work with sytaxe like this (entity1.entity11)
 */
Entity *yeGetByStrFast(Entity *entity, const char *name);

#define yeGetIntDirect(entity) (YE_TO_INT(entity)->value)

/**
 * @return 0 if @entity is NULL
 */
int	yeGetInt(Entity *entity);

/**
 * @return	value of entity at @pos in @array, 0 if entity doesn't existe
 */
static inline int yeGetIntAtByIdx(Entity *array, int pos)
{
  return yeGetInt(yeGetByIdx(array, pos));
}

static inline int yeGetIntAtByStr(Entity *array, const char *pos)
{
  return yeGetInt(yeGetByStrFast(array, pos));
}

static inline uint32_t yeGetUInt(Entity *i) {
  return (uint32_t)yeGetInt(i);
}

#define yeGetIntAt(array, pos) (yeGetInt(yeGet(array, pos)))

/**
 * @return 0 if @entity is NULL
 */
double yeGetFloat(Entity *entity);

#define yeGetFloatDirect(entity) (YE_TO_FLOAT(entity)->value)

/**
 * @return	value of entity at @pos in @array, 0 if entity doesn't existe
 */
static inline int yeGetFloatAtByIdx(Entity *array, int pos)
{
  return yeGetFloat(yeGetByIdx(array, pos));
}

static inline int yeGetFloatAtByStr(Entity *array, const char *pos)
{
  return yeGetFloat(yeGetByStrFast(array, pos));
}

#define yeGetFloatAt(array, pos) (yeGetFloat(yeGet(array, pos)))

/**
 * @return the string value
 */
const char *yeGetString(Entity *entity);

/**
 * @TODO	do the generic version for strings
 * @return	value of entity at @pos in @array, 0 if entity doesn't existe
 */
static inline const char *yeGetStringAtByIdx(Entity *array, int pos)
{
  return yeGetString(yeGetByIdx(array, pos));
}

static inline const char *yeGetStringAtByStr(Entity *array, const char *key)
{
  return yeGetString(yeGet(array, key));
}

#define yeGetStringAt(array, pos) yeGetString(yeGet(array, pos))
void *yeGetData(Entity *entity);

#define yeGetDataAt(array, pos) yeGetData(yeGet(array, pos))
/**
 * @return	value of entity at @pos in @array, 0 if entity doesn't existe
 */
static inline void *yeGetDataAtByIdx(Entity *array, int pos)
{
  return yeGetData(yeGetByIdx(array, pos));
}

/**
 * @return	value of entity at @pos in @array, 0 if entity doesn't existe
 */
static inline void *yeGetDataAtByStr(Entity *array, const char *pos)
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
int yePushBackExt(Entity *entity, Entity *toPush,
		  const char *name, uint64_t flag);

/**
 * Push @toPush at @idx
 */
int yePushAt(Entity *array, Entity *toPush, int idx);

/**
 * Insert arrat at idx, might be ways lot eavierst that yePushAt, because it
 * move all elems after idx
 */
int yeInsertAt(Entity *array, Entity *toPush, size_t idx, const char *name);

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

static inline Entity *yeRemoveChildByIdx(Entity *array, int toRemove);

Entity *yeRemoveChildByStr(Entity *array, const char *toRemove);

static inline Entity *yeRemoveChildByIdx(Entity *array, int toRemove)
{
  return yeRemoveChild(array, yeGet(array, toRemove));
}

/**
 * function who which an entity and set it to  0, "" or NULL
 */
Entity *yeCreate(EntityType type, void *val, Entity *fathers, const char *name);

/*
 * Destructor and constructors.
 */
Entity *yeCreateInt(int value, Entity *fathers, const char *name);
Entity *yeCreateIntAt(int value, Entity *father, const char *name, int idx);
Entity *yeCreateFloat(double value, Entity *fathers, const char *name);
Entity *yeCreateFloatAt(double value, Entity *fathers, const char *name, int idx);
Entity *yeCreateString(const char *string, Entity *fathers, const char *name);
Entity *yeCreateStringAt(const char *string, Entity *father,
			 const char *name, int idx);
Entity *yeCreateNString(const char *string, int l, Entity *fathers,
			const char *name);

Entity *yeCreateInts_(Entity *fathers, int nbVars, ...);

#define yeCreateInts(father, args...)				\
  (yeCreateInts_((father), YUI_GET_ARG_COUNT(args), args))


#define yeCreateFunctionSimple(name, manager, father)	\
  yeCreateFunction(name, manager, father, name)

Entity *yeCreateFunction(const char *funcName, void *manager,
			 Entity *father, const char *name);

Entity *yeCreateFunctionExt(const char *funcName, void *manager,
			    Entity *father, const char *name, uint64_t flags);

Entity *yeCreateArrayByCStr(Entity *fathers, const char *name);

static inline Entity *yeCreateArrayByEntity(Entity *fathers, Entity *name)
{
  return yeCreateArrayByCStr(fathers, yeGetString(name));
}

#ifndef __cplusplus
#define yeCreateArray(fathers, name)					\
  _Generic((name),							\
	   Entity *: yeCreateArrayByEntity,				\
	   void *: yeCreateArrayByEntity,				\
	   int: yeCreateArrayByCStr,					\
	   Y_GEN_CLANG_ARRAY(char, yeCreateArrayByCStr),		\
	   const char *: yeCreateArrayByCStr,				\
	   char *: yeCreateArrayByCStr)((fathers), (name))
#endif

static inline Entity *yeTryCreateArray(Entity *father, const char *name)
{
  Entity *ret = yeGet(father, name);
  if (!ret) {
    yeCreateArrayByCStr(father, name);
  }
  return ret;
}

Entity *yeCreateArrayAt(Entity *fathers, const char *name, int idx);

Entity *yeCreateArrayExt(Entity *fathers, const char *name, uint32_t flags);

Entity *yeCreateDataAt(void *value, Entity *father, const char *name, int idx);
Entity *yeCreateData(void *value, Entity *father, const char *name);
Entity *yeCreateDataExt(void *value, Entity *father, const char *name,
			YDataFlag flag);

/**
 * Create an Array which is a pair of 2 elements contening: @value1 and @value2
 * @names and @father are optional
 */
static inline Entity *yeCreateTuple(Entity *value1, Entity *value2,
				    Entity *father, const char *name)
{
  Entity *ret = yeCreateArray(father, name);
  yePushBack(ret, value1, NULL);
  yePushBack(ret, value2, NULL);
  return ret;
}

void yeDestroy(Entity *entity);
void yeDestroyInt(Entity *entity);
void yeDestroyFloat(Entity *entity);
void yeDestroyString(Entity *entity);
void yeDestroyFunction(Entity *entity);
void yeDestroyRef(Entity *entity);
void yeDestroyArray(Entity *entity);
void yeDestroyData(Entity *entity) ;

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
void	yeSetString(Entity *entity, const char *value);

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
#define yeSetAtIntIxd(ENTITY, INDEX, VALUE)		\
  _Generic((VALUE),					\
	   int: yeSetIntAt,				\
	   float: yeSetFloatAt,				\
	   Y_GEN_CLANG_ARRAY(char, yeSetStringAt),	\
	   const char *: yeSetStringAt,			\
	   char *: yeSetStringAt)(ENTITY, INDEX, VALUE)

#define YE_SET_AT_STRIDX_INTERNAL(WHAT)				\
  _Generic((WHAT),						\
	   int: yeSetIntAtStrIdx,				\
	   float: yeSetFloatAtStrIdx,				\
	   const char *: yeSetStringAtStrIdx,			\
	   Y_GEN_CLANG_ARRAY(char, yeSetStringAtStrIdx),	\
	   char *: yeSetStringAtStrIdx)

#define yeSetAtStrIdx(ENTITY, INDEX, VALUE)		\
  yeSetAtStrIdxInternal(VALUE)(ENTITY, INDEX, VALUE)

#define yeSetAt(ENTITY, INDEX, VALUE)					\
  _Generic((INDEX),							\
	   int: _Generic((VALUE),					\
			 int: yeSetIntAt,				\
			 float: yeSetFloatAt,				\
			 const char *: yeSetStringAt,			\
			 Y_GEN_CLANG_ARRAY(char, yeSetStringAt),	\
			 char *: yeSetStringAt),			\
	   char *: YE_SET_AT_STRIDX_INTERNAL(VALUE),			\
	   Y_GEN_CLANG_ARRAY(char, YE_SET_AT_STRIDX_INTERNAL(VALUE)),	\
	   const char *: YE_SET_AT_STRIDX_INTERNAL(VALUE)		\
	   )(ENTITY, INDEX, VALUE)

#endif

//char *: YE_SET_AT_STRIDX_INTERNAL(VALUE))(ENTITY, INDEX, VALUE)

#define RECREATE_IS_Float YFLOAT
#define RECREATE_IS_Int YINT
#define RECREATE_IS_Function YFUNCTION
#define RECREATE_IS_String YSTRING

#define YE_GET_REAL_TYPE(type) RECREATE_IS_##type

#define YE_IMPL_RECREATE(_type, value, father, name)		\
  Entity *ret = yeGetByStrFast(father, name);			\
								\
  if (ret) {							\
    if (unlikely(ret->type != YE_GET_REAL_TYPE(_type))) {	\
      yeRemoveChild(father, ret);				\
    } else {							\
      yeSet##_type(ret, value);					\
      return ret;						\
    }								\
  }								\
  return yeCreate##_type(value, father, name);			\


static inline Entity *yeReCreateFunction(const char *funcName, void *manager,
					 Entity *father, const char *name)
{
  yeRemoveChildByStr(father, name);
  return yeCreateFunction(funcName, manager, father, name);
}

Entity *yeReCreateData(void *value, Entity *father, const char *name);

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

static inline Entity *yeReCreateInt(int value, Entity *father,
				    const char *name)
{
  YE_IMPL_RECREATE(Int, value, father, name);
}


static inline Entity *yeReCreateFloat(double value, Entity *father,
				      const char *name)
{
  YE_IMPL_RECREATE(Float, value, father, name);
}

static inline Entity *yeReCreateString(const char *string,
				       Entity *father, const char *name)
{
  YE_IMPL_RECREATE(String, string, father, name);
}

static inline Entity *yeTryCreateInt(int value, Entity *father,
				     const char *name)
{
  Entity *ret = yeGetByStrFast(father, name);
  if (!ret) {
    yeCreateInt(value, father, name);
  }
  return ret;
}

/**
 * Get the len attribute of an Entity
 * @param entity  The Entity we want to get the len
 * @return the attribute len of the entity
 */
size_t yeLen(Entity *entity);


#include "entity-string.h"


/**
 * @param entity
 * @return the entity's value if entity is of type YFUNCTION, NULL otherwise
 */
const char *yeGetFunction(Entity *entity);

void *yeGetFunctionFastPath(Entity *entity);

/**
 * Check if Entity are the same type and if they are not NULL and copy the values from src to dest.
 * @src	The source Entity from where the values will be copied from.
 * @dest The destination Entity to where the values will be pasted.
 * @return NULL if entities do not have the same type or are NULL, @dest otherwise.
 */
Entity *yeCopy(Entity* src, Entity* dest);

Entity *yeCreateCopy(Entity *src, Entity *father, const char *name);

/**
 * @brief move @what from @src to @dest
 */
Entity	*yeMoveByEntity(Entity* src, Entity* dest, Entity *what,
			const char *dstName);

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

static inline Entity *yeGetByEntity(Entity *array, Entity *key)
{
  if (yeType(key) == YINT)
    return yeGet(array, yeGetInt(key));
  else if (yeType(key) == YSTRING)
    return yeGet(array, yeGetString(key));
  return NULL;
}

static inline int yeArrayContainEntity(Entity *array, const char *str)
{
  return !!yeGet(array, str);
}

int yeArrayContainEntitiesInternal(Entity *entity, ...);

#define yeArrayContainEntities(array, ARGS...)		\
  yeArrayContainEntitiesInternal(array, ARRAY, NULL)

static inline int yeStringIndexChar(Entity *entityStr, const char *chars)
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
  e->value += i;
  return 0;
}

static inline int yeAddLong(Entity *e, long i)
{
  switch (yeType(e)) {
  case YSTRING:
    return yeStringAddLong(e, i);
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
    return yeStringAddInt(e, i);
  default :
    return -1;
  }
}

static inline int yeAddStr(Entity *e, const char *str)
{
  switch (yeType(e)) {
  case YSTRING:
    return yeStringAdd(e, str);
  default :
    return -1;
  }
}

static inline int yeAddEnt(Entity *e, Entity *e2)
{
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

static inline Entity *yeFind(Entity *entity,
			     Entity *(*finder)(const char *,
					       Entity *, void *),
			     void *arg)
{
  Y_BLOCK_ARRAY_FOREACH_PTR(YE_TO_ARRAY(entity)->values, tmp, it, ArrayEntry) {
    Entity *ret;

    if ((ret = finder(tmp->name, tmp->entity, arg)) != NULL)
      return ret;
  }
  return NULL;
}

/**
 * Convert an Entity to a C String (char *)
 * @param	entity The entity
 * @param	deep If @entity is an array, how deep we should print it
 * @param	flag Aditional falgs
 * @return	the newly allocated string, need to be free
 */
char *yeToCStr(Entity *entity, int deep, int flag);

/**
 * rename @ptr inside @array with @name if
 * @ptr is found.
 */
int yeRenamePtrStr(Entity *array, Entity *ptr, const char *str);
int yeRenameIdxStr(Entity *array, int idx, const char *str);

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
 * @FIXME: woooooo remove that t, "doesttttttt" does not mean something
 */
static inline int yeDoestInclude(Entity *array, Entity *toFind)
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

/**
 * @return 1 if content of a and b is the same, work only for string, int and float
 */
static inline int yeEqual(Entity *a, Entity *b)
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

#endif
