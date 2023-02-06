#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

void contrast_stretch(SDL_Surface* image);
void grayscale_convert(SDL_Surface* image);
void gaussian_filter(SDL_Surface *image, int n);
void denoise(SDL_Surface* image, int n);
void threshold(SDL_Surface* image, int nth);
