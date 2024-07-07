//
// Created by Piotr on 26.05.2024.
//

#ifndef KITTY_OS_CPP_ACPI_HPP
#define KITTY_OS_CPP_ACPI_HPP

#include <stdint.h>
#include <kstd/kstdio.hpp>
#include <limine.h>
#include <mm/vmm.hpp>

struct acpi_sdt_common
{
    uint32_t signature;
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oemid[6];
    uint64_t oem_table_id;
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

struct acpi_rsdp
{
    uint64_t signature;
    uint8_t checksum;
    uint8_t oemid[6];
    uint8_t revision;
    uint32_t rsdt_address;      // Note: This is a physical address.
    uint32_t length;
    uint64_t xsdt_address;      // Note: This is a physical address.
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed));

struct acpi_rsdt
{
    acpi_sdt_common common;
    // At the end of this we have table pointers that are 32-bit
} __attribute__((packed));

struct acpi_apic
{
    acpi_sdt_common common;

    uint32_t local_interrupt_controller_address; // 32-bit physical address
    uint32_t flags;

    // After this we got interrupt controller structures
} __attribute__((packed));

struct GenericAddressStructure
{
    uint8_t AddressSpace;
    uint8_t BitWidth;
    uint8_t BitOffset;
    uint8_t AccessSize;
    uint64_t Address;
};

struct acpi_fadt
{
    acpi_sdt_common common;

    uint32_t FirmwareCtrl;
    uint32_t Dsdt;

    // field used in ACPI 1.0; no longer in use, for compatibility only
    uint8_t  Reserved;

    uint8_t  PreferredPowerManagementProfile;
    uint16_t SCI_Interrupt;
    uint32_t SMI_CommandPort;
    uint8_t  AcpiEnable;
    uint8_t  AcpiDisable;
    uint8_t  S4BIOS_REQ;
    uint8_t  PSTATE_Control;
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;
    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;
    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;
    uint32_t GPE0Block;
    uint32_t GPE1Block;
    uint8_t  PM1EventLength;
    uint8_t  PM1ControlLength;
    uint8_t  PM2ControlLength;
    uint8_t  PMTimerLength;
    uint8_t  GPE0Length;
    uint8_t  GPE1Length;
    uint8_t  GPE1Base;
    uint8_t  CStateControl;
    uint16_t WorstC2Latency;
    uint16_t WorstC3Latency;
    uint16_t FlushSize;
    uint16_t FlushStride;
    uint8_t  DutyOffset;
    uint8_t  DutyWidth;
    uint8_t  DayAlarm;
    uint8_t  MonthAlarm;
    uint8_t  Century;

    // reserved in ACPI 1.0; used since ACPI 2.0+
    uint16_t BootArchitectureFlags;

    uint8_t  Reserved2;
    uint32_t Flags;

    // 12 byte structure; see below for details
    GenericAddressStructure ResetReg;

    uint8_t  ResetValue;
    uint8_t  Reserved3[3];

    // 64bit pointers - Available on ACPI 2.0+
    uint64_t                X_FirmwareControl;
    uint64_t                X_Dsdt;

    GenericAddressStructure X_PM1aEventBlock;
    GenericAddressStructure X_PM1bEventBlock;
    GenericAddressStructure X_PM1aControlBlock;
    GenericAddressStructure X_PM1bControlBlock;
    GenericAddressStructure X_PM2ControlBlock;
    GenericAddressStructure X_PMTimerBlock;
    GenericAddressStructure X_GPE0Block;
    GenericAddressStructure X_GPE1Block;
} __attribute__((packed));

struct acpi_mcfg
{
    acpi_sdt_common common;
    uint64_t reserved;
    // After this we got base address allocation structures
} __attribute__((packed));

struct acpi_mcfg_base_address_allocation_structure
{
    uint64_t base_address;
    uint16_t pci_segment_group;
    uint8_t start_bus_number;
    uint8_t end_bus_number;
    uint32_t reserved;
} __attribute__((packed));

struct acpi_madt
{
    acpi_sdt_common common;
    uint32_t lapic_address;
    uint32_t flags;         // 1 = dual 8259 PICs installed.
} __attribute__((packed));

struct acpi_madt_entry
{
    uint8_t entry_type;
    uint8_t entry_length;
} __attribute__((packed));

inline const char* madt_entry_type_to_string(uint8_t entry_type)
{
    switch (entry_type)
    {
        case 0: return "Processor Local APIC";
        case 1: return "I/O APIC";
        case 2: return "I/O APIC Interrupt Source Override";
        case 3: return "I/O APIC Non-maskable interrupt source";
        case 4: return "Local APIC Non-maskable interrupts";
        case 5: return "Local APIC Address override";
        case 6: return "Processor Local x2APIC";
        default:
            return "Unknown APIC entry.";
    }
}

struct acpi_xsdt
{
    acpi_sdt_common common;
    // At the end of this we have table pointers that are 64-bit
};

void acpi_init();

acpi_mcfg* acpi_get_mcfg();
acpi_madt* acpi_get_madt();
acpi_fadt* acpi_get_fadt();

#endif //KITTY_OS_CPP_ACPI_HPP
