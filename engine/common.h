#pragma once

#include <SDL.h>
#include <stdio.h>

#define bool int
#define false 0
#define true 1

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define SIZE 200
#define SPEED 600
#define GRAVITY 600 
#define FPS 60
#define JUMP -1200

#define VIRTUAL_WIDTH 640 
#define VIRTUAL_HEIGHT 360 

#define MAX_KEYS SDL_NUM_SCANCODES

enum GameState {
   StartState,
   PlayState,
   HighScoreState,
   EnterHighScoreState,
   GameOverState,
   PaddleSelectState,
   ServeState,
   VictoryState
};

// Structure to hold state information
typedef struct 
{
    enum GameStates state;
    void (*enter)(void);
    void (*update)(float dt);
    void (*render)(void);
} State;

// Structure to manage the current state
typedef struct 
{
    State *currentState;
} StateManager;

void state_manager_switch( StateManager *sm,  State *newState);


typedef struct {
  Uint8 r;
  Uint8 g;
  Uint8 b;
} Color;

enum Mode {FILL, LINE};


#define WHITE_COLOR (SDL_Color){255,255,255}


#define Foreach(a, b) for(int i=a; i<= b; i++)