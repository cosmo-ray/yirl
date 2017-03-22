#include <unistd.h>
#include "entity-script.h"
#include "game.h"

static const char *gamePath = "./";

static inline int die(int ret, GameConfig *cfg)
{
  ygCleanGameConfig(cfg);
  ygEnd();
  return ret;
}

#define TRY_OR_DIE(cmd, die)	 do {		\
    if (cmd)					\
      return die;				\
  } while (0);

int main(void)
{
  yeInitMem();
  GameConfig cfg;
  char buff[1024];
  Entity *modDesc = yeCreateArray(NULL, NULL);
  Entity *tmp;
  Entity *map;
  Entity *path;

  yuiDebugInit(); //Can not be init twice :)
  TRY_OR_DIE(ygInitGameConfig(&cfg, gamePath, NONE), -1);
  TRY_OR_DIE(ygInit(&cfg), die(-1, &cfg));
  /* put current path inside buff */
  getcwd(buff, 1024);
  yuistrcpy(buff + yuistrlen(buff), "/test.sm");
  path = yeCreateString(buff, NULL, NULL);

  tmp = yeCreateArray(modDesc, NULL);
  yeCreateString(".", tmp, "map-char");
  tmp = yeCreateArray(modDesc, NULL);
  yeCreateString("#",tmp, "map-char");
  tmp = yeCreateArray(modDesc, NULL);
  yeCreateString("_", tmp, "map-char");

  ygLoadMod("../../../modules/sm-reader/");
  map = yesCall(yeGet(ygGetMod("sm-reader"), "load-map"),
		path, modDesc);
  printf("map: %s\n", yeToCStr(map, 4, 0));
  printf("m-d: %s\n", yeToCStr(modDesc, 3, 0));

  YE_DESTROY(path);
  YE_DESTROY(map);
  YE_DESTROY(modDesc);
  return die(0, &cfg);
}

#undef TRY_OR_DIE
