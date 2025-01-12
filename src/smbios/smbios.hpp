#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// SMBIOS 2.1 (32-bit) Entry Point Structure
struct smbios_entry_point_32
{
    char anchor_string[4];              // Offset 00h: _SM_ (4 ASCII characters)
    uint8_t eps_checksum;               // Offset 04h: Checksum of the Entry Point Structure (EPS)
    uint8_t eps_length;                 // Offset 05h: Length of the Entry Point Structure in bytes
    uint8_t smbios_major_version;       // Offset 06h: SMBIOS major version
    uint8_t smbios_minor_version;       // Offset 07h: SMBIOS minor version
    uint16_t max_structure_size;        // Offset 08h: Maximum structure size in bytes
    uint8_t eps_revision;               // Offset 0Ah: Entry Point revision
    uint8_t formatted_area[5];          // Offset 0Bh: Reserved/Formatted Area
    char intermediate_anchor_string[5]; // Offset 10h: _DMI_ (5 ASCII characters)
    uint8_t intermediate_checksum;      // Offset 15h: Checksum for the Intermediate Entry Point Structure (IEPS)
    uint16_t structure_table_length;    // Offset 16h: Length of the SMBIOS structure table
    uint32_t structure_table_address;   // Offset 18h: Physical address of the structure table
    uint16_t num_smbios_structures;     // Offset 1Ch: Number of SMBIOS structures
    uint8_t smbios_bcd_revision;        // Offset 1Eh: SMBIOS revision in BCD format
} __attribute__((packed));

// SMBIOS 3.0 (64-bit) Entry Point Structure
struct smbios_entry_point_64
{
    char anchor_string[5];             // Offset 00h: _SM3_ (5 ASCII characters)
    uint8_t eps_checksum;              // Offset 05h: Checksum of the Entry Point Structure (EPS)
    uint8_t eps_length;                // Offset 06h: Length of the Entry Point Structure in bytes
    uint8_t smbios_major_version;      // Offset 07h: SMBIOS major version
    uint8_t smbios_minor_version;      // Offset 08h: SMBIOS minor version
    uint8_t smbios_docrev;             // Offset 09h: SMBIOS document revision
    uint8_t eps_revision;              // Offset 0Ah: Entry Point revision
    uint8_t reserved;                  // Offset 0Bh: Reserved, set to 0
    uint32_t structure_table_max_size; // Offset 0Ch: Maximum size of SMBIOS Structure Table in bytes
    uint64_t structure_table_address;  // Offset 10h: 64-bit physical starting address of the SMBIOS Structure Table
} __attribute__((packed));

// SMBIOS Structure Header
struct smbios_structure_header
{
    uint8_t type;    // Offset 00h: Type of structure
    uint8_t length;  // Offset 01h: Length of the formatted area
    uint16_t handle; // Offset 02h: Unique handle for the structure
} __attribute__((packed));

// Helper macros for DMI validation
#define SMBIOS_ANCHOR_STRING "_SM_"
#define DMI_ANCHOR_STRING "_DMI_"

void initialize_smbios();

// Function prototypes
bool validate_smbios_entry_point(const struct smbios_entry_point_32 *entry_point);
bool validate_smbios_dmi_data(const struct smbios_entry_point_32 *entry_point, const void *dmi_table,
                              size_t table_size);
bool validate_smbios_entry_point_64(const struct smbios_entry_point_64 *entry_point);
void print_smbios_entries();
void print_smbios_string_table(const smbios_structure_header *header, size_t table_size);
const char *get_smbios_string(const smbios_structure_header *header, size_t table_size, size_t string_index);
