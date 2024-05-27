#include "graphics.h"

void draw_rect(SDL_Renderer *renderer, enum Mode mode, int x, int y, int width, int height, Color color)
{
  // Set the rectangle's attributes
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = width;
  rect.h = height;

  if(mode == FILL)
  {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0xFF);  // Red color for filled
    SDL_RenderFillRect(renderer, &rect);
  }
  else if(mode == LINE)
  {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 0xFF);  // Green color for outlined
    SDL_RenderDrawRect(renderer, &rect);
  }
  else
    printf("Invalid mode\n");
}