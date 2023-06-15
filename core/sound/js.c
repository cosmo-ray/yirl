/*
**Copyright (C) 2023 Matthias Gatto
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

#include <emscripten.h>
#include "utils.h"
#include "sound.h"

#define ARRAY_SIZE 255

static int lastElem;

int musiques[ARRAY_SIZE];

static int init(void)
{
	EM_ASM({
			window.yall_audios = [];
		});
	return 0;
}

static int end(void)
{
	return 0;
}

static int next_free_elem(void)
{
	for (int i = 0; i < ARRAY_SIZE; ++i) {
		int el = (lastElem + i) % ARRAY_SIZE;
		if (!musiques[el])
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

	if (access(path, F_OK) < 0) {
		DPRINT_ERR("%s doesn't exist", path);
		return -1;
	}
	*el = nameId;
	return 0;
}

static int chunk_load(const char *path)
{
	int audio;
	int dst;

	if (music_set_elem_path_check(path, &dst) < 0)
		return -1;
	EM_ASM(console.log('TEST\n'););
	audio = EM_ASM_INT({
			var audio = new Audio(UTF8ToString($0));
			window.yall_audios.push(audio);
			console.log("[[[audio ",UTF8ToString($0),":", audio, "]]]");
			return window.yall_audios.length - 1;
		}, path);
	printf("AUDIO ! (tristesse)): %d\n", audio);
	musiques[dst] = audio;
	return dst;
}

#define CHECK_NAMEID(nameId) do {				\
		if (nameId >= ARRAY_SIZE)			\
			return -1;				\
	} while (0)

static int play(int nameId)
{
	CHECK_NAMEID(nameId);
	printf("play %d\n", nameId);
	EM_ASM({
			console.log("play: ", $0);
			console.log(window.yall_audios[$0]);

			window.yall_audios[$0].loop = false;
			window.yall_audios[$0].play();
		}, musiques[nameId]);
	return nameId;
}

static int play_loop(int nameId)
{
	CHECK_NAMEID(nameId);
	EM_ASM({
			window.yall_audios[$0].loop = true;
			window.yall_audios[$0].play();
		}, musiques[nameId]);
	return nameId;
}

static int js_pause(int nameId)
{
	CHECK_NAMEID(nameId);
	EM_ASM({
			window.yall_audios[$0].pause();
		}, musiques[nameId]);
	return 0;
}

static int stop(int nameId)
{
	CHECK_NAMEID(nameId);
	EM_ASM({
			window.yall_audios[$0].pause();
			window.yall_audios[$0].currentTime = 0;
		}, musiques[nameId]);
	return nameId;
}

static int duration(int nameId)
{
	CHECK_NAMEID(nameId);
	return EM_ASM_INT({
			return window.yall_audios[$0].duration;
		}, musiques[nameId]);
}

SoundState jsDriver = {
  .libInit = init,
  .libEnd = end,
  .pause = js_pause,
  .load = chunk_load,
  .play = play,
  .load_music = chunk_load,
  .play_loop = play_loop,
  .stop = stop,
  .duration = duration
};
