    //
// Created by Piotr on 06.06.2024.
//

#include "debug_print.hpp"

#include <kstd/kstring.hpp>

void dbg_init()
{
    // Use COM1 as debug output
    com_disable_interrupts(COM1);
    com_set_baud_rate(COM1, 115200);
    com_set_data_bits(COM1, data_length_8);
    com_set_parity_none(COM1);
    com_set_stop_bits(COM1);
    com_set_fifo(COM1);
    com_flush_fifo(COM1);
    com_enable_rts(COM1);
    com_enable_dtr(COM1);
}

void dbg_write_str(const char* msg)
{
    for (size_t i = 0; kstd::strlen(msg) > i; i++)
    {
        com_write_byte(COM1, msg[i]);
    }
}

void dbg_write_chr(const char msg)
{
    com_write_byte(COM1, msg);
}