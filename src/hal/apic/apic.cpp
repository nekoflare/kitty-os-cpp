#include <acpi/acpi.hpp>
#include <debug.hpp>
#include <hal/apic/apic.hpp>
#include <hal/io.hpp>
#include <limine.h> // we will use limine_smp_info to get bsp lapic id
#include <mem/common_memory.hpp>
#include <sys/symbols.hpp>

limine_smp_request smp_request = {.id = LIMINE_SMP_REQUEST, .revision = 0, .response = nullptr, .flags = 0};

uint64_t get_cpu_count()
{
    return smp_request.response->cpu_count;
}

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
uint32_t apic_read(uintptr_t apic_base, uint32_t offset)
{
    volatile uint32_t *register_address =
        reinterpret_cast<volatile uint32_t *>(apic_base + offset + get_higher_half_offset());
    return *register_address;
}

// Write to an APIC register
void apic_write(uintptr_t apic_base, uint32_t offset, uint32_t value)
{
    volatile uint32_t *register_address =
        reinterpret_cast<volatile uint32_t *>(apic_base + offset + get_higher_half_offset());
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
        debug_printf("SMP request response is null.\n");
        asm volatile("cli; hlt");
    }

    bsp_lapic_id = smp_request.response->bsp_lapic_id;
    debug_printf("BSP LAPIC ID: %d\n", bsp_lapic_id);

    auto lapic_count = get_local_apic_count();
    auto lapics = get_local_apics();

    for (size_t lapic_entry_id = 0; lapic_count > lapic_entry_id; lapic_entry_id++)
    {
        if (lapics[lapic_entry_id].apic_id == bsp_lapic_id)
        {
            auto lapic = lapics[lapic_entry_id];
            debug_printf("Found our LAPIC:\n"
                         "    Processor ID: %d\n"
                         "    APIC ID: %d\n"
                         "    Flags: %x\n",
                         static_cast<uint32_t>(lapic.processor_id), static_cast<uint32_t>(lapic.apic_id),
                         static_cast<uint32_t>(lapic.flags));
        }
    }

    apic_write(get_lapic_address(), 0xf0, apic_read(get_lapic_address(), 0xf0) | 0x100);
}

EXPORT_SYMBOL(apic_read);
EXPORT_SYMBOL(apic_write);
EXPORT_SYMBOL(apic_send_eoi);
EXPORT_SYMBOL(get_cpu_count);
EXPORT_SYMBOL(get_lapic_id);