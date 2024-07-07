//
// Created by Piotr on 13.06.2024.
//

#include "kbd.hpp"

#include <public/kdu/apis/keyboard.hpp>
#include <public/kdu/driver_ctrl.hpp>

struct conv_table_entry
{
    char character;
    char output_character;
};

conv_table_entry conv_table[] = {
        {'1', '!'},
        {'2', '@'},
        {'3', '#'},
        {'4', '$'},
        {'5', '%'},
        {'6', '^'},
        {'7', '&'},
        {'8', '*'},
        {'9', '('},
        {'0', ')'},
        {'-', '_'},
        {'=', '+'},
        {'[', '{'},
        {']', '}'},
        {';', ':'},
        {'\'', '"'},
        {'\\', '|'},
        {',', '<'},
        {'.', '>'},
        {'/', '?'},
        {'`', '~'}
};

char kbd_convert(char input)
{
    for (auto& i : conv_table)
    {
        if (i.character == input)
        {
            return i.output_character;
        }
    }

    return input;
}

bool kbd_is_in_table(char c)
{
    for (auto& i : conv_table)
    {
        if (i.character == c)
        {
            return true;
        }
    }

    return false;
}

KeyboardKeyState kbd_get_kks()
{
    static KeyboardKeyState kks {};
    ioctl_auto(DT_KEYBOARD, nullptr, GetKeyState, nullptr, reinterpret_cast<char*>(&kks));
    return kks;
}

bool capslock = false;
bool shift = false;

void kbd_read(char* dest, bool write_back)
{
    size_t idx = 0;
    while (true)
    {
        auto kks = kbd_get_kks();

        if (kks.character == 0)
        {
            if (kks.special_character_code == SC_ENTER)
            {
                return;
            }

            if (kks.special_character_code == SC_BACKSPACE)
            {
                if (idx > 0)
                {
                    kstd::move_cursor_x(-1);
                    kstd::putc(' ');
                    kstd::move_cursor_x(-1);

                    dest[idx - 1] = 0;
                    idx --;
                }
                continue;
            }

            if (kks.special_character_code == SC_CAPSLOCK)
            {
                capslock = true;
                continue;
            }

            if (kks.special_character_code == SC_CAPSLOCK_DOWN)
            {
                capslock = false;
                continue;
            }

            if (kks.special_character_code == SC_LSHIFT)
            {
                shift = true;
                continue;
            }

            if (kks.special_character_code == SC_RSHIFT)
            {
                shift = true;
                continue;
            }

            if (kks.special_character_code == SC_LSHIFT_DOWN)
            {
                shift = false;
                continue;
            }

            if (kks.special_character_code == SC_RSHIFT_DOWN)
            {
                shift = false;
                continue;
            }

            // parse shifts
            continue;
        }

        auto c = kks.character;

        if (capslock && !shift) {
            c = kstd::toupper(c);
        } else if ((!capslock && shift) || (capslock && shift)) {
            if (kbd_is_in_table(c)) {
                c = kbd_convert(c);
            } else {
                if (capslock && shift && kstd::isalpha(c)) {
                    c = kstd::tolower(c);
                } else {
                    c = kstd::toupper(c);
                }
            }
        }

        if (write_back) kstd::putc(c);

        dest[idx] = c;
        idx++;
    }
}