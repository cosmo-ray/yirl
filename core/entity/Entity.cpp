#include "entity.h"

void setAt(Entity *entity, unsigned int index, const char *value)
{
  setStringAt(entity, index, value);
}

void setAt(Entity *entity, unsigned int index, int value)
{
  setIntAt(entity, index, value);
}

void setAt(Entity *entity, unsigned int index, Entity *value)
{
  setRefAt(entity, index, value);
}

void setAt(Entity *entity, unsigned int index, float value)
{
  setFloatAt(entity, index, value);
}

void setAt(Entity *entity, const char *index, const char *value)
{
  setStringAtStrIdx(entity, index, value);
}
void setAt(Entity *entity, const char *index, int value)
{
  setIntAtStrIdx(entity, index, value);
}

void setAt(Entity *entity, const char *index, Entity *value)
{
  setRefAtStrIdx(entity, index, value);
}

void setAt(Entity *entity, const char *index, float value)
{
  setFloatAtStrIdx(entity, index, value);
}

Entity *yeGet(Entity *entity, unsigned int index)
{
  return getEntity(entity, index);
}

Entity *yeGet(Entity *entity, const char *name)
{
  return findEntity(entity, name);
}

