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
#include <bits/stdc++.h>
#include "data.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string.h>
#include <vector>
using namespace std;

#define global static

typedef struct button_struct {
  bool pressed; 
  SDL_FRect rect; 
  const char *name;
} button_struct;

typedef struct letter_struct {
  bool pressed; 
  int points;
  SDL_FRect rect; 
  const char *name;
} letter_struct;

typedef struct {
  SDL_FRect rect; 
  const char *name;
  int points;
} word_struct;

typedef struct gameState {
  bool running;
  int currentRound;
  int totalRounds;
  bool roundWon;
  int scoreToWin;
  int newLetters;
  int score;
  bool gameOver;
} gameState;

global SDL_Window *window = NULL;
global SDL_Renderer *renderer = NULL;
global int letters_size = 16;
global vector<word_struct> wordVector;
global vector<letter_struct> letter_structs;
global vector<button_struct> button_structs;
global gameState state {
  .running = true,
  .currentRound = 0,
  .totalRounds = 2,
  .roundWon = false,
  .scoreToWin = 10,
  .newLetters = 3,
  .score = 0,
  .gameOver = false
};

void render_letters(letter_struct letter) {
  if (letter.pressed) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  } else {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  }

  SDL_RenderFillRect(renderer, &letter.rect);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  float scale = 4.0f;
  float text_w = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(letter.name);
  float text_h = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

  SDL_SetRenderScale(renderer, scale, scale);
  float x = (letter.rect.x + (letter.rect.w - text_w * scale) / 2.0f) / scale;
  float y = (letter.rect.y + (letter.rect.h - text_h * scale) / 2.0f) / scale;
  SDL_RenderDebugText(renderer, x, y, letter.name);

  float point_scale = 2.5;
  float point_w = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * (to_string(letter.points)).length();
  float point_h = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
  SDL_SetRenderScale(renderer, point_scale, point_scale);
  float point_x = (letter.rect.x + (letter.rect.w - point_w * point_scale) / 2.0f) / point_scale;
  float point_y = (letter.rect.y + (letter.rect.h - point_h * point_scale) / 2.0f) / point_scale;
  SDL_RenderDebugTextFormat(renderer, point_x+12, point_y+12, "%d",letter.points);

  SDL_SetRenderScale(renderer, 1.0f, 1.0f);
} 

void render_buttons(button_struct button) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  SDL_RenderFillRect(renderer, &button.rect);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  float scale = 4.0f;
  float text_w = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(button.name);
  float text_h = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

  SDL_SetRenderScale(renderer, scale, scale);
  float x = (button.rect.x + (button.rect.w - text_w * scale) / 2.0f) / scale;
  float y = (button.rect.y + (button.rect.h - text_h * scale) / 2.0f) / scale;
  SDL_RenderDebugText(renderer, x, y, button.name);

  SDL_SetRenderScale(renderer, 1.0f, 1.0f);
} 

void render_word(word_struct word) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderFillRect(renderer, &word.rect);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  float scale = 4.0f;
  float text_w = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(word.name);
  float text_h = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

  SDL_SetRenderScale(renderer, scale, scale);
  float x = (word.rect.x + (word.rect.w - text_w * scale) / 2.0f) / scale;
  float y = (word.rect.y + (word.rect.h - text_h * scale) / 2.0f) / scale;
  SDL_RenderDebugText(renderer, x, y, word.name);

  float point_scale = 2.5;
  float point_w = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * (to_string(word.points)).length();
  float point_h = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
  SDL_SetRenderScale(renderer, point_scale, point_scale);
  float point_x = (word.rect.x + (word.rect.w - point_w * point_scale) / 2.0f) / point_scale;
  float point_y = (word.rect.y + (word.rect.h - point_h * point_scale) / 2.0f) / point_scale;
  SDL_RenderDebugTextFormat(renderer, point_x+12, point_y+12, "%d",word.points);

  SDL_SetRenderScale(renderer, 1.0f, 1.0f);
} 

void drawWord(letter_struct letter) {
  auto letterInWord = find_if(
    wordVector.begin(),
    wordVector.end(),
    [&](const word_struct &w) {
    return strcmp(w.name, letter.name) == 0;
    }
  ); 

  if (letter.pressed) {
    float x;
    if (wordVector.size() > 0) {
      x = wordVector.back().rect.x + 200;
    } else {
      x = 100;
    }
    wordVector.push_back({
      .rect = { x, 100, 100, 100 },
      .name = letter.name, 
      .points = letter.points 
    });
  } else { 
    if (letterInWord != wordVector.end()) {
      wordVector.erase(letterInWord);
    }
  }
}

void resetWord() {
  wordVector.clear();
  for (int i = 0; i < letter_structs.size(); i++) {
    letter_structs[i].pressed = false;
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
  SDL_RenderDebugTextFormat(renderer, x, y, "SCORE %d", state.score);
  SDL_RenderDebugTextFormat(renderer, 100, y, "Round %d/%d", state.currentRound, state.totalRounds);
  SDL_RenderDebugTextFormat(renderer, 200, y, "Resets %d", state.newLetters);
  SDL_RenderDebugTextFormat(renderer, 300, y, "Points to win %d", state.scoreToWin);
  SDL_SetRenderScale(renderer, 1.0f, 1.0f);
}

void drawNewLetters() {
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  float scale = 4.0f;
  float text_w = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE; 
  float text_h = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

  SDL_SetRenderScale(renderer, scale, scale);
  float x = 200;
  float y = 10;
  SDL_RenderDebugTextFormat(renderer, x, y, "Resets %d", state.newLetters);
  SDL_SetRenderScale(renderer, 1.0f, 1.0f);

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
}

void drawGameOver() {
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  float scale = 4.0f;
  float text_w = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE; 
  float text_h = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

  SDL_SetRenderScale(renderer, scale, scale);
  float x = 130;
  float y = 60;
  SDL_RenderDebugText(renderer, x, y, "GAME OVER");
  SDL_RenderDebugTextFormat(renderer, 250, y, "SCORE %d", state.score);
  SDL_SetRenderScale(renderer, 1.0f, 1.0f);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

  button_structs.clear();
  button_structs.push_back({
    .pressed = false,
    .rect = { 400, 400, 300, 150 },
    .name = "Exit",
  }); 
  button_structs.push_back({
    .pressed = false,
    .rect = { 800, 400, 300, 150 },
    .name = "Try again",
  }); 

  for(int i = 0; i < button_structs.size(); i++) {
    render_buttons(button_structs[i]);
  }
}

void drawRoundWon() {
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  float scale = 4.0f;
  float text_w = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE; 
  float text_h = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

  SDL_SetRenderScale(renderer, scale, scale);
  float x = 130;
  float y = 60;
  SDL_RenderDebugText(renderer, x, y, "YOU WON");
  SDL_RenderDebugTextFormat(renderer, 200, y, "SCORE %d", state.score);

  SDL_SetRenderScale(renderer, 1.0f, 1.0f);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

  button_structs.clear();
  button_structs.push_back({
    .pressed = false,
    .rect = { 400, 400, 300, 150 },
    .name = "Exit",
  }); 
  button_structs.push_back({
    .pressed = false,
    .rect = { 800, 400, 400, 150 },
    .name = "Next round",
  }); 

  for(int i = 0; i < button_structs.size(); i++) {
    render_buttons(button_structs[i]);
  }
}

void drawLetters() {
  letter_structs.clear();

  for (int i = 0; i < letters_size; i++) {
    float x;
    float y;

    if (i == 0) {
      x = 600;
      y = 300;

    } else if (i % 4 == 0) {
      x = 600;
      y = letter_structs[i - 1].rect.y + 200;
    } else if (i != 0) {
      x = letter_structs[i - 1].rect.x + 200;
      y = letter_structs[i - 1].rect.y;
    } 
    char letter = letters[rand() % 26];

    letter_structs.push_back({
      .pressed = false,
      .points = points[letter],
      .rect = { x, y, 100, 100 },
      .name = new char[2] { letter, '\0' },
    }); 
  }
}

void newLetters(bool use=false) {
  if (state.newLetters > 0) {
    if (use) {
      state.newLetters--;
    }
    resetWord();
    drawLetters();
  } 
}

void shuffleLetters() {
  string letterString;
  for (int i = 0; i < letter_structs.size(); i++) {
    letterString.append(letter_structs[i].name);
  }

  shuffle(letterString.begin(), letterString.end(), default_random_engine(0));

  for (int i = 0; i < letter_structs.size(); i++) {
    letter_structs[i].name = new char[2] { letterString[i], '\0'};
  }
}

void roundWon() {
  if (state.totalRounds < 6) {
    state.totalRounds++;
  }
  state.scoreToWin += 5;
  state.roundWon = true;
}

void checkRound() {
  if (state.score >= state.scoreToWin) {
    roundWon();
    return;
  }

  if (state.currentRound >= state.totalRounds) {
    if (state.score < state.scoreToWin) {
      state.gameOver = true;
    } else {
      roundWon();
    }
  }
}

void setScore() {
  string s;
  int potentialScore = 0;
  for (int i = 0; i < wordVector.size(); i++) {
    s.append(wordVector[i].name);
    potentialScore += wordVector[i].points;
  }
  auto realWord = find(words.begin(), words.end(), s);

  if (realWord != words.end()){
    state.score += potentialScore;
    int multiplier = 5;
    int over_5 = wordVector.size() - 5;

    if (over_5 > 0) {
      state.score += over_5 * multiplier;
    }
    newLetters();
    state.currentRound++;
    resetWord();
    checkRound();
  } 
}

void drawButtons() {
  button_structs.push_back({
    .pressed = false,
    .rect = { 1450, 700, 300, 150 },
    .name = "Submit"
  }); 
  button_structs.push_back({
    .pressed = false,
    .rect = { 1450, 400, 300, 150 },
    .name = "Reset",
  }); 
  button_structs.push_back({
    .pressed = false,
    .rect = { 100, 400, 400, 150 },
    .name = "New letters",
  }); 
  button_structs.push_back({
    .pressed = false,
    .rect = { 100, 700, 300, 150 },
    .name = "Shuffle",
  }); 
}

void restart() {
  SDL_RenderClear(renderer);
  state.score = 0;
  state.newLetters = 3;
  state.gameOver = false;
  state.roundWon = false;
  state.currentRound = 0;
  button_structs.clear();
  drawButtons();
  resetWord();
  newLetters();
}

void removeWordLetter(vector<word_struct> wordVector, int i) {
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

  drawLetters();
  drawButtons();

  while (state.running) {
    SDL_Event event;
    uint64_t PerfCountFrequency = SDL_GetPerformanceFrequency();
    uint64_t LastCounter = SDL_GetPerformanceCounter();

    if (state.gameOver) {
      drawGameOver();
    } else if (state.roundWon) {
      drawRoundWon();
    } else {
      drawScore();
      for(int i = 0; i < letter_structs.size(); i++) {
        render_letters(letter_structs[i]);
      }
      for(int i = 0; i < button_structs.size(); i++) {
        render_buttons(button_structs[i]);
      }
      for(int i = 0; i < wordVector.size(); i++) {
        render_word(wordVector[i]);
      }
    }

    while (SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_EVENT_QUIT:
          state.running = false;
          break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
          SDL_FPoint point = { event.button.x, event.button.y };
          for (int i = 0; i < letter_structs.size(); i++) {
            letter_struct& obj = letter_structs[i];
            if (SDL_PointInRectFloat(&point, &obj.rect)) {
              if (obj.pressed) {
                obj.pressed = false;
                drawWord(obj);
              } else {
                obj.pressed = true;
                drawWord(obj);
              }
            }
          }
          for (int i = 0; i < wordVector.size(); i++) {
            if (SDL_PointInRectFloat(&point, &wordVector[i].rect)) {
              auto letterInWord = find_if(
                letter_structs.begin(),
                letter_structs.end(),
                [&](const letter_struct &l) {
                  return strcmp(l.name, wordVector[i].name) == 0;
                }
              ); 
              if (letterInWord != letter_structs.end()) {
                size_t index = distance(letter_structs.begin(), letterInWord);
                letter_structs[index].pressed = false;
              }

              wordVector.erase(wordVector.begin() + i);
            }
          }
          if (wordVector.size() > 0) {
            printf("WORD %f\n", wordVector.front().rect.x);
          }

          for (int j = 0; j < wordVector.size(); j++) {
            float x = 100 + 200 * j;
            wordVector[j].rect.x = x;
          }
          for (int i = 0; i < button_structs.size(); i++) {
            button_struct& obj = button_structs[i];
            if (SDL_PointInRectFloat(&point, &obj.rect)) {
              if (strcmp(obj.name, "Submit") == 0) {
                setScore();
              } 
              if (strcmp(obj.name, "Reset") == 0) {
                resetWord();
              }
              if (strcmp(obj.name, "New letters") == 0) {
                newLetters(true);
              }
              if (strcmp(obj.name, "Shuffle") == 0) {
                shuffleLetters();
              }
              if (strcmp(obj.name, "Exit") == 0) {
                state.running = false;
              }
              if (strcmp(obj.name, "Try again") == 0) {
                restart();
              }
              if (strcmp(obj.name, "Next round") == 0) {
                restart();
              }
            }
          }
          break;
        }
      }
    }

    SDL_RenderPresent(renderer);

    uint64_t EndCounter = SDL_GetPerformanceCounter();
    uint64_t CounterElapsed = EndCounter - LastCounter;

    double MSPerFrame(((1000.0f * (double)CounterElapsed) / (double)PerfCountFrequency));
    double FPS = (double)PerfCountFrequency / (double)CounterElapsed;

    // printf("%.02f ms/f, FPS: %.02f/s\n", MSPerFrame, FPS);
    SDL_Delay(16);
    LastCounter = EndCounter;
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
