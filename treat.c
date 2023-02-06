#include <stdio.h>
#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "treat.h"

#include <SDL2/SDL.h>
#include <stdbool.h>

void denoise(SDL_Surface *image) {
  Uint32 *pixels = (Uint32 *) image->pixels;
  int w = image->w;
  int h = image->h;

  for (int i = 1; i < h - 1; i++) {
    for (int j = 1; j < w - 1; j++) {
      Uint32 centerPixel = pixels[i * w + j];

      // Get the RGB values of the center pixel
      Uint8 centerR, centerG, centerB;
      SDL_GetRGB(centerPixel, image->format, &centerR, &centerG, &centerB);

      // Compute the average of the surrounding pixels
      Uint32 sumR = 0, sumG = 0, sumB = 0;
      for (int ii = -1; ii <= 1; ii++) {
        for (int jj = -1; jj <= 1; jj++) {
          Uint32 surroundingPixel = pixels[(i + ii) * w + (j + jj)];

          Uint8 surroundingR, surroundingG, surroundingB;
          SDL_GetRGB(surroundingPixel, image->format, &surroundingR, &surroundingG, &surroundingB);

          sumR += surroundingR;
          sumG += surroundingG;
          sumB += surroundingB;
        }
      }

      Uint8 avgR = sumR / 9;
      Uint8 avgG = sumG / 9;
      Uint8 avgB = sumB / 9;

      // Replace the center pixel with the average of the surrounding pixels if it deviates too much
      if (abs(centerR - avgR) > 50 || abs(centerG - avgG) > 50 || abs(centerB - avgB) > 50) {
        pixels[i * w + j] = SDL_MapRGB(image->format, avgR, avgG, avgB);
      }
    }
  }
}

void grayscale_convert(SDL_Surface* image) {
    if (SDL_LockSurface(image) < 0) {
        errx(1, "Error locking surface: %s\n", SDL_GetError());
        return;
    }

    Uint32* pixels = image->pixels;
    int len = image->w * image->h;

    for (int i=0; i<len; i++) {
        Uint32 pixel = pixels[i];
        Uint8 r = pixel >> 16 & 0xFF;
        Uint8 g = pixel >> 8 & 0xFF;
        Uint8 b = pixel & 0xFF;

        Uint8 v = 0.212671f * r + 0.715160f * g + 0.072169f * b;
        pixels[i] = (0xFF << 24) | (v << 16) | (v << 8) | v;
    }

    SDL_UnlockSurface(image);
}

void threshold(SDL_Surface* image) {
    if (SDL_LockSurface(image) < 0) {
        errx(1, "Error locking surface: %s\n", SDL_GetError());
        return;
    }

    Uint32* pixels = image->pixels;
    int len = image->w * image->h;

    for (int i=0; i<len; i++) {
        Uint32 pixel = pixels[i];
        Uint8 r = pixel >> 16 & 0xFF;
        Uint8 g = pixel >> 8 & 0xFF;
        Uint8 b = pixel & 0xFF;

        Uint8 v = 0.212671f * r + 0.715160f * g + 0.072169f * b;
        v = (v<128 ? 0 : 255);
        pixels[i] = (0xFF << 24) | (v << 16) | (v << 8) | v;
    }

    SDL_UnlockSurface(image);
}

