#pragma once

#include "common.h"

typedef struct {
  float x;
  float y;
  float z;
  float w;
} Vec4;

typedef struct {
  float x;
  float y;
  float z;
} Vec3;

typedef struct {
  float x;
  float y;
  
} Vec2;


typedef struct {
  float data[4][4];
} Mat4;

const char *int_to_string(int value);
int random(int min, int max);
int math_min(int num1, int num2);
int math_max(int num1, int num2);
bool AABB(SDL_Rect rect1, SDL_Rect rect2);


Mat4 createTranslationMatrix(Vec3 translation);
Mat4 createTranslationMatrix2(Vec3 translation);
Mat4 getViewMatrix(Vec3 position);
Mat4 createScaleMatrix(Vec3 scale);
Mat4 createRotationZMatrix(float angle);
Mat4 multiplyMatrices(Mat4 a, Mat4 b);
Mat4 createOrthographicMatrix(float left, float right, float bottom, float top, float nearPlane, float farPlane);
Mat4 createIdentityMatrix();
void printMatrix(Mat4 matrix);