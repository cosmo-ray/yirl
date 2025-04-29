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

#ifndef _GPU_COMPAT_H_
#define _GPU_COMPAT_H_

#include <SDL2/SDL.h>

void SDL_Clear(SDL_Renderer *renderer, SDL_Color *color) {
	// Set the color to clear with
	if (color)
		SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
	else
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	// Clear the rendering target with the specified color
	SDL_RenderClear(renderer);
}

static SDL_Texture *GPU_CopyImageFromSurfaceRect(SDL_Renderer *renderer, SDL_Surface *surface, const SDL_Rect *rect) {
	if (!surface || !renderer || !rect) {
		printf("Invalid surface, renderer, or rect provided.\n");
		return NULL;
	}

	// Create a new surface with the size of the rect
	SDL_Surface *subSurface = SDL_CreateRGBSurface(0, rect->w, rect->h, surface->format->BitsPerPixel,
						       surface->format->Rmask, surface->format->Gmask,
						       surface->format->Bmask, surface->format->Amask);
	if (!subSurface) {
		printf("Failed to create sub-surface: %s\n", SDL_GetError());
		return NULL;
	}

	// Copy the content of the rectangle into the new surface
	SDL_BlitSurface(surface, rect, subSurface, NULL);

	// Create a texture from the new surface
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, subSurface);

	if (!texture) {
		printf("Failed to create texture from sub-surface: %s\n", SDL_GetError());
	}

	// Free the temporary sub-surface
	SDL_FreeSurface(subSurface);

	return texture;
}

#endif

