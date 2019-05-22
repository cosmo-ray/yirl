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

#ifndef _SOUND_H
#define _SOUND_H

typedef struct {
  int ( *libInit)(void);
  int (* libEnd)(void);
  int (* load)(const char *path);
  int (* play)(int);
  int (* play_loop)(int);
  int (* load_music)(const char *path);
  int (* status)(int);
  int (* ySoundLevel)(int, int);
  int (* pause)(int);
  int (* stop)(int);
} SoundState;

extern int audioLibUsed;

int ysound_init(void);
int ysound_end(void);

int ySoundLoad(const char *path);

/**
 * @return 0 upon success, -1 on error
 */
int ySoundPlay(int id);

int ySoundMusicLoad(const char *path);

/**
 * @return 0 upon success, -1 on error
 */
int ySoundMusicPlay(int id);

int ySoundMusicStop(int id);

/**
 * Same as ySoundMusicPlay() in loop
 */
int ySoundPlayLoop(int id);

/**
 * @return 0 upon success, -1 on error
 */
int ySoundLevel(int id, int soundLvl);

/**
 * @return 1 if media is playable, else 0
 */
int ySoundStatus(int id);

int ySoundPause(int id);

int ySoundStop(int id);

int sound_init(void);

#endif
