//
// Created by Piotr on 05.06.2024.
//

#ifndef KITTY_OS_CPP_UART_HPP
#define KITTY_OS_CPP_UART_HPP

#include <hal/x64/io.hpp>
#include <kernel_settings.hpp>

enum com_ports
{
    COM1 = 0x3F8,
    COM2 = 0x2F8,
    COM3 = 0x3E8,
    COM4 = 0x2E8,
    COM5 = 0x5F8,
    COM6 = 0x4F8,
    COM7 = 0x5E8,
    COM8 = 0x4E8
};

enum data_bits
{
    data_length_5 = 0b00,
    data_length_6 = 0b01,
    data_length_7 = 0b10,
    data_length_8 = 0b11
};

inline void com_set_dlab(const uint16_t com_port)
{
    uint8_t v = inb(com_port + 3);
    v |= (1 << 7);
    outb(com_port + 3, v);
}

inline void com_clear_dlab(const uint16_t com_port)
{
    uint8_t v = inb(com_port + 3);
    v &= ~(1 << 7);
    outb(com_port + 3, v);
}

inline void com_write_byte(const uint16_t com_port, char value)
{
    if constexpr (dbg_output_data)
    {
        com_clear_dlab(com_port);
        outb(com_port, value);
    }
}

inline char com_read_byte(const uint16_t com_port)
{
    if constexpr (dbg_input_data)
    {
        com_clear_dlab(com_port);
        return inb(com_port);
    }
    else
    {
        return 0;
    }
}

inline void com_disable_interrupts(const uint16_t com_port)
{
    com_clear_dlab(com_port);
    outb(com_port + 1, 0);
}

inline void com_enable_interrupts(const uint16_t com_port)
{
    com_clear_dlab(com_port);
    outb(com_port + 1, 1);
}

inline void com_set_baud_rate(const uint16_t com_port, unsigned int baud_rate)
{
    com_set_dlab(com_port);
    unsigned int divisor = 115200 / baud_rate;
    unsigned char most_significant = (divisor >> 8) & 0xff;
    unsigned char least_significant = divisor & 0xff;
    outb(com_port, least_significant);
    outb(com_port + 1, most_significant);
}

inline uint8_t com_get_lcr(const uint16_t com_port)
{
    return inb(com_port + 3);
}

inline void com_set_lcr(const uint16_t com_port, char data)
{
    outb(com_port + 3, data);
}

inline void com_set_data_bits(const uint16_t com_port, data_bits bits)
{
    auto lcr = com_get_lcr(com_port);
    com_set_lcr(com_port, (lcr & ~(0b11)) | bits);
}

inline void com_set_stop_bits(const uint16_t com_port)
{
    auto lcr = com_get_lcr(com_port);
    lcr |= (1 << 2);
    com_set_lcr(com_port, lcr);
}

inline void com_clear_stop_bits(const uint16_t com_port)
{
    auto lcr = com_get_lcr(com_port);
    lcr &= ~(1 << 2);
    com_set_lcr(com_port, lcr);
}

inline void com_set_parity_none(const uint16_t com_port)
{
    auto lcr = com_get_lcr(com_port);
    lcr &= ~(0b111 << 3);
    com_set_lcr(com_port, lcr);
}

inline void com_set_parity_odd(const uint16_t com_port)
{
    com_set_parity_none(com_port);
    auto lcr = com_get_lcr(com_port);
    lcr |= 0b001 << 3;
    com_set_lcr(com_port, lcr);
}

inline void com_set_parity_even(const uint16_t com_port)
{
    com_set_parity_none(com_port);
    auto lcr = com_get_lcr(com_port);
    lcr |= 0b011 << 3;
    com_set_lcr(com_port, lcr);
}

inline void com_set_parity_mark(const uint16_t com_port)
{
    com_set_parity_none(com_port);
    auto lcr = com_get_lcr(com_port);
    lcr |= 0b101 << 3;
    com_set_lcr(com_port, lcr);
}

inline void com_set_parity_space(const uint16_t com_port)
{
    com_set_parity_none(com_port);
    auto lcr = com_get_lcr(com_port);
    lcr |= 0b111 << 3;
    com_set_lcr(com_port, lcr);
}

inline uint8_t com_get_ier(const uint16_t com_port)
{
    com_clear_dlab(com_port);
    auto ier = inb(com_port + 1);
    return ier;
}

inline void com_set_ier(const uint16_t com_port, char ier)
{
    com_clear_dlab(com_port);
    outb(com_port + 1, ier);
}

inline uint8_t com_get_fcr(const uint16_t com_port)
{
    auto fcr = inb(com_port + 2);
    return fcr;
}

inline void com_set_fcr(const uint16_t com_port, char fcr)
{
    outb(com_port + 2, fcr);
}

inline void com_set_fifo(const uint16_t com_port)
{
    auto fcr = com_get_fcr(com_port);
    fcr |= 0b1;
    com_set_fcr(com_port, fcr);
}

inline void com_clear_fifo(const uint16_t com_port)
{
    auto fcr = com_get_fcr(com_port);
    fcr &= ~0b1;
    com_set_fcr(com_port, fcr);
}

inline bool com_is_interrupt_pending(const uint16_t com_port)
{
    auto fcr = com_get_ier(com_port);
    return fcr & 0b1;
}

inline void com_enable_1_byte_itl(const uint16_t com_port)
{
    auto fcr = com_get_fcr(com_port);
    fcr &= (0b00 << 5);
    com_set_fcr(com_port, fcr);
}

inline void com_enable_4_byte_itl(const uint16_t com_port)
{
    auto fcr = com_get_fcr(com_port);
    com_enable_1_byte_itl(com_port);
    fcr &= (0b01 << 5);
    com_set_fcr(com_port, fcr);
}

inline void com_enable_8_byte_itl(const uint16_t com_port)
{
    auto fcr = com_get_fcr(com_port);
    com_enable_1_byte_itl(com_port);
    fcr &= (0b10 << 5);
    com_set_fcr(com_port, fcr);
}

inline void com_enable_14_byte_itl(const uint16_t com_port)
{
    auto fcr = com_get_fcr(com_port);
    com_enable_1_byte_itl(com_port);
    fcr &= (0b11 << 5);
    com_set_fcr(com_port, fcr);
}

inline uint8_t com_get_mcr(const uint16_t com_port)
{
    auto mcr = inb(com_port + 4);
    return mcr;
}

inline void com_set_mcr(const uint16_t com_port, char mcr)
{
    outb(com_port + 4, mcr);
}

inline void com_set_irq(const uint16_t com_port)
{
    auto mcr = com_get_mcr(com_port);
    mcr |= (1 << 3);
    com_set_mcr(com_port, mcr);
}

inline void com_clear_irq(const uint16_t com_port)
{
    auto mcr = com_get_mcr(com_port);
    mcr &= ~(1 << 3);
    com_set_mcr(com_port, mcr);
}

inline void com_flush_fifo(const uint16_t com_port)
{
    auto fcr = com_get_fcr(com_port);
    fcr |= 0b110;
    com_set_fcr(com_port, fcr);
}

inline void com_enable_rts(const uint16_t com_port)
{
    auto mcr = com_get_mcr(com_port);
    mcr |= 0b10;
    com_set_mcr(com_port, mcr);
}

inline void com_enable_dtr(const uint16_t com_port)
{
    auto mcr = com_get_mcr(com_port);
    mcr |= 0b1;
    com_set_mcr(com_port, mcr);
}

#endif //KITTY_OS_CPP_UART_HPP
