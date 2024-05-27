#include "text.h"

bool text_init()
{
  if (TTF_Init() != 0)
  {
    printf("TTF_Init Error: %s\n", TTF_GetError());
    return false;
  }
  return true;
}

bool text_setup_font(Text *text, const char *path, int fontSize)
{
  text->fontSize = fontSize;
  text->font = TTF_OpenFont(path, text->fontSize);
  if (!text->font)
  {
    printf("error openning font file: %s\n", SDL_GetError());
    return false;
  }
  return true;
}

void text_draw(Text *text,
              SDL_Renderer *renderer,
              int xPos, int yPos,
              const char* txt,
              bool center,
              SDL_Color color)
{
    // Create surface
    SDL_Surface *surfaceMessage = TTF_RenderText_Solid(text->font, txt, color);
    if (!surfaceMessage)
    {
        printf("Failed to create a surface for text message: %s\n", TTF_GetError());
        return; // Early return on error
    }
    // Create texture from surface
    SDL_Texture *Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    SDL_FreeSurface(surfaceMessage); 
    if (!Message)
    {
        printf("Failed to create a texture for text message: %s\n", SDL_GetError());
        return; // Early return on error
    }

    // Query the texture to get the width and height of the rendered text
    int textWidth, textHeight;
    SDL_QueryTexture(Message, NULL, NULL, &textWidth, &textHeight);

    // Calculate the position based on the center flag
    int drawX = xPos;
    int drawY = yPos;

    if (center)
    {
        drawX -= textWidth / 2;
        drawY -= textHeight / 2;
    }

    SDL_Rect MessageRect = {drawX, drawY, textWidth, textHeight};

    // Copy the texture to the renderer
    SDL_RenderCopy(renderer, Message, NULL, &MessageRect);
    // Destroy the texture to free up memory
    SDL_DestroyTexture(Message);
}