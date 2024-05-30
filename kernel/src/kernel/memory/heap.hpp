//
// Created by Piotr on 13.05.2024.
//

#ifndef KITTY_OS_CPP_HEAP_HPP
#define KITTY_OS_CPP_HEAP_HPP

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <kstd/kstdio.hpp>
#include <kernel/memory/pmm.hpp>
#include <kernel/memory/vmm.hpp>
#include <control/control.hpp>

enum malloc_state
{
    MALLOC_STATE_END_OF_MEMORY, // its usable tho
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

    // Free memory
    size_t available_memory = 0;
    size_t used_memory = 0;
    size_t commited_memory_pages = 0;

    // Base address
    uint64_t base_address = 0;
public:
    void init();
    void commit_page(); // Add page and map it.
    void* alloc(size_t len);
    void free(void* ptr);
};

void heap_init();
void* kmalloc(size_t len);
void kfree(void* ptr);
void commit_page();


#endif //KITTY_OS_CPP_HEAP_HPP
