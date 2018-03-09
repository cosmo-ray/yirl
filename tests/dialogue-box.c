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

void testDialogueBox(void)
{
  yeInitMem();
  GameConfig cfg;
  Entity *canvas = yeCreateArray(NULL, NULL);
  YWidgetState *wid;

  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));
  g_assert(ygLoadMod(TESTS_PATH"../modules/dialogue-box/"));
  yeCreateString("canvas", canvas, "<type>");
  yeCreateString("QuitOnKeyDown", canvas, "action");
  yeCreateString("rgba: 180 40 200 255", canvas, "background");
  yeCreateArray(canvas, "objs");

  wid = ywidNewWidget(canvas, NULL);
  yesCall(ygGet("DialogueBox.new"), canvas, 10, 10,
	  "hello I love you\nwon't you give me you're name");
  /* ygCall("DialogueBox.new", canvas, 10, 10, "hello I love you"); */
  g_assert(wid);
  ywidSetMainWid(wid);
  ygDoLoop();
  yeDestroy(canvas);
  ygCleanGameConfig(&cfg);
  ygEnd();
}
