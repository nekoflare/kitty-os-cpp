//
// Created by Neko on 20.01.2025.
//

#include "apic.h"

#include <acpi/acpi.h>
#include <dbg/log.h>
#include <hal/cpu/io.h>
#include <hal/cpu/msr.h>
#include <limine.h>
#include <mem/virtual.h>

limine_smp_request smp_request = {.id = LIMINE_SMP_REQUEST, .revision = 0, .response = nullptr, .flags = 0};

uint32_t get_lapic_id()
{
    return apic_read(get_lapic_address(), APIC_REGISTER_ID);
}

void disable_pic()
{
    outb(0x20, 0xff); // mask all interrupts on master pic
    outb(0xa0, 0xff); // mask all interrupts on slave pic
}

uint32_t bsp_lapic_id = ~(0);

// Read from an APIC register
uint32_t apic_read(const uintptr_t apic_base, const uint32_t offset)
{
    const auto register_address = reinterpret_cast<volatile uint32_t *>(apic_base + offset);
    return *register_address;
}

// Write to an APIC register
void apic_write(uintptr_t apic_base, const uint32_t offset, const uint32_t value)
{
    const auto register_address = reinterpret_cast<volatile uint32_t *>(apic_base + offset);
    *register_address = value;
}

void apic_send_eoi(uintptr_t apic_base)
{
    apic_write(apic_base, APIC_REGISTER_EOI, 0x0);
}

void initialize_apic()
{
    if (smp_request.response == nullptr)
    {
        debug_print("SMP request response is null.\n");
        asm volatile("cli; hlt");
    }

    bsp_lapic_id = smp_request.response->bsp_lapic_id;
    debug_print("BSP LAPIC ID: %d\n", bsp_lapic_id);

    auto lapic_count = get_local_apic_count();
    auto local_apics = get_local_apics();

    for (size_t lapic_entry_id = 0; lapic_count > lapic_entry_id; lapic_entry_id++)
    {
        if (local_apics[lapic_entry_id].apic_id == bsp_lapic_id)
        {
            auto lapic = local_apics[lapic_entry_id];
            debug_print("Found our LAPIC:\n"
                        "    Processor ID: %d\n"
                        "    APIC ID: %d\n"
                        "    Flags: %x\n",
                        static_cast<uint32_t>(lapic.processor_id), static_cast<uint32_t>(lapic.apic_id),
                        static_cast<uint32_t>(lapic.flags));
        }
    }

    apic_write(get_lapic_address() + get_higher_half_memory_offset(), 0xf0,
               apic_read(get_lapic_address() + get_higher_half_memory_offset(), 0xf0) | 0x100);
}