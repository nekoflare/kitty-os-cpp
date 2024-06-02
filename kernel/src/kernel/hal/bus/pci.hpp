//
// Created by Piotr on 30.05.2024.
//

#ifndef KITTY_OS_CPP_PCI_HPP
#define KITTY_OS_CPP_PCI_HPP

#include <kernel/hal/x64/io.hpp>
#include <firmware/acpi/acpi.hpp>
#include <kstd/kstdio.hpp>

enum pci_header_type
{
    PCI_HEADER_DEVICE = 0x0,
    PCI_HEADER_PCI_TO_PCI_BRIDGE = 0x1,
    PCI_HEADER_PCI_TO_CARDBUS_BRIDGE = 0x2
};

enum pci_class_code
{
    UNCLASSIFIED = 0x0,
    MASS_STORAGE_CONTROLLER = 0x1,
    NETWORK_CONTROLLER = 0x2,
    DISPLAY_CONTROLLER = 0x3,
    MULTIMEDIA_CONTROLLER = 0x4,
    MEMORY_CONTROLLER = 0x5,
    BRIDGE = 0x6,
    SIMPLE_COMMUNICATION_CONTROLLER = 0x7,
    BASE_SYSTEM_PERIPHERAL = 0x8,
    INPUT_DEVICE_CONTROLLER = 0x9,
    DOCKING_STATION = 0xA,
    PROCESSOR = 0xB,
    SERIAL_BUS_CONTROLLER = 0xC,
    WIRELESS_CONTROLLER = 0xD,
    INTELIGENT_CONTROLLER = 0xE,
    SATELLITE_COMMUNICATION_CONTROLLER = 0xF,
    ENCRYPTION_CONTROLLER = 0x10,
    SIGNAL_PROCESSING_CONTROLLER = 0x11,
    PROCESSING_ACCELERATOR = 0x12,
    NON_ESSENTIAL_INSTRUMENTATION  = 0x13,
    /// 0x14 - 0x3F (Reserved)
    CO_PROCESSOR = 0x40,
    /// 0x41 - 0xFE (Reserved)
    UNASSIGNED_CLASS = 0xFF
};

struct pci_dev
{
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_device_id;
    uint16_t status;
    uint64_t mmio_base;
    uint64_t pci_segment;

    uint16_t class_code;
    uint16_t subclass_code;
    uint16_t prog_if_code;

    size_t bus, slot, function;

    bool is_pcie;

    uint8_t header_type;
    void* raw_pci_device_data;
    struct pci_dev* next;
};

struct pci_dev_hdr_common
{
    uint16_t vendor_id, device_id, command, status;
    uint8_t revision_id, prog_if_code, subclass_code, class_code;
    uint8_t cache_line_size, latency_timer, header_type, bist;
} __attribute__((packed));

struct pci_dev_header
{
    struct pci_dev_hdr_common hdr;
    uint32_t bars[5];
    uint32_t cardbus_cis_pointer;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
    uint32_t expansion_rom_base_addr;
    uint8_t cap_pointer;
    uint16_t rsvd0;
    uint8_t rsvd1;
    uint32_t rsvd2;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint8_t min_grant;
    uint8_t max_latency;
} __attribute__((packed));

struct pci_db_entry
{
    const uint16_t vendor_id;
    const uint16_t device_id;
    const uint16_t subsystem_vendor_id;
    const uint16_t subsystem_device_id;
    const char* name;
};

constexpr uint32_t PCI_CFG_ADDR = 0xCF8;
constexpr uint32_t PCI_CFG_DATA = 0xCFC;

extern pci_db_entry pci_db[];
extern const size_t pci_db_count;

void pci_init();
void pcie_init();

constexpr uint64_t pcie_create_mmio(acpi_mcfg_base_address_allocation_structure* structure, size_t bus_index, size_t slot_index, size_t function_index, size_t offset)
{
    return 0;
}

#endif //KITTY_OS_CPP_PCI_HPP
