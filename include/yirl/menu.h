/*
**Copyright (C) 2015 Matthias Gatto
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

/*
 * Entity description:
 * {
 * "<type>": "menu",
 * "pre-text": "OPTIONAL TEXT BEFORE MENU",
 * "entries": [
 *  {
 *    "text": "xxx",
 *    "action": FUNC
 *  } // ...
 *  ],
 * "mn-type": "panel", // optional, use if panel needed
 * // TODO
 * }
 */

#ifndef	_YIRL_MENU_H_
#define	_YIRL_MENU_H_ 10

#include "yirl/widget.h"

int ywMenuInit(void);
int ywMenuEnd(void);
int ywMenuGetCurrent(YWidgetState *opac);
int ywMenuGetThreshold(YWidgetState *state);

static inline Entity *ywMenuSliderFromEntry(Entity *entry)
{
	return yeGet(yeGet(entry, "subentries"), yeGetIntAt(entry, "slider_idx"));
}

static inline int ywMenuGetCurrentByEntity(Entity *entity) {
	return ywMenuGetCurrent(ywidGetState(entity));
}

Entity *ywMenuGetCurrentEntry(Entity *entity);

void ywMenuSetCurrentEntry(Entity *entity, Entity *newCur);

void ywMenuClear(Entity *menu);

int ywMenuHasChange(YWidgetState *opac);
int ywMenuPosFromPix(Entity *wid, uint32_t x, uint32_t y);

void ywMenuDown(Entity *wid);
void ywMenuUp(Entity *wid);
void *ywMenuMove(Entity *ent, uint32_t at);

Entity *ywMenuPushEntryByEnt(Entity *menu, const char *name, Entity *func);
#ifdef Y_INSIDE_TCC
static inline Entity *
ywMenuPushEntryByf(Entity *menu, const char *name,
		   void *(*func)(int, void **))
{
	Entity *r;
	yeAutoFree Entity *f = yeCreateFunctionExt(
		0, ygGetTccManager(), NULL, NULL, YE_FUNC_NO_FASTPATH_INIT);

	YE_TO_FUNC(f)->fastPath = func;
	r = ywMenuPushEntryByEnt(menu, name, f);
	return r;
}

#define ywMenuPushEntry(m, n, f)				\
	_Generic(f,						\
		 Entity *: ywMenuPushEntryByEnt,		\
		 void * (*) (int, void **): ywMenuPushEntryByf,	\
		 void *: ywMenuPushEntryByf	\
		)(m,n,f)

#else

static inline Entity *
ywMenuPushEntry(Entity *menu, const char *name, Entity *func)
{
	return ywMenuPushEntryByEnt(menu, name, func);
}

#endif

/*
 * slider_array: an array or vector entity:
 * [{"text": SLIDER_OPTION_NAME, "action": CALLBACK}, ...]
 */
Entity *ywMenuPushSlider(Entity *menu, const char *name, Entity *slider_array);

/*
 * same as ywMenuPushSlider, but push a slide down menu, and subentries is optionel
 */
Entity *ywMenuPushSlideDownSubMenu(Entity *menu, const char *name, Entity *subentries);

Entity *ywMenuPushTextInput(Entity *menu, const char *name);

Entity *ywMenuGetEntry(Entity *menu, int idx);

_Bool ywMenuRemoveLastEntry(Entity *menu);

static inline Entity *ywMenuGetCurSliderSlide(Entity *menu)
{
	Entity *entry = ywMenuGetCurrentEntry(menu);
	Entity *slider = yeGet(entry, "slider");
	int slider_idx;

	if (!slider)
		return NULL;
	slider_idx = yeGetIntAt(entry, "slider_idx");
	return yeGet(slider, slider_idx);
}

static inline int ywMenuNbEntries(Entity *mn)
{
	return yeLenAt(mn, "entries");
}


static inline Entity *ywMenuLoaderPercent(Entity *loader)
{
	return yeGet(loader, "loading-bar-%");
}

static inline void ywMenuSetLoaderPercent(Entity *loader, int val)
{
	yeSetAt(loader, "loading-bar-%", val);
}

InputStatue ywMenuCallActionOnByEntity(Entity *opac, Entity *event, int idx);
InputStatue ywMenuCallActionOnByState(YWidgetState *opac, Entity *event,
				      int idx);

#ifndef Y_INSIDE_TCC

#define ywMenuCallActionOn(wid, eve, idx)			\
	_Generic((wid),						\
		 Entity * : ywMenuCallActionOnByEntity,		\
		 YWidgetState * : ywMenuCallActionOnByState	\
		)(wid, eve, idx)

#else

#define ywMenuCallActionOn ywMenuCallActionOnByEntity

#endif

#endif
