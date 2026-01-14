#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint> // For uint16_t
#include <stack> // Stack variable
#include <iostream>
#include <fstream> // Getting rom
#include <string> // Specifying path
#include <cstring>
#include <random> // For CXNN
#include <chrono> // For timer and display
#include <stdexcept>
#include <vector>
#include <SDL2/SDL.h> // IO, sound

#define SCALE 10

class Chip8 {

    public: 
    static constexpr int CHIP_8 = 1;
    static constexpr int SUPER_CHIP = 2; // Modern
    
    // Constructor
    Chip8(int type = CHIP_8) : chip(type), high_res(false), running(true), key(false), index(0) {   
        add_fonts();
        reset();

    }

    void cycle(); // Advances execution
    bool poll(SDL_Event event); // Gets all inputs
    void display(SDL_Renderer* renderer); // Shows display state, 60 HZ
    void load_game(const std::string& path); // Loads game into memory

    uint8_t get_delay_countdown();
    void decrement_delay_countdown();
    uint8_t get_sound_countdown();
    void decrement_sound_countdown();
    bool get_display_changed();
    void set_display_changed(bool state);
    bool is_running();

    private: 

    uint16_t PC; // Program Counter
    uint8_t memory[0x1000]; // 4 kB long
    uint16_t I; // Index Register
    uint8_t V[16]; // 16 variable registers
    uint8_t flag[8]; // Used to save and load registers in SUPER_CHIP
    uint8_t delay_countdown; // Timers' countdown values
    uint8_t sound_countdown;
    std::stack<uint16_t> stack; // Reserve the stack, LIFO
    bool screen[64][32]; // [x][y] 
    bool screen_super[128][64];

    int pixels_vertical; // SUPER_CHIP scroll down amount, should be negative or 0
    int pixels_horizontal; // SUPER_CHIP horizontally scrolled amount, left is negative, right is positive

    int chip;
    bool keypad[16]; // 1-4 down to Z-V
    bool display_changed; // 1 if instruction changed display state
    bool high_res;
    bool running;

    void reset(); // Reset to boot state
    void add_fonts(); // Adds fonts to reserved memory 0x050 - 0x09F

    // For FX0A
    bool key;
    uint8_t index;
};

#endif