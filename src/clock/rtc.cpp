//
// Created by Neko on 25.01.2025.
//

#include "rtc.h"

#include <dbg/log.h>
#include <hal/cpu/io.h>
#include <hal/irq/irq.h>
#include <smp/smp.h>

handle irq8_handle;

// Helper functions
static uint8_t read_cmos_register(const uint8_t reg)
{
    outb(0x70, reg);  // Select the register
    return inb(0x71); // Read the value
}

static uint8_t bcd_to_binary(const uint8_t bcd)
{
    return (bcd & 0x0F) + (bcd >> 4) * 10;
}

static int is_bcd_mode()
{
    const uint8_t status_b = read_cmos_register(0x0B); // Read register B
    return !(status_b & 0x04);                         // Bit 2 is clear for BCD mode
}

static int is_24_hour_mode()
{
    const uint8_t status_b = read_cmos_register(0x0B); // Read register B
    return status_b & 0x02;                            // Bit 1 is set for 24-hour mode
}

static uint8_t convert_to_24_hour(uint8_t hour)
{
    const int is_pm = hour & 0x80; // Check if the PM bit (0x80) is set
    hour &= 0x7F;                  // Mask off the PM bit

    if (is_pm && hour != 12)
    {
        hour += 12; // Convert PM hours to 24-hour format
    }
    else if (!is_pm && hour == 12)
    {
        hour = 0; // Handle midnight (12 AM)
    }

    return hour;
}

system_date get_system_date()
{
    system_date date;

    const int bcd_mode = is_bcd_mode();
    const int hour_24_mode = is_24_hour_mode();

    date.second = read_cmos_register(0x00);
    date.minute = read_cmos_register(0x02);
    date.hour = read_cmos_register(0x04);
    date.day = read_cmos_register(0x07);
    date.month = read_cmos_register(0x08);
    date.year = read_cmos_register(0x09);

    if (bcd_mode)
    {
        date.second = bcd_to_binary(date.second);
        date.minute = bcd_to_binary(date.minute);
        date.hour = bcd_to_binary(date.hour);
        date.day = bcd_to_binary(date.day);
        date.month = bcd_to_binary(date.month);
        date.year = bcd_to_binary(date.year);
    }

    if (!hour_24_mode)
    {
        date.hour = convert_to_24_hour(date.hour);
    }

    if (date.year < 70)
    {
        date.year += 2000;
    }
    else
    {
        date.year += 1900;
    }

    return date;
}

// IRQ handler for RTC interrupts
static void irq_handler(handle irq)
{
    outb(0x70, 0x0C); // Select register C
    inb(0x71);        // Acknowledge the interrupt by reading register C
}

// RTC initialization
void initialize_rtc()
{
    asm volatile("cli");

    outb(0x70,
         0x8A);       // Select Status Register A and disable NMI (set the 0x80 bit)
    outb(0x71, 0x20); // Set frequency divider (0x20 for 2 Hz)

    // Hook IRQ 8 for RTC interrupts
    auto status = hook_irq(&irq8_handle, false, get_current_processor_id(), 8, irq_handler);
    if (status < 0)
    {
        debug_print("Failed to hook IRQ 8.\n");
        asm volatile("cli; hlt");
    }

    outb(0x70, 0x8B);         // Select Status Register B and disable NMI
    uint8_t prev = inb(0x71); // Read current value of Register B
    outb(0x70, 0x8B);         // Re-select Register B (read resets index to D)
    outb(0x71,
         prev | 0x40); // Write back with bit 6 set (enable periodic interrupts)

    asm volatile("sti");
}
