#include <stdio.h>
#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "treat.h"

#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

void sharpen(SDL_Surface *image) {
    Uint32 *pixels = (Uint32 *)image->pixels;
    int width = image->w;
    int height = image->h;
    Uint32 *temp_pixels = (Uint32 *)malloc(width * height * sizeof(Uint32));

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int sum_r = 0, sum_g = 0, sum_b = 0;
            Uint8 r[9], g[9], b[9];
            int k = 0;
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    Uint8 r_temp, g_temp, b_temp;
                    SDL_GetRGB(pixels[(y + i) * width + x + j], image->format, &r_temp, &g_temp, &b_temp);
                    r[k] = r_temp;
                    g[k] = g_temp;
                    b[k] = b_temp;
                    k++;
                }
            }
            sum_r = -r[0] - r[2] - r[6] - r[8] + 5 * r[1] + 5 * r[3] + 5 * r[5] + 5 * r[7];
            sum_g = -g[0] - g[2] - g[6] - g[8] + 5 * g[1] + 5 * g[3] + 5 * g[5] + 5 * g[7];
            sum_b = -b[0] - b[2] - b[6] - b[8] + 5 * b[1] + 5 * b[3] + 5 * b[5] + 5 * b[7];
            double ch = 0.05;
            temp_pixels[y * width + x] = SDL_MapRGB(image->format, CLAMP(sum_r*ch, 0, 255), CLAMP(sum_g*ch, 0, 255), CLAMP(sum_b*ch, 0, 255));
        }
    }
    memcpy(pixels, temp_pixels, width * height * sizeof(Uint32));
    free(temp_pixels);
}

void dilation(SDL_Surface *image, int size) {
    Uint32 *pixels = (Uint32 *)image->pixels;
    int width = image->w;
    int height = image->h;
    Uint32 *temp_pixels = (Uint32 *)malloc(width * height * sizeof(Uint32));

    int half_size = size / 2;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Uint8 max_r = 0, max_g = 0, max_b = 0;
            for (int k = -half_size; k <= half_size; k++) {
                for (int l = -half_size; l <= half_size; l++) {
                    int x_k = x + k;
                    int y_l = y + l;
                    if (x_k >= 0 && x_k < width && y_l >= 0 && y_l < height) {
                        Uint8 r, g, b;
                        SDL_GetRGB(pixels[y_l * width + x_k], image->format, &r, &g, &b);
                        max_r = MAX(max_r, r);
                        max_g = MAX(max_g, g);
                        max_b = MAX(max_b, b);
                    }
                }
            }
            temp_pixels[y * width + x] = SDL_MapRGB(image->format, max_r, max_g, max_b);
        }
    }
    memcpy(pixels, temp_pixels, width * height * sizeof(Uint32));
    free(temp_pixels);
}

void contrast(SDL_Surface *image, double contrast) {
    Uint32 *pixels = (Uint32 *)image->pixels;
    int width = image->w;
    int height = image->h;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Uint8 r, g, b;
            SDL_GetRGB(pixels[y * width + x], image->format, &r, &g, &b);
            r = (Uint8)CLAMP(((double)r - 128.0) * contrast + 128.0, 0, 255);
            g = (Uint8)CLAMP(((double)g - 128.0) * contrast + 128.0, 0, 255);
            b = (Uint8)CLAMP(((double)b - 128.0) * contrast + 128.0, 0, 255);
            pixels[y * width + x] = SDL_MapRGB(image->format, r, g, b);
        }
    }
}

void otsu_threshold(SDL_Surface *image) {
    int histogram[256] = {0};
    int total = image->w * image->h;
    Uint32* pixels = image->pixels;

    for (int y = 0; y < image->h; y++) {
        for (int x = 0; x < image->w; x++) {
            Uint32 pixel = pixels[y * image->w + x];

            //Uint8 gray = (pixel & 0xff) + ((pixel >> 8) & 0xff) + ((pixel >> 16) & 0xff);
            //gray /= 3;
            Uint8 gray = 0.3 * (pixel & 0xff) + 0.59 * ((pixel >> 8) & 0xff) + 0.11 * ((pixel >> 16) & 0xff);

            histogram[gray]++;
        }
    }

    // Calculate probability of each gray level
    float prob[256];
    for (int i = 0; i < 256; i++) {
        prob[i] = (float)histogram[i] / total;
    }

    // Calculate cumulative mean
    float omega[256];
    omega[0] = prob[0];
    for (int i = 1; i < 256; i++) {
        omega[i] = omega[i - 1] + prob[i];
    }

    // Calculate cumulative mean of the classes between class means
    float micro[256];
    micro[0] = 0;
    for (int i = 1; i < 256; i++) {
        micro[i] = micro[i - 1] + i * prob[i];
    }

    // Determine threshold
    float max_sigma = -1;
    int threshold = 0;
    for (int i = 0; i < 256; i++) {
        float sigma = powf(micro[255] * omega[i] - micro[i], 2) / (omega[i] * (1 - omega[i]));
        if (sigma > max_sigma) {
            max_sigma = sigma;
            threshold = i;
        }
    }

    // Binarize image
    for (int y = 0; y < image->h; y++) {
        for (int x = 0; x < image->w; x++) {
            Uint32 pixel = pixels[y * image->w + x];

            //Uint8 gray = (pixel & 0xff) + ((pixel >> 8) & 0xff) + ((pixel >> 16) & 0xff);
            //gray /= 3;
            Uint8 gray = 0.3 * (pixel & 0xff) + 0.59 * ((pixel >> 8) & 0xff) + 0.11 * ((pixel >> 16) & 0xff);
            int v = gray<threshold ? 0 : 255;

            Uint32 color = SDL_MapRGB(image->format, v, v, v);
            pixels[y * image->w + x] = color;
        }
    }
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

SDL_Surface* resize_image(SDL_Surface *image, int new_width, int new_height) {
    SDL_Surface *resized = SDL_CreateRGBSurface(0, new_width, new_height, 32, 0, 0, 0, 0);
    if (resized == NULL) {
        return NULL;
    }

    double x_ratio = (double)image->w / new_width;
    double y_ratio = (double)image->h / new_height;
    double px, py;

    Uint32* pixels = image->pixels;
    Uint32* pixelsr = resized->pixels;

    for (int y = 0; y < new_height; y++) {
        for (int x = 0; x < new_width; x++) {
            px = floor(x * x_ratio);
            py = floor(y * y_ratio);

            //Uint32 pix = pixels[py * image->w + px];
            //pixelsr[y * resized->w + x] = pix;
        }
    }

    return resized;
}