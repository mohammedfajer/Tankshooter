#ifndef TEXT_H
#define TEXT_H

#include "common.h"
#include <SDL_ttf.h>
#include <SDL.h>

typedef struct {
  int fontSize;
  TTF_Font *font;
  int w;
  int h;
} Text;

bool text_init();
bool text_setup_font(Text *text, const char *path, int fontSize);

void text_draw(
  Text *text,
  SDL_Renderer *renderer,
  int xPos, int yPos,
  const char* txt,
  bool center,
  SDL_Color color
  );

#endif // TEXT_H