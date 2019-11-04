/*
**Copyright (C) 2017 Matthias Gatto
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

#include "yirl/game.h"
#include "yirl/entity-script.h"
#include "tests.h"

static void *doEvents(int n, union ycall_arg *args, int *types)
{
  static int state = 0;
  Entity *wid = args[0].e;
  Entity *eve = args[1].e;

  if (eve && (ywidEveType(eve) == YKEY_DOWN)) {
    if (ywidEveKey(eve) == ' ') {
      if (!state) {
	yesCall(ygGet("DialogueBox.remove"), wid, yeGet(wid, "dialogue"));
	yeRemoveChild(wid, "dialogue");

	Entity *mn = yeCreateArray(NULL, NULL);
	yeCreateString("hello I love U\nlet me jump in you're game", mn,
		       "text");
	Entity *entries = yeCreateArray(mn, "answers");
	Entity *entry = yeCreateArray(entries, NULL);
	yeCreateString("she's walking at the streett", entry, "text");

	entry = yeCreateArray(entries, NULL);
	yeCreateString("peoples are steange", entry, "text");
	yeCreateInt(1, entry, "hiden");

	entry = yeCreateArray(entries, NULL);
	yeCreateString("faces look ugly and you're alone", entry, "text");
	yesCall(ygGet("DialogueBox.new_menu"), wid, 10, 10, mn,
		wid, "dialogue");
	yeDestroy(mn);
      } else if (state == 1) {
	yesCall(ygGet("DialogueBox.moveAnswer"), yeGet(wid, "dialogue"), 1);
      } else if (state == 2) {
	Entity *answer = yesCall(ygGet("DialogueBox.getAnswer"),
				 yeGet(wid, "dialogue"), 1);
	yeSetAt(answer, "hiden", 0);
	yesCall(ygGet("DialogueBox.reload"), wid, yeGet(wid, "dialogue"));
      } else if (state == 3) {
	yesCall(ygGet("DialogueBox.remove"), wid, yeGet(wid, "dialogue"));
      }
      ++state;
      return (void *)ACTION;
    }
  }
  return (void *)NOTHANDLE;
}

void testDialogueBox(void)
{
  yeInitMem();
  GameConfig cfg;
  Entity *canvas = yeCreateArray(NULL, NULL);
  YWidgetState *wid;
  Entity *actions;

  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));
  g_assert(ygLoadMod(TESTS_PATH"../modules/dialogue-box/"));
  yeCreateString("canvas", canvas, "<type>");
  ysRegistreNativeFunc("doEvents", doEvents);
  actions = yeCreateArray(canvas, "actions");
  yeCreateString("doEvents", actions, NULL);
  yeCreateString("QuitOnKeyDown", actions, NULL);

  yeCreateString("rgba: 180 40 200 255", canvas, "background");
  yeCreateArray(canvas, "objs");

  wid = ywidNewWidget(canvas, NULL);
  yesCall(ygGet("DialogueBox.new_text"), canvas, 10, 10,
	  "hello I love you\nwon't you give me you're name",
	  canvas, "dialogue");
  /* ygCall("DialogueBox.new", canvas, 10, 10, "hello I love you"); */
  g_assert(wid);
  ywidSetMainWid(wid);
  ygDoLoop();
  ygCleanGameConfig(&cfg);
  ygEnd();
}
