#include "cpu.h"
#include "font.h"
#include <string.h>

void initialize_state(StateChip8 *state)
{
    state->PC = 0;
    state->delay_timer = 0;
    state->sound_timer = 0;
    state->I = 0;
    state->stack_pointer = 0; //??
    memset(state->stack, 0, STACK_DEPTH * sizeof(uint16_t));
    memset(state->V, 0, V_REGISTER_COUNT * sizeof(uint8_t));
    memset(state->stack, 0, STACK_DEPTH * sizeof(uint16_t));
    memset(state->V, 0, V_REGISTER_COUNT * sizeof(uint8_t));
    load_font(state);
}

void load_rom(StateChip8 *state, uint8_t* rom, size_t rom_size)
{
    memcpy(state->memory + 0x200, rom, rom_size);
}

void load_font(StateChip8 *state)
{
    uint8_t *font = make_font();
    memcpy(state->memory, font, FONT_SIZE);
}

int emulate_op(StateChip8 *state)
{
    //fetch
    uint16_t opcode = (uint16_t)state->memory[state->PC];
    // uint8_t opcode = state->memory[state->PC];
    state->PC += 2;

    //decode
}