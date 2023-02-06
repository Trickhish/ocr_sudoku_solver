#pragma once

#define MAX_LINES 256
#define PI 3.14159265

struct Vector2D {
  double x;
  double y;
};

struct Line {
  struct Vector2D start;
  struct Vector2D end;
};

typedef struct {
  long x;
  long y;
} Point;

struct Line* getLines(SDL_Surface* img, int* lni);