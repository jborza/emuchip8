//TODO set up 64x32 display
//TODO

#include <SDL2/SDL.h>
#include <stdio.h>
#include "state.h"
#include "cpu.h"
#include "keymap.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320

static SDL_Surface *chip8buffer = NULL;
static SDL_Surface *argbbuffer = NULL;
static SDL_Texture *texture = NULL;
static SDL_Renderer *renderer;
static StateChip8 *state;

uint32_t total_cycles;

void draw_chip8_screen(SDL_Surface *surface, StateChip8 *state)
{
    SDL_LockSurface(surface);
    //draw random junk
    uint32_t *pixels = (uint32_t *)surface->pixels;
    for (int i = 0; i < CHIP8_DISPLAY_SIZE; i++)
    {
        pixels[i] = state->display[i] == 0 ? 0 : 0xFFFFFFFF;
    }
    SDL_UnlockSurface(surface);
}

void update()
{
    //draw some stuff into the buffer
    draw_chip8_screen(chip8buffer, state);
    //surface to texture
    SDL_BlitScaled(chip8buffer, NULL, argbbuffer, NULL);
    // Update the intermediate texture with the contents of the RGBA buffer.
    SDL_UpdateTexture(texture, NULL, argbbuffer->pixels, argbbuffer->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

uint8_t *read_testrom(size_t *rom_size)
{
    // FILE* file = fopen("roms/BC_test.ch8", "rb");
    // FILE* file = fopen("roms/Sierpinski.ch8", "rb");
    // FILE* file = fopen("roms/BMP Viewer - Hello (C8 example) [Hap, 2005].ch8", "rb"); //OK
    // FILE *file = fopen("roms/15 Puzzle [Roger Ivie].ch8", "rb");
    // char* name = "roms/Breakout (Brix hack) [David Winter, 1997].ch8";
    // char* name = "roms/SQRT Test [Sergey Naydenov, 2010].ch8"; //OK
    // char* name = "roms/Sierpinski.ch8"; //OK
    // char* name = "roms/Space Invaders [David Winter].ch8";
    // char* name = "roms/sinusoid.ch8";
    char* name = "roms/keyboard.ch8";
    FILE *file = fopen(name,"rb");
    if (!file)
    {
        printf("Couldn't load ROM!");
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

void emu_cycle()
{
    emulate_op(state);
    total_cycles++;

    //TODO better timing
    if(total_cycles % 9 == 0)
        update_timers(state);
    usleep(1850);
}

int main(int argc, char *args[])
{
    state = (StateChip8 *)malloc(sizeof(StateChip8));
    initialize_state(state);
    size_t rom_size;
    uint8_t *rom = read_testrom(&rom_size);
    load_rom(state, rom, rom_size);
    free(rom);

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
            else if(event.type == SDL_KEYDOWN){
                for(int i = 0; i < CHIP8_KEY_COUNT; i++){
                    if(event.key.keysym.sym == KEYMAP[i]){
                        state->keys[i] = 1;
                    }
                }
            }
            else if(event.type == SDL_KEYUP){
                for(int i = 0; i < CHIP8_KEY_COUNT; i++){
                    if(event.key.keysym.sym == KEYMAP[i]){
                        state->keys[i] = 0;
                    }
                }
            }
        }
        //cycle the emulator
        emu_cycle();
        if (state->draw_flag)
        {
            state->draw_flag = 0;
            update();
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
