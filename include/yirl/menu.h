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
 *    "action": FUNC   // see "Action callbacks" below
 *  } // ...
 *  ],
 * "mn-type": "panel", // optional, set to "panel" for horizontal layout
 * "pre-text": "text", // optional text to display before entries
 * "current": 0,       // index of the current selection
 * "moveOn": FUNC,    // callback (menu_ent, idx, entry) called on move
 * "onEsc": FUNC,     // callback (menu_ent, idx, entry) called on Escape
 *
 * // Entries properties:
 * // "hiden": 1,     // entry is invisible and skipped
 * // "disabled": 1,  // entry is visible but inactive and skipped
 * }
 *
 * Action callbacks
 * ----------------
 * "moveOn", "on" and "onEsc" all receive the current entry as their last
 * argument:
 *
 *     func(menu, idx, entry)
 *
 * "action" is the exception, it receives only the menu and the event:
 *
 *     func(menu, event)
 *
 * The entry owning the action is NOT passed. The entry is an ordinary
 * entity owned by the caller, so the caller can store its own data on it
 * at build time and read it back to know which entry fired:
 *
 *     Entity *entry = ywMenuGetCurrentEntry(menu);
 *     int idx = yeGetIntAt(entry, "my-own-key");
 *
 * For a slider or subentries entry, ywMenuMove() moves between the options
 * and the OPTION is dispatched, not the entry. ywMenuGetCurrentEntry() then
 * returns the slider entry, and the selected option is obtained with:
 *
 *     Entity *option = ywMenuGetCurSliderSlide(menu);
 *
 * or, starting from an entry, with ywMenuSliderFromEntry() and
 * ywMenuSliderFromEntryAt().
 *
 * An entry with "disabled" set is skipped by ywMenuMove() and never
 * dispatches anything: mnActions_() returns NOTHANDLE before calling the
 * action.
 *
 * To trigger the action of a known entry index:
 *
 *     ywMenuCallActionOn(menu, event, idx);
 *
 * All accessors used above are exposed to the script bindings, where a NULL
 * return is a NULL entity (null in JS).
 */

#ifndef	_YIRL_MENU_H_
#define	_YIRL_MENU_H_ 10

#include "yirl/widget.h"

int ywMenuInit(void);
int ywMenuEnd(void);
int ywMenuGetCurrent(YWidgetState *opac);
int ywMenuGetThreshold(YWidgetState *state);

static inline Entity *ywMenuSliderEntries(Entity *entry)
{
	Entity *sub = yeGet(entry, "subentries");
	if (!sub) {
		sub = yeGet(entry, "slide");
	}

	return sub;
}

static inline void ywMenuSliderClear(Entity *entry)
{
	Entity *sub = ywMenuSliderEntries(entry);
	yeSetAt(entry, "slider_idx", -1);
	yeClearArray(sub);
}

/* option of a slider or subentries entry, by index */
static inline Entity *ywMenuSliderFromEntryAt(Entity *entry, int at)
{
	Entity *sub = ywMenuSliderEntries(entry);
	return yeGet(sub, at);
}

/* option currently selected in a slider or subentries entry */
static inline Entity *ywMenuSliderFromEntry(Entity *entry)
{
	Entity *sub = ywMenuSliderEntries(entry);
	return yeGet(sub, yeGetIntAt(entry, "slider_idx"));
}

static inline int ywMenuGetCurrentByEntity(Entity *entity) {
	return ywMenuGetCurrent(ywidGetState(entity));
}

/*
 * Entry currently selected in a menu, NULL if the menu has none.
 *
 * This is the way to reach the entry from inside its own "action", which
 * only gets (menu, event). Note that for a slider or subentries entry this
 * returns the slider entry, not the selected option: use
 * ywMenuGetCurSliderSlide() for the latter.
 */
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

/* entry at index idx, NULL if out of range */
Entity *ywMenuGetEntry(Entity *menu, int idx);

_Bool ywMenuRemoveLastEntry(Entity *menu);

/*
 * Option currently selected in the current slider or subentries entry,
 * NULL if the current entry is a plain one.
 *
 * A slider entry dispatches its OPTION, so the option is what carries the
 * data an action needs.
 */
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

/*
 * Dispatch the action of the entry at index idx, and make it the current
 * entry. Like a user activation, the action only gets (menu, event), and a
 * slider or subentries entry dispatches its selected option.
 */
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
