//
// Created by Piotr on 12.06.2024.
//

#ifndef KITTY_OS_CPP_PS2_KBD_HPP
#define KITTY_OS_CPP_PS2_KBD_HPP

#include <kernel/hal/x64/idt/idt.hpp>
#include <cstdint>

enum special_character
{
    SC_NONE,
    SC_LCTRL,
    SC_RCTRL,
    SC_ESC,
    SC_LSHIFT,
    SC_RSHIFT,
    SC_LALT,
    SC_RALT,
    SC_CAPSLOCK,
    SC_NUMLOCK,
    SC_SCROLLLOCK,
    SC_UP,
    SC_DOWN,
    SC_LEFT,
    SC_RIGHT,
    SC_ENTER,
    SC_BACKSPACE,
    SC_TAB,
    SC_SPACE,
    SC_HOME,
    SC_PAGEUP,
    SC_END,
    SC_LSHIFT_DOWN,
    SC_ESC_DOWN,
    SC_ENTER_DOWN,
};

struct scan_code_entry_t
{
    uint64_t scan_code;
    const unsigned char value;
    bool is_press;
    const char* decor_name;
    special_character special_chr;
};

void ps2_parse_key(uint8_t* buffer, size_t scan_code_length, bool was_press);
uint64_t combine_scan_code(uint8_t* scan_code, int length);
void ps2_kbd_irq([[gnu::unused]] Registers_x86_64* regs);
void clear_scan_code_buffer();
void ps2_kbd_init();
void kbd_read(char* dest, bool pipe_to_terminal = true);

#endif //KITTY_OS_CPP_PS2_KBD_HPP
