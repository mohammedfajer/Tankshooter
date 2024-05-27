#pragma once

#include <SDL.h>

void engine_set_icon(const char *path);

SDL_Texture *engine_create_virtual_resolution(int width, int height);