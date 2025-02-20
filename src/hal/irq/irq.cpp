//
// Created by Neko on 20.01.2025.
//

#include "irq.h"

#include <acpi/acpi.h>
#include <cstdint>
#include <dbg/log.h>
#include <hal/apic/apic.h>
#include <hal/ioapic/ioapic.h>
#include <mem/virtual.h>
#include <smp/smp.h>

#define MAX_CPUS 256

struct map_entry_t
{
    uint64_t irqId;
    uint64_t vector;
    uint64_t cpu_id;
    std::vector<std::function<void(handle irq)>> callbacks;
};

struct irq_map_entry_t
{
    uint8_t vec[32];
    map_entry_t mapping[256];
};

struct irq_map_entry_t vectors[MAX_CPUS];

void initialize_irq_vectors()
{
    for (size_t i = 0; MAX_CPUS > i; i++)
    {
        vectors[i].vec[0] = 0xff;
        vectors[i].vec[1] = 0xff;
        vectors[i].vec[2] = 0xff;
        vectors[i].vec[3] = 0xff;
    }
}

uint64_t get_irq_from_handle(handle irq)
{
    const auto ent = static_cast<map_entry_t *>(irq);
    return ent->irqId;
}

uint64_t get_idt_vector_from_handle(handle irq)
{
    const auto ent = static_cast<map_entry_t *>(irq);
    return ent->vector;
}

uint64_t get_cpu_id_from_handle(handle irq)
{
    const auto ent = static_cast<map_entry_t *>(irq);
    return ent->cpu_id;
}

uint64_t allocate_vector_on_cpu(const uint64_t cpu_index)
{
    if (cpu_index >= MAX_CPUS)
    {
        return INVALID_VECTOR;
    }

    for (size_t i = 0; i < 32; i++)
    {
        auto &nibble = vectors[cpu_index].vec[i];
        if (nibble != 0xff)
        { // Check if there's any free bit
            for (size_t j = 0; j < 8; j++)
            {
                if ((nibble & (1 << j)) == 0)
                {                       // Check if bit `j` is free
                    nibble |= (1 << j); // Mark bit `j` as used
                    return i * 8 + j;   // Return the allocated vector index
                }
            }
        }
    }

    return INVALID_VECTOR;
}

void free_vector_on_cpu(const uint64_t cpu_index, const uint64_t vector)
{
    if (cpu_index >= MAX_CPUS || vector >= 256)
    {
        return;
    }

    const size_t nibble_index = vector / 8;
    const size_t bit_index = vector % 8;

    vectors[cpu_index].vec[nibble_index] |= (1 << bit_index);
}

status hook_irq(handle *irq_handle, bool is_gsi, const uint64_t cpu_id, uint64_t irq,
                const std::function<void(handle irq)> &handler)
{
    // ? add check if cpu actually exists ?
    asm volatile("cli");

    if (cpu_id >= MAX_CPUS)
    {
        asm volatile("sti");
        return STATUS_INVALID_ARGUMENTS;
    }

    const auto vector = allocate_vector_on_cpu(cpu_id);

    if (vector == INVALID_VECTOR)
    {
        asm volatile("sti");
        return STATUS_OUT_OF_RESOURCES;
    }

    const auto io_apics = get_io_apics();
    const auto io_apic_count = get_io_apic_count();

    if (!is_gsi)
    {
        const auto interrupt_source_overrides = get_isos();
        const auto interrupt_source_override_count = get_iso_count();

        for (size_t i = 0; interrupt_source_override_count > i; i++)
        {
            if (const auto interrupt_source_override = interrupt_source_overrides[i];
                interrupt_source_override.source == irq)
            {
                if (!interrupt_source_overrides[i].bus == 0)
                    continue;

                const auto previous_irq = irq;
                irq = interrupt_source_override.global_system_interrupt;

                debug_print("Found an override for IRQ%ld: GSI%ld\n", previous_irq, irq);

                break;
            }
        }
    }

    for (size_t i = 0; io_apic_count > i; i++)
    {
        const auto io_apic_address = io_apics[i].io_apic_address + get_higher_half_memory_offset();
        const auto gsi_base = io_apics[i].global_system_interrupt_base;
        const auto max_redirections = get_ioapic_max_redirections(io_apic_address);

        const auto gsi_end = gsi_base + max_redirections;
        if (irq >= gsi_base && irq < gsi_end)
        {
            vectors[cpu_id].mapping[vector].callbacks.push_back(handler);
            vectors[cpu_id].mapping[vector].vector = vector;
            vectors[cpu_id].mapping[vector].irqId = irq;
            vectors[cpu_id].mapping[vector].cpu_id = cpu_id;

            debug_print("Mapped IRQ %ld to vector %ld.\n", irq, vector);

            *irq_handle = static_cast<void *>(&vectors[cpu_id].mapping[vector]);

            // now tell the ioapic to redirect GSIs to this vector to that cpu!
            ioapic_set_redirection(io_apic_address, irq - gsi_base, cpu_id, vector, 0b000, false, true, false, false,
                                   false);

            // acknowledge whatever
            apic_send_eoi(get_lapic_address() + get_higher_half_memory_offset());
            asm volatile("sti");
            return STATUS_SUCCESS;
        }
    }

    asm volatile("sti");
    return STATUS_FAILURE;
}

void dispatch_irq(uint64_t vector)
{
    const auto processor_id = get_current_processor_id();
    const auto mapping = vectors[processor_id].mapping;

    for (size_t i = 0; 256 > i; i++)
    {
        if (const auto mapping_entry = mapping[i]; mapping_entry.vector == vector && !mapping_entry.callbacks.empty())
        {
            for (const auto &callback : mapping[i].callbacks)
            {
                callback(&vectors[processor_id].mapping[vector]);
            }
        }
    }
}