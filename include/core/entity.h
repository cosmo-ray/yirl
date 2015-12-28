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
#include <glib.h>
#include "block-array.h"

#define NONNULL(arg) __attribute__ ((nonnull (arg)))

#ifdef __linux__
# ifdef LIB
# define WEAK __attribute__((weak))
# else
# define WEAK //nothing :)
	#endif
#else
# define WEAK //nothing :)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

  typedef enum {
    YE_FIND_MONE = 0,
    YE_FIND_LINK_NO_GET = 1,
    YE_FIND_LINK_NO_DEEP = 2
  } YeFindFlag;

  /*Les differents type d'entite, chacune est definie par une structure apres*/
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
    if (X->refCount == 1)	{		\
      yeDestroy(X);				\
      X = NULL;					\
    } else {					\
      yeDestroy(X);				\
    }						\
  } while (0);

  /**
   * @father is the entity contening this one (a struct or an array)
   */
#define	ENTITY_HEADER				\
  struct Entity_	**fathers;		\
  unsigned int nbFathers;			\
  unsigned int refCount;			\
  EntityType	type;				\

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

    unsigned int len;
    char	*value;
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

    unsigned int len;
    char	*value;
    unsigned int nArgs;
  } FunctionEntity;


  /**
   * @param str   the type name
   * @return      the corresponding type, -1 if type not found
   */
  EntityType yeStringToType(const char *str) WEAK;

  /**
   * @param type
   * @return the corresponding string of the type
   */
  const char *yeTypeToString(int type) WEAK;

#define YE_ARRAY_FOREACH_SET_VAL(it, val)				\
  ((val = yBlockArrayIteratorGetPtr(it, ArrayEntry)->entity) || 1)

#define YE_ARRAY_FOREACH(array, val)					\
  Entity *val;								\
  for (BlockArrayIterator it##val =					\
	 yBlockArrayIteratorCreate(&YE_TO_ARRAY(array)->values, 0);	\
       !yBlockArrayIteratorIsEnd(&it##val) &&				\
	 YE_ARRAY_FOREACH_SET_VAL(it##val, val);			\
       yBlockArrayIteratorIncr(&it##val))

  
  /**
   * @return:	the entity at the position of @index or NULL
   */
  Entity *yeGetByIdx(Entity *entity, size_t index) WEAK;
    
  /**
   * @param entity  the entity whe are looking into
   * @param name    the entity name whe are looking for
   * @return        The found Entity named @name in @entity
   */
  Entity *yeGetByStr(Entity *entity, const char *name) WEAK;

#ifdef __cplusplus
  extern "C++"
  {
    Entity *yeGet(Entity *entity, unsigned int index) WEAK;
    Entity *yeGet(Entity *entity, const char *name) WEAK;
  }
#else
#define yeGet(ENTITY, INDEX) _Generic((INDEX),				\
				      unsigned int: yeGetByIdx,		\
				      int: yeGetByIdx,			\
				      long : yeGetByIdx,		\
				      unsigned long: yeGetByIdx,	\
				      const char *: yeGetByStrFast,	\
				      char *: yeGetByStrFast) (ENTITY, INDEX)

#endif

  /**
   * Like yeGetStr but dosn't work with sytaxe like this (entity1.entity11)
   */
  Entity *yeGetByStrFast(Entity *entity, const char *name) WEAK;

  
  /**
   ~* change the capacity than the array can store
   */
  Entity *yeExpandArray(Entity *entity, unsigned int size) WEAK;

  /**
   * Add a new entity to @array>
   * @array:	the entity where we will add a new entity
   * @toPush:	the entity to add
   */
  int yePushBack(Entity *array, Entity *toPush, const char *name) WEAK;

  /**
   * @array:	the array
   * @return:	the entity that is erased from the entity @array
   */
  Entity *yePopBack(Entity *array) WEAK;

  Entity *yeRemoveChild(Entity *array, Entity *toRemove);

  static inline Entity *yeRemoveChildByStr(Entity *array, const char *toRemove)
  {
    return yeRemoveChild(array, yeGetByStr(array, toRemove));
  }
  
  /**
   * function who which an entity and set it to  0, "" or NULL
   */
  Entity *yeCreate(EntityType type, void *val, Entity *fathers, const char *name) WEAK;

  /* 
   * Destructor and constructors.
   */
  Entity *yeCreateInt(int value, Entity *fathers, const char *name) WEAK;
  Entity *yeCreateFloat(double value, Entity *fathers, const char *name) WEAK;
  Entity *yeCreateString(const char *string, Entity *fathers, const char *name) WEAK;
  Entity *yeCreateFunction(const char *string, Entity *fathers, const char *name) WEAK;
  Entity *yeCreateArray(Entity *fathers, const char *name) WEAK;

  Entity *yeCreateData(void *value, Entity *father, const char *name) WEAK;

  void yeDestroy(Entity *entity) WEAK;
  void yeDestroyInt(Entity *entity) WEAK;
  void yeDestroyFloat(Entity *entity) WEAK;
  void yeDestroyString(Entity *entity) WEAK;
  void yeDestroyFunction(Entity *entity) WEAK;
  void yeDestroyRef(Entity *entity) WEAK;
  void yeDestroyArray(Entity *entity) WEAK;
  void yeDestroyData(Entity *entity)  WEAK;
 
  /**
   * @parap entity
   * @param value
   * @return -1 if entity is not og type YINT, <value> otherwise
   */
  void	yeSetInt(Entity *entity, int value) WEAK;

  /**
   * @parap entity
   * @param value
   * @return -1 if entity is not og type YFLOAT, <value> otherwise
   */
  void	yeSetFloat(Entity *entity, double value) WEAK;

  /**
   * Set a value to a StringEntity. Free the value if <entity> already had one
   * @param entity  the StringEntity to set the string to
   * @param val     the string to set to the StringEntity
   */
  void	yeSetString(Entity *entity, const char *value) WEAK;

  /**
   * @brief set a function entity to NULL
   */
  void	yeUnsetFunction(Entity *entity) WEAK;

  
  /**
   * Free the entity's value and set the new value to the entity
   * @param entity
   * @param value
   * @return return <value>
   */
  void	yeSetFunction(Entity *entity, const char *value) WEAK;

  void  yeSetDestroy(Entity *entity, void (*func)(void *)) WEAK;

#define yeSet(ENTITY, VALUE) _Generic((VALUE),				\
				      int: yeSetInt,			\
				      double: yeSetFloat,		\
				      const char *: yeSetString,	\
				      char *: yeSetString)(ENTITY, VALUE)

  
  /**
   * @brief set the information about the arguments of a function
   * @param nArgs number of arguments
   */
  void	yeSetFunctionArgs(Entity *entity, unsigned int nArgs) WEAK;
  
  /**
   * Set basic information to the entity <entity>
   * @param entity  the entity to set the basic informations
   * @param name    the name to set
   * @param type    the type of the entity
   * @param fathers  the parent entity of <entity>
   * @return the entity <entity>
   */
  Entity *yeInit(Entity *entity, EntityType type,
		 Entity *father, const char *name)  WEAK;

  /**
   * set to a value to the index if the entity is an array or a generic array
   */
  void	yeSetIntAt(Entity *entity, unsigned int index, int value) WEAK;
  void	yeSetFloatAt(Entity *entity, unsigned int index, double value) WEAK;
  void	yeSetStringAt(Entity *entity, unsigned int index, const char *value) WEAK;
  void	yeSetIntAtStrIdx(Entity *entity, const char *index, int value) WEAK;
  void	yeSetFloatAtStrIdx(Entity *entity, const char *index, double value) WEAK;
  void	yeSetStringAtStrIdx(Entity *entity, const char *index, const char *value) WEAK;


  int yeAttach(Entity *on, Entity *entity, unsigned int idx, const char *name);

#ifdef __cplusplus
extern "C++"
{
  void yeSetAt(Entity *entity, unsigned int index, const char *value) WEAK;
  void yeSetAt(Entity *entity, unsigned int index, int value) WEAK;
  void yeSetAt(Entity *entity, unsigned int index, float value) WEAK;
  void yeSetAt(Entity *entity, const char *index, const char *value) WEAK;
  void yeSetAt(Entity *entity, const char *index, int value) WEAK;
  void yeSetAt(Entity *entity, const char *index, float value) WEAK;
}
#else
#define yeSetAtIntIxd(ENTITY, INDEX, VALUE) _Generic((VALUE),		\
						     int: yeSetIntAt,	\
						     float: yeSetFloatAt, \
						     const char *: yeSetStringAt, \
						     char *: yeSetStringAt)(ENTITY, INDEX, VALUE)

#define yeSetAtStrIdx(ENTITY, INDEX, VALUE) _Generic((VALUE),			\
						     int: yeSetIntAtStrIdx, \
						     float: yeSetFloatAtStrIdx,	\
						     const char *: yeSetStringAtStrIdx, \
						     char *: yeSetStringAtStrIdx)(ENTITY, INDEX, VALUE)

#define yeSetAt(ENTITY, INDEX, VALUE) _Generic((INDEX),			\
    int: _Generic((VALUE),						\
		  int: yeSetIntAt,					\
		  float: yeSetFloatAt,					\
		  const char *: yeSetStringAt,				\
		  char *: yeSetStringAt),				\
    char *: _Generic((VALUE),						\
		     int: yeSetIntAtStrIdx,				\
		     float: yeSetFloatAtStrIdx,				\
		     const char *: yeSetStringAtStrIdx,			\
		     char *: yeSetStringAtStrIdx))(ENTITY, INDEX, VALUE)
  
#endif
  
  
  /**
   * Get the len attribute of an Entity
   * @param entity  The Entity we want to get the len
   * @return    return the attribute len of the entity
   */
  size_t yeLen(Entity *entity) WEAK;;

  /**
   * @parap entity
   * @param value
   * @return -1 if entity is not og type YINT, <value> otherwise
   */
  int	yeGetInt(Entity *entity) WEAK;

  /**
   * @param entity
   * @return the entity's value if entity is of type YFLOAT, -1 otherwise
   */
  double yeGetFloat(Entity *entity) WEAK;

  /**
   * @param entity
   * @return the string value 
   */
  const char *yeGetString(Entity *entity) WEAK;

  void *yeGetData(Entity *entity) WEAK;

#define YE_FOREACH_FATHER_SET_FATHER(child, father, idx)	\
  ((father = yeFathers(child)[(idx)]) || 1)
  
#define YE_FOREACH_FATHER(child, father)				\
  Entity *father = NULL;						\
  for (uint32_t father##idx = 0; father##idx < child->nbFathers &&	\
	 YE_FOREACH_FATHER_SET_FATHER(child, father, father##idx);	\
       ++father##idx)
  
  /**
   * @param entity
   * @return the entity's fathers
   */
  Entity **yeFathers(Entity *entity) WEAK;

  /**
   * @param entity
   * @return the entity's value if entity is of type YFUNCTION, NULL otherwise
   */
  const char	*yeGetFunction(Entity *entity) WEAK;

  int	yeFunctionNumberArgs(const Entity *entity) WEAK;

  /**
   * @param entity
   * @return if entity is not null return the type, -1 otherwise
   */
  int	yeType(const Entity *entity);

  /**
   * Check if Entity are the same type and if they are not NULL and copy the values from src to dest.
   * @param src	The source Entity from where the values will be copied from.
   * @param dest	The destination Entity to where the values will be pasted.
   * @return	NULL if entities do not have the same type or are NULL, dest Entity otherwise.
   */
  Entity*		yeCopy(Entity* src, Entity* dest);

  /**
   * Copy the data from src Entity to dest Entity.
   * Get the values and copy each Entity in the StructEntity.
   * @param src	Source Entity from where the data will be copy
   * @param dest	Destination Entity where the data will be past
   * @return	destination Entity if src AND dest or not null, NULL otherwise
   */
  ArrayEntity*		yeCopyContener(ArrayEntity* src, ArrayEntity* dest);



  static inline int yeOpsIntAddInt(IntEntity *e, int i)
  {
    e->value += i;
    return 0;
  }

  static inline int yeOpsAddInt(Entity *e, int i)
  {
    switch (yeType(e)) {
    case YINT:
      return yeOpsIntAddInt(YE_TO_INT(e), i);
    }
    return -1;
  }

  static inline int yeOpsAddEntInt(Entity *e, IntEntity *ie)
  {
    return yeOpsAddInt(e, yeGetInt(YE_TO_ENTITY(ie)));
  }

  static inline int yeOpsAddEnt(Entity *e, Entity *e2)
  {
    switch (yeType(e2)) {
    case YINT:
      return yeOpsAddEntInt(e, YE_TO_INT(e2));
    }
    return -1;
  }

  Entity *yeFindLink(Entity *father, const char *targetPath, int flag);
 
#ifdef __cplusplus
}
#endif


#endif
