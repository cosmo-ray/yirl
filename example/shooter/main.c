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
#include "utils.h"

static const char *gamePath = "./"; 

static inline int die(int ret, GameConfig *cfg)
{
  ygCleanGameConfig(cfg);
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
  TRY_OR_DIE(ygInitGameConfig(&cfg, gamePath, SDL2), die(-1, &cfg));
  TRY_OR_DIE(ygInit(&cfg), die(-1, &cfg));
  TRY_OR_DIE(ygStartLoop(&cfg), die(-1, &cfg));
  return die(0, &cfg);
}

#undef TRY_OR_DIE
