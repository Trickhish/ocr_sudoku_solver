#include <stdio.h>
#include "treat.h"

int main() {
    SDL_Surface* oim = IMG_Load("2.jpg");
    SDL_Surface* img = SDL_ConvertSurfaceFormat(oim, SDL_PIXELFORMAT_RGB888, 0);
    
    //grayscale_convert(img);
    threshold(img);
    
    IMG_SaveJPG(img, "out1.jpg", 90);

    denoise(img);

    IMG_SaveJPG(img, "out2.jpg", 90);
    
    SDL_FreeSurface(oim);
    SDL_FreeSurface(img);

    return(0);
}
