/*
* Copyright 2015 Adrien Kara
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <glib.h>
#include <stdio.h>
#include <stdint.h>

#include "sound.h"
#include "sound-libvlc.h"

/* Media instace name */
gchar *soundName[ARRAY_SIZE];

/* Audio lib */
int audioLibUsed = -1;

/* ---- Sound Manager ---- */
void sound_init(enum AudioLib libSelected)
{
    audioLibUsed = libSelected;
}

int sound_manager(const char *name, uint32_t flags, int soundLvl, const char *path)
{
    int freeId, nameId;

    int (*SoundLib[AUDIO_LIB_END])(int, uint32_t, int, const char *);
    SoundLib[LIB_VLC] = libvlc;

    nameId = 0;
    freeId = 0;

    for (int i = 1, ii = 0; i < ARRAY_SIZE; ++i, ++ii) {

        if (soundName[ii] != NULL && !(*SoundLib[audioLibUsed])(ii, SOUND_STATUS, 0, NULL)) {
            (*SoundLib[audioLibUsed])(ii, STOP_SOUND, 0, NULL);
        }

        if (!g_strcmp0(name, soundName[ii])) {
            nameId = i;
            break;
        } else if (soundName[ii] == NULL && freeId == 0) {
            freeId = i;
        }
    }

    if (nameId == 0) {
        nameId = freeId;
    }

    --nameId;

    if (soundName[nameId] == NULL && !(flags & PLAY_SOUND || flags & PLAY_LOOP_SOUND)) {
        return -1;
    }

    soundName[nameId] = g_strdup(name);

    return (*SoundLib[audioLibUsed])(nameId, flags, soundLvl, path);
}

/* ---- shortcut ---- */
int sound_play(const char *name, const char *path)
{
    return sound_manager(name, PLAY_SOUND, 0, path);
}

int sound_play_loop(const char *name, const char *path)
{
    return sound_manager(name, PLAY_LOOP_SOUND, 0, path);
}

int sound_level(const char *name, int soundLvl)
{
    return sound_manager(name, SOUND_LEVEL, soundLvl, NULL);
}

int sound_status(const char *name)
{
    return sound_manager(name, SOUND_STATUS, 0, NULL);
}

void sound_stop(const char *name)
{
    sound_manager(name, STOP_SOUND, 0, NULL);
}
