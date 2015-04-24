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
#include	"entity.h"
#include	"debug_core.h"

/**
 * contain all the functions use to destruct entity
 * Must be in the same order than the EntityType Enum
 * TODO up to date
 */
void (*destroyTab[])(Entity *) = {
  yeDestroyStruct,
  yeDestroyInt,
  yeDestroyFloat,
  yeDestroyString,
  yeDestroyArray,
  yeDestroyFunction,
  yeDestroyStatic
};

const char * EntityTypeStrings[] = { "struct", "int", "float", "string",
				     "array", "function", "static" };

/**
 * @param entity
 * @param type
 * @return 1 if entity is not null and entity's type is the same as type, 0 otherwise
 */
static int	checkType(const Entity *entity, EntityType type)
{
  return (entity != NULL && entity->type == type);
}

EntityType	yeType(const Entity *entity)
{
  if (entity != NULL)
    return (entity->type);
  return (-1);
}

static inline int yStrEqual(const char *str1, const char *str2)
{
  int i;

  for (i = 0; str1[i]; ++i)
    if (str1[i] != str2[i])
      return 0;
  return 1;
}

EntityType yeStringToType(const char *str)
{
  int i;
  
  for (i = 0; i < NBR_ENTITYTYPE; ++i)
  {
    if (yStrEqual(str, EntityTypeStrings[i]))
    	return (i);
  }
  return (-1);
}

/**
 * @param type
 * @return the corresponding string of the type
 */
const char *yeTypeToString(int type)
{
  return (type < 0 || type >= NBR_ENTITYTYPE)
    ? ("(null)")
    : (EntityTypeStrings[type]);
}

/**
 * Get the len attribute of an StructEntity
 * @param entity  The Entity we want to get the len
 * @return    return the attribute len of the entity
 */
unsigned int yeLen(Entity *entity)
{
  if (!entity) {
    LOG_WARN("entity NULL in getLen\n");
    return (0);
  }
  return (((StructEntity *)entity)->len);
}

/**
 * @param entity  the entity where we want to get an entity
 * @param index   the index of the entity to get
 * @return  return entity is found, NULL otherwise
 */
Entity *yeGetIdx(Entity *entity, unsigned int index)
{
  if (entity == NULL) {
    DPRINT_WARN("entity is NULL\n");
    return NULL;
  }
  Entity *tmp;
  if (index >= ((StructEntity *)entity)->len)
    return (NULL);
  tmp = ((StructEntity *)entity)->values[index];
  if (!tmp) {
    DPRINT_WARN("can not get entity");
    return (NULL);
  }
  if (tmp->type == STATIC) {
    ((StaticEntity *)tmp)->value->name = tmp->name;
    ((StaticEntity *)tmp)->value->fathers = tmp->fathers;
    return (((StaticEntity *)tmp)->value);
  }
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
static Entity *yeGetIdxFastWithEnd(Entity *entity, const char *name, int end)
{
  int	i = 0;
  Entity *tmp;
  while ((tmp = yeGetIdx(entity, i)) != NULL) {
    if (!strncmp(tmp->name, name, end))
      return (tmp);
    ++i;
  }
  DPRINT_INFO( "could not find %s\n", name);
  return (NULL);
}

/**
 * @param entity  The entity where we want to find the entity
 * @name  The entity name we are looking for
 * @return return the entity named <name> in the entity <entity>
 */
Entity *yeGetIdxFast(Entity *entity, const char *name)
{
  unsigned int	i = 0;
  Entity *tmp;

  while ((tmp = yeGetIdx(entity, i)) != NULL)
  {
    if (yStrEqual(yePrintableName(tmp), name))
    	return (tmp);
    ++i;
  }
  DPRINT_WARN("could not find %s in  %s\n", name,
	      yePrintableName(entity));
  return NULL;
}

/**
 * @param entity  the entity whe are looking into
 * @param name    the entity name whe are looking for
 * @return        The found Entity named <name> in <entity>
 */
Entity *yeGetStr(Entity *entity, const char *name)
{
  int	i;

  if (entity == NULL) {
    DPRINT_ERR("can not find entity fot %s\n", name);
    return NULL;
    }
  DPRINT_INFO("finding entity %s in entity %s\t[%s:%d]\n", name,
	      tryGetEntityName(entity), __FILE__, __LINE__);
  i = findIdxPoint(name);
  return (i != -1) ?
    (yeGet(yeGetIdxFastWithEnd(entity, name, i), name + i + 1)) :
    (yeGetIdxFast(entity, name));
}

/**
 * @param value   value of the InEntity
 * @param fathers  the parent entity of the new entity
 * @return  return the new created entity
 */
Entity *yeCreateInt(int value, Entity *father)
{
  IntEntity *ret;
  YE_ALLOC_ENTITY(ret, IntEntity);
  yeInit((Entity *)ret, NULL, YINT, father);
  ret->value = value;
  return ((Entity *)ret);
}

/**
 * @param contentType   the type of the content
 * @param fathers        the fathers of the created entity
 * @return  return a new ArrayEntity 
 */
Entity *yeCreateArray(Entity *father)
{
  DPRINT_INFO("create array\n");
  ArrayEntity *ret;
  YE_ALLOC_ENTITY(ret, ArrayEntity);
  yeInit((Entity *)ret, NULL, ARRAY, father);
  ret->len = 0;
  ret->values = NULL;
  return ((Entity *)ret);
}

/**
 * @param value
 * @param fathers  the fathers of the entity to create
 * @return  a new StaticEntity
 */
Entity *yeCreateStatic(Entity *value, Entity *father)
{
  StaticEntity *ret;
  YE_ALLOC_ENTITY(ret, StaticEntity);
  yeInit((Entity *)ret, NULL, STATIC, father);
  ret->value = value;
  return ((Entity *)ret);
}

/**
 * @param value
 * @param fathers  the fathers of the entity to create
 * @return  a new FloatEntity
 */
Entity *yeCreateFloat(double value, Entity *father)
{
  FloatEntity *ret;
  YE_ALLOC_ENTITY(ret, FloatEntity);
  yeInit((Entity *)ret, NULL, YFLOAT, father);
  ret->value = value;
  return ((Entity *)ret);
}

/**
 * @param value
 * @param fathers  the fathers of the entity to create
 * @return  a new StructEntity
 */
Entity *yeCreateStruct(Entity *father)
{
  StructEntity *ret;
  YE_ALLOC_ENTITY(ret, StructEntity);
  yeInit(YE_TO_ENTITY(ret), NULL, STRUCT, father);
  ret->len = 0;
  ret->values = NULL;
  return (YE_TO_ENTITY(ret));
}

/**
 * @param value   the name of the function
 * @param fathers  the fathers of the entity to create
 * @return  a new FunctionEntity
 */
Entity *yeCreateFunction(const char *value, Entity *father)
{
  FunctionEntity *ret;
  YE_ALLOC_ENTITY(ret, FunctionEntity);
  yeInit((Entity *)ret, NULL, FUNCTION, father);
  ret->nArgs = 0;
  if (value == NULL)
    ret->value = NULL;
  else
    ret->value = strdup(value);
  /* char buf[1024]; */
  /* entityToString((Entity *)ret, buf, 1024); */
  return ((Entity *)ret);
}

/**
 * @param value
 * @param fathers  the fathers of the entity to create
 * @return  a new StringEntity
 */
Entity *yeCreateString(const char *string, Entity *father)
{
  StringEntity *ret;
  YE_ALLOC_ENTITY(ret, StringEntity);
  yeInit((Entity *)ret, NULL, YSTRING, father);
  if (string == NULL) {
    ret->value = NULL;
    ret->len = 0;
  } else {
    ret->value = strdup(string);
    ret->len = strlen(string);
  }
  return ((Entity *)ret);
}

static void destroyChild(Entity *entity)
{
  for (int i = 0, end = yeLen(entity); i < end; ++i)
    yeDestroy(yeGet(entity, i));
}

/**
 * @param entity
 */
void yeDestroyInt(Entity *entity)
{
  YE_DESTROY_ENTITY(entity, IntEntity);
}

/**
 * @param entity
 */
void yeDestroyFloat(Entity *entity)
{
  YE_DESTROY_ENTITY(entity, FloatEntity);
}

/**
 * @param entity
 */
void yeDestroyFunction(Entity *entity)
{
  YE_DESTROY_ENTITY(entity, FunctionEntity);
}

/**
 * @param entity
 */
void yeDestroyString(Entity *entity)
{
  if (((StringEntity *)entity)->value != NULL &&
      entity->refCount == 1) {
    free(((StringEntity *)entity)->value);
  }
  YE_DESTROY_ENTITY(entity, StringEntity);
}

/**
 * @TODO: TO IMPLEMENT
 * @param entity
 */
void yeDestroyStruct(Entity *entity)
{
  destroyChild(entity);
  YE_DESTROY_ENTITY(entity, StructEntity);
}

/**
 * @TODO: TO IMPLEMENT
 * @param entity
 */
void yeDestroyStatic(Entity *entity)
{
  YE_DESTROY_ENTITY(entity, StaticEntity);
}

/**
 * @param entity
 */
void yeDestroyArray(Entity *entity)
{
  destroyChild(entity);
  YE_DESTROY_ENTITY(entity, ArrayEntity);
}

void yeDestroy(Entity *entity)
{
  destroyTab[entity->type](entity);
}

/**
 * Create a new entity of type <type>
 * @param type        the type of the entity we want to create
 * @param fathers      the fathers of the entity to create
 * @param typeAyyar   the type of content to create an ArrayEntity
 */
Entity *yeCreate(EntityType type, Entity *father)
{
  switch (type)
    {
    case STRUCT:
      return (yeCreateStruct(father));
    case YSTRING:
      return (yeCreateString(NULL, father));
    case YINT:
      return (yeCreateInt(0, father));
    case YFLOAT:
      return (yeCreateFloat(0, father));
    case STATIC:
      return (yeCreateStatic(NULL, father));
    case ARRAY:
      return (yeCreateArray(father));
    case FUNCTION:
      return (yeCreateFunction(NULL, father));
    default:
      DPRINT_ERR( "%s generic constructor not yet implemented\n",
		  yeTypeToString(type));
      break;
    }
  return (NULL);
}

/**
 * Will add new entity to the array entity
 * @param entity    the entity to manage
 * @param size      the new size
 * @param arraytype the type of the ArrayEntity's values
 * @return the ArrayEntity
 */
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
  for (; i < size; ++i) {
    yeAttach(YE_TO_ENTITY(entity), NULL, i);
  }
  entity->len = size;
  return entity;
}

/**
 * Will add new entity to the array entity
 * @param entity    the entity to manage
 * @param size      the new size
 * @param arraytype the type of the ArrayEntity's values
 * @return the ArrayEntity if entity is an ArrayEntity, NULL otherwise
 */
Entity *yeExpandArray(Entity *entity, unsigned int size)
{
  if (!checkType(entity, ARRAY)) {
    DPRINT_ERR("yeExpandArray: bad entity\n");
    return (NULL);
  }
  return ((Entity*)manageArrayInternal((ArrayEntity*)entity, size));
}

/**
 * Add a new entity to the entity <entity>
 * @param entity  the entity where we will add a new entity
 * @param toPush  the entity to add
 */
void	yePushBack(Entity *entity, Entity *toPush)
{
  int	len;

  if (!checkType(entity, ARRAY)) {
    DPRINT_ERR("yePushBack: bad entity, should be of type array instead of %s\n",
	       yeTypeToString( yeType(entity)));
    return;
  }
  len = yeLen(entity);
  yeExpandArray(entity, len + 1);
  yeAttach(entity, toPush, len);
}

Entity *yeArrayRemove(Entity *array, Entity *toRemove)
{
  int	len;
  Entity *tmp = NULL;
  Entity *ret;

  if (!checkType(array, ARRAY)) {
    DPRINT_ERR("yeArrayRemove: bad entity\n");
    return NULL;
  }
  len = yeLen(array);
  for (int i = 0; i < len; ++i) {
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


/**
 * @param entity
 * @return  the entity that is erased from the entity <entity>
 */
Entity *yePopBack(Entity *entity)
{
  int	len;
  Entity *ret;

  if (!checkType(entity, ARRAY)) {
    DPRINT_ERR("yePopBack: bad entity\n");
    return NULL;
  }
  len = yeLen(entity);
  ret = yeGet(entity, len - 1);
  yeExpandArray(entity, len - 1);
  return (ret);
}

static void yeAttachFather(Entity *father, Entity *ref)
{
  if (father == NULL || ref == NULL)
    return;
  if (father->fathers == NULL)
    father->fathers = malloc(sizeof(Entity *));
  else
    father->fathers = realloc(father->fathers, sizeof(Entity *) * father->refCount);
  father->fathers[father->refCount] = ref;
}

/**
 * Set basic information to the entity <entity>
 * @param entity  the entity to set the basic informations
 * @param name    the name to set
 * @param type    the type of the entity
 * @param fathers  the parent entity of <entity>
 * @return the entity <entity>
 */
Entity *yeInit(Entity *entity, const char *name, EntityType type, Entity *father)
{
  if (!entity)
    return NULL;
  if (name != NULL && entity->name != NULL)
    free((char *)entity->name);
  if (name == NULL) {
    entity->name = NULL;
  }  else {
    entity->name = strdup(name);
  }
  entity->type = type;
  entity->fathers = NULL;
  yeAttachFather(father, entity);
  return (entity);
}


/**
 * Set a value to a StringEntity. Free the value if <entity> already had one
 * @param entity  the StringEntity to set the string to
 * @param val     the string to set to the StringEntity
 */
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
  if (on->type != ARRAY || on->type != STRUCT)
    return -1;
  if (yeLen(on) >= idx)
    return -1;
  YE_TO_ARRAY(on)->values[idx] = entity;
  yeAttachFather(on, entity);
  return 0;
}

void	yeSetStringAt(Entity *entity, unsigned int index, const char *value)
{
  return yeSetString(yeGet(entity, index), value);
}

int	yeSetIntAt(Entity *entity, unsigned int index, int value)
{
  return yeSetInt(yeGet(entity, index), value);
}

int	yeSetFloatAt(Entity *entity, unsigned int index, double value)
{
  return yeSetFloat(yeGet(entity, index), value);
}

void	yeSetStringAtStrIdx(Entity *entity, const char *index, const char *value)
{
  return yeSetString(yeGet(entity, index), value);
}

int	yeSetIntAtStrIdx(Entity *entity, const char *index, int value)
{
  return yeSetInt(yeGet(entity, index), value);
}

int	yeSetFloatAtStrIdx(Entity *entity, const char *index, double value)
{
  return yeSetFloat(yeGet(entity, index), value);
}

/* int	setElemAt(Entity *entity, int index, const char *value) */
/* { */
/*     switch (entity->type) */
/*     { */
      
/*     } */
/* } */


void	yeUnsetFunction(Entity *entity)
{
  yeSetFunction(entity, NULL);
  yeSetFunctionArgs(entity, 0);
}

/**
 * Free the entity's value and set the new value to the entity
 * @param entity
 * @param value
 * @return return <value>
 */
const char	*yeSetFunction(Entity *entity, const char *value)
{
  if (((FunctionEntity *)(entity))->value != NULL)
    free(((FunctionEntity *)(entity))->value);
  if (value != NULL)
    ((FunctionEntity *)(entity))->value = strdup(value);
  else
    ((FunctionEntity *)(entity))->value = NULL;
  return (value);
}

void	yeSetFunctionArgs(Entity *entity, unsigned int nArgs)
{
  ((FunctionEntity *)entity)->nArgs = nArgs;
}


/**
 * @parap entity
 * @param value
 * @return -1 if entity is not og type YINT, <value> otherwise
 */
int	yeSetInt(Entity *entity, int value)
{
  DPRINT("setInt: set %s with a value of %d\n", yeName(entity), value);
  if (!checkType(entity, YINT)) {
    RETURN_ERROR_BAD_TYPE("setInt", entity, -1);
  }
  ((IntEntity *)entity)->value = value;
  return (value);
}

/**
 * @parap entity
 * @param value
 * @return -1 if entity is not og type YFLOAT, <value> otherwise
 */
int	yeSetFloat(Entity *entity, double value)
{
  DPRINT("setFloat: set %s with a value of %f\n", yeName(entity), value);
  if (!checkType(entity, YFLOAT)) {
    RETURN_ERROR_BAD_TYPE("setFloat", entity, -1);
  }
  ((FloatEntity *)entity)->value = value;
  return (value);
}

/**
 * @param entity
 * @return the string value 
 */
const char *yeGetString(Entity *entity)
{
  if (!checkType(entity, YSTRING)) {
    RETURN_ERROR_BAD_TYPE("getStringVal", entity, NULL);
  }
  return ((StringEntity *)entity)->value;
}

/**
 * @parap entity
 * @param value
 * @return -1 if entity is not og type YINT, <value> otherwise
 */
int	yeGetInt(Entity *entity)
{
  if (!checkType(entity, YINT)) {
    RETURN_ERROR_BAD_TYPE("getIntVal", entity, -1);
  }
  return YE_TO_INT(entity)->value;
}

/**
 * @param entity
 * @return the entity's value if entity is of type FUNCTION, NULL otherwise
 */
const char	*yeGetFunction(Entity *entity)
{
  if (!checkType(entity, FUNCTION)) {
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

/**
 * @param entity
 * @return the entity's value if entity is of type YFLOAT, -1 otherwise
 */
double	yeGetFloat(Entity *entity)
{
  if (!checkType(entity, YFLOAT)) {
    RETURN_ERROR_BAD_TYPE("yeGetFloat", entity, -1);
  }
  return ((FloatEntity *)entity)->value;
}

/**
 * @param entity
 * @return the entity's name
 */
const char *yeName(const Entity *entity)
{
  return (entity->name);
}

/**
 * @param entity
 * @return the entity's fathers
 */
Entity **yeFathers(Entity *entity)
{
  return (entity->fathers);
}

/**
 * @param entity
 * @return the entity's name if entity is not null, "(null)" otherwise
 */
const char *yePrintableName(const Entity *entity)
{
  if (entity == NULL)
    return ("(null)");
  return (yeName(entity));
}
  
/**
 * @param entity
 * @return the entity's structure's name if entity is not null, "(null)" otherwise
 */
const char *yePrintableStructName(const Entity *entity)
{
  if (entity == NULL)
    return ("(null)");
  return (YE_TO_C_STRUCT(entity)->structName);
}


/**
 * @param src   the entity to copy from
 * @param des   the entity to copy to
 */
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
    case STRUCT:
      yeCopyStruct((StructEntity*)src, (StructEntity*)dest);
      break;
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
    case ARRAY:
      DPRINT_WARN("Unimpleted case ! line %d\n", __LINE__);
      break;
    case FUNCTION:
      nArgs = yeFunctionNumberArgs(src);
      strVal = yeGetFunction(src);
      DPRINT_INFO("\t\tvalue is function '%s'\n", strVal);
      yeSetFunction(dest, strVal);
      yeSetFunctionArgs(dest, nArgs);
      break;
    case STATIC:
      DPRINT_WARN("Unimpleted case ! line %d\n", __LINE__);
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

StructEntity*		yeCopyStruct(StructEntity* src, StructEntity* dest)
  {
    unsigned int i;

    if (src == NULL || dest == NULL)
      return NULL;
    DPRINT_INFO("There is %d attributes in '%s'\n", yeLen((Entity*)src),
		yePrintableStructName((const Entity*)src));
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
  static Entity (*testInfLoop[256]); //array use to store structure alerady call to kepp entitytostring to infinitely recusively call himself
  static int tifIndex = 0;
  (void)testInfLoop; /*c'est juste que si on ne fait rien avec testInfLoop le compilateur sous windows fait une erreur*/
  
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
    case FUNCTION: 
      if (yeGetFunction(entity) == NULL)
	ETS_RETURN (snprintf(buf, sizeBuf, "function %s: (null)",
			     yePrintableName(entity)));
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
    case ARRAY:
      strcpy(buf, "["); // may bug here if sizeBuf is too small
      ETS_INCR_RET(1);
      for (i = 0; i < yeLen(entity); ++i)
	{
	  retETS = ETS_REC_CALL(yeGetIdx(entity, i), buf, sizeBuf);
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
    case STATIC:
      strcpy(buf, "static of ("); // may bug here if sizeBuf is too small
      ETS_INCR_RET(8);
      retETS = ETS_REC_CALL(((StaticEntity *)entity)->value, buf, sizeBuf);
      if (retETS < 0)
	goto error;
      ETS_INCR_RET(retETS);
      strcpy(buf, ")"); // may bug here if sizeBuf is too small
      ETS_INCR_RET(1);
      ETS_RETURN (ret);
    case STRUCT:
      retETS = snprintf(buf, sizeBuf, "%s : {\n", yePrintableName(entity));
      if (retETS < 0)
	goto error;
      ETS_INCR_RET(retETS);
      for (i = 0; i < yeLen(entity); ++i)
	{
	  /* printf("in for\n"); */
	  retETS = snprintf(buf, sizeBuf, "%s : ",
			    yePrintableName(yeGetIdx(entity, i)));
	  if (retETS < 0)
	    goto error;
	  /* printf("cur buf(name): %s\n", buf); */
	  ETS_INCR_RET(retETS);
	  testInfLoop[tifIndex] = yeGetIdx(entity, i);
	  ++tifIndex;
	  retETS = ETS_REC_CALL(yeGetIdx(entity, i), buf, sizeBuf);
	  if (retETS < 0)
	    goto error;
	  /* printf("cur buf(val): %s\n", buf); */
	  ETS_INCR_RET(retETS);
	  if (i + 1 < yeLen(entity))
	    {
	      strcpy(buf, ",\n");  // may bug here if sizeBuf is too small
	      /* printf("cur buf(style): %s\n", buf); */
	      ETS_INCR_RET(2);
	    }
	  else
	    {
	      strcpy(buf, "\n");  // may bug here if sizeBuf is too small
	      /* printf("cur buf(style): %s\n", buf); */
	      ETS_INCR_RET(1);
	    }
	}
      strcpy(buf, "}");  // may bug here if sizeBuf is too small
      ETS_INCR_RET(1);
      buf[0] = 0;
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
