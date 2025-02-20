//
// Created by Neko on 25.01.2025.
//

#include "pcie.h"

#include <acpi/acpi.h>
#include <dbg/log.h>
#include <mem/virtual.h>
#include <vector>

static mcfg *pcie_table;
static std::vector<mcfg_entry *> mcfg_entries;

void traverse_pcie()
{
    debug_print("Traversing PCI-e...\n");

    for (const auto &entry : mcfg_entries)
    {
        for (auto bus = entry->start_bus; entry->end_bus > bus; bus++)
        {
            for (auto device = 0; 32 > device; device++)
            {
                for (auto function = 0; 8 > function; function++)
                {
                    const auto address =
                        reinterpret_cast<void *>((bus * 256 + (device * 8) + function) * 4096 + entry->base_address +
                                                 get_higher_half_memory_offset());

                    if (const auto header = static_cast<pcie_header *>(address);
                        header->vendor_id != 0xffff && header->device_id != 0xffff)
                    {
                        debug_print("Vendor: %04hX Device: %04hX\n", header->vendor_id, header->device_id);
                    }
                }
            }
        }
    }
}

void initialize_pcie()
{
    pcie_table = static_cast<mcfg *>(get_acpi_table(MCFG_SIGNATURE));
    if (!pcie_table)
    {
        debug_print("No PCI-e on this device.\n");

        return;
    }

    const auto entry_count = (pcie_table->sdt.length - sizeof(sdt) - sizeof(pcie_table->reserved)) / sizeof(mcfg_entry);
    for (size_t entry_index = 0; entry_count > entry_index; entry_index++)
    {
        auto &entry =
            *reinterpret_cast<mcfg_entry *>(reinterpret_cast<uintptr_t>(pcie_table) + sizeof(sdt) +
                                            sizeof(pcie_table->reserved) + (entry_index * sizeof(mcfg_entry)));

        debug_print("Base address: %016lX Start bus: %x End bus: %x\n", entry.base_address,
                    static_cast<uint32_t>(entry.start_bus), static_cast<uint32_t>(entry.end_bus));

        mcfg_entries.push_back(&entry);
    }

    traverse_pcie();
}

void pcie_raw_read(const uint16_t segment, const uint8_t bus, const uint8_t slot, const uint8_t function,
                   const size_t offset, uint8_t *buffer, const size_t length)
{
    if (!buffer || length == 0)
        return;

    for (const auto &entry : mcfg_entries)
    {
        if (entry->segment_group != segment)
            continue;

        if (bus < entry->start_bus || bus > entry->end_bus)
            continue;

        const uintptr_t base_address = entry->base_address + get_higher_half_memory_offset();
        const uintptr_t address =
            base_address + ((bus - entry->start_bus) << 20) + (slot << 15) + (function << 12) + offset;

        const auto *src = reinterpret_cast<uint8_t *>(address);
        for (size_t i = 0; i < length; i++)
        {
            buffer[i] = src[i];
        }
        return;
    }
}
