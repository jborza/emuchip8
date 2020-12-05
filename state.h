#pragma once
#include <stdint.h>

#define V_REGISTER_COUNT 16
#define STACK_DEPTH 16
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define DISPLAY_SIZE DISPLAY_WIDTH*DISPLAY_HEIGHT
#define CHIP8_MEMORY_SIZE 4096

typedef struct StateChip8 {
    uint8_t* memory;
    uint16_t PC;
    uint16_t I;
    uint8_t V[V_REGISTER_COUNT];
    uint16_t stack[STACK_DEPTH];
    uint8_t stack_pointer;
    uint8_t sound_timer;
    uint8_t delay_timer;
    uint8_t display[DISPLAY_SIZE];
} StateChip8;