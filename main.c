//TODO set up 64x32 display
//TODO

#include <SDL2/SDL.h>
#include <stdio.h>
#include "state.h"
#include "cpu.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320

static SDL_Surface *chip8buffer = NULL;
static SDL_Surface *argbbuffer = NULL;
static SDL_Texture *texture = NULL;
static SDL_Renderer *renderer;
static StateChip8 *state;

void draw_chip8_screen(SDL_Surface *surface, StateChip8 *state)
{
    SDL_LockSurface(surface);
    //draw random junk
    uint32_t *pixels = (uint32_t*)surface->pixels;
    for(int i = 0; i < CHIP8_DISPLAY_SIZE; i++){
        pixels[i] = state->display[i] == 0 ? 0 : 0xFFFFFFFF;
    }
    SDL_UnlockSurface(surface);
}

void update(){
    //draw some stuff into the buffer
    draw_chip8_screen(chip8buffer, state);
    SDL_RenderClear(renderer);
    //surface to texture
    SDL_BlitScaled(chip8buffer, NULL, argbbuffer, NULL);
    // Update the intermediate texture with the contents of the RGBA buffer.
    SDL_UpdateTexture(texture, NULL, argbbuffer->pixels, argbbuffer->pitch);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

uint8_t* read_testrom(size_t *rom_size) {
	FILE* file = fopen("roms/logo.ch8", "rb");
	if (!file) {
		printf("Couldn't load roms/logo.ch8");
		exit(1);
	}
    //get file size
    fseek(file, 0, SEEK_END);
    int size = (int)ftell(file);
    *rom_size = size;
    fseek(file, 0, SEEK_SET);
    
	uint8_t *rom_buffer = malloc(size);

	int read = fread(rom_buffer, sizeof(uint8_t), size, file);
	fclose(file);
	return rom_buffer;
}

void emu_cycle(){
    emulate_op(state);
}

int main(int argc, char *args[])
{
    state = (StateChip8*) malloc(sizeof(StateChip8));
    initialize_state(state);
    size_t rom_size;
    uint8_t *rom = read_testrom(&rom_size);
    load_rom(state, rom, rom_size);
    free(rom);
    
    //draw some junk
    for(int i = 0; i < 32; i++){
        int coord = i * DISPLAY_WIDTH + i;
        state->display[coord] = 255;
    }
    SDL_Window *window = NULL;
    SDL_Surface *screenSurface = NULL;
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow(
        "emuchip8",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (window == NULL)
    {
        fprintf(stderr, "could not create window: %s\n", SDL_GetError());
        return 1;
    }
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);
    screenSurface = SDL_GetWindowSurface(window);

    //set up other surfaces
    chip8buffer = SDL_CreateRGBSurface(0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 32, 0, 0, 0, 0);
    argbbuffer = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);

    while (1)
    {
      // Get the next event
      SDL_Event event;
      if (SDL_PollEvent(&event))
      {
        if (event.type == SDL_QUIT)
        {
          // Break out of the loop on quit
          break;
        }
      }
      //cycle the emulator
      emu_cycle();
      update();
    }
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}