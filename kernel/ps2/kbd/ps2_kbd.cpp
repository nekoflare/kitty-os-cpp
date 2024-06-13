//
// Created by Piotr on 12.06.2024.
//

#include "ps2_kbd.hpp"
#include <kernel/irqs/uniirq.hpp>

volatile const scan_code_entry_t scan_codes[] = {
        {0x001d,   0x00, true,  "L-Ctrl", SC_LCTRL},
        {0xe01d,   0x00, true,  "R-Ctrl", SC_RCTRL},
        {0x0001,   0x00, true,  "Esc",    SC_ESC},
        {0x0002,   '1',  true,  "1",      SC_NONE},
        {0x0003,   '2',  true,  "2",      SC_NONE},
        {0x0004,   '3',  true,  "3",      SC_NONE},
        {0x0005,   '4',  true,  "4",      SC_NONE},
        {0x0006,   '5',  true,  "5",      SC_NONE},
        {0x0007,   '6',  true,  "6",      SC_NONE},
        {0x0008,   '7',  true,  "7",      SC_NONE},
        {0x0009,   '8',  true,  "8",      SC_NONE},
        {0x000A,   '9',  true,  "9",      SC_NONE},
        {0x000B,   '0',  true,  "0",      SC_NONE},
        {0x000C,   '-',  true,  "-",      SC_NONE},
        {0x000D,   '=',  true,  "=",      SC_NONE},
        {0x000E,   0x00, true,  "Backspace", SC_BACKSPACE},
        {0x000F,   0x00, true,  "Tab",    SC_TAB},
        {0x0010,   'q',  true,  "Q",      SC_NONE},
        {0x0011,   'w',  true,  "W",      SC_NONE},
        {0x0012,   'e',  true,  "E",      SC_NONE},
        {0x0013,   'r',  true,  "R",      SC_NONE},
        {0x0014,   't',  true,  "T",      SC_NONE},
        {0x0015,   'y',  true,  "Y",      SC_NONE},
        {0x0016,   'u',  true,  "U",      SC_NONE},
        {0x0017,   'i',  true,  "I",      SC_NONE},
        {0x0018,   'o',  true,  "O",      SC_NONE},
        {0x0019,   'p',  true,  "P",      SC_NONE},
        {0x001A,   '[',  true,  "[",      SC_NONE},
        {0x001B,   ']',  true,  "]",      SC_NONE},
        {0x001C,   0x00, true,  "Enter",  SC_ENTER},
        {0x001E,   'a',  true,  "A",      SC_NONE},
        {0x001F,   's',  true,  "S",      SC_NONE},
        {0x0020,   'd',  true,  "D",      SC_NONE},
        {0x0021,   'f',  true,  "F",      SC_NONE},
        {0x0022,   'g',  true,  "G",      SC_NONE},
        {0x0023,   'h',  true,  "H",      SC_NONE},
        {0x0024,   'j',  true,  "J",      SC_NONE},
        {0x0025,   'k',  true,  "K",      SC_NONE},
        {0x0026,   'l',  true,  "L",      SC_NONE},
        {0x0027,   ';',  true,  ";",      SC_NONE},
        {0x0028,   '\'', true,  "'",      SC_NONE},
        {0x0029,   '`',  true,  "`",      SC_NONE},
        {0x002A,   0x00, true,  "L-Shift", SC_LSHIFT},
        {0x002B,   '\\', true,  "\\",     SC_NONE},
        {0x002C,   'z',  true,  "Z",      SC_NONE},
        {0x002D,   'x',  true,  "X",      SC_NONE},
        {0x002E,   'c',  true,  "C",      SC_NONE},
        {0x002F,   'v',  true,  "V",      SC_NONE},
        {0x0030,   'b',  true,  "B",      SC_NONE},
        {0x0031,   'n',  true,  "N",      SC_NONE},
        {0x0032,   'm',  true,  "M",      SC_NONE},
        {0x0033,   ',',  true,  ",",      SC_NONE},
        {0x0034,   '.',  true,  ".",      SC_NONE},
        {0x0035,   '/',  true,  "/",      SC_NONE},
        {0x0036,   0x00, true,  "R-Shift", SC_RSHIFT},
        {0x0037,   '*',  true,  "*",      SC_NONE},
        {0x0038,   0x00, true,  "L-Alt",  SC_LALT},
        {0x0039,   ' ',  true,  "Space",  SC_NONE},
        {0x003A,   0x00, true,  "Caps Lock", SC_CAPSLOCK},
        {0xE01C,   0x00, true,  "Enter",  SC_ENTER},
        {0xE035,   0x00, true,  "/",      SC_NONE},
        {0xE037,   0x00, true,  "Print Screen", SC_NONE},
        {0xE038,   0x00, true,  "R-Alt",  SC_RALT},
        {0xE047,   0x00, true,  "Home",   SC_HOME},
        {0xE048,   0x00, true,  "Up",     SC_UP},
        {0xE049,   0x00, true,  "Page Up", SC_PAGEUP},
        {0xE04B,   0x00, true,  "Left",   SC_LEFT},
        {0xE04D,   0x00, true,  "Right",  SC_RIGHT},
        {0xE04F,   0x00, true,  "End",    SC_END},
        {0xE050,   0x00, true,  "Down",   SC_DOWN},
        {0x00aa,   0x00, false, "L-Shift (Down)", SC_LSHIFT_DOWN},
        {0x009c,   0x00, false, "Enter (Down)", SC_ENTER_DOWN}
};

volatile bool capslock_pressed = false;
volatile bool left_control_pressed = false;
volatile bool right_control_pressed = false;
volatile bool left_shift_pressed = false;
volatile bool right_right_pressed = false;
volatile bool alt_pressed = false;
volatile bool enter_key_pressed = false;
volatile unsigned char last_key = 0;

void kbd_read(char* dest, bool pipe_to_terminal)
{
    size_t idx = 0;
    while (true)
    {
        if (enter_key_pressed == true)
        {
            enter_key_pressed = false;
            return;
        }

        if (last_key == 0)
        {
            continue;
        }

        auto k = last_key;
        if (pipe_to_terminal) kstd::printf("%c", k);
        last_key = 0;
        dest[idx] = k;
        idx++;
    }
}

void ps2_parse_key(uint8_t* buffer, size_t scan_code_length, bool was_press)
{
    auto scan_code = combine_scan_code(buffer, scan_code_length);
    const volatile scan_code_entry_t* this_entry = nullptr;

    for (size_t i = 0; i < sizeof(scan_codes) / sizeof(scan_code_entry_t); ++i)
    {
        if (scan_codes[i].scan_code == scan_code)
        {
            this_entry = &scan_codes[i];
            break; // Exit loop once a match is found
        }
        this_entry = nullptr;
    }

    if (this_entry == nullptr)
    {
        return;
    }

    if (this_entry->special_chr == SC_LSHIFT)
    {
        left_shift_pressed = true;
        return;
    }

    if (this_entry->special_chr == SC_LSHIFT_DOWN)
    {
        left_shift_pressed = false;
        return;
    }

    if (this_entry->special_chr == SC_ENTER)
    {
        enter_key_pressed = true;
        return;
    }

    if (this_entry->special_chr == SC_ENTER_DOWN)
    {
        enter_key_pressed = false;
        return;
    }

    if (this_entry->value != 0)
    {
        if (left_shift_pressed)
        {
            auto code = kstd::toupper(this_entry->value);
            // kstd::printf("%c", code);
            last_key = code;
        }
        else
        {
            // kstd::printf("%c", this_entry->value);
            last_key = this_entry->value;
        }
    }
}

static bool special_codes = false;
static uint8_t scan_code_buffer[6];
static int scan_code_length = 0;

uint64_t combine_scan_code(uint8_t* scan_code, int length)
{
    uint64_t combined_code = 0;
    for (int i = 0; i < length; ++i)
    {
        combined_code <<= 8;  // Shift left by 8 bits to make room for the next byte
        combined_code |= scan_code[i];  // Combine the next byte
    }
    return combined_code;
}

void clear_scan_code_buffer()
{
    scan_code_length = 0;
    special_codes = false;
    for (size_t i = 0; i < sizeof(scan_code_buffer); ++i)
    {
        scan_code_buffer[i] = 0;
    }
}

void ps2_kbd_irq([[gnu::unused]] Registers_x86_64* regs)
{
    // Read the scan code from the keyboard data port (0x60)
    auto scan_code = inb(0x60);
    bool was_press = (scan_code & 0x80) ? false : true;

    // Handle special code prefix (0xE0 or 0xE1)
    if (scan_code == 0xE0 || scan_code == 0xE1)
    {
        special_codes = true;
        scan_code_buffer[0] = scan_code;
        scan_code_length = 1;
        return;
    }

    // If in the middle of a special code sequence, add the byte to the buffer
    if (special_codes)
    {
        scan_code_buffer[scan_code_length++] = scan_code;
        if ((scan_code_buffer[0] == 0xE0 && scan_code_length == 2) ||  // Handle 0xE0 xx
            (scan_code_buffer[0] == 0xE1 && scan_code_length == 6))    // Handle 0xE1 xx xx xx xx
        {
            ps2_parse_key(scan_code_buffer, scan_code_length, was_press);
            clear_scan_code_buffer();
        }
        return;
    }

    // Handle single-byte scan codes
    scan_code_buffer[0] = scan_code;
    scan_code_length = 1;
    ps2_parse_key(scan_code_buffer, scan_code_length, was_press);
    clear_scan_code_buffer();
}


void ps2_kbd_init()
{
    uirq_register_irq(1, &ps2_kbd_irq);
    uirq_unmask_irq(1);
}