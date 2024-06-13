//
// Created by Piotr on 24.05.2024.
//

#ifndef KITTY_OS_CPP_SMBIOS_HPP
#define KITTY_OS_CPP_SMBIOS_HPP

#include <limine.h>
#include <kstd/kstdio.hpp>
#include <cstdint>
#include <kernel/memory/vmm.hpp>
#include <kstd/kstring.hpp>

typedef struct {
    char anchor_string[4];
    uint8_t ep_checksum;
    uint8_t ep_length;
    uint8_t smbios_major_ver;
    uint8_t smbios_minor_ver;
    uint16_t structure_max_size;
    uint8_t ep_revision;
    uint8_t formatted_area[5];
    uint8_t intermediate_anchor_string[5];
    uint8_t intermediate_checksum;
    uint16_t structure_table_length;
    uint32_t structure_table_address;
    uint16_t number_of_structures;
    uint8_t bcd_revision;
} SMBIOS_32bit;

typedef uint16_t SMBIOS_HANDLE;

typedef struct {
    uint8_t type;
    uint8_t length;
    SMBIOS_HANDLE handle;
} SMBIOS_Tag;

typedef struct {
    SMBIOS_Tag tag;
    SMBIOS_HANDLE physical_mem_array_handle;
    SMBIOS_HANDLE memory_error_information_handle;
    uint16_t total_width;
    uint16_t data_width;
    uint16_t size;
    uint8_t form_factor;
    uint8_t device_set;
    uint8_t device_locator;
    uint8_t bank_locator;
    uint8_t memory_type;
    uint16_t type_detail;
    uint16_t speed;
    uint8_t manufacturer;
    uint8_t serial_number;
    uint8_t asset_tag;
    uint8_t part_number;
    uint8_t attributes;
    uint32_t extended_size;
    uint16_t configured_memory_speed;
    uint16_t minimum_voltage;
    uint16_t maximum_voltage;
    uint16_t configured_voltage;
    uint8_t memory_technology;
    uint16_t memory_operatin_mode_capability;
    uint8_t firmware_version;
    uint16_t module_manufacturer_id;
    uint16_t module_product_id;
    uint16_t memory_subsystem_controller_manufacturer_id;
    uint16_t memory_subsystem_controller_product_id;
    uint64_t non_volatile_size;
    uint64_t volatile_size;
    uint64_t cache_size;
    uint64_t logical_size;
    uint32_t extended_speed;
    uint32_t extended_configured_memory_speed;
    uint16_t pmic0_manufacturer_id;
    uint16_t pmic0_revision_number;
    uint16_t rcd_manufacturer_id;
    uint16_t rcd_revision_number;
} SMBIOS_MemoryDevice;

enum MemoryDeviceType {
    TypeOther = 0x01,
    TypeUnknown = 0x02,
    TypeDRAM = 0x03,
    TypeEDRAM = 0x04,
    TypeVRAM = 0x05,
    TypeSRAM = 0x06,
    TypeRAM = 0x07,
    TypeROM = 0x08,
    TypeFLASH = 0x09,
    TypeEEPROM = 0x0A,
    TypeFEPROM = 0x0B,
    TypeEPROM = 0x0C,
    TypeCDRAM = 0x0D,
    Type3DRAM = 0x0E,
    TypeSDRAM = 0x0F,
    TypeSGRAM = 0x10,
    TypeRDRAM = 0x11,
    TypeDDR = 0x12,
    TypeDDR2 = 0x13,
    TypeDDR2FBDIMM = 0x14,
    TypeDDR3 = 0x18,
    TypeFBD2 = 0x19,
    TypeDDR4 = 0x1A,
    TypeLPDDR = 0x1B,
    TypeLPDDR2 = 0x1C,
    TypeLPDDR3 = 0x1D,
    TypeLPDDR4 = 0x1E,
    TypeLogicalNonVolatileDevice = 0x1F,
    TypeHBM = 0x20,
    TypeHBM2 = 0x21,
    TypeDDR5 = 0x22,
    TypeLPDDR5 = 0x23,
    TypeHBM3 = 0x24
};

constexpr uint64_t smbios_convert_memory_size(uint16_t size) {
    return (size == 0xFFFF) ? 0 :
    (size == 0x7FFF) ? 0 :
    ((size & 0x8000) != 0) ? static_cast<uint64_t>(size & 0x7FFF) * 1024 :
    static_cast<uint64_t>(size & 0x7FFF) * 1024 * 1024;
}

constexpr const char* memory_device_type_to_string(uint8_t mdt) {
    switch (mdt) {
        case TypeOther: return "Other";
        case TypeUnknown: return "Unknown";
        case TypeDRAM: return "DRAM";
        case TypeEDRAM: return "EDRAM";
        case TypeVRAM: return "VRAM";
        case TypeSRAM: return "SRAM";
        case TypeRAM: return "RAM";
        case TypeROM: return "ROM";
        case TypeFLASH: return "FLASH";
        case TypeEEPROM: return "EEPROM";
        case TypeFEPROM: return "FEPROM";
        case TypeEPROM: return "EPROM";
        case TypeCDRAM: return "CDRAM";
        case Type3DRAM: return "3DRAM";
        case TypeSDRAM: return "SDRAM";
        case TypeSGRAM: return "SGRAM";
        case TypeRDRAM: return "RDRAM";
        case TypeDDR: return "DDR";
        case TypeDDR2: return "DDR2";
        case TypeDDR2FBDIMM: return "DDR2 FB-DIMM";
        case TypeDDR3: return "DDR3";
        case TypeFBD2: return "FBD2";
        case TypeDDR4: return "DDR4";
        case TypeLPDDR: return "LPDDR";
        case TypeLPDDR2: return "LPDDR2";
        case TypeLPDDR3: return "LPDDR3";
        case TypeLPDDR4: return "LPDDR4";
        case TypeLogicalNonVolatileDevice: return "Logical Non-Volatile Device";
        case TypeHBM: return "HBM";
        case TypeHBM2: return "HBM2";
        case TypeDDR5: return "DDR5";
        case TypeLPDDR5: return "LPDDR5";
        case TypeHBM3: return "HBM3";
        default: return "Unknown";
    }
}

constexpr const char* smbios_entry_type_to_string(uint8_t type) {
    switch (type) {
        case 0:
            return "BIOS Information";
        case 1:
            return "System Information";
        case 2:
            return "Baseboard (or Module) Information";
        case 3:
            return "System Enclosure or Chassis";
        case 4:
            return "Processor Information";
        case 5:
            return "Memory Controller Information (Obsolete)";
        case 6:
            return "Memory Module Information (Obsolete)";
        case 7:
            return "Cache Information";
        case 8:
            return "Port Connector Information";
        case 9:
            return "System Slots";
        case 10:
            return "On Board Devices Information (Obsolete)";
        case 11:
            return "OEM Strings";
        case 12:
            return "System Configuration Options";
        case 13:
            return "BIOS Language Information";
        case 14:
            return "Group Associations";
        case 15:
            return "System Event Log";
        case 16:
            return "Physical Memory Array";
        case 17:
            return "Memory Device";
        case 18:
            return "32-Bit Memory Error Information";
        case 19:
            return "Memory Array Mapped Address";
        case 20:
            return "Memory Device Mapped Address";
        case 21:
            return "Built-in Pointing Device";
        case 22:
            return "Portable Battery";
        case 23:
            return "System Reset";
        case 24:
            return "Hardware Security";
        case 25:
            return "System Power Controls";
        case 26:
            return "Voltage Probe";
        case 27:
            return "Cooling Device";
        case 28:
            return "Temperature Probe";
        case 29:
            return "Electrical Current Probe";
        case 30:
            return "Out-of-Band Remote Access";
        case 31:
            return "Boot Integrity Services (BIS) Entry Point";
        case 32:
            return "System Boot Information";
        case 33:
            return "64-Bit Memory Error Information";
        case 34:
            return "Management Device";
        case 35:
            return "Management Device Component";
        case 36:
            return "Management Device Threshold Data";
        case 37:
            return "Memory Channel";
        case 38:
            return "IPMI Device Information";
        case 39:
            return "System Power Supply";
        case 40:
            return "Additional Information";
        case 41:
            return "Onboard Devices Extended Information";
        case 42:
            return "Management Controller Host Interface";
        case 43:
            return "TPM Device";
        case 44:
            return "Processor Additional Information";
        case 45:
            return "Firmware Inventory Information";
        case 46:
            return "String Property";
        case 126:
            return "Inactive";
        case 127:
            return "End-Of-Table";
        default:
            return "Unknown entry type";
    }
}

typedef struct {
    SMBIOS_Tag tag;
    uint8_t rsvd[6];
    uint8_t boot_status; // This size fucking varies.
} SMBIOS_SystemBootInformation;

void smbios_init();
void* smbios_get_entry(size_t idx);
size_t smbios_get_structure_count();

void smbios_dump_info();

#endif //KITTY_OS_CPP_SMBIOS_HPP
