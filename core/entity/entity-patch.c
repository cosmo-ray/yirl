/*
**Copyright (C) 2018 Matthias Gatto
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

#include "entity.h"

enum PatchOperation
  {
   YEP_NONE = 0,
   YEP_ADD,
   YEP_MOD,
   YEP_SUP,
   YEP_CHANGE_TYPE,
   YEP_LAST,
  };

const char *patchOpToStr[YEP_LAST] =
  {
   "YEP_NONE",
   "YEP_ADD",
   "YEP_MOD",
   "YEP_SUP",
   "YEP_CHANGE_TYPE"
  };

enum PatchIdxInfo
  {
   YEP_OPERATION = 0,
   YEP_NAME_IDX = 1,
   YEP_ELEM = 2,
  };

static void doPatch(Entity *orginalEntity, Entity *patchEntity, Entity *patch);

static inline Entity *patchCreate(Entity *father, const char *name)
{
  Entity *ret = yeCreateArray(father, name);
  yeCreateIntAt(YEP_NONE, ret, NULL, YEP_OPERATION);
  yeCreateIntAt(YEP_NONE, ret, NULL, YEP_NAME_IDX);
  return ret;
}

static inline void tryCreateChildArray(Entity *patch)
{
  if (unlikely(yeGetIntAt(patch, YEP_OPERATION) == YEP_NONE)) {
    yeSetIntAt(patch, YEP_OPERATION, YEP_MOD);
    yeCreateArrayAt(patch, NULL, YEP_ELEM);
  }
}

static inline void pushPatchChildElem(Entity *patch, Entity *tmpPatch)
{
  tryCreateChildArray(patch);
  yePushBack(yeGet(patch, YEP_ELEM), tmpPatch, NULL);
}

static inline Entity *getPatchChildElem(Entity *patch)
{
  tryCreateChildArray(patch);
  return patchCreate(yeGet(patch, YEP_ELEM), NULL);
}

static void doPatchArray(Entity *orginalEntity, Entity *patchEntity, Entity *patch)
{
  (void)patchEntity;
  int ol = yeLen(orginalEntity), pl = yeLen(patchEntity), l = ol > pl ? ol : pl;
  int isArray = yeIsPureArray(orginalEntity);

  if (isArray) {
    for (int i = 0; i < l; ++i) {
      Entity *cur = yeGet(orginalEntity, i);
      Entity *other = yeGet(patchEntity, i);

      if (i < ol && i < pl) {
	Entity *patchChildElem = patchCreate(NULL, NULL);

	doPatch(cur, other, patchChildElem);
	if (yeGetIntAt(patchChildElem, YEP_OPERATION) != YEP_NONE) {
	  pushPatchChildElem(patch, patchChildElem);
	  yeCreateIntAt(i, patchChildElem, NULL , YEP_NAME_IDX);
	}
	yeDestroy(patchChildElem);
      } else if (i < pl) {
	Entity *patchChildElem = getPatchChildElem(patch);

	yeSetIntAt(patchChildElem, YEP_OPERATION, YEP_ADD);
	yePushAt(patchChildElem, other, YEP_ELEM);
	yeCreateIntAt(i, patchChildElem, NULL , YEP_NAME_IDX);
      } else {
	Entity *patchChildElem = getPatchChildElem(patch);

	yeSetIntAt(patchChildElem, YEP_OPERATION, YEP_SUP);
	yeCreateIntAt(i, patchChildElem, NULL, YEP_NAME_IDX);
      }
    }
  } else {
    YE_ARRAY_FOREACH_ENTRY(orginalEntity, entry) {
      if (!entry->name)
	continue;
      Entity *other = yeGet(patchEntity, entry->name);
      if (!other) {
	Entity *patchChildElem = getPatchChildElem(patch);

	yeSetIntAt(patchChildElem, YEP_OPERATION, YEP_SUP);
	yeCreateStringAt(entry->name, patchChildElem, NULL, YEP_NAME_IDX);
      } else {
	Entity *patchChildElem = patchCreate(NULL, NULL);

	doPatch(entry->entity, other, patchChildElem);
	if (yeGetIntAt(patchChildElem, YEP_OPERATION) != YEP_NONE) {
	  pushPatchChildElem(patch, patchChildElem);
	  yeCreateStringAt(entry->name, patchChildElem, NULL , YEP_NAME_IDX);
	}
	yeDestroy(patchChildElem);
      }
    }
    YE_ARRAY_FOREACH_ENTRY(patchEntity, entry2) {
      if (!entry2->name)
	continue;
      Entity *other = yeGet(orginalEntity, entry2->name);
      if (!other) {
	Entity *patchChildElem = getPatchChildElem(patch);

	yeSetIntAt(patchChildElem, YEP_OPERATION, YEP_ADD);
	yePushAt(patchChildElem, entry2->entity, YEP_ELEM);
	yeCreateStringAt(entry2->name, patchChildElem, NULL , YEP_NAME_IDX);
      }
    }
  }
}

static inline void patchDoDiffType(Entity *patchEntity, Entity *patch)
{
  yeSetIntAt(patch, YEP_OPERATION, YEP_CHANGE_TYPE);
  yePushAt(patch, patchEntity, YEP_ELEM);
  return;
}

#define patchCheckDoSameType(orginalEntity, patchEntity,		\
			     patch, type)				\
  ({									\
    if (!yeEqual(orginalEntity, patchEntity))				\
      {									\
	yeSetIntAt(patch, YEP_OPERATION, YEP_MOD);			\
	yeCreate##type##At(yeGet##type(patchEntity), patch,		\
			   NULL, YEP_ELEM);				\
      }									\
  })

static void doPatch(Entity *orginalEntity, Entity *patchEntity, Entity *patch)
{
  switch (yeType(orginalEntity)) {
  case YARRAY:
    if (yeType(patchEntity) != YARRAY)
      return patchDoDiffType(patchEntity, patch);
    return doPatchArray(orginalEntity, patchEntity, patch);
  case YFLOAT:
    if (yeType(patchEntity) != YFLOAT)
      return patchDoDiffType(patchEntity, patch);
    return patchCheckDoSameType(orginalEntity, patchEntity, patch, Float);
  case YSTRING:
    if (yeType(patchEntity) != YSTRING)
      return patchDoDiffType(patchEntity, patch);
    return patchCheckDoSameType(orginalEntity, patchEntity, patch, String);
  case YINT:
    if (yeType(patchEntity) != YINT)
      return patchDoDiffType(patchEntity, patch);
    return patchCheckDoSameType(orginalEntity, patchEntity, patch, Int);
  default:
    DPRINT_ERR("type not handle\n");
  }
}

#undef patchCheckDoSameType

Entity *yePatchCreate(Entity *orginalEntity, Entity *patchEntity,
		      Entity *father, const char *name)
{
	if (!orginalEntity || !patchEntity)
		return NULL;
	Entity *ret = patchCreate(father, name);

	doPatch(orginalEntity, patchEntity, ret);
	return ret;
}

void yePatchAply(Entity *dest, Entity *patch)
{
	yePatchAplyExt(dest, patch, 0);
}

void yePatchAplyExt(Entity *dest, Entity *patch, uint32_t flag)
{
	if (yeGetIntAt(patch, YEP_OPERATION) != YEP_MOD)
		return;

	Entity *elem = yeGet(patch, YEP_ELEM);
	int t = yeType(elem);

	if (t == YSTRING || t == YINT || t == YFLOAT) {
		yeCopy(elem, dest);
		return;
	}
	YE_ARRAY_FOREACH(elem, sub_el) {
		int operation = yeGetIntAt(sub_el, YEP_OPERATION);

		if (yeType(yeGet(sub_el, YEP_NAME_IDX)) == YSTRING) {
			const char *key = yeGetStringAt(sub_el, YEP_NAME_IDX);

			if (operation == YEP_ADD ||
			    operation == YEP_CHANGE_TYPE) {
				yeReplaceBack(dest, yeGet(sub_el, YEP_ELEM),
					      key);
			} else if (operation == YEP_SUP &&
				   !(flag & YE_PATCH_NO_SUP)) {
				yeRemoveChild(dest, key);
			} else if (operation == YEP_MOD) {
				Entity *sub_dest = yeGet(dest, key);
				yePatchAplyExt(sub_dest, sub_el, flag);
			}

		} else  {
			int idx = yeGetIntAt(sub_el, YEP_NAME_IDX);

			if (operation == YEP_ADD ||
			    operation == YEP_CHANGE_TYPE) {
				yePushAt(dest, yeGet(sub_el, YEP_ELEM), idx);
			} else if (operation == YEP_SUP &&
				   !(flag & YE_PATCH_NO_SUP)) {
				yeRemoveChild(dest, idx);
			} else if (operation == YEP_MOD) {
				Entity *sub_dest = yeGet(dest, idx);
				yePatchAplyExt(sub_dest, sub_el, flag);
			}
		}
	}
}

