A CHIP-8 emulator in C

Resources:
https://en.wikipedia.org/wiki/CHIP-8
https://tobiasvl.github.io/blog/write-a-chip-8-emulator/

ROMs:
https://github.com/loktar00/chip8/tree/master/roms

### Virtual machine description:

64x32 pixel monochrome display
4K of 8-bit RAM
16 8-bit "variable" registers V0-VF
I 16-bit address register
Stack of 16-bit addresses for call/return
16-bit PC - program counter
8-bit delay timer (decremented at 60 Hz) until it reaches 0
8-bit sound timer (decremented at 60 Hz), beeps when it reaches 0
keypad 0x1-0xF

### Instruction set

See https://en.wikipedia.org/wiki/CHIP-8#Opcode_table

### Timing

Standard execution speed of around 700 instructions per second