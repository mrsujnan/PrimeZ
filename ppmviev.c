#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

// Function to load a PPM image into an SDL texture
SDL_Texture *loadPPM(const char *filename, SDL_Renderer *renderer, int *width,
                     int *height) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open file: %s\n", filename);
    return NULL;
  }

  char format[3];
  if (fscanf(file, "%2s", format) != 1 || strcmp(format, "P3") != 0) {
    fprintf(stderr, "Invalid PPM format (must be P3): %s\n", filename);
    fclose(file);
    return NULL;
  }

  // Skip comments
  int c;
  while ((c = fgetc(file)) == '#') {
    while (fgetc(file) != '\n')
      ;
  }
  ungetc(c, file);

  // Read width, height, and max color value
  int maxColorValue;
  if (fscanf(file, "%d %d %d", width, height, &maxColorValue) != 3 ||
      maxColorValue != 255) {
    fprintf(stderr, "Invalid PPM header: %s\n", filename);
    fclose(file);
    return NULL;
  }

  // Allocate memory for pixel data
  Uint8 *pixels = malloc((*width) * (*height) * 3);
  if (!pixels) {
    fprintf(stderr, "Failed to allocate memory for image\n");
    fclose(file);
    return NULL;
  }

  // Read pixel data
  for (int i = 0; i < (*width) * (*height) * 3; i++) {
    if (fscanf(file, "%hhu", &pixels[i]) != 1) {
      fprintf(stderr, "Invalid pixel data in file: %s\n", filename);
      free(pixels);
      fclose(file);
      return NULL;
    }
  }
  fclose(file);

  // Create SDL texture
  SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
      pixels, *width, *height, 24, *width * 3, 0x0000FF, 0x00FF00, 0xFF0000, 0);
  if (!surface) {
    fprintf(stderr, "Failed to create SDL surface: %s\n", SDL_GetError());
    free(pixels);
    return NULL;
  }

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
  free(pixels);

  if (!texture) {
    fprintf(stderr, "Failed to create SDL texture: %s\n", SDL_GetError());
    return NULL;
  }

  return texture;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <ppm-image-file>\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "SDL Initialization Error: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  // Create an SDL window
  SDL_Window *window = SDL_CreateWindow(
      "PPM Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
      SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (!window) {
    fprintf(stderr, "Window Creation Error: %s\n", SDL_GetError());
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // Create an SDL renderer
  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!renderer) {
    fprintf(stderr, "Renderer Creation Error: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // Load the PPM image
  int imgWidth, imgHeight;
  SDL_Texture *texture = loadPPM(argv[1], renderer, &imgWidth, &imgHeight);
  if (!texture) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_FAILURE;
  }

  // Main event loop
  int running = 1;
  SDL_Event event;
  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = 0;
      }
    }

    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    // Render the texture
    SDL_Rect destRect = {0, 0, 800, 600};
    SDL_RenderCopy(renderer, texture, NULL, &destRect);

    // Present the renderer
    SDL_RenderPresent(renderer);
  }

  // Clean up
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return EXIT_SUCCESS;
}
