/*
**Copyright (C) 2017 Matthias Gatto
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
#include <SDL2/SDL_mixer.h>

#include "utils.h"
#include "sound.h"
#include "sound-libvlc.h"

#define ARRAY_SIZE 128

static Mix_Music *musiques[ARRAY_SIZE];
static uint8_t is_used[ARRAY_SIZE];

static int lastElem;

static int init(void)
{
  int ret = Mix_Init(MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 |
		     MIX_INIT_OGG);

  if (ret < 0) {
    DPRINT_ERR("Mix_Init fail");
    return -1;
  }
  ret = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT,
		      MIX_DEFAULT_CHANNELS, 640);
  if (ret < 0) {
    DPRINT_ERR("Mix_OpenAudio fail");
    Mix_Quit();
    return -1;
  }
  return 0;
}

static int end(void)
{
  for (int i = 0; i < ARRAY_SIZE; ++i) {
    sound_stop(i);
  }
  Mix_Quit();
  return 0;
}

static int libsdl_load(const char *path)
{
  int nameId = lastElem;

  if (g_file_test(path, G_FILE_TEST_EXISTS) == 0) {
    DPRINT_ERR("%s doesn't exist", path);
    goto error;
  }


  musiques[nameId] = Mix_LoadMUS(path);
  if (!musiques[nameId]) {
    DPRINT_ERR("fail to load %s", path);
    goto error;
  }
  is_used[nameId] = 1;
  ++lastElem;
  return nameId;
 error:
  return -1;
}

#define CHECK_NAMEID(nameId) do {				\
    if (!is_used[nameId] || nameId >= ARRAY_SIZE)		\
      return -1;						\
  } while (0)

static int libsdl_play(int nameId)
{
  CHECK_NAMEID(nameId);
  return Mix_PlayMusic(musiques[nameId], 0);
}

static int libsdl_play_loop(int nameId)
{
  CHECK_NAMEID(nameId);
  return Mix_PlayMusic(musiques[nameId], -1);
}

/* int libsdl_soundLvl(int nameId, int soundLvl) */
/* { */
/*   CHECK_NAMEID(nameId); */
/* } */

/* int libsdl_status(int nameId) */
/* { */
/*   CHECK_NAMEID(nameId); */
/* } */

/* int libsdl_pause(int nameId) */
/* { */
/*   int result = 0; */

/*   CHECK_NAMEID(nameId); */
/*   return result; */
/* } */

static int libsdl_stop(int nameId)
{
  CHECK_NAMEID(nameId);
  Mix_FreeMusic(musiques[nameId]);
  is_used[nameId] = 0;
  return 0;
}

SoundState sdlDriver = {
  .libInit = init,
  .libEnd = end,
  .load = libsdl_load,
  .play = libsdl_play,
  .play_loop = libsdl_play_loop,
  .stop = libsdl_stop
};

#undef ARRAY_SIZE
