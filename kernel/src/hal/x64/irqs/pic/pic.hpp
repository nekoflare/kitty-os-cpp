//
// Created by Piotr on 03.06.2024.
//

#ifndef KITTY_OS_CPP_PIC_HPP
#define KITTY_OS_CPP_PIC_HPP

#include <hal/x64/io.hpp>

#define MASTER_PIC_CMD 0x20
#define MASTER_PIC_DATA 0x21
#define SLAVE_PIC_CMD 0xA0
#define SLAVE_PIC_DATA 0xA1

#define PIC_EOI 0x20

#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

#define PIC_READ_IRR                0x0a    /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR                0x0b    /* OCW3 irq service next CMD read */

inline void pic_send_eoi(int irq)
{
    if (irq >= 8)
    {
        outb(SLAVE_PIC_CMD, PIC_EOI);
    }

    outb(MASTER_PIC_CMD, PIC_EOI);
}

inline uint16_t pic_get_irq_register(int ocw3)
{
    outb(MASTER_PIC_CMD, ocw3);
    outb(SLAVE_PIC_CMD, ocw3);
    return (inb(SLAVE_PIC_CMD) << 8) | inb(MASTER_PIC_CMD);
}

void pic_remap(int master_offset, int slave_offset);
void irq_set_mask(uint8_t irq);
void irq_clear_mask(uint8_t irq);
uint16_t pic_get_irr();
uint16_t pic_get_isr();

#endif //KITTY_OS_CPP_PIC_HPP
