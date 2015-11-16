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
#define RETURN_ERROR_BAD_TYPE(function, entity, returnValue) do {	\
  DPRINT_INFO("%s: bad entity, this entity is of type %s\n",		\
	      (function), yeTypeToString(yeType(entity)));		\
  return (returnValue);							\
  } while (0)

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
  if (index >= YE_TO_ARRAY(entity)->len)
    return NULL;
  tmp = yBlockArrayGet(&YE_TO_ARRAY(entity)->values, index, ArrayEntry).entity;
  return tmp;
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


static inline ArrayEntry *yeGetArrayEntryByIdx(Entity *entity, uint32_t i)
{
  return yBlockArrayGetPtr(&YE_TO_ARRAY(entity)->values, i, ArrayEntry);
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

  Y_BLOCK_ARRAY_FOREACH_PTR(&YE_TO_ARRAY(entity)->values, tmp, it, ArrayEntry) {
    if (strncmp(tmp->name, name, end))
      return tmp->entity;
  }
  return NULL;
}

Entity *yeGetByStrFast(Entity *entity, const char *name)
{
  if (!entity || !name)
    return NULL;

  Y_BLOCK_ARRAY_FOREACH_PTR(&YE_TO_ARRAY(entity)->values, tmp, it, ArrayEntry) {
    if (!tmp->name)
      continue;
    if (yuiStrEqual(tmp->name, name))
      return tmp->entity;
  }
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

Entity *yeCreateInt(int value, Entity *father, const char *name)
{
  IntEntity *ret;

  YE_ALLOC_ENTITY(ret, IntEntity);
  yeInit((Entity *)ret, YINT, father, name);
  ret->value = value;
  return ((Entity *)ret);
}

Entity *yeCreateData(void *value, Entity *father, const char *name)
{
  DataEntity *ret;

  YE_ALLOC_ENTITY(ret, DataEntity);
  yeInit((Entity *)ret, YDATA, father, name);
  ret->value = value;
  return ((Entity *)ret);
}

Entity *yeCreateArray(Entity *father, const char *name)
{
  ArrayEntity *ret;

  YE_ALLOC_ENTITY(ret, ArrayEntity);
  yeInit((Entity *)ret, YARRAY, father, name);
  ret->len = 0;
  yBlockArrayInit(&ret->values, BlockArray);
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

Entity *yeCreateFunction(const char *value, Entity *father, const char *name)
{
  FunctionEntity *ret;

  YE_ALLOC_ENTITY(ret, FunctionEntity);
  yeInit((Entity *)ret, YFUNCTION, father, name);
  ret->nArgs = 0;
  ret->value = NULL;
  yeSetString(YE_TO_ENTITY(ret), value);
  return (YE_TO_ENTITY(ret));
}

Entity *yeCreateString(const char *string, Entity *father, const char *name)
{
  StringEntity *ret;

  YE_ALLOC_ENTITY(ret, StringEntity);
  yeInit((Entity *)ret, YSTRING, father, name);
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
    ArrayEntry *ae = yeGetArrayEntryByIdx(entity, i);
    g_free(ae->name);
    Entity *tmp = ae->entity;
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
  if (YE_TO_STRING(entity)->value != NULL &&
      entity->refCount == 1) {
    free(YE_TO_STRING(entity)->value);
  }
  YE_DESTROY_ENTITY(entity, StringEntity);
}

void yeDestroyData(Entity *entity)
{
  if (YE_TO_DATA(entity)->value && YE_TO_DATA(entity)->destroy)
    YE_TO_DATA(entity)->destroy(YE_TO_DATA(entity)->value);
  YE_DESTROY_ENTITY(entity, DataEntity);
}

void yeDestroyArray(Entity *entity)
{
  if(entity->refCount == 1) {
    destroyChilds(entity);
    yBlockArrayFree(&YE_TO_ARRAY(entity)->values);
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

Entity *yeCreate(EntityType type, void *val, Entity *father, const char *name)
{
  switch (type)
    {
    case YSTRING:
      return (yeCreateString(val, father, name));
    case YINT:
      return (yeCreateInt(*((int *)val), father, name));
    case YFLOAT:
      return (yeCreateFloat(*((double *)val), father, name));
    case YARRAY:
      return (yeCreateArray(father, name));
    case YFUNCTION:
      return (yeCreateFunction(val, father, name));
    case YDATA:
      return (yeCreateData(val, father, name));
    default:
      DPRINT_ERR( "%s generic constructor not yet implemented\n",
		  yeTypeToString(type));
      break;
    }
  return (NULL);
}


static inline void arrayEntryInit(ArrayEntry *ae)
{
  ae->entity = NULL;
  ae->name = NULL;
}

static inline void arrayEntryDestroy(ArrayEntry *ae)
{
  g_free(ae->name);
  yeDestroy(ae->entity);
  arrayEntryInit(ae);
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
  unsigned int	i = size - 1;

  if (size < entity->len && !(flag & NO_ENTITY_DESTROY)) {
    for (; i < entity->len; ++i) {
      arrayEntryDestroy(&yBlockArrayGet(&entity->values, i, ArrayEntry));
      yBlockArrayUnset(&entity->values, i);
    }
  }

  yBlockArrayAssureBlock(&entity->values, size);
  entity->len = size;

  for (; i < size; ++i) {
    yBlockArraySet(&entity->values, i);
    arrayEntryInit(yeGetArrayEntryByIdx(YE_TO_ENTITY(entity), i));
  }
  return entity;
}

Entity *yeExpandArray(Entity *entity, unsigned int size)
{
  if (!checkType(entity, YARRAY)) {
    DPRINT_ERR("yeExpandArray: bad entity\n");
    return NULL;
  }
  return ((Entity*)manageArrayInternal((ArrayEntity*)entity, size, NONE));
}

int	yePushBack(Entity *entity, Entity *toPush, const char *name)
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
  if (yeAttach(entity, toPush, len, name))
    return -1;
  return 0;
}

Entity *yeRemoveChild(Entity *array, Entity *toRemove)
{
  int	len;
  ArrayEntry *tmp = NULL;

  if (!checkType(array, YARRAY)) {
    DPRINT_ERR("yeRemoveChild: bad entity\n");
    return NULL;
  }
  len = yeLen(array);
  for (int i = 0; i < len; ++i) {
    Entity *ret;

    tmp = yeGetArrayEntryByIdx(array, i);
    ret = tmp->entity;
    if (ret == toRemove) {
      arrayEntryDestroy(tmp);

      if (i == (len - 1))
	manageArrayInternal(YE_TO_ARRAY(array),
			    yeLen(array) - 1, NO_ENTITY_DESTROY);
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

Entity *yeInit(Entity *entity, EntityType type, Entity *father, const char *name)
{
  if (!entity)
    return NULL;
  entity->type = type;
  entity->nbFathers = 0;
  entity->fathers = NULL;
  if (!yePushBack(father, entity, name))
    YE_DECR_REF(entity);
  return (entity);
}

void	yeSetString(Entity *entity, const char *val)
{
  if (YE_TO_STRING(entity)->value != NULL)
    free(YE_TO_STRING(entity)->value);
  if (val != NULL) {
    YE_TO_STRING(entity)->value = strdup(val);
    YE_TO_STRING(entity)->len = strlen(val);
  } else {
    YE_TO_STRING(entity)->value = NULL;
    YE_TO_STRING(entity)->len = 0;
  }
}

void yeSetDestroy(Entity *entity, void (*destroyFunc)(void *))
{
  YE_TO_DATA(entity)->destroy = destroyFunc;
}

int yeAttach(Entity *on, Entity *entity,
	     unsigned int idx, const char *name)
{
  ArrayEntry *entry;
  
  if (!on)
    return -1;
  if (on->type != YARRAY)
    return -1;
  if (idx >= yeLen(on))
    return -1;

  entry = yeGetArrayEntryByIdx(on, idx);
  arrayEntryDestroy(entry);

  entry->entity = entity;
  entry->name = g_strdup(name);  
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

void	*yeGetData(Entity *entity)
{
  if (!checkType(entity, YDATA)) {
    RETURN_ERROR_BAD_TYPE("getDataVal", entity, NULL);
  }
  return YE_TO_DATA(entity)->value;
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


Entity **yeFathers(Entity *entity)
{
  return (entity->fathers);
}

Entity*		yeCopy(Entity* src, Entity* dest)
{
  const char* strVal = NULL;
  int	nArgs;

  if (src != NULL && dest != NULL
      && yeType(src) == yeType(dest)) {
    DPRINT_INFO("\tentity are '%s'\n", yeTypeToString(yeType(src)));
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

ArrayEntity	*yeCopyContener(ArrayEntity* src, ArrayEntity* dest)
  {
    if (src == NULL || dest == NULL)
      return NULL;

    ArrayEntry *destElem;

    Y_BLOCK_ARRAY_FOREACH_PTR(&src->values, elem, it, ArrayEntry) {

      yBlockArrayCopyElem(&dest->values, it, elem);

      destElem = yBlockArrayGetPtr(&dest->values, it, ArrayEntry);
      destElem->name = g_strdup(elem->name);
      yeCopy(elem->entity, destElem->entity);
    }
    return dest;
  }

  /* macro for perf purpose */
#undef YE_INCR_REF
  
#undef YE_DECR_REF

#undef YE_DESTROY_ENTITY
  
#undef YE_ALLOC_ENTITY

#undef RETURN_ERROR_BAD_TYPE
