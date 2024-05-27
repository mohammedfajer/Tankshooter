#include "utils.h"


const char *int_to_string(int value)
{
  static char str[12];
  snprintf(str, sizeof(str), "%d", value);
  return str;
}

int random(int min, int max)
{
  if(min > max)
  {
    int temp = min;
    min = max;
    max = temp;
  }

  // Generates a random number in range [min, max]
  return min + rand() % (max - min + 1);
}

int math_min(int num1, int num2)
{
  if(num1 < num2) return num1;
  else return num2;
}

int math_max(int num1, int num2)
{
  if(num1 > num2) return num1;
  else return num2;
}

bool AABB(SDL_Rect rect1, SDL_Rect rect2)
{
  if(rect1.x < rect2.x + rect2.w && rect1.x + rect1.w > rect2.x && 
     rect1.y < rect2.y + rect2.h && rect1.y + rect1.h > rect2.y) 
     return true;
  else
    return false;
}