//
// Created by Neko on 20.01.2025.
//

#include "ioapic.h"

#include <acpi/acpi.h>
#include <dbg/log.h>
#include <mem/virtual.h>

// Write to an IO APIC register
void ioapic_write(uintptr_t ioapic_base, uint32_t reg, uint32_t value)
{
    volatile uint32_t *ioapic_index = reinterpret_cast<volatile uint32_t *>(ioapic_base);
    volatile uint32_t *ioapic_data = reinterpret_cast<volatile uint32_t *>(ioapic_base + 0x10);

    *ioapic_index = reg;
    *ioapic_data = value;
}

uint32_t ioapic_read(uintptr_t ioapic_base, uint32_t reg)
{
    volatile uint32_t *ioapic_index = reinterpret_cast<volatile uint32_t *>(ioapic_base);
    volatile uint32_t *ioapic_data = reinterpret_cast<volatile uint32_t *>(ioapic_base + 0x10);

    *ioapic_index = reg;
    return *ioapic_data;
}

void ioapic_set_redirection(uintptr_t ioapic_base, uint8_t irq, uint64_t destination, uint8_t vector,
                            uint8_t delivery_mode, bool level_triggered, bool physical_destination,
                            bool delivery_status, bool pin_polarity, bool mask_irq)
{
    uint32_t low_index = IOAPIC_REGISTER_REDIRECTION_BASE + (irq * 2);
    uint32_t high_index = IOAPIC_REGISTER_REDIRECTION_BASE + (irq * 2) + 1;

    ioapic_entry entry = {};

    entry.vector = vector;
    entry.delivery_mode = delivery_mode;
    entry.destination_mode = !physical_destination; // 0 for Physical, 1 for Logical
    entry.delivery_status = delivery_status;        // 1 for In-process
    entry.pin_polarity = pin_polarity;              // 1 for Active Low, 0 for Active High
    entry.trigger_mode = level_triggered;           // 1 for Level, 0 for Edge
    entry.mask = mask_irq;                          // 1 to mask the IRQ, 0 to unmask

    if (physical_destination)
    {
        entry.destination = static_cast<uint8_t>(destination & 0x0F);
    }
    else
    {
        entry.destination = static_cast<uint8_t>(destination & 0xFF);
    }

    uint64_t redirection = 0;

    redirection |= static_cast<uint64_t>(entry.vector);
    redirection |= static_cast<uint64_t>(entry.delivery_mode) << 8;
    redirection |= static_cast<uint64_t>(entry.destination_mode) << 11;
    redirection |= static_cast<uint64_t>(entry.delivery_status) << 12;
    redirection |= static_cast<uint64_t>(entry.pin_polarity) << 13;
    redirection |= static_cast<uint64_t>(entry.remote_irr) << 14;
    redirection |= static_cast<uint64_t>(entry.trigger_mode) << 15;
    redirection |= static_cast<uint64_t>(entry.mask) << 16;
    redirection |= static_cast<uint64_t>(entry.destination) << 56;

    ioapic_write(ioapic_base, high_index, static_cast<uint32_t>(redirection >> 32));
    ioapic_write(ioapic_base, low_index, static_cast<uint32_t>(redirection & 0xFFFFFFFF));
}

uint64_t ioapic_get_redirection(uintptr_t ioapic_base, uint8_t irq)
{
    uint32_t low_index = IOAPIC_REGISTER_REDIRECTION_BASE + (irq * 2);
    uint32_t high_index = IOAPIC_REGISTER_REDIRECTION_BASE + (irq * 2) + 1;

    uint32_t low = ioapic_read(ioapic_base, low_index);
    uint32_t high = ioapic_read(ioapic_base, high_index);

    return (static_cast<uint64_t>(high) << 32) | low;
}

uint8_t get_ioapic_max_redirections(uintptr_t ioapic_base)
{
    uint32_t version = ioapic_read(ioapic_base, 0x01);
    return ((version >> 16) & 0xFF) + 1;
}
void initialize_ioapic()
{
    auto io_apic_count = get_io_apic_count();
    auto io_apics = get_io_apics();

    for (size_t i = 0; i < io_apic_count; ++i)
    {
        const auto &entry = io_apics[i];

        uintptr_t ioapic_base = entry.io_apic_address + get_higher_half_memory_offset();
        uint8_t max_redirections = get_ioapic_max_redirections(ioapic_base);

        debug_print("Initializing IOAPIC (ID=%u, Address=0x%08lX, Redirections=%u)\n", entry.io_apic_id, ioapic_base,
                    max_redirections);

        debug_print("IOAPIC (ID=%u) initialized and all IRQs masked.\n", entry.io_apic_id);
    }
}