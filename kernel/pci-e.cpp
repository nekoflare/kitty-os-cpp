//
// Created by Piotr on 26.05.2024.
//

#include "pci-e.hpp"

acpi_mcfg* pcie_memory_configuration;

void pcie_init()
{
    kstd::printf("[PCI-e] Initializing PCI express...\n");

    pcie_memory_configuration = acpi_get_mcfg();

    if (pcie_memory_configuration == nullptr)
    {
        kstd::printf("[PCI-e] This system doesn't have PCI express memory configuration ACPI table.\n");

        return;
    }

    // Correct the calculation for the start of the BAAS entries
    auto baas = reinterpret_cast<acpi_mcfg_base_address_allocation_structure*>(
            reinterpret_cast<uint8_t*>(pcie_memory_configuration) + sizeof(acpi_mcfg));
    size_t entry_count = (pcie_memory_configuration->common.length - sizeof(acpi_mcfg)) / sizeof(acpi_mcfg_base_address_allocation_structure);

    kstd::printf("[PCI-e] Entry count: %lld\n", entry_count);

    for (size_t i = 0; i < entry_count; ++i)
    {
        acpi_mcfg_base_address_allocation_structure entry = baas[i];

        kstd::printf("Base address: %llx\n", entry.base_address);
        kstd::printf("PCI Segment group: %hx\n", entry.pci_segment_group);
        kstd::printf("Start bus number: %hhx\n", entry.start_bus_number);
        kstd::printf("End bus number: %hhx\n\n", entry.end_bus_number);
    }

    kstd::printf("[PCI-e] Done!\n");
}