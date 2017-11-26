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

#define ARRAY_SIZE          64

/* Flags on hexValue */
#define PLAY_SOUND          0x00000001
#define PLAY_LOOP_SOUND     0x00000002

#define STOP_SOUND          0x00000004

#define SOUND_LEVEL         0x00000010
#define SOUND_STATUS        0x00000020
#define SOUND_PAUSE         0x00000040

/* Audio lib available */
enum AudioLib { LIB_VLC, AUDIO_LIB_END };

extern char *soundName[ARRAY_SIZE];

/**
 * Select library you want
 *
 * \param libSelected name of lib you want
 */
void sound_init(enum AudioLib libSelected);


/**
 * Play sound, shortcut of sound_manager(name, PLAY_SOUND, 0, path);
 *
 * \param name the media instance name
 * \param path is the media path
 * \return 0 upon success, -1 on error
 */
int sound_play(const char *name, const char *path);

/**
 * Same of sound_play() in loop
 */
int sound_play_loop(const char *name, const char *path);

/**
 * Change sound level, shortcut of sound_manager(name, SOUND_LEVEL, 100, NULL);
 *
 * \param name the media instance name
 * \param soundLvl with SOUND_LEVEL flag define what sound level you want
 * \return 0 upon success, -1 on error
 */
int sound_level(const char *name, int soundLvl);

/**
 * Stop sound, shortcut of sound_manager(name, SOUND_STATUS, 0, NULL);
 *
 * \param name the media instance name
 * \return 1 if media is playable, else 0
 */
int sound_status(const char *name);

/**
 * Stop sound, shortcut of sound_manager(name, STOP_SOUND, 0, NULL);
 *
 * \param name the media instance name
 */
void sound_stop(const char *name);

/**
 * Lib check width GLib (Unit testing)
 */
void yirl_sound_cpp(void);

#endif
