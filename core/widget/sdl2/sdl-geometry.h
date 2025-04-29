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

#ifndef _SDL_GEOMETRY_H_
#define _SDL_GEOMETRY_H_

#include <SDL2/SDL.h>
#include <math.h>

#define YGEO_MAX_SEGMENTS 128
#define YGEO_MAX_VERTICES (YGEO_MAX_SEGMENTS * 5)

void RendRectangleRoundFilled(SDL_Renderer *renderer, SDL_Rect rect, float radius, SDL_Color color) {
    // Ensure the radius is valid
    if (radius > rect.w / 2) radius = rect.w / 2;
    if (radius > rect.h / 2) radius = rect.h / 2;

    // Preallocate a static array for vertices
    static SDL_Vertex vertices[YGEO_MAX_VERTICES];
    int indices[YGEO_MAX_VERTICES];
    int vertexIdx = 1;
    int indiceIdx = 0;

    // Define the center of the rectangle
    float centerX = rect.x + rect.w / 2.0;
    float centerY = rect.y + rect.h / 2.0;

    // Add the center vertex
    vertices[0] = (SDL_Vertex){{centerX, centerY}, color, {0, 0}};

    // Number of segments for the rounded corners
    int cornerSegments = (int)(radius * M_PI / 2);
    if (cornerSegments > YGEO_MAX_SEGMENTS) cornerSegments = YGEO_MAX_SEGMENTS;

    float angleStep = M_PI / 2 / cornerSegments;

    // Generate the geometry for edges and corners
    for (int corner = 0; corner < 4; ++corner) {
        float cx, cy, startAngle;
        SDL_FPoint edgeStart, edgeEnd;

        // Initialize corner center, starting angle, and edge points
        switch (corner) {
            case 0: // Top-left corner and top edge
                cx = rect.x + radius;
                cy = rect.y + radius;
                startAngle = M_PI;
                edgeStart = (SDL_FPoint){rect.x + radius, rect.y};
                edgeEnd = (SDL_FPoint){rect.x + rect.w - radius, rect.y};
                break;
            case 1: // Top-right corner and right edge
                cx = rect.x + rect.w - radius;
                cy = rect.y + radius;
                startAngle = -M_PI / 2;
                edgeStart = (SDL_FPoint){rect.x + rect.w, rect.y + radius};
                edgeEnd = (SDL_FPoint){rect.x + rect.w, rect.y + rect.h - radius};
                break;
            case 2: // Bottom-right corner and bottom edge
                cx = rect.x + rect.w - radius;
                cy = rect.y + rect.h - radius;
                startAngle = 0;
                edgeStart = (SDL_FPoint){rect.x + rect.w - radius, rect.y + rect.h};
                edgeEnd = (SDL_FPoint){rect.x + radius, rect.y + rect.h};
                break;
            case 3: // Bottom-left corner and left edge
                cx = rect.x + radius;
                cy = rect.y + rect.h - radius;
                startAngle = M_PI / 2;
                edgeStart = (SDL_FPoint){rect.x, rect.y + rect.h - radius};
                edgeEnd = (SDL_FPoint){rect.x, rect.y + radius};
                break;
        }

        // Add two triangles to represent the straight edge
	indices[indiceIdx++] = 0;
	indices[indiceIdx++] = vertexIdx;
        vertices[vertexIdx++] = (SDL_Vertex){edgeStart, color, {0, 0}}; // Start of the edge
	indices[indiceIdx++] = vertexIdx;
        vertices[vertexIdx++] = (SDL_Vertex){edgeEnd, color, {0, 0}};   // End of the edge

        // Add the arc for this corner
        SDL_FPoint prevPoint = {cx + radius * cosf(startAngle), cy + radius * sinf(startAngle)};
        for (int i = 1; i <= cornerSegments; ++i) {
            float angle = startAngle + i * angleStep;

            // Point on the arc
            SDL_FPoint arcPoint = {cx + radius * cosf(angle), cy + radius * sinf(angle)};

            // Add a triangle: center -> previous arc point -> current arc point
	    indices[indiceIdx++] = 0;
	    indices[indiceIdx++] = vertexIdx;
            vertices[vertexIdx++] = (SDL_Vertex){prevPoint, color, {0, 0}}; // Previous arc point
	    indices[indiceIdx++] = vertexIdx;
            vertices[vertexIdx++] = (SDL_Vertex){arcPoint, color, {0, 0}};  // Current arc point

            // Update the previous point
            prevPoint = arcPoint;
        }
    }

    // Render the filled rounded rectangle using SDL_RenderGeometry
    SDL_RenderGeometry(renderer, NULL, vertices, vertexIdx, indices, indiceIdx);
}

void RendRectangleRound(SDL_Renderer *renderer, SDL_Rect rect, float radius, SDL_Color color) {
    // Set the drawing color
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // Ensure the radius is valid
    if (radius > rect.w / 2) radius = rect.w / 2;
    if (radius > rect.h / 2) radius = rect.h / 2;

    // Calculate the corner centers
    float cx1 = rect.x + radius; // Top-left corner
    float cy1 = rect.y + radius;
    float cx2 = rect.x + rect.w - radius; // Top-right corner
    float cy2 = rect.y + radius;
    float cx3 = rect.x + radius; // Bottom-left corner
    float cy3 = rect.y + rect.h - radius;
    float cx4 = rect.x + rect.w - radius; // Bottom-right corner
    float cy4 = rect.y + rect.h - radius;

    // Draw the straight edges
    SDL_RenderDrawLineF(renderer, cx1, rect.y, cx2, rect.y);               // Top edge
    SDL_RenderDrawLineF(renderer, cx1, rect.y + rect.h, cx2, rect.y + rect.h); // Bottom edge
    SDL_RenderDrawLineF(renderer, rect.x, cy1, rect.x, cy3);               // Left edge
    SDL_RenderDrawLineF(renderer, rect.x + rect.w, cy1, rect.x + rect.w, cy3); // Right edge

    // Draw the rounded corners (arcs)
     // Rounded corners
    SDL_FPoint points[YGEO_MAX_SEGMENTS * 4]; // Static array for all corner points
    int pointIdx = 0;

    // Number of segments based on radius, but capped to MAX_SEGMENTS
    int cornerSegments = (int)(radius * M_PI / 2);
    if (cornerSegments > YGEO_MAX_SEGMENTS) cornerSegments = YGEO_MAX_SEGMENTS;

    float angleStep = M_PI / 2 / cornerSegments; // Angle step for each segment

    // Generate points for each corner
    for (int i = 0; i < cornerSegments; ++i) {
        float angle = i * angleStep;

        // Top-left corner
        points[pointIdx++] = (SDL_FPoint){cx1 - radius * cosf(angle), cy1 - radius * sinf(angle)};
        // Top-right corner
        points[pointIdx++] = (SDL_FPoint){cx2 + radius * cosf(angle), cy2 - radius * sinf(angle)};
        // Bottom-left corner
        points[pointIdx++] = (SDL_FPoint){cx3 - radius * cosf(angle), cy3 + radius * sinf(angle)};
        // Bottom-right corner
        points[pointIdx++] = (SDL_FPoint){cx4 + radius * cosf(angle), cy4 + radius * sinf(angle)};
    }

    // Draw all the corner points
    SDL_RenderDrawPointsF(renderer, points, pointIdx);
}

int RenderCircle(SDL_Renderer *renderer, SDL_Texture *texture, SDL_Color color, int x, int y, int radius, int isFilled) {
    #define NUM_SEGMENTS 64

    if (!isFilled) {
        SDL_FPoint points[NUM_SEGMENTS + 2]; // +1 for closing, +1 for safety

        for (int i = 0; i <= NUM_SEGMENTS; ++i) {
            float theta = (float)i / (float)NUM_SEGMENTS * 2.0f * (float)M_PI;
            points[i].x = (float)x + cosf(theta) * radius;
            points[i].y = (float)y + sinf(theta) * radius;
        }

        // Close the circle (last point same as first)
        points[NUM_SEGMENTS + 1] = points[0];

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        if (SDL_RenderDrawLinesF(renderer, points, NUM_SEGMENTS + 2) != 0) {
            return -1;
        }
        return 0;
    }

    // ---- Filled path below ----
    SDL_Vertex vertices[NUM_SEGMENTS + 2];
    int indices[NUM_SEGMENTS * 3];
    int num_vertices = NUM_SEGMENTS + 2;
    int num_indices = NUM_SEGMENTS * 3;

    // Center vertex
    vertices[0].position.x = (float)x;
    vertices[0].position.y = (float)y;
    vertices[0].color = color;
    vertices[0].tex_coord.x = 0.5f;
    vertices[0].tex_coord.y = 0.5f;

    for (int i = 0; i <= NUM_SEGMENTS; ++i) {
        float theta = (float)i / (float)NUM_SEGMENTS * 2.0f * (float)M_PI;
        float dx = cosf(theta) * radius;
        float dy = sinf(theta) * radius;

        vertices[i + 1].position.x = (float)x + dx;
        vertices[i + 1].position.y = (float)y + dy;
        vertices[i + 1].color = color;
        vertices[i + 1].tex_coord.x = (cosf(theta) + 1.0f) * 0.5f;
        vertices[i + 1].tex_coord.y = (sinf(theta) + 1.0f) * 0.5f;
    }

    for (int i = 0; i < NUM_SEGMENTS; ++i) {
        indices[i * 3 + 0] = 0;
        indices[i * 3 + 1] = i + 1;
        indices[i * 3 + 2] = i + 2;
    }

    if (SDL_RenderGeometry(renderer, texture, vertices, num_vertices, indices, num_indices) != 0) {
        return -1;
    }

    return 0;
}

#endif
