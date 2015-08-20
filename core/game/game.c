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

#include "game.h"
/* description */
#include "json-desc.h"
#include "description.h"

/* scripting */
#include "lua-script.h"

#include "entity.h"
#include "curses-driver.h"
#include "sdl-driver.h"
#include "menu.h"
#include "map.h"
#include "text-screen.h"


static int init;
static void *jsonManager;
static void *luaManager;

static YDescriptionOps *parsers[MAX_NB_MANAGER];

#define CHECK_RET_AND_RET(operation, err_val, ret) do {	\
    if ((operation) == (err_val))			\
      return (ret);					\
  } while (0)

#define CHECK_RET_AND_GOTO(operation, err_val, label)		\
    if ((operation) == (err_val))				\
      goto label;

int ygInit(void)
{
  static int t;

  /* Init parseurs */
  CHECK_RET_AND_RET(t = ydJsonInit(), -1, -1);
  CHECK_RET_AND_RET(jsonManager = ydNewManager(t), NULL, -1);
  parsers[t] = jsonManager; 

  /* Init scripting */
  /* TODO init internal lua function */
  CHECK_RET_AND_RET(t = ysLuaInit(), -1, -1);
  CHECK_RET_AND_RET(luaManager = ysNewManager(NULL, t), NULL, -1);

  /* Init widgets */
  CHECK_RET_AND_RET(ywMenuInit(), -1, -1);
  CHECK_RET_AND_RET(ywMapInit(), -1, -1);
  CHECK_RET_AND_RET(ywTextScreenInit(), -1, -1);

  //TODO check which render to use :)
  ysdl2Init();
  ycursInit();
  CHECK_RET_AND_RET(ycursRegistreMenu(), -1, -1);
  CHECK_RET_AND_RET(ycursRegistreTextScreen(), -1, -1);
  CHECK_RET_AND_RET(ycursRegistreMap(), -1, -1);
  CHECK_RET_AND_RET(ysdl2RegistreTextScreen(), -1, -1);
  CHECK_RET_AND_RET(ysdl2RegistreMenu(), -1, -1);

  return 0;
}

void ygEnd()
{
  ydDestroyManager(jsonManager);
  ydJsonEnd();
  ywTextScreenEnd();
  ycursDestroy();
  ysdl2Destroy();
}

int ygStartLoop(GameConfig *config)
{
  char *tmp;
  Entity *mainMod;
  int ret = -1;

  if (!init) {
        DPRINT_ERR("ygInit() should be call before calling ygStartLoop");
    return -1;
  }
  if (!config || !config->rConf || !config->startingMod) {
    DPRINT_ERR("GameConfig is brocken(some ptr are NULL)");
    return -1;
  }
  if (!config->startingMod->path) {
    DPRINT_ERR("!config->startingMod->path is NULL");
    return -1;
  }


  tmp = g_strconcat(config->startingMod->path, "/start.json", NULL);
  if (!tmp) {
    DPRINT_ERR("can not allocated path(like something went really wrong)");
    return -1;
  }
  mainMod = ydFromFile(jsonManager, tmp);
  if (!mainMod) {
    DPRINT_ERR("fail to parse file: %s", tmp);
    goto cleanup;
  }

  ret = 0;
 cleanup:
  g_free(tmp);
  YE_DESTROY(mainMod);
  return ret;
}

#undef CHECK_RET_AND_GOTO
#undef CHECK_RET_AND_RET
