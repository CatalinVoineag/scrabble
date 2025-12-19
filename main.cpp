#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_timer.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include "data.cpp"
#include <vector>
#include <string.h>
using namespace std;

#define global static

typedef struct {
  bool pressed; 
  SDL_FRect rect; 
  const char *name;
} button_struct;

global SDL_Window *window = NULL;
global SDL_Renderer *renderer = NULL;
global bool running = true;

global int letters_size = 16;
global int buttons_size = letters_size;
global vector<button_struct> word;
global vector<button_struct> button_structs;
global int score = 0;

void render_button(SDL_FRect *rect, const char *name, bool pressed) {
  if (pressed) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
  } else {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  }

  SDL_RenderFillRect(renderer, rect);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  float scale = 4.0f;
  float text_w = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(name);
  float text_h = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

  SDL_SetRenderScale(renderer, scale, scale);
  float x = (rect->x + (rect->w - text_w * scale) / 2.0f) / scale;
  float y = (rect->y + (rect->h - text_h * scale) / 2.0f) / scale;
  SDL_RenderDebugText(renderer, x, y, name);

  SDL_SetRenderScale(renderer, 1.0f, 1.0f);
} 

void drawWord() {
  int index = rand() % 6;
  const char *random_word = words[index];

  float x = 300;
  for (int i = 0; i < strlen(random_word); i++) {
    word.push_back({
      .pressed = false,
      .rect = { x, 100, 100, 100 },
      .name = new char[2] { random_word[i], '\0' } 
    });
    x += 200;
  }
}

void drawScore() {
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  float scale = 4.0f;
  float text_w = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE; 
  float text_h = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

  SDL_SetRenderScale(renderer, scale, scale);
  float x = 10;
  float y = 10;
  SDL_RenderDebugTextFormat(renderer, x, y, "SCORE %d", score);
  SDL_SetRenderScale(renderer, 1.0f, 1.0f);
}

void drawLetters() {
  for (int i = 0; i < letters_size; i++) {
    float x;
    float y;

    if (i == 0) {
      x = 600;
      y = 300;

    } else if (i % 4 == 0) {
      x = 600;
      y = button_structs[i - 1].rect.y + 200;
    } else if (i != 0) {
      x = button_structs[i - 1].rect.x + 200;
      y = button_structs[i - 1].rect.y;
    } 

    button_structs.push_back({
      .pressed = false,
      .rect = { x, y, 100, 100 },
      .name = new char[2] { letters[rand() % 26], '\0' }
    }); 
  }
}

int main() {
  srand((unsigned)time(NULL));
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  if (!SDL_CreateWindowAndRenderer("Word", 1920, 1080, SDL_WINDOW_EXTERNAL, &window, &renderer)) {
    SDL_Log("Cannot create window %s\n", SDL_GetError());
  } 

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  SDL_Keycode keycode;

  drawWord();
  drawLetters();
  button_structs.push_back({
    .pressed = false,
    .rect = { 1450, 700, 300, 150 },
    .name = "Submit"
  }); 
  buttons_size++;

  while (running) {
    SDL_Event event;
    uint64_t PerfCountFrequency = SDL_GetPerformanceFrequency();
    uint64_t LastCounter = SDL_GetPerformanceCounter();
    drawScore();

    while (SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_EVENT_QUIT:
          running = false;
          break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
          SDL_FPoint point = { event.button.x, event.button.y };
          for (int i = 0; i < buttons_size; i++) {
            button_struct& obj = button_structs[i];
            
            if (SDL_PointInRectFloat(&point, &obj.rect)) {
              if (obj.pressed) {
                obj.pressed = false;
              } else {
                obj.pressed = true;
                if (strcmp(obj.name, "Submit") == 0) {
                  score++;
                }
              }
            }
          }
          break;
        }
        case SDL_EVENT_KEY_DOWN:
          keycode = event.key.key;
          switch(keycode) {
            case SDLK_W:
              break;
            case SDLK_S:
              break;
            case SDLK_D:
              break;
            case SDLK_A:
              break;
          }
          break;
      }
    }
    for(int i = 0; i < word.size(); i++) {
      render_button(&word[i].rect, word[i].name, word[i].pressed);
    }

    for(int i = 0; i < buttons_size; i++) {
      render_button(
        &button_structs[i].rect,
        button_structs[i].name,
        button_structs[i].pressed
      );
    }
    SDL_RenderPresent(renderer);

    uint64_t EndCounter = SDL_GetPerformanceCounter();
    uint64_t CounterElapsed = EndCounter - LastCounter;

    double MSPerFrame(((1000.0f * (double)CounterElapsed) / (double)PerfCountFrequency));
    double FPS = (double)PerfCountFrequency / (double)CounterElapsed;

    // printf("%.02f ms/f, FPS: %.02f/s\n", MSPerFrame, FPS);
    LastCounter = EndCounter;
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
