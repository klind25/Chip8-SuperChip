#include "chip8.h"

void Chip8::cycle() {
    // Infinite loop of fetch, decode, execute
    uint16_t instruction = memory[PC] << 8 | memory[PC + 0x001];
    PC += 0x002;

    switch (instruction >> 12) {
        case 0x0: {
            switch (instruction & 0xFF) {
                case 0xE0: { // Clear screen                         
                    memset(screen, 0, sizeof(screen));
                    memset(screen_super, 0, sizeof(screen_super)); // SUPER_CHIP high resolution screen
                    display_changed = 1;                                        
                    break;
                }
                case 0xEE: { // Returning from subroutine
                    PC = stack.top();
                    stack.pop();
                    break;
                }

                // SUPER_CHIP specific instructions
                case 0xFB: { // Scroll right
                    pixels_horizontal -= 4;
                    break;
                }

                case 0xFC: { // Scroll left
                    pixels_horizontal += 4;
                    break;
                }

                case 0xFD: {
                    running = false;
                    break;
                }

                case 0xFE: {
                    high_res = false;
                    break;
                }

                case 0xFF: {
                    high_res = true;
                    break;
                }

                default:
                    break;
            }

            switch ((instruction >> 4) & 0xF) {
                case 0xC: { // Scroll down
                    pixels_vertical -= instruction & 0xF;
                    break;
                }

                default:
                    break;

            }
            break;
        }
                    
        case 0x1: { // Jump
            PC = instruction & 0xFFF;
            break;
        }

        case 0x2: { // Call subroutine
            stack.push(PC);
            PC = instruction & 0xFFF;
            break;
        }
        
        // Jump conditionally
        case 0x3: {
            if (V[(instruction >> 8) & 0xF] == (instruction & 0xFF)) {
                PC += 0x002;
            }
            break;
        }

        case 0x4: {
            if (V[(instruction >> 8) & 0xF] != (instruction & 0xFF)) {
                PC += 0x002;
            }
            break;
        }

        case 0x5: {
            if (V[(instruction >> 8) & 0xF] == V[(instruction >> 4) & 0xF]) {
                PC += 0x002;
            }
            break;
        }

        case 0x6: { // Set
            V[(instruction >> 8) & 0xF] = instruction & 0xFF;
            break;
        }

        case 0x7: { // Addition
            V[(instruction >> 8) & 0xF] += instruction & 0xFF;
            break;
        }

        case 0x8: { // Logic and Arithmetic
            int x = (instruction >> 8) & 0xF;
            int y = (instruction >> 4) & 0xF;

            switch (instruction & 0xF) {
                case 0x0: {
                    V[x] = V[y];
                    break;
                }

                case 0x1: {
                    V[x] |= V[y];
                    switch(chip) {
                        case CHIP_8: {
                            V[15] = 0;
                            break;
                        }
                        default:
                            break;
                    }
                    
                    break;
                }

                case 0x2: {
                    V[x] &= V[y];
                    switch(chip) {
                        case CHIP_8: {
                            V[15] = 0;
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                }

                case 0x3: {
                    V[x] ^= V[y];
                    switch(chip) {
                        case CHIP_8: {
                            V[15] = 0;
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                }

                case 0x4: { // Addition with possible overflow
                    int Vx = V[x];
                    int Vy = V[y];

                    V[x] += V[y];

                    if (Vx+Vy > 255) {
                        V[15] = 1;
                    }
                    else {
                        V[15] = 0;
                    }

                    break;
                }

                case 0x5: { // Subtraction with possible underflow
                    int Vx = V[x];
                    int Vy = V[y];

                    V[x] -= V[y];

                    if (Vx-Vy >= 0) {
                        V[15] = 1;
                    }
                    else {
                        V[15] = 0;
                    }

                    break;
                }

                case 0x6: { // Shift
                    switch (chip) {
                        case CHIP_8: {
                            uint8_t holder = V[y] & 0x1;
                            V[x] = V[y] >> 1;
                            V[15] = holder;
                            break;
                        }
                        
                        case SUPER_CHIP: { 
                            uint8_t holder = V[x] & 0x1;
                            V[x] = V[x] >> 1;
                            V[15] = holder;
                            break;
                        }

                    }
                    break;
                }

                case 0x7: { // Subtraction with possible underflow
                    int Vx = V[x];
                    int Vy = V[y];

                    V[x] = V[y] - V[x];

                    if (Vy-Vx >= 0) {
                        V[15] = 1;
                    }
                    else {
                        V[15] = 0;
                    }

                    break;
                }

                case 0xE: {
                    switch (chip) {
                        case CHIP_8: {
                            uint8_t holder = (V[y] & 0x80) >> 7;
                            V[x] = V[y] << 1;
                            V[15] = holder;
                            break;
                        }

                        case SUPER_CHIP: {   
                            uint8_t holder = (V[x] & 0x80) >> 7;
                            V[x] = V[x] << 1;
                            V[15] = holder;
                            break;
                        }
                        
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
            

        // Last jump conditionally
        case 0x9: {
            if (V[(instruction >> 8) & 0xF] != V[(instruction >> 4) & 0xF]) {
                PC += 0x002;
            }
            break;
        }

        case 0xA: { // Set index
            I = instruction & 0xFFF;
            break;
        }

        case 0xB: { // Jump with offset
            switch (chip) {
                case CHIP_8: {
                    PC = (instruction & 0xFFF) + V[0];
                    break;
                }
                
                case SUPER_CHIP: {   
                    PC = (instruction & 0xFFF) + V[(instruction >> 8) & 0xF];
                    break;
                }
            }
            break;
        }

        case 0xC: { // Random number 
            std::random_device rd; // Seeding random number generator
            std::mt19937 mt(rd());
            std::uniform_int_distribution<uint8_t> dist(0, 255); // Define distribution

            V[(instruction >> 8) & 0xF] = dist(mt) & (instruction & 0xFF);
            break;
        }

        case 0xD: { // Display
            switch (chip) {
                case CHIP_8: {
                    int width = 63;
                    int height = 31;

                    uint8_t X0 = V[(instruction >> 8) & 0xF] & width; // AND 63 is the same as modulo
                    uint8_t Y = V[(instruction >> 4) & 0xF] & height;
                    uint8_t rows;
                    V[15] = 0;
                    uint8_t data;
                    uint8_t X;
                    uint8_t bit_value;
                    display_changed = 1;

                    for (rows = 0; rows < (instruction & 0xF); ++rows) {
                        data = memory[I + rows];
                        X = X0;

                        for (int i = 0; i < 8; ++i) {
                            bit_value = (data >> (7 - i)) & 1;

                            if (bit_value && screen[X][Y]) { // Both 1, turn off and set flag
                                screen[X][Y] = 0;
                                V[15] = 1;                               
                            }
                            else if (bit_value && !screen[X][Y]) { // Screen off, turn on
                                screen[X][Y] = 1;
                            }

                            if (X == (width + 1)) { // Reached edge of screen
                                continue;
                            }
                            ++X;
                        }
                        
                        if (Y == (height + 1)) { // Reached bottom of screen
                            continue;
                        }
                        ++Y;
                    
                    }
                    break;
                }
            
                case SUPER_CHIP: {   
                    
                    int width = high_res ? 127 : 63; // High res on/off
                    int height = high_res ? 63 : 31; // High res on/off

                    int display_type = instruction & 0xF;
                    int rows = (high_res && (display_type == 0)) ? 16 : display_type; // DXY0 or DXYN
                    int bytes_per_row = (rows == 16) ? 2 : 1; // Specific needed for super sprite

                    uint8_t X = V[(instruction >> 8) & 0xF] & width; // AND 63 is the same as modulo
                    uint8_t Y = V[(instruction >> 4) & 0xF] & height;
                    V[15] = 0;
                    uint8_t data;
                    int x;
                    int y;
                    uint8_t bit_value;

                    for (int j = 0; j < rows; ++j) {
                        for (int byte = 0; byte < bytes_per_row; ++byte) {
                            data = memory[I + j + byte*8];
                            y = Y + pixels_vertical + j;

                            // Skip rows that are off-screen
                            if (y < 0 || y > height) {
                                continue;
                            }

                            for (int i = 0; i < 8; ++i) {
                                bit_value = (data >> (7 - i)) & 1;
                                x = X + pixels_horizontal + byte*8 + i;

                                // Skip columns that are off-screen
                                if (x < 0 || x > width) {
                                    continue;
                                }
                               
                                if (bit_value && screen_super[x][y]) {
                                    screen_super[x][y] = 0;
                                    V[15] = 1;
                                    display_changed = 1;
                                }
                                else if (bit_value && !screen_super[x][y]) {
                                    screen_super[x][y] = 1;
                                    display_changed = 1;
                                }
                                
                            }

                        }
                    }
                        
                    break;
                            
                }
                
            }   
            break; 
        }

        case 0xE: { // Skip if
            switch (instruction & 0xF) {
                case 0x1: {
                    uint8_t Vx = V[(instruction >> 8) & 0xF];
                    if (!keypad[Vx]) {
                        PC += 0x002;
                    }
                    break;
                }
                    
                case 0xE: {
                    uint8_t Vx = V[(instruction >> 8) & 0xF];
                    if (keypad[Vx]) {
                        PC += 0x002;
                    }
                    break;
                }

                default:
                    break;

            }
            break;
        }

        case 0xF: {
            switch(instruction & 0xF) {
                case 0x0: { // Set I to big hex location
                    uint8_t value = V[(instruction >> 8) & 0xF] & 0xF;
                    switch(value) {
                        case 0x0: {
                            I = 0x0A0;
                            break;
                        }

                        case 0x1: {
                            I = 0x0AA;
                            break;
                        }

                        case 0x2: {
                            I = 0x0B4;
                            break;
                        }

                        case 0x3: {
                            I = 0x0BE;
                            break;
                        }

                        case 0x4: {
                            I = 0x0C8;
                            break;
                        }

                        case 0x5: {
                            I = 0x0D2;
                            break;
                        }

                        case 0x6: {
                            I = 0x0DC;
                            break;
                        }

                        case 0x7: {
                            I = 0x0E6;
                            break;
                        }

                        case 0x8: {
                            I = 0x0F0;
                            break;
                        }

                        case 0x9: {
                            I = 0x0FA;
                            break;
                        }

                        case 0xA: {
                            I = 0x104;
                            break;
                        }

                        case 0xB: {
                            I = 0x10E;
                            break;
                        }

                        case 0xC: {
                            I = 0x118;
                            break;
                        }

                        case 0xD: {
                            I = 0x122;
                            break;
                        }

                        case 0xE: {
                            I = 0x12C;
                            break;
                        }

                        case 0xF: {
                            I = 0x136;
                            break;
                        }

                        default:
                            break;
                    }
                    break;
                }
                    
                case 0x3: { // Binary coded decimal conversion
                    int Vx = V[(instruction >> 8) & 0xF];
                    memory[I] = Vx / 100;
                    memory[I+1] = (Vx % 100) / 10;
                    memory[I+2] = Vx % 10;
                    break;
                }

                case 0x5: {
                    switch ((instruction >> 4) & 0xF) {
                        case 0x1: { // Set delay timer
                            delay_countdown = V[(instruction >> 8) & 0xF];
                            break;
                        }

                        case 0x5: { // Store into memory
                            switch(chip) {
                                case CHIP_8: {
                                    uint8_t x = (instruction >> 8) & 0xF;
                                    for (int i = 0; i < x + 1; ++ i) {
                                        memory[I] = V[i];
                                        ++I;
                                    }
                                    break;
                                }

                                case SUPER_CHIP: {   
                                    uint8_t x = (instruction >> 8) & 0xF;
                                    for (int i = 0; i < x + 1; ++ i) {
                                        memory[I+i] = V[i];
                                    }
                                    break;
                                }
                            }
                            break;
                        }

                        case 0x6: { // Load from memory
                            switch(chip) {
                                case CHIP_8: {
                                    uint8_t x = (instruction >> 8) & 0xF;
                                    for (int i = 0; i < x + 1; ++ i) {
                                        V[i] = memory[I];
                                        ++I;
                                    }
                                    break;
                                }

                                case SUPER_CHIP: {   
                                    uint8_t x = (instruction >> 8) & 0xF;
                                    for (int i = 0; i < x + 1; ++ i) {
                                        V[i] = memory[I+i];
                                    }
                                    break;
                                }
                            }
                            break;
                        }

                        case 0x7: { // Save to flag registers
                            int count = (instruction >> 8) & 0xF;
                            if (count > 15) {
                                count = 15;
                            }
                            for (int i = 0; i <= count; ++i) {
                                flag[i] = V[i];
                            }
                            break;
                        }

                        case 0x8: { // Restore from flag registers
                            int count = (instruction >> 8) & 0xF;
                            if (count > 15) {
                                count = 15;
                            }
                            for (int i = 0; i <= count; ++i) {
                                V[i] = flag[i];
                            }
                            break;
                        }

                        default:
                            break;

                    }
                    break;
                }

                case 0x7: { // Read delay timer
                    V[(instruction >> 8) & 0xF] = delay_countdown;
                    break;
                }

                case 0x8: { // Set sound timer
                    sound_countdown = V[(instruction >> 8) & 0xF];
                    break;
                }

                case 0x9: { // Set I to font location
                    uint8_t value = V[(instruction >> 8) & 0xF] & 0xF;
                    switch(value) {
                        case 0x0: {
                            I = 0x050;
                            break;
                        }

                        case 0x1: {
                            I = 0x055;
                            break;
                        }

                        case 0x2: {
                            I = 0x05A;
                            break;
                        }

                        case 0x3: {
                            I = 0x05F;
                            break;
                        }

                        case 0x4: {
                            I = 0x064;
                            break;
                        }

                        case 0x5: {
                            I = 0x069;
                            break;
                        }

                        case 0x6: {
                            I = 0x06E;
                            break;
                        }

                        case 0x7: {
                            I = 0x073;
                            break;
                        }

                        case 0x8: {
                            I = 0x078;
                            break;
                        }

                        case 0x9: {
                            I = 0x07D;
                            break;
                        }

                        case 0xA: {
                            I = 0x082;
                            break;
                        }

                        case 0xB: {
                            I = 0x087;
                            break;
                        }

                        case 0xC: {
                            I = 0x08C;
                            break;
                        }

                        case 0xD: {
                            I = 0x091;
                            break;
                        }

                        case 0xE: {
                            I = 0x096;
                            break;
                        }

                        case 0xF: {
                            I = 0x09A;
                            break;
                        }

                        default:
                            break;
                    }
                    break;
                }

                case 0xE: { // Add index and set flag
                    uint8_t x = (instruction >> 8) & 0xF;
                    int Vx = V[x];

                    I += V[x];

                    if (Vx+I > 255) {
                        V[15] = 1;
                    }
                    else {
                        V[15] = 0;
                    }

                    break;
                }   

                case 0xA: { // Get key
                    if (!running) {
                        break;
                    }
                    
                    if (key && !keypad[index]) {
                        V[(instruction >> 8) & 0xF] = index;
                        key = false;
                        break;
                    }
                    index = 0;
                    for (int i = 0; i < 16; ++i) {
                        if (keypad[i]) {
                            key = true;
                            break;
                        }
                        index += 1;
                    }

                    PC -= 0x002;
                  
                    break;
                }

                default:
                    break;
            }
        }

        default: {
            break;
        }
    }
    
}

bool Chip8::poll(SDL_Event event) {
    // Reads inputs from 1234 down to ZXCV
    bool running = true;

    while (SDL_PollEvent(&event)) {
        if (!running) {
            break;
        }

        switch(event.type) {
            case SDL_QUIT: {
                running = false;
                break;
            }

            case SDL_KEYDOWN: {
                switch(event.key.keysym.scancode) {
                    case SDL_SCANCODE_1: { // 1
                        keypad[1] = 1;
                        break;
                    }

                    case SDL_SCANCODE_2: { // 2
                        keypad[2] = 1;
                        break;
                    }

                    case SDL_SCANCODE_3: { // 3
                        keypad[3] = 1;
                        break;
                    }

                    case SDL_SCANCODE_4: { // C
                        keypad[12] = 1;
                        break;
                    }

                    case SDL_SCANCODE_Q: { // 4
                        keypad[4] = 1;
                        break;
                    }

                    case SDL_SCANCODE_W: { // 5
                        keypad[5] = 1;
                        break;
                    }

                    case SDL_SCANCODE_E: { // 6
                        keypad[6] = 1;
                        break;
                    }

                    case SDL_SCANCODE_R: { // D
                        keypad[13] = 1;
                        break;
                    }

                    case SDL_SCANCODE_A: { // 7
                        keypad[7] = 1;
                        break;
                    }

                    case SDL_SCANCODE_S: { // 8
                        keypad[8] = 1;
                        break;
                    }

                    case SDL_SCANCODE_D: { // 9
                        keypad[9] = 1;
                        break;
                    }

                    case SDL_SCANCODE_F: { // E
                        keypad[14] = 1;
                        break;
                    }

                    case SDL_SCANCODE_Z: { // A
                        keypad[10] = 1;
                        break;
                    }

                    case SDL_SCANCODE_X: { // 0
                        keypad[0] = 1;
                        break;
                    }

                    case SDL_SCANCODE_C: { // B
                        keypad[11] = 1;
                        break;
                    }

                    case SDL_SCANCODE_V: { // F
                        keypad[15] = 1;
                        break;
                    }

                    default: {
                        break;
                    }
                }
                break;
            }

            case SDL_KEYUP: {
                switch(event.key.keysym.scancode) {
                    case SDL_SCANCODE_1: { // 1
                        keypad[1] = 0;
                        break;
                    }

                    case SDL_SCANCODE_2: { // 2
                        keypad[2] = 0;
                        break;
                    }

                    case SDL_SCANCODE_3: { // 3
                        keypad[3] = 0;
                        break;
                    }

                    case SDL_SCANCODE_4: { // C
                        keypad[12] = 0;
                        break;
                    }

                    case SDL_SCANCODE_Q: { // 4
                        keypad[4] = 0;
                        break;
                    }

                    case SDL_SCANCODE_W: { // 5
                        keypad[5] = 0;
                        break;
                    }

                    case SDL_SCANCODE_E: { // 6
                        keypad[6] = 0;
                        break;
                    }

                    case SDL_SCANCODE_R: { // D
                        keypad[13] = 0;
                        break;
                    }

                    case SDL_SCANCODE_A: { // 7
                        keypad[7] = 0;
                        break;
                    }

                    case SDL_SCANCODE_S: { // 8
                        keypad[8] = 0;
                        break;
                    }

                    case SDL_SCANCODE_D: { // 9
                        keypad[9] = 0;
                        break;
                    }

                    case SDL_SCANCODE_F: { // E
                        keypad[14] = 0;
                        break;
                    }

                    case SDL_SCANCODE_Z: { // A
                        keypad[10] = 0;
                        break;
                    }

                    case SDL_SCANCODE_X: { // 0
                        keypad[0] = 0;
                        break;
                    }

                    case SDL_SCANCODE_C: { // B
                        keypad[11] = 0;
                        break;
                    }

                    case SDL_SCANCODE_V: { // F
                        keypad[15] = 0;
                        break;
                    }

                    default: {
                        break;
                    }
                }
                break;       
            }
        }
            
    }
    return running;
}

void Chip8::display(SDL_Renderer* renderer) {
    // Update output to new display state created by cycle
    // Clear display
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw on pixels
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    if (chip == CHIP_8) {
        SDL_Rect rects[2048];
        int count = 0;

        for (int y = 0; y < 32; ++y) {
            for (int x = 0; x < 64; ++x) {
                if (screen[x][y]) {
                    rects[count++] = {x, y, 1, 1};
                }
            }
        }

        SDL_RenderFillRects(renderer, rects, count); // Fill all at the same time
    }
    

    else { // SUPER_CHIP high resolution screen

        if (high_res) {
            SDL_RenderSetLogicalSize(renderer, 128, 64);
        } 
        else {
            SDL_RenderSetLogicalSize(renderer, 64, 32);
        }

        SDL_Rect rects[8192];
        int count = 0;

        for (int y = 0; y < 64; ++y) {
            for (int x = 0; x < 128; ++x) {
                if (screen_super[x][y]) {
                    rects[count++] = {x, y, 1, 1};
                }
            }
        }
        
        SDL_RenderFillRects(renderer, rects, count);
        
    }
    
    SDL_RenderPresent(renderer);
}

void Chip8::load_game(const std::string& path) {
    // Puts the game into memory beginning at 0x200
    std::ifstream rom(path, std::ios::binary | std::ios::ate); // Direct binary and sent to the end 
    if (!rom) {
        throw std::runtime_error("Failed to open rom");
    }
    std::streamsize size = rom.tellg(); // Getting final read position
    if (size > (4096 - 0x200)) {
        throw std::runtime_error("Size of rom too large");
    }

    rom.seekg(0, std::ios::beg); // Set to beginning

    std::vector<uint8_t> buffer(size);
    if (!rom.read(reinterpret_cast<char*>(buffer.data()), size)) { // Interpreting data
        throw std::runtime_error("Failed to read ROM");
    }

    std::memcpy(&memory[0x200], buffer.data(), buffer.size());

}

uint8_t Chip8::get_delay_countdown() {
    return delay_countdown;
}

void Chip8::decrement_delay_countdown() {
    delay_countdown -= 1;
}

uint8_t Chip8::get_sound_countdown() {
    return sound_countdown;
}

void Chip8::decrement_sound_countdown() {
    sound_countdown -= 1;
}

bool Chip8::get_display_changed() {
    return display_changed;
}

void Chip8::set_display_changed(bool state) {
    display_changed = state;
}

bool Chip8::is_running() {
    return running;
}

void Chip8::reset() {
    // Clear memory and revert to state
    PC = 0x200;
    delay_countdown = 0;
    sound_countdown = 0;
    
    I = 0;
    memset(memory + 0x200, 0, sizeof(memory) - 0x200); // Resets non-reserved memory
    
    memset(screen, 0, sizeof(screen));
    memset(screen_super, 0, sizeof(screen_super)); // SUPER_CHIP high resolution screen
     
    memset(V, 0, sizeof(V));
    memset(flag, 0, sizeof(flag));

    while (!stack.empty()) {
        stack.pop();
    }

    memset(keypad, 0, sizeof(keypad));
    index = 0;
    key = false;

    high_res = false;

    pixels_vertical = 0;
    pixels_horizontal = 0;
    display_changed = 1;
    
}

void Chip8::add_fonts() {
    // 0
    memory[0x050] = 0b11110000;
    memory[0x051] = 0b10010000;
    memory[0x052] = 0b10010000;
    memory[0x053] = 0b10010000;
    memory[0x054] = 0b11110000;

    // 1
    memory[0x055] = 0b00100000;
    memory[0x056] = 0b01100000;
    memory[0x057] = 0b00100000;
    memory[0x058] = 0b00100000;
    memory[0x059] = 0b01110000;

    // 2
    
    memory[0x05A] = 0b11110000;
    memory[0x05B] = 0b00010000;
    memory[0x05C] = 0b11110000;
    memory[0x05D] = 0b10000000;
    memory[0x05E] = 0b11110000;

    // 3
    
    memory[0x05F] = 0b11110000;
    memory[0x060] = 0b00010000;
    memory[0x061] = 0b01110000;
    memory[0x062] = 0b00010000;
    memory[0x063] = 0b11110000;

    // 4
    memory[0x064] = 0b10010000;
    memory[0x065] = 0b10010000;
    memory[0x066] = 0b11110000;
    memory[0x067] = 0b00010000;
    memory[0x068] = 0b00010000;

    // 5
    memory[0x069] = 0b11110000;
    memory[0x06A] = 0b10000000;
    memory[0x06B] = 0b11110000;
    memory[0x06C] = 0b00010000;
    memory[0x06D] = 0b11110000;

    // 6
    memory[0x06E] = 0b11110000;
    memory[0x06F] = 0b10000000;
    memory[0x070] = 0b11110000;
    memory[0x071] = 0b10010000;
    memory[0x072] = 0b11110000;

    // 7
    memory[0x073] = 0b11110000;
    memory[0x074] = 0b00010000;
    memory[0x075] = 0b00100000;
    memory[0x076] = 0b01000000;
    memory[0x077] = 0b01000000;

    // 8
    memory[0x078] = 0b11110000;
    memory[0x079] = 0b10010000;
    memory[0x07A] = 0b11110000;
    memory[0x07B] = 0b10010000;
    memory[0x07C] = 0b11110000;

    // 9
    memory[0x07D] = 0b11110000;
    memory[0x07E] = 0b10010000;
    memory[0x07F] = 0b11110000;
    memory[0x080] = 0b00010000;
    memory[0x081] = 0b11110000;

    // A
    memory[0x082] = 0b11110000;
    memory[0x083] = 0b10010000;
    memory[0x084] = 0b11110000;
    memory[0x085] = 0b10010000;
    memory[0x086] = 0b10010000;

    // B
    memory[0x087] = 0b11100000;
    memory[0x088] = 0b10010000;
    memory[0x089] = 0b11100000;
    memory[0x08A] = 0b10010000;
    memory[0x08B] = 0b11100000;
    

    // C
    memory[0x08C] = 0b11110000;
    memory[0x08D] = 0b10000000;
    memory[0x08E] = 0b10000000;
    memory[0x08F] = 0b10000000;
    memory[0x090] = 0b11110000;
    

    // D
    memory[0x091] = 0b11100000;
    memory[0x092] = 0b10010000;
    memory[0x093] = 0b10010000;
    memory[0x094] = 0b10010000;
    memory[0x095] = 0b11100000;

    // E
    memory[0x096] = 0b11110000;
    memory[0x097] = 0b10000000;
    memory[0x098] = 0b11100000;
    memory[0x099] = 0b10000000;
    memory[0x09A] = 0b11110000;

    // F
    memory[0x09B] = 0b11110000;
    memory[0x09C] = 0b10000000;
    memory[0x09D] = 0b11100000;
    memory[0x09E] = 0b10000000;
    memory[0x09F] = 0b10000000;


    // SUPER_CHIP high resolution sprites

    // 0
    memory[0x0A0] = 0b11111111; 
    memory[0x0A1] = 0b11111111; 
    memory[0x0A2] = 0b11000011; 
    memory[0x0A3] = 0b11000011; 
    memory[0x0A4] = 0b11000011; 
    memory[0x0A5] = 0b11000011; 
    memory[0x0A6] = 0b11000011; 
    memory[0x0A7] = 0b11000011; 
    memory[0x0A8] = 0b11111111; 
    memory[0x0A9] = 0b11111111; 

    // 1
    memory[0x0AA] = 0b00011000; 
    memory[0x0AB] = 0b01111000; 
    memory[0x0AC] = 0b11111000; 
    memory[0x0AD] = 0b00011000; 
    memory[0x0AE] = 0b00011000;  
    memory[0x0AF] = 0b00011000; 
    memory[0x0B0] = 0b00011000;  
    memory[0x0B1] = 0b00011000; 
    memory[0x0B2] = 0b11111111; 
    memory[0x0B3] = 0b11111111; 

    // 2
    memory[0x0B4] = 0b11111111; 
    memory[0x0B5] = 0b11111111; 
    memory[0x0B6] = 0b00000011; 
    memory[0x0B7] = 0b00000011; 
    memory[0x0B8] = 0b11111111;  
    memory[0x0B9] = 0b11111111; 
    memory[0x0BA] = 0b11000000;  
    memory[0x0BB] = 0b11000000; 
    memory[0x0BC] = 0b11111111; 
    memory[0x0BD] = 0b11111111; 

    // 3
    memory[0x0BE] = 0b11111111; 
    memory[0x0BF] = 0b11111111; 
    memory[0x0C0] = 0b00000011; 
    memory[0x0C1] = 0b00000011; 
    memory[0x0C2] = 0b11111111;  
    memory[0x0C3] = 0b11111111; 
    memory[0x0C4] = 0b00000011;  
    memory[0x0C5] = 0b00000011; 
    memory[0x0C6] = 0b11111111; 
    memory[0x0C7] = 0b11111111; 

    // 4
    memory[0x0C8] = 0b11000011; 
    memory[0x0C9] = 0b11000011; 
    memory[0x0CA] = 0b11000011; 
    memory[0x0CB] = 0b11000011; 
    memory[0x0CC] = 0b11111111;  
    memory[0x0CD] = 0b11111111; 
    memory[0x0CE] = 0b00000011;  
    memory[0x0CF] = 0b00000011; 
    memory[0x0D0] = 0b00000011; 
    memory[0x0D1] = 0b00000011; 

    // 5
    memory[0x0D2] = 0b11111111; 
    memory[0x0D3] = 0b11111111; 
    memory[0x0D4] = 0b11000000; 
    memory[0x0D5] = 0b11000000; 
    memory[0x0D6] = 0b11111111;  
    memory[0x0D7] = 0b11111111; 
    memory[0x0D8] = 0b00000011;  
    memory[0x0D9] = 0b00000011; 
    memory[0x0DA] = 0b11111111; 
    memory[0x0DB] = 0b11111111; 

    // 6
    memory[0x0DC] = 0b11111111; 
    memory[0x0DD] = 0b11111111; 
    memory[0x0DE] = 0b11000000; 
    memory[0x0DF] = 0b11000000; 
    memory[0x0E0] = 0b11111111;  
    memory[0x0E1] = 0b11111111; 
    memory[0x0E2] = 0b11000011;  
    memory[0x0E3] = 0b11000011; 
    memory[0x0E4] = 0b11111111; 
    memory[0x0E5] = 0b11111111; 

    // 7
    memory[0x0E6] = 0b11111111; 
    memory[0x0E7] = 0b11111111; 
    memory[0x0E8] = 0b00000011; 
    memory[0x0E9] = 0b00000110; 
    memory[0x0EA] = 0b00001100;  
    memory[0x0EB] = 0b00011000; 
    memory[0x0EC] = 0b00011000;  
    memory[0x0ED] = 0b00011000; 
    memory[0x0EE] = 0b00011000; 
    memory[0x0EF] = 0b00011000; 

    // 8
    memory[0x0F0] = 0b11111111; 
    memory[0x0F1] = 0b11111111; 
    memory[0x0F2] = 0b11000011; 
    memory[0x0F3] = 0b11000011; 
    memory[0x0F4] = 0b11111111;  
    memory[0x0F5] = 0b11111111; 
    memory[0x0F6] = 0b11000011;  
    memory[0x0F7] = 0b11000011; 
    memory[0x0F8] = 0b11111111; 
    memory[0x0F9] = 0b11111111; 

    // 9
    memory[0x0FA] = 0b11111111; 
    memory[0x0FB] = 0b11111111; 
    memory[0x0FC] = 0b11000011; 
    memory[0x0FD] = 0b11000011; 
    memory[0x0FE] = 0b11111111;  
    memory[0x0FF] = 0b11111111; 
    memory[0x100] = 0b00000011;  
    memory[0x101] = 0b00000011; 
    memory[0x102] = 0b00000011; 
    memory[0x103] = 0b00000011; 

    // A
    memory[0x104] = 0b11111111;  
    memory[0x105] = 0b11111111; 
    memory[0x106] = 0b11000011; 
    memory[0x107] = 0b11000011; 
    memory[0x108] = 0b11111111;  
    memory[0x109] = 0b11111111; 
    memory[0x10A] = 0b11000011; 
    memory[0x10B] = 0b11000011; 
    memory[0x10C] = 0b11000011;  
    memory[0x10D] = 0b11000011; 

    // B
    memory[0x10E] = 0b11111100;  
    memory[0x10F] = 0b11111110; 
    memory[0x110] = 0b11000011; 
    memory[0x111] = 0b11000011; 
    memory[0x112] = 0b11111110;  
    memory[0x113] = 0b11111110; 
    memory[0x114] = 0b11000011; 
    memory[0x115] = 0b11000011; 
    memory[0x116] = 0b11111110;  
    memory[0x117] = 0b11111100; 

    // C
    memory[0x118] = 0b11111111;  
    memory[0x119] = 0b11111111; 
    memory[0x11A] = 0b11000000; 
    memory[0x11B] = 0b11000000; 
    memory[0x11C] = 0b11000000;  
    memory[0x11D] = 0b11000000; 
    memory[0x11E] = 0b11000000; 
    memory[0x11F] = 0b11000000; 
    memory[0x120] = 0b11111111;  
    memory[0x121] = 0b11111111; 

    // D
    memory[0x122] = 0b11111100;  
    memory[0x123] = 0b11111110; 
    memory[0x124] = 0b11000111; 
    memory[0x125] = 0b11000011; 
    memory[0x126] = 0b11000001;  
    memory[0x127] = 0b11000001; 
    memory[0x128] = 0b11000011; 
    memory[0x129] = 0b11000111; 
    memory[0x12A] = 0b11111110;  
    memory[0x12B] = 0b11111100; 

    // E
    memory[0x12C] = 0b11111111;  
    memory[0x12D] = 0b11111111; 
    memory[0x12E] = 0b11000000; 
    memory[0x12F] = 0b11000000; 
    memory[0x130] = 0b11111111;  
    memory[0x131] = 0b11111111; 
    memory[0x132] = 0b11000000; 
    memory[0x133] = 0b11000000; 
    memory[0x134] = 0b11111111;  
    memory[0x135] = 0b11111111; 

    // F
    memory[0x136] = 0b11111111;  
    memory[0x137] = 0b11111111; 
    memory[0x138] = 0b11000000; 
    memory[0x139] = 0b11000000; 
    memory[0x13A] = 0b11110000;  
    memory[0x13B] = 0b11110000; 
    memory[0x13C] = 0b11000000; 
    memory[0x13D] = 0b11000000; 
    memory[0x13E] = 0b11000000;  
    memory[0x13F] = 0b11000000; 
    
}


