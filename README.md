# Chip8-SuperChip

A cycle-accurate CHIP-8 and SUPER-CHIP emulator written in C++20 using SDL2, supporting both low-resolution and high-resolution graphics modes.


## Features

- Full CHIP-8 instruction set
- SUPER-CHIP extensions (128×64 high-resolution mode)
- Accurate sprite collision detection (VF flag)
- Configurable clock speed
- SDL2-based graphics, input, and timing
- Proper display scaling using SDL logical rendering
- ROM loading from disk

 
## Key Mapping

CHIP-8 keypad mapped to keyboard:

1 2 3 4      →   1 2 3 C  
Q W E R      →   4 5 6 D  
A S D F      →   7 8 9 E  
Z X C V      →   A 0 B F


## Build Requirements

- C++
- SDL2


## Bash

make  
./chip8
- Follow the in-terminal instructions
- If you get: "Failed to open rom", try putting the ROM into the current folder and type only the name.


## Architecture

- CPU: Runs fetch, decode, execute with a configurable cycle rate
- Memory: 4 KB, with dedicated memory ending at 0x200
- Display: 64x32 for Chip8, 128x64 for SuperChip @ 60 Hz
- Input: Polled using SDL


## Notes

- Does not support XO-Chip


## Credits

Thank you to Tobias V. Langhoff for his guide on how to get started and to Timendus for the tests. 

Link to guide: https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#add-super-chip-support  
Link to tests: https://github.com/Timendus/chip8-test-suite?tab=readme-ov-file
  

