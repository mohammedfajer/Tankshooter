#include "gl_texture.h"



static SDL_Surface 
*loadImage(const char* filePath) {
    SDL_Surface* imageSurface = IMG_Load(filePath);
    if (!imageSurface) {
        printf("Failed to load image: %s\n", IMG_GetError());
        return NULL;
    }
    return imageSurface;
}

Texture2D 
loadTexture(const char* filePath) {

    Texture2D t = {};

    // Load image using SDL
    SDL_Surface* surface = loadImage(filePath);
    if (!surface) {

        printf("Failed to load image \n");
        return t;
    }

    // Generate texture ID
    GLuint textureID;
    glGenTextures(1, &textureID);

    // Bind texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Flip texture vertically
// Flip image data vertically
for (int y = 0; y < surface->h / 2; ++y) {
    for (int x = 0; x < surface->w; ++x) {
        // Swap pixels between top and bottom rows
        Uint32* topPixel = (Uint32*)((Uint8*)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel);
        Uint32* bottomPixel = (Uint32*)((Uint8*)surface->pixels + (surface->h - y - 1) * surface->pitch + x * surface->format->BytesPerPixel);
        Uint32 temp = *topPixel;
        *topPixel = *bottomPixel;
        *bottomPixel = temp;
    }
}

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Load image data into texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

    t.id = textureID;
    t.w = surface->w;
    t.h = surface->h;

    // Free SDL surface
    SDL_FreeSurface(surface);

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    

    return t;
}

GLuint loadTextureColorKey(const char* filePath, const SDL_Color colorKey) {
    // Load image using SDL
    SDL_Surface* surface = loadImage(filePath);
    if (!surface) return 0;

    // Convert surface to the same format as the window
    SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(surface);
    if (!formattedSurface) return 0;

    // Apply color keying
    Uint32 key = SDL_MapRGBA(formattedSurface->format, colorKey.r, colorKey.g, colorKey.b, colorKey.a);
    SDL_SetColorKey(formattedSurface, SDL_TRUE, key);

    // Generate texture ID
    GLuint textureID;
    glGenTextures(1, &textureID);

    // Bind texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Load image data into texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, formattedSurface->w, formattedSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, formattedSurface->pixels);

    // Free SDL surface
    SDL_FreeSurface(formattedSurface);

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}