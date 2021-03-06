#pragma once
#include <stdint.h>

#define V_REGISTER_COUNT 16
#define STACK_DEPTH 16
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define CHIP8_DISPLAY_SIZE DISPLAY_WIDTH*DISPLAY_HEIGHT
#define CHIP8_MEMORY_SIZE 4096
#define CHIP8_KEY_COUNT 16
#define FONT_OFFSET 0x10
#define PROGRAM_OFFSET 0x200

typedef struct StateChip8 {
    uint8_t memory[CHIP8_MEMORY_SIZE];
    uint16_t PC;
    uint16_t I;
    uint8_t V[V_REGISTER_COUNT];
    uint16_t stack[STACK_DEPTH];
    uint8_t stack_pointer;
    uint8_t sound_timer;
    uint8_t delay_timer;
    uint8_t display[CHIP8_DISPLAY_SIZE];
    int draw_flag;
    uint8_t keys[CHIP8_KEY_COUNT];
} StateChip8;