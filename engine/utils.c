#include "utils.h"

// ========================================================
// MATH

// Function to create a translation matrix
Mat4 createTranslationMatrix(Vec3 translation) {
    Mat4 result = {{
        {1.0, 0.0, 0.0, translation.x},
        {0.0, 1.0, 0.0, translation.y},
        {0.0, 0.0, 1.0, translation.z},
        {0.0, 0.0, 0.0, 1.0}
    }};
    return result;
}

Mat4 createTranslationMatrix2(Vec3 translation) {
    Mat4 result = {{
        {1.0, 0.0, 0.0, 0},
        {0.0, 1.0, 0.0, 0},
        {0.0, 0.0, 1.0, 0},
        {translation.x, translation.y, translation.z, 1.0}
    }};
    return result;
}

Mat4 getViewMatrix(Vec3 position) {
    Vec3 cameraFront = {0.0f, 0.0f, -1.0f};
    Vec3 cameraUp = {0.0f, 1.0f, 0.0f};
    Mat4 viewMatrix;
    
    // Calculate the direction the camera is looking at
    Vec3 target = {position.x, position.y, 20.0f};
    Vec3 cameraDirection = {
        target.x - position.x,
        target.y - position.y,
        target.z - position.z
    };
    
    // Normalize camera direction
    float length = sqrt(cameraDirection.x * cameraDirection.x + 
                        cameraDirection.y * cameraDirection.y + 
                        cameraDirection.z * cameraDirection.z);
    cameraDirection.x /= length;
    cameraDirection.y /= length;
    cameraDirection.z /= length;

    // Calculate the right vector
    Vec3 cameraRight = {
        cameraUp.y * cameraDirection.z - cameraUp.z * cameraDirection.y,
        cameraUp.z * cameraDirection.x - cameraUp.x * cameraDirection.z,
        cameraUp.x * cameraDirection.y - cameraUp.y * cameraDirection.x
    };

    // Normalize camera right
    length = sqrt(cameraRight.x * cameraRight.x + 
                  cameraRight.y * cameraRight.y + 
                  cameraRight.z * cameraRight.z);
    cameraRight.x /= length;
    cameraRight.y /= length;
    cameraRight.z /= length;

    // Calculate the up vector
    cameraUp.x = cameraDirection.y * cameraRight.z - cameraDirection.z * cameraRight.y;
    cameraUp.y = cameraDirection.z * cameraRight.x - cameraDirection.x * cameraRight.z;
    cameraUp.z = cameraDirection.x * cameraRight.y - cameraDirection.y * cameraRight.x;

    // Set the view matrix values
    viewMatrix.data[0][0] = cameraRight.x;
    viewMatrix.data[0][1] = cameraRight.y;
    viewMatrix.data[0][2] = cameraRight.z;
    viewMatrix.data[0][3] = -position.x * cameraRight.x - position.y * cameraRight.y - position.z * cameraRight.z;
    
    viewMatrix.data[1][0] = cameraUp.x;
    viewMatrix.data[1][1] = cameraUp.y;
    viewMatrix.data[1][2] = cameraUp.z;
    viewMatrix.data[1][3] = -position.x * cameraUp.x - position.y * cameraUp.y - position.z * cameraUp.z;
    
    viewMatrix.data[2][0] = -cameraDirection.x;
    viewMatrix.data[2][1] = -cameraDirection.y;
    viewMatrix.data[2][2] = -cameraDirection.z;
    viewMatrix.data[2][3] = position.x * cameraDirection.x + position.y * cameraDirection.y + position.z * cameraDirection.z;
    
    viewMatrix.data[3][0] = 0.0f;
    viewMatrix.data[3][1] = 0.0f;
    viewMatrix.data[3][2] = 0.0f;
    viewMatrix.data[3][3] = 1.0f;

    return viewMatrix;
}

// Function to create a scale matrix
Mat4 createScaleMatrix(Vec3 scale) {
    Mat4 result = {{
        {scale.x, 0.0, 0.0, 0.0},
        {0.0, scale.y, 0.0, 0.0},
        {0.0, 0.0, scale.z, 0.0},
        {0.0, 0.0, 0.0, 1.0}
    }};
    return result;
}

// Function to create a rotation matrix around Z-axis
Mat4 createRotationZMatrix(float angle) {
    float cosAngle = cos(angle);
    float sinAngle = sin(angle);
    Mat4 result = {{
        {cosAngle, -sinAngle, 0.0, 0.0},
        {sinAngle, cosAngle, 0.0, 0.0},
        {0.0, 0.0, 1.0, 0.0},
        {0.0, 0.0, 0.0, 1.0}
    }};
    return result;
}

// Function to multiply two 4x4 matrices
Mat4 multiplyMatrices(Mat4 a, Mat4 b) {
    Mat4 result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.data[i][j] = 0;
            for (int k = 0; k < 4; k++) {
                result.data[i][j] += a.data[i][k] * b.data[k][j];
            }
        }
    }
    return result;
}

Mat4 createOrthographicMatrix(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    Mat4 orthoMatrix = {{
        {2.0f / (right - left), 0.0f, 0.0f, 0.0f},
        {0.0f, 2.0f / (top - bottom), 0.0f, 0.0f},
        {0.0f, 0.0f, -2.0f / (farPlane - nearPlane), 0.0f},
        {-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(farPlane + nearPlane) / (farPlane - nearPlane), 1.0f}
    }};
    return orthoMatrix;
}

Mat4 createIdentityMatrix() {
    Mat4 result = {{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    }};
    return result;
}

void printMatrix(Mat4 matrix)
{
    printf("Matrix Contents:\n");
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            printf("%.2f ", matrix.data[i][j]);
        }
        printf("\n");
    }
}

// =========================================================


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