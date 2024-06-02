//
// Created by Piotr on 26.05.2024.
//

#ifndef KITTY_OS_CPP_ACPI_HPP
#define KITTY_OS_CPP_ACPI_HPP

#include <stdint.h>
#include <kstd/kstdio.hpp>
#include <limine.h>
#include <kernel/memory/vmm.hpp>

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

struct acpi_fadt
{
    acpi_sdt_common common;

    uint32_t firmware_ctrl; // Physical address of FACS
    uint32_t dsdt_address; // Physical address of DSDT
    uint8_t reserved;
    uint8_t preferred_PM_profile;
    uint16_t sci_int;
    uint32_t smi_cmd;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_cnt;
    uint32_t PM1a_EVT_BLK;
    uint32_t PM1b_EVT_BLK;
    uint32_t PM1a_CNT_BLK;
    uint32_t PM1b_CNT_BLK;
    uint32_t PM2_CNT_BLK;
    uint32_t PM_TMR_BLK;
    uint32_t GPE0_BLK;
    uint32_t GPE1_BLK;
    uint8_t PM1_EVT_LEN;
    uint8_t PM1_CNT_LEN;
    uint8_t PM2_CNT_LEN;
    uint8_t PM_TMR_LEN;
    uint8_t GPE0_BLK_LEN;
    uint8_t GPE1_BLK_LEN;
    uint8_t GPE1_BASE;
    uint8_t CST_CNT;
    uint16_t P_LVL2_LAT;
    uint16_t P_LVL3_LAT;
    uint16_t FLUSH_SIZE;
    uint16_t FLUSH_STRIDE;
    uint8_t DUTY_OFFSET;
    uint8_t DUTY_WIDTH;
    uint8_t DAY_ALRM;
    uint8_t MON_ALRM;
    uint8_t CENTURY;
    uint16_t IAPC_BOOT_ARCH;
    uint8_t reserved2;
    uint32_t flags;
    uint8_t RESET_REG[12];
    uint8_t RESET_VALUE;
    uint16_t ARM_BOOT_ARCH;
    uint8_t fadt_minor_version;
    uint64_t X_FIRMWARE_CTRL;
    uint64_t X_DSDT;
    uint8_t X_PM1a_EVT_BLK[12];
    uint8_t X_PM1b_EVT_BLK[12];
    uint8_t X_PM1a_CNT_BLK[12];
    uint8_t X_PM1b_CNT_BLK[12];
    uint8_t X_PM2_CNT_BLK[12];
    uint8_t X_PM_TMR_BLK[12];
    uint8_t X_GPE0_BLK[12];
    uint8_t X_GPE1_BLK[12];
    uint8_t SLEEP_CONTROL_REG[12];
    uint8_t SLEEP_STATUS_REG[12];
    uint64_t hypervisor_vendor_identity;
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

#endif //KITTY_OS_CPP_ACPI_HPP
