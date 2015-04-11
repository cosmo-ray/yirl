/*
**Copyright (C) <2013> <YIRL_Team>
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

#ifndef	ENTITY_H
#define	ENTITY_H

#include <stdio.h>

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

/**
 * Here some macros to mutualise the code of entity
 */
#define RETURN_ERROR_BAD_TYPE(function, entity, returnValue) DPRINT_ERR("%s: bad entity, this entity is of type %s\n", (function), entityTypeToString(tryGetType(entity))); return (returnValue)



#ifdef __cplusplus
extern "C"
{
#endif
  /*Les differents type d'entite, chacune est definie par une structure apres*/
  typedef enum
    {
      BAD_TYPE = -1,
      STRUCT = 0,
      YINT,
      YFLOAT,
      YSTRING,
      REF,
      ARRAY,
      FUNCTION, //here will the fun begin
      GEN,
      STATIC
    } EntityType;
  
#define	NBR_ENTITYTYPE	9

#define	YE_TO_ENTITY(X) ((Entity *)X)
#define	YE_TO_C_ENTITY(X) ((const Entity *)X)
#define	YE_TO_INT(X) ((IntEntity *)X)
#define	YE_TO_C_INT(X) ((const IntEntity *)X)
#define	YE_TO_FLOAT(X) ((FloatEntity *)X)
#define	YE_TO_C_FLOAT(X) ((const FloatEntity *)X)
#define	YE_TO_STRING(X) ((StringEntity *)X)
#define	YE_TO_C_STRING(X) ((const StringEntity *)X)
#define	YE_TO_STRUCT(X) ((StructEntity *)X)
#define	YE_TO_C_STRUCT(X) ((const StructEntity *)X)
#define	YE_TO_ARRAY(X) ((ArrayEntity *)X)
#define	YE_TO_C_ARRAY(X) ((const ArrayEntity *)X)
#define	YE_TO_FUNC(X) ((FunctionEntity *)X)
#define	YE_TO_C_FUNC(X) ((const FunctionEntity *)X)

  struct	Entity;

  /**
   * @father is the entity contening this one (a struct or an array)
   */
#define	ENTITY_HEADER				\
  EntityType	type;				\
  const char	*name;				\
  struct Entity	*father;			\
  unsigned int refCount;

  /* macro for perf purpose */
#define yeIncrRef(entity) do {			\
    entity->refCount += 1;			\
  } while (0)

#define yeDecrRef(entity) do {			\
    entity->refCount -= 1;			\
  } while (0)

#define DESTROY_ENTITY(entity, type) do {	\
    yeDecrRef(entity);				\
    if (entity->refCount <= 0)			\
      free(((type *)entity));			\
  } while (0);
  
#define ALLOC_ENTITY(RET, TYPE) do {		\
    ret = malloc(sizeof(TYPE));			\
    ret->refCount = 1;				\
  } while (0);

  typedef struct Entity
  {
    ENTITY_HEADER

  } Entity;

  typedef	struct
  {
    ENTITY_HEADER

    unsigned int len;
    Entity	**values;
    const char	*structName;
    // "link" to the prototype of this Structure
    // prototype is a void * because EntityStructPrototype can't contain a class(i think)
    const void	*prototype;
  } StructEntity;

  typedef	struct
  {
    ENTITY_HEADER

    unsigned int len;
    Entity	**values;
    EntityType	contentType;
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
    
    // the name of the function to call
    char	*value;
    unsigned int nArgs;
    EntityType	*args; // including return value
  } FunctionEntity;

  typedef	struct
  {
    ENTITY_HEADER

    Entity	*value;
  } StaticEntity;

  /**
   * @param str   the type name
   * @return      the corresponding type, -1 if type not found
   */
  EntityType stringToEntityType(const char *str) WEAK;

  /**
   * @param type
   * @return the corresponding string of the type
   */
  const char *entityTypeToString(int type) WEAK;

  /**
   * @return the entity at the position of the index or NULL
   */
  Entity *getEntity(Entity *entity, unsigned int index) WEAK;
    
  /**
   * @return the entity at the position wich has the given name or NULL
   */
  Entity *findEntity(Entity *entity, const char *name) WEAK;

#ifdef __cplusplus
  extern "C++"
  {
    /**
     * @return the entity at the position of the index or NULL
     */
    Entity *yeGet(Entity *entity, unsigned int index) WEAK;
    Entity *yeGet(Entity *entity, const char *name) WEAK;
  }
#else
#define yeGet(ENTITY, INDEX) _Generic((INDEX),			\
				      unsigned int: getEntity,	\
				      int: getEntity,		\
				      const char *: findEntity,	\
				      char *: findEntity) (ENTITY, INDEX)

#endif


  /**
   * Like findEntity but dosn't work with sytaxe like this (entity1.entity11)
   */
  Entity *findDirectEntity(Entity *entity, const char *name) WEAK;

  
  /**
   * change the capacity than the array can store and init it to 0, "" or NULL
   */
  ArrayEntity	*manageArrayEntity(ArrayEntity *entity, unsigned int size, EntityType arrayType) WEAK;
  Entity	*manageArray(Entity *entity, unsigned int size, EntityType arrayType) WEAK;
  void	pushBack(Entity *array, Entity *toPush) WEAK;
  Entity *popBack(Entity *array) WEAK;
  Entity *arrayRemove(Entity *array, Entity *toRemove, int deepSearch);
  
  /**
   * function who alloc an entity and set it to  0, "" or NULL
   */
  Entity *genericCreatEntity(EntityType type, Entity *father, EntityType typeArray) WEAK;

   /** Ensemble de fonction pour cree et detruire chaque type d'entite.
    * Cannot initialise a structure in thers functions because the data are to complex
    */
  Entity *creatStructEntity(Entity *father) WEAK;
  Entity *creatIntEntity(int value, Entity *father) WEAK;
  Entity *creatFloatEntity(double value, Entity *father) WEAK;
  Entity *creatStringEntity(const char *string, Entity *father) WEAK;
  Entity *creatFunctionEntity(const char *string, Entity *father) WEAK;
  Entity *creatArrayEntity(EntityType contentType, Entity *father) WEAK;
  Entity *creatRefEntity(Entity *value, Entity *father) WEAK;
  Entity *creatStaticEntity(Entity *value, Entity *father) WEAK;

  void destroyEntity(Entity *entity) WEAK;
  void destroyStructEntity(Entity *entity) WEAK;
  void destroyIntEntity(Entity *entity) WEAK;
  void destroyFloatEntity(Entity *entity) WEAK;
  void destroyStringEntity(Entity *entity) WEAK;
  void destroyFunctionEntity(Entity *entity) WEAK;
  void destroyRefEntity(Entity *entity) WEAK;
  void destroyStaticEntity(Entity *entity) WEAK;
  void destroyArrayEntity(Entity *entity) WEAK;

  /**
   * Set a referenced Entity to a Entity after checking the type of the referencing entity
   * @param entity	the referencing Entity
   * @param other	the referenced Entity
   */
  int	setInt(Entity *entity, int value) WEAK;
  int	setFloat(Entity *entity, double value) WEAK;
  void	setString(Entity *entity, const char *value) WEAK;

  /**
   * @brief set a function entity to NULL
   */
  void	unsetFunction(Entity *entity) WEAK;

  
  const char	*setFunction(Entity *entity, const char *value) WEAK;
  
  /**
   * @brief set the information about the arguments of a function
   * @param nArgs number of arguments
   * @param args the argument list, sould be alocate first.
   */
  void	setFunctionArgs(Entity *entity, unsigned int nArgs, EntityType *args) WEAK;
  

  Entity *setEntityBasicInfo(Entity *entity, const char *name, EntityType type, Entity *father)  WEAK;
  /**
   * set to a value to the index if the entity is an array or a generic array
   */
  
  int	setIntAt(Entity *entity, unsigned int index, int value) WEAK;
  int	setFloatAt(Entity *entity, unsigned int index, double value) WEAK;
  void	setStringAt(Entity *entity, unsigned int index, const char *value) WEAK;
  int	setIntAtStrIdx(Entity *entity, const char *index, int value) WEAK;
  int	setFloatAtStrIdx(Entity *entity, const char *index, double value) WEAK;
  void	setStringAtStrIdx(Entity *entity, const char *index, const char *value) WEAK;

  
#ifdef __cplusplus
extern "C++"
{
  void setAt(Entity *entity, unsigned int index, const char *value) WEAK;
  void setAt(Entity *entity, unsigned int index, int value) WEAK;
  void setAt(Entity *entity, unsigned int index, Entity *value) WEAK;
  void setAt(Entity *entity, unsigned int index, float value) WEAK;
  void setAt(Entity *entity, const char *index, const char *value) WEAK;
  void setAt(Entity *entity, const char *index, int value) WEAK;
  void setAt(Entity *entity, const char *index, Entity *value) WEAK;
  void setAt(Entity *entity, const char *index, float value) WEAK;
}
#else
#define setAt(ENTITY, INDEX, VALUE) _Generic((VALUE),		\
					     int: setIntAt,		\
					     float: setFloatAt,		\
					     const char *: setStringAt, \
					     char *: setStringAt)(ENTITY, INDEX, VALUE)

#endif
  
  
  unsigned int getLen(Entity *entity) WEAK;;
  /*Geter pour les entites 
    certain sont particuliere comme get Father qui permet de recuperer entite pere d'une entite (entite pere est celle qui a appeler/cree l'entite en parametre*/
  Entity *getReferencedObj(Entity *entity) WEAK;
  int	getIntVal(Entity *entity) WEAK;
  double getFloatVal(Entity *entity) WEAK;
  const char *getStringVal(Entity *entity) WEAK;
  const char *getName(const Entity *entity) WEAK;
  Entity *getFather(Entity *entity) WEAK;
  const char	*getFunctionVal(Entity *entity) WEAK;
  int	getFunctionNumberArgs(const Entity *entity) WEAK;
  EntityType getFunctionArg(const Entity *entity, int i) WEAK;
  EntityType getContentType(const Entity *entity) WEAK;

  /**
   * check if the entity's type and type is the same
   * check at the same time if the entity is NULL
   */
  int	checkType(const Entity *entity, EntityType type) WEAK;

  const char *tryGetEntityName(const Entity *entity);
  const char *tryGetStructEntityName(const Entity *entity);

  /**
   * @param entity
   * @return if <entity> is not null return the type, -1 otherwise
   */
  int	tryGetType(const Entity *entity);

  /**
   * @param buf the buffer where the string is store
   * @param sizeBuf the size of buf
   * @return the number of caracter write into buf, -1 if not enough place
   */
  int entityToString(Entity *entity, char *buf, int sizeBuf);

  /**
   * Check if Entity are the same type and if they are not NULL and copy the values from src to dest.
   * @param src	The source Entity from where the values will be copied from.
   * @param dest	The destination Entity to where the values will be pasted.
   * @return	NULL if entities do not have the same type or are NULL, dest Entity otherwise.
   */
  Entity*		copyEntity(Entity* src, Entity* dest);

  /**
   * Copy the data from src Entity to dest Entity.
   * Get the values and copy each Entity in the StructEntity.
   * @param src	Source Entity from where the data will be copy
   * @param dest	Destination Entity where the data will be past
   * @return	destination Entity if src AND dest or not null, NULL otherwise
   */
  StructEntity*		copyStructEntity(StructEntity* src, StructEntity* dest);


#ifdef __cplusplus
}
#endif


#endif
