#ifndef TEXT_H
#define TEXT_H

#include "common.h"
#include <SDL_ttf.h>
#include <SDL.h>

#include <glad/glad.h> // Include GLAD header

typedef struct {
  int fontSize;
  TTF_Font *font;
  int w;
  int h;
} Text;

bool text_init();
bool text_setup_font(Text *text, const char *path, int fontSize);

void text_draw_gl(Text *text, GLuint shaderProgram,
               int xPos, int yPos,
               const char *txt,
               bool center,
               SDL_Color color);

void text_draw(
  Text *text,
  SDL_Renderer *renderer,
  int xPos, int yPos,
  const char* txt,
  bool center,
  SDL_Color color
  );

#endif // TEXT_H