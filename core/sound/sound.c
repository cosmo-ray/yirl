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

#include "utils.h"
#include "sound.h"
#include "sound-libvlc.h"

/* Audio lib */
int audioLibUsed = -1;

#define RET_OR_CALL(func, args...)		\
  if (unlikely(!func))				\
    return -1;					\
  return func(args)

SoundState noSound;

extern SoundState defaultSoundDriver;

int ysound_init(void)
{
  if (unlikely(!defaultSoundDriver.libInit))
    return -1;
  return defaultSoundDriver.libInit();
}

int ysound_end(void)
{
  if (unlikely(!defaultSoundDriver.libEnd))
    return -1;
  return defaultSoundDriver.libEnd();
}

/* ---- shortcut ---- */
int ySoundLoad(const char *path)
{
  RET_OR_CALL(defaultSoundDriver.load, path);
}
int ySoundLoadMusic(const char *path)
{
	RET_OR_CALL(defaultSoundDriver.load_music, path);
}

int ySoundPlay(int id)
{
  RET_OR_CALL(defaultSoundDriver.play, id);
}

int ySoundPlayLoop(int id)
{
  RET_OR_CALL(defaultSoundDriver.play_loop, id);
}

int ySoundLevel(int id, int soundLvl)
{
  RET_OR_CALL(defaultSoundDriver.ySoundLevel, id, soundLvl);
}

int ySoundStatus(int id)
{
  RET_OR_CALL(defaultSoundDriver.status, id);
}

int ySoundStop(int id)
{
  RET_OR_CALL(defaultSoundDriver.stop, id);
}

#undef RET_OR_CALL
