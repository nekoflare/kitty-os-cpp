#pragma once

#include <elf.h>
#include <stddef.h>
#include <stdint.h>

struct elf_object
{
    // ELF Header Information
    Elf64_Ehdr *elf_header64; // ELF header (for 64-bit ELF)

    // Section Headers
    Elf64_Shdr *section_headers64;

    // Program Headers
    Elf64_Phdr *program_headers64;

    // String Tables (for section names, symbol names, etc.)
    char *section_string_table; // Section string table
    char *string_table;         // Symbol string table

    // Section Data (cached section content)
    void *sections_data; // Holds the data of sections for quick access

    // Symbols and Symbol Tables (for dynamic linking)
    Elf64_Sym *symbol_table64; // For ELF64
    uint32_t num_symbols;      // Number of symbols in the symbol table

    // Relocations (for dynamic linking)
    Elf64_Rel *relocation_table64; // Relocation table for 64-bit ELF
    uint32_t num_relocations;      // Number of relocations

    // Dynamic Section (for dynamic ELF, e.g., shared libraries)
    Elf64_Dyn *dynamic_section64; // For ELF64

    // File-related info (for file I/O management)
    void *file_data;  // Pointer to the raw ELF file data
    size_t file_size; // ELF file size

    // Dynamic Loader State (for user-space)
    bool is_dynamic;            // True if ELF is dynamically linked
    void *dynamic_loader_state; // State related to dynamic linking (e.g., address of GOT)

    // Kernel-related state
    bool is_kernel_module;     // Flag if it's a kernel module
    void *module_base_address; // Base address of the loaded module
};

elf_object *parse_elf_data(void *raw_data, size_t data_size, bool make_kernel);
void dump_elf_object_info(const elf_object *elf_obj);
void load_elf_kernel_module(elf_object *elf_obj);