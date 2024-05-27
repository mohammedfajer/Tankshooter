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

#include "math.c"
#include "quad.c"

#include "fbo.c"

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


void renderFramebufferToScreen(int windowWidth, int windowHeight) {
    float targetAspectRatio = (float)VIRTUAL_WIDTH / VIRTUAL_HEIGHT;
    int width = windowWidth;
    int height = (int)(width / targetAspectRatio + 0.5f);

    if (height > windowHeight) {
        height = windowHeight;
        width = (int)(height * targetAspectRatio + 0.5f);
    }

    int vp_x = (windowWidth / 2) - (width / 2);
    int vp_y = (windowHeight / 2) - (height / 2);

    glViewport(vp_x, vp_y, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glUseProgram(gScreenShaderProgram);
  glBindVertexArray(gQuadVAO);
  glBindTexture(GL_TEXTURE_2D, gTextureColorBuffer);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // Set up and draw a quad with the framebuffer texture
    // Assuming you have a function to draw a quad
    

    glBindTexture(GL_TEXTURE_2D, 0);
}




void render_scene()
{

  glBindFramebuffer(GL_FRAMEBUFFER, gFbo);
 glViewport(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
  glEnable(GL_DEPTH_TEST);

  

   

    // Clear the viewport area with the desired background color
    glClearColor(40.0f / 255.0f, 45.0f / 255.0f, 52.0f / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render your scene here (sprites, objects, etc.)
    render_sprite_quad(gSpritePos.x, gSpritePos.y, 100, 100, 0.0f);
    printf("spritePos: %d %d\n", (int)gSpritePos.x, (int)gSpritePos.y);
    render_sprite_quad(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 50, 50, 0.0f);
 

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

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



  // Init Keys
  SDL_memset(keys, 0, SDL_NUM_SCANCODES * sizeof(bool));
  SDL_memset(keysR, 0, SDL_NUM_SCANCODES * sizeof(bool));

  init_sprite_quad();
  gSpriteShaderProgram = createShaderProgram(g_sprite_vertex_shader_source, g_sprite_fragment_shader_source);


 

  init_framebuffer(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
  init_quad();

  gScreenShaderProgram = createShaderProgram(gScreenVertexShaderSource,
    gScreenFragmentShaderSource);

   glUseProgram(gSpriteShaderProgram);

  gViewMatrix = createIdentityMatrix();

  gViewMatrix = getViewMatrix((Vec3){0,0, 0.0});
  
  gProjectionMatrix = createOrthographicMatrix(0, WINDOW_WIDTH,  WINDOW_HEIGHT, 0, -1.0F, 1.0f);
  glUniformMatrix4fv(glGetUniformLocation(gSpriteShaderProgram, "view"), 1, GL_FALSE, (const GLfloat *)gViewMatrix.data);
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
          // int width, height;
          // float targetAspectRatio = (float)VIRTUAL_WIDTH / VIRTUAL_HEIGHT;

          // width = WINDOW_WIDTH;
          // height = (int)(width / targetAspectRatio + 0.5f);

          // if (height > WINDOW_HEIGHT) {
          //     height = WINDOW_HEIGHT;
          //     width = (int)(height * targetAspectRatio + 0.5f);
          // }

          // int vp_x = (WINDOW_WIDTH / 2) - (width / 2);
          // int vp_y = (WINDOW_HEIGHT / 2) - (height / 2);

          // glViewport(vp_x, vp_y, width, height);

          // glUseProgram(gSpriteShaderProgram);


          // gProjectionMatrix = createOrthographicMatrix(0, width,  height, 0, -1.0F, 1.0f);
  
          // glUniformMatrix4fv(glGetUniformLocation(gSpriteShaderProgram, "projection"), 1, GL_FALSE, (const GLfloat *)gProjectionMatrix.data);



          // gViewMatrix = getViewMatrix((Vec3){0, 0, 0.0});

          // float scale_x = (float)WINDOW_WIDTH / (float)VIRTUAL_WIDTH;
          // float scale_y = (float)WINDOW_HEIGHT / (float)VIRTUAL_HEIGHT;




          // Mat4 scaleMatrix = createScaleMatrix((Vec3){scale_x, scale_y, 1.0});
          // gViewMatrix = multiplyMatrices(gViewMatrix, scaleMatrix);

          // glUniformMatrix4fv(glGetUniformLocation(gSpriteShaderProgram, "view"), 1, GL_FALSE, (const GLfloat *)gViewMatrix.data);


          int windowWidth = gEvent.window.data1;
        int windowHeight = gEvent.window.data2;

        // Adjust viewport and projection for letterboxing
        renderFramebufferToScreen(windowWidth, windowHeight);

        }
        if(gEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        {    
        }
      } break;

      default:
        break;
    }    
  }
}

void love_update(float dt)
{


  if(love_wasPressedR(SDL_SCANCODE_UP))
  {
    gSpritePos.y -= 50 * dt;
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
    gSpritePos.y += 50 * dt;
  }

  if(love_wasPressedR(SDL_SCANCODE_C))
  {
    gCameraPos.x -= 60 * dt;
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