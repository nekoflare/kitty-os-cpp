#include <acpi/acpi.hpp>
#include <debug.hpp>
#include <hal/pcie/pcie.hpp>
#include <hal/pcie/pcie_database.hpp>
#include <mem/common_memory.hpp>
#include <vector>

struct mcfg *mcfg = nullptr;
void *mcfg_table_pointer = nullptr;

std::vector<mcfg_entry *> mcfg_entries;

void traverse_pcie()
{
    debug_printf("Traversing PCI-e...\n");

    for (auto &entry : mcfg_entries)
    {
        for (auto bus = entry->start_bus; entry->end_bus > bus; bus++)
        {
            for (auto device = 0; 32 > device; device++)
            {
                for (auto function = 0; 8 > function; function++)
                {
                    auto address = reinterpret_cast<void *>((((bus * 256) + (device * 8) + function) * 4096) +
                                                            entry->base_address + get_higher_half_offset());

                    auto pcie_header = reinterpret_cast<struct pcie_header *>(address);
                    if (pcie_header->vendor_id != 0xffff && pcie_header->device_id != 0xffff)
                    {
                        auto db_entry = find_in_pcie_database(pcie_header->vendor_id, pcie_header->device_id);
                        if (db_entry)
                        {
                            debug_printf("Vendor: %s Device: %s\n", db_entry->vendor_name, db_entry->device_name);
                        }
                        else
                        {
                            debug_printf("Vendor ID: %04hX Device ID: %04hX\n", pcie_header->vendor_id,
                                         pcie_header->device_id);
                        }
                    }
                }
            }
        }
    }
}

void initialize_pcie()
{
    mcfg_table_pointer = get_acpi_table(MCFG_SIGNATURE);
    if (mcfg_table_pointer == nullptr)
    {
        debug_printf("No MCFG table has been found.\n");
        return;
    }

    mcfg = reinterpret_cast<struct mcfg *>(mcfg_table_pointer);
    auto entry_count = (mcfg->sdt.length - sizeof(struct sdt) - sizeof(mcfg->reserved)) / sizeof(mcfg_entry);
    debug_printf("Entry count: %zu\n", entry_count);
    for (size_t entry_index = 0; entry_count > entry_index; entry_index++)
    {
        auto &entry = *reinterpret_cast<mcfg_entry *>(reinterpret_cast<uintptr_t>(mcfg) + sizeof(struct sdt) +
                                                      sizeof(mcfg->reserved) + (entry_index * sizeof(mcfg_entry)));

        debug_printf("Base address: %016lX Start bus: %x End bus: %x\n", entry.base_address,
                     static_cast<uint32_t>(entry.start_bus), static_cast<uint32_t>(entry.end_bus));

        mcfg_entries.push_back(&entry);
    }

    traverse_pcie();
}
