//
// Created by Piotr on 13.06.2024.
//

#ifndef KITTY_OS_CPP_MOUSE_HPP
#define KITTY_OS_CPP_MOUSE_HPP

#include <cstdint>

enum special_character : int
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
    SC_CAPSLOCK_DOWN,
    SC_RSHIFT_DOWN
};

constexpr char invalid_character = 0;

struct KeyboardKeyState
{
    uint64_t scan_code;         // This is raw value of scan code.
    char character;             // This is character of the scan code as ASCII.
    bool is_pressed;            // Is the key being pressed? true if yes.
    special_character special_character_code;   // Special character code of character == invalid_character
};

enum KeyboardIoctls : uint64_t
{
    GetKeyState,    // This is blocking operation. It will wait for the key press/release and return it.
    GetLastKeyState, // Non-blocking operation, get last key state.
};

#endif //KITTY_OS_CPP_MOUSE_HPP
