/*
** Copyright 2015 Kara adrien
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

#include <glib.h>
#include <stdint.h>
#include <unistd.h>
#include "tests.h"
#include "sound.h"
#include "game.h"

void testYSoundLib(void)
{

  GameConfig cfg;

  g_assert(!ygInitGameConfig(&cfg, NULL, YNONE));
  g_assert(!ygInit(&cfg));

  /* Working check */
  /* g_assert(ySoundLoad("./BlablablaMrFreeman.mp3") == 0); */
  /* g_assert(ySoundPlay(0) != -1); */
  /* sleep(3); */
  /* g_assert(ySoundPlayLoop("42", "BlablablaMrFreeman.mp3") != -1); */
  /* g_assert(ySoundStatus(0) != -1); */
  /* g_assert(ySoundLevel(0, 10) != -1); */
  /* g_assert(ySoundPlayLoop(0) != -1); */
  /* sleep(7); */

  /* Bad request check */
  g_assert(ySoundLoad("404.wav.false") == -1);

  ySoundStop(0);
  ygEnd();
  ygCleanGameConfig(&cfg);
}
