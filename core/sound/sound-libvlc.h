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

#ifndef _SOUND_LIB_VLC_H
#define _SOUND_LIB_VLC_H


int libvlc_load(const char *path);

/**
 * Play media file
 *
 * \param nameId the ID of media instance name
 * \param path is the media path
 * \param loop, play if zero, play loop if non-zero
 * \return 0 upon success, -1 on error
 */
int libvlc_play(int nameId);

/**
 * Set current media volume
 *
 * \param nameId the ID of media instance name
 * \param soundLvl the volume in percents (0 = mute, 100 = 0dB)
 * \return 0 if the volume was set, -1 if it was out of range
 */
int libvlc_soundLvl(int nameId, int soundLvl);

/**
 * Is the player able to play
 *
 * \param nameId the ID of media instance name
 * \return 1 if media is playable, else 0
 */
int libvlc_status(int nameId);

/**
 * Pause or resume (no effect if there is no media)
 * If media is already on pause resume, else ... pause
 *
 * \param nameId the ID of media instance name
 */
int libvlc_pause(int nameId);

/**
 * Stop (no effect if there is no media)
 *
 * \param nameId the ID of media instance name
 */
int libvlc_stop(int nameId);

#endif
