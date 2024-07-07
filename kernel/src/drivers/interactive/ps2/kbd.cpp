//
// Created by Piotr on 13.06.2024.
//

#include "kbd.hpp"

#include <public/kdu/driver_entry.hpp>
#include <public/kdu/apis/keyboard.hpp>
#include <hal/x64/irqs/uniirq.hpp>
#include <kstd/kstring.hpp>
#include <kstd/kmutex.hpp>

static const scan_code_entry_t scan_codes[] = {
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
        {0x009c,   0x00, false, "Enter (Down)", SC_ENTER_DOWN},
        {0x00ba,   0x00, false, "Capslock (Down)", SC_CAPSLOCK_DOWN},
        {0x00b6,   0x00, false, "Right shift (Down)", SC_RSHIFT_DOWN}
};

static unsigned char scan_code_buffer[6] = {0};
static size_t scan_code_length = 0;
volatile static bool special_codes = false;

// States of keyboard.
volatile static bool left_shift_pressed = false;
volatile static bool right_shift_pressed = false;
volatile static bool enter_pressed = false;
static const scan_code_entry_t* last_key = nullptr;

kstd::mutex ps2kbd_key_mtx;

static void ps2kbd_irq_handler([[maybe_unused]] Registers_x86_64* regs)
{
    auto scan_code = inb(0x60);
    bool was_press = (scan_code & 0x80) ? false : true;

    if (scan_code == 0xe0 || scan_code == 0xe1)
    {
        special_codes = true;
        scan_code_buffer[0] = scan_code;
        scan_code_length = 1;
        return;
    }

    if (special_codes)
    {
        scan_code_buffer[scan_code_length++] = scan_code;
        if ((scan_code_buffer[0] == 0xE0 && scan_code_length == 2) ||  // Handle 0xE0 xx
            (scan_code_buffer[0] == 0xE1 && scan_code_length == 6))    // Handle 0xE1 xx xx xx xx
        {
            auto code = ps2kbd_combine_scan_code(scan_code_buffer, scan_code_length);
            kstd::printf("%lx\n", code);
            ps2kbd_parse_key(scan_code_buffer, scan_code_length, was_press);
            special_codes = false;  // Reset special codes flag
            scan_code_length = 0;   // Reset scan code length
        }
        return;
    }

    scan_code_buffer[0] = scan_code;
    scan_code_length = 1;
    auto code = ps2kbd_combine_scan_code(scan_code_buffer, scan_code_length);
    kstd::printf("%lx\n", code);
    ps2kbd_parse_key(scan_code_buffer, scan_code_length, was_press);
}


static uint64_t ps2kbd_combine_scan_code(unsigned char* sc_buf, size_t sc_len)
{
    uint64_t combined_code = 0;

    for (size_t i = 0; i < sc_len; ++i)
    {
        combined_code <<= 8;
        combined_code |= sc_buf[i];
    }

    if (sc_len == 1)
    {
        combined_code &= 0xff;
    }

    if (sc_len == 2)
    {
        combined_code &= 0xffff;
    }

    if (sc_len == 3)
    {
        combined_code &= 0xffffff;
    }

    if (sc_len == 4)
    {
        combined_code &= 0xffffffff;
    }

    if (sc_len == 5)
    {
        combined_code &= 0xffffffffff;
    }

    return combined_code;
}

static void ps2kbd_parse_key(unsigned char* sc_buf, size_t sc_len, [[maybe_unused]] bool sc_is_press)
{
    auto scan_code = ps2kbd_combine_scan_code(sc_buf, sc_len);
    const scan_code_entry_t* sc_entry = nullptr;

    for (size_t i = 0; i < sizeof(scan_codes) / sizeof(scan_code_entry_t); i++)
    {
        if (scan_codes[i].scan_code == scan_code)
        {
            sc_entry = &scan_codes[i];
            break;
        }
    }

    if (sc_entry == nullptr) return;

    last_key = sc_entry;
    if (sc_is_press) {
        ps2kbd_key_mtx.unlock();
    }
}


static void ps2kbd_get_key_state(char* request_answer)
{
    ps2kbd_key_mtx.lock();
    auto k = last_key;
    auto resp = reinterpret_cast<KeyboardKeyState*>(request_answer);
    resp->scan_code = k->scan_code;
    resp->character = k->value;
    resp->is_pressed = k->is_press;
    resp->special_character_code = k->special_chr;
}

static driver_handle_t ps2kbd_entry([[maybe_unused]] pci_dev* dev)
{
    // Todo: check if there's a keyboard connected.

    uirq_register_irq(1, &ps2kbd_irq_handler);
    uirq_unmask_irq(1);

    ps2kbd_key_mtx.lock();

    ps2kbd_desc.is_loaded = true;
    return {};
}

static driver_status_t ps2kbd_cleanup()
{
    ps2kbd_desc.is_loaded = false;
    return DS_SUCCESS;
}

static driver_status_t ps2kbd_ioctl(
        [[maybe_unused]] driver_handle_t* driver_handle,
        uint64_t ioctl,
        [[maybe_unused]] const char* request_data,
        char* request_answer
)
{
    switch (ioctl)
    {
        case GetKeyState:
            ps2kbd_get_key_state(request_answer);
            return DS_SUCCESS;
    }
    return DS_FAILURE; // No ioctl found.
}

volatile driver_type driver_entry_t ps2kbd_desc = {
        .driver_name = "PS/2 keyboard",
        .driver_author = "Piotr",
        .driver_description = "Keyboard driver",
        .driver_version = "1.0",
        .driver_designation = DT_KEYBOARD,
        .driver_load = DL_ALWAYS_LOAD,
        .driver_entry = &ps2kbd_entry,
        .driver_cleanup = &ps2kbd_cleanup,
        .driver_ioctl = &ps2kbd_ioctl,
        .requirements = nullptr,
        .requirements_count = 0,
        .is_loaded = false
};