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

#include "utils.h"
#include "sound.h"
#include "sound-libvlc.h"

#define ARRAY_SIZE 128

static libvlc_instance_t *soundInst[ARRAY_SIZE];
static libvlc_media_player_t *soundPlay[ARRAY_SIZE];
static libvlc_media_t *soundMedia[ARRAY_SIZE];
static uint8_t is_used[ARRAY_SIZE];

static int lastElem;

SoundState vlcDriver = {
  .load = libvlc_load,
  .play = libvlc_play,
  .play_loop = NULL,
  .status = libvlc_status,
  .ySoundLevel = libvlc_soundLvl,
  .pause = libvlc_pause,
  .stop = libvlc_stop
};

int libvlc_load(const char *path)
{
  int nameId = lastElem;

  if (g_file_test(path, G_FILE_TEST_EXISTS) == 0) {
    DPRINT_ERR("%s doesn't exist", path);
    goto error;
  }

  soundInst[nameId] = libvlc_new(0, NULL);
  soundMedia[nameId] = libvlc_media_new_path(soundInst[nameId], path);
  soundPlay[nameId] = libvlc_media_player_new_from_media(soundMedia[nameId]);

  if (soundInst[nameId]  == NULL || soundMedia[nameId] == NULL ||
      soundPlay[nameId] == NULL) {
    DPRINT_ERR("fail to load %s", path);
    goto error;
  }

  /* if (loop != 0) { */
  /*   libvlc_media_add_option(media, "input-repeat=-1"); */
  /* } */

  is_used[nameId] = 1;
  ++lastElem;
  return nameId;
 error:
  libvlc_stop(nameId);
  return -1;
}

#define CHECK_NAMEID(nameId) do {				\
    if (!is_used[nameId] || nameId >= ARRAY_SIZE)		\
      return -1;						\
  } while (0)

int libvlc_play(int nameId)
{
  CHECK_NAMEID(nameId);
  return libvlc_media_player_play(soundPlay[nameId]);
}

int libvlc_soundLvl(int nameId, int soundLvl)
{
  CHECK_NAMEID(nameId);
  return libvlc_audio_set_volume(soundPlay[nameId], soundLvl);
}

int libvlc_status(int nameId)
{
  CHECK_NAMEID(nameId);
  return libvlc_media_player_will_play(soundPlay[nameId]);
}

int libvlc_pause(int nameId)
{
  int result = 0;

  CHECK_NAMEID(nameId);
  if (libvlc_media_player_is_playing(soundPlay[nameId]) == 0) {
    result = libvlc_media_player_play(soundPlay[nameId]);
  } else {
    libvlc_media_player_pause(soundPlay[nameId]);
  }

  return result;
}

int libvlc_stop(int nameId)
{
  CHECK_NAMEID(nameId);
  is_used[nameId] = 0;
  libvlc_media_player_stop(soundPlay[nameId]);
  libvlc_media_player_release(soundPlay[nameId]);
  libvlc_media_release(soundMedia[nameId]);
  libvlc_release(soundInst[nameId]);
  return 0;
}

#undef ARRAY_SIZE
