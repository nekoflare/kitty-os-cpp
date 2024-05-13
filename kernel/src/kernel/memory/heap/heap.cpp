//
// Created by Piotr on 13.05.2024.
//

#include "heap.hpp"

void Heap::init()
{
    kstd::printf("Initializing heap.\n");

    this->pml4_entry = 0;

    // Find free pml4e entry.

    // Get pml4e
    this->pml4e_pointer = reinterpret_cast<pml4e*>(get_logical_address_pml4());

    // Starting from 256 because that's where kernel virtual memory space starts.
    for (size_t i = 256; 512 > i; i++)
    {
        if (this->pml4e_pointer[i].pdpe_base_address == 0)
        {
            this->pml4_entry = i;
        }
    }

    if (this->pml4_entry == 0)
    {
        kstd::printf("Couldn't find free PML4e entry.\n");

        return;
    }

    // https://wiki.osdev.org/Writing_a_memory_manager

    // Allocate first page and go to the n
}

void Heap::commit_page()
{
    void* page = pmsa_alloc_page();

    if (page == nullptr)
    {
        kstd::printf("Failure commiting a page!\n");
        unreachable();
    }

    // map the memory
    if (this->last_pte_entry > 512)
    {
        this->last_pte_entry = 0;
        this->last_pde_entry ++;
    }

    if (this->last_pde_entry > 512)
    {
        this->last_pde_entry = 0;
        this->last_pdpe_entry ++;
    }

    if (this->last_pdpe_entry > 512)
    {
        kstd::printf("Out of virtual memory!\n");
        kstd::printf("How does one waste 512 GB of virtual memory!??\n");
    }

    // page it in
    uint64_t va = mm_create_va(false, this->pml4_entry, this->last_pdpe_entry, this->last_pde_entry, this->last_pte_entry, 0);

    vmsa_map_page(
            this->pml4e_pointer,
            va,
            reinterpret_cast<uint64_t>(page),
            PROT_SUPERVISOR | PROT_EXEC | PROT_RW,
            MAP_PRESENT,
            MISC_INVLPG,
            0
            );

    this->last_pte_entry ++;
}

static Heap heap;

void* kmalloc(size_t len)
{
    return nullptr;
}

void kfree(void* ptr)
{
    return;
}

void commit_page()
{
    heap.commit_page();
}

void heap_init()
{
    heap.init();
}