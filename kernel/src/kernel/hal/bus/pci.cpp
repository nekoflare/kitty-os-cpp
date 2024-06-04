//
// Created by Piotr on 30.05.2024.
//

#include <kdu/driver_ctrl.hpp>
#include <kstd/kstring.hpp>
#include "pci.hpp"

static bool is_pci_initialized = false;
static bool is_pcie_initialized = false;

void pci_walk_bus(size_t bus_idx)
{

}

void pcie_get_4k_data(acpi_mcfg_base_address_allocation_structure* structure, size_t bus_index, size_t slot_index, size_t function_index, uint32_t* dest)
{
    // Assuming pcie_create_mmio returns a pointer to the start of the memory-mapped IO region
    uint8_t* src = reinterpret_cast<uint8_t*>(pcie_create_mmio(structure, bus_index, slot_index, function_index, 0) + vmm_hhdm->offset);

    // Using kstd::memcpy to copy 4096 bytes from src to dest
    kstd::memcpy(dest, src, 4096);
}

inline void pcie_call_driver_header_0(acpi_mcfg_base_address_allocation_structure* baas, uint32_t* data, size_t bus, size_t slot, size_t function)
{
    pci_dev_header* hdr = reinterpret_cast<pci_dev_header*>(data);

    auto dev = new pci_dev;
    dev->vendor_id = hdr->hdr.vendor_id;
    dev->device_id = hdr->hdr.device_id;
    dev->subsystem_vendor_id = hdr->subsystem_vendor_id;
    dev->subsystem_device_id = hdr->subsystem_id;
    dev->status = hdr->hdr.status;
    dev->mmio_base = baas->base_address;
    dev->pci_segment = baas->pci_segment_group;
    dev->class_code = hdr->hdr.class_code;
    dev->subclass_code = hdr->hdr.subclass_code;
    dev->prog_if_code = hdr->hdr.prog_if_code;
    dev->bus = bus;
    dev->slot = slot;
    dev->function = function;
    dev->is_pcie = true;
    dev->header_type = hdr->hdr.header_type;
    dev->raw_pci_device_data = static_cast<void*>(data);
    dev->next = nullptr;
    dev->str = baas;

    pcix_add_device(dev);
    driver_ctrl_find_and_call(dev);
}

inline void pcie_call_driver_header_1(acpi_mcfg_base_address_allocation_structure* baas, uint32_t* data, size_t bus, size_t slot, size_t function)
{
    pci_dev_hdr_common* hdr = reinterpret_cast<pci_dev_hdr_common*>(data);
    //kstd::printf("Unsupported hdr type: %hx\n", hdr->header_type);
}

inline void pcie_call_driver_header_2(acpi_mcfg_base_address_allocation_structure* baas, uint32_t* data, size_t bus, size_t slot, size_t function)
{
    pci_dev_hdr_common* hdr = reinterpret_cast<pci_dev_hdr_common*>(data);
    //kstd::printf("Unsupported hdr type: %hx\n", hdr->header_type);
}

void pcie_call_driver(acpi_mcfg_base_address_allocation_structure* baas, uint32_t* data, size_t bus, size_t slot, size_t function)
{
    pci_dev_hdr_common* hdr = reinterpret_cast<pci_dev_hdr_common*>(data);
    hdr->header_type &= ~(1 << 7); // Ignore MF

    switch (hdr->header_type)
    {
        case 0x0:
            pcie_call_driver_header_0(baas, data, bus, slot, function);
            break;
        case 0x1:
            pcie_call_driver_header_1(baas, data, bus, slot, function);
            break;
        case 0x2:
            pcie_call_driver_header_2(baas, data, bus, slot, function);
            break;
        default:
            kstd::printf("Unknown header type %hhx\n", hdr->header_type);
            break;
    }
}
size_t invalid_device_count = 0;
void pcie_parse_device(acpi_mcfg_base_address_allocation_structure* structure, size_t bus_index, size_t slot_index)
{
    auto data = new uint32_t[1024];
    pcie_get_4k_data(structure, bus_index, slot_index, 0, data);
    pci_dev_hdr_common* hdr = reinterpret_cast<pci_dev_hdr_common*>(data);

    if (((hdr->header_type & 0x80) >> 8) == 1)
    {
        delete[] data;
        for (size_t i = 0; 8 > i; i++)
        {
            data = new uint32_t[1024];
            pcie_get_4k_data(structure, bus_index, slot_index, i, data);
            pci_dev_hdr_common* hdr = reinterpret_cast<pci_dev_hdr_common*>(data);
            hdr->header_type &= ~(1 << 7);

            if (hdr->device_id != 0xffff && hdr->vendor_id != 0xffff)
            {
                kstd::printf("Found multi-function device: %s Function: %zu\n", pci_get_device_name(hdr->vendor_id, hdr->device_id, 0, 0), i);

                pcie_call_driver(structure, data, bus_index, slot_index, i);
            }
        }

        return;
    }

    if (hdr->device_id != 0xffff && hdr->vendor_id != 0xffff)
    {
        kstd::printf("Found device: %s\n", pci_get_device_name(hdr->vendor_id, hdr->device_id, 0, 0));
        pcie_call_driver(structure, data, bus_index, slot_index, 0);
        invalid_device_count = 0;
    }
    else
    {
        invalid_device_count++;
    }


    delete[] data;
}

const char* pci_get_device_name(const uint16_t vendor_id, const uint16_t device_id, const uint16_t subsystem_vendor_id, const uint16_t subsystem_device_id)
{
    for (size_t i = 0; pci_db_count > i; i++)
    {
        auto e = pci_db[i];
        if (e.vendor_id == vendor_id && e.device_id == device_id && e.subsystem_vendor_id == subsystem_vendor_id && e.subsystem_device_id == subsystem_device_id)
        {
            return e.name;
        }
    }
    return "Unknown device";
}

void pcie_walk_bus(acpi_mcfg_base_address_allocation_structure* structure, size_t bus_index)
{
    for (size_t slot = 0; 32 > slot; slot++)
    {
        pcie_parse_device(structure, bus_index, slot);
    }
}

void pcie_walk_buses(acpi_mcfg_base_address_allocation_structure* structure)
{
    for (size_t bus = structure->start_bus_number; structure->end_bus_number > bus; bus++)
    {
        if (invalid_device_count > 200) return;
        pcie_walk_bus(structure, bus);
    }
}

void pci_init()
{
    if (is_pci_initialized || is_pcie_initialized) return;
}

void pcie_enable_bus_mastering(pci_dev* dev)
{
    uint16_t* mmio = reinterpret_cast<uint16_t*>(pcie_create_mmio(dev->str, dev->bus, dev->slot, dev->function, 4) + vmm_hhdm->offset);
    *mmio |= (1 << 2);
}

void pcie_enable_io_mastering(pci_dev* dev)
{
    uint16_t* mmio = reinterpret_cast<uint16_t*>(pcie_create_mmio(dev->str, dev->bus, dev->slot, dev->function, 4) + vmm_hhdm->offset);
    *mmio |= (1 << 0);
}

void pcie_enable_mem_mastering(pci_dev* dev)
{
    uint16_t* mmio = reinterpret_cast<uint16_t*>(pcie_create_mmio(dev->str, dev->bus, dev->slot, dev->function, 4) + vmm_hhdm->offset);
    *mmio |= (1 << 1);
}

void pci_dump_database()
{
    for (size_t i = 0; pci_db_count > i; i++)
    {
        auto e = pci_db[i];
        if (e.subsystem_vendor_id != 0)
            kstd::printf("(%zu). %s\n", i, e.name);
    }
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

pci_dev* pci_dev_head = nullptr;

void pcix_add_device(pci_dev* dev)
{
    if (pci_dev_head == nullptr)
    {
        pci_dev_head = dev;
    }
    else
    {
        pci_dev* head = pci_dev_head;
        while (head->next != nullptr) head = head->next;
        head->next = dev;
    }
}
