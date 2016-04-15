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
  GameConfig cfg;
  char buff[1024];
  Entity *modDesc = yeCreateArray(NULL, NULL);
  Entity *map;

  yuiDebugInit(); //Can not be init twice :)
  TRY_OR_DIE(ygInitGameConfig(&cfg, gamePath, SDL2), -1);
  TRY_OR_DIE(ygInit(&cfg), die(-1, &cfg));
  /* put current path inside buff */
  getcwd(buff, 1024);
  strcpy(buff + strlen(buff), "/test.sm");

  yeCreateInt(0, modDesc, ".");
  yeCreateInt(1, modDesc, "#");
  yeCreateInt(2, modDesc, "_");
  ygLoadMod("../../../modules/sm-reader/");
  map = yesCall(yeGet(ygGetMod("sm-reader"), "load-map"), buff, modDesc);
  printf("map: %s\n", yeToString(map, 2, 0));
  printf("m-d: %s\n", yeToString(modDesc, 3, 0));

  YE_DESTROY(map);
  YE_DESTROY(modDesc);
  return die(0, &cfg);
}

#undef TRY_OR_DIE
