#include <stdio.h>
#include "treat.h"
#include "grid_detection.h"

int main() {
    SDL_Surface* oim = IMG_Load("2.jpg");
    SDL_Surface* img = SDL_ConvertSurfaceFormat(oim, SDL_PIXELFORMAT_RGB888, 0);

    sharpen(img);
    contrast(img, 1.5);
    otsu_threshold(img);

    IMG_SaveJPG(img, "out1.jpg", 90);

    int* lni=-1;
    struct Line* lns = geteLines(img, lni);

    /*
    SDL_Rect rect = {50, 50, 100, 100};
    SDL_RenderFillRect(img, &rect);
    */

    IMG_SaveJPG(img, "out2.jpg", 90);

    SDL_FreeSurface(oim);
    SDL_FreeSurface(img);

    return(0);
}
