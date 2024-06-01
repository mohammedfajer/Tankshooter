#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <glad/glad.h> // Include GLAD header


typedef struct {
    GLuint id;
    float w;
    float h;
} Texture2D;

Texture2D 
loadTexture(const char* filePath);

GLuint loadTextureColorKey(const char* filePath, const SDL_Color colorKey);