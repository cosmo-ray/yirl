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
#include "tests.h"

static void *attack(va_list ap)
{
  printf("atack time !\n");
  return NULL;
}

void testSukeFightMod(void)
{
  yeInitMem();
  GameConfig cfg;
  Entity *fight_example;
  Entity *mod;
  YWidgetState *wid;

  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));
  mod = ygLoadMod(TESTS_PATH"../modules/sukeban-fight/");
  g_assert(mod);
  fight_example = ygFileToEnt(YJSON,
			      TESTS_PATH
			      "../modules/sukeban-fight/combat_example.json",
			      NULL);
  g_assert(fight_example);
  ysRegistreNativeFunc("attack", attack);
  yeCreateFunctionSimple("attack", ysNativeManager(), mod);
  wid = ywidNewWidget(fight_example, NULL);
  g_assert(wid);
  ywidSetMainWid(wid);
  ygDoLoop();
  yeDestroy(fight_example);
  ygCleanGameConfig(&cfg);
  ygEnd();
}
