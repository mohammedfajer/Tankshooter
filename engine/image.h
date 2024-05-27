#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include "common.h"

typedef struct {
  SDL_Texture *texture;
  int width, height;

} Sprite;

bool image_init();

bool image_load(Sprite *sprite, SDL_Renderer *renderer, const char *image_path);

void image_draw(Sprite *sprite, SDL_Renderer *renderer, int x, int y, int scale_x, int scale_y);

void image_clean(Sprite *sprite);

