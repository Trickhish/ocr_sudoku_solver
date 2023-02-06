#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <math.h>
#include "grid_detection.h"

int isblack(int r, int g, int b) {
    return(((r+g+b)/2) < 125);
}

int dist(int a, int b) {
    return(a>b ? a-b : b-a);
}

void drawLine(SDL_Surface* img, int c, int m) {
    SDL_Rect rect;
    if (m==0) {
        rect.x=0;
        rect.y=c;
        rect.w=img->w;
        rect.h=5;
    } else {
        rect.x=c;
        rect.y=0;
        rect.w=5;
        rect.h=img->h;
    }

    SDL_FillRect(img, &rect, SDL_MapRGB(img->format, 255,0,0));
}

void drawCross(SDL_Surface* img, int x, int y) {
    SDL_Rect rect = {x-30,y-5, 60,10};
    SDL_FillRect(img, &rect, SDL_MapRGB(img->format, 255,0,0));

    SDL_Rect rect2 = {x-5,y-30, 10,60};
    SDL_FillRect(img, &rect2, SDL_MapRGB(img->format, 255,0,0));
}

struct Line* LineOfCords(int x, int y, int xx, int yy) {
    struct Line* l;
    struct Vector2D sv = {x,y};
    struct Vector2D ev = {xx,yy};
    (l).start = sv;
    (l).end = ev;
    return(l);
}

struct Vector2D lineIntersection(struct Line line1, struct Line line2) {
  struct Vector2D result = {NAN, NAN};

  struct Vector2D a = line1.start;
  struct Vector2D b = line1.end;
  struct Vector2D c = line2.start;
  struct Vector2D d = line2.end;

  double denominator = ((b.x - a.x) * (d.y - c.y)) - ((b.y - a.y) * (d.x - c.x));
  if (denominator == 0) {
    return result;
  }

  double numerator1 = ((a.y - c.y) * (d.x - c.x)) - ((a.x - c.x) * (d.y - c.y));
  double numerator2 = ((a.y - c.y) * (b.x - a.x)) - ((a.x - c.x) * (b.y - a.y));

  double r = numerator1 / denominator;
  double s = numerator2 / denominator;

  if ((r < 0 || r > 1) || (s < 0 || s > 1)) {
    // intersection outside of line segment
    return result;
  }

  result.x = a.x + (r * (b.x - a.x));
  result.y = a.y + (r * (b.y - a.y));

  return result;
}

struct Line* getLines(SDL_Surface* img, int* lni) {
     Uint32 *pixels = (Uint32 *)img->pixels;

    int w=img->w;
    int h=img->h;
    int x=w/2;
    int y=0;
    int lb=-2;
    int stb=0;

    struct Line* ll = LineOfCords(0,0,0,0);

    *lni=-1;

    while (y<h) {
        Uint8 r, g, b;
        SDL_GetRGB(pixels[y * w + x], img->format, &r, &g, &b);

        if (isblack(r,g,b)) {
            if (lb==-2) {
                y++;
                x++;
                continue;
            }
            if (!stb && dist(y,lb)>10) {
                struct Line l = {{0,y}, {w,y}};
                lni++;
                *(ll+ *lni)=l;
                printf("ligne a y=%i\n",y);
                drawLine(img, y, 0);

                stb=1;
            }
            lb=y;
        } else {
            if (lb==-2) {
                lb=-1;
            }
            stb=0;
        }

        y++;
    }

    y=h/2;
    x=0;
    lb=-2;
    stb=0;
    while (x<w) {
        Uint8 r, g, b;
        SDL_GetRGB(pixels[y * w + x], img->format, &r, &g, &b);

        if (isblack(r,g,b)) {
            if (lb==-2) {
                y++;
                x++;
                continue;
            }
            if (!stb && dist(x,lb)>10) {
                struct Line l = {{x,0}, {x,h}};
                lni++;
                *(ll+ *lni)=l;
                printf("ligne a x=%i\n",y);
                drawLine(img, x, 1);

                stb=1;
            }
            lb=x;
        } else {
            if (lb==-2) {
                lb=-1;
            }
            stb=0;
        }

        x++;
    }

    return(ll);
}