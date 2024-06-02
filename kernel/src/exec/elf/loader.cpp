//
// Created by Piotr on 31.05.2024.
//

#include "loader.hpp"

elf_object_t* elf_create_object(void* elf_bin, size_t elf_bin_size)
{
    auto obj = new elf_object_t;

    obj->elf_bin = elf_bin;
    obj->elf_bin_size = elf_bin_size;

    // Allocate new PML4e.
    uint64_t pml4e_pa = pmm_alloc_page();
    auto pml4e_va = vmm_make_virtual<uint8_t*>(pml4e_pa);

    obj->pml4e_dir_physical = pml4e_pa;

    // Clear pml4e
    for (size_t i = 0; 4096 > i; i++)
        pml4e_va[i] = 0x0;

    // Copy last 256 entries of kernel pml4e to this pml4e.
    pml4e* kernel_pml4e = vmm_make_virtual<pml4e*>(vmm_get_pml4());
    pml4e* program_pml4e = reinterpret_cast<pml4e*>(pml4e_va);

    for (size_t i = 0; 512 > i; i++)
    {
        program_pml4e[i] = kernel_pml4e[i];
    }

    kstd::printf("Copied PML4e over to program's PML4e.\n");

    obj->pml4e_dir = program_pml4e;

    return obj;
}

void elf_move_data(elf_object_t* obj, uint64_t virtual_address, void* data, size_t len)
{
    size_t pages = (len + 4095) / PAGE_SIZE; // Calculate number of pages needed
    kstd::printf("required pages: %llx\n", pages);
    kstd::printf("%llx -> %llx (%lld)\n", data, virtual_address, len);

    if (pages == 0)
    {
        kstd::printf("ignoring this entry\n");
        return;
    }

    uint64_t original_virtual_address = virtual_address; // Save the starting virtual address

    vmm_address va = vmm_split_va(virtual_address);
    size_t pml4e = va.pml4e;
    size_t pdpe = va.pdpe;
    size_t pde = va.pde;
    size_t pte = va.pte;
    size_t off = va.offset;

    for (size_t i = 0; i < pages; i++)
    {
        uint64_t page = pmm_alloc_page();

        vmm_map(obj->pml4e_dir, virtual_address, page, PROT_RW | PROT_SUPERVISOR, MAP_PRESENT, MISC_INVLPG);

        pte++;
        if (pte >= 512)
        {
            pte = 0;
            pde++;
        }
        if (pde >= 512)
        {
            pde = 0;
            pdpe++;
        }
        if (pdpe >= 512)
        {
            pdpe = 0;
            pml4e++;
        }

        va.pml4e = pml4e;
        va.pdpe = pdpe;
        va.pde = pde;
        va.pte = pte;
        va.offset = off;

        virtual_address = vmm_sva_to_va(va);
        kstd::printf("new VA: %llx\n", virtual_address);
    }

    // Move the memory now.
    kstd::printf("\n\n%hhx\n\n", *(uint8_t*)data);
    bochs_breakpoint();

    auto d = reinterpret_cast<uint8_t*>(data);
    auto v = reinterpret_cast<uint8_t*>(original_virtual_address); // Use the original virtual address
    for (size_t i = 0; i < len; i++)
    {
        // bochs_breakpoint();
        v[i] = d[i];
        if (d[i]) kstd::printf("%llx: %hhx ",v+i, v[i]);
        if (v[i] != d[i])
        {
            kstd::printf("Move failure.\n");
        }
    }

    kstd::printf("Moved %lld bytes to %llx from %llx.\n", len, original_virtual_address, data);
    //bochs_breakpoint();
}

void elf_load_object(elf_object_t* obj) {
    kstd::printf("ELF Header:\n");
    elf_header_64* hdr = reinterpret_cast<elf_header_64*>(obj->elf_bin);

    kstd::printf("  Magic:   ");
    for (int i = 0; i < 16; ++i) {
        kstd::printf("%hhx ", hdr->magic[i]);
    }
    kstd::printf("\n");

    kstd::printf("  Class:                             ELF64\n");
    kstd::printf("  Data:                              2's complement, little endian\n");
    kstd::printf("  Version:                           %d (current)\n", hdr->elf_hdr_version);
    kstd::printf("  OS/ABI:                            UNIX - System V\n");
    kstd::printf("  ABI Version:                       %d\n", hdr->os_abi);
    kstd::printf("  Type:                              ");
    switch (hdr->type) {
        case 1: kstd::printf("REL (Relocatable file)\n"); break;
        case 2: kstd::printf("EXEC (Executable file)\n"); break;
        case 3: kstd::printf("DYN (Shared object file)\n"); break;
        case 4: kstd::printf("CORE (Core file)\n"); break;
        default: kstd::printf("Unknown\n"); break;
    }
    kstd::printf("  Machine:                           Advanced Micro Devices X86-64\n");
    kstd::printf("  Version:                           0x%x\n", hdr->elf_version);
    kstd::printf("  Entry point address:               0x%llx\n", hdr->program_entry_offset);
    kstd::printf("  Start of program headers:          %lld (bytes into file)\n", hdr->program_header_table_offset);
    kstd::printf("  Start of section headers:          %lld (bytes into file)\n", hdr->section_header_table_offset);
    kstd::printf("  Flags:                             0x%x\n", hdr->flags);
    kstd::printf("  Size of this header:               %d (bytes)\n", hdr->elf_hdr_size);
    kstd::printf("  Size of program headers:           %d (bytes)\n", hdr->sizeof_entry_in_program_hdr_table);
    kstd::printf("  Number of program headers:         %d\n", hdr->program_hdr_entry_count);
    kstd::printf("  Size of section headers:           %d (bytes)\n", hdr->sizeof_entry_in_section_hdr_table);
    kstd::printf("  Number of section headers:         %d\n", hdr->section_hdr_entry_count);
    kstd::printf("  Section header string table index: %d\n", hdr->section_idx_to_hdr_string_table);

    if (hdr->endianess != 1) {
        kstd::printf("Endianness is wrong.\n");
        return;
    }

    if (hdr->os_abi != 0) {
        kstd::printf("OS abi isn't SYS-V.\n");
        return;
    }

    if (hdr->type != 2) {
        kstd::printf("This is not an executable!.\n");
        kstd::printf("Type is: %hx\n", hdr->type);
        return;
    }

    if (hdr->elf_version != 1) {
        kstd::printf("Elf version isn't equal to 1.\n");
        return;
    }

    uint64_t program_entry_size = hdr->sizeof_entry_in_program_hdr_table;
    uint64_t program_entry_file_offset = hdr->program_header_table_offset;
    uint64_t program_entry_count = hdr->program_hdr_entry_count;

    // Calculate the size of a program header entry
    size_t program_hdr_size = sizeof(elf_segment);

    obj->start = hdr->program_entry_offset;


    // Iterate through each program header entry
    for (size_t i = 0; i < program_entry_count; i++) {
        // Calculate the offset of the current program header entry
        uint64_t entry_offset = program_entry_file_offset + i * program_entry_size;

        // Retrieve the program header entry
        auto program_entry = reinterpret_cast<elf_segment*>(obj->elf_bin + entry_offset);

        // Print the segment type, virtual address, and segment length
        //kstd::printf("Segment %lld:\n", i);
        //kstd::printf("\tSegment Type: %u\n", program_entry->type);
        //kstd::printf("\tVirtual Address: 0x%llx\n", program_entry->p_vaddr);
        //kstd::printf("\tSegment Length: 0x%llx\n", program_entry->p_memsz);
        //kstd::printf("\tFile sz: 0x%llx\n", program_entry->p_filesz);

        if (program_entry->type == 1)
            elf_move_data(obj, program_entry->p_vaddr, obj->elf_bin + program_entry->p_offset, program_entry->p_filesz);
        bochs_breakpoint();
    }

}


void elf_invoke_object(elf_object_t* obj)
{
    elf_trampoline(obj->pml4e_dir_physical, obj->start);

    kstd::printf("Invoking the object.\n");
}

void elf_destroy_object(elf_object_t* obj)
{
    // Don't forget to free everything in the way of first 256 entries of PML4e!

    pmm_free_page(reinterpret_cast<uint64_t>(obj->pml4e_dir_physical));
    delete obj;
}