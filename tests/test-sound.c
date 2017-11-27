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

  g_assert(!ygInitGameConfig(&cfg, NULL, SDL2));
  g_assert(!ygInit(&cfg));

  /* Working check */
  g_assert(sound_load("./BlablablaMrFreeman.mp3") == 0);
  g_assert(sound_play(0) != -1);
  sleep(3);
  /* g_assert(sound_play_loop("42", "BlablablaMrFreeman.mp3") != -1); */
  /* g_assert(sound_status(0) != -1); */
  /* g_assert(sound_level(0, 10) != -1); */
  g_assert(sound_play_loop(0) != -1);
  sleep(7);

  /* Bad request check */
  g_assert(sound_load("404.wav.false") == -1);

  sound_stop(0);
  ygEnd();
}
