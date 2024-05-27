#include <stdlib.h>
#include <time.h>
#include <assert.h>


#include <SDL.h>
#include <glad/glad.h> // Include GLAD header

#include <SDL_ttf.h>
#include <SDL_image.h>

// Engine
#include "./engine/common.h"
#include "./engine/utils.h"
#include "./engine/text.h"
#include "./engine/image.h"
#include "./engine/linkedlist.h"
#include "./engine/sound.h"
#include "./engine/graphics.h"
#include "./engine/init.h"

// Game

// SDL State
SDL_Renderer *gRenderer = NULL;
SDL_Window *gWindow = NULL;
SDL_Texture *gRenderTexture = NULL;
SDL_Event gEvent;

// Game State
bool gRunning = true;

// Game States
extern StateManager gStateManager;

extern State gStartState ;
extern State gHighScoreState ;
extern State gEnterHighScoreState ;
extern State gGameOverState ;
extern State gPlayState ;
extern State gServeState ;
extern State gPaddleSelectState ;
extern State gVictoryState ;

// Font
Text gSmallFont;
Text gMediumFont;
Text gLargeFont;

// Textures
Sprite gBackgroundImg;
Sprite gMainImg;
Sprite gArrowsImg;
Sprite gHeartsImg;
Sprite gParticleImg;

// Sound Effects

SoundEffect gPaddleHitSoundEffect;
SoundEffect gScoreSoundEffect;
SoundEffect gWallHitSoundEffect;
SoundEffect gConfirmSoundEffect;
SoundEffect gSelectSoundEffect;
SoundEffect gNoSelectSoundEffect;
SoundEffect gBrickHit1SoundEffect;
SoundEffect gBrickHit2SoundEffect;
SoundEffect gHurtSoundEffect;
SoundEffect gVictorySoundEffect;
SoundEffect gRecoverSoundEffect;
SoundEffect gHighScoreSoundEffect;
SoundEffect gPauseSoundEffect;

Music gMusic;

// File Scope Global Data
static const int FRAME_DELAY = 1000 / FPS;
static bool resized = false;
static float fpsCounter = 0;

// Keyboard Handling
bool keys[SDL_NUM_SCANCODES] = {0};
bool keysR[SDL_NUM_SCANCODES] = {0}; // with repeat

// Render
static SDL_Rect dstrect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

// GL Context
static SDL_GLContext gGlContext;



GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource);

GLuint gFbo;
GLuint gTextureColorBuffer;

float gQuadVertices [] = {
    -1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f
};

unsigned int gQuadIndices[] = {
  0,1,2,
  0,2,3
};

 float correction = 1.0F;

GLuint gQuadVAO, gQuadVBO, gQuadEBO;
GLuint gScreenShaderProgram;

// Shader sources for screen quad
const char* gScreenVertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "    TexCoord = aTexCoord;\n"
    "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "}\0";

const char* gScreenFragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D screenTexture;\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(screenTexture, TexCoord );\n"
    "}\n\0";


// Quad Draw Test
GLuint gSpriteQuadVBO, gSpriteQuadVAO, gSpriteQuadEBO;
GLuint gSpriteShaderProgram;


const char* g_sprite_vertex_shader_source = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 position;\n"
  
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    
    "void main() {\n"
    "    gl_Position =  projection * view * model * vec4(position, 1.0);\n"
    
    "}\n";

const char* g_sprite_fragment_shader_source =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    
    "void main() {\n"
    "    FragColor = vec4(1.0, 0.5, 0.2, 1.0);\n" // Orange color
    "}\n";


typedef struct {
  float x;
  float y;
  float z;
} Vec3;

typedef struct {
  float data[4][4];
} Mat4;

static Vec3 gCameraPos = {0,0,0};

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


// Mat4 createOrthographicMatrix(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
//     Mat4 orthoMatrix = {{
//         {2.0f / (right - left), 0.0f, 0.0f, 0.0f},
//         {0.0f, 2.0f / (top - bottom), 0.0f, 0.0f},
//         {0.0f, 0.0f, -2.0f / (farPlane - nearPlane), 0.0f},
//         {-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(farPlane + nearPlane) / (farPlane - nearPlane), 1.0f}
//     }};
//     return orthoMatrix;
// }

// Mat4 createOrthographicMatrix(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
//     Mat4 orthoMatrix = {{
//         {2.0f / (right - left), 0.0f, 0.0f, -(right + left) / (right - left)},
//         {0.0f, 2.0f / (top - bottom), 0.0f, -(top + bottom) / (top - bottom)},
//         {0.0f, 0.0f, -2.0f / (farPlane - nearPlane), -(farPlane + nearPlane) / (farPlane - nearPlane)},
//         {0.0f, 0.0f, 0.0f, 1.0f}
//     }};
//     return orthoMatrix;
// }


Mat4 createOrthographicMatrix(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    Mat4 orthoMatrix = {{
        {2.0f / (right - left), 0.0f, 0.0f, 0.0f},
        {0.0f, 2.0f / (top - bottom), 0.0f, 0.0f},
        {0.0f, 0.0f, -2.0f / (farPlane - nearPlane), 0.0f},
        {-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(farPlane + nearPlane) / (farPlane - nearPlane), 1.0f}
    }};
    return orthoMatrix;
}

Mat4 createOrthographicMatrix2(float left, float right, float bottom, float top, float nearPlane, float farPlane, float tx, float ty, float sx, float sy) {
    Mat4 orthoMatrix = {{
        {2.0f / sx, 0.0f, 0.0f, 0.0f},
        {0.0f, 2.0f / sy, 0.0f, 0.0f},
        {0.0f, 0.0f, -2.0f / (farPlane - nearPlane), 0.0f},
        {-(right + left) / (right - left) + tx, -(top + bottom) / (top - bottom) + ty, -(farPlane + nearPlane) / (farPlane - nearPlane), 1.0f}
    }};
    return orthoMatrix;
}


// Mat4 createOrthographicMatrix(float left, float right, float bottom, float top) {
//     Mat4 orthoMatrix = {{
//         {2.0f / (right - left), 0.0f, 0.0f, 0.0f},
//         {0.0f, 2.0f / (top - bottom), 0.0f, 0.0f},
//         {0.0f, 0.0f, -1.0f, 0.0f},
//         {-(right + left) / (right - left), -(top + bottom) / (top - bottom), 0.0f, 1.0f}
//     }};
//     return orthoMatrix;
// }

Mat4 createIdentityMatrix() {
    Mat4 result = {{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    }};
    return result;
}

Mat4 gProjectionMatrix;
Mat4 gViewMatrix;

Vec3 gSpritePos;

void init_sprite_quad() {
    // Define vertices for a simple quad
 

float vertices[] = {
     0.5f,  0.5f, 0.0f,  // top right
     0.5f, 0.5f, 0.0f,  // bottom right
    0.5f, 0.5f, 0.0f,  // bottom left
    0.5f,  0.5f, 0.0f   // top left 
};
unsigned int indices[] = {  // note that we start from 0!
    2, 1, 0,   // first triangle
    0, 1, 3    // second triangle
};  


    // Create Vertex Array Object and Vertex Buffer Object
    glGenVertexArrays(1, &gSpriteQuadVAO);
    glGenBuffers(1, &gSpriteQuadVBO);
    glGenBuffers(1, &gSpriteQuadEBO); // Add EBO for indices

    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s)
    glBindVertexArray(gSpriteQuadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, gSpriteQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gSpriteQuadEBO); // Bind EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Pass indices data

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Unbind VAO
    glBindVertexArray(0);
}



void render_sprite_quad(float posX, float posY, float scaleX, float scaleY, float rotationZ) {
    // Bind VAO and draw quad
    glUseProgram(gSpriteShaderProgram);
    glBindVertexArray(gSpriteQuadVAO);

     // Adjust scale based on letterboxing correction factor
   
    // Set transformation matrices
    Mat4 translationMatrix = createTranslationMatrix((Vec3) {posX, posY, 0.0});
    Mat4 scaleMatrix = createScaleMatrix((Vec3){scaleX, scaleY, 1.0});
   // Mat4 rotationMatrix = createRotationZMatrix(rotationZ);
    // Calculate model matrix by multiplying translation, rotation, and scale matrices
    Mat4 modelMatrix = createIdentityMatrix();
    
    modelMatrix = multiplyMatrices(modelMatrix, scaleMatrix);
    //modelMatrix = multiplyMatrices(rotationMatrix, modelMatrix);
   modelMatrix = multiplyMatrices(modelMatrix, translationMatrix);

    // T * C



    //Set uniform matrices
    //Assuming view and projection matrices are set elsewhere
   //glUniformMatrix4fv(glGetUniformLocation(gSpriteShaderProgram, "model"), 1, GL_TRUE, (const GLfloat *)modelMatrix.data);


    int MatrixID = glGetUniformLocation(gSpriteShaderProgram, "model");

    glUniformMatrix4fv(MatrixID, 1, GL_TRUE, &modelMatrix.data[0][0]);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void init_quad()
{
  glGenVertexArrays(1, &gQuadVAO);
  glGenBuffers(1, &gQuadVBO);
  glGenBuffers(1, &gQuadEBO);
  glBindVertexArray(gQuadVAO);

  glBindBuffer(GL_ARRAY_BUFFER, gQuadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(gQuadVertices), gQuadVertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gQuadEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gQuadIndices), gQuadIndices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

  glBindVertexArray(0);
}





void render_quad()
{
  glDisable(GL_DEPTH_TEST);

  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(gScreenShaderProgram);
  glBindVertexArray(gQuadVAO);
  glBindTexture(GL_TEXTURE_2D, gTextureColorBuffer);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  glBindVertexArray(0);
}


void init_framebuffer(int width, int height)
{
  glGenFramebuffers(1, &gFbo);
  glBindFramebuffer(GL_FRAMEBUFFER, gFbo);

  glGenTextures(1, &gTextureColorBuffer);
  glBindTexture(GL_TEXTURE_2D, gTextureColorBuffer);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

  // Filtering
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gTextureColorBuffer, 0);


  GLuint rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      fprintf(stderr, "Framebuffer is not complete!\n");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void render_scene()
{
  glBindFramebuffer(GL_FRAMEBUFFER, gFbo);
  glEnable(GL_DEPTH_TEST);

  // Clear the screen
  glClearColor(40.0f / 255.0f, 45.0f / 255.0f, 52.0f / 255.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


 

  // Render your scene here (sprites, objects, etc.)
  render_sprite_quad(gSpritePos.x, gSpritePos.y, 100, 100, 0.0f);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float calculateLetterboxingCorrection(int windowWidth, int windowHeight) {
    // float virtualAspect = (float)VIRTUAL_WIDTH / (float)VIRTUAL_HEIGHT;
    // float windowAspect = (float)windowWidth / (float)windowHeight;
    
    // if (windowAspect > virtualAspect) {
    //     // Window wider than virtual screen
    //     return (float)windowHeight / VIRTUAL_HEIGHT;
    // } else {
    //     // Window taller than virtual screen
    //     return (float)windowWidth / VIRTUAL_WIDTH;
    // }

    float windowAspect = (float)windowWidth / (float)windowHeight;
    float virtualAspect = (float)VIRTUAL_WIDTH / (float)VIRTUAL_HEIGHT;
    
    int viewportWidth, viewportHeight;
    int viewportX = 0, viewportY = 0;

    float correction = 0.0f;

    if (windowAspect > virtualAspect) {
        viewportHeight = windowHeight;
        viewportWidth = (int)(windowHeight * virtualAspect);
        viewportX = (windowWidth - viewportWidth) / 2;

        correction = (float)windowHeight / VIRTUAL_HEIGHT;
    } else {
        viewportWidth = windowWidth;
        viewportHeight = (int)(windowWidth / virtualAspect);
        viewportY = (windowHeight - viewportHeight) / 2;
        correction = (float)windowWidth / VIRTUAL_WIDTH;;
    }

    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

    return correction;
}





void applyLetterboxing(int windowWidth, int windowHeight) {
    printf("Applying Letter boxing\n");

    // Calculate viewport dimensions
    float windowAspect = (float)windowWidth / (float)windowHeight;
    float virtualAspect = (float)VIRTUAL_WIDTH / (float)VIRTUAL_HEIGHT;
    int viewportWidth, viewportHeight;
    int viewportX = 0, viewportY = 0;

    if (windowAspect > virtualAspect) {
    // Window is wider than the virtual aspect ratio
      viewportHeight = windowHeight;
      viewportWidth = (int)(windowHeight * virtualAspect);
      viewportX = (windowWidth - viewportWidth) / 2;
      printf("Viewport X: %d, Viewport Width: %d\n", viewportX, viewportWidth);
    } else {
        // Window is taller than the virtual aspect ratio
        viewportWidth = windowWidth;
        viewportHeight = (int)(windowWidth / virtualAspect);
        viewportY = (windowHeight - viewportHeight) / 2;
        printf("Viewport Y: %d, Viewport Height: %d\n", viewportY, viewportHeight);
    }

    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

    // Calculate translation and scaling factors
    float tx = -2.0f * ((float)viewportX / (float)windowWidth) + 1.0f;
    float ty = -2.0f * ((float)viewportY / (float)windowHeight) + 1.0f;
    float sx = 2.0f * (float)viewportWidth / (float)windowWidth;
    float sy = 2.0f * (float)viewportHeight / (float)windowHeight;

    // Calculate orthographic projection matrix
    float left, right, bottom, top;
    float aspectRatio = (float)viewportWidth / (float)viewportHeight;
    float halfWidth = 0;
float halfHeight = 0;

    if (aspectRatio > 1.0f) {
        // If the viewport is wider than it is tall
         halfWidth = VIRTUAL_WIDTH / 2.0f;
         halfHeight = halfWidth / aspectRatio;
        left = -halfWidth;
        right = halfWidth;
        bottom = -halfHeight;
        top = halfHeight;
    } else {
        // If the viewport is taller than it is wide
         halfHeight = VIRTUAL_HEIGHT / 2.0f;
         halfWidth = halfHeight * aspectRatio;
        left = -halfWidth;
        right = halfWidth;
        bottom = -halfHeight;
        top = halfHeight;
    }

    // Adjust the left and right edges based on viewport translation
    left += tx * (right - left);
    right += tx * (right - left);

    printf("Left is %f\n", left );

glUseProgram(gSpriteShaderProgram);
//     glBindVertexArray(gSpriteQuadVAO);

    // Create orthographic projection matrix
    // Mat4 orthoMatrix = createOrthographicMatrix2(viewportX, viewportX + viewportWidth, viewportHeight, viewportWidth, -1, 1, 0, ty, sx, sy);
    // gProjectionMatrix = createOrthographicMatrix(-1.0 *aspectRatio, viewportWidth, 1 * aspectRatio, viewportHeight, -1, 1.0f);

    // Apply orthographic projection matrix
    //glUniformMatrix4fv(glGetUniformLocation(gSpriteShaderProgram, "projection"), 1, GL_TRUE, (const GLfloat *)gProjectionMatrix.data);
}


// void applyLetterboxing(int windowWidth, int windowHeight) {
//     printf("Applying Letter boxing\n");

//     // Calculate viewport dimensions
//     float windowAspect = (float)windowWidth / (float)windowHeight;
//     float virtualAspect = (float)VIRTUAL_WIDTH / (float)VIRTUAL_HEIGHT;
//     int viewportWidth, viewportHeight;
//     int viewportX = 0, viewportY = 0;

//     // Calculate the aspect ratio of the virtual content
//     float contentAspect = virtualAspect;

//     // If the window aspect ratio is wider than the virtual aspect ratio
//     if (windowAspect > virtualAspect) {
//         viewportHeight = windowHeight;
//         viewportWidth = (int)(windowHeight * virtualAspect);
//         viewportX = (windowWidth - viewportWidth) / 2;

//         printf("Viewport X: %d, Viewport Width: %d\n", viewportX, viewportWidth);
//     } else { // If the window aspect ratio is taller than the virtual aspect ratio
//         viewportWidth = windowWidth;
//         viewportHeight = (int)(windowWidth / virtualAspect);
//         viewportY = (windowHeight - viewportHeight) / 2;

//         // Adjust the content aspect ratio to match the stretched viewport
//         contentAspect = windowAspect;

//         printf("Viewport Y: %d, Viewport Height: %d\n", viewportY, viewportHeight);
//     }

//     glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

 

  

   

    
//     // Calculate the aspect ratio of the scaled viewport
//     float scaledViewportAspect = (float)viewportWidth / (float)viewportHeight;

//     // Calculate the width and height of the content area
//     float contentWidth = VIRTUAL_WIDTH;
//     float contentHeight = VIRTUAL_HEIGHT;

//     if (scaledViewportAspect > virtualAspect) {
//         // Scale content width
//         contentWidth = contentHeight * scaledViewportAspect;
//     } else {
//         // Scale content height
//         contentHeight = contentWidth / scaledViewportAspect;
//     }

//     // Calculate the left edge based on the viewport translation
//     float left = -contentWidth / 2.0f + ((float)viewportWidth - windowWidth) / 2.0f;
//     float right = left + contentWidth;

//       printf("Left is %f\n", left );

//        glUseProgram(gSpriteShaderProgram);
//     glBindVertexArray(gSpriteQuadVAO);

//     // Create orthographic projection matrix
//     Mat4 orthoMatrix = createOrthographicMatrix(viewportX, right, -contentHeight / 2.0f, contentHeight / 2.0f, -1, 1);

//     // Apply orthographic projection matrix
//     glUniformMatrix4fv(glGetUniformLocation(gSpriteShaderProgram, "projection"), 1, GL_TRUE, (const GLfloat *)orthoMatrix.data);
// }

void display_fps(float fps);

void opengl_init()
{
  gGlContext = SDL_GL_CreateContext(gWindow);

  if(!gGlContext)
  {
    SDL_Log("Failed to create OpenGL context: %s", SDL_GetError());
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
    return;
  }
}

void init_glad()
{
   // Initialize GLAD
  if (!gladLoadGL()) {
      SDL_Log("Failed to initialize GLAD");
      SDL_GL_DeleteContext(gGlContext);
      SDL_DestroyWindow(gWindow);
      SDL_Quit();
     return;
  }
}

void init_sdl_ttf()
{
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
  {
    printf("error initializing sdl: %s\n", SDL_GetError());
    return;
  }

  text_init();

  sound_effect_init();

  gWindow = SDL_CreateWindow("Breakout",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                         SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL );
  if (!gWindow)
  {
    printf("error creating window: %s\n", SDL_GetError());
    SDL_Quit();
    return;
  }

  opengl_init();
  init_glad();

  // Print OpenGL version
  SDL_Log("OpenGL Version: %s", glGetString(GL_VERSION));

  // Enable V-Sync
  SDL_GL_SetSwapInterval(1);

  // Set viewport
  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

  bool success = image_init();
  if(!success)
  {
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
    return;
  }

  Uint32 renderFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC 
  | SDL_RENDERER_TARGETTEXTURE ;
  gRenderer = SDL_CreateRenderer(gWindow, -1, renderFlags);
  if (!gRenderer)
  {
    printf("error creating renderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
    return;
  }
}

void set_render_target(SDL_Renderer *renderer, SDL_Texture *renderTexture)
{
  SDL_SetRenderTarget(renderer, renderTexture);
}

void reset_render_target(SDL_Renderer *renderer)
{
  set_render_target(renderer, NULL);
}

void display_fps(float fps)
{
  const char *fpsStr = int_to_string(fps);
  char buffer[512];
  SDL_memset(buffer, 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%s %s", "FPS: ", fpsStr);
  text_draw(&gSmallFont, gRenderer, 5, 5, buffer, false, (SDL_Color){0,255,0});
}

void love_load()
{
  gRenderTexture = engine_create_virtual_resolution(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

  // Init Global Fonts
  text_setup_font(&gSmallFont, "./data/fonts/font.ttf", 8);
  text_setup_font(&gMediumFont, "./data/fonts/font.ttf", 16);
  text_setup_font(&gLargeFont, "./data/fonts/font.ttf", 32);

  // Init Global Spries  
  

  // Init Global Sounds
  sound_effect_load(&gPaddleHitSoundEffect, "./data/sounds/paddle_hit.wav");
  sound_effect_load(&gScoreSoundEffect, "./data/sounds/score.wav");
  sound_effect_load(&gWallHitSoundEffect, "./data/sounds/wall_hit.wav");  
  sound_effect_load(&gConfirmSoundEffect, "./data/sounds/confirm.wav");
  sound_effect_load(&gSelectSoundEffect, "./data/sounds/select.wav");
  sound_effect_load(&gNoSelectSoundEffect, "./data/sounds/no-select.wav");
  sound_effect_load(&gBrickHit1SoundEffect, "./data/sounds/brick-hit-1.wav");
  sound_effect_load(&gBrickHit2SoundEffect, "./data/sounds/brick-hit-2.wav");
  sound_effect_load(&gHurtSoundEffect, "./data/sounds/hurt.wav");    
  sound_effect_load(&gVictorySoundEffect, "./data/sounds/victory.wav");
  sound_effect_load(&gRecoverSoundEffect, "./data/sounds/recover.wav");
  sound_effect_load(&gHighScoreSoundEffect, "./data/sounds/high_score.wav");
  sound_effect_load(&gPauseSoundEffect, "./data/sounds/pause.wav");   

  sound_music_load(&gMusic, "./data/sounds/music.wav"); 

  // Start Game Music
  //sound_music_play(&gMusic, true);

  // Init State Manager
  //gStateManager.currentState = &gStartState;
  
  //state_manager_switch(&gStateManager, &gStartState); // starting at start state

  // Init Keys
  SDL_memset(keys, 0, SDL_NUM_SCANCODES * sizeof(bool));
  SDL_memset(keysR, 0, SDL_NUM_SCANCODES * sizeof(bool));

  init_framebuffer(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
  init_quad();

  gScreenShaderProgram = createShaderProgram(gScreenVertexShaderSource,
    gScreenFragmentShaderSource);


  init_sprite_quad();
  gSpriteShaderProgram = createShaderProgram(g_sprite_vertex_shader_source, g_sprite_fragment_shader_source);


  // Set uniform matrices
  // Assuming view and projection matrices are set elsewhere

   glUseProgram(gSpriteShaderProgram);

  gViewMatrix = createIdentityMatrix();

  gViewMatrix = getViewMatrix((Vec3){0,0, 0.0});
  
  gProjectionMatrix = createOrthographicMatrix(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1.0F, 1.0f);
  glUniformMatrix4fv(glGetUniformLocation(gSpriteShaderProgram, "view"), 1, GL_TRUE, (const GLfloat *)gViewMatrix.data);
  glUniformMatrix4fv(glGetUniformLocation(gSpriteShaderProgram, "projection"), 1, GL_FALSE, (const GLfloat *)gProjectionMatrix.data);

  gSpritePos = (Vec3){0,0,0};



}

void love_keypressed(int key)
{
  if(key >= MAX_KEYS) return;

  printf("key is pressed\n");

  keys[key] = true;

 
}

bool love_wasPressed(int key)
{
  assert(key >= 0 && key < MAX_KEYS);

  if(keys[key])
  {
    printf("key was pressed\n");
    return true;
  }
  return false;
}

void love_keyreleased(int key)
{
   if (key >= MAX_KEYS) return;

  keys[key] = false;
}

void love_keypressedR(int key)
{
  if(key >= MAX_KEYS) return;

  printf("key is down\n");

  keysR[key] = true;
}

bool love_wasPressedR(int key)
{
  assert(key >= 0 && key < MAX_KEYS);

  if(keysR[key])
  {
    printf("key was down\n");
    return true;
  }
  return false;
}

void love_keyreleasedR(int key)
{
  if (key >= MAX_KEYS) return;

  keysR[key] = false;
}

void handle_window_resize()
{
   /*
      This Maintain the aspect ratio by letter boxing
      Always maintains aspect ratio
    */
    // Get the window size
    int windowWidth, windowHeight;
    SDL_GetWindowSize(gWindow, &windowWidth, &windowHeight);

    // Calculate aspect ratio
    float windowAspect = (float)windowWidth / (float)windowHeight;
    float virtualAspect = (float)VIRTUAL_WIDTH / (float)VIRTUAL_HEIGHT;

    if (windowAspect > virtualAspect) {
        // Window is wider than the virtual aspect ratio
        dstrect.h = windowHeight;
        dstrect.w = (int)(windowHeight * virtualAspect);
        dstrect.x = (windowWidth - dstrect.w) / 2;
        dstrect.y = 0;
    } else {
        // Window is taller than the virtual aspect ratio
        dstrect.w = windowWidth;
        dstrect.h = (int)(windowWidth / virtualAspect);
        dstrect.x = 0;
        dstrect.y = (windowHeight - dstrect.h) / 2;
    }
}

void input_handling()
{
  while (SDL_PollEvent(&gEvent))
  {
    switch (gEvent.type)
    {
      case SDL_QUIT:
      {
        gRunning = false;
      } break;
        
      case SDL_KEYDOWN: 
      {
        love_keypressedR(gEvent.key.keysym.scancode);

        if(gEvent.key.repeat == 0)
          love_keypressed(gEvent.key.keysym.scancode);
      } break;

      case SDL_KEYUP:
      {
         love_keyreleasedR(gEvent.key.keysym.scancode);

        if(gEvent.key.repeat == 0)
          love_keyreleased(gEvent.key.keysym.scancode);
      } break;

      case SDL_WINDOWEVENT:
      { 
        if(gEvent.window.event == SDL_WINDOWEVENT_RESIZED)
        {
           handle_window_resize();

           // Solved the drawing artifect zig zag
          applyLetterboxing(gEvent.window.data1, gEvent.window.data2);
        
          //  int windowWidth = gEvent.window.data1;
          // int windowHeight = gEvent.window.data2;
          // float correction = calculateLetterboxingCorrection(windowWidth, windowHeight);
          // adjustOrthographicProjection(windowWidth, windowHeight, correction);

        }
         

        if(gEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        {
          applyLetterboxing(gEvent.window.data1, gEvent.window.data2);

          //   int windowWidth = gEvent.window.data1;
          // int windowHeight = gEvent.window.data2;
          // float correction = calculateLetterboxingCorrection(windowWidth, windowHeight);
          // adjustOrthographicProjection(windowWidth, windowHeight, correction);
          
        }
      } break;

      default:
        break;
    }    
  }
}



void love_update(float dt)
{
  // Handle state transitions logic
  //handle_state_machine_transitions();

  // Update current state
  //gStateManager.currentState->update(dt);

 
  //cameraPos.x -= dt *  10.0f;

  if(love_wasPressedR(SDL_SCANCODE_UP))
  {
    gSpritePos.y += 50 * dt;
  }
   if(love_wasPressedR(SDL_SCANCODE_RIGHT))
  {
    gSpritePos.x += 50 * dt;
  }
   if(love_wasPressedR(SDL_SCANCODE_LEFT))
  {
    gSpritePos.x -= 50 * dt;
  }
   if(love_wasPressedR(SDL_SCANCODE_DOWN))
  {
    gSpritePos.y -= 50 * dt;
  }

  if(love_wasPressedR(SDL_SCANCODE_C))
  {
    gCameraPos.x -= 2 * dt;
  }


  glUseProgram(gSpriteShaderProgram);

  gViewMatrix = getViewMatrix(gCameraPos);

  // Transpose if its in row-major as opengl wants it in colum major
  glUniformMatrix4fv(glGetUniformLocation(gSpriteShaderProgram, "view"), 1, GL_TRUE, (const GLfloat *)gViewMatrix.data);

  if(love_wasPressed(SDL_SCANCODE_ESCAPE)) gRunning = false;

  // Reset keys array
  SDL_memset(keys, 0, SDL_NUM_SCANCODES * sizeof(bool));
}

void love_draw()
{
  // Set the render target to our texture
  //set_render_target(gRenderer, gRenderTexture);

  // SDL_SetRenderDrawColor(gRenderer, 40, 45, 52, 255);
  // SDL_RenderClear(gRenderer);
  //glClearColor(40.0f / 255.0f, 45.0f / 255.0f, 52.0f / 255.0f, 1.0f);
 // glClear(GL_COLOR_BUFFER_BIT);

  // Render Order is important

  // Draw Background regardless of what state we are in.
  //float xScaleFactor = VIRTUAL_WIDTH / (gBackgroundImg.width - 1) * 2;
 //float yScaleFactor = VIRTUAL_HEIGHT / (gBackgroundImg.height -1) * 2;

  //image_draw(&gBackgroundImg, gRenderer, 0,0, xScaleFactor, yScaleFactor);

  //gStateManager.currentState->render();

  //display_fps(fpsCounter);
 
  // Reset the render target to the default render target (the window)
  //reset_render_target(gRenderer);

  // Clear the window renderer before presenting the new frame
  // SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
  // SDL_RenderClear(gRenderer);
  

  // Copy the texture to the renderer, scaling it to fit the window
  // SDL_RenderCopy(gRenderer, gRenderTexture, NULL, &dstrect);
  // SDL_RenderPresent(gRenderer);

  // Render scene to framebuffer
  render_scene();

  glClearColor(0,0,0, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // render framebuffer texture to screen
  render_quad();


  SDL_GL_SwapWindow(gWindow);

}

int main(int argc, char *argv[])
{
  // Seed the random number generator
  srand(time(NULL));

  init_sdl_ttf();  
  love_load();

  // Variables for delta time
  Uint32 lastTime = SDL_GetTicks();
  float dt = 0.0f;
  
  Uint32 fpsLastTime = SDL_GetTicks();
  int frameCount = 0;
 
  fpsCounter = 0;

  while (gRunning)
  {
    Uint32 frameStart = SDL_GetTicks();

    input_handling();
    love_update(dt);
    love_draw();

    // Copy the texture to the renderer, scaling it to fit the window
    //SDL_RenderCopy(gRenderer, gRenderTexture, NULL, &dstrect);
    /* Draw to window and loop */
    //SDL_RenderPresent(gRenderer);

    // Calculate delta time
    Uint32 currentTime = SDL_GetTicks();
    dt = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    
    // Increment frame count
    frameCount++;

    // Calculate FPS every second
    if (SDL_GetTicks() - fpsLastTime >= 1000) {
        fpsCounter = frameCount / ((SDL_GetTicks() - fpsLastTime) / 1000.0f);
        frameCount = 0;
        fpsLastTime = SDL_GetTicks();
    }
    
    // Frame time management
    Uint32 frameTime = SDL_GetTicks() - frameStart;
    if (frameTime < FRAME_DELAY) {
        SDL_Delay(FRAME_DELAY - frameTime);
    }
  }
  return 0;
}