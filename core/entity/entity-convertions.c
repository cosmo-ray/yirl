#include <stdlib.h>
#include "entity.h"
#include <inttypes.h>

Entity *yeBrutalCast(Entity *entity, int type)
{
  switch (yeType(entity)) {
    /*-- int --*/
  case YINT:
    switch (type) {
    case YINT:
      return entity;
    case YQUADINT:
      entity->type = YQUADINT;
      YE_TO_QINT(entity)->x = (int)YE_TO_INT(entity)->value;
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
    case YVECTOR:
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
      YE_TO_INT(entity)->value = YE_TO_FLOAT(entity)->value;
      return entity;
    case YQUADINT:
      entity->type = YQUADINT;
      YE_TO_QINT(entity)->x = YE_TO_FLOAT(entity)->value;
      return entity;
    case YDATA:
      entity->type = YDATA;
      YE_TO_DATA(entity)->value = (void *)YE_TO_INT(entity)->value;
      return entity;
    case YSTRING:
    case YFUNCTION:
    case YARRAY:
    case YVECTOR:
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
    case YVECTOR:
    case YQUADINT:
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
    case YVECTOR:
    case YQUADINT:
    case YHASH:
    case BAD_TYPE:
    case NBR_ENTITYTYPE:
      return NULL;
    }
    break;

  case BAD_TYPE:
  case NBR_ENTITYTYPE:
  case YARRAY:
  case YQUADINT:
  case YVECTOR:
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
		case YQUADINT:
		case YHASH:
		case YVECTOR:
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
		case YQUADINT:
		case YARRAY:
		case YVECTOR:
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
		case YVECTOR:
		case YQUADINT:
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

	case YVECTOR:
		break;
	case YHASH:
		break;
	case YARRAY:
		switch (type) {
		case YVECTOR:
		{
			yeAutoFree Entity *tmp = yeCreateCopy2(entity, NULL, NULL, 1);
			yeClearArray(entity);
			yBlockArrayFree(&YE_TO_ARRAY(entity)->values);
			entity->type = YVECTOR;
			YE_TO_VECTOR(entity)->data = (void *)yeMetadata(entity, VectorEntity);
			YE_TO_VECTOR(entity)->len = 0;
			YE_TO_VECTOR(entity)->max = yeMetadataSize(VectorEntity) / sizeof(Entity *);
			for (size_t i = 0; i < yeLen(tmp); ++i) {
				yePushBack(entity, yeGet(tmp, i), NULL);
			}
			return entity;
		}
		break;
		case YHASH:
		{
			yeAutoFree Entity *tmp = yeCreateCopy2(entity, NULL, NULL, 1);
			yeClearArray(entity);
			yBlockArrayFree(&YE_TO_ARRAY(entity)->values);
			entity->type = YHASH;
			YE_TO_HASH(entity)->values = kh_init(entity_hash);
			for (size_t i = 0; i < yeLen(tmp); ++i) {
				char *key = yeGetKeyAt(tmp, i);
				static char buf[1024];

				if (unlikely(!key)) {
					snprintf(buf, 1023, "i-"PRIint64, i);
					key = buf;
				}
				yePush(entity, yeGet(tmp, i), key);
			}
			return entity;
		}
			break;
		default:
			break;
		}
		break;
	case YQUADINT:
	case BAD_TYPE:
	case NBR_ENTITYTYPE:
	case YFUNCTION:
		return NULL;
	}
	return NULL;
}
