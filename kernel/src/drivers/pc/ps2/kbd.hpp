//
// Created by Piotr on 13.06.2024.
//

#ifndef KITTY_OS_CPP_KBD_HPP
#define KITTY_OS_CPP_KBD_HPP

#include <kdu/apis/keyboard.hpp>
#include <kdu/driver_entry.hpp>
#include <kernel/hal/x64/idt/idt.hpp>

struct scan_code_entry_t
{
    uint64_t scan_code;
    const unsigned char value;
    bool is_press;
    const char* decor_name;
    special_character special_chr;
};

static driver_handle_t ps2kbd_entry(pci_dev* dev);
static driver_status_t ps2kbd_cleanup();
static driver_status_t ps2kbd_ioctl(
        driver_handle_t* driver_handle,
        uint64_t ioctl,
        const char* request_data,
        char* request_answer
        );

static void ps2kbd_irq_handler(Registers_x86_64* regs);
static void ps2kbd_parse_key(char* sc_buf, size_t sc_len, bool sc_is_press);

// Ioctls
static void ps2kbd_get_key_state(char* request_answer);

extern volatile driver_type driver_entry_t ps2kbd_desc;

#endif //KITTY_OS_CPP_KBD_HPP
