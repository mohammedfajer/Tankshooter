#include "sound.h"
#include <stdio.h>


bool sound_music_load(Music *music, const char *path)
{
    music->music = Mix_LoadMUS(path);
    if(music == NULL) {
        printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
        return false;
    }
    return true;
}

void sound_music_play(Music *music, bool looping)
{
    // Play the music
    int loop = (looping) ? -1 : 0;
    Mix_PlayMusic(music->music, loop); // -1 loops the music indefinitely, 0 plays once, 1 plays twice, and so on.
}

void sound_music_clean(Music *music)
{
    if(music->music != NULL)
    {
        Mix_FreeMusic(music->music);
    }
}

void sound_effect_init()
{
  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
      fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
       return;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "Failed to initialize SDL_mixer: %s\n", Mix_GetError());
        SDL_Quit();
       return;
    }
}

void sound_effect_load(SoundEffect *eff, const char *path)
{
   // Load the sound effect from the specified path
    eff->source = Mix_LoadWAV(path);
    if (!eff->source) {
        fprintf(stderr, "Failed to load sound effect: %s\n", Mix_GetError());
    }
}

void sound_effect_clean(SoundEffect *eff)
{
  // Free the sound effect
    if (eff->source) {
        Mix_FreeChunk(eff->source);
        eff->source = NULL;
    }
}

void sound_effect_play(SoundEffect *eff)
{
   if (eff->source) {
        // Play the sound effect
        if (Mix_PlayChannel(-1, eff->source, 0) == -1) {
            fprintf(stderr, "Failed to play sound effect: %s\n", Mix_GetError());
        }
    } else {
        fprintf(stderr, "Sound effect not loaded.\n");
    }
}
