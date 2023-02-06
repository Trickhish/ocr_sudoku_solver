#include <stdio.h>
#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "treat.h"

#include <SDL2/SDL.h>
#include <stdbool.h>

void erosion(SDL_Surface* image) {
    
}

void gaussian_filter(SDL_Surface *imageSurface, int blur_extent) //This manipulates with SDL_Surface and gives it box blur effect
{
    for (int y = 0; y < imageSurface->h; y++)
    {
        for (int x = 0; x < (imageSurface->pitch / 4); x++)
        {
            Uint32 color = ((Uint32*)imageSurface->pixels)[(y * (imageSurface->pitch / 4)) + x];

            //SDL_GetRGBA() is a method for getting color
            //components from a 32 bit color
            Uint8 r = 0, g = 0, b = 0;
            SDL_GetRGB(color, imageSurface->format, &r, &g, &b);

            Uint32 rb = 0, gb = 0, bb = 0;

            //Within the two for-loops below, colors of adjacent pixels are added up

            for (int yo = -blur_extent; yo <= blur_extent; yo++)
            {
                for (int xo = -blur_extent; xo <= blur_extent; xo++) {
                    if (y + yo >= 0 && x + xo >= 0 && y + yo < imageSurface->h && x + xo < (imageSurface->pitch / 4)) {
                        Uint32 colOth = ((Uint32*)imageSurface->pixels)[((y + yo) * (imageSurface->pitch / 4)) + (x + xo)];

                        Uint8 ro = 0, go = 0, bo = 0;
                        SDL_GetRGB(colOth, imageSurface->format, &ro, &go, &bo);

                        rb += ro;
                        gb += go;
                        bb += bo;
                    }
                }
            }

            r = (Uint8)(rb / (((blur_extent * 2) + 1) * ((blur_extent * 2) + 1)));
            g = (Uint8)(gb / (((blur_extent * 2) + 1) * ((blur_extent * 2) + 1)));
            b = (Uint8)(bb / (((blur_extent * 2) + 1) * ((blur_extent * 2) + 1)));

            //color = (r << 24) | (g << 8) | (b << 16) | (b);
            color = SDL_MapRGB(imageSurface->format, r, g, b);
            ((Uint32*)imageSurface->pixels)[(y * (imageSurface->pitch / 4)) + x] = color;
        }
    }
}

Uint8 rgbL(uint n) {
    return((Uint8) n<0 ? 0 : (n>255 ? 255 : n));
}

void contrast_stretch(SDL_Surface* image) {
    if (SDL_LockSurface(image) < 0) {
        errx(1, "Error locking surface: %s\n", SDL_GetError());
        return;
    }
    Uint32* pixels = image->pixels;
    int len = image->w * image->h;

    int rf[256]={0};
    int gf[256]={0};
    int bf[256]={0};

    for (int i=0; i<len; i++) {
        Uint32 pixel = pixels[i];
        Uint8 r = pixel >> 16 & 0xFF;
        Uint8 g = pixel >> 8 & 0xFF;
        Uint8 b = pixel & 0xFF;
        rf[r]++;
        gf[g]++;
        bf[b]++;
    }

    int mxr=0, mxg=0, mxb=0, mnr=0, mng=0, mnb=0;
    for (size_t i=0; i<256; i++) {
        if (rf[i]>rf[mxr]) {
            mxr=i;
        } else if (rf[i]<mnr) {
            mnr=i;
        }
        if (gf[i]>gf[mxg]) {
            mxg=i;
        } else if (gf[i]<mng) {
            mng=i;
        }
        if (bf[i]>bf[mxb]) {
            mxb=i;
        } else if (bf[i]<mnb) {
            mnb=i;
        }
    }

    for (int i=0; i<len; i++) {
        Uint32 pixel = pixels[i];
        Uint8 r = pixel >> 16 & 0xFF;
        Uint8 g = pixel >> 8 & 0xFF;
        Uint8 b = pixel & 0xFF;

        r=rgbL( (r-mnr) * (255/(mxr-mnr)) );
        g=rgbL( (g-mng) * (255/(mxg-mng)) );
        b=rgbL( (r-mnb) * (255/(mxb-mnb)) );

        pixels[i] = SDL_MapRGB(image->format, r, g, b);
    }

    SDL_UnlockSurface(image);
}

void denoise(SDL_Surface *image, int n) {
  Uint32 *pixels = (Uint32 *) image->pixels;
  int w = image->w;
  int h = image->h;

  if (n<1) {
    n=1;
  }

  for (int i = n; i < h - n; i++) {
    for (int j = n; j < w - n; j++) {
      Uint32 centerPixel = pixels[i * w + j];

      // Get the RGB values of the center pixel
      Uint8 centerR, centerG, centerB;
      SDL_GetRGB(centerPixel, image->format, &centerR, &centerG, &centerB);

      // Compute the average of the surrounding pixels
      Uint32 sumR = 0, sumG = 0, sumB = 0;
      for (int ii = -n; ii <= n; ii++) {
        for (int jj = -n; jj <= n; jj++) {
          Uint32 surroundingPixel = pixels[(i + ii) * w + (j + jj)];

          Uint8 surroundingR, surroundingG, surroundingB;
          SDL_GetRGB(surroundingPixel, image->format, &surroundingR, &surroundingG, &surroundingB);

          sumR += surroundingR;
          sumG += surroundingG;
          sumB += surroundingB;
        }
      }

      int pxn = (n*2+1)*(n*2+1);

      Uint8 avgR = sumR / pxn;
      Uint8 avgG = sumG / pxn;
      Uint8 avgB = sumB / pxn;

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

        Uint8 v = 0.3 * r + 0.59 * g + 0.11 * b;
        pixels[i] = (0xFF << 24) | (v << 16) | (v << 8) | v;
    }

    SDL_UnlockSurface(image);
}

void histo(unsigned int histo[256], unsigned w,
           unsigned h, unsigned int image[w][h]) {
  for (unsigned int i = 0; i < w; i++)
    for (unsigned int j = 0; j < h; j++)
      histo[image[i][j]] += 1;
}

void otsu_threshold() {
    int h[256];
    for (unsigned int i = 0; i < w; i++)
        for (unsigned int j = 0; j < h; j++)
            h[image[i][j]] += 1;
        }
    }
}

void threshold(SDL_Surface* image, int nth) {
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
        v = (v<nth ? 0 : 255);
        pixels[i] = (v << 24) | (v << 16) | (v << 8) | v;
    }

    SDL_UnlockSurface(image);
}

