//
// Created by Neko on 20.01.2025.
//

#include "acpi.h"

#include <cstring>
#include <dbg/log.h>
#include <hal/apic/apic.h>
#include <hal/cpu/msr.h>
#include <limine.h>
#include <mem/virtual.h>

limine_rsdp_request rsdp_request = {.id = LIMINE_RSDP_REQUEST, .revision = 0, .response = nullptr};

rsdp *rsdp = nullptr;
rsdt *rsdt = nullptr;
xsdt *xsdt = nullptr;
bool using_xsdt = false;
size_t table_count = 0;
uint64_t table_addresses[256] = {0}; // a table that contains virtual addresses to all tables found.

apic *apic_table;

size_t local_apic_count = 0;
local_apic_entry local_apics[256] = {};

size_t io_apic_count = 0;
io_apic_entry io_apics[256] = {};

size_t iso_count = 0; // Interrupt Source Override
interrupt_source_override_entry interrupt_source_overrides[256] = {};

size_t nmi_count = 0; // Non-Maskable Interrupt
non_maskable_interrupt_entry non_maskable_interrupts[256] = {};

void *get_rsdp_address()
{
    return rsdp;
}

static void add_table(uint64_t physical_address)
{
    uint64_t virtual_address = physical_address + get_higher_half_memory_offset();
    table_addresses[table_count] = virtual_address;
    table_count++;
}

static void print_table_signatures()
{
    debug_print("Found tables: ");
    for (size_t table_index = 0; table_count > table_index; table_index++)
    {
        auto table_pointer = table_addresses[table_index];
        auto table_structure = reinterpret_cast<struct sdt *>(table_pointer);
        debug_print("%.4s ", table_structure->signature);
    }
    debug_print("\n");
}

static void traverse()
{
    if (using_xsdt)
    {
        auto xsdt_address = reinterpret_cast<uintptr_t>(xsdt);
        xsdt_address += sizeof(struct sdt);
        size_t entry_count = (xsdt->sdt.length - sizeof(struct sdt)) / 8;

        for (size_t table_index = 0; entry_count > table_index; table_index++)
        {
            auto address = reinterpret_cast<uint64_t *>(xsdt_address + table_index * 8);
            add_table(*address);
        }
    }
    else
    {
        // using rsdt
        auto rsdt_address = reinterpret_cast<uintptr_t>(rsdt);
        rsdt_address += sizeof(struct sdt);
        size_t entry_count = (rsdt->sdt.length - sizeof(struct sdt)) / 4;

        for (size_t table_index = 0; entry_count > table_index; table_index++)
        {
            auto address = reinterpret_cast<uint32_t *>(rsdt_address + table_index * 4);
            add_table(static_cast<uint64_t>(*address));
        }
    }
}

void *get_acpi_table(const char signature[4])
{
    for (size_t table_index = 0; table_count > table_index; table_index++)
    {
        auto table_pointer = table_addresses[table_index];
        auto table_structure = reinterpret_cast<struct sdt *>(table_pointer);

        if (memcmp(&table_structure->signature, signature, 4) == 0)
        {
            return reinterpret_cast<void *>(table_pointer);
        }
    }

    return nullptr; // no table found.
}

void initialize_acpi()
{
    if (auto rsdp_response = rsdp_request.response; rsdp_response == nullptr)
    {
        debug_print("RSDP not found.\n");
        asm volatile("cli; hlt");
    }

    rsdp = static_cast<struct rsdp *>(rsdp_request.response->address);

    if (rsdp->revision >= 2)
    {
        // it's using XSDT
        xsdt = reinterpret_cast<struct xsdt *>(get_higher_half_memory_offset() + rsdp->xsdt_address);
        using_xsdt = true;
    }
    else
    {
        // it's using RSDT
        rsdt = reinterpret_cast<struct rsdt *>(get_higher_half_memory_offset() + rsdp->rsdt_address);
    }

    if (using_xsdt)
    {
        debug_print("Using XSDT\n");
    }
    else
    {
        debug_print("Using RSDT\n");
    }

    traverse();
    print_table_signatures();
    parse_apic_table();
    display_apic_entries();
}

uint64_t get_lapic_address()
{
    return apic_table->local_controller_address;
}

size_t get_local_apic_count()
{
    return local_apic_count;
}

local_apic_entry *get_local_apics()
{
    return local_apics;
}

size_t get_io_apic_count()
{
    return io_apic_count;
}

io_apic_entry *get_io_apics()
{
    return io_apics;
}

size_t get_iso_count()
{
    return iso_count;
}

interrupt_source_override_entry *get_isos()
{
    return interrupt_source_overrides;
}

size_t get_nmi_count()
{
    return nmi_count;
}

non_maskable_interrupt_entry *get_nmis()
{
    return non_maskable_interrupts;
}

void parse_apic_table()
{
    auto this_apic_table = static_cast<struct apic *>(get_acpi_table(APIC_SIGNATURE));
    if (this_apic_table == nullptr)
    {
        debug_print("APIC table not found.\n");
        return;
    }

    debug_print("Parsing APIC table...\n");
    debug_print("Local APIC address: 0x%08X\n", this_apic_table->local_controller_address);
    debug_print("Flags: 0x%08X\n", this_apic_table->flags);

    // Parse APIC entries
    uintptr_t current_entry = reinterpret_cast<uintptr_t>(this_apic_table) + sizeof(struct apic);
    uintptr_t table_end = reinterpret_cast<uintptr_t>(this_apic_table) + this_apic_table->sdt.length;

    apic_table = this_apic_table;

    while (current_entry < table_end)
    {
        auto entry = reinterpret_cast<struct apic_entry *>(current_entry);

        switch (entry->type)
        {
        case LOCAL_APIC:
            if (local_apic_count < 256)
            {
                local_apics[local_apic_count] = *reinterpret_cast<struct local_apic_entry *>(entry);
                local_apic_count++;
            }
            break;

        case IO_APIC:
            if (io_apic_count < 256)
            {
                io_apics[io_apic_count] = *reinterpret_cast<struct io_apic_entry *>(entry);
                io_apic_count++;
            }
            break;

        case INTERRUPT_SOURCE_OVERRIDE:
            if (iso_count < 256)
            {
                interrupt_source_overrides[iso_count] =
                    *reinterpret_cast<struct interrupt_source_override_entry *>(entry);
                iso_count++;
            }
            break;

        case NON_MASKABLE_INTERRUPT:
            if (nmi_count < 256)
            {
                non_maskable_interrupts[nmi_count] = *reinterpret_cast<struct non_maskable_interrupt_entry *>(entry);
                nmi_count++;
            }
            break;

        default:
            debug_print("Unknown APIC entry type: %u\n", entry->type);
            break;
        }

        current_entry += entry->length;
    }
}

void display_apic_entries()
{
    debug_print("Local APICs (%zu):\n", get_local_apic_count());
    for (size_t i = 0; i < get_local_apic_count(); i++)
    {
        auto &entry = get_local_apics()[i];
        debug_print("  Processor ID=%u, APIC ID=%u, Flags=0x%08X\n", entry.processor_id, entry.apic_id, entry.flags);
    }

    debug_print("IO APICs (%zu):\n", get_io_apic_count());
    for (size_t i = 0; i < get_io_apic_count(); i++)
    {
        auto &entry = get_io_apics()[i];
        debug_print("  IO APIC ID=%u, Address=0x%08X, GSI Base=%u\n", entry.io_apic_id, entry.io_apic_address,
                    entry.global_system_interrupt_base);
    }

    debug_print("Interrupt Source Overrides (%zu):\n", get_iso_count());
    for (size_t i = 0; i < get_iso_count(); i++)
    {
        auto &entry = get_isos()[i];
        debug_print("  Bus=%u, Source=%u, GSI=%u, Flags=0x%04X\n", entry.bus, entry.source,
                    entry.global_system_interrupt, entry.flags);
    }

    debug_print("Non-Maskable Interrupts (%zu):\n", get_nmi_count());
    for (size_t i = 0; i < get_nmi_count(); i++)
    {
        auto &entry = get_nmis()[i];
        debug_print("  Processor ID=%u, Flags=0x%04X, LINT=%u\n", entry.processor_id, entry.flags, entry.lint);
    }
}
