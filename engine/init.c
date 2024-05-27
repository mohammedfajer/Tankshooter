#include "init.h"

#include "common.h"

extern SDL_Window *gWindow;
extern SDL_Renderer *gRenderer;

void engine_set_icon(const char *path)
{
  // Load Icon
  SDL_Surface* icon = SDL_LoadBMP(path);
  if (!icon)
  {
    printf("Unable to load image! SDL_Error: %s\n", SDL_GetError());
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
    return;
  }

  // Set window icon
  SDL_SetWindowIcon(gWindow, icon);

  //Free the loaded image as it's no longer needed
  SDL_FreeSurface(icon);
}


SDL_Texture *engine_create_virtual_resolution(int width, int height)
{
   // Create a texture to render to with resolution 320x240
  SDL_Texture *renderTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_TARGET,
                                            width, height);
  if (renderTexture == NULL)
  {
    printf("SDL_CreateTexture Error: %s\n", SDL_GetError());
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
  }

  return renderTexture;
}