#include "state.h"
//fetch/decode/execute loop

void initialize(StateChip8 *state){
    state->PC = 0;
    state->delay_timer = 0;
    state->sound_timer = 0;
    state->I = 0;
    state->stack_pointer = 0; //??
}

void load_rom(StateChip8 *state){
    //TODO implement
}

int emulate_op(StateChip8 *state)
{
    //fetch
    uint16_t opcode = (uint16_t)state->memory[state->PC] 
    // uint8_t opcode = state->memory[state->PC];
    state->PC += 2;

    //decode
    
}