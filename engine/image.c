

#include "image.h"

bool image_init()
{
  // Init SDL Image
  if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
  {
    printf("IMG_Init Error: %s\n", IMG_GetError());
    return false;
  }
  return true;
}

bool image_load(Sprite *sprite, SDL_Renderer *renderer, const char *image_path)
{
  SDL_Surface *bmp = IMG_Load(image_path);
  if (bmp == NULL)
  {
    printf("IMG_Load Error: %s\n", IMG_GetError());
    return false;
  }

  sprite->texture = SDL_CreateTextureFromSurface(renderer, bmp);
  SDL_FreeSurface(bmp);

  if (sprite->texture == NULL)
  {
    printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
    return false;
  }

  SDL_QueryTexture(sprite->texture, NULL, NULL, &sprite->width, &sprite->height);
  printf("img dim: %d %d\n", sprite->width, sprite->height);

  return true;
}

void image_draw(Sprite *sprite, SDL_Renderer *renderer,
                int x, int y, int scale_x, int scale_y)
{
  SDL_Rect srcRect, dstRect;
  srcRect.x = 0;
  srcRect.y = 0;
  srcRect.w = sprite->width;
  srcRect.h = sprite->height;
  
  dstRect.x = x;
  dstRect.y = y;
  dstRect.w = (int)(srcRect.w * scale_x);
  dstRect.h = (int)(srcRect.h * scale_y);
  
  // Added this to enable flipping
  SDL_RendererFlip flip = SDL_FLIP_NONE;
  if(scale_y < 0)
  {
    flip = SDL_FLIP_HORIZONTAL;
  }

  SDL_RenderCopyEx(renderer, sprite->texture, &srcRect, &dstRect, 0, NULL, flip);
}

void image_clean(Sprite *sprite)
{
  if (sprite->texture != NULL)
  {
    SDL_DestroyTexture(sprite->texture);
  }
}

