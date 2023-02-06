#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

void grayscale_convert(SDL_Surface* image);
void denoise(SDL_Surface* image);
void threshold(SDL_Surface* image);
