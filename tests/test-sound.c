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
#include "tests.h"
#include "sound.h"

void testYSoundLib(void)
{
    sound_init(LIB_VLC);

    /* g_assert check */
    g_assert(TRUE);

    /* Working check */
    g_assert(sound_play("Test", "BlablablaMrFreeman.mp3") != -1);
    g_assert(sound_play_loop("42", "BlablablaMrFreeman.mp3") != -1);
    g_assert(sound_status("Test") != -1);
    g_assert(sound_level("Test", 10) != -1);

    /* Bad request check */
    g_assert(sound_play("false.0", "404.wav.false") == -1);
    g_assert(sound_status("false") == -1);
    g_assert(sound_level("false", 10) == -1);

    sound_stop("Test");
    sound_stop("42");
}
