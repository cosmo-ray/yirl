/*
* Copyright 2015 Kara adrien
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
#include <vlc/vlc.h>

#include "sound.h"
#include "sound-libvlc.h"

extern gchar *soundName[ARRAY_SIZE];

libvlc_instance_t       *soundInst[ARRAY_SIZE];
libvlc_media_player_t   *soundPlay[ARRAY_SIZE];

/* ---- Libs ---- */
int libvlc(int nameId, uint32_t flags, int soundLvl, const char *path)
{
    int result = 0;

    if (flags & PLAY_SOUND) {
        result = libvlc_play(nameId, path, 0);
	}
    if (flags & PLAY_LOOP_SOUND) {
		result = libvlc_play(nameId, path, 1);
	}
    if (flags & STOP_SOUND) {
    	libvlc_stop(nameId);
    }
    if (flags & SOUND_LEVEL) {
    	result = libvlc_soundLvl(nameId, soundLvl);
    }
    if (flags & SOUND_PAUSE) {
    	libvlc_pause(nameId);
    }
    if (flags & SOUND_STATUS) {
    	result = libvlc_status(nameId);
    }

    return result;
}

int libvlc_play(int nameId, const char *path, int loop)
{
    libvlc_media_t *media;

    if ((soundInst[nameId] = libvlc_new(0, NULL))                             == NULL ||
        (media             = libvlc_media_new_path(soundInst[nameId], path))  == NULL ||
        (soundPlay[nameId] = libvlc_media_player_new_from_media(media))       == NULL) {
        libvlc_stop(nameId);
        return -1;
    }

    if (loop != 0) {
        libvlc_media_add_option(media, "input-repeat=-1");
    }

    libvlc_media_release(media);

    if (g_file_test(path, G_FILE_TEST_EXISTS) == 0) {
        libvlc_stop(nameId);
        return -1;
    } else {
        return libvlc_media_player_play(soundPlay[nameId]);
    }
}

int libvlc_soundLvl(int nameId, int soundLvl)
{
    return libvlc_audio_set_volume(soundPlay[nameId], soundLvl);
}

int libvlc_status(int nameId)
{
    return libvlc_media_player_will_play(soundPlay[nameId]);
}

int libvlc_pause(int nameId)
{
    int result = 0;

    if (libvlc_media_player_is_playing(soundPlay[nameId]) == 0) {
        result = libvlc_media_player_play(soundPlay[nameId]);
    } else {
        libvlc_media_player_pause(soundPlay[nameId]);
    }

    return result;
}

void libvlc_stop(int nameId)
{
    if (soundName[nameId] != NULL) {
        g_free(soundName[nameId]);
        soundName[nameId] = NULL;
    }

    libvlc_media_player_stop(soundPlay[nameId]);
    libvlc_media_player_release(soundPlay[nameId]);
    libvlc_release(soundInst[nameId]);
}
