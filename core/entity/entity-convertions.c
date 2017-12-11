#include <stdlib.h>
#include "entity.h"

Entity *yeBrutalCast(Entity *entity, int type)
{
  switch (yeType(entity)) {
    /*-- int --*/
  case YINT:
    switch (type) {
    case YINT:
      return entity;
    case YFLOAT:
      entity->type = YFLOAT;
      YE_TO_FLOAT(entity)->value = YE_TO_INT(entity)->value;
      return entity;
    case YDATA:
      entity->type = YDATA;
      YE_TO_DATA(entity)->value = (void *)YE_TO_INT(entity)->value;
      return entity;
    case YSTRING:
    case YFUNCTION:
    case BAD_TYPE:
    case YARRAY:
    case NBR_ENTITYTYPE:
      return NULL;
    }
    break;

    /*-- float --*/
  case YFLOAT:
    switch (type) {
    case YFLOAT:
      return entity;
    case YINT:
      entity->type = YINT;
      YE_TO_FLOAT(entity)->value = YE_TO_INT(entity)->value;
      return entity;
    case YDATA:
      entity->type = YDATA;
      YE_TO_DATA(entity)->value = (void *)YE_TO_INT(entity)->value;
      return entity;
    case YSTRING:
    case YFUNCTION:
    case YARRAY:
    case BAD_TYPE:
    case NBR_ENTITYTYPE:
      return NULL;
    }
    break;

    /*-- string --*/
  case YSTRING:
    switch (type) {
    case YSTRING:
      return entity;
    case YINT:
      entity->type = YINT;
      YE_TO_INT(entity)->value = (size_t)YE_TO_STRING(entity)->value;
      return entity;
    case YFLOAT:
      entity->type = YFLOAT;
      YE_TO_FLOAT(entity)->value = (size_t)YE_TO_STRING(entity)->value;
      return entity;
    case YDATA:
      entity->type = YSTRING;
      YE_TO_STRING(entity)->value = YE_TO_STRING(entity)->value;
      return entity;
    case YFUNCTION:
    case YARRAY:
    case BAD_TYPE:
    case NBR_ENTITYTYPE:
      return NULL;
    }
    break;

    /*-- data --*/
  case YDATA:
    switch (type) {
    case YDATA:
      return entity;
    case YINT:
      entity->type = YINT;
      YE_TO_INT(entity)->value = (size_t)YE_TO_DATA(entity)->value;
      return entity;
    case YFLOAT:
      entity->type = YFLOAT;
      YE_TO_FLOAT(entity)->value = (size_t)YE_TO_DATA(entity)->value;
      return entity;
    case YSTRING:
      entity->type = YSTRING;
      YE_TO_STRING(entity)->value = YE_TO_DATA(entity)->value;
      return entity;
    case YFUNCTION:
    case YARRAY:
    case BAD_TYPE:
    case NBR_ENTITYTYPE:
      return NULL;
    }
    break;

  case BAD_TYPE:
  case NBR_ENTITYTYPE:
  case YARRAY:
  case YFUNCTION:
    return NULL;
  }
  return NULL;
}

Entity *yeConvert(Entity *entity, int type)
{
  char *c_tmp;

  switch (yeType(entity)) {
    /*-- int --*/
  case YINT:
    switch (type) {
    case YINT:
	    return entity;
    case YFLOAT:
    case YDATA:
    case YSTRING:
    case YFUNCTION:
    case BAD_TYPE:
    case YARRAY:
    case NBR_ENTITYTYPE:
      return NULL;
    }
    break;

    /*-- float --*/
  case YFLOAT:
    switch (type) {
    case YFLOAT:
	    return entity;
    case YINT:
    case YDATA:
    case YSTRING:
    case YFUNCTION:
    case YARRAY:
    case BAD_TYPE:
    case NBR_ENTITYTYPE:
      return NULL;
    }
    break;

    /*-- string --*/
  case YSTRING:
    switch (type) {
    case YSTRING:
      return entity;
    case YARRAY:
      {
	Entity *str;
	int len = yeLen(entity);

	c_tmp = YE_TO_STRING(entity)->value;
	entity->type = YARRAY;
	yBlockArrayInit(&YE_TO_ARRAY(entity)->values, ArrayEntry);
	str = yeCreateString(NULL, entity, NULL);
	YE_TO_STRING(str)->value = c_tmp;
	YE_TO_STRING(str)->len = len;
	return entity;
      }
    case YINT:
    case YFLOAT:
    case YDATA:
    case YFUNCTION:
    case BAD_TYPE:
    case NBR_ENTITYTYPE:
      return NULL;
    }
    break;

    /*-- data --*/
  case YDATA:
      return NULL;
    break;

  case YARRAY:
  case BAD_TYPE:
  case NBR_ENTITYTYPE:
  case YFUNCTION:
    return NULL;
  }
  return NULL;
}
