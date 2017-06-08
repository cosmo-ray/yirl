#include	"entity.h"

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
