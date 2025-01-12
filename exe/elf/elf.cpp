#include <algorithm>
#include <cstring>
#include <debug.hpp>
#include <exe/elf/elf.hpp>
#include <mem/physical_memory.hpp>
#include <mem/virtual_memory.hpp>
#include <sys/symbols.hpp>

// Function to parse ELF raw data into the elf_object structure
elf_object *parse_elf_data(void *raw_data, size_t data_size, bool make_kernel)
{
    elf_object *elf_obj = new elf_object;
    if (!elf_obj)
    {
        debug_printf("Memory allocation failed for elf_object\n");
        return nullptr;
    }

    elf_obj->file_data = raw_data;
    elf_obj->file_size = data_size;

    // Parse the ELF header
    elf_obj->elf_header64 = (Elf64_Ehdr *)raw_data;
    if (elf_obj->elf_header64->e_ident[EI_MAG0] != ELFMAG0 || elf_obj->elf_header64->e_ident[EI_MAG1] != ELFMAG1 ||
        elf_obj->elf_header64->e_ident[EI_MAG2] != ELFMAG2 || elf_obj->elf_header64->e_ident[EI_MAG3] != ELFMAG3)
    {
        debug_printf("Invalid ELF magic number\n");
        delete elf_obj;
        return nullptr;
    }

    // Parse section headers
    elf_obj->section_headers64 = (Elf64_Shdr *)((char *)raw_data + elf_obj->elf_header64->e_shoff);

    // Parse program headers
    elf_obj->program_headers64 = (Elf64_Phdr *)((char *)raw_data + elf_obj->elf_header64->e_phoff);

    // Parse the section string table
    Elf64_Shdr &strtab_section = elf_obj->section_headers64[elf_obj->elf_header64->e_shstrndx];
    elf_obj->section_string_table = (char *)raw_data + strtab_section.sh_offset;

    // Parse the symbol table
    elf_obj->symbol_table64 = nullptr;
    elf_obj->num_symbols = 0;
    for (size_t i = 0; i < elf_obj->elf_header64->e_shnum; ++i)
    {
        Elf64_Shdr &section = elf_obj->section_headers64[i];
        if (section.sh_type == SHT_SYMTAB || section.sh_type == SHT_DYNSYM)
        {
            elf_obj->symbol_table64 = (Elf64_Sym *)((char *)raw_data + section.sh_offset);
            elf_obj->num_symbols = section.sh_size / sizeof(Elf64_Sym);

            // Correctly get the string table for symbol names
            Elf64_Shdr &symtab_string_section = elf_obj->section_headers64[section.sh_link];
            elf_obj->string_table = (char *)raw_data + symtab_string_section.sh_offset;
            break;
        }
    }

    // Parse relocation table
    elf_obj->relocation_table64 = nullptr;
    elf_obj->num_relocations = 0;
    for (size_t i = 0; i < elf_obj->elf_header64->e_shnum; ++i)
    {
        Elf64_Shdr &section = elf_obj->section_headers64[i];
        if (section.sh_type == SHT_RELA || section.sh_type == SHT_REL)
        {
            elf_obj->relocation_table64 = (Elf64_Rel *)((char *)raw_data + section.sh_offset);
            elf_obj->num_relocations = section.sh_size / sizeof(Elf64_Rel);
            break;
        }
    }

    // Parse dynamic section (for dynamically linked ELF files)
    elf_obj->dynamic_section64 = nullptr;
    elf_obj->is_dynamic = false;
    for (size_t i = 0; i < elf_obj->elf_header64->e_phnum; ++i)
    {
        Elf64_Phdr &phdr = elf_obj->program_headers64[i];
        if (phdr.p_type == PT_DYNAMIC)
        {
            elf_obj->dynamic_section64 = (Elf64_Dyn *)((char *)raw_data + phdr.p_offset);
            elf_obj->is_dynamic = true;
            break;
        }
    }

    elf_obj->is_kernel_module = false;
    if (elf_obj->elf_header64->e_type == ET_DYN && make_kernel)
    {
        elf_obj->is_kernel_module = true;
    }

    return elf_obj;
}

// Function to dump ELF object information for debugging purposes
void dump_elf_object_info(const elf_object *elf_obj)
{
    if (!elf_obj)
    {
        debug_printf("Invalid ELF object\n");
        return;
    }

    // ELF Header Information
    debug_printf("ELF Header:\n");
    debug_printf("  Magic: %c%c%c%c\n", elf_obj->elf_header64->e_ident[EI_MAG0],
                 elf_obj->elf_header64->e_ident[EI_MAG1], elf_obj->elf_header64->e_ident[EI_MAG2],
                 elf_obj->elf_header64->e_ident[EI_MAG3]);
    debug_printf("  Type: %u\n", elf_obj->elf_header64->e_type);
    debug_printf("  Machine: %u\n", elf_obj->elf_header64->e_machine);
    debug_printf("  Version: %u\n", elf_obj->elf_header64->e_version);
    debug_printf("  Entry point: 0x%lx\n", elf_obj->elf_header64->e_entry);
    debug_printf("  Program header offset: 0x%lx\n", elf_obj->elf_header64->e_phoff);
    debug_printf("  Section header offset: 0x%lx\n", elf_obj->elf_header64->e_shoff);
    debug_printf("  Flags: 0x%x\n", elf_obj->elf_header64->e_flags);
    debug_printf("  ELF header size: %u\n", elf_obj->elf_header64->e_ehsize);

    // Section Headers Information
    debug_printf("\nSection Headers:\n");
    for (size_t i = 0; i < elf_obj->elf_header64->e_shnum; ++i)
    {
        Elf64_Shdr &section = elf_obj->section_headers64[i];
        debug_printf("  Section %zu:\n", i);
        debug_printf("    Name: %s\n", elf_obj->section_string_table + section.sh_name);
        debug_printf("    Type: %u\n", section.sh_type);
        debug_printf("    Flags: 0x%lx\n", section.sh_flags);
        debug_printf("    Address: 0x%lx\n", section.sh_addr);
        debug_printf("    Offset: 0x%lx\n", section.sh_offset);
        debug_printf("    Size: 0x%lx\n", section.sh_size);
        debug_printf("    Link: %u\n", section.sh_link);
        debug_printf("    Info: %u\n", section.sh_info);
        debug_printf("    Address alignment: 0x%lx\n", section.sh_addralign);
        debug_printf("    Entry size: 0x%lx\n", section.sh_entsize);
    }

    // Program Headers Information
    debug_printf("\nProgram Headers:\n");
    for (size_t i = 0; i < elf_obj->elf_header64->e_phnum; ++i)
    {
        Elf64_Phdr &phdr = elf_obj->program_headers64[i];
        debug_printf("  Program Header %zu:\n", i);
        debug_printf("    Type: %u\n", phdr.p_type);
        debug_printf("    Offset: 0x%lx\n", phdr.p_offset);
        debug_printf("    Virtual address: 0x%lx\n", phdr.p_vaddr);
        debug_printf("    Physical address: 0x%lx\n", phdr.p_paddr);
        debug_printf("    File size: 0x%lx\n", phdr.p_filesz);
        debug_printf("    Memory size: 0x%lx\n", phdr.p_memsz);
        debug_printf("    Flags: 0x%x\n", phdr.p_flags);
        debug_printf("    Alignment: 0x%lx\n", phdr.p_align);
    }

    // Symbol Table Information
    if (elf_obj->symbol_table64 && elf_obj->num_symbols > 0)
    {
        debug_printf("\nSymbol Table:\n");

        for (size_t i = 0; i < elf_obj->num_symbols; ++i)
        {
            Elf64_Sym &sym = elf_obj->symbol_table64[i];

            // Correctly retrieve the symbol name from the string table
            const char *symbol_name = (sym.st_name != 0) ? (elf_obj->string_table + sym.st_name) : "<unnamed>";

            debug_printf("  Symbol %zu:\n", i);
            debug_printf("    Name: %s\n", symbol_name);
            debug_printf("    Value: 0x%lx\n", sym.st_value);
            debug_printf("    Size: %u\n", sym.st_size);
            debug_printf("    Info: 0x%x\n", sym.st_info);
            debug_printf("    Other: 0x%x\n", sym.st_other);
            debug_printf("    Section index: %u\n", sym.st_shndx);
        }
    }

    // Relocation Table Information
    if (elf_obj->relocation_table64 && elf_obj->num_relocations > 0)
    {
        debug_printf("\nRelocation Table:\n");
        for (size_t i = 0; i < elf_obj->num_relocations; ++i)
        {
            Elf64_Rel &rel = elf_obj->relocation_table64[i];
            debug_printf("  Relocation %zu:\n", i);
            debug_printf("    Offset: 0x%lx\n", rel.r_offset);
            debug_printf("    Info: 0x%lx\n", rel.r_info);
        }
    }
}

// Function to calculate the maximum virtual memory needed for an ELF file
size_t calculate_max_virtual_memory(const elf_object *elf_obj)
{
    if (!elf_obj)
    {
        debug_printf("Invalid ELF object\n");
        return 0;
    }

    size_t max_virtual_memory = 0;

    // Iterate through the program headers to find the maximum memory size
    for (size_t i = 0; i < elf_obj->elf_header64->e_phnum; ++i)
    {
        Elf64_Phdr &phdr = elf_obj->program_headers64[i];

        // Check if the segment type is loadable (PT_LOAD)
        if (phdr.p_type == PT_LOAD)
        {
            size_t segment_end = phdr.p_vaddr + phdr.p_memsz;
            max_virtual_memory = std::max(max_virtual_memory, segment_end);
        }
    }

    return max_virtual_memory;
}

// Function to find a symbol in the module's symbol table
uintptr_t find_symbol_in_module(const char *symbol_name, elf_object *elf_obj)
{
    if (!elf_obj || !elf_obj->symbol_table64 || elf_obj->num_symbols == 0)
    {
        debug_printf("Early returning: symbol table 64 is nullptr or num symbols is 0.");
        return 0;  // No symbols in the module
    }

    // Search the module's symbol table for the symbol
    for (size_t i = 0; i < elf_obj->num_symbols; ++i)
    {
        Elf64_Sym &symbol = elf_obj->symbol_table64[i];
        const char *name = (symbol.st_name != 0) ? (elf_obj->string_table + symbol.st_name) : nullptr;

        if (name && strcmp(name, symbol_name) == 0)
        {
            return symbol.st_value + elf_obj->module_base_address;  // Found the symbol in the module
        }
    }

    return 0;  // Symbol not found in the module
}

// Function to resolve a symbol from the module's symbol table first, then fallback to the kernel's symbol table
uintptr_t resolve_symbol(const char *symbol_name, elf_object *elf_obj)
{

    uintptr_t symbol_addr;
    
    symbol_addr = find_symbol_by_name(symbol_name);
    if (symbol_addr != 0)
    {
        return symbol_addr;  // Return the address if found in the module
    }

    symbol_addr = find_symbol_in_module(symbol_name, elf_obj);

    if (!symbol_addr)
    {
        debug_printf("No symbol found: %s\n", symbol_name);
        asm volatile ("cli; hlt");
    }

    return symbol_addr;
}


extern char symbols_start;
extern char symbols_end;

// Function to load ELF kernel module
void load_elf_kernel_module(elf_object *elf_obj)
{
    if (!elf_obj)
    {
        debug_printf("Invalid ELF object or missing dynamic section\n");
        return;
    }

    debug_printf("Loading ELF Kernel Module...\n");

    auto max_vmem = calculate_max_virtual_memory(elf_obj);
    // align it
    max_vmem = (max_vmem + 4095) & (~4095);

    debug_printf("Virtual memory needed: %zd\n", max_vmem);

    auto virtual_address_base = allocate_virtual_memory_kernel(max_vmem);
    elf_obj->module_base_address = reinterpret_cast<void *>(virtual_address_base);
    debug_printf("Allocated at: %p\n", elf_obj->module_base_address);

    for (size_t i = 0; max_vmem / 4096 > i; i++)
    {
        auto page = allocate_page();
        if (!page)
        {
            debug_printf("Failed to allocate the page in loading kernel module.\n");
            asm volatile("cli; hlt");
        }

        bool resp =
            virtual_map(page, virtual_address_base + (i * 4096), true, false, true, false, false, false, 0, true);

        if (!resp)
        {
            debug_printf("Failed to map the page to the kernel module's virtual space.\n");
            asm volatile("cli; hlt");
        }
    }

    // ahhh, time to load sections!
    // Load sections into memory
    debug_printf("Loading ELF sections into memory...\n");
    for (size_t i = 0; i < elf_obj->elf_header64->e_shnum; ++i)
    {
        Elf64_Shdr &section = elf_obj->section_headers64[i];

        if (section.sh_flags & SHF_ALLOC)
        {
            // Allocate memory for this section
            void *dest_address = reinterpret_cast<void *>(virtual_address_base + section.sh_addr);
            void *src_address =
                reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(elf_obj->file_data) + section.sh_offset);

            // If the section has data, copy it; otherwise, zero-fill
            if (section.sh_size > 0)
            {
                if (section.sh_type != SHT_NOBITS)
                {
                    debug_printf("Copying section %s to memory at %p\n",
                                 elf_obj->section_string_table + section.sh_name, dest_address);
                    std::memcpy(dest_address, src_address, section.sh_size);
                }
                else
                {
                    debug_printf("Zero-filling section %s at memory %p\n",
                                 elf_obj->section_string_table + section.sh_name, dest_address);
                    std::memset(dest_address, 0, section.sh_size);
                }
            }
        }
    }

    // Handle relocations for .rela.dyn section
    debug_printf("\nHandling Relocations from .rela.dyn:\n");

    // Look for the .rela.dyn section (relocations for dynamic symbols)
    Elf64_Shdr &rela_dyn_section = elf_obj->section_headers64[elf_obj->elf_header64->e_shstrndx];
    for (size_t i = 0; i < elf_obj->elf_header64->e_shnum; ++i)
    {
        Elf64_Shdr &section = elf_obj->section_headers64[i];

        // Check if the section is the .rela.dyn section
        if (section.sh_type == SHT_RELA && strcmp(elf_obj->section_string_table + section.sh_name, ".rela.dyn") == 0)
        {
            // The section is .rela.dyn
            Elf64_Rela *relocation_table = (Elf64_Rela *)((char *)elf_obj->file_data + section.sh_offset);
            size_t num_relocations = section.sh_size / sizeof(Elf64_Rela);

            // Process all relocations in this section
            for (size_t j = 0; j < num_relocations; ++j)
            {
                Elf64_Rela &rela = relocation_table[j];

                // Switch based on relocation type (add more cases as necessary)
                switch (ELF64_R_TYPE(rela.r_info))
                {
                case R_X86_64_JUMP_SLOT: {
                    // This is the JUMP_SLOT relocation type (0x7)
                    debug_printf("  Handling R_X86_64_JUMP_SLOT relocation at offset 0x%lx\n", rela.r_offset);

                    // Get the symbol index from the relocation entry
                    unsigned int symbol_index = ELF64_R_SYM(rela.r_info);

                    // Get the symbol from the symbol table using the symbol index
                    Elf64_Sym &symbol = elf_obj->symbol_table64[symbol_index];

                    // Retrieve the symbol name from the string table
                    const char *symbol_name =
                        (symbol.st_name != 0) ? (elf_obj->string_table + symbol.st_name) : "<unnamed>";

                    uintptr_t *got_entry =
                        (uintptr_t *)(rela.r_offset + reinterpret_cast<uintptr_t>(elf_obj->module_base_address));

                    // Display symbol name and GOT entry location
                    debug_printf("    Symbol: %s\n", symbol_name);
                    debug_printf("    GOT entry at 0x%lx\n", (uintptr_t)got_entry);

                    *got_entry = resolve_symbol(symbol_name, elf_obj);

                    break;
                }

                default:
                    debug_printf("  Unknown relocation type: 0x%lx\n", ELF64_R_TYPE(rela.r_info));
                    break;
                }
            }
            break; // We've processed .rela.dyn, exit loop
        }
    }
}

EXPORT_SYMBOL(parse_elf_data);
EXPORT_SYMBOL(dump_elf_object_info);
EXPORT_SYMBOL(load_elf_kernel_module);
