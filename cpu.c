#include "cpu.h"
#include "font.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void initialize_state(StateChip8 *state)
{
    state->PC = 0x200;
    state->delay_timer = 0;
    state->sound_timer = 0;
    state->I = 0;
    state->stack_pointer = 0; //??
    memset(state->stack, 0, STACK_DEPTH * sizeof(uint16_t));
    memset(state->V, 0, V_REGISTER_COUNT * sizeof(uint8_t));
    memset(state->memory, 0, CHIP8_MEMORY_SIZE * sizeof(uint8_t));
    memset(state->display, 0, CHIP8_DISPLAY_SIZE * sizeof(uint8_t));
    load_font(state);
}

void load_rom(StateChip8 *state, uint8_t *rom, size_t rom_size)
{
    memcpy(state->memory + 0x200, rom, rom_size);
}

void load_font(StateChip8 *state)
{
    uint8_t *font = make_font();
    memcpy(state->memory, font, FONT_SIZE);
}

void update_timers(StateChip8 *state)
{
    if (state->delay_timer > 0)
        state->delay_timer--;
    if (state->sound_timer > 0)
        state->sound_timer--;
}

void draw(StateChip8 *state, uint16_t opcode, uint8_t vx, uint8_t vy)
{
    //DXYN
    //Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N+1 pixels.
    //Each row of 8 pixels is read as bit-coded starting from memory location I;
    //I value doesn’t change after the execution of this instruction.
    //VF is set to 1 if any screen pixels are flipped from set to unset
    //when the sprite is drawn, and to 0 if that doesn’t happen
    uint8_t rx = state->V[vx]; //sprite x
    uint8_t ry = state->V[vy]; //sprite y
    uint8_t sprite_height = opcode & 0x000F;
    uint8_t pixel;
    //reset collision bit
    state->V[0xF] = 0;

    for (int y = 0; y < sprite_height; y++)
    {
        pixel = state->memory[state->I + y];
        for (int x = 0; x < 8; x++)
        {
            if ((pixel & (0x80 >> x)) != 0)
            {
                if (state->display[(ry + y) * DISPLAY_WIDTH + rx + x] == 1)
                {
                    state->V[0xF] = 0;
                }
                state->display[(ry + y) * DISPLAY_WIDTH + rx + x] ^= 1;
            }
        }
    }

    state->draw_flag = 1;
}

void emulate_op(StateChip8 *state)
{
    //fetch
    uint16_t opcode = state->memory[state->PC];
    opcode <<= 8;
    opcode |= state->memory[state->PC + 1];
    state->PC += 2;
    //parse out vx and vy
    uint8_t vx, vy;
    vx = (opcode & 0x0F00) >> 8;
    vy = (opcode & 0x00F0) >> 4;
    //decode first 4 bits - the first nibble of the opcode:
    switch (opcode & 0xF000)
    {
    case 0x0000:
        switch (opcode & 0x00FF)
        {
        case 0x00E0:
            //clear display
            memset(state->display, 0, CHIP8_DISPLAY_SIZE * sizeof(uint8_t));
            break;
        case 0x00EE:
            state->PC = state->stack[state->stack_pointer];
            state->stack_pointer--;
            break;
        }
        break;
    case 0x1000:
        //jump to NNN
        state->PC = opcode & 0x0FFF;
        break;
    case 0x2000:
        //call subroutine at NNN
        state->stack_pointer++;
        state->stack[state->stack_pointer] = state->PC;
        state->PC = opcode & 0x0FFF;
        break;
    case 0x3000:
        //skip if vx == NN
        if (state->V[vx] == (opcode & 0x00FF))
            state->PC += 2;
        break;
    case 0x4000:
        //skip if vx != NN
        if (state->V[vx] != (opcode & 0x00FF))
            state->PC += 2;
        break;
    case 0x5000:
        //skip if VX == VY
        if (state->V[vx] == state->V[vy])
            state->PC += 2;
        break;
    case 0x6000:
        //set register X to NN (6XNN)
        state->V[vx] = opcode & 0x00FF;
        break;
    case 0x7000:
        //add NN to VX
        state->V[vx] += opcode & 0x00FF;
        break;
    case 0x8000:
        //TODO
        switch (opcode & 0x000F)
        {
        case 0x0000:
            //vx = vy
            state->V[vx] = state->V[vy];
            break;
        case 0x0001:
            //vx = vx | vy
            state->V[vx] |= state->V[vy];
            break;
        case 0x0002:
            //vx = vx | vy
            state->V[vx] &= state->V[vy];
            break;
        case 0x0003:
            //vx = vx ^ vy
            state->V[vx] ^= state->V[vy];
            break;
        case 0x0004:
            //vx = vx+vy, vf=carry
            {
                uint16_t result = state->V[vx] + state->V[vy];
                if (result > 0xFF)
                {
                    state->V[0xF] = 1;
                }
                else
                {
                    state->V[0xF] = 0;
                }
                state->V[vx] = result & 0xFF;
            }
            break;
        case 0x0005:
            //VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
            {
                if (state->V[vx] > state->V[vy])
                    state->V[0xF] = 1;
                else
                    state->V[0xF] = 0;
                state->V[vx] -= state->V[vy];
            }
            break;
        case 0x0006:
            //Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
            state->V[0xF] = state->V[vx] & 0x1;
            state->V[vx] >>= 1;
            break;
        case 0x0007:
            //Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
            //vx = vy - vx
            if (state->V[vy] > state->V[vx])
                state->V[0xF] = 1;
            else
                state->V[0xF] = 0;
            state->V[vx] = state->V[vy] - state->V[vx];
            break;
        case 0x000E:
            //Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
            state->V[0xF] = state->V[vx] >> 7;
            state->V[vx] <<= 1;
            break;
        default:
            printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
        }
        break;
    case 0x9000:
        //skips if VX doesn't equal VY.
        if (state->V[vx] != state->V[vy])
            state->PC += 2;
        break;
    case 0xA000:
        //set index register to NNN
        state->I = opcode & 0x0FFF;
        break;
    case 0xB000:
        //jump to NNN plus V0
        state->PC = (opcode & 0xFFF) + state->V[0];
        break;
    case 0xC000:
        //Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
        state->V[vx] = rand() & (opcode & 0x00FF);
        break;
    case 0xD000:
        draw(state, opcode, vx, vy);
        break;
    case 0xE000:
        //key operations
        switch (opcode & 0x00FF)
        {
        case 0x009E:
            //skip next instruction if key at VX is pressed
            if(state->keys[state->V[vx]])
                state->PC+=2;
            break;
        case 0x00A1:
            //skip next instruction if key at VX is not pressed
             if(!(state->keys[state->V[vx]]))
                state->PC+=2;
            break;
        }
        break;
    case 0xF000:
        switch (opcode & 0x00FF)
        {
        case 0x0007:
            //set vx = timer delay
            state->V[vx] = state->delay_timer;
            break;
        case 0x000A:
            //wait for a key press, store it in vx
            if (!state->current_key)
            {
                //should be blocking (e.g. we should not advance PC)
                state->PC -= 2;
                return;
            }
            state->V[vx] = state->current_key;
            break;
        case 0x0015:
            //set delay timer to VX
            state->delay_timer = state->V[vx];
            break;
        case 0x0018:
            //set sound timer to VX
            state->sound_timer = state->V[vx];
            break;
        case 0x001E:
            // add vx to I
            state->I += state->V[vx];
            break;
        case 0x0029:
            //set i to the location of sprite for the character in vx
            state->I = state->V[vx] * 5;
            break;
        case 0x0033:
            //take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.
            state->memory[state->I] = state->V[vx] / 100;
            state->memory[state->I + 1] = (state->V[vx] / 10) % 10;
            state->memory[state->I + 2] = (state->V[vx] % 100) % 10;
            break;
        case 0x0055:
            for (int i = 0; i <= vx; i++)
            {
                state->memory[state->I + i] = state->V[i];
            }
            break;
        case 0x0065:
            //fill v0 to vx with values from memory starting at i
            for (int i = 0; i <= vx; i++)
            {
                state->V[i] = state->memory[state->I + i];
            }
        }
        break;
    default:
        printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
    }
}