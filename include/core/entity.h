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
 * All there's functions are made to be use in lua too
 */

#ifndef	_YIRL_ENTITY_H
#define	_YIRL_ENTITY_H

#include <stdio.h>
#include "block-array.h"

#define NONNULL(arg) __attribute__ ((nonnull (arg)))


typedef enum {
  YE_FIND_MONE = 0,
  YE_FIND_LINK_NO_GET = 1,
  YE_FIND_LINK_NO_DEEP = 2
} YeFindFlag;

/* All entity type, each is defined later inside a struture */
typedef enum
  {
    BAD_TYPE = -1,
    YINT= 0,
    YFLOAT,
    YSTRING,
    YARRAY,
    YFUNCTION,
    YDATA,
    NBR_ENTITYTYPE
  } EntityType;

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


    /* TODO: remove thers macros and do a static inline function  */
#define yeIncrRef YE_INCR_REF


/**
 * @father is the entity contening this one (a struct or an array)
 */
#define	ENTITY_HEADER				\
  struct Entity_	**fathers;		\
  unsigned int nbFathers;			\
  unsigned int refCount;			\
  EntityType type;				\

typedef struct Entity_
{
  ENTITY_HEADER

} Entity;

typedef	struct {
  Entity *entity;
  char *name;
} ArrayEntry;

typedef	struct
{
  ENTITY_HEADER

  BlockArray values;
} ArrayEntity;

typedef	struct
{
  ENTITY_HEADER

  int		value;
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


#define YE_ARRAY_FOREACH(array, val)		\
  YE_ARRAY_FOREACH_EXT(array, val, it##val)

int	yeArrayIdx(Entity *array, const char *lookup);
  
/**
 * @return the entity at the position of @index or NULL
 */
Entity *yeGetByIdx(Entity *entity, size_t index);
    
/**
 * @param entity  the entity whe are looking into
 * @param name    the entity name whe are looking for
 * @return The found Entity named @name in @entity
 */
Entity *yeGetByStr(Entity *entity, const char *name);

/**
 * Same as yeGetByStrFast, but store the index in @idx
 */
Entity *yeGetByStrExt(Entity *entity, const char *name, int64_t *idx);

#define yeGet(ENTITY, INDEX) _Generic((INDEX),				\
				      unsigned int: yeGetByIdx,		\
				      int: yeGetByIdx,			\
				      long : yeGetByIdx,		\
				      long long : yeGetByIdx,		\
				      unsigned long long : yeGetByIdx,	\
				      unsigned long: yeGetByIdx,	\
				      Y_GEN_CLANG_ARRAY(char, yeGetByStrFast), \
				      const char *: yeGetByStrFast,	\
				      char *: yeGetByStrFast) (ENTITY, INDEX)


/**
 * Like yeGetStr but dosn't work with sytaxe like this (entity1.entity11)
 */
Entity *yeGetByStrFast(Entity *entity, const char *name);

  
/**
 * change the capacity than the array can store
 */
Entity *yeExpandArray(Entity *entity, unsigned int size);

/**
 * Add a new entity to @array>
 * @array:	the entity where we will add a new entity
 * @toPush:	the entity to add
 */
int yePushBack(Entity *array, Entity *toPush, const char *name);

/**
 * Push @toPush at @idx if the element is not empty, return -1 othervise
 */
int yPushAt(Entity *array, Entity *toPush, int idx);
  
/**
 * @array:	the array
 * @return the entity that is erased from the entity @array
 */
Entity *yePopBack(Entity *array);

Entity *yeRemoveChild(Entity *array, Entity *toRemove);

static inline Entity *yeRemoveChildByStr(Entity *array, const char *toRemove)
{
  return yeRemoveChild(array, yeGetByStr(array, toRemove));
}
  
/**
 * function who which an entity and set it to  0, "" or NULL
 */
Entity *yeCreate(EntityType type, void *val, Entity *fathers, const char *name);

/* 
 * Destructor and constructors.
 */
Entity *yeCreateInt(int value, Entity *fathers, const char *name);
Entity *yeCreateFloat(double value, Entity *fathers, const char *name);
Entity *yeCreateString(const char *string, Entity *fathers, const char *name);

#define yeCreateFunctionSimple(name, manager, father)	\
  yeCreateFunction(name, manager, father, name)

Entity *yeCreateFunction(const char *funcName, void *manager,
			 Entity *father, const char *name);
Entity *yeCreateArray(Entity *fathers, const char *name);
Entity *yeCreateArrayAt(Entity *fathers, const char *name, int idx);

Entity *yeCreateData(void *value, Entity *father, const char *name);

/**
 * Create an Array which is a pair of 2 elements contening: value1 and value2
 * Names are optional
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

/**
 * @parap entity
 * @param value
 * @return -1 if entity is not og type YINT, <value> otherwise
 */
void	yeSetInt(Entity *entity, int value);

/**
 * @parap entity
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
 * Set basic information to the entity <entity>
 * @param entity   the entity to set the basic informations
 * @param name     the name to set
 * @param type     the type of the entity
 * @param fathers  the parent entity of <entity>
 * @return the entity <entity>
 */
Entity *yeInit(Entity *entity, EntityType type,
	       Entity *father, const char *name) ;

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
 * we could rename this function as: "yePushAt"
 */
int yeAttach(Entity *on, Entity *entity, unsigned int idx, const char *name);

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

//char *: YE_SET_AT_STRIDX_INTERNAL(VALUE))(ENTITY, INDEX, VALUE)
    
  
#define YE_IMPL_RECREATE(type, value, father, name)	\
  Entity *ret = yeGetByStr(father, name);		\
							\
  if (ret) {						\
    yeSet##type(ret, value);				\
    return ret;						\
  }							\
  return yeCreate##type(value, father, name);		\

static inline Entity *yeReCreateArray(Entity *father,
				      const char *name, Entity *newArray)
{
  Entity *ret = yeGetByStr(father, name);

  yeRemoveChild(father, ret);
  if (newArray) {
    return yePushBack(father, newArray, name) < 0 ? NULL : newArray;
  }
  return yeCreateArray(father, name);
}

static inline Entity *yeReCreateInt(double value, Entity *father,
				    const char *name)
{
  YE_IMPL_RECREATE(Int, value, father, name)
    }


static inline Entity *yeReCreateFloat(double value, Entity *father,
				      const char *name)
{
  YE_IMPL_RECREATE(Float, value, father, name)
    }

static inline Entity *yeReCreateString(const char *string,
				       Entity *father, const char *name)
{
  YE_IMPL_RECREATE(String, string, father, name)
    }


/**
 * Get the len attribute of an Entity
 * @param entity  The Entity we want to get the len
 * @return the attribute len of the entity
 */
size_t yeLen(Entity *entity);;

/**
 * @parap entity
 * @param value
 * @return -1 if entity is not og type YINT, <value> otherwise
 */
int	yeGetInt(Entity *entity);

/**
 * @param entity
 * @return the entity's value if entity is of type YFLOAT, -1 otherwise
 */
double yeGetFloat(Entity *entity);

/**
 * @param entity
 * @return the string value 
 */
const char *yeGetString(Entity *entity);

void *yeGetData(Entity *entity);

#define YE_FOREACH_FATHER_SET_FATHER(child, father, idx)	\
  ((father = yeFathers(child)[(idx)]) || 1)
  
#define YE_FOREACH_FATHER(child, father)				\
  Entity *father = NULL;						\
  for (uint32_t father##idx = 0; child && father##idx < child->nbFathers && \
	 YE_FOREACH_FATHER_SET_FATHER(child, father, father##idx);	\
       ++father##idx)
  
/**
 * @param entity
 * @return the entity's fathers
 */
Entity **yeFathers(Entity *entity);

/**
 * @param entity
 * @return the entity's value if entity is of type YFUNCTION, NULL otherwise
 */
const char	*yeGetFunction(Entity *entity);

/**
 * @param entity
 * @return if entity is not null return the type, -1 otherwise
 */
static inline EntityType yeType(const Entity *entity)
{
	if (likely(entity != NULL))
		return (entity->type);
	return (-1);
}

/**
 * @param entity
 * @return the entity's value if entity is of type YFUNCTION, NULL otherwise
 */
const char	*yeGetFunction(Entity *entity);

/**
 * @param entity
 * @return if entity is not null return the type, -1 otherwise
 */
int	yeType(const Entity *entity);

/**
 * Check if Entity are the same type and if they are not NULL and copy the values from src to dest.
 * @param src		The source Entity from where the values will be copied from.
 * @param dest	The destination Entity to where the values will be pasted.
 * @return NULL if entities do not have the same type or are NULL, dest Entity otherwise.
 */
Entity*		yeCopy(Entity* src, Entity* dest);

/**
 * Copy the data from src Entity to dest Entity.
 * Get the values and copy each Entity in the StructEntity.
 * @param src		Source Entity from where the data will be copy
 * @param dest	Destination Entity where the data will be past
 * @return destination Entity if src AND dest or not null, NULL otherwise
 */
ArrayEntity*		yeCopyContener(ArrayEntity* src, ArrayEntity* dest);

static inline int yeArrayContainEntity(Entity *array, const char *str)
{
  return !!yeGet(array, str);
}

static inline int yeArrayContainEntitiesInternal(Entity *entity, ...)
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
}

static inline int yeOpsIntAddInt(IntEntity *e, int i)
{
  e->value += i;
  return 0;
}


static inline int yeAddInt(Entity *e, int i)
{
  switch (yeType(e)) {
  case YINT:
    return yeOpsIntAddInt(YE_TO_INT(e), i);
  default :
    return -1;
  }
}

static inline int yeAddEntInt(Entity *e, IntEntity *ie)
{
  return yeAddInt(e, yeGetInt(YE_TO_ENTITY(ie)));
}


static inline int yeAddEnt(Entity *e, Entity *e2)
{
  switch (yeType(e2)) {
  case YINT:
    return yeAddEntInt(e, YE_TO_INT(e2));
  default :
    return -1;
  }
  return -1;
}

Entity *yeFindLink(Entity *father, const char *targetPath, int flag);
 
/**
 * Convert an Entity to a C String (char *)
 * @entity	The entity
 * @deep	If @entity is an array, how deep we should print it
 * @flag	Aditional falg
 * @return	the newly allocated string, need to be free
 */
char *yeToCStr(Entity *entity, int deep, int flag);

/**
 * rename @ptr inside @array with @name if
 * @ptr is found.
 */
int yeRenamePtrStr(Entity *array, Entity *ptr, const char *str);

/**
 * @brief remove all entity name @name inside @array and push @toPush
 * @return the entity that have been push
 */
static inline Entity * yeReplaceBack(Entity *array, Entity *toPush,
				     const char *name)
{
  Entity *tmp;
  Entity *ret = NULL;

  yeIncrRef(toPush);

 again:
  tmp = yeGetByStrFast(array, name);
  if (tmp) {
    yeRemoveChild(array, tmp);
    goto again;
  }

  if (!yePushBack(array, toPush, name))
    ret = toPush;

  yeDestroy(toPush);
  return ret;
}

static inline int yeReplace(Entity *array, Entity *toReplace, Entity *toPush)
{
  if (!array || !toReplace || !toPush)
    return -1;

  Y_BLOCK_ARRAY_FOREACH_PTR(&YE_TO_ARRAY(array)->values, tmp,
			    it, ArrayEntry) {
    if (tmp && tmp->entity == toReplace) {
      YE_DESTROY(toReplace);
      tmp->entity = toPush;
      YE_INCR_REF(toPush);
      return 0;
    }
  }

  return -1;
}

#endif
