#pragma once

#include <SDL.h>
#include <SDL_mixer.h>

#include "common.h"

typedef struct {
  Mix_Chunk *source;
} SoundEffect;


typedef struct {
  Mix_Music *music;
} Music;


bool sound_music_load(Music *music, const char *path);

void sound_music_play(Music *music, bool looping);

void sound_music_clean(Music *music);

void sound_effect_init();

void sound_effect_load(SoundEffect *eff, const char *path);

void sound_effect_clean(SoundEffect *eff);

void sound_effect_play(SoundEffect *eff);

