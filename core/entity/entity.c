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
  destroyStructEntity,
  destroyIntEntity,
  destroyFloatEntity,
  destroyStringEntity,
  destroyRefEntity,
  destroyArrayEntity,
  destroyFunctionEntity,
  destroyRefEntity, // TODO replace by destructor for generic
  destroyStaticEntity
};

const char * EntityTypeStrings[] = { "struct", "int", "float", "string", "ref",  "array", "function", "gen", "static" };

int	tryGetType(const Entity *entity)
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

EntityType stringToEntityType(const char *str)
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
const char *entityTypeToString(int type)
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
unsigned int getLen(Entity *entity)
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
Entity *getEntity(Entity *entity, unsigned int index)
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
    //if (((StaticEntity *)tmp)->value->type == FUNCTION)
    ((StaticEntity *)tmp)->value->father = tmp->father;
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
static Entity *findDirectEntityWithEnd(Entity *entity, const char *name, int end)
{
  int	i = 0;
  Entity *tmp;
  while ((tmp = getEntity(entity, i)) != NULL) {
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
Entity *findDirectEntity(Entity *entity, const char *name)
{
  DPRINT_INFO("findDirectEntity %s in %s \t[%s:%d]\n", name, tryGetEntityName(entity), __FILE__, __LINE__);
  unsigned int	i = 0;
  Entity *tmp;
  while ((tmp = getEntity(entity, i)) != NULL)
  {
    if (yStrEqual(tryGetEntityName(tmp), name))
    	return (tmp);
    ++i;
  }
  DPRINT_WARN("could not find %s\n", name);
  return NULL;
}

/**
 * @param entity  the entity whe are looking into
 * @param name    the entity name whe are looking for
 * @return        The found Entity named <name> in <entity>
 */
Entity *findEntity(Entity *entity, const char *name)
{
  int	i;

  if (entity == NULL) {
    DPRINT_ERR("can not find entity fot %s\n", name);
    return NULL;
    }
  DPRINT_INFO("finding entity %s in entity %s\t[%s:%d]\n", name, tryGetEntityName(entity), __FILE__, __LINE__);
  i = findIdxPoint(name);
  return (i != -1) ?
    (yeGet(findDirectEntityWithEnd(entity, name, i), name + i + 1)) :
    (findDirectEntity(entity, name));
}

/**
 * @param value   value of the InEntity
 * @param father  the parent entity of the new entity
 * @return  return the new created entity
 */
Entity *creatIntEntity(int value, Entity *father)
{
  IntEntity *ret;
  ALLOC_ENTITY(ret, IntEntity);
  setEntityBasicInfo((Entity *)ret, NULL, YINT, father);
  ret->value = value;
  return ((Entity *)ret);
}

/**
 * @param contentType   the type of the content
 * @param father        the father of the created entity
 * @return  return a new ArrayEntity 
 */
Entity *creatArrayEntity(EntityType contentType, Entity *father)
{
  DPRINT_INFO("create array\n");
  ArrayEntity *ret;
  ALLOC_ENTITY(ret, ArrayEntity);
  setEntityBasicInfo((Entity *)ret, NULL, ARRAY, father);
  ret->len = 0;
  ret->values = NULL;
  ret->contentType = contentType;
  return ((Entity *)ret);
}

/**
 * @param value   the entity to reference
 * @param father  the father of the entity to create
 * @return  a new RefEntity
 */
NONNULL(1)  Entity *creatRefEntity(Entity *value, Entity *father)
{
  (void) father;
  if (value == NULL) {
    LOG_ERR("can not create a ref to a NULL entity\n");
    return NULL;
  }
  yeIncrRef(value);
  return value;
}

/**
 * @param value
 * @param father  the father of the entity to create
 * @return  a new StaticEntity
 */
Entity *creatStaticEntity(Entity *value, Entity *father)
{
  StaticEntity *ret;
  ALLOC_ENTITY(ret, StaticEntity);
  setEntityBasicInfo((Entity *)ret, NULL, STATIC, father);
  ret->value = value;
  return ((Entity *)ret);
}

/**
 * @param value
 * @param father  the father of the entity to create
 * @return  a new FloatEntity
 */
Entity *creatFloatEntity(double value, Entity *father)
{
  FloatEntity *ret;
  ALLOC_ENTITY(ret, FloatEntity);
  setEntityBasicInfo((Entity *)ret, NULL, YFLOAT, father);
  ret->value = value;
  return ((Entity *)ret);
}

/**
 * @param value
 * @param father  the father of the entity to create
 * @return  a new StructEntity
 */
Entity *creatStructEntity(Entity *father)
{
  StructEntity *ret;
  ALLOC_ENTITY(ret, StructEntity);
  setEntityBasicInfo(YE_TO_ENTITY(ret), NULL, STRUCT, father);
  ret->len = 0;
  ret->values = NULL;
  return (YE_TO_ENTITY(ret));
}

/**
 * @param value   the name of the function
 * @param father  the father of the entity to create
 * @return  a new FunctionEntity
 */
Entity *creatFunctionEntity(const char *value, Entity *father)
{
  FunctionEntity *ret;
  ALLOC_ENTITY(ret, FunctionEntity);
  setEntityBasicInfo((Entity *)ret, NULL, FUNCTION, father);
  ret->nArgs = 0;
  ret->args = NULL;
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
 * @param father  the father of the entity to create
 * @return  a new StringEntity
 */
Entity *creatStringEntity(const char *string, Entity *father)
{
  StringEntity *ret;
  ALLOC_ENTITY(ret, StringEntity);
  setEntityBasicInfo((Entity *)ret, NULL, YSTRING, father);
  if (string == NULL) {
    ret->value = NULL;
    ret->len = 0;
  } else {
    ret->value = strdup(string);
    ret->len = strlen(string);
  }
  return ((Entity *)ret);
}

/**
 * @param entity
 */
void destroyIntEntity(Entity *entity)
{
  DESTROY_ENTITY(entity, IntEntity);
}

/**
 * @param entity
 */
void destroyFloatEntity(Entity *entity)
{
  DESTROY_ENTITY(entity, FloatEntity);
}

/**
 * @param entity
 */
void destroyFunctionEntity(Entity *entity)
{
  if (((FunctionEntity *)entity)->args != NULL &&
      entity->refCount == 1)
    free(((FunctionEntity *)entity)->args);
  DESTROY_ENTITY(entity, FunctionEntity);
}

/**
 * @param entity
 */
void destroyStringEntity(Entity *entity)
{
  if (((StringEntity *)entity)->value != NULL &&
      entity->refCount == 1)
    free(((StringEntity *)entity)->value);
  DESTROY_ENTITY(entity, StringEntity);
}

/**
 * @TODO: TO IMPLEMENT
 * @param entity
 */
void destroyStructEntity(Entity *entity)
{
  //TODO: free nested entity
  DESTROY_ENTITY(entity, StructEntity);
}

/**
 * @TODO: TO IMPLEMENT
 * @param entity
 */
void destroyRefEntity(Entity *entity)
{
  yeDecrRef(entity);
}

/**
 * @TODO: TO IMPLEMENT
 * @param entity
 */
void destroyStaticEntity(Entity *entity)
{
  DESTROY_ENTITY(entity, StaticEntity);
}

/**
 * @param entity
 */
void destroyArrayEntity(Entity *entity)
{
  unsigned int	i = 0;

  while (i < ((ArrayEntity *)entity)->len)
  {
    destroyTab[((ArrayEntity *)entity)->contentType](((ArrayEntity *)entity)->values[i]);
    ++i;
  }
  DESTROY_ENTITY(entity, ArrayEntity);
}

void destroyEntity(Entity *entity)
{
  destroyTab[entity->type](entity);
}

/**
 * Create a new entity of type <type>
 * @param type        the type of the entity we want to create
 * @param father      the father of the entity to create
 * @param typeAyyar   the type of content to create an ArrayEntity
 */
Entity *genericCreatEntity(EntityType type, Entity *father, EntityType typeAyyar)
{
  /* printf("gen create with type:%s!\n", entityTypeToString(type)); */
  switch (type)
    {
    case STRUCT:
      return (creatStructEntity(father));
    case YSTRING:
      return (creatStringEntity(NULL, father));
    case YINT:
      return (creatIntEntity(0, father));
    case YFLOAT:
      return (creatFloatEntity(0, father));
    case STATIC:
      return (creatStaticEntity(NULL, father));
    case ARRAY:
      return (creatArrayEntity(typeAyyar, father));
    case FUNCTION:
      return (creatFunctionEntity(NULL, father));
    default:
      DPRINT_ERR( "%s generic constructor not yet implemented\n", entityTypeToString(type));
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
ArrayEntity	*manageArrayEntity(ArrayEntity *entity, unsigned int size, EntityType arrayType)
{
  unsigned int	i;
  char	buf[1024];

  if (entity->len == 0) {
    entity->values = malloc(sizeof(Entity *) * size);
    i = 0;
  } else {
    entity->values = realloc(entity->values, sizeof(Entity *) * size);
    i = entity->len;
  }
  while (i < size)
  {
    /* DPRINT_INFO("%s create a %d\n", entity->name, entity->contentType); */
    entity->values[i] = genericCreatEntity(entity->contentType, (Entity*)entity, arrayType );
    snprintf(buf, 1024, "%s.At%di%d", entity->name, arrayType, i);
    DPRINT_INFO("set name:%s\n", buf);
    setEntityBasicInfo(entity->values[i], buf, entity->contentType, (Entity*)entity);
    ++i;
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
Entity *manageArray(Entity *entity, unsigned int size, EntityType arrayType)
{
  if (!checkType(entity, ARRAY)) {
    DPRINT_ERR("manageArray: bad entity\n");
    return (NULL);
  }
  return ((Entity*)manageArrayEntity((ArrayEntity*)entity, size, arrayType));
}

/**
 * Add a new entity to the entity <entity>
 * @param entity  the entity where we will add a new entity
 * @param toPush  the entity to add
 */
void	pushBack(Entity *entity, Entity *toPush)
{
  int	len;
  EntityType arrayType = YINT;

  if (!checkType(entity, ARRAY)) {
    DPRINT_ERR("pushBack: bad entity, should be of type array instead of %s\n", entityTypeToString( tryGetType(entity)));
    return;
  }
  len = getLen(entity);
  if (len > 1 && yeGet(entity, 0)->type == ARRAY)
    arrayType = ((ArrayEntity *)yeGet(entity, 0))->contentType;

  manageArrayEntity((ArrayEntity*)entity, len + 1, arrayType);
  ((ArrayEntity *)entity)->values[len] = toPush;
  //setAt(entity, len, toPush);
}

Entity *arrayRemove(Entity *array, Entity *toRemove, int deepSearch)
{
  int	len;
  Entity *tmp = NULL;
  Entity *ret;

  if (!checkType(array, ARRAY)) {
    DPRINT_ERR("popBack: bad entity\n");
    return NULL;
  }
  len = getLen(array);
  for (int i = 0; i < len; ++i) {
    tmp = yeGet(array, i);
    ret = tmp;
    if (tmp != toRemove && (tryGetType(tmp) == REF) && deepSearch)
      tmp = getReferencedObj(tmp);
    if (tmp == toRemove) {
      for (int i2 = i + 1; i2 < len; ++i, ++i2) {
	YE_TO_ARRAY(array)->values[i] = YE_TO_ARRAY(array)->values[i2];
      }
      popBack(array);
      return ret;
    }
  }
  return NULL;
}


/**
 * @param entity
 * @return  the entity that is erased from the entity <entity>
 */
Entity *popBack(Entity *entity)
{
  EntityType arrayType = YINT;
  int	len;
  Entity *ret;

  if (!checkType(entity, ARRAY)) {
    DPRINT_ERR("popBack: bad entity\n");
    return NULL;
  }
  len = getLen(entity);
  ret = yeGet(entity, 0);
  if (len > 1 && ret->type == ARRAY)
    arrayType = YE_TO_ARRAY(ret)->contentType;
  ret = yeGet(entity, len - 1);
  manageArrayEntity((ArrayEntity*)entity, len - 1, arrayType);
  return (ret);
}

/**
 * Set basic information to the entity <entity>
 * @param entity  the entity to set the basic informations
 * @param name    the name to set
 * @param type    the type of the entity
 * @param father  the parent entity of <entity>
 * @return the entity <entity>
 */
Entity *setEntityBasicInfo(Entity *entity, const char *name, EntityType type, Entity *father)
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
  entity->father = father;
  return (entity);
}


/**
 * Set a value to a StringEntity. Free the value if <entity> already had one
 * @param entity  the StringEntity to set the string to
 * @param val     the string to set to the StringEntity
 */
void	setString(Entity *entity, const char *val)
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

void	setStringAt(Entity *entity, unsigned int index, const char *value)
{
  return setString(yeGet(entity, index), value);
}

int	setIntAt(Entity *entity, unsigned int index, int value)
{
  return setInt(yeGet(entity, index), value);
}

int	setFloatAt(Entity *entity, unsigned int index, double value)
{
  return setFloat(yeGet(entity, index), value);
}

void	setStringAtStrIdx(Entity *entity, const char *index, const char *value)
{
  return setString(yeGet(entity, index), value);
}

int	setIntAtStrIdx(Entity *entity, const char *index, int value)
{
  return setInt(yeGet(entity, index), value);
}

int	setFloatAtStrIdx(Entity *entity, const char *index, double value)
{
  return setFloat(yeGet(entity, index), value);
}

/* int	setElemAt(Entity *entity, int index, const char *value) */
/* { */
/*     switch (entity->type) */
/*     { */
      
/*     } */
/* } */


void	unsetFunction(Entity *entity)
{
  setFunction(entity, NULL);
  setFunctionArgs(entity, 0, NULL);
}

/**
 * Free the entity's value and set the new value to the entity
 * @param entity
 * @param value
 * @return return <value>
 */
const char	*setFunction(Entity *entity, const char *value)
{
  if (((FunctionEntity *)(entity))->value != NULL)
    free(((FunctionEntity *)(entity))->value);
  if (value != NULL)
    ((FunctionEntity *)(entity))->value = strdup(value);
  else
    ((FunctionEntity *)(entity))->value = NULL;
  return (value);
}

void	setFunctionArgs(Entity *entity, unsigned int nArgs, EntityType *args)
{
  ((FunctionEntity *)entity)->nArgs = nArgs;
  if (((FunctionEntity *)entity)->args != NULL)
    free(((FunctionEntity *)entity)->args);
  ((FunctionEntity *)entity)->args = args;
}


/**
 * @parap entity
 * @param value
 * @return -1 if entity is not og type YINT, <value> otherwise
 */
int	setInt(Entity *entity, int value)
{
  DPRINT("setInt: set %s with a value of %d\n", getName(entity), value);
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
int	setFloat(Entity *entity, double value)
{
  DPRINT("setFloat: set %s with a value of %f\n", getName(entity), value);
  if (!checkType(entity, YFLOAT)) {
    RETURN_ERROR_BAD_TYPE("setFloat", entity, -1);
  }
  ((FloatEntity *)entity)->value = value;
  return (value);
}

/**
 * @param entity
 * @return the entity content type if entity is of type REF, YINT otherwise
 */
EntityType getContentType(const Entity *entity)
{
  if (!checkType(entity, ARRAY)) {
    RETURN_ERROR_BAD_TYPE("getContentType", entity, YINT);
  }
  return (((ArrayEntity*)entity)->contentType);
}

/**
 * @param entity
 * @return the string value if entity is of type YSTRING, NULL otherwise
 */
const char *getStringVal(Entity *entity)
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
int	getIntVal(Entity *entity)
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
const char	*getFunctionVal(Entity *entity)
{
  if (!checkType(entity, FUNCTION)) {
    RETURN_ERROR_BAD_TYPE("getFunctionVal", entity, NULL);
  }
  return YE_TO_FUNC(entity)->value;
}

EntityType getFunctionArg(const Entity *entity, int i)
{
  if (!entity) {
    DPRINT_WARN("getFunctionArg: can not get arh from an NULL entity");
    return (YINT);
  }
  if (i > getFunctionNumberArgs(entity)) {
      DPRINT_ERR("try to get an arguments superior to the number of argument in a function\n");
      return (YINT);
    }
  return (YE_TO_FUNC(entity)->args[i]);
}

int	getFunctionNumberArgs(const Entity *entity)
{
  if (!entity) {
    DPRINT_WARN("getFunctionNumberArgs: entity is NULL");
    return (YINT);
  }  
  return YE_TO_C_FUNC(entity)->nArgs;
}

/**
 * @param entity
 * @return the entity's value if entity is of type YFLOAT, -1 otherwise
 */
double	getFloatVal(Entity *entity)
{
  if (!checkType(entity, YFLOAT)) {
    RETURN_ERROR_BAD_TYPE("getFloatVal", entity, -1);
  }
  return ((FloatEntity *)entity)->value;
}

/**
 * function for retrocompatibility should be destroy
 */
Entity *getReferencedObj(Entity *entity)
{
  return entity;
}

/**
 * @param entity
 * @return the entity's name
 */
const char *getName(const Entity *entity)
{
  return (entity->name);
}

/**
 * @param entity
 * @return the entity's father
 */
Entity *getFather(Entity *entity)
{
  return (entity->father);
}

/**
 * @param entity
 * @param type
 * @return 1 if entity is not null and entity's type is the same as <type>, 0 otherwise
 */
int	checkType(const Entity *entity, EntityType type)
{
  return (entity != NULL && entity->type == type);
}

/**
 * @param entity
 * @return the entity's name if entity is not null, "(null)" otherwise
 */
const char *tryGetEntityName(const Entity *entity)
{
  if (entity == NULL)
    return ("(null)");
  return (getName(entity));
}
  
/**
 * @param entity
 * @return the entity's structure's name if entity is not null, "(null)" otherwise
 */
const char *tryGetStructEntityName(const Entity *entity)
{
  if (entity == NULL)
    return ("(null)");
  return (YE_TO_C_STRUCT(entity)->structName);
}

/**
 * Will create an Entity of the same type as <src> and of type YINT
 * @param src     the entity to copy
 * @param name    the name of the new entity
 * @param father  the father of the new entity
 * @return
 */
Entity*		createCopyOf(Entity *src, const char *name, Entity *father)
{
  Entity *ret = genericCreatEntity(src->type, father, YINT);
  setEntityBasicInfo(ret, name, src->type, father);
  return (copyEntity(src, ret));
}

/**
 * @param src   the entity to copy from
 * @param des   the entity to copy to
 */
Entity*		copyEntity(Entity* src, Entity* dest)
{
  const char* strVal = NULL;
  int	nArgs;
  EntityType *args = NULL;

  if (src != NULL && dest != NULL
      && tryGetType(src) == tryGetType(dest)) {
    DPRINT_INFO("\tentity '%s' are '%s'\n", tryGetEntityName(src), entityTypeToString(tryGetType(src)));
    switch (tryGetType(src))
    {
    case STRUCT:
      copyStructEntity((StructEntity*)src, (StructEntity*)dest);
      break;
    case YINT:
      setInt(dest, getIntVal(src));
      break;
    case YFLOAT:
      setFloat(dest, getFloatVal(src));
      break;
    case YSTRING:
      strVal = getStringVal(src);
      DPRINT_INFO("\t\tvalue is string \"%s\"\n", (strVal != NULL) ? strVal : "null");
      setString(dest, strVal);
      break;
    case ARRAY:
      DPRINT_WARN("Unimpleted case ! line %d\n", __LINE__);
      break;
    case FUNCTION:
      nArgs = getFunctionNumberArgs(src);
      args = malloc(nArgs * sizeof(EntityType));

      int i = 0;
      while (i < nArgs)
	{
	  args[i] = getFunctionArg(src, i);
	  ++i;
	}
      strVal = getFunctionVal(src);
      DPRINT_INFO("\t\tvalue is function '%s'\n", strVal);
      setFunction(dest, strVal);
      setFunctionArgs(dest, nArgs, args);
      break;
    case GEN:
      DPRINT_WARN("Unimpleted case ! line %d\n", __LINE__);
      break;
    case STATIC:
      DPRINT_WARN("Unimpleted case ! line %d\n", __LINE__);
      break;
    }
    return dest;
  }
  return NULL;
}

StructEntity*		copyStructEntity(StructEntity* src, StructEntity* dest)
  {
    unsigned int i;

    if (src == NULL || dest == NULL)
      return NULL;
    DPRINT_INFO("There is %d attributes in '%s'\n", getLen((Entity*)src), tryGetStructEntityName((const Entity*)src));
    for (i = 0; i < getLen((Entity*)src) && i < getLen((Entity*)dest); i++)
      {
	copyEntity(src->values[i], dest->values[i]);
      }
    return dest;
  }


#define	ETS_REC_CALL(A,B,C) entityToString(A,B,C);++nbrSpace;
#define	ETS_RETURN(X) --nbrSpace;return(X);
#define	ETS_INCR_RET(I)	\
  ret += I;\
  sizeBuf -= I;\
  buf += I
int entityToString(Entity *entity, char *buf, int sizeBuf)
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
  switch (tryGetType(entity))
    {
    case YSTRING:
      ETS_RETURN (snprintf(buf, sizeBuf, "%s", getStringVal(entity)));
    case YINT:
      //printf("val: %d\n", getIntVal(entity));
      ETS_RETURN (snprintf(buf, sizeBuf, "%d", getIntVal(entity)));
    case YFLOAT:
      //printf("val: %f\n", getFloatVal(entity));
      ETS_RETURN (snprintf(buf, sizeBuf, "%f", getFloatVal(entity)));
    case FUNCTION: 
      if (getFunctionVal(entity) == NULL)
	ETS_RETURN (snprintf(buf, sizeBuf, "function %s: (null)", tryGetEntityName(getReferencedObj(entity))));
      retETS = snprintf(buf, sizeBuf, "function %s: nb arg: %d\n", tryGetEntityName(getReferencedObj(entity)), getFunctionNumberArgs(entity));
      if (retETS < 0)
	goto error;
      ETS_INCR_RET(retETS);
      retETS = snprintf(buf, sizeBuf, "function to call: %s", getFunctionVal(entity));
      if (retETS < 0)
	goto error;
      ETS_INCR_RET(retETS);
      ETS_RETURN (ret);
    case ARRAY:
      strcpy(buf, "["); // may bug here if sizeBuf is too small
      ETS_INCR_RET(1);
      for (i = 0; i < getLen(entity); ++i)
	{
	  retETS = ETS_REC_CALL(getEntity(entity, i), buf, sizeBuf);
	  if (retETS < 0)
	    goto error;
	  ETS_INCR_RET(retETS);
	  if (getLen(entity) > i + 1)
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
    case GEN:
      strcpy(buf, "lol");
      ETS_RETURN (3);
    case STRUCT:
      retETS = snprintf(buf, sizeBuf, "%s : {\n", tryGetEntityName(entity));
      if (retETS < 0)
	goto error;
      ETS_INCR_RET(retETS);
      for (i = 0; i < getLen(entity); ++i)
	{
	  /* printf("in for\n"); */
	  retETS = snprintf(buf, sizeBuf, "%s : ", tryGetEntityName(getEntity(entity, i)));
	  if (retETS < 0)
	    goto error;
	  /* printf("cur buf(name): %s\n", buf); */
	  ETS_INCR_RET(retETS);
	  testInfLoop[tifIndex] = getEntity(entity, i);
	  ++tifIndex;
	  retETS = ETS_REC_CALL(getEntity(entity, i), buf, sizeBuf);
	  if (retETS < 0)
	    goto error;
	  /* printf("cur buf(val): %s\n", buf); */
	  ETS_INCR_RET(retETS);
	  if (i + 1 < getLen(entity))
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
  DPRINT_ERR("Error occured in entityToString on %s\n", tryGetEntityName(entity));
  nbrSpace = 0;
  return (-1);
}
#undef	ETS_REC_CALL
#undef	ETS_RETURN
#undef	ETS_INCR_RET
