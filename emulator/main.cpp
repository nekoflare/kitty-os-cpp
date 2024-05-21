#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include "utils.hpp"
#include <filesystem>
#include <fstream>
#include <format>
#include <SDL.h>
#include <thread>
#include "memory/memory.hpp"
#include "utils/cmd_line_parser.hpp"
#include "emulator/emulator.hpp"
#include "instructions/base/branching/branching.hpp"

constexpr size_t usable_mem_blk_count = 12;
constexpr size_t usable_mem_blk_size = MB(4);
uint8_t* fw_bin = nullptr;
std::string fw_filename;
size_t fw_mem_blk_count = 0;
size_t fw_mem_blk_size = KB(4);

constexpr size_t fb_width = 800;
constexpr size_t fb_height = 600;
constexpr size_t fb_bpp = 32; // Bits per pixel
constexpr uint64_t fb_addr = 0x80000000;
constexpr size_t fb_size = fb_width * fb_height * (fb_bpp / 8);
uint8_t* fb_buffer = nullptr;

void renderFramebuffer(SDL_Window* window) {
    // Create an SDL surface from the framebuffer data
    SDL_Surface* framebufferSurface = SDL_CreateRGBSurfaceFrom(
            fb_buffer, fb_width, fb_height, fb_bpp,
            fb_width * (fb_bpp / 8),
            0x0000FF, 0x00FF00, 0xFF0000, 0
    );

    if (!framebufferSurface) {
        std::cerr << "Failed to create surface from framebuffer: " << SDL_GetError() << std::endl;
        return;
    }

    // Get the window's surface
    SDL_Surface* windowSurface = SDL_GetWindowSurface(window);
    if (!windowSurface) {
        std::cerr << "Failed to get window surface: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(framebufferSurface);
        return;
    }

    // Blit the framebuffer surface onto the window's surface
    if (SDL_BlitSurface(framebufferSurface, nullptr, windowSurface, nullptr) != 0) {
        std::cerr << "Failed to blit surface: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(framebufferSurface);
        return;
    }

    // Update the window's surface
    SDL_UpdateWindowSurface(window);

    // Free the framebuffer surface
    SDL_FreeSurface(framebufferSurface);
}

void sdl_event_loop(SDL_Window* window) {
    bool quit = false;
    while (!quit) {
        SDL_Event e;
        SDL_WaitEvent(&e);

        if (e.type == SDL_QUIT) {
            quit = true;
        }

        // Render framebuffer on window
        renderFramebuffer(window);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
    CmdLineParser parser(argc, argv);
    Memory mem0;

    // sdl stuff
    if (SDL_Init(SDL_INIT_VIDEO) <  0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, fb_width, fb_height, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return 1;
    }

    // init framebuffer
    fb_buffer = new uint8_t[fb_size];

    fw_filename = parser.get<std::string>("/FW");
    if (fw_filename.empty())
    {
        std::cout << "INFO: Using default firmware." << std::endl;
        fw_filename = "fw.bin";
    }

    if (!std::filesystem::exists(fw_filename))
    {
        std::cerr << "Couldn't find firmware file!" << std::endl;
        return 1;
    }

    std::ifstream fw_fd(fw_filename, std::ios::binary);
    if (!fw_fd.is_open()) {
        std::cerr << "Failed to open firmware file." << std::endl;
        return 1;
    }

    ssize_t fw_size = static_cast<ssize_t>(std::filesystem::file_size(fw_filename));
    if (fw_size % 4096 != 0) {
        std::cerr << "Invalid size of firmware." << std::endl;
        fw_fd.close();
        return 1;
    }

    fw_bin = new uint8_t[fw_size];
    fw_fd.read(reinterpret_cast<char *>(fw_bin), fw_size);
    if (fw_fd.fail()) {
        std::cerr << "Failed to read firmware file." << std::endl;
        fw_fd.close();
        return 1;
    }

    fw_fd.close();

    // Create 16 entries of 16MB of memory.
    std::cout << "Creating 16 chunks of 16MB of memory resulting in " << 16 * 16 << " MB of memory." << std::endl;

    for (auto i = 0; usable_mem_blk_count > i; i++) {
        uint8_t *chunk;
        chunk = new uint8_t[usable_mem_blk_count];
        mem0.MapMemory(chunk, usable_mem_blk_size, mem0.last_address, MemoryType::MEM_USABLE);
    }

    // map firmware
    auto mm = mem0.MapMemory(fw_bin, fw_size, mem0.last_address, MemoryType::MEM_FW);
    //Processor processor(static_cast<uint64_t>(mm->gLogicalAddress + fw_size - 8), mem0);

    // map framebuffer.
    mem0.MapMemory(fb_buffer, fb_size, fb_addr, MemoryType::MEM_RESERVED, false);

    for (auto* e : mem0) {
        std::cout << "Memory block at logical address: " << std::hex << e->gLogicalAddress
                  << ", length: " << std::dec << e->gLength
                  << ", type: " << memory_type_strings[e->gType] << std::endl;
    }

    //std::thread cpuThread([&]() {
    //    processor.Execute();
    //});
    //cpuThread.detach();

    Emulator emulator;
    emulator.AddProcessor(mm->gLogicalAddress + fw_size - 8, true); // Boot Processor (BP)

    for (size_t i = 15; i > 0; i--) {
        emulator.AddProcessor(0, false); // They're pointing to null. Should crash when not initialized correctly
    }

    BranchingInstructions bi;
    emulator.AddInstructionSet(bi);

    emulator.EmulationLoop();

    sdl_event_loop(window);

    return 0;
}
