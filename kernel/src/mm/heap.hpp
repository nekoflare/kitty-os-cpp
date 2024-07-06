//
// Created by Piotr on 13.05.2024.
//

#ifndef KITTY_OS_CPP_HEAP_HPP
#define KITTY_OS_CPP_HEAP_HPP

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <kstd/kstdio.hpp>
#include <mm/pmm.hpp>
#include <mm/vmm.hpp>
#include <arch/x64/control/control.hpp>

enum malloc_state
{
    MALLOC_STATE_FREE,
    MALLOC_STATE_USED
};

struct malloc_tag
{
    size_t length;
    malloc_state state;
};

class Heap
{
private:
    pml4e* pml4e_pointer = nullptr;
    size_t pml4_entry = 0;

    // Last entries where we mapped data.
    size_t last_pdpe_entry = 0;
    size_t last_pde_entry = 0;
    size_t last_pte_entry = 0;
public:
    // Free memory
    size_t available_memory = 0;
    size_t used_memory = 0;

    size_t committed_memory_pages = 0;
    uint64_t base_address = 0;
    size_t entry_count = 0;

    void init();
    void commit_page(); // Add page and map it.
    void free(void* ptr);
    void merge_block();
    void print_memory_entries();

    void* alloc_normal(size_t len);
};

void heap_init();
void* kmalloc(size_t len);
void kfree(void* ptr);
void commit_page();
void merge_blocks();
void heap_print_entries();
uint64_t heap_get_available_memory();
uint64_t heap_get_used_memory();
uint64_t heap_get_used_virtual_memory();

#endif //KITTY_OS_CPP_HEAP_HPP
