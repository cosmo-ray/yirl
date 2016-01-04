/*
** Copyright 2015 Kara adrien
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
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
