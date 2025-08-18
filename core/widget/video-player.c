/*
**Copyright (C) 2025 Matthias Gatto
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

/* based on pl_mpeg_player_sdl, see license bellow V */

/*
PL_MPEG Example - Video player using SDL2's accelerated 2d renderer
SPDX-License-Identifier: MIT

Dominic Szablewski - https://phoboslab.org
Siteswap - https://github.com/siteswapv4


-- Usage

pl_mpeg_player_sdl <video-file.mpg>

Use the arrow keys to seek forward/backward by 3 seconds. Click anywhere on the
window to seek to seek through the whole file.


-- About

This program demonstrates a simple video/audio player using plmpeg for decoding
and SDL2 with the accelerated 2d renderer.

*/

#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg/pl_mpeg.h"

#include "sdl2/sdl-internal.h"
#include <yirl/all.h>

static int wid_t = -1;

struct YVideoPlayerState {
	YWidgetState state;
	plm_t *plm;
	double last_time;
	int wants_to_quit;
	double seek_to;
	SDL_Texture *texture;
	SDL_Rect rectangle;
	SDL_AudioDeviceID audio_device;
};


static void app_on_video(plm_t *mpeg, plm_frame_t *frame, void *user) {
	struct YVideoPlayerState *self = (void *)user;

	SDL_UpdateYUVTexture(self->texture, NULL, frame->y.data, frame->y.width, frame->cb.data, frame->cb.width, frame->cr.data,  frame->cr.width);
}

static void app_on_audio(plm_t *mpeg, plm_samples_t *samples, void *user) {
	struct YVideoPlayerState *self = (void *)user;

	// Hand the decoded samples over to SDL

	int size = sizeof(float) * samples->count * 2;
	SDL_QueueAudio(self->audio_device, samples->interleaved, size);
}

static int init(YWidgetState *opac, Entity *entity, void *args)
{
	struct YVideoPlayerState *self = (void *)opac;
	Entity *widPix = yeGet(entity, "wid-pix");
	int keep_size = yeGetIntAt(entity, "keep-size");

	const char *filename = yeGetStringAt(entity, "filename");
	if (!filename) {
		DPRINT_ERR("Couldn't open %s", filename);
		return -1;
	}

	ywSetTurnLengthOverwrite(-1);
	self->seek_to = -1;
	self->plm = plm_create_with_filename(filename);
	if (!self->plm) {
		DPRINT_ERR("Couldn't open %s", filename);
		return -1;
	}
	if (!plm_probe(self->plm, 5000 * 1024)) {
		DPRINT_ERR("No MPEG video or audio streams found in %s", filename);
		return -1;
	}
	int samplerate = plm_get_samplerate(self->plm);
	SDL_Log(
		"Opened %s - framerate: %f, samplerate: %d, duration: %f",
		filename,
		plm_get_framerate(self->plm),
		plm_get_samplerate(self->plm),
		plm_get_duration(self->plm)
		);

	plm_set_video_decode_callback(self->plm, app_on_video, self);
	plm_set_audio_decode_callback(self->plm, app_on_audio, self);

	plm_set_loop(self->plm, TRUE);
	plm_set_audio_enabled(self->plm, TRUE);
	plm_set_audio_stream(self->plm, 0);

	if (plm_get_num_audio_streams(self->plm) > 0) {
		// Initialize SDL Audio
		SDL_AudioSpec audio_spec = {
			.freq = samplerate,
			.format = AUDIO_F32,
			.channels = 2,
			.samples = 4096
		};

		self->audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);
		if (self->audio_device == 0) {
			SDL_Log("Failed to open audio device: %s", SDL_GetError());
		}
		SDL_PauseAudioDevice(self->audio_device, 0);

		// Adjust the audio lead time according to the audio_spec buffer size
		plm_set_audio_lead_time(self->plm, (double)audio_spec.samples / (double)samplerate);
	}
	// Adjust rectangle to video size
	if (keep_size) {
		self->rectangle.w = plm_get_width(self->plm);
		self->rectangle.h = plm_get_height(self->plm);
	} else {
		self->rectangle.w = ywRectW(widPix);
		self->rectangle.h = ywRectH(widPix);
	}

	self->texture = SDL_CreateTexture(
		sdl_global()->pWindow,
		SDL_PIXELFORMAT_IYUV,
		SDL_TEXTUREACCESS_STREAMING,
		plm_get_width(self->plm),
 		plm_get_height(self->plm)
		);


	ywidGenericCall(opac, wid_t, init);
	return 0;
}

static InputStatue event(YWidgetState *opac, Entity *event)
{
	struct YVideoPlayerState *self = (void *)opac;
	Entity *entity = opac->entity;
	Entity *eve = event;
	Entity *on_quit = yeGet(entity, "on-quit");

	YEVE_FOREACH(eve, event) {
		if (ywidEveType(eve) == YKEY_DOWN) {
			int k = ywidEveKey(eve);
			switch (k) {
			case Y_RIGHT_KEY:
				self->seek_to = plm_get_time(self->plm) + 3;
				break;
			case Y_LEFT_KEY:
				self->seek_to = plm_get_time(self->plm) - 3;
				break;
			case 'q':
			case Y_ESC_KEY:
				if (on_quit)
					yesCall(yeGet(on_quit, 1), entity);
			}
		}
	}
	return ACTION;
}

static int rend(YWidgetState *opac) {
	ywidGenericRend(opac, wid_t, render);
	return 0;
}

static int destroy(YWidgetState *opac) {
	struct YVideoPlayerState *self = (void *)opac;
	plm_destroy(self->plm);

	if (self->audio_device) {
		SDL_CloseAudioDevice(self->audio_device);
	}

	if (self->texture) {
		SDL_DestroyTexture(self->texture);
	}
	return 0;
}

static void *alloc(void)
{
	struct YVideoPlayerState *s = malloc(sizeof *s);
	*s = (struct YVideoPlayerState){
		.state={
			.render = rend,
			.init = init,
			.destroy = destroy,
			.handleEvent = event,
			.type = wid_t
		}
	};
	return (void *)s;
}

int ywVideoPlayerInit(void)
{
	if (ysdl2Type() < 0) {
		DPRINT_ERR("sdl is needed for VideoPlayer");
 	}
	if (wid_t != -1)
		return wid_t;
	wid_t = ywidRegister(alloc, "VideoPlayer");
	return wid_t;
}

int ywVideoPlayerEnd(void)
{
	if (ywidUnregiste(wid_t) < 0)
		return -1;
	wid_t = -1;
	return 0;
}

#include <unistd.h>
static int sdl2Render(YWidgetState *opac, int t)
{
	struct YVideoPlayerState *self = (void *)opac;
	double seek_to = self->seek_to;


	// Clear pWindow, copy texture and present
	SDL_RenderClear(sdl_global()->pWindow);

	SDL_RenderCopy(sdl_global()->pWindow, self->texture, NULL, &self->rectangle);

	SDL_RenderPresent(sdl_global()->pWindow);

	usleep(30000);
	// Compute the delta time since the last app_update(), limit max step to
	// 1/30th of a second
	double current_time = (double)SDL_GetTicks() / 1000.0;
	double elapsed_time = current_time - self->last_time;
	/* if (elapsed_time > 1.0 / 30.0) { */
	/* 	elapsed_time = 1.0 / 30.0; */
	/* } */
	printf("%f - %f - %f\n", current_time, elapsed_time, 1.0/30.0);
	self->last_time = current_time;

	// Seek or advance decode
	if (seek_to != -1) {
		SDL_ClearQueuedAudio(self->audio_device);
		plm_seek(self->plm, seek_to, FALSE);
		self->seek_to = -1;
	}
	else {
		plm_decode(self->plm, elapsed_time);
	}

	if (plm_has_ended(self->plm)) {
		self->wants_to_quit = TRUE;
	}
	return 0;
}

static int sdl2Init(YWidgetState *wid, int t)
{
	wid->renderStates[t].opac = y_new(SDLWid, 1);
	sdlWidInit(wid, t);
	return 0;
}

int ysdl2RegistreVideoPlayer(void)
{
	int ret = ywidRegistreTypeRender("VideoPlayer", ysdl2Type(),
					 sdl2Render, sdl2Init, sdlWidDestroy);
	return ret;
}
