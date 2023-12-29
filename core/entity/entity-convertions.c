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
	    entity->type = YSTRING;
	    YE_TO_STRING(entity)->value = (void *)YE_TO_INT(entity)->value;
	    return entity;
    case YFUNCTION:
    case BAD_TYPE:
    case YARRAY:
    case YHASH:
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
    case YHASH:
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
    case YHASH:
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
    case YHASH:
    case BAD_TYPE:
    case NBR_ENTITYTYPE:
      return NULL;
    }
    break;

  case BAD_TYPE:
  case NBR_ENTITYTYPE:
  case YARRAY:
  case YHASH:
  case YFUNCTION:
    return NULL;
  }
  return NULL;
}

Entity *yeConvert(Entity *entity, int type)
{
  switch (yeType(entity)) {
    /*-- int --*/
  case YINT:
    switch (type) {
    case YINT:
	    return entity;
    case YFLOAT:
      return yeBrutalCast(entity, YFLOAT);
    case YDATA:
    case YSTRING:
    case YFUNCTION:
    case YHASH:
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
      return yeBrutalCast(entity, YINT);
    case YDATA:
    case YSTRING:
      /* Not Posible Because of the stupdity of small entity */
    case YFUNCTION:
    case YHASH:
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
	const char *c_tmp;
	char *to_free = yeStringFreeable(entity);

	c_tmp = yeGetString(entity);
	Entity *tmp = yeCreateString(c_tmp, NULL, NULL);
	entity->type = YARRAY;
	yBlockArrayInit(&YE_TO_ARRAY(entity)->values, ArrayEntry);
	yePushBack(entity, tmp, NULL);
	yeDestroy(tmp);
	free(to_free);
	return entity;
      }
    case YINT:
    case YFLOAT:
    case YDATA:
    case YFUNCTION:
    case YHASH:
    case BAD_TYPE:
    case NBR_ENTITYTYPE:
      return NULL;
    }
    break;

    /*-- data --*/
  case YDATA:
      return NULL;
    break;

  case YHASH:
  case YARRAY:
  case BAD_TYPE:
  case NBR_ENTITYTYPE:
  case YFUNCTION:
    return NULL;
  }
  return NULL;
}
