#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <glad/glad.h> // Include GLAD header

GLuint loadTexture(const char* filePath);

GLuint loadTextureColorKey(const char* filePath, const SDL_Color colorKey);