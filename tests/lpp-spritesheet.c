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

#include "yirl/game.h"
#include "yirl/entity-script.h"
#include "tests.h"

void testsLpcSpritesheet(void)
{
  yeInitMem();
  GameConfig cfg;
  Entity *canvas = yeCreateArray(NULL, NULL);
  Entity *mod;

  /* YWidgetState *wid; */
  /* Entity *actions; */

  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));
  mod = ygLoadMod(TESTS_PATH"../modules/Universal-LPC-spritesheet/");
  if (!mod) {
    printf("can't load Universal-LPC-spritesheet\n"
	   "you might need to init that submodule\n");
    goto exit;
  }

 exit:
  yeDestroy(canvas);
  ygCleanGameConfig(&cfg);
  ygEnd();
}
