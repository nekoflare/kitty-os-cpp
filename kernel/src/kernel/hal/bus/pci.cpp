//
// Created by Piotr on 30.05.2024.
//

#include <kdu/driver_ctrl.hpp>
#include "pci.hpp"

static bool is_pci_initialized = false;
static bool is_pcie_initialized = false;

void pci_walk_bus(size_t bus_idx)
{

}

void pcie_parse_device(acpi_mcfg_base_address_allocation_structure* structure, size_t bus_index, size_t slot_index, size_t function_index)
{

}

void pcie_walk_slot(acpi_mcfg_base_address_allocation_structure* structure, size_t bus_index, size_t slot_index)
{
    for (size_t function = 0; 8 > function; function++)
    {
        pcie_parse_device(structure, bus_index, slot_index, function);
    }
}

void pcie_walk_bus(acpi_mcfg_base_address_allocation_structure* structure, size_t bus_index)
{
    for (size_t slot = 0; 32 > slot; slot++)
    {
        pcie_walk_slot(structure, bus_index, slot);
    }
}

void pcie_walk_buses(acpi_mcfg_base_address_allocation_structure* structure)
{
    for (size_t bus = structure->start_bus_number; structure->end_bus_number > bus; bus++)
    {
        pcie_walk_bus(structure, bus);
    }
}

void pci_init()
{
    if (is_pci_initialized || is_pcie_initialized) return;


}

void pcie_init() {
    if (is_pci_initialized || is_pcie_initialized) return;

    acpi_mcfg* mcfg = acpi_get_mcfg();
    if (mcfg == nullptr) {
        kstd::printf("MCFG table is null. Not using PCI-e\n");
        return; // Exit function if MCFG table is null
    }

    // Calculate the number of base address allocation structures in the MCFG table
    size_t num_entries = (mcfg->common.length - sizeof(acpi_mcfg)) / sizeof(acpi_mcfg_base_address_allocation_structure);

    // Iterate through each base address allocation structure
    for (size_t i = 0; i < num_entries; ++i) {
        auto baas = reinterpret_cast<acpi_mcfg_base_address_allocation_structure*>(
                reinterpret_cast<uintptr_t>(mcfg) + sizeof(acpi_mcfg) + i * sizeof(acpi_mcfg_base_address_allocation_structure));


        pcie_walk_buses(baas);
    }

    // Mark PCIe as initialized
    is_pcie_initialized = true;
}
