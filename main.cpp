#include "chip8.h"
#include <cstdio>

int main() {
    uint32_t time_accumulated = 0;
    uint32_t cpu_time_accumulated = 0;

    uint32_t delta;

    uint32_t last_time = 0;
    uint32_t cpu_last_time = 0;

    uint32_t now;
    const double tick = 1000.0 / 60.0; // in ms

    SDL_Event event;
    bool SDL_running = true;

    // Get chip type and ROM
    int chip;
    std::cout << "Enter 1 for CHIP8 or 2 for SUPER_CHIP: ";
    std::cin >> chip;

    const double cpu_tick = (chip == 1) ? 1000.0 / 600.0 : 1000.0 / 6000.0;

    std::string path;
    std::cout << "Enter the path of the ROM: ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore previous cin newline
    std::getline(std::cin, path);

    // Display stuff
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        fprintf(stderr, "Could not initialise SDL: %s\n", SDL_GetError());
        return -1;
    }
    if (chip == 1) {
        if (SDL_CreateWindowAndRenderer(64*SCALE, 32*SCALE, 0, &window, &renderer) != 0) {
            fprintf(stderr, "Could not initialise create window and renderer: %s\n", SDL_GetError());
            return -1;
        }
    }
    else {
        if (SDL_CreateWindowAndRenderer(128*SCALE, 64*SCALE, 0, &window, &renderer) != 0) {
            fprintf(stderr, "Could not initialise create window and renderer: %s\n", SDL_GetError());
            return -1;
        }
    }
    
    SDL_RenderSetScale(renderer, SCALE, SCALE);

    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // RGBA
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    // Audio
    bool sound_on = false;

    SDL_AudioSpec spec;
    spec.freq = 44100;
    spec.format = AUDIO_S16SYS;
    spec.channels = 1;
    spec.samples = 512; // Can determine latency
    spec.callback = NULL;
    SDL_AudioDeviceID device_id = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);

    Sint16 samples[735]; // Lasts one cycle of decrementing sound countdown
    samples[734] = 20000;
    for (int i = 1; i < 735; i = i + 2) { // Needs to be square wave
        samples[i] = -5000;
        samples[i-1] = 5000;
    }

    Chip8 emulator{chip}; 
    emulator.load_game(path);
    
    while (emulator.is_running() && SDL_running) { // Make sure SDL and emulator are both on
        // --- Get inputs ---
        SDL_running = emulator.poll(event);

        // --- Audio ---
        // Turn on audio
        if (!sound_on && emulator.get_sound_countdown() > 0) {
            SDL_ClearQueuedAudio(device_id); 
            sound_on = true;
            SDL_QueueAudio(device_id, samples, sizeof(samples));
            SDL_PauseAudioDevice(device_id, 0);
        }
        // Turn off audio
        else if (sound_on && emulator.get_sound_countdown() == 0) {
            SDL_ClearQueuedAudio(device_id);
            sound_on = false;
            SDL_PauseAudioDevice(device_id, 1);
        }    

        // --- CPU Cycle ---
        now = SDL_GetTicks(); 
        delta = now - cpu_last_time;
        cpu_time_accumulated += delta;
        cpu_last_time = now;

        while (cpu_time_accumulated >= cpu_tick) {
            emulator.cycle(); 
            cpu_time_accumulated -= cpu_tick;
        }

        // Get timing right
        now = SDL_GetTicks(); 
        delta = now - last_time;
        time_accumulated += delta;
        last_time = now;

        while (time_accumulated >= tick) { // Counts down at 60 Hz
            // --- Timers ---
            if (emulator.get_delay_countdown() > 0)
                emulator.decrement_delay_countdown();
            if (emulator.get_sound_countdown() > 0) {
                emulator.decrement_sound_countdown();             
            }
            
            // --- Display ---
            if (emulator.get_display_changed()) { // Only presents when necessary
                emulator.display(renderer);
                emulator.set_display_changed(false);
            }
                     
            time_accumulated -= tick;
        }
        
    }
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}