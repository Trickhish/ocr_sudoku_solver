#include <stdio.h>
#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

int truncate(int value)
{
    if(value < 0) return 0;
    if(value > 255) return 255;

    return value;
}

/**
 * Converts a colored pixel into grayscale.
 *
 * pixel_color: Color of the pixel to convert in the RGB format.
 * format: Format of the pixel used by the surface.
 */

Uint32 pixel_to_grayscale(Uint32 pixel_color, SDL_PixelFormat* format)
{
    Uint8 r, g, b;

    SDL_GetRGB(pixel_color, format, &r, &g, &b);

    //r = g = b = (r+g+b) / 3;
    r = g = b = 0.3*r + 0.59*g + 0.11*b;
    Uint32 color = SDL_MapRGB(format, r, g, b);

    return color;
}

/**
 * Converts a colored pixel to either a black or white pixel
 *
 * pixel_color: Color of the pixel to convert in the RGB format.
 * format: Format of the pixel used by the surface
 */

Uint32 binarize_pixel(Uint32 pixel_color, SDL_PixelFormat* format)
{
    Uint8 r, g, b;

    SDL_GetRGB(pixel_color, format, &r, &g, &b);

    //brightness
    int bright = 0;
    r = (Uint8) truncate((int) r - bright);
    g = (Uint8) truncate((int) g - bright);
    b = (Uint8) truncate((int) b - bright);

    //contrast
    int contrast = 0;
    float factor = (259.0 * (contrast + 255.0)) / (255.0 * (259.0 - contrast));
    r = (Uint8) truncate((int) (factor * (r - 128) + 128));
    g = (Uint8) truncate((int) (factor * (g - 128) + 128));
    b = (Uint8) truncate((int) (factor * (b - 128) + 128));

    int avg = 0.299 * r + 0.587 * g + 0.114 * b;
    //int avg = r+g+b/3;

/*    if (avg > 113)
        r = g = b = 0;
    else
        r = g = b = 255;
*/
    r = g = b = avg;
    Uint32 color = SDL_MapRGB(format, r, g, b);

    return color;
}

/**
 * Applies pixel_to_grayscale() to every pixel in the given surface.
 *
 * surface: Surface to be neutralized/grayscaled.
 */

void surface_to_grayscale(SDL_Surface* surface)
{
    Uint32* pixels = surface->pixels;
    int len = surface->w * surface->h;// gets the length of pixels with the
                                      // width and length of the given surface.
    SDL_PixelFormat* format = surface->format;

    if (SDL_LockSurface(surface) < 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    for (int i = 0;i<len;i++)
    {
        //pixels[i] = binarize_pixel(pixels[i],format);
        pixels[i] = pixel_to_grayscale(pixels[i],format);
    }

    SDL_UnlockSurface(surface);
}

int main() {
    SDL_Surface* image = IMG_Load("1.jpg");
    if(!image) {
        printf("Erreur de chargement de l'image : %s",SDL_GetError());
        return -1;
    }

    surface_to_grayscale(image);

    IMG_SaveJPG(image, "out.jpg", 50);
    SDL_FreeSurface(image);
    return(0);

    //Uint32* pixels = image->pixels;
    //int len = image->w * image->h;
    printf("w:%i, h:%i\n", image->w, image->h);

    SDL_PixelFormat* format = image->format;
    Uint32* pixels = image->pixels;
    int len = image->w * image->h;

    printf("\ncoucou\n");

    for (int i=0; i<len; i++) {
        pixels[i] = pixel_to_grayscale(pixels[i],format);
    }

    IMG_SaveJPG(image, "out.jpg", 50);
    SDL_FreeSurface(image);

    return(0);
}