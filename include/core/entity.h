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
      ARRAY,
      FUNCTION
    } EntityType;
  
#define	NBR_ENTITYTYPE	6

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

#define YE_DESTROY(X) do {			\
  if (X->refCount == 1)	{			\
    yeDestroy(X);				\
    X = NULL;					\
  } else {					\
    yeDestroy(X);				\
  }						\
  } while (0);
  
  struct	Entity;

  /**
   * @father is the entity contening this one (a struct or an array)
   */
#define	ENTITY_HEADER				\
  char	*name;					\
  struct Entity	**fathers;			\
  unsigned int nbFathers;			\
  unsigned int refCount;			\
  EntityType	type;				\



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
    // prototype is a void * because EntityStructPrototype can't contain a class(i think)
    const void	*prototype;
  } StructEntity;

  typedef	struct
  {
    ENTITY_HEADER

    unsigned int len;
    Entity	**values;
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

  /**
   * @return the entity at the position of the index or NULL
   */
  Entity *yeGetIdx(Entity *entity, unsigned int index) WEAK;
    
  /**
   * @return the entity at the position wich has the given name or NULL
   */
  Entity *yeGetStr(Entity *entity, const char *name) WEAK;

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
				      unsigned int: yeGetIdx,	\
				      int: yeGetIdx,		\
				      const char *: yeGetStr,	\
				      char *: yeGetStr) (ENTITY, INDEX)

#endif


  /**
   * Like yeGetStr but dosn't work with sytaxe like this (entity1.entity11)
   */
  Entity *yeGetIdxFast(Entity *entity, const char *name) WEAK;

  
  /**
   * change the capacity than the array can store and init it to 0, "" or NULL
   */
  Entity *yeExpandArray(Entity *entity, unsigned int size) WEAK;
  int yePushBack(Entity *array, Entity *toPush) WEAK;
  Entity *yePopBack(Entity *array) WEAK;
  Entity *yeRemoveChild(Entity *array, Entity *toRemove);
  
  /**
   * function who alloc an entity and set it to  0, "" or NULL
   */
  Entity *yeCreate(char *name, EntityType type, Entity *fathers) WEAK;

   /** Ensemble de fonction pour cree et detruire chaque type d'entite.
    * Cannot initialise a structure in thers functions because the data are to complex
    */
  Entity *yeCreateStruct(char *name, Entity *fathers) WEAK;
  Entity *yeCreateInt(char *name, int value, Entity *fathers) WEAK;
  Entity *yeCreateFloat(char *name, double value, Entity *fathers) WEAK;
  Entity *yeCreateString(char *name, const char *string, Entity *fathers) WEAK;
  Entity *yeCreateFunction(char *name, const char *string, Entity *fathers) WEAK;
  Entity *yeCreateArray(char *name, Entity *fathers) WEAK;

  void yeDestroy(Entity *entity) WEAK;
  void yeDestroyStruct(Entity *entity) WEAK;
  void yeDestroyInt(Entity *entity) WEAK;
  void yeDestroyFloat(Entity *entity) WEAK;
  void yeDestroyString(Entity *entity) WEAK;
  void yeDestroyFunction(Entity *entity) WEAK;
  void yeDestroyRef(Entity *entity) WEAK;
  void yeDestroyArray(Entity *entity) WEAK;

  /**
   * Set a referenced Entity to a Entity after checking the type of the referencing entity
   * @param entity	the referencing Entity
   * @param other	the referenced Entity
   */
  void	yeSetInt(Entity *entity, int value) WEAK;
  void	yeSetFloat(Entity *entity, double value) WEAK;
  void	yeSetString(Entity *entity, const char *value) WEAK;

  /**
   * @brief set a function entity to NULL
   */
  void	yeUnsetFunction(Entity *entity) WEAK;

  
  void	yeSetFunction(Entity *entity, const char *value) WEAK;

#define yeSet(ENTITY, VALUE) _Generic((VALUE),				\
				      int: yeSetInt,			\
				      double: yeSetFloat,		\
				      const char *: yeSetString,	\
				      char *: yeSetString)(ENTITY, VALUE)

  
  /**
   * @brief set the informa
tion about the arguments of a function
   * @param nArgs number of arguments
   */
  void	yeSetFunctionArgs(Entity *entity, unsigned int nArgs) WEAK;
  

  Entity *yeInit(Entity *entity, const char *name, EntityType type, Entity *father)  WEAK;

  /**
   * set to a value to the index if the entity is an array or a generic array
   */
  void	yeSetIntAt(Entity *entity, unsigned int index, int value) WEAK;
  void	yeSetFloatAt(Entity *entity, unsigned int index, double value) WEAK;
  void	yeSetStringAt(Entity *entity, unsigned int index, const char *value) WEAK;
  void	yeSetIntAtStrIdx(Entity *entity, const char *index, int value) WEAK;
  void	yeSetFloatAtStrIdx(Entity *entity, const char *index, double value) WEAK;
  void	yeSetStringAtStrIdx(Entity *entity, const char *index, const char *value) WEAK;


  int yeAttach(Entity *on, Entity *entity, unsigned int idx);

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

#define yeSetAt(ENTITY, INDEX, VALUE) _Generic((INDEX), \
					       int: yeSetAtIntIxd,	\
					       char *:yeSetAtStrIdx)(ENTITY, INDEX, VALUE)
  
#endif
  
  
  unsigned int yeLen(Entity *entity) WEAK;;
  /*Geter pour les entites 
    certain sont particuliere comme get Fathers qui permet de recuperer entite pere d'une entite (entite pere est celle qui a appeler/cree l'entite en parametre*/
  int	yeGetInt(Entity *entity) WEAK;
  double yeGetFloat(Entity *entity) WEAK;
  const char *yeGetString(Entity *entity) WEAK;
  const char *yeName(const Entity *entity) WEAK;
  Entity **yeFathers(Entity *entity) WEAK;
  const char	*yeGetFunction(Entity *entity) WEAK;
  int	yeFunctionNumberArgs(const Entity *entity) WEAK;

  /* as yeName but return a printable result */
  const char *yePrintableName(const Entity *entity);
  const char *yePrintableStructName(const Entity *entity);

  /**
   * @param entity
   * @return if entity is not null return the type, -1 otherwise
   */
  int	yeType(const Entity *entity);

  /**
   * @param buf the buffer where the string is store
   * @param sizeBuf the size of buf
   * @return the number of caracter write into buf, -1 if not enough place
   */
  int yeToString(Entity *entity, char *buf, int sizeBuf);

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
  StructEntity*		yeCopyStruct(StructEntity* src, StructEntity* dest);


#ifdef __cplusplus
}
#endif


#endif
