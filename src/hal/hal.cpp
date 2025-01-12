#include <acpi/acpi.hpp>
#include <cstring>
#include <debug.hpp>
#include <hal/apic/apic.hpp>
#include <hal/gdt/gdt.hpp>
#include <hal/hal.hpp>
#include <hal/idt/idt.hpp>
#include <hal/ioapic/ioapic.hpp>
#include <optional>
#include <sys/symbols.hpp>
#include <vector>

struct occupied_vectors
{
    uint8_t vectors[32]; // 32 * 8 = 256
} __attribute__((packed));

struct ioapic_descriptor
{
    uint64_t ioapic_address;
    uint64_t min_gsi;
    uint64_t max_gsi;
};

std::vector<std::pair<uint64_t, occupied_vectors>> cpu_vectors; // pair of lapic id and vectors.
std::vector<ioapic_descriptor> ioapic_descriptors;

void initialize_hal(void)
{
    load_gdt();
    load_idt();
}

void initialize_irqs()
{
    // initialize vectors.
    // mask first 32 interrupts.
    uint8_t vec[32] = {};
    vec[0] = 0xff;
    vec[1] = 0xff;
    vec[2] = 0xff;
    vec[3] = 0xff;

    for (uint64_t i = 0; get_cpu_count() > i; i++)
    {
        struct occupied_vectors ovec
        {
        };
        memcpy(&ovec.vectors, &vec, sizeof(vec));
        cpu_vectors.push_back(std::make_pair(i, ovec));
    }

    auto ioapic_count = get_io_apic_count();
    auto ioapics = get_io_apics();

    for (size_t i = 0; ioapic_count > i; i++)
    {
        struct ioapic_descriptor ioa_desc = {.ioapic_address = ioapics[i].io_apic_address,
                                             .min_gsi = ioapics[i].global_system_interrupt_base,
                                             .max_gsi = ioapics[i].global_system_interrupt_base +
                                                        get_ioapic_max_redirections(ioapics[i].io_apic_address)};

        ioapic_descriptors.push_back(ioa_desc);
    }
}

void print_ioapic_descriptors()
{
    for (const auto &e : ioapic_descriptors)
    {
        debug_printf("Min GSI: %ld Max GSI: %ld\n", e.min_gsi, e.max_gsi);
    }
}

// Allocate a vector and return a pair of CPU index and vector bit.
std::optional<std::pair<uint64_t, uint8_t>> allocate_vector()
{
    for (auto &[cpu_index, ovec] : cpu_vectors)
    {
        for (uint8_t byte = 0; byte < 32; ++byte) // Iterate over each byte in the vector array
        {
            if (ovec.vectors[byte] != 0xFF) // Check if there is a free bit
            {
                // Find the first free bit in the byte
                for (uint8_t bit = 0; bit < 8; ++bit)
                {
                    uint8_t mask = 1 << bit;
                    if ((ovec.vectors[byte] & mask) == 0)
                    {
                        // Allocate this bit
                        ovec.vectors[byte] |= mask;
                        // Return the CPU index and vector bit
                        return std::make_pair(cpu_index, byte * 8 + bit);
                    }
                }
            }
        }
    }

    // No available vector found
    return std::nullopt;
}

uint8_t translate_irq_to_gsi(uint8_t irq)
{
    auto override_count = get_iso_count();
    auto overrides = get_isos();

    for (size_t i = 0; override_count > i; i++)
    {
        const auto &e = overrides[i];

        if (e.source == irq && e.bus == 0) // if source is the our irq and it comes from the irq bus.
        {
            debug_printf("An override for IRQ%hhd has been found: GSI%hhd.\n", irq, e.global_system_interrupt);
            return e.global_system_interrupt;
        }
    }

    debug_printf("No override found for IRQ%hhd\n", irq);
    return irq;
}

struct gsi_mapping
{
    uint64_t gsi;
    uint64_t cpu;
    uint64_t vec;
    std::function<void()> f;
};

std::vector<gsi_mapping> mapping;

bool attach_irq_handler(uint8_t irq, std::function<void()> function)
{
    // convert irq to gsi.
    auto translated = translate_irq_to_gsi(irq);
    auto allocated = allocate_vector();

    debug_printf("Allocated a vector: %hhd on CPU: %ld\n", allocated.value().second, allocated.value().first);

    // gotta save how vectors and cpus are connected to IRQs..

    for (const auto &e : ioapic_descriptors)
    {
        if (e.min_gsi <= translated && e.max_gsi >= translated)
        {
            // use this ioapic.
            ioapic_set_redirection(e.ioapic_address, translated, allocated.value().first, allocated.value().second,
                                   0b000, 0, true, false, false, false);

            mapping.push_back(
                {.gsi = translated, .cpu = allocated.value().first, .vec = allocated.value().second, .f = function});

            return true;
        }
    }

    return false;
}

// void attach_gsi_handler(uint8_t irq, std::function<void()> function)
// {
// }

void receive_irq(uint64_t interrupt_id, uint64_t cpu_id)
{
    for (const auto &e : mapping)
    {
        if (e.vec == interrupt_id + 0x20 && e.cpu == cpu_id)
        {
            e.f();
            return;
        }
    }

    debug_printf("AN IRQ HAS BEEN RECEIVED BUT NO ONE WILL RECEIVE IT!\n");
    asm volatile("cli; hlt");
}

EXPORT_SYMBOL(attach_irq_handler);
// EXPORT_SYMBOL(attach_gsi_handler);
EXPORT_SYMBOL(receive_irq);