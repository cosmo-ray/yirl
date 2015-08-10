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
#include	"entity.h"
#include	"utils.h"
#include	"debug_core.h"

inline static void yeDestroyInternal(Entity *entity);

/**
 * Here some macros to mutualise the code of entity
 */
#define RETURN_ERROR_BAD_TYPE(function, entity, returnValue) DPRINT_INFO("%s: bad entity, this entity is of type %s\n", (function), yeTypeToString(yeType(entity))); return (returnValue)

  /* macro for perf purpose */
#define YE_INCR_REF(entity) do {		\
    entity->refCount += 1;			\
  } while (0)
  
#define YE_DECR_REF(entity) do {		\
    entity->refCount -= 1;		       	\
  } while (0)

#define YE_DESTROY_ENTITY(entity, type) do {	\
    YE_DECR_REF(entity);			\
    if (entity->refCount < 1) {			\
      free(entity->fathers);			\
      free(entity->name);			\
      free(((type *)entity));			\
    }						\
  } while (0);
  
#define YE_ALLOC_ENTITY(ret, type) do {		\
    ret = malloc(sizeof(type));			\
    ret->refCount = 1;				\
  } while (0);


/**
 * contain all the functions use to destruct entity
 * Must be in the same order than the EntityType Enum
 * TODO up to date
 */
void (*destroyTab[])(Entity *) = {
  yeDestroyInt,
  yeDestroyFloat,
  yeDestroyString,
  yeDestroyArray,
  yeDestroyFunction,
};

const char * EntityTypeStrings[] = { "int", "float", "string",
				     "array", "function"};

/**
 * @param entity
 * @param type
 * @return 1 if entity is not null and entity's type is the same as type, 0 otherwise
 */
static inline int	checkType(const Entity *entity, EntityType type)
{
  return (entity != NULL && entity->type == type);
}

EntityType	yeType(const Entity *entity)
{
  if (entity != NULL)
    return (entity->type);
  return (-1);
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

unsigned int yeLen(Entity *entity)
{
  if (!entity) {
    LOG_WARN("entity NULL in getLen\n");
    return (0);
  }
  return (((ArrayEntity *)entity)->len);
}

Entity *yeGetByIdx(Entity *entity, unsigned int index)
{
  if (entity == NULL) {
    DPRINT_WARN("entity is NULL\n");
    return NULL;
  }
  Entity *tmp;
  if (index >= ((ArrayEntity *)entity)->len)
    return NULL;
  tmp = ((ArrayEntity *)entity)->values[index];
  return (tmp);
}

/**
 * @param name  the name we will search the character '.' into
 * @return the index of the charactere '.' in name
 */
static int	findIdxPoint(const char *name)
{
  char* res = strchr(name, '.');
  return (res == NULL)
    ? -1
    : res - name;
}

/**
 * Look for an entity situated directly in <entity> wich have a name wich begin like <name> for a length of <end>
 * @param entity  the parent entity where we want to find the entity
 * @param name    The entity name we are looking for
 * @param end     the size of the <name> parameter we want to look for
 * @return        return the first entity in the parent <entity> found
 */
static Entity *yeGetByIdxFastWithEnd(Entity *entity, const char *name, int end)
{
  int	i = 0;
  Entity *tmp;
  while ((tmp = yeGetByIdx(entity, i)) != NULL) {
    if (!strncmp(tmp->name, name, end))
      return (tmp);
    ++i;
  }
  DPRINT_INFO( "could not find %s\n", name);
  return (NULL);
}

Entity *yeGetByStrFast(Entity *entity, const char *name)
{
  unsigned int	i = 0;
  Entity *tmp;

  while ((tmp = yeGetByIdx(entity, i)) != NULL)
  {
    if (yuiStrEqual(yePrintableName(tmp), name))
    	return (tmp);
    ++i;
  }
  DPRINT_INFO("could not find %s in  %s\n", name,
	      yePrintableName(entity));
  return NULL;
}

Entity *yeGetByStr(Entity *entity, const char *name)
{
  int	i;

  if (entity == NULL) {
    DPRINT_INFO("can not find entity fot %s\n", name);
    return NULL;
    }

  i = findIdxPoint(name);
  return (i != -1) ?
    (yeGet(yeGetByIdxFastWithEnd(entity, name, i), name + i + 1)) :
    (yeGetByStrFast(entity, name));
}

Entity *yeCreateInt(const char *name, int value, Entity *father)
{
  IntEntity *ret;
  YE_ALLOC_ENTITY(ret, IntEntity);
  yeInit((Entity *)ret, name, YINT, father);
  ret->value = value;
  return ((Entity *)ret);
}

Entity *yeCreateArray(const char *name, Entity *father)
{
  ArrayEntity *ret;
  YE_ALLOC_ENTITY(ret, ArrayEntity);
  yeInit((Entity *)ret, name, YARRAY, father);
  ret->len = 0;
  ret->values = NULL;
  return (YE_TO_ENTITY(ret));
}

Entity *yeCreateFloat(const char *name, double value, Entity *father)
{
  FloatEntity *ret;
  YE_ALLOC_ENTITY(ret, FloatEntity);
  yeInit((Entity *)ret, name, YFLOAT, father);
  ret->value = value;
  return ((Entity *)ret);
}

Entity *yeCreateFunction(const char *name, const char *value, Entity *father)
{
  FunctionEntity *ret;
  YE_ALLOC_ENTITY(ret, FunctionEntity);
  yeInit((Entity *)ret, name, YFUNCTION, father);
  ret->nArgs = 0;
  ret->value = NULL;
  yeSetString(YE_TO_ENTITY(ret), value);
  return (YE_TO_ENTITY(ret));
}

Entity *yeCreateString(const char *name, const char *string, Entity *father)
{
  StringEntity *ret;
  YE_ALLOC_ENTITY(ret, StringEntity);
  yeInit((Entity *)ret, name, YSTRING, father);
  ret->value = NULL;
  yeSetString(YE_TO_ENTITY(ret), string);
  return (YE_TO_ENTITY(ret));
}

#undef   REAJUSTE_REF

static void yeRemoveFather(Entity *entity, Entity *father)
{
  if (entity == NULL || father == NULL)
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
    Entity *tmp = yeGet(entity, i);
    yeRemoveFather(tmp, entity);
    yeDestroyInternal(tmp);
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
  if (((StringEntity *)entity)->value != NULL &&
      entity->refCount == 1) {
    free(((StringEntity *)entity)->value);
  }
  YE_DESTROY_ENTITY(entity, StringEntity);
}

void yeDestroyArray(Entity *entity)
{
  if(entity->refCount == 1) {
    destroyChilds(entity);
    free(YE_TO_ARRAY(entity)->values);
  }
  YE_DESTROY_ENTITY(entity, ArrayEntity);
}

inline static void yeDestroyInternal(Entity *entity)
{
  if (entity)
    destroyTab[entity->type](entity);
}

void yeDestroy(Entity *entity)
{
  yeDestroyInternal(entity);
}

Entity *yeCreate(const char *name, EntityType type, void *val, Entity *father)
{
  switch (type)
    {
    case YSTRING:
      return (yeCreateString(name, val, father));
    case YINT:
      return (yeCreateInt(name, *((int *)val), father));
    case YFLOAT:
      return (yeCreateFloat(name, *((double *)val), father));
    case YARRAY:
      return (yeCreateArray(name, father));
    case YFUNCTION:
      return (yeCreateFunction(name, val, father));
    default:
      DPRINT_ERR( "%s generic constructor not yet implemented\n",
		  yeTypeToString(type));
      break;
    }
  return (NULL);
}

static ArrayEntity	*manageArrayInternal(ArrayEntity *entity,
					     unsigned int size)
{
  unsigned int	i = 0;

  if (entity->len == 0) {
    entity->values = malloc(sizeof(Entity *) * size);
    i = 0;
  } else {
    entity->values = realloc(entity->values, sizeof(Entity *) * size);
    i = entity->len;
  }
  entity->len = size;
  for (; i < size; ++i) {
    entity->values[i] = NULL;
  }
  return entity;
}

Entity *yeExpandArray(Entity *entity, unsigned int size)
{
  if (!checkType(entity, YARRAY)) {
    DPRINT_ERR("yeExpandArray: bad entity\n");
    return NULL;
  }
  return ((Entity*)manageArrayInternal((ArrayEntity*)entity, size));
}

int	yePushBack(Entity *entity, Entity *toPush)
{
  int	len;

  if (!entity || !toPush)
    return -1;
  if (!checkType(entity, YARRAY)) {
    DPRINT_ERR("yePushBack: bad entity, "
	       "should be of type array or struct instead of %s\n",
	       yeTypeToString( yeType(entity)));
    return -1;
  }
  len = yeLen(entity);
  if (yeExpandArray(entity, len + 1) == NULL)
    return -1;
  if (yeAttach(entity, toPush, len))
    return -1;
  return 0;
}

Entity *yeRemoveChild(Entity *array, Entity *toRemove)
{
  int	len;
  Entity *tmp = NULL;

  if (!checkType(array, YARRAY)) {
    DPRINT_ERR("yeRemoveChild: bad entity\n");
    return NULL;
  }
  len = yeLen(array);
  for (int i = 0; i < len; ++i) {
    Entity *ret;

    tmp = yeGet(array, i);
    ret = tmp;
    if (tmp == toRemove) {
      for (int i2 = i + 1; i2 < len; ++i, ++i2) {
	YE_TO_ARRAY(array)->values[i] = YE_TO_ARRAY(array)->values[i2];
      }
      yePopBack(array);
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
  if (entity == NULL || father == NULL)
    return;
  if (entity->fathers == NULL)
    entity->fathers = malloc(sizeof(Entity *));
  else
    entity->fathers = realloc(entity->fathers, sizeof(Entity *) * (entity->nbFathers + 1));
  entity->fathers[entity->nbFathers] = father;
  entity->nbFathers += 1;
}

Entity *yeInit(Entity *entity, const char *name, EntityType type, Entity *father)
{
  if (!entity)
    return NULL;
  if (name == NULL) {
    entity->name = NULL;
  }  else {
    entity->name = strdup(name);
  }
  entity->type = type;
  entity->nbFathers = 0;
  entity->fathers = NULL;
  if (!yePushBack(father, entity))
    YE_DECR_REF(entity);
  return (entity);
}

int	yeSetName(Entity *entity, const char *name)
{
  if (!entity)
    return -1;

  if (yeName(entity))
    free(entity->name);
  entity->name = g_strdup(name);
  return 0;
}

void	yeSetString(Entity *entity, const char *val)
{
  if (((StringEntity *)entity)->value != NULL)
    free(((StringEntity *)entity)->value);
  if (val != NULL) {
    ((StringEntity *)entity)->value = strdup(val);
    ((StringEntity *)entity)->len = strlen(val);
  } else {
    ((StringEntity *)entity)->value = NULL;
    ((StringEntity *)entity)->len = 0;
  }
}

int yeAttach(Entity *on, Entity *entity, unsigned int idx)
{
  if (!on)
    return -1;
  if (on->type != YARRAY)
    return -1;
  if (idx >= yeLen(on))
    return -1;
  if (YE_TO_ARRAY(on)->values[idx])
    yeDestroyInternal(YE_TO_ARRAY(on)->values[idx]);
  YE_TO_ARRAY(on)->values[idx] = entity;
  yeAttachFather(entity, on);
  entity->refCount += 1;
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
  yeSetFunction(entity, NULL);
  yeSetFunctionArgs(entity, 0);
}

void	yeSetFunction(Entity *entity, const char *value)
{
  return yeSetString(entity, value);
}

void	yeSetFunctionArgs(Entity *entity, unsigned int nArgs)
{
  ((FunctionEntity *)entity)->nArgs = nArgs;
}


void	yeSetInt(Entity *entity, int value)
{
  if (yeType(entity) == YFLOAT)
    return (yeSetFloat(entity, value));
  ((IntEntity *)entity)->value = value;
}

void	yeSetFloat(Entity *entity, double value)
{
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

const char	*yeGetFunction(Entity *entity)
{
  if (!checkType(entity, YFUNCTION)) {
    RETURN_ERROR_BAD_TYPE("getFunctionVal", entity, NULL);
  }
  return YE_TO_FUNC(entity)->value;
}

int	yeFunctionNumberArgs(const Entity *entity)
{
  if (!entity) {
    DPRINT_WARN("yeFunctionNumberArgs: entity is NULL");
    return (YINT);
  }  
  return YE_TO_C_FUNC(entity)->nArgs;
}

double	yeGetFloat(Entity *entity)
{
  if (!checkType(entity, YFLOAT)) {
    RETURN_ERROR_BAD_TYPE("yeGetFloat", entity, -1);
  }
  return ((FloatEntity *)entity)->value;
}

const char *yeName(const Entity *entity)
{
  return (entity->name);
}

Entity **yeFathers(Entity *entity)
{
  return (entity->fathers);
}

const char *yePrintableName(const Entity *entity)
{
  if (entity == NULL)
    return ("(null)");
  return (yeName(entity));
}

Entity*		yeCopy(Entity* src, Entity* dest)
{
  const char* strVal = NULL;
  int	nArgs;

  if (src != NULL && dest != NULL
      && yeType(src) == yeType(dest)) {
    DPRINT_INFO("\tentity '%s' are '%s'\n", yePrintableName(src),
		yeTypeToString(yeType(src)));
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
      nArgs = yeFunctionNumberArgs(src);
      strVal = yeGetFunction(src);
      DPRINT_INFO("\t\tvalue is function '%s'\n", strVal);
      yeSetFunction(dest, strVal);
      yeSetFunctionArgs(dest, nArgs);
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

ArrayEntity*		yeCopyContener(ArrayEntity* src, ArrayEntity* dest)
  {
    unsigned int i;

    if (src == NULL || dest == NULL)
      return NULL;
    for (i = 0; i < yeLen((Entity*)src) && i < yeLen((Entity*)dest); i++)
      {
	yeCopy(src->values[i], dest->values[i]);
      }
    return dest;
  }


#define	ETS_REC_CALL(A,B,C) yeToString(A,B,C);++nbrSpace;
#define	ETS_RETURN(X) --nbrSpace;return(X);
#define	ETS_INCR_RET(I)	\
  ret += I;\
  sizeBuf -= I;\
  buf += I
int yeToString(Entity *entity, char *buf, int sizeBuf)
{
  unsigned int i;
  int ret = 0;
  int retETS; // the variable use to store the ETS_RETURN of the entityToString call inside EntityToString
  static int nbrSpace = 0; // nbr space before each printing
  
  if (sizeBuf <= 30)
    return (-1);
  switch (yeType(entity))
    {
    case YSTRING:
      ETS_RETURN (snprintf(buf, sizeBuf, "%s", yeGetString(entity)));
    case YINT:
      //printf("val: %d\n", getIntVal(entity));
      ETS_RETURN (snprintf(buf, sizeBuf, "%d", yeGetInt(entity)));
    case YFLOAT:
      //printf("val: %f\n", yeGetFloat(entity));
      ETS_RETURN (snprintf(buf, sizeBuf, "%f", yeGetFloat(entity)));
    case YFUNCTION:
      if (yeGetFunction(entity) == NULL) {
	ETS_RETURN (snprintf(buf, sizeBuf, "function %s: (null)",
			     yePrintableName(entity)));
      }
      retETS = snprintf(buf, sizeBuf, "function %s: nb arg: %d\n",
			yePrintableName(entity), yeFunctionNumberArgs(entity));
      if (retETS < 0)
	goto error;
      ETS_INCR_RET(retETS);
      retETS = snprintf(buf, sizeBuf, "function to call: %s",
			yeGetFunction(entity));
      if (retETS < 0)
	goto error;
      ETS_INCR_RET(retETS);
      ETS_RETURN (ret);
    case YARRAY:
      strcpy(buf, "["); // may bug here if sizeBuf is too small
      ETS_INCR_RET(1);
      for (i = 0; i < yeLen(entity); ++i)
	{
	  retETS = ETS_REC_CALL(yeGetByIdx(entity, i), buf, sizeBuf);
	  if (retETS < 0)
	    goto error;
	  ETS_INCR_RET(retETS);
	  if (yeLen(entity) > i + 1)
	    strcpy(buf, ", ");  // may bug here if sizeBuf is too small
	  ETS_INCR_RET(2);
	}
      strcpy(buf, "]");  // may bug here if sizeBuf is too small
      ETS_INCR_RET(1);
      ETS_RETURN (ret);
    default:
      goto error;
    }
 error:
  DPRINT_ERR("Error occured in entityToString on %s\n", yePrintableName(entity));
  nbrSpace = 0;
  return (-1);
}
#undef	ETS_REC_CALL
#undef	ETS_RETURN
#undef	ETS_INCR_RET


  /* macro for perf purpose */
#undef YE_INCR_REF
  
#undef YE_DECR_REF

#undef YE_DESTROY_ENTITY
  
#undef YE_ALLOC_ENTITY

#undef RETURN_ERROR_BAD_TYPE
