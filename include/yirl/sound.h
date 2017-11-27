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
  int (* status)(int);
  int (* sound_level)(int, int);
  int (* pause)(int);
  int (* stop)(int);
} SoundState;

extern int audioLibUsed;

int ysound_init(void);
int ysound_end(void);

int sound_load(const char *path);

/**
 * Play sound, shortcut of sound_manager(name, PLAY_SOUND, 0, path);
 *
 * \return 0 upon success, -1 on error
 */
int sound_play(int id);

/**
 * Same as sound_play() in loop
 */
int sound_play_loop(int id);

/**
 * Change sound level, shortcut of sound_manager(name, SOUND_LEVEL, 100, NULL);
 *
 * \param soundLvl with SOUND_LEVEL flag define what sound level you want
 * \return 0 upon success, -1 on error
 */
int sound_level(int id, int soundLvl);

/**
 * Stop sound, shortcut of sound_manager(name, SOUND_STATUS, 0, NULL);
 *
 * \param name the media instance name
 * \return 1 if media is playable, else 0
 */
int sound_status(int id);

/**
 * Stop sound, shortcut of sound_manager(name, STOP_SOUND, 0, NULL);
 *
 * \param name the media instance name
 */
int sound_pause(int id);

/**
 * Stop sound, shortcut of sound_manager(name, STOP_SOUND, 0, NULL);
 *
 * \param name the media instance name
 */
int sound_stop(int id);

int sound_init(void);

#endif
