//
// Created by Piotr on 03.06.2024.
//

#include "pic.hpp"

void pic_remap(int master_offset, int slave_offset)
{
    uint8_t master_data;
    uint8_t slave_data;

    master_data = inb(MASTER_PIC_DATA);
    slave_data = inb(SLAVE_PIC_DATA);

    outb(MASTER_PIC_CMD, ICW1_INIT | ICW1_ICW4);
    iowait(1);

    outb(SLAVE_PIC_CMD, ICW1_ICW4 | ICW1_ICW4);
    iowait(1);

    outb(MASTER_PIC_DATA, master_offset);
    iowait(1);

    outb(SLAVE_PIC_DATA, slave_offset);
    iowait(1);

    outb(MASTER_PIC_DATA, 4);
    iowait(1);

    outb(SLAVE_PIC_DATA, 2);
    iowait(1);

    outb(MASTER_PIC_DATA, ICW4_8086);
    iowait(1);
    outb(SLAVE_PIC_DATA, ICW4_8086);
    iowait(1);

    // restore masks
    outb(MASTER_PIC_DATA, master_data);
    outb(SLAVE_PIC_DATA, slave_data);
}

void irq_set_mask(uint8_t irq)
{
    uint16_t port;
    uint8_t value;

    if (irq < 8)
    {
        port = MASTER_PIC_DATA;
    }
    else
    {
        port = SLAVE_PIC_DATA;
        irq -= 8;
    }

    value = inb(port) | ( 1 << irq);
    outb(port, value);
}

void irq_clear_mask(uint8_t irq)
{
    uint16_t port;
    uint8_t value;

    if (irq < 8)
    {
        port = MASTER_PIC_DATA;
    }
    else
    {
        port = SLAVE_PIC_DATA;
        irq -= 8;
    }

    value = inb(port) & ~( 1 << irq);
    outb(port, value);
}

uint16_t pic_get_irr()
{
    return pic_get_irq_register(PIC_READ_IRR);
}

uint16_t pic_get_isr()
{
    return pic_get_irq_register(PIC_READ_ISR);
}