#pragma once

#include <limine.h>
#include <stdint.h>

uintptr_t get_higher_half_offset();

inline void *get_pml4()
{
    uint64_t cr3 = 0;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    cr3 &= 0xfffffffffffff000;
    cr3 += get_higher_half_offset();
    return reinterpret_cast<void *>(cr3);
}

uint64_t get_kernel_base_address();
uint64_t get_highest_memory_map_address();
limine_memmap_response *get_memory_map();
void* get_kernel_page_map();
void set_kernel_page_map();