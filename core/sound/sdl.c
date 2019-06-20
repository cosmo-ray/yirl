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

#define ARRAY_SIZE 255

enum sount_type {
	SOUND_UNUSED_,
	SOUND_MIX_,
	SOUND_CHUNK_
};

static union {
	Mix_Music *m;
	Mix_Chunk *c;
} musiques[ARRAY_SIZE];
static uint8_t types[ARRAY_SIZE];

static int lastElem;

static inline int find_chan(Mix_Chunk *c)
{
	for (int i = 0; i < ARRAY_SIZE; ++i) {
		if (Mix_GetChunk(i) == c)
			return i;
	}
	return -1;
}

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
  Mix_AllocateChannels(255);
  return 0;
}

static int end(void)
{
  for (int i = 0; i < ARRAY_SIZE; ++i) {
    ySoundStop(i);
  }
  Mix_Quit();
  return 0;
}

static int next_free_elem(void)
{
	for (int i = 0; i < ARRAY_SIZE; ++i) {
		int el = (lastElem + i) % ARRAY_SIZE;
		if (!types[el])
			return el;
	}
	return -1;
}

static int music_set_elem_path_check(const char *path, int *el)
{
	int nameId = next_free_elem();

	if (nameId < 0) {
		DPRINT_ERR("Max Music reach");
		return -1;
	}

	if (g_file_test(path, G_FILE_TEST_EXISTS) == 0) {
		DPRINT_ERR("%s doesn't exist", path);
		return -1;
	}
	*el = nameId;
	return 0;
}

static int libsdl_chunk_load(const char *path)
{
	int nameId;

	if (music_set_elem_path_check(path, &nameId) < 0)
		goto error;
	musiques[nameId].c = Mix_LoadWAV(path);
	if (!musiques[nameId].c) {
		DPRINT_ERR("fail to load %s", path);
		goto error;
	}
	types[nameId] = SOUND_CHUNK_;
	++lastElem;
	lastElem = lastElem % ARRAY_SIZE;
	return nameId;
error:
	return -1;
}

static int libsdl_music_load(const char *path)
{
	int nameId;

	if (music_set_elem_path_check(path, &nameId) < 0)
		goto error;
	musiques[nameId].m = Mix_LoadMUS(path);
	if (!musiques[nameId].m) {
		DPRINT_ERR("fail to load %s", path);
		goto error;
	}
	types[nameId] = SOUND_MIX_;
	++lastElem;
	lastElem = lastElem % ARRAY_SIZE;
	return nameId;
error:
	return -1;
}

#define CHECK_NAMEID(nameId) do {				\
		if (!types[nameId] || nameId >= ARRAY_SIZE)	\
			return -1;				\
	} while (0)

static int libsdl_play(int nameId)
{
  CHECK_NAMEID(nameId);
  if (types[nameId] == SOUND_MIX_)
	  return Mix_PlayMusic(musiques[nameId].m, 0);
  int c = find_chan(musiques[nameId].c);

  if (c >= 0) {
	  Mix_Resume(c);
	  return 0;
  }
  return Mix_PlayChannel(-1, musiques[nameId].c, 0);
}

static int libsdl_play_loop(int nameId)
{
  CHECK_NAMEID(nameId);
  if (types[nameId] == SOUND_MIX_)
	  return Mix_PlayMusic(musiques[nameId].m, 1);
  return Mix_PlayChannel(-1, musiques[nameId].c, 1);
}

/* int libsdl_soundLvl(int nameId, int soundLvl) */
/* { */
/*   CHECK_NAMEID(nameId); */
/* } */

/* int libsdl_status(int nameId) */
/* { */
/*   CHECK_NAMEID(nameId); */
/* } */

static int libsdl_pause(int nameId)
{
  CHECK_NAMEID(nameId);
  if (types[nameId] == SOUND_MIX_) {
	  Mix_PauseMusic();
	  return 0;
  }

  int c = find_chan(musiques[nameId].c);

  if (c < 0)
	  return -1;
  Mix_Pause(c);
  return 0;
}

static int libsdl_stop(int nameId)
{
	CHECK_NAMEID(nameId);
	if (types[nameId] == SOUND_MIX_)
		Mix_FreeMusic(musiques[nameId].m);
	else
		Mix_FreeChunk(musiques[nameId].c);
	types[nameId] = SOUND_UNUSED_;
	return 0;
}

SoundState sdlDriver = {
  .libInit = init,
  .libEnd = end,
  .pause = libsdl_pause,
  .load = libsdl_chunk_load,
  .play = libsdl_play,
  .load_music = libsdl_music_load,
  .play_loop = libsdl_play_loop,
  .stop = libsdl_stop
};

#undef ARRAY_SIZE
