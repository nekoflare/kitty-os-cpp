//
// Created by Neko on 25.01.2025.
//

#ifndef PCIE_H
#define PCIE_H

#include <cstddef>
#include <cstdint>

struct pcie_header
{
    uint16_t vendor_id;      // Offset 0x00
    uint16_t device_id;      // Offset 0x02
    uint16_t command;        // Offset 0x04
    uint16_t status;         // Offset 0x06
    uint8_t revision_id;     // Offset 0x08
    uint8_t prog_if;         // Offset 0x09
    uint8_t subclass;        // Offset 0x0A
    uint8_t class_code;      // Offset 0x0B
    uint8_t cache_line_size; // Offset 0x0C
    uint8_t latency_timer;   // Offset 0x0D
    uint8_t header_type;     // Offset 0x0E
    uint8_t bist;            // Offset 0x0F
} __attribute__((packed));

struct pcie_device
{
    pcie_header base;                 // Base PCI header (common fields)
    uint32_t bar[6];                  // Offset 0x10 - 0x28: Base Address Registers (BAR0-BAR5)
    uint32_t cardbus_cis_ptr;         // Offset 0x28
    uint16_t subsystem_vendor_id;     // Offset 0x2C
    uint16_t subsystem_id;            // Offset 0x2E
    uint32_t expansion_rom_base_addr; // Offset 0x30
    uint8_t capabilities_ptr;         // Offset 0x34
    uint8_t reserved[3];              // Offset 0x35 - 0x37
    uint8_t interrupt_line;           // Offset 0x3C
    uint8_t interrupt_pin;            // Offset 0x3D
    uint8_t min_grant;                // Offset 0x3E
    uint8_t max_latency;              // Offset 0x3F
} __attribute__((packed));

struct pcie_bridge
{
    pcie_header base;                   // Base PCI header (common fields)
    uint32_t bar[2];                    // Offset 0x10 - 0x14: Base Address Registers (BAR0-BAR1)
    uint8_t primary_bus_number;         // Offset 0x18
    uint8_t secondary_bus_number;       // Offset 0x19
    uint8_t subordinate_bus_number;     // Offset 0x1A
    uint8_t secondary_latency_timer;    // Offset 0x1B
    uint8_t io_base;                    // Offset 0x1C
    uint8_t io_limit;                   // Offset 0x1D
    uint16_t secondary_status;          // Offset 0x1E
    uint16_t memory_base;               // Offset 0x20
    uint16_t memory_limit;              // Offset 0x22
    uint16_t prefetchable_memory_base;  // Offset 0x24
    uint16_t prefetchable_memory_limit; // Offset 0x26
    uint32_t prefetchable_base_upper;   // Offset 0x28
    uint32_t prefetchable_limit_upper;  // Offset 0x2C
    uint16_t io_base_upper;             // Offset 0x30
    uint16_t io_limit_upper;            // Offset 0x32
    uint8_t capabilities_ptr;           // Offset 0x34
    uint8_t reserved[3];                // Offset 0x35 - 0x37
    uint32_t expansion_rom_base_addr;   // Offset 0x38
    uint8_t interrupt_line;             // Offset 0x3C
    uint8_t interrupt_pin;              // Offset 0x3D
    uint16_t bridge_control;            // Offset 0x3E
} __attribute__((packed));

struct pcie_cardbus_bridge
{
    pcie_header base;                  // Base PCI header (common fields)
    uint32_t cardbus_socket_base_addr; // Offset 0x10
    uint8_t secondary_status;          // Offset 0x14
    uint8_t reserved;                  // Offset 0x15
    uint8_t capabilities_ptr;          // Offset 0x16
    uint8_t cardbus_latency_timer;     // Offset 0x18
    uint8_t subordinate_bus_number;    // Offset 0x19
    uint8_t cardbus_bus_number;        // Offset 0x1A
    uint8_t pci_bus_number;            // Offset 0x1B
    uint32_t memory_base_addr_0;       // Offset 0x1C
    uint32_t memory_limit_0;           // Offset 0x20
    uint32_t memory_base_addr_1;       // Offset 0x24
    uint32_t memory_limit_1;           // Offset 0x28
    uint32_t io_base_addr_0;           // Offset 0x2C
    uint32_t io_limit_0;               // Offset 0x30
    uint32_t io_base_addr_1;           // Offset 0x34
    uint32_t io_limit_1;               // Offset 0x38
    uint16_t subsystem_vendor_id;      // Offset 0x40
    uint16_t subsystem_device_id;      // Offset 0x42
    uint32_t legacy_mode_base_addr;    // Offset 0x44
} __attribute__((packed));

constexpr uint16_t STATUS_DETECTED_PARITY_ERROR = (1 << 15);    // Bit 15
constexpr uint16_t STATUS_SIGNALLED_SYSTEM_ERROR = (1 << 14);   // Bit 14
constexpr uint16_t STATUS_RECEIVED_MASTER_ABORT = (1 << 13);    // Bit 13
constexpr uint16_t STATUS_RECEIVED_TARGET_ABORT = (1 << 12);    // Bit 12
constexpr uint16_t STATUS_SIGNALLED_TARGET_ABORT = (1 << 11);   // Bit 11
constexpr uint16_t STATUS_DEVSEL_TIMING = (0b11 << 9);          // Bits 9-10 (2 bits)
constexpr uint16_t STATUS_MASTER_DATA_PARITY_ERROR = (1 << 8);  // Bit 8
constexpr uint16_t STATUS_FAST_BACK_TO_BACK_CAPABLE = (1 << 7); // Bit 7
constexpr uint16_t STATUS_66MHZ_CAPABLE = (1 << 5);             // Bit 5
constexpr uint16_t STATUS_CAPABILITIES_LIST = (1 << 4);         // Bit 4
constexpr uint16_t STATUS_INTERRUPT_STATUS = (1 << 3);          // Bit 3

constexpr uint16_t COMMAND_INTERRUPT_DISABLE = (1 << 10);          // Bit 10
constexpr uint16_t COMMAND_FAST_BACK_TO_BACK_ENABLE = (1 << 9);    // Bit 9
constexpr uint16_t COMMAND_SERR_ENABLE = (1 << 8);                 // Bit 8
constexpr uint16_t COMMAND_PARITY_ERROR_RESPONSE = (1 << 6);       // Bit 6
constexpr uint16_t COMMAND_VGA_PALETTE_SNOOP = (1 << 5);           // Bit 5
constexpr uint16_t COMMAND_MEMORY_WRITE_AND_INVALIDATE = (1 << 4); // Bit 4
constexpr uint16_t COMMAND_SPECIAL_CYCLES = (1 << 3);              // Bit 3
constexpr uint16_t COMMAND_BUS_MASTER = (1 << 2);                  // Bit 2
constexpr uint16_t COMMAND_MEMORY_SPACE = (1 << 1);                // Bit 1
constexpr uint16_t COMMAND_IO_SPACE = (1 << 0);                    // Bit 0

constexpr uint8_t HEADER_TYPE_MASK = 0x7F; // Mask for bits 6-0 (Header Type)
constexpr uint8_t MF_MASK = 0x80;          // Mask for bit 7 (MF)

constexpr uint8_t BIST_CAPABLE_MASK = 0x80;    // Mask for bit 7 (BIST Capable)
constexpr uint8_t START_BIST_MASK = 0x40;      // Mask for bit 6 (Start BIST)
constexpr uint8_t COMPLETION_CODE_MASK = 0x0F; // Mask for bits 0-3 (Completion Code)

constexpr uint32_t MEM_BASE_ADDR_MASK = 0xFFFFFFF0;    // Mask for the 16-byte aligned base address (Bits 31-4)
constexpr uint32_t MEM_PREFETCHABLE_MASK = 0x00000008; // Mask for Bit 3 (Prefetchable bit)
constexpr uint32_t MEM_TYPE_MASK = 0x00000006;         // Mask for Bits 2-1 (Memory type)
constexpr uint32_t MEM_ALWAYS_0_MASK = 0x00000001;     // Mask for Bit 0 (Always 0)

constexpr uint32_t IO_BASE_ADDR_MASK = 0xFFFFFFFC; // Mask for the 4-byte aligned base address (Bits 31-2)
constexpr uint32_t IO_RESERVED_MASK = 0x00000002;  // Mask for Bit 1 (Reserved, should be set to 1)
constexpr uint32_t IO_ALWAYS_1_MASK = 0x00000001;  // Mask for Bit 0 (Always 1, indicates I/O space)

void initialize_pcie();
void pcie_raw_read(uint16_t segment, uint8_t bus, uint8_t slot, uint8_t function, size_t offset, uint8_t *buffer,
                   size_t length);

#endif // PCIE_H
