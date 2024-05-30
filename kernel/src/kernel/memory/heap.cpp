//
// Created by Piotr on 13.05.2024.
//

#include "heap.hpp"

void Heap::init()
{
    kstd::printf("Initializing heap.\n");

    this->pml4_entry = 0;
    this->last_pte_entry = 0;
    this->last_pde_entry = 0;
    this->last_pdpe_entry = 0;

    // Find free pml4e entry.

    // Get pml4e
    this->pml4e_pointer = vmm_make_virtual<struct pml4e*>(vmm_get_pml4());

    // Starting from 256 because that's where kernel virtual memory space starts.
    for (size_t i = 256; 512 > i; i++)
    {
        if (!this->pml4e_pointer[i].pdpe_ptr)
        {
            this->pml4_entry = i;
        }
    }

    if (this->pml4_entry == 0)
    {
        kstd::printf("Couldn't find free PML4e entry.\n");

        return;
    }

    this->base_address = vmm_create_virtual_address(false, this->pml4_entry, 0, 0, 0, 0);

    kstd::printf("PML4e entry: %lx\n", this->pml4_entry);
    kstd::printf("PDPe entry: %lx\n", this->last_pdpe_entry);
    kstd::printf("PDe entry: %lx\n", this->last_pde_entry);
    kstd::printf("PTe entry: %lx\n", this->last_pte_entry);

    // https://wiki.osdev.org/Writing_a_memory_manager

    this->commit_page();
}

void Heap::commit_page()
{
    void* page = reinterpret_cast<void*>(pmm_alloc_page());

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
    uint64_t va = vmm_create_virtual_address(false, this->pml4_entry, this->last_pdpe_entry, this->last_pde_entry, this->last_pte_entry, 0);
    kstd::printf("[HEAP] Entry: %llx %llx %llx %llx\n",this->pml4_entry, this->last_pdpe_entry, this->last_pde_entry, this->last_pte_entry );
    bool ok = vmm_map(
            this->pml4e_pointer,
            va,
            reinterpret_cast<uint64_t>(page),
            PROT_SUPERVISOR | PROT_RW,
            MAP_PRESENT,
            MISC_INVLPG
            );

    if (!ok)
    {
        kstd::printf("kys"); // bro make it more better bro
        bochs_breakpoint();

        unreachable();
    }

    this->last_pte_entry ++;

    // tag the memory
    struct malloc_tag* tag = reinterpret_cast<struct malloc_tag*>(va);

    kstd::printf("Tag address: %lx\n", tag);

    tag->state = MALLOC_STATE_FREE;
    tag->length = 4096 - sizeof (struct malloc_tag);

    // update the commitment memory.
    this->available_memory += (4096 - sizeof(struct malloc_tag));
    this->commited_memory_pages ++;
}


void* Heap::alloc(size_t len)
{
    // Get the first entry.
    struct malloc_tag* this_tag = reinterpret_cast<struct malloc_tag*>(this->base_address);
    kstd::printf("%llx\n", this_tag);

    if (len % 16 != 0)
    {
        kstd::printf("Auto aligning the length.\n");
        len += (16 - (len % 16)) % 16;
        kstd::printf("New size: %llx.\n", len);
    }

    // Iterate through the tags to find a suitable free block.
    while (true)
    {
        if (this_tag->state == MALLOC_STATE_END_OF_MEMORY) {
            // Reached the end of memory, no suitable block found.
            return nullptr;
        }

        if (this_tag->state == MALLOC_STATE_FREE && this_tag->length >= (len + sizeof(struct malloc_tag)))
        {
            // Found a suitable free block.
            // Record the original length.
            uint64_t original_length = this_tag->length;

            // Adjust the length of the current tag to reflect the allocated block.
            this_tag->length = len;
            this_tag->state = MALLOC_STATE_USED;

            // Calculate the address for the new tag.
            uint64_t new_tag_address = reinterpret_cast<uint64_t>(this_tag) + len + sizeof(struct malloc_tag);
            struct malloc_tag* new_tag = reinterpret_cast<struct malloc_tag*>(new_tag_address);

            // Check if the new tag address is within the heap's available memory.
            if (new_tag_address + sizeof(struct malloc_tag) > this->base_address + this->available_memory) {
                // If the new tag would exceed available memory, set the current tag's state to end of memory.
                this_tag->state = MALLOC_STATE_END_OF_MEMORY;
            } else {
                // Set up the new tag with the remaining length.
                new_tag->length = original_length - len - sizeof(struct malloc_tag);
                new_tag->state = MALLOC_STATE_FREE;

                // Update available and used memory.
                this->available_memory -= (len + sizeof(struct malloc_tag));
                this->used_memory += (len + sizeof(struct malloc_tag));
            }

            // Return the address after the current tag's metadata.
            return reinterpret_cast<void*>(reinterpret_cast<uint64_t>(this_tag) + sizeof(struct malloc_tag));
        }

        // Move to the next tag.
        this_tag = reinterpret_cast<struct malloc_tag*>(reinterpret_cast<uint64_t>(this_tag) + this_tag->length + sizeof(struct malloc_tag));
    }
}


void Heap::free(void* ptr)
{
    // Check if ptr is nullptr
    if (ptr == nullptr) {
        return; // Nothing to free
    }

    // Calculate the address of the corresponding malloc_tag
    struct malloc_tag* tag_to_free = reinterpret_cast<struct malloc_tag*>(reinterpret_cast<uint64_t>(ptr) - sizeof(struct malloc_tag));

    // Mark the tag as free
    tag_to_free->state = MALLOC_STATE_FREE;

    // Update used_memory and available_memory
    this->used_memory -= (tag_to_free->length + sizeof(struct malloc_tag));
    this->available_memory += (tag_to_free->length + sizeof(struct malloc_tag));

    kstd::printf("Freed memory: %llx.\n", ptr);
}

static Heap heap;

void* kmalloc(size_t len)
{
    return heap.alloc(len);
}

void kfree(void* ptr)
{
    heap.free(ptr);
}

void commit_page()
{
    heap.commit_page();
}

void heap_init()
{
    heap.init();
}