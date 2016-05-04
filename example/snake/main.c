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
  TRY_OR_DIE(ygInitGameConfig(&cfg, gamePath, CURSES), -1);
  TRY_OR_DIE(ygInit(&cfg), die(-1, &cfg));
  TRY_OR_DIE(ygStartLoop(&cfg), die(-1, &cfg));
  return die(0, &cfg);
}

#undef TRY_OR_DIE
