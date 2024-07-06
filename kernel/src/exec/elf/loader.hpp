
//
// Created by Piotr on 31.05.2024.
//

#ifndef KITTY_OS_CPP_LOADER_HPP
#define KITTY_OS_CPP_LOADER_HPP

#include <mm/vmm.hpp>
#include <mm/pmm.hpp>

extern "C" void elf_trampoline(uint64_t new_pml4e, uint64_t new_address);

struct elf_object_t
{
    void* elf_bin;
    size_t elf_bin_size;
    pml4e* pml4e_dir;
    uint64_t pml4e_dir_physical;
    uint64_t start;
};

/// Create new ELF object.
elf_object_t* elf_create_object(void* elf_bin, size_t elf_bin_size);

/// Load the object to the memory.
void elf_load_object(elf_object_t* obj);

/// Execute the object.
void elf_invoke_object(elf_object_t* obj);

/// Destroy the object.
void elf_destroy_object(elf_object_t* obj);

constexpr uint8_t elf_magic[4] = {0x7F, 'E', 'L', 'F'};

/// ELF header
struct elf_header_64
{
    uint8_t magic[4];
    uint8_t version; // 1 = 32-bit, 2 = 64-bit
    uint8_t endianess; // 1 - little endian 2 - big endian
    uint8_t elf_hdr_version;
    uint8_t os_abi; // 0 for sysv
    uint8_t padding[8];
    uint16_t type; // 1 - relocatable, 2 - exectuable, 3 - shared, 4 - core
    uint16_t instruction_set;
    uint32_t elf_version; // must be 1
    uint64_t program_entry_offset;
    uint64_t program_header_table_offset;
    uint64_t section_header_table_offset;
    uint32_t flags;
    uint16_t elf_hdr_size;
    uint16_t sizeof_entry_in_program_hdr_table;
    uint16_t program_hdr_entry_count;
    uint16_t sizeof_entry_in_section_hdr_table;
    uint16_t section_hdr_entry_count;
    uint16_t section_idx_to_hdr_string_table;
} __attribute__((packed));

struct elf_segment
{
    uint32_t type;           // Position 0-3
    uint32_t flags;          // Position 4-7
    uint64_t p_offset;       // Position 8-15
    uint64_t p_vaddr;        // Position 16-23
    uint64_t p_paddr;        // Position 24-31
    uint64_t p_filesz;       // Position 32-39
    uint64_t p_memsz;        // Position 40-47
    uint64_t alignment;      // Position 48-55
} __attribute__((packed));

struct elf_section_header {
    uint32_t name_offset;            // Offset in bytes to a string in the .shstrtab section
    uint32_t type;                   // Section type (SHT_*)
    uint64_t flags;                  // Section flags (SHF_*)
    uint64_t virtual_address;        // Address in memory
    uint64_t offset;                 // Offset in the file
    uint64_t size;                   // Size of the section
    uint32_t link;                   // Link to another section
    uint32_t info;                   // Additional information
    uint64_t alignment;              // Required alignment
    uint64_t entry_size;             // Size of each entry if the section holds a table
};

#endif //KITTY_OS_CPP_LOADER_HPP