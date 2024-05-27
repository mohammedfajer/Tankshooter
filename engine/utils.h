#pragma once

#include "common.h"

const char *int_to_string(int value);
int random(int min, int max);
int math_min(int num1, int num2);
int math_max(int num1, int num2);
bool AABB(SDL_Rect rect1, SDL_Rect rect2);