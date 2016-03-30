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

  yuiDebugInit(); //Can not be init twice :)
  TRY_OR_DIE(ygInitGameConfig(&cfg, gamePath, SDL2), -1);
  TRY_OR_DIE(ygInit(&cfg), die(-1, &cfg));
  printf("mod: %p\n", ygLoadMod("../../../modules/sm-reader/"));
  printf("mod(again): %p\n", ygGetMod("sm-reader"));
  printf("func: %p\n", yeGet(ygGetMod("sm-reader"), "load-map"));
  printf("map: %p\n", (char *)yesCall(yeGet(ygGetMod("sm-reader"), "load-map"),
				      "./test.sm"));
  return die(0, &cfg);
}

#undef TRY_OR_DIE
