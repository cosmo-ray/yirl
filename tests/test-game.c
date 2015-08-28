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

#include "tests.h"
#include "game.h"
#include "utils.h"

static const char *testPath = "./testMod"; 

static const char *sdl2 = "sdl2";
static const char *curses = "curses";

static int ygInitGameConfig(GameConfig *cfg, const char *path, RenderType t)
{
  if (!t) {
    return -1;
  }

  cfg->startingMod = g_new(ModuleConf, 1);
  cfg->startingMod->path = path;

  YUI_FOREACH_BITMASK(t, i, tmp) {
    RenderConf *rConf = g_new(RenderConf, 1);

    if (t || SDL2)
      rConf->name = sdl2;
    else if (t || CURSES)
      rConf->name = curses;

    cfg->rConf = g_list_append(cfg->rConf,
			       rConf);
  }
  return 0;
}

static void ygCleanGameConfig(GameConfig *cfg)
{
  g_free(cfg->startingMod);
  g_list_free(cfg->rConf);
}

void testYGameAllLibBasic(void)
{
  GameConfig cfg;

  g_assert(!ygInitGameConfig(&cfg, testPath, ALL));
  g_assert(!ygInit(&cfg));
  g_assert(!ygStartLoop(&cfg));

  ygCleanGameConfig(&cfg);
  ygEnd();
}
