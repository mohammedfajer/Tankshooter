#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h> // Include the header for rand() function


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
#include "./engine/win32_fileIO.h"
#include "./engine/gl_texture.h"

typedef struct {
  int borderWidth;
  int borderHeight;
  int viewportWidth;
  int viewportHeight;
} ViewportRegion;

ViewportRegion setViewport(int windowWidth, int windowHeight, int contentWidth, int contentHeight) ;
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

const float FIXED_TIMESTEP = 1.0f / 60.0f;
float accumulator = 0.0f;




GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource);

static Vec3 gCameraPos = {0,0,0};
#include "quad.c"
#include "quadBatch.c"

// Quad Batch
QuadBatch gQuadBatch;


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

}
// Function to calculate sinusoidal motion between two endpoints and back
void sinusoidalMotion(float *y, float t, float dt) {
    // Calculate the amplitude and period of the sinusoidal motion
    float amplitude = (VIRTUAL_HEIGHT - 20) / 2; // Adjusted amplitude to fit within the screen
    float period = 2.f; // One full oscillation per second

    // Calculate the intermediate position using sine function
    *y = VIRTUAL_HEIGHT / 2 + amplitude * sin(2 * M_PI * t / period)  ;
    
   
}

void render_scene()
{ 
  


  // render_sprite_quad(VIRTUAL_WIDTH / 2, gSpritePos.y, 640 - 10, 10, 0.0f);
  // printf("spritePos: %d %d\n", (int)gSpritePos.x, (int)gSpritePos.y);


  // static float angle = 0.0f;
  // angle += 0.01f;
  // render_sprite_quad(VIRTUAL_WIDTH / 2, VIRTUAL_HEIGHT / 2, 50, 50, angle);

  // Render the QuadBatch
  render_quad_batch(&gQuadBatch);
  
}

void renderToFramebuffer() {

    glClearColor(40.0f / 255.0f, 45.0f / 255.0f, 52.0f / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Set the shader and matrices for rendering
    glUseProgram(gSpriteShaderProgram);
    //gProjectionMatrix = createOrthographicMatrix(0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, 0, -1.0f, 100.0f);
    //glUniformMatrix4fv(glGetUniformLocation(gSpriteShaderProgram, "projection"), 1, GL_FALSE, (const GLfloat *)gProjectionMatrix.data);

    gViewMatrix = getViewMatrix((Vec3){gCameraPos.x, gCameraPos.y, 0.0});
    // Mat4 translateCamera = createTranslationMatrix2((Vec3){gCameraPosx, gCameraPos.y, 0.0f});
    // gViewMatrix = multiplyMatrices(gViewMatrix, translateCamera);
    glUniformMatrix4fv(glGetUniformLocation(gSpriteShaderProgram, "view"), 1, GL_TRUE, (const GLfloat *)gViewMatrix.data);

    // Assuming you have a way to get the current time, e.g., using SDL_GetTicks()
    float time = SDL_GetTicks() / 1000.0f; // Convert milliseconds to seconds

    glUseProgram(gSpriteShaderProgram);
    glUniform1f(glGetUniformLocation(gSpriteShaderProgram, "time"), time);

    // Render your scene here
    render_scene();

    
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

  // Set OpenGL attributes
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); // Enable multisampling
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // Choose the number of samples (4 in this case)


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

  // Enable multisampling
  glEnable(GL_MULTISAMPLE);

  glEnable(GL_BLEND); // Enable blending for transparency
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  

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






  glUseProgram(gSpriteShaderProgram);

  gViewMatrix = getViewMatrix((Vec3){0,0, 0.0});
  
  gProjectionMatrix = createOrthographicMatrix(0, VIRTUAL_WIDTH,  VIRTUAL_HEIGHT, 0, -1.0F, 1.0f);
  glUniformMatrix4fv(glGetUniformLocation(gSpriteShaderProgram, "view"), 1, GL_FALSE, (const GLfloat *)gViewMatrix.data);
  glUniformMatrix4fv(glGetUniformLocation(gSpriteShaderProgram, "projection"), 1, GL_FALSE, (const GLfloat *)gProjectionMatrix.data);

  gSpritePos = (Vec3){0,0,0};


  // Init Quad Batch
  // Initialize the QuadBatch
    init_quad_batch(&gQuadBatch);

    // Add some quads
    // add_quad_color(&gQuadBatch, (Vec2){100, 100}, (Vec2){50, 50}, (Vec4){1,0,0,1});
    // add_quad_color(&gQuadBatch, (Vec2){200, 200}, (Vec2){100, 100}, (Vec4){0, 1, 0, 1});

    Texture2D texture = loadTexture("./data/graphics/background.png");
    gQuadBatch.globalTexId = texture.id;

  {
    int windowWidth, windowHeight;
SDL_GetWindowSize(gWindow, &windowWidth, &windowHeight);

ViewportRegion region = setViewport(windowWidth, windowHeight, VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

// Calculate the aspect ratio of the viewport region
float aspectRatio = (float)region.viewportWidth / region.viewportHeight;

// Calculate the aspect ratio of the texture
float textureAspectRatio = (float)texture.w / texture.h;

float scaleX, scaleY;

if (textureAspectRatio > aspectRatio) {
    // Texture is wider than the viewport, so we need to adjust the height
    scaleY = (float)region.viewportHeight / texture.h;
    scaleX = scaleY * textureAspectRatio; // Maintain texture aspect ratio
} else {
    // Texture is taller than or equal to the viewport, so we need to adjust the width
    scaleX = (float)region.viewportWidth / texture.w;
    scaleY = scaleX / textureAspectRatio; // Maintain texture aspect ratio
}

// Ensure the scaled texture remains within the bounds of the viewport
if (scaleX * texture.w > region.viewportWidth) {
    scaleX = region.viewportWidth / texture.w;
}
if (scaleY * texture.h > region.viewportHeight) {
    scaleY = region.viewportHeight / texture.h;
}

// Now, use scaleX and scaleY to scale your texture appropriately
add_quad_texture(&gQuadBatch, (Vec2){1 , 1}, (Vec2){scaleX * texture.w, scaleY * texture.h}, (Vec4){0, 1, 0, 1}, texture.id);
  }

 
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
          int windowWidth = gEvent.window.data1;
          int windowHeight = gEvent.window.data2;


                   


          // Adjust viewport and projection for letterboxing
          //renderFramebufferToScreen(windowWidth, windowHeight);
        }
        
      } break;

      default:
        break;
    }    
  }
}

void love_update(float dt)
{
  const Uint8* states = SDL_GetKeyboardState(NULL);

  


  if(states[SDL_SCANCODE_UP])
  {
    gSpritePos.y -= (int) 20 * dt * 2.5f ;
  }
  if(states[SDL_SCANCODE_DOWN])
  {
    gSpritePos.y += (int)20 * dt * 2.5f;
  }
  
  if(states[SDL_SCANCODE_RIGHT])
  {
    gSpritePos.x += (int)20 * dt * 2.5f;
  }
  if(states[SDL_SCANCODE_LEFT])
  {
    gSpritePos.x -= (int)20 * dt * 2.5f;
  }
   

  if(love_wasPressedR(SDL_SCANCODE_C))
  {
    gCameraPos.x -= (0.25f * dt);
  }

  if(love_wasPressed(SDL_SCANCODE_ESCAPE)) gRunning = false;

   //SDL_memset(keys, 0, SDL_NUM_SCANCODES * sizeof(bool));
  //SDL_memset(keysR, 0, SDL_NUM_SCANCODES * sizeof(bool));


  // Render your scene here (sprites, objects, etc.)
  static float t = 0.0f; t+=0.01f;
    float y;
        sinusoidalMotion(&y, t, dt);
  gSpritePos.y = y;
}




ViewportRegion setViewport(int windowWidth, int windowHeight, int contentWidth, int contentHeight) {

    ViewportRegion region = {};

    // Calculate the aspect ratios
    float windowAspect = (float)windowWidth / (float)windowHeight;
    float contentAspect = (float)contentWidth / (float)contentHeight;

    int viewportWidth, viewportHeight;
    int viewportX, viewportY;

    if (windowAspect > contentAspect) {
        // Window is wider than content
        viewportHeight = windowHeight;
        viewportWidth = (int)(contentAspect * viewportHeight);
        viewportX = (windowWidth - viewportWidth) / 2;
        viewportY = 0;
    } else {
        // Window is taller than content
        viewportWidth = windowWidth;
        viewportHeight = (int)(viewportWidth / contentAspect);
        viewportX = 0;
        viewportY = (windowHeight - viewportHeight) / 2;
    }

    // Set the viewport
    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

    region.borderWidth = viewportX;
    region.borderHeight = viewportY;
    region.viewportWidth = viewportWidth;
    region.viewportHeight = viewportHeight;

    gProjectionMatrix = createOrthographicMatrix(0, (float)VIRTUAL_WIDTH, (float)VIRTUAL_HEIGHT, 0.0f, -1.0f, 1.0f);

   

    glUniformMatrix4fv(glGetUniformLocation(gSpriteShaderProgram, "projection"), 1, GL_FALSE, (const GLfloat *)gProjectionMatrix.data);
    



    return (region);
}



void love_draw()
{
 
  int windowWidth, windowHeight;
  SDL_GetWindowSize(gWindow, &windowWidth, &windowHeight);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set the viewport for content rendering
  ViewportRegion region = setViewport(windowWidth, windowHeight, VIRTUAL_WIDTH, VIRTUAL_HEIGHT); // Example content size (800x600)

   // Enable scissor testing to restrict rendering to the viewport area
    glEnable(GL_SCISSOR_TEST);
    glScissor(region.borderWidth, region.borderHeight, region.viewportWidth, region.viewportHeight);

    // Render your scene with color A
      renderToFramebuffer();

     

    // Disable scissor testing to render the black areas outside of the viewport
    glDisable(GL_SCISSOR_TEST);


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


    // TODO(mo): digest everything and find way to solve diagonal movement jitter.

    input_handling();

    // Calculate delta time
    Uint32 currentTime = SDL_GetTicks();
    dt = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;


    

    accumulator += dt;
    while(accumulator >= FIXED_TIMESTEP)
    {
      love_update(FIXED_TIMESTEP);
      accumulator -= FIXED_TIMESTEP;


     
    }

    //love_update(dt);
    love_draw();

    // // Increment frame count
    // frameCount++;

    // // Calculate FPS every second
    // if (SDL_GetTicks() - fpsLastTime >= 1000) {
    //     fpsCounter = frameCount / ((SDL_GetTicks() - fpsLastTime) / 1000.0f);
    //     frameCount = 0;
    //     fpsLastTime = SDL_GetTicks();
    // }
    
    // // Frame time management
    // Uint32 frameTime = SDL_GetTicks() - frameStart;
    // if (frameTime < FRAME_DELAY) {
    //     SDL_Delay(FRAME_DELAY - frameTime);
    // }
  }
  return 0;
}