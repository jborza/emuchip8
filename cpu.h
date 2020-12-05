#include "state.h"
#include <stddef.h>

void load_rom(StateChip8 *state, uint8_t *rom, size_t rom_size);
void initialize_state(StateChip8 *state);
int emulate_op(StateChip8 *state);
void load_font(StateChip8 *state);